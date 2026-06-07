// BTService_UpdateBlackboard.h
#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateBlackboard.generated.h"

UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTService_UpdateBlackboard : public UBTService
{
    GENERATED_BODY()
public:
    UBTService_UpdateBlackboard();
protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetEnemyKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetItemKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector HealthKey;
};