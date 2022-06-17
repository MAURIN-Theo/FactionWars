// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability_Projectile.h"

#include <string>

#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/SphereComponent.h"
#include "FactionWars/FactionWarsCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AAbility_Projectile::AAbility_Projectile()
{
	InitProjMovement();
}

void AAbility_Projectile::BeginCasting_NetMulticast()
{
	Super::BeginCasting_NetMulticast();

	RegisterOnProjHitEvent();
	SetHomingTarget();
}

void AAbility_Projectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AAbility_Projectile::ActivateEffect()
{
	Super::ActivateEffect();

	//Detach projectile to not follow caster...
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (nullptr != projectileMovement)
	{
		projectileMovement->Activate(true);
	}
}

void AAbility_Projectile::OnProjHitEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	if (Target == nullptr)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, (TEXT("Projectile Target nullptr")));
		MyDestroy();
		return;
	}

	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
			                                 FString::Printf(TEXT("I Hit: %s"), *OtherActor->GetName()));

		if (OtherActor == Target)
		{
			SpawnProjEmitter(SweepResult);
			UGameplayStatics::ApplyDamage(Target, AbilityDetails.Damage, GetCaster()->Controller,
			                              this, AbilityDetails.DamageType);
			MyDestroy();
		}
	}
}

void AAbility_Projectile::SetHomingTarget_Implementation() const
{
	if (nullptr != Target && nullptr != Caster && nullptr != projectileMovement)
	{
		projectileMovement->HomingTargetComponent = Target->GetRootComponent();
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Error %hs"), __FUNCTION__));
		}
	}
}

void AAbility_Projectile::InitProjMovement()
{
	projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	if (nullptr != projectileMovement)
	{
		projectileMovement->InitialSpeed = 800.f;
		projectileMovement->MaxSpeed = 800.f;
		projectileMovement->ProjectileGravityScale = 0.f;
		projectileMovement->bIsHomingProjectile = true;
		projectileMovement->HomingAccelerationMagnitude = 100000.f;
		projectileMovement->bRotationFollowsVelocity = true;
		projectileMovement->SetIsReplicated(true);

		projectileMovement->SetAutoActivate(false);
	}
}

void AAbility_Projectile::SpawnProjEmitter_Implementation(const FHitResult& Hit) const
{
	if (nullptr != ProjPS_Impact)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ProjPS_Impact, Hit.Location);
	}
}

void AAbility_Projectile::RegisterOnProjHitEvent()
{
	if (nullptr != RootComponent)
	{
		UShapeComponent* RootCasted = Cast<UShapeComponent>(RootComponent);

		if (nullptr != RootCasted)
		{
			RootCasted->OnComponentBeginOverlap.AddDynamic(this, &AAbility_Projectile::OnProjHitEvent);
		}
	}
}
