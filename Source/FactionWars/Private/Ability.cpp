// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability.h"

#include "AbilityEffect.h"
#include "ActiveAbilityContainer.h"
#include "ResourceComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "FactionWars/FactionWarsCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AAbility::AAbility()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	
	InitSphereCol();
	InitParticleSys();
}

void AAbility::MyDestroy()
{
	GetWorld()->GetTimerManager().SetTimer(AbilityDestroyTimer, this, &AAbility::DestroyFroDelegate, 0.1f, false);
}

// Called when the game starts or when spawned
void AAbility::BeginPlay()
{
	Super::BeginPlay();

	if (nullptr != SphereCol)
	{
		SphereCol->SetSphereRadius(AbilityDetails.ColliderRadius);
	}
}

// Called every frame
void AAbility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (nullptr != Caster && (nullptr != Target || Caster != Target))
	{
		if (AbilityDetails.IsLookingAtTarget == true && HasBeenActivated == false)
		{
			const FRotator targetLookAt = UKismetMathLibrary::FindLookAtRotation(
			Caster->GetActorLocation(), Target->GetActorLocation());
			const FRotator newRotation(Caster->GetActorRotation().Pitch, targetLookAt.Yaw, Caster->GetActorRotation().Roll);

			Caster->SetActorRotation(newRotation);
		}
	}
}

void AAbility::BeginCasting_NetMulticast_Implementation()
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Begin Casting!"));
	}*/
	if (nullptr != Container)
	{
		SetCaster(Container->OwnerChara);
		Container->OnBeginCasting.Broadcast();
	}

	
	if (nullptr != Caster)
	{
		Caster->IsCasting = true;
		
		if (Caster->ManaRes != nullptr)
		{
			Caster->ManaRes->ModifyResource(-AbilityDetails.Cost);
		}

		Caster->OnAbilitySpawned.Broadcast(this);

		USkeletalMeshSocket const* socketPtr = Caster->GetMesh()->GetSocketByName(FName(TEXT(HAND_SOCKET)));
		
		if (nullptr != socketPtr)
		{
			// Attach himself to the skeleton hand socket...
			const FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
			AttachToComponent(Caster->GetMesh(), rules, socketPtr->SocketName);
		}

		if (AbilityDetails.IsInterruptible)
		{
			Caster->OnInterrupt.AddDynamic(this, &AAbility::CastInterrupted);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Null Caster in Begin Casting"));
		}
	}
	
	PlayAnimMontage_Casting();
	
	if (AbilityDetails.CastTime > 0.0f)
	{
		const float computedCastTime = AbilityDetails.CastTime / Caster->GetCastingSpeed();
		
		// Cast ability at the end of the timer
		GetWorldTimerManager().SetTimer(CastingTimer, this, &AAbility::CastAbility, computedCastTime, false);
	}
	else
	{
		CastAbility();
	}
}

void AAbility::CastAbility()
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Ability Casted!"));
	}*/

	if (nullptr != Caster && AbilityDetails.IsInterruptible)
	{
		Caster->OnInterrupt.RemoveDynamic(this, &AAbility::CastInterrupted);
	}
	else if (nullptr == Caster)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Error CastAbility"));
		}
	}

	PlayAnimMontage_Cast();
}

void AAbility::CastInterrupted()
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Cast Interrupted!"));
	}*/

	if (nullptr != Caster)
	{
		//Stops current anim montage
		Caster->StopAnimMontage(nullptr);
		Caster->OnInterrupt.RemoveDynamic(this, &AAbility::CastInterrupted);

		if (nullptr != Container)
		{
			Container->BeginCooldown();
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Error CastInterrupted"));
		}
	}

	MyDestroy();
}

void AAbility::ActivateEffect()
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Activate Effect!"));
	}*/

	HasBeenActivated = true;
	
	if (nullptr != Container)
	{
		Container->OnAbilityCasted.Broadcast();
	}

	if (nullptr != Caster)
	{
		Caster->OnInterrupt.RemoveDynamic(this, &AAbility::CastInterrupted);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Error ActivateEffect, no caster"));
		}
	}

	SetLifeSpan(AbilityDetails.LifeSpan);

	if (AbilityDetails.TickEffectTime > FLT_EPSILON)
	{
		GetWorld()->GetTimerManager().SetTimer(TickEffectTimer, this, &AAbility::TickEffect,
		                                       AbilityDetails.TickEffectTime, true);
	}

	SpawnAbilityEffects();
}

void AAbility::TickEffect()
{
	if (nullptr != Tick_PS)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Tick_PS, GetActorLocation());
	}
}

void AAbility::PlayAnimMontage_Casting() const
{
	if (nullptr != Caster)
	{
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("PlayAnimMontage_Casting!"));
		}*/

		Caster->OnTryPlayingMontage.Broadcast(CastingAnimMontageName, Caster->GetCastingSpeed(), NAME_None);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Error PlayAnimMontage_Casting!"));
		}
	}
}

void AAbility::PlayAnimMontage_Cast()
{
	if (nullptr != Caster)
	{
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("PlayAnimMontage_Cast!"));
		}*/
		
		Caster->OnTryPlayingMontage.Broadcast(CastAnimMontageName, Caster->GetCastingSpeed(), MontageAnimSection);

		Caster->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(
			this, &AAbility::OnAnimMontageNotify);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Error PlayAnimMontage_Cast"));
		}
	}
}

void AAbility::OnAnimMontageNotify(FName NotifyName,
                                   const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Notify begins..."));
	}*/

	if (NotifyName != MontageNotifyName)
	{
		return;
	}

	if (nullptr != Caster)
	{
		//Remove itself from delegate
		Caster->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.RemoveDynamic(
			this, &AAbility::OnAnimMontageNotify);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Error OnAnimMontageNotify"));
		}
	}

	ActivateEffect();
}

void AAbility::InitSphereCol()
{
	SphereCol = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));

	if (nullptr != SphereCol)
	{
		SetRootComponent(SphereCol);
		SphereCol->InitSphereRadius(AbilityDetails.ColliderRadius);
	}
}

void AAbility::InitParticleSys()
{
	AbilityParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));

	if (nullptr != AbilityParticleSystem)
	{
		AbilityParticleSystem->SetupAttachment(RootComponent);
	}
}

void AAbility::SpawnAbilityEffects()
{
	if (nullptr != UGameplayStatics::GetGameMode(GetWorld()))
	{
		for (auto abilityEffect : AbilityEffects)
		{
			FTransform effectTrans = FTransform(Target->GetTransform());
			effectTrans.SetRotation(FQuat::Identity);
		
			AAbilityEffect* abilityEffSpawned = GetWorld()->SpawnActorDeferred<AAbilityEffect>(abilityEffect, effectTrans);
		
			if (nullptr != abilityEffSpawned)
			{
				abilityEffSpawned->Setup(Caster, Target);
				UGameplayStatics::FinishSpawningActor(abilityEffSpawned, effectTrans);
			}
		}
	}
}

void AAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAbility, Caster);
	DOREPLIFETIME(AAbility, Target);
	DOREPLIFETIME(AAbility, HasBeenActivated);
	DOREPLIFETIME(AAbility, Container);
}
