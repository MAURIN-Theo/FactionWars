// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ComputeAbilitiesWeights_BBT.generated.h"


//Forward declarations
class UActiveAbilityContainer;
class AFactionWarsCharacter;


struct ComputedWeight
{
	float weight = 0.0f;
	AFactionWarsCharacter* target = nullptr;
	UActiveAbilityContainer* container = nullptr;
};

UCLASS(Blueprintable)
class FACTIONWARS_API UComputeAbilitiesWeights_BBT : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UComputeAbilitiesWeights_BBT(FObjectInitializer const& _objIni);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector AbilityContainerKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector AbilityDistanceKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector EQS_SpacingKey;
private:
	virtual ComputedWeight ComputeAbilityWeight(AFactionWarsCharacter* _owner, UActiveAbilityContainer* _container);
};
