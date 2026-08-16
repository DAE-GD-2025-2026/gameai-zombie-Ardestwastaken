#include "AI/BTT_ShootAtZombie.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Survivor/SurvivorPawn.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

UBTT_ShootAtZombie::UBTT_ShootAtZombie()
{
	NodeName = TEXT("Shoot At Zombie");
	bNotifyTick = false; 
}

EBTNodeResult::Type UBTT_ShootAtZombie::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	if (!Controller || !Survivor || !BB) return EBTNodeResult::Failed;

	AActor* Zombie = Cast<AActor>(BB->GetValueAsObject(ZombieActorKey.SelectedKeyName));
	if (!Zombie) return EBTNodeResult::Failed;

	const float Dist = FVector::Dist(Survivor->GetActorLocation(), Zombie->GetActorLocation());
	if (Dist > MaxShootRange) return EBTNodeResult::Failed;

	Controller->SetFocalPoint(Zombie->GetActorLocation());

	UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return EBTNodeResult::Failed;

	const TArray<ABaseItem*>& Items = Inv->GetInventory();

	int32 PistolSlot = -1, ShotgunSlot = -1;
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (!Items[i] || Items[i]->GetValue() <= 0) continue;
		if (Items[i]->GetItemType() == EItemType::Pistol && PistolSlot < 0) PistolSlot = i;
		if (Items[i]->GetItemType() == EItemType::Shotgun && ShotgunSlot < 0) ShotgunSlot = i;
	}
	const int32 SlotToUse = (ShotgunSlot >= 0) ? ShotgunSlot : PistolSlot; // shotgun preferred
	if (SlotToUse < 0) return EBTNodeResult::Failed;

	Inv->UseItem(SlotToUse);

	if (Items[SlotToUse] && Items[SlotToUse]->GetValue() <= 0)
	{
		Inv->RemoveItem(SlotToUse);
	}

	return EBTNodeResult::Succeeded;
}
