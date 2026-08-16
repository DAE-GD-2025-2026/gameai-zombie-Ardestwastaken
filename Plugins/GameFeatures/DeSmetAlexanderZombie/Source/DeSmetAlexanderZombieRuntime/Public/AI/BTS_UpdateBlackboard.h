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

	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector ZombieActorKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector HasGunKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector IsLowHealthKey;
	UPROPERTY(EditAnywhere, Category = "Health") float LowHealthPercent{ 0.35f };

};