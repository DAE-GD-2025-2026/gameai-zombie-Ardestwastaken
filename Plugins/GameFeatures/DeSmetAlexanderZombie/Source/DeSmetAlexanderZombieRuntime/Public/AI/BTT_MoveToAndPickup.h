#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_MoveToAndPickup.generated.h"

class ASurvivorPawn;
class ABaseItem;
class UInventoryComponent;

UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTT_MoveToAndPickup : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_MoveToAndPickup();

	virtual uint16 GetInstanceMemorySize() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ItemActorKey;

private:
	
	float ScoreItem(ABaseItem* Item, ASurvivorPawn* Survivor) const;
	int32 FindFreeSlot(UInventoryComponent* Inv) const;
	int32 FindWorstSlot(UInventoryComponent* Inv, ASurvivorPawn* Survivor) const;
	void  AttemptPickup(ASurvivorPawn* Survivor, ABaseItem* Item) const;
	void  ClearItemKey(UBehaviorTreeComponent& OwnerComp) const;
};
