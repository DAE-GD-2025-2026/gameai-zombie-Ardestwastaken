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

void UBTS_UpdateBlackboard::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Survivor = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Survivor || !BB) return;

	if (UStudentPerceptor* Perceptor = Survivor->FindComponentByClass<UStudentPerceptor>())
	{
		BB->SetValueAsObject(ZombieActorKey.SelectedKeyName, Perceptor->GetNearestVisibleZombie());
	}

	if (UHealthComponent* Health = Survivor->FindComponentByClass<UHealthComponent>())
	{
		const float Pct = (float)Health->GetHealth() / (float)Health->GetMaxHealth();
		BB->SetValueAsBool(IsLowHealthKey.SelectedKeyName, Pct < LowHealthPercent);
	}

	bool bHasGun = false;
	if (UInventoryComponent* Inv = Survivor->FindComponentByClass<UInventoryComponent>())
	{
		for (ABaseItem* Item : Inv->GetInventory())
		{
			if (Item && (Item->GetItemType() == EItemType::Pistol || Item->GetItemType() == EItemType::Shotgun))
			{
				bHasGun = true;
				break;
			}
		}
	}
	BB->SetValueAsBool(HasGunKey.SelectedKeyName, bHasGun);
}