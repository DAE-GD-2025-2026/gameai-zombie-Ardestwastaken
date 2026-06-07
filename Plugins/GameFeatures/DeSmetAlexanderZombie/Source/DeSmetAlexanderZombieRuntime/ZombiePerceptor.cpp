// ZombiePerceptor.cpp
#include "ZombiePerceptor.h"
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"
#include "Village/House/House.h"

void UZombiePerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    const bool bSensed = Stimulus.WasSuccessfullySensed();

    if (Actor->IsA<ABaseZombie>())
    {
        if (bSensed) VisibleZombies.AddUnique(Actor);
        else         VisibleZombies.Remove(Actor);
    }
    else if (Actor->IsA<ABaseItem>())
    {
        if (bSensed) KnownItems.AddUnique(Actor);
        else         KnownItems.Remove(Actor);
    }
    else if (Actor->IsA<AHouse>())
    {
        if (bSensed) KnownHouses.AddUnique(Actor);
    }
}

AActor* UZombiePerceptor::GetClosestZombie() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;
    AActor* Closest = nullptr;
    float BestDist = FLT_MAX;
    for (AActor* Z : VisibleZombies)
    {
        if (!IsValid(Z)) continue;
        float D = FVector::Dist(Owner->GetActorLocation(), Z->GetActorLocation());
        if (D < BestDist) { BestDist = D; Closest = Z; }
    }
    return Closest;
}

AActor* UZombiePerceptor::GetBestItem() const
{
    // Medkits first, then food, then weapons, ignore garbage
    // (you'll refine this later)
    for (AActor* I : KnownItems)
        if (IsValid(I)) return I;
    return nullptr;
}