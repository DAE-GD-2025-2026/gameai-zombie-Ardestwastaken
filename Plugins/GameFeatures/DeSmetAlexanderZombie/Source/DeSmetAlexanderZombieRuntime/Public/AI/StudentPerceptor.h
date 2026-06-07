#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "StudentPerceptor.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DESMETALEXANDERZOMBIERUNTIME_API UStudentPerceptor : public UActorComponent
{
	GENERATED_BODY()

public:
	UStudentPerceptor();
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	AActor* GetNearestVisibleZombie() const { return NearestZombie; }

	FVector GetLastKnownZombieLocation() const { return LastKnownZombieLocation; }

	bool HasVisibleZombie() const { return NearestZombie != nullptr; }

	AActor* GetNearestVisibleItem() const { return NearestItem; }

	AActor* GetNearestVisibleHouse() const { return NearestHouse; }

	bool WasRecentlyDamaged() const { return bRecentlyDamaged; }
	void ClearDamageFlag() { bRecentlyDamaged = false; }

private:
	TArray<TWeakObjectPtr<AActor>> VisibleZombies;
	TArray<TWeakObjectPtr<AActor>> VisibleItems;

	UPROPERTY() TObjectPtr<AActor> NearestZombie{ nullptr };
	UPROPERTY() TObjectPtr<AActor> NearestItem { nullptr };
	UPROPERTY() TObjectPtr<AActor> NearestHouse { nullptr };

	FVector LastKnownZombieLocation{ FVector::ZeroVector };
	bool bRecentlyDamaged { false };
};
