// ZombiePerceptor.h
#pragma once
#include "CoreMinimal.h"
#include "StudentPerceptor.h"
#include "ZombiePerceptor.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DESMETALEXANDERZOMBIERUNTIME_API UZombiePerceptor : public UStudentPerceptor
{
    GENERATED_BODY()
public:
    virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;

    // Stored knowledge
    UPROPERTY() TArray<AActor*> VisibleZombies;
    UPROPERTY() TArray<AActor*> KnownItems;
    UPROPERTY() TArray<AActor*> KnownHouses;

    AActor* GetClosestZombie() const;
    AActor* GetBestItem() const;
};