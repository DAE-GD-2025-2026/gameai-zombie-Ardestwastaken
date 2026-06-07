#include "AI/BTS_UpdateBlackboard.h"
#include "AI/StudentPerceptor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "Survivor/SurvivorPawn.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

UBTS_UpdateBlackboard::UBTS_UpdateBlackboard()
{
	NodeName = TEXT("Update Blackboard");
	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UBTS_UpdateBlackboard::TickNode(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return;

	ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
	if (!Survivor) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	UStudentPerceptor* Perceptor = Survivor->FindComponentByClass<UStudentPerceptor>();
	if (!Perceptor) return;

	BB->SetValueAsObject(ZombieActorKey.SelectedKeyName, Perceptor->GetNearestVisibleZombie());

	BB->SetValueAsObject(ItemActorKey.SelectedKeyName, Perceptor->GetNearestVisibleItem());

	BB->SetValueAsObject(HouseActorKey.SelectedKeyName, Perceptor->GetNearestVisibleHouse());

	if (UHealthComponent* Health = Survivor->FindComponentByClass<UHealthComponent>())
	{
		const float Pct = static_cast<float>(Health->GetHealth()) /
			static_cast<float>(Health->GetMaxHealth());
		BB->SetValueAsBool(IsLowHealthKey.SelectedKeyName, Pct < LowHealthPercent);
	}

	if (UStaminaComponent* Stamina = Survivor->FindComponentByClass<UStaminaComponent>())
	{
		const float Pct = Stamina->GetCurrentStamina() / Stamina->GetMaxStamina();
		BB->SetValueAsBool(IsLowStaminaKey.SelectedKeyName, Pct < LowStaminaPercent);
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
				bHasGun = true; break;
			}
		}
	}
	BB->SetValueAsBool(HasGunKey.SelectedKeyName, bHasGun);
}
