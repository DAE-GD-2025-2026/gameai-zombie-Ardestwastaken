#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BTInjector.generated.h"

/**
 * UBTInjector
 *
 * This component is added to the SurvivorPawn by the GameFeature "Add Components"
 * action. Its only job is to hand the survivor's AIController a Behavior Tree
 * to run, one frame after BeginPlay (to guarantee the controller is assigned).
 *
 * HOW TO USE IN EDITOR
 * --------------------
 * 1. In your GameFeature DataAsset, add an "Add Components" action.
 * 2. Set the Actor class to BP_Survivor.
 * 3. Add UBTInjector to the component list.
 * 4. After adding the action, find the BehaviorTreeAsset property and
 *    assign BT_Survivor to it.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DESMETALEXANDERZOMBIERUNTIME_API UBTInjector : public UActorComponent
{
	GENERATED_BODY()

public:
	UBTInjector();
	virtual void BeginPlay() override;

	/** Assign BT_Survivor here in the GameFeature DataAsset. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
};
