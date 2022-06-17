// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability_Melee.h"

#include "Components/SphereComponent.h"
#include "FactionWars/FactionWarsCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AAbility_Melee::AAbility_Melee()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	detectionMesh = CreateDefaultSubobject<UStaticMeshComponent>("DetectionMesh");

	if (detectionMesh != nullptr && RootComponent != nullptr)
	{
		detectionMesh->SetupAttachment(RootComponent);
	}

	if (nullptr != AbilityParticleSystem)
	{
		AbilityParticleSystem->SetupAttachment(RootComponent);
	}

	SphereCol->DestroyComponent();
}

void AAbility_Melee::ActivateEffect()
{
	Super::ActivateEffect();

	TArray<AActor*> characters;
	detectionMesh->GetOverlappingActors(characters, TSubclassOf<AFactionWarsCharacter>());

	for (const auto chara : characters)
	{
		if (Caster->GetTeamAttitudeTowards(*chara) == ETeamAttitude::Hostile)
		{
			UGameplayStatics::ApplyDamage(chara, AbilityDetails.Damage, GetCaster()->Controller,
									  this, AbilityDetails.DamageType);
		}
	}

	MyDestroy();
}

void AAbility_Melee::BeginPlay()
{
	Super::BeginPlay();

	if (detectionMesh != nullptr)
	{
		detectionMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
}
