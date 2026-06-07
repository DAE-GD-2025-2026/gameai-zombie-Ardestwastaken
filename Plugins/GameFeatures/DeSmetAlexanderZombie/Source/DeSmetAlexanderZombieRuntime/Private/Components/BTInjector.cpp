#include "Components/BTInjector.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTInjector::UBTInjector()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBTInjector::BeginPlay()
{
	Super::BeginPlay();

	if (!BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTInjector: No BehaviorTreeAsset assigned! "
		       "Set it in the GameFeature DataAsset."));
		return;
	}

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	// Delay one tick so the AIController is guaranteed to be assigned.
	// (Controller is set by the GameMode after BeginPlay fires on the pawn.)
	GetWorld()->GetTimerManager().SetTimerForNextTick([this, Pawn]()
	{
		AAIController* Controller = Cast<AAIController>(Pawn->GetController());
		if (!Controller)
		{
			UE_LOG(LogTemp, Warning, TEXT("UBTInjector: Pawn has no AIController. "
			       "Make sure BP_SurvivorAIController is set as the AI controller class."));
			return;
		}

		// RunBehaviorTree starts the BT on the controller's BrainComponent.
		// It also creates and attaches the BlackboardComponent automatically
		// based on the Blackboard asset referenced inside the BT asset.
		Controller->RunBehaviorTree(BehaviorTreeAsset);
	});
}
