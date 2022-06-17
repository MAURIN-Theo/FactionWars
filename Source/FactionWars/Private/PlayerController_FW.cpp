// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController_FW.h"

#include "ActiveAbilityContainer.h"
#include "FactionWars/FactionWarsCharacter.h"

void APlayerController_FW::UseThisAbility(const int _abilityIndex)
{
	APawn* controlledPawn = GetPawn();

	if (nullptr == controlledPawn)
	{
		return;
	}

	AFactionWarsCharacter* charaOwned = Cast<AFactionWarsCharacter>(controlledPawn);

	if (nullptr == charaOwned)
	{
		return;
	}

	UActiveAbilityContainer* container = charaOwned->AbilityContainersArray[_abilityIndex];

	if (nullptr == container)
	{
		return;
	}

	container->UseAbilityContained_Server();
}
