// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Common/InventoryComponent.h"
#include "Items/ItemType.h"
#include "BTT_LootHouse.generated.h"

class ABaseItem;
class ASurvivorPawn;

// BTT_LootHouse.h
UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTT_LootHouse : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_LootHouse();
	virtual uint16 GetInstanceMemorySize() const override;
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent&, uint8*) override;
	virtual void TickTask(UBehaviorTreeComponent&, uint8*, float) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent&, uint8*, EBTNodeResult::Type) override;
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector HouseActorKey;
	UPROPERTY(EditAnywhere, Category = "Loot") int32 TargetFoodCount{ 2 };
	UPROPERTY(EditAnywhere, Category = "Loot") int32 TargetMedkitCount{ 2 };
private:
	bool IsWorthy(ABaseItem* Item, UInventoryComponent* Inv) const;
	int32 CountOfType(UInventoryComponent* Inv, EItemType Type) const;
	bool AttemptPickup(ASurvivorPawn* Survivor, ABaseItem* Item) const;
};

struct FLootHouseMemory
{
	TArray<TWeakObjectPtr<ABaseItem>> ItemsInHouse;
	int32 CurrentIndex{ 0 };
	bool bIsMoving{ false };
};