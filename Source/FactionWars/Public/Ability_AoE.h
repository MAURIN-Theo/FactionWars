// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability.h"
#include "Ability_AoE.generated.h"

//Forward declarations
class UDecalComponent;

UCLASS()
class FACTIONWARS_API AAbility_AoE : public AAbility
{
	GENERATED_BODY()

public:
	AAbility_AoE();

	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDecalComponent* Decal = nullptr;
	
	virtual void ActivateEffect() override;

	virtual void TickEffect() override;

protected:
	virtual void SetupPositionAoE();

private:
	void InitDecal();
	void EnableVFX() const;
	void SetCollisionToOverlap() const;
};
