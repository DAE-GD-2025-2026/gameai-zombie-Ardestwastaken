#include "AI/BTT_UpdateBlackboard.h"
#include "AI/StudentPerceptor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Survivor/SurvivorPawn.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

UBTT_UpdateBlackboard::UBTT_UpdateBlackboard()
{
	NodeName = TEXT("Update Blackboard");
	bNotifyTick = false; 
}

EBTNodeResult::Type UBTT_UpdateBlackboard::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
	if (!Survivor) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	UStudentPerceptor* Perceptor = Survivor->FindComponentByClass<UStudentPerceptor>();
	if (Perceptor)
	{
		BB->SetValueAsObject(ZombieActorKey.SelectedKeyName, Perceptor->GetNearestVisibleZombie());
		BB->SetValueAsObject(ItemActorKey.SelectedKeyName, Perceptor->GetNearestVisibleItem());
		BB->SetValueAsObject(HouseActorKey.SelectedKeyName, Perceptor->GetNearestVisibleHouse());
	}

	if (UHealthComponent* Health = Survivor->FindComponentByClass<UHealthComponent>())
	{
		const float HealthPct = static_cast<float>(Health->GetHealth())
		                      / static_cast<float>(Health->GetMaxHealth());
		BB->SetValueAsBool(IsLowHealthKey.SelectedKeyName, HealthPct < LowHealthPercent);
	}

	if (UStaminaComponent* Stamina = Survivor->FindComponentByClass<UStaminaComponent>())
	{
		const float StaminaPct = Stamina->GetCurrentStamina() / Stamina->GetMaxStamina();
		BB->SetValueAsBool(IsLowStaminaKey.SelectedKeyName, StaminaPct < LowStaminaPercent);
	}

	bool bHasGun = false;
	if (UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>())
	{
		for (ABaseItem* Item : Inv->GetInventory())
		{
			if (Item && Item->GetValue() > 0 &&
				(Item->GetItemType() == EItemType::Pistol ||
				 Item->GetItemType() == EItemType::Shotgun))
			{
				bHasGun = true;
				break;
			}
		}
	}
	BB->SetValueAsBool(HasGunKey.SelectedKeyName, bHasGun);

	return EBTNodeResult::Succeeded;
}
