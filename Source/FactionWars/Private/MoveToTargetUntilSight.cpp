// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveToTargetUntilSight.h"

#include "ActiveAbilityContainer.h"
#include "AIController_FW.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "FactionWars/FactionWarsCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UMoveToTargetUntilSight::UMoveToTargetUntilSight(FObjectInitializer const& _objIni)
{
	NodeName = TEXT("Move To Target Until Sight");
}

EBTNodeResult::Type UMoveToTargetUntilSight::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type NodeResult = EBTNodeResult::InProgress;

	AAIController_FW* aiController = Cast<AAIController_FW>(OwnerComp.GetAIOwner());

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

	UObject* targetObj = BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName);
	AFactionWarsCharacter* target = Cast<AFactionWarsCharacter>(targetObj);

	if (nullptr != target)
	{
		aiChara->SetTarget(target);
	}

	UObject* abilityContainerObj = BlackboardComp->GetValueAsObject(AbilityContainerKey.SelectedKeyName);
	UActiveAbilityContainer* abilityContainer = Cast<UActiveAbilityContainer>(abilityContainerObj);

	if (abilityContainer)
	{
		AFactionWarsCharacter* computedTarget = abilityContainer->ComputeTarget_Ability(aiChara->GetTarget());
		if (abilityContainer->CheckTargetDistance(computedTarget) == true && aiChara->
			CheckTargetVisibility(computedTarget) == true)
		{
			aiController->StopMovement();

			const FRotator targetLookAt = UKismetMathLibrary::FindLookAtRotation(
				aiChara->GetActorLocation(), computedTarget->GetActorLocation());
			const FRotator newRotation(aiChara->GetActorRotation().Pitch, targetLookAt.Yaw,
			                           aiChara->GetActorRotation().Roll);

			aiChara->SetActorRotation(newRotation);

			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return EBTNodeResult::Succeeded;
		}
	}

	NodeResult = Super::ExecuteTask(OwnerComp, NodeMemory);
	return NodeResult;
}
