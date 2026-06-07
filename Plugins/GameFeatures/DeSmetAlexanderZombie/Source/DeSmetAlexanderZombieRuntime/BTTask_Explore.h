// BTTask_Explore.h
#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Explore.generated.h"

UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTTask_Explore : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_Explore();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, Category="Explore")
    float WanderRadius = 3000.f;

    UPROPERTY(EditAnywhere, Category="Explore")
    float AcceptanceRadius = 200.f;

private:
    FVector Destination = FVector::ZeroVector;
};