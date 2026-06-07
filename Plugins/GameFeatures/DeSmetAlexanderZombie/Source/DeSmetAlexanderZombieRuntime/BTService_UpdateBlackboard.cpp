// BTService_UpdateBlackboard.cpp
#include "BTService_UpdateBlackboard.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "ZombiePerceptor.h"
#include "Survivor/SurvivorPawn.h"
#include "Common/HealthComponent.h"

UBTService_UpdateBlackboard::UBTService_UpdateBlackboard()
{
    NodeName = TEXT("Update Blackboard");
    Interval = 0.1f;
}

void UBTService_UpdateBlackboard::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return;

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    UZombiePerceptor* Perc = Pawn->FindComponentByClass<UZombiePerceptor>();
    if (Perc)
    {
        BB->SetValueAsObject(TargetEnemyKey.SelectedKeyName, Perc->GetClosestZombie());
        BB->SetValueAsObject(TargetItemKey.SelectedKeyName, Perc->GetBestItem());
    }

    if (ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Pawn))
    {
        if (UHealthComponent* HC = Survivor->FindComponentByClass<UHealthComponent>())
        {
            float Norm = (float)HC->GetHealth() / (float)HC->GetMaxHealth();
            BB->SetValueAsFloat(HealthKey.SelectedKeyName, Norm);
        }
    }
}