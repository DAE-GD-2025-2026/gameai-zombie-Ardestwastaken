#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTS_UpdateBlackboard.generated.h"

UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTS_UpdateBlackboard : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_UpdateBlackboard();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;

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
	float LowStaminaPercent{ 0.10f };
};