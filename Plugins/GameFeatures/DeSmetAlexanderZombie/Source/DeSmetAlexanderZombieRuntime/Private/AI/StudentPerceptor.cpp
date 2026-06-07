#include "AI/StudentPerceptor.h"

#include "Survivor/SurvivorPawn.h"
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"
#include "Village/House/House.h"
#include "Perception/AIPerceptionComponent.h"

UStudentPerceptor::UStudentPerceptor()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();

	ASurvivorPawn* Pawn = Cast<ASurvivorPawn>(GetOwner());
	if (!Pawn) return;

	UAIPerceptionComponent* Perception = Pawn->FindComponentByClass<UAIPerceptionComponent>();
	if (Perception)
	{
		Perception->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Cast<ABaseZombie>(Actor))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (!VisibleZombies.Contains(Actor))
			{
				VisibleZombies.Add(Actor);
			}
			LastKnownZombieLocation = Actor->GetActorLocation();
		}
		else
		{
			VisibleZombies.Remove(Actor);
		}
		NearestZombie = nullptr;
		float BestDist = TNumericLimits<float>::Max();
		const FVector MyLoc = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
		for (TWeakObjectPtr<AActor> Z : VisibleZombies)
		{
			if (!Z.IsValid()) continue;
			const float D = FVector::DistSquared(MyLoc, Z->GetActorLocation());
			if (D < BestDist) { BestDist = D; NearestZombie = Z.Get(); }
		}
		return;
	}

	if (Cast<ABaseItem>(Actor))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (!VisibleItems.Contains(Actor))
				VisibleItems.Add(Actor);
		}
		else
		{
			VisibleItems.Remove(Actor);
		}
		NearestItem = nullptr;
		float BestDist = TNumericLimits<float>::Max();
		const FVector MyLoc = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
		for (TWeakObjectPtr<AActor> I : VisibleItems)
		{
			if (!I.IsValid()) continue;
			const float D = FVector::DistSquared(MyLoc, I->GetActorLocation());
			if (D < BestDist) { BestDist = D; NearestItem = I.Get(); }
		}
		return;
	}

	if (Cast<AHouse>(Actor))
	{
		if (Stimulus.WasSuccessfullySensed())
			NearestHouse = Actor;
		else if (Actor == NearestHouse)
			NearestHouse = nullptr;
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
		bRecentlyDamaged = true;
}
