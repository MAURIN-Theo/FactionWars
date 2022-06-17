// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ActivateAbility_BBT.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class FACTIONWARS_API UActivateAbility_BBT : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UActivateAbility_BBT(FObjectInitializer const& _objIni);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector AbilityContainerKey;
};
