// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivateAbility_BBT.h"

#include "ActiveAbilityContainer.h"
#include "AIController_FW.h"
#include "BehaviorTree/BlackboardComponent.h"

UActivateAbility_BBT::UActivateAbility_BBT(FObjectInitializer const& _objIni)
{
	NodeName = TEXT("Activate Ability X");
}

EBTNodeResult::Type UActivateAbility_BBT::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (nullptr == BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}
	
	UObject* abilityContainerObj = BlackboardComp->GetValueAsObject(AbilityContainerKey.SelectedKeyName);
	UActiveAbilityContainer* abilityContainer = Cast<UActiveAbilityContainer>(abilityContainerObj);

	if (nullptr == abilityContainer)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	abilityContainer->UseAbilityContained_Server();
	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}
