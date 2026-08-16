#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "BTT_UseItem.generated.h"

UCLASS()
class DESMETALEXANDERZOMBIERUNTIME_API UBTT_UseItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_UseItem();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsLowHealthKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsLowStaminaKey;

	UPROPERTY(EditAnywhere, Category = "Item") EItemType DesiredType { EItemType::Medkit };
};
