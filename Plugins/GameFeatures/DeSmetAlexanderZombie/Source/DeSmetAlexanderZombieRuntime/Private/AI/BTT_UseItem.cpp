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

EBTNodeResult::Type UBTT_UseItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	if (!Survivor) return EBTNodeResult::Failed;

	UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return EBTNodeResult::Failed;

	const TArray<ABaseItem*>& Items = Inv->GetInventory();
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i] && Items[i]->GetItemType() == DesiredType && Items[i]->GetValue() > 0)
		{
			Inv->UseItem(i);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}