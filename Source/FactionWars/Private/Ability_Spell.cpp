// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability_Spell.h"

#include "FactionWars/FactionWarsCharacter.h"
#include "Kismet/GameplayStatics.h"

void AAbility_Spell::ActivateEffect()
{
	Super::ActivateEffect();

	if (nullptr == Target)
	{
		return;
	}

	AttachToActor(Target, FAttachmentTransformRules::SnapToTargetIncludingScale);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Tick_PS, GetActorLocation());
	UGameplayStatics::ApplyDamage(Target, AbilityDetails.Damage, GetCaster()->Controller,
										  this, AbilityDetails.DamageType);

	// Delay on destroy because the actor was destroyed before spawning emitter on clients...
	MyDestroy();
}