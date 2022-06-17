// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "MoveToTargetUntilSight.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class FACTIONWARS_API UMoveToTargetUntilSight : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UMoveToTargetUntilSight(FObjectInitializer const& _objIni);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector AbilityContainerKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetKey;
};
