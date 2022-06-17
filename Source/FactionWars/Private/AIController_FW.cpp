// Fill out your copyright notice in the Description page of Project Settings.


#include "AIController_FW.h"

#include "FactionWars/FactionWarsCharacter.h"

AAIController_FW::AAIController_FW(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

void AAIController_FW::SetGenericTeamId(const FGenericTeamId& _TeamID)
{
	Super::SetGenericTeamId(_TeamID);

	AFactionWarsCharacter* charaControlled = Cast<AFactionWarsCharacter>(GetPawn());

	if (nullptr != charaControlled)
	{
		charaControlled->SetGenericTeamId(_TeamID);
	}
}

FGenericTeamId AAIController_FW::GetGenericTeamId() const
{
	return Super::GetGenericTeamId();
}

void AAIController_FW::BeginPlay()
{
	Super::BeginPlay();

	SetGenericTeamId(FGenericTeamId(InitialTeamID));
}
