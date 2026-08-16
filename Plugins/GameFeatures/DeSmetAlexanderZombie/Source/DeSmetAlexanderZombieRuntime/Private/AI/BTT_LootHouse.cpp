#include "AI/BTT_LootHouse.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Survivor/SurvivorPawn.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Village/House/House.h"

UBTT_LootHouse::UBTT_LootHouse()
{
	NodeName = TEXT("Loot House");
	bNotifyTick = true;
}

uint16 UBTT_LootHouse::GetInstanceMemorySize() const
{
	return sizeof(FLootHouseMemory);
}

int32 UBTT_LootHouse::CountOfType(UInventoryComponent* Inv, EItemType Type) const
{
	int32 Count = 0;
	for (ABaseItem* Item : Inv->GetInventory())
		if (Item && Item->GetItemType() == Type) ++Count;
	return Count;
}

bool UBTT_LootHouse::IsWorthy(ABaseItem* Item, UInventoryComponent* Inv) const
{
	if (!Item || Item->GetValue() <= 0 || Item->GetItemType() == EItemType::Garbage)
		return false;

	switch (Item->GetItemType())
	{
	case EItemType::Food:
		return CountOfType(Inv, EItemType::Food) < TargetFoodCount;

	case EItemType::Medkit:
		return CountOfType(Inv, EItemType::Medkit) < TargetMedkitCount;

	case EItemType::Pistol:
		// Only worth grabbing a pistol if we're currently unarmed.
		return CountOfType(Inv, EItemType::Pistol) + CountOfType(Inv, EItemType::Shotgun) == 0;

	case EItemType::Shotgun:
		// Worth it if unarmed, or as an upgrade over a pistol we're already holding.
		return CountOfType(Inv, EItemType::Shotgun) == 0;

	default:
		return false;
	}
}

bool UBTT_LootHouse::AttemptPickup(ASurvivorPawn* Survivor, ABaseItem* Item) const
{
	UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>();
	if (!Inv || !Item) return false;

	const TArray<ABaseItem*>& Items = Inv->GetInventory();

	int32 FreeSlot = -1;
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (!Items[i]) { FreeSlot = i; break; }
	}

	if (FreeSlot >= 0)
	{
		Inv->GrabItem(FreeSlot, Item);
		return true;
	}

	// No free slot: the only legitimate swap at a full, on-target loadout
	// is upgrading a held Pistol into a Shotgun.
	if (Item->GetItemType() == EItemType::Shotgun)
	{
		for (int32 i = 0; i < Items.Num(); ++i)
		{
			if (Items[i] && Items[i]->GetItemType() == EItemType::Pistol)
			{
				Inv->RemoveItem(i);
				Inv->GrabItem(i, Item);
				return true;
			}
		}
	}

	return false;
}

EBTNodeResult::Type UBTT_LootHouse::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FLootHouseMemory* Memory = reinterpret_cast<FLootHouseMemory*>(NodeMemory);
	Memory->ItemsInHouse.Empty();
	Memory->CurrentIndex = 0;
	Memory->bIsMoving = false;

	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Controller || !Survivor || !BB) return EBTNodeResult::Failed;

	AHouse* House = Cast<AHouse>(BB->GetValueAsObject(HouseActorKey.SelectedKeyName));
	if (!House) return EBTNodeResult::Failed;

	const FHouseBounds Bounds = House->GetBounds();
	const FBox HouseBox(Bounds.Origin - Bounds.Extent, Bounds.Origin + Bounds.Extent);

	TArray<AActor*> AllItems;
	UGameplayStatics::GetAllActorsOfClass(Survivor->GetWorld(), ABaseItem::StaticClass(), AllItems);

	for (AActor* A : AllItems)
	{
		if (A && HouseBox.IsInside(A->GetActorLocation()))
		{
			Memory->ItemsInHouse.Add(Cast<ABaseItem>(A));
		}
	}

	if (Memory->ItemsInHouse.IsEmpty())
		return EBTNodeResult::Succeeded; // empty house, nothing to do

	return EBTNodeResult::InProgress; // TickTask drives the per-item loop
}

void UBTT_LootHouse::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FLootHouseMemory* Memory = reinterpret_cast<FLootHouseMemory*>(NodeMemory);

	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	UInventoryComponent* Inv = Survivor ? Survivor->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!Controller || !Survivor || !Inv)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Finish the in-flight move (if any) before considering the next item.
	if (Memory->bIsMoving)
	{
		UPathFollowingComponent* PF = Controller->GetPathFollowingComponent();
		if (!PF || PF->GetStatus() != EPathFollowingStatus::Idle)
			return; // still walking there

		Memory->bIsMoving = false;

		if (ABaseItem* Item = Memory->ItemsInHouse[Memory->CurrentIndex].Get())
		{
			AttemptPickup(Survivor, Item);
		}
		Memory->CurrentIndex++;
	}

	// Walk the remaining items, skipping anything not (or no longer) worthy —
	// re-checked live, since earlier pickups this same loot pass change the quotas.
	while (Memory->CurrentIndex < Memory->ItemsInHouse.Num())
	{
		ABaseItem* Item = Memory->ItemsInHouse[Memory->CurrentIndex].Get();
		if (!Item || !IsValid(Item) || !IsWorthy(Item, Inv))
		{
			Memory->CurrentIndex++;
			continue;
		}

		FAIMoveRequest MoveReq(Item);
		MoveReq.SetAcceptanceRadius(Inv->GetPickupRange());
		MoveReq.SetUsePathfinding(true);
		const FPathFollowingRequestResult Result = Controller->MoveTo(MoveReq);

		if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			AttemptPickup(Survivor, Item);
			Memory->CurrentIndex++;
			continue;
		}
		if (Result.Code == EPathFollowingRequestResult::Failed)
		{
			Memory->CurrentIndex++;
			continue;
		}

		Memory->bIsMoving = true;
		return; // wait for the move to finish, resumed next tick
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // whole house done
}

void UBTT_LootHouse::OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FLootHouseMemory* Memory = reinterpret_cast<FLootHouseMemory*>(NodeMemory);

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller) Controller->StopMovement();

	Memory->ItemsInHouse.Empty();
	Memory->bIsMoving = false;
}