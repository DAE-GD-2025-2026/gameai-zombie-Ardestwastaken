#include "AI/SurvivorPerceptor.h"

#include "Survivor/SurvivorPawn.h"
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"
#include "Village/House/House.h"

#include "Perception/AIPerceptionComponent.h"

USurvivorPerceptor::USurvivorPerceptor()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurvivorPerceptor::BeginPlay()
{
	Super::BeginPlay();

	ASurvivorPawn* Pawn = Cast<ASurvivorPawn>(GetOwner());
	if (!Pawn) return;

	UAIPerceptionComponent* Perception = Pawn->FindComponentByClass<UAIPerceptionComponent>();
	if (Perception)
	{
		Perception->OnTargetPerceptionUpdated.AddDynamic(this, &USurvivorPerceptor::OnPerceptionUpdated);
	}
}

void USurvivorPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{

	if (Cast<ABaseZombie>(Actor))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			NearestZombie = Actor;
			LastKnownZombieLocation = Actor->GetActorLocation();
		}
		else
		{
			NearestZombie = nullptr;
		}
	}
	else if (Cast<ABaseItem>(Actor))
	{
		if (Stimulus.WasSuccessfullySensed())
			NearestItem = Actor;
		else if (Actor == NearestItem)
			NearestItem = nullptr;
	}
	else if (Cast<AHouse>(Actor))
	{
		if (Stimulus.WasSuccessfullySensed())
			NearestHouse = Actor;
		else if (Actor == NearestHouse)
			NearestHouse = nullptr;
	}
	else
	{
		if (Stimulus.WasSuccessfullySensed())
			bRecentlyDamaged = true;
	}
}
