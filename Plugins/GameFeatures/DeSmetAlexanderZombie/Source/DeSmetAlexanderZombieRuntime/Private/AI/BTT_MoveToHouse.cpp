// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTT_MoveToHouse.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"

#include "Survivor/SurvivorPawn.h"
#include "Village/House/House.h"

struct FMoveToHouseMemory { bool bIsMoving{ false }; };

UBTT_MoveToHouse::UBTT_MoveToHouse() { NodeName = TEXT("Move To House"); bNotifyTick = true; }
uint16 UBTT_MoveToHouse::GetInstanceMemorySize() const { return sizeof(FMoveToHouseMemory); }

void UBTT_MoveToHouse::GatherHouses(UWorld* World)
{
	AllHouses.Empty();
	for (TActorIterator<AHouse> It(World); It; ++It)
		AllHouses.Add(*It);

	UnvisitedIndices.Empty();
	for (int32 i = 0; i < AllHouses.Num(); ++i)
		UnvisitedIndices.Add(i);
}

FVector UBTT_MoveToHouse::GetNavigableInsidePoint(AHouse* House) const
{
	const FHouseBounds Bounds = House->GetBounds();
	const FVector CenterXY = FVector(Bounds.Origin.X, Bounds.Origin.Y, House->GetActorLocation().Z);

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(House->GetWorld());
	if (!NavSys) return CenterXY;

	FNavLocation NavLoc;
	const FVector Extent(Bounds.Extent.X * 0.5f, Bounds.Extent.Y * 0.5f, 100.f);
	if (NavSys->ProjectPointToNavigation(CenterXY, NavLoc, Extent))
		return NavLoc.Location;

	FNavLocation FallbackLoc;
	if (NavSys->ProjectPointToNavigation(House->GetActorLocation(), FallbackLoc, FVector(300, 300, 100)))
		return FallbackLoc.Location;

	return House->GetActorLocation();
}

EBTNodeResult::Type UBTT_MoveToHouse::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* Memory = reinterpret_cast<FMoveToHouseMemory*>(NodeMemory);
	Memory->bIsMoving = false;

	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Controller || !Survivor || !BB) return EBTNodeResult::Failed;

	if (AllHouses.IsEmpty()) GatherHouses(Survivor->GetWorld());
	if (UnvisitedIndices.IsEmpty())
	{
		for (int32 i = 0; i < AllHouses.Num(); ++i) UnvisitedIndices.Add(i); // refill, tour again
	}
	if (UnvisitedIndices.IsEmpty()) return EBTNodeResult::Failed; // no houses in level

	int32 BestSlot = 0;
	float BestDistSq = TNumericLimits<float>::Max();
	const FVector MyLoc = Survivor->GetActorLocation();
	for (int32 Slot = 0; Slot < UnvisitedIndices.Num(); ++Slot)
	{
		const float DSq = FVector::DistSquared(MyLoc, AllHouses[UnvisitedIndices[Slot]]->GetActorLocation());
		if (DSq < BestDistSq) { BestDistSq = DSq; BestSlot = Slot; }
	}
	AHouse* Target = AllHouses[UnvisitedIndices[BestSlot]];
	UnvisitedIndices.RemoveAtSwap(BestSlot);

	BB->SetValueAsObject(HouseActorKey.SelectedKeyName, Target);

	const FVector InsidePoint = GetNavigableInsidePoint(Target);
	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(InsidePoint);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);
	MoveReq.SetUsePathfinding(true);

	const auto Result = Controller->MoveTo(MoveReq);
	if (Result.Code == EPathFollowingRequestResult::Failed) return EBTNodeResult::Failed;
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal) return EBTNodeResult::Succeeded;

	Memory->bIsMoving = true;
	return EBTNodeResult::InProgress;
}

void UBTT_MoveToHouse::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float)
{
	auto* Memory = reinterpret_cast<FMoveToHouseMemory*>(NodeMemory);
	if (!Memory->bIsMoving) return;
	AAIController* Controller = OwnerComp.GetAIOwner();
	UPathFollowingComponent* PF = Controller ? Controller->GetPathFollowingComponent() : nullptr;
	if (!PF || PF->GetStatus() != EPathFollowingStatus::Idle) return;
	Memory->bIsMoving = false;
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

void UBTT_MoveToHouse::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type)
{
	if (AAIController* Controller = OwnerComp.GetAIOwner()) Controller->StopMovement();
	reinterpret_cast<FMoveToHouseMemory*>(NodeMemory)->bIsMoving = false;
}
