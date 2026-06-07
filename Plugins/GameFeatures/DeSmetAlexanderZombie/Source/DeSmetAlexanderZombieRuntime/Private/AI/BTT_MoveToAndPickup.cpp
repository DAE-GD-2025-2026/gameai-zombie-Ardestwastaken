#include "AI/BTT_MoveToAndPickup.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

#include "Survivor/SurvivorPawn.h"
#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

struct FMoveToAndPickupMemory
{
	bool bIsMoving{ false };
};

UBTT_MoveToAndPickup::UBTT_MoveToAndPickup()
{
	NodeName = TEXT("Move To And Pickup");
	bNotifyTick = true;
}

uint16 UBTT_MoveToAndPickup::GetInstanceMemorySize() const
{
	return sizeof(FMoveToAndPickupMemory);
}

float UBTT_MoveToAndPickup::ScoreItem(ABaseItem* Item, ASurvivorPawn* Survivor) const
{
	if (!Item || Item->GetValue() <= 0)            return -999.f;
	if (Item->GetItemType() == EItemType::Garbage) return -999.f;

	const UHealthComponent* Health   = Survivor->FindComponentByClass<UHealthComponent>();
	const UStaminaComponent* Stamina = Survivor->FindComponentByClass<UStaminaComponent>();
	const UInventoryComponent* Inv   = Survivor->FindComponentByClass<UInventoryComponent>();

	const float HealthPct  = Health  ? static_cast<float>(Health->GetHealth()) / static_cast<float>(Health->GetMaxHealth()) : 1.f;
	const float StaminaPct = Stamina ? Stamina->GetCurrentStamina() / Stamina->GetMaxStamina() : 1.f;

	bool bAlreadyHasGun = false;
	if (Inv)
	{
		for (ABaseItem* Held : Inv->GetInventory())
		{
			if (Held && (Held->GetItemType() == EItemType::Pistol ||
				Held->GetItemType() == EItemType::Shotgun))
			{
				bAlreadyHasGun = true; break;
			}
		}
	}

	switch (Item->GetItemType())
	{
	case EItemType::Medkit:  return (1.f - HealthPct)  * 100.f;
	case EItemType::Food:    return (1.f - StaminaPct) * 70.f;
	case EItemType::Pistol:  return bAlreadyHasGun ? 20.f : 80.f;
	case EItemType::Shotgun: return bAlreadyHasGun ? 15.f : 60.f;
	default:                 return -999.f;
	}
}

int32 UBTT_MoveToAndPickup::FindFreeSlot(UInventoryComponent* Inv) const
{
	const TArray<ABaseItem*>& Items = Inv->GetInventory();
	for (int32 i = 0; i < Items.Num(); ++i)
		if (Items[i] == nullptr) return i;
	return -1;
}

int32 UBTT_MoveToAndPickup::FindWorstSlot(UInventoryComponent* Inv, ASurvivorPawn* Survivor) const
{
	const TArray<ABaseItem*>& Items = Inv->GetInventory();
	int32 WorstSlot  = 0;
	float WorstScore = TNumericLimits<float>::Max();
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		const float Score = ScoreItem(Items[i], Survivor);
		if (Score < WorstScore) { WorstScore = Score; WorstSlot = i; }
	}
	return WorstSlot;
}

void UBTT_MoveToAndPickup::AttemptPickup(ASurvivorPawn* Survivor, ABaseItem* Item) const
{
	if (!Survivor || !Item || !IsValid(Item)) return;
	UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return;

	int32 Slot = FindFreeSlot(Inv);
	if (Slot < 0)
	{
		const int32 WorstSlot  = FindWorstSlot(Inv, Survivor);
		const float WorstScore = ScoreItem(Inv->GetInventory()[WorstSlot], Survivor);
		if (ScoreItem(Item, Survivor) <= WorstScore) return;
		Inv->RemoveItem(WorstSlot);
		Slot = WorstSlot;
	}
	Inv->GrabItem(Slot, Item);
}

void UBTT_MoveToAndPickup::ClearItemKey(UBehaviorTreeComponent& OwnerComp) const
{
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
		BB->SetValueAsObject(ItemActorKey.SelectedKeyName, nullptr);
}

EBTNodeResult::Type UBTT_MoveToAndPickup::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FMoveToAndPickupMemory* Memory = reinterpret_cast<FMoveToAndPickupMemory*>(NodeMemory);
	Memory->bIsMoving = false;

	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	if (!Controller || !Survivor || !BB) return EBTNodeResult::Failed;

	ABaseItem* Item = Cast<ABaseItem>(BB->GetValueAsObject(ItemActorKey.SelectedKeyName));

	if (!Item || !IsValid(Item))
	{
		ClearItemKey(OwnerComp);
		return EBTNodeResult::Failed;
	}

	const float ItemScore = ScoreItem(Item, Survivor);
	if (ItemScore < 0.f)
	{
		ClearItemKey(OwnerComp);
		return EBTNodeResult::Failed;
	}

	UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return EBTNodeResult::Failed;

	if (FindFreeSlot(Inv) < 0)
	{
		const int32 WorstSlot = FindWorstSlot(Inv, Survivor);
		const float WorstScore = ScoreItem(Inv->GetInventory()[WorstSlot], Survivor);
		if (ItemScore <= WorstScore)
		{
			ClearItemKey(OwnerComp);
			return EBTNodeResult::Failed;
		}
	}

	FAIMoveRequest MoveReq(Item);
	MoveReq.SetAcceptanceRadius(Inv->GetPickupRange());
	MoveReq.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = Controller->MoveTo(MoveReq);
	if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		ClearItemKey(OwnerComp);
		return EBTNodeResult::Failed;
	}
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		AttemptPickup(Survivor, Item);
		ClearItemKey(OwnerComp);
		return EBTNodeResult::Succeeded;
	}

	Memory->bIsMoving = true;
	return EBTNodeResult::InProgress;
}

void UBTT_MoveToAndPickup::TickTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, float DeltaSeconds)
{
	FMoveToAndPickupMemory* Memory = reinterpret_cast<FMoveToAndPickupMemory*>(NodeMemory);
	if (!Memory->bIsMoving) return;

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	UPathFollowingComponent* PF = Controller->GetPathFollowingComponent();
	if (!PF || PF->GetStatus() != EPathFollowingStatus::Idle) return;

	Memory->bIsMoving = false;

	ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	ABaseItem* Item = BB ? Cast<ABaseItem>(BB->GetValueAsObject(ItemActorKey.SelectedKeyName)) : nullptr;

	AttemptPickup(Survivor, Item);
	ClearItemKey(OwnerComp);
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

void UBTT_MoveToAndPickup::OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FMoveToAndPickupMemory* Memory = reinterpret_cast<FMoveToAndPickupMemory*>(NodeMemory);

	if (TaskResult != EBTNodeResult::Succeeded)
	{
		ClearItemKey(OwnerComp);
	}

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller) Controller->StopMovement();
	Memory->bIsMoving = false;
}
