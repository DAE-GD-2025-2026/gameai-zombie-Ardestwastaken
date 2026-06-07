#include "AI/BTT_MoveToExplore.h"

#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Village/House/House.h"
#include "GameFramework/Pawn.h"

struct FMoveToExploreMemory
{
	bool bIsMoving{ false };
};

UBTT_MoveToExplore::UBTT_MoveToExplore()
{
	NodeName = TEXT("Move To Explore");
	bNotifyTick = true;
}

uint16 UBTT_MoveToExplore::GetInstanceMemorySize() const
{
	return sizeof(FMoveToExploreMemory);
}

void UBTT_MoveToExplore::GatherHouses(UWorld* World)
{
	AllHouses.Empty();
	for (TActorIterator<AHouse> It(World); It; ++It)
		AllHouses.Add(*It);

	UnvisitedIndices.Empty();
	for (int32 i = 0; i < AllHouses.Num(); ++i)
		UnvisitedIndices.Add(i);
}

AActor* UBTT_MoveToExplore::PickNextTarget(const FVector& FromLocation)
{
	if (UnvisitedIndices.IsEmpty())
	{
		for (int32 i = 0; i < AllHouses.Num(); ++i)
			UnvisitedIndices.Add(i);
	}
	if (UnvisitedIndices.IsEmpty()) return nullptr;

	int32 BestSlot = 0;
	float BestDistSq = TNumericLimits<float>::Max();
	for (int32 Slot = 0; Slot < UnvisitedIndices.Num(); ++Slot)
	{
		AActor* House = AllHouses[UnvisitedIndices[Slot]];
		if (!House) continue;
		const float DSq = FVector::DistSquared(FromLocation, House->GetActorLocation());
		if (DSq < BestDistSq) { BestDistSq = DSq; BestSlot = Slot; }
	}

	AActor* Chosen = AllHouses[UnvisitedIndices[BestSlot]];
	UnvisitedIndices.RemoveAtSwap(BestSlot);
	return Chosen;
}

FVector UBTT_MoveToExplore::GetNavigableInsidePoint(AHouse* House) const
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

EBTNodeResult::Type UBTT_MoveToExplore::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FMoveToExploreMemory* Memory = reinterpret_cast<FMoveToExploreMemory*>(NodeMemory);
	Memory->bIsMoving = false;

	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (!Controller || !Pawn) return EBTNodeResult::Failed;

	if (AllHouses.IsEmpty())
		GatherHouses(GetWorld());

	if (AllHouses.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("BTT_MoveToExplore: No AHouse actors found in the world!"));
		return EBTNodeResult::Failed;
	}

	CurrentTarget = PickNextTarget(Pawn->GetActorLocation());
	if (!CurrentTarget) return EBTNodeResult::Failed;

	AHouse* House = Cast<AHouse>(CurrentTarget);
	const FVector InsidePoint = House ? GetNavigableInsidePoint(House) : CurrentTarget->GetActorLocation();

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(InsidePoint);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);
	MoveReq.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = Controller->MoveTo(MoveReq);
	if (Result.Code == EPathFollowingRequestResult::Failed)   return EBTNodeResult::Failed;
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal) return EBTNodeResult::Succeeded;

	Memory->bIsMoving = true;
	return EBTNodeResult::InProgress;
}

void UBTT_MoveToExplore::TickTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, float DeltaSeconds)
{
	FMoveToExploreMemory* Memory = reinterpret_cast<FMoveToExploreMemory*>(NodeMemory);
	if (!Memory->bIsMoving) return;

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	UPathFollowingComponent* PF = Controller->GetPathFollowingComponent();
	if (!PF || PF->GetStatus() != EPathFollowingStatus::Idle) return;

	Memory->bIsMoving = false;
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

void UBTT_MoveToExplore::OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FMoveToExploreMemory* Memory = reinterpret_cast<FMoveToExploreMemory*>(NodeMemory);

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller) Controller->StopMovement();
	Memory->bIsMoving = false;
}
