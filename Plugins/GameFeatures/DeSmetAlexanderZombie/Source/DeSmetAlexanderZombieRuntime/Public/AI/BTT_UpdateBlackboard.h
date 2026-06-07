#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BTT_UpdateBlackboard.generated.h"

UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTT_UpdateBlackboard : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_UpdateBlackboard();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Perception")
	FBlackboardKeySelector ZombieActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Perception")
	FBlackboardKeySelector ItemActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Perception")
	FBlackboardKeySelector HouseActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Status")
	FBlackboardKeySelector HasGunKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Status")
	FBlackboardKeySelector IsLowHealthKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Status")
	FBlackboardKeySelector IsLowStaminaKey;

	UPROPERTY(EditAnywhere, Category = "Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHealthPercent{ 0.35f };

	UPROPERTY(EditAnywhere, Category = "Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowStaminaPercent{ 0.25f };
};
