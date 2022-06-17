// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability.h"
#include "Ability_Projectile.generated.h"

//Forward declarations...
class UProjectileMovementComponent;

UCLASS()
class FACTIONWARS_API AAbility_Projectile : public AAbility
{
	GENERATED_BODY()

public:
	AAbility_Projectile();

	//Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* projectileMovement = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* ProjPS_Impact = nullptr;

	virtual void BeginCasting_NetMulticast() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void ActivateEffect() override;

	UFUNCTION()
	virtual void OnProjHitEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool
	                            bFromSweep, const FHitResult& SweepResult);

private:
	UFUNCTION(NetMulticast, Reliable)
	void SetHomingTarget() const;
	
	void InitProjMovement();

	UFUNCTION(NetMulticast, Reliable)
	void SpawnProjEmitter(const FHitResult& Hit) const;
	void RegisterOnProjHitEvent();
};
