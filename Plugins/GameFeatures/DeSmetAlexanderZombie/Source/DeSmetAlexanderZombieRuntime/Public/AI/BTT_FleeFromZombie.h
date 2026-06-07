#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FleeFromZombie.generated.h"

UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTT_FleeFromZombie : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_FleeFromZombie();

	virtual uint16 GetInstanceMemorySize() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ZombieActorKey;

	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeDistance{ 1200.f };

	UPROPERTY(EditAnywhere, Category = "Flee")
	float NavProjectionExtent{ 500.f };

};
