#include "AI/BTT_FleeFromZombie.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Survivor/SurvivorPawn.h"
#include "Common/StaminaComponent.h"
#include "Common/InventoryComponent.h"
#include "Common/InventoryComponent.h"

struct FFleeFromZombieMemory
{
	bool bIsMoving{ false };
};

UBTT_FleeFromZombie::UBTT_FleeFromZombie()
{
	NodeName = TEXT("Flee From Zombie");
	bNotifyTick = true;
}

uint16 UBTT_FleeFromZombie::GetInstanceMemorySize() const
{
	return sizeof(FFleeFromZombieMemory);
}

EBTNodeResult::Type UBTT_FleeFromZombie::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FFleeFromZombieMemory* Memory = reinterpret_cast<FFleeFromZombieMemory*>(NodeMemory);
	Memory->bIsMoving = false;

	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor   = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	if (!Controller || !Survivor || !BB) return EBTNodeResult::Failed;

	AActor* Zombie = Cast<AActor>(BB->GetValueAsObject(ZombieActorKey.SelectedKeyName));
	if (!Zombie) return EBTNodeResult::Failed;

	const FVector SurvivorLoc = Survivor->GetActorLocation();
	const FVector ZombieLoc   = Zombie->GetActorLocation();

	FVector FleeDir    = (SurvivorLoc - ZombieLoc).GetSafeNormal();
	FVector FleeTarget = SurvivorLoc + FleeDir * FleeDistance;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(FleeTarget, NavLoc,
			FVector(NavProjectionExtent, NavProjectionExtent, NavProjectionExtent)))
		{
			FleeTarget = NavLoc.Location;
		}
	}

	// Insert right before the sprint decision in ExecuteTask:
	UStaminaComponent* Stamina = Survivor->FindComponentByClass<UStaminaComponent>();
	UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>();

	const bool bStaminaEmpty = Stamina && (Stamina->GetCurrentStamina() <= 0.f);
	if (bStaminaEmpty && Inv)
	{
		const TArray<ABaseItem*>& Items = Inv->GetInventory();
		for (int32 i = 0; i < Items.Num(); ++i)
		{
			if (Items[i] && Items[i]->GetItemType() == EItemType::Food && Items[i]->GetValue() > 0)
			{
				Inv->UseItem(i);
				break;
			}
		}
	}

	const bool bHasStaminaToSprint = Stamina
		? (Stamina->GetCurrentStamina() / Stamina->GetMaxStamina()) > 0.30f
		: false;
	if (bHasStaminaToSprint) Survivor->StartRunning();

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(FleeTarget);
	MoveReq.SetAcceptanceRadius(100.f);
	MoveReq.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = Controller->MoveTo(MoveReq);

	if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		Survivor->StopRunning();
		return EBTNodeResult::Failed;
	}
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		Survivor->StopRunning();
		return EBTNodeResult::Succeeded;
	}

	Memory->bIsMoving = true;
	return EBTNodeResult::InProgress;
}

void UBTT_FleeFromZombie::TickTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, float DeltaSeconds)
{
	FFleeFromZombieMemory* Memory = reinterpret_cast<FFleeFromZombieMemory*>(NodeMemory);
	if (!Memory->bIsMoving) return;

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
	if (Survivor)
	{
		UStaminaComponent* Stamina = Survivor->FindComponentByClass<UStaminaComponent>();
		if (Stamina && Survivor->IsRunning())
		{
			const float StaminaPct = Stamina->GetCurrentStamina() / Stamina->GetMaxStamina();
			if (StaminaPct < 0.10f)
				Survivor->StopRunning();
		}
	}

	UPathFollowingComponent* PF = Controller->GetPathFollowingComponent();
	if (!PF || PF->GetStatus() != EPathFollowingStatus::Idle) return;

	Memory->bIsMoving = false;
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

void UBTT_FleeFromZombie::OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FFleeFromZombieMemory* Memory = reinterpret_cast<FFleeFromZombieMemory*>(NodeMemory);

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller)
	{
		Controller->StopMovement();
		if (ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn()))
			Survivor->StopRunning();
	}
	Memory->bIsMoving = false;
}
