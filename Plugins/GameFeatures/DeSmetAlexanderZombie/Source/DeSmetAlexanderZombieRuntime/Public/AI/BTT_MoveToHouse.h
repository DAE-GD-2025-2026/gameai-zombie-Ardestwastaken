// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_MoveToHouse.generated.h"

class AHouse;

/// BTT_MoveToHouse.h
UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTT_MoveToHouse : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_MoveToHouse();
	virtual uint16 GetInstanceMemorySize() const override;
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent&, uint8*) override;
	virtual void TickTask(UBehaviorTreeComponent&, uint8*, float) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent&, uint8*, EBTNodeResult::Type) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector HouseActorKey;
	UPROPERTY(EditAnywhere, Category = "Explore") float AcceptanceRadius{ 150.f };

private:
	TArray<AHouse*> AllHouses;
	TArray<int32> UnvisitedIndices;
	void GatherHouses(UWorld* World);
	FVector GetNavigableInsidePoint(AHouse* House) const;
};