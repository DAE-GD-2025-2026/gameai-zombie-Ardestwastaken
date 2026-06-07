#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ShootAtZombie.generated.h"

/**
 */
UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTT_ShootAtZombie : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_ShootAtZombie();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ZombieActorKey;

	/** Only attempt to shoot if zombie is within this range (Unreal Units). */
	UPROPERTY(EditAnywhere, Category = "Combat")
	float MaxShootRange{ 1200.f };
};
