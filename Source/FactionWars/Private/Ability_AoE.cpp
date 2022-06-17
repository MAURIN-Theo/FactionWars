// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability_AoE.h"

#include "Components/DecalComponent.h"
#include "Components/ShapeComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "FactionWars/FactionWarsCharacter.h"
#include "Kismet/GameplayStatics.h"

AAbility_AoE::AAbility_AoE()
{
	SetCollisionToOverlap();
	InitDecal();
}

void AAbility_AoE::BeginPlay()
{
	Super::BeginPlay();

	if (nullptr != AbilityParticleSystem)
	{
		AbilityParticleSystem->Deactivate();
	}
}

void AAbility_AoE::ActivateEffect()
{
	Super::ActivateEffect();

	SetActorRotation(FRotator::ZeroRotator);
	Decal->SetWorldRotation(FRotator(90.0f, 0.0f, 0.0f));

	SetupPositionAoE();

	EnableVFX();

	//Detach projectile to not follow caster...
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AAbility_AoE::TickEffect()
{
	Super::TickEffect();

	TArray<AActor*> characters;
	const ETeamAttitude::Type attitudeToTarget = Caster->GetTeamAttitudeTowards(*Target);
	
	GetOverlappingActors(characters, TSubclassOf<AFactionWarsCharacter>());

	for (const auto chara : characters)
	{
		if (Caster->GetTeamAttitudeTowards(*chara) == attitudeToTarget)
		{
			UGameplayStatics::ApplyDamage(chara, AbilityDetails.Damage, GetCaster()->Controller,
									  this, AbilityDetails.DamageType);
		}
	}
}

void AAbility_AoE::SetupPositionAoE()
{
	if (Target == nullptr)
	{
		return;
	}
	
	FHitResult hitResult = FHitResult();
	const FVector start = Target->GetActorLocation();
	const FVector end = start + FVector(0, 0, -10000);
	FCollisionQueryParams params = FCollisionQueryParams();
	params.AddIgnoredActor(Target);

	GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_WorldStatic, params);

	if (hitResult.IsValidBlockingHit())
	{
		SetActorLocation(hitResult.Location);
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,TEXT("No ground found for AoE"));
	}
}

void AAbility_AoE::InitDecal()
{
	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));

	if (nullptr != Decal)
	{
		Decal->SetupAttachment(RootComponent);
		Decal->SetVisibility(false);
	}
}

void AAbility_AoE::EnableVFX() const
{
	if (nullptr != AbilityParticleSystem)
	{
		AbilityParticleSystem->Activate();
	}

	if (nullptr != Decal)
	{
		Decal->SetVisibility(true);
	}
}

void AAbility_AoE::SetCollisionToOverlap() const
{
	if (nullptr != RootComponent)
	{
		UShapeComponent* RootCasted = Cast<UShapeComponent>(RootComponent);

		if (nullptr != RootCasted)
		{
			RootCasted->SetCollisionResponseToAllChannels(ECR_Overlap);
		}
	}
}
