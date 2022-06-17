// Fill out your copyright notice in the Description page of Project Settings.


#include "ComputeAbilitiesWeights_BBT.h"

#include "Ability.h"
#include "ActiveAbilityContainer.h"
#include "AIController_FW.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "FactionWars/FactionWarsCharacter.h"

UComputeAbilitiesWeights_BBT::UComputeAbilitiesWeights_BBT(FObjectInitializer const& _objIni)
{
	NodeName = TEXT("Compute Abilities Weights");
}

EBTNodeResult::Type UComputeAbilitiesWeights_BBT::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController_FW* aiController = Cast<AAIController_FW>(OwnerComp.GetAIOwner());
	float lastHighestWeight = 0.0f;
	ComputedWeight bestContainer;

	if (nullptr == aiController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (nullptr == BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	AFactionWarsCharacter* aiChara = Cast<AFactionWarsCharacter>(aiController->GetPawn());

	if (nullptr == aiChara)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	TArray<ComputedWeight> computedWeights;
	for (const auto container : aiChara->AbilityContainersArray)
	{
		const ComputedWeight computedWeight = ComputeAbilityWeight(aiChara, container);

		if (nullptr != computedWeight.container)
		{
			computedWeights.Add(computedWeight);
		}

		/*if (computedWeight.weight > lastHighestWeight)
		{
			lastHighestWeight = computedWeight.weight;
			bestContainer = computedWeight;
		}*/
	}

	if (computedWeights.Num() - 1 >= 0)
	{
		const int randomItem = FMath::RandRange(0, computedWeights.Num() - 1);
		bestContainer = computedWeights[randomItem];
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsObject(AbilityContainerKey.SelectedKeyName, bestContainer.container);
	BlackboardComp->SetValueAsObject(TargetKey.SelectedKeyName, bestContainer.target);

	const float distance = bestContainer.container->AbilityContained.GetDefaultObject()->AbilityDetails.Distance;
	BlackboardComp->SetValueAsFloat(AbilityDistanceKey.SelectedKeyName, distance - 50.f);

	BlackboardComp->SetValueAsFloat(EQS_SpacingKey.SelectedKeyName, distance / 4.f);

	UBehaviorTree* abilityBT = bestContainer.container->AbilityContained.GetDefaultObject()->PersonalBehaviorTree;
	OwnerComp.SetDynamicSubtree(FGameplayTag::RequestGameplayTag("AbilitySubTree"), abilityBT);

	aiChara->SetTarget(bestContainer.target);

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

ComputedWeight UComputeAbilitiesWeights_BBT::ComputeAbilityWeight(AFactionWarsCharacter* _owner,
                                                                  UActiveAbilityContainer* _container)
{
	TArray<ComputedWeight> computedWeights;

	for (const auto targetToTest : _owner->PossibleAllyTargets)
	{
		if (_container->PreCheckCanBeActivated() == true)
		{
			AFactionWarsCharacter* computedTarget = _container->ComputeTarget_Ability(targetToTest);

			if (nullptr != computedTarget)
			{
				ComputedWeight computedWeight;

				computedWeight.weight = 100.0f;
				computedWeight.target = computedTarget;
				computedWeight.container = _container;

				computedWeights.Add(computedWeight);
			}
		}
	}

	for (const auto targetToTest : _owner->PossibleHostileTargets)
	{
		if (_container->PreCheckCanBeActivated() == true)
		{
			AFactionWarsCharacter* computedTarget = _container->ComputeTarget_Ability(targetToTest);

			if (nullptr != computedTarget)
			{
				ComputedWeight computedWeight;

				computedWeight.weight = 100.0f;
				computedWeight.target = computedTarget;
				computedWeight.container = _container;

				computedWeights.Add(computedWeight);
			}
		}
	}

	if (computedWeights.Num() - 1 >= 0)
	{
		const int randomItem = FMath::RandRange(0, computedWeights.Num() - 1);
		return computedWeights[randomItem];
	}
	else
	{
		return ComputedWeight();
	}
}
