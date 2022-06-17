// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityEffect.h"

#include "Components/SphereComponent.h"
#include "FactionWars/FactionWarsCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AAbilityEffect::AAbilityEffect()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicatingMovement(true);

	InitSphereCol();
	InitParticleSys();
}

void AAbilityEffect::Setup(AFactionWarsCharacter* _caster, AFactionWarsCharacter* _target)
{
	Caster = _caster;
	Target = _target;
}

// Called when the game starts or when spawned
void AAbilityEffect::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilityEffectDetails.LifeSpan > 0)
	{
		SetLifeSpan(AbilityEffectDetails.LifeSpan + 0.001);
	}
	else
	{
		SetLifeSpan(AbilityEffectDetails.LifeSpan);
	}

	ClearDuplicates(Target);
	AttachToActor(Target, FAttachmentTransformRules::SnapToTargetIncludingScale);

	Target->OnStatusEffectActivated.Broadcast(this);
	Target->OnRequestUpdateUI.Broadcast();

	if (AbilityEffectDetails.TickEffectTime > FLT_EPSILON)
	{
		GetWorld()->GetTimerManager().SetTimer(TickEffectTimer, this, &AAbilityEffect::TickEffect,
		                                       AbilityEffectDetails.TickEffectTime, true);
	}
}

void AAbilityEffect::TickEffect()
{
	UGameplayStatics::ApplyDamage(Target, AbilityEffectDetails.Damage, Caster->Controller,
	                              this, AbilityEffectDetails.DamageType);

	if (nullptr != AbilityEffectDetails.Tick_PS)
	{
		if (AbilityEffectDetails.IsTick_PS_Attached == true && nullptr != Target)
		{
			UGameplayStatics::SpawnEmitterAttached(AbilityEffectDetails.Tick_PS, Target->GetRootComponent());
		}
		else
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), AbilityEffectDetails.Tick_PS, GetActorLocation());
		}
	}
}

void AAbilityEffect::ClearDuplicates(AFactionWarsCharacter* _target)
{
	if (nullptr == _target)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Error ClearDuplicates, no target"));
		}
		return;
	}

	TArray<AActor*> actors;
	TArray<AAbilityEffect*> actorsToDestroy;

	_target->GetAttachedActors(actors);

	for (AActor* actor : actors)
	{
		if (nullptr != actor && actor->GetClass() == GetClass() && actor != this)
		{
			AAbilityEffect* ability = Cast<AAbilityEffect>(actor);
			actorsToDestroy.AddUnique(ability);
		}
	}

	for (int i = actorsToDestroy.Num() - 1; i >= 0; i--)
	{
		actorsToDestroy[i]->Destroy();
	}
}

void AAbilityEffect::InitSphereCol()
{
	SphereCol = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));

	if (nullptr != SphereCol)
	{
		SetRootComponent(SphereCol);
		SphereCol->InitSphereRadius(AbilityEffectDetails.ColliderRadius);
	}
}

void AAbilityEffect::InitParticleSys()
{
	AbilityParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));

	if (nullptr != AbilityParticleSystem)
	{
		AbilityParticleSystem->SetupAttachment(RootComponent);
	}
}

void AAbilityEffect::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAbilityEffect, Caster);
	DOREPLIFETIME(AAbilityEffect, Target);
}
