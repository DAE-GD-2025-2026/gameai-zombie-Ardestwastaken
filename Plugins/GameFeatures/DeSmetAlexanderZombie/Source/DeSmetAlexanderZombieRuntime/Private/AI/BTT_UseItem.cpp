#include "AI/BTT_UseItem.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Survivor/SurvivorPawn.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

UBTT_UseItem::UBTT_UseItem()
{
	NodeName = TEXT("Use Emergency Item");
	bNotifyTick = false;
}

EBTNodeResult::Type UBTT_UseItem::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor   = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	if (!Survivor || !BB) return EBTNodeResult::Failed;

	UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return EBTNodeResult::Failed;

	const bool bNeedHealth  = BB->GetValueAsBool(IsLowHealthKey.SelectedKeyName);
	const bool bNeedStamina = BB->GetValueAsBool(IsLowStaminaKey.SelectedKeyName);

	const EItemType WantedFirst  = bNeedHealth  ? EItemType::Medkit : EItemType::Food;
	const EItemType WantedSecond = bNeedHealth  ? EItemType::Food   : EItemType::Medkit;

	const TArray<ABaseItem*>& Items = Inv->GetInventory();

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		ABaseItem* Item = Items[i];
		if (Item && Item->GetItemType() == WantedFirst && Item->GetValue() > 0)
		{
			Inv->UseItem(i);
			return EBTNodeResult::Succeeded;
		}
	}

	// Try primary need first
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		ABaseItem* Item = Items[i];
		if (Item && Item->GetItemType() == WantedFirst && Item->GetValue() > 0)
		{
			Inv->UseItem(i);
			return EBTNodeResult::Succeeded;
		}
	}

	// Always try secondary as fallback, not just when both flags are set
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		ABaseItem* Item = Items[i];
		if (Item && Item->GetItemType() == WantedSecond && Item->GetValue() > 0)
		{
			Inv->UseItem(i);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
