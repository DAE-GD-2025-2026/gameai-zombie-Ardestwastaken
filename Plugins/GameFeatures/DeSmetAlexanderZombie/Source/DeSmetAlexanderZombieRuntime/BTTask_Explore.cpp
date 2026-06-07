// BTTask_Explore.cpp
#include "BTTask_Explore.h"
#include "AIController.h"
#include "NavigationSystem.h"

#include "Navigation/PathFollowingComponent.h"

UBTTask_Explore::UBTTask_Explore()
{
    NodeName = TEXT("Explore");
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Explore::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation NavPoint;
    if (!NavSys || !NavSys->GetRandomReachablePointInRadius(Pawn->GetActorLocation(), WanderRadius, NavPoint))
        return EBTNodeResult::Failed;

    Destination = NavPoint.Location;

    // Capture the request result
    EPathFollowingRequestResult::Type MoveResult = Controller->MoveToLocation(Destination, AcceptanceRadius);

    if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        return EBTNodeResult::Succeeded;
    }
    else if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

    // Task stays active until the movement component says it finished
    return EBTNodeResult::InProgress;
}
void UBTTask_Explore::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    // Check if the AI has finished its current move request
    if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("REACHED END VIA PATHFOLLOWING"));
        }
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UBTTask_Explore::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AAIController* C = OwnerComp.GetAIOwner())
        C->StopMovement();
    return EBTNodeResult::Aborted;
}