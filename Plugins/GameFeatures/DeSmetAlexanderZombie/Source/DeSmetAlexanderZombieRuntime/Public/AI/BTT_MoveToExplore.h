#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_MoveToExplore.generated.h"

class AHouse;

UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTT_MoveToExplore : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_MoveToExplore();

	virtual uint16 GetInstanceMemorySize() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius{ 60.f };

private:

	UPROPERTY() TArray<TObjectPtr<AActor>> AllHouses;
	TArray<int32> UnvisitedIndices;
	UPROPERTY() TObjectPtr<AActor> CurrentTarget{ nullptr };

	void    GatherHouses(UWorld* World);
	AActor* PickNextTarget(const FVector& FromLocation);
	FVector GetNavigableInsidePoint(AHouse* House) const;
};
