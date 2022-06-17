// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability.h"
#include "Ability_Melee.generated.h"

// Forward declarations
class UStaticMeshComponent;


UCLASS()
class FACTIONWARS_API AAbility_Melee : public AAbility
{
	GENERATED_BODY()

public:
	AAbility_Melee();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* detectionMesh = nullptr;

	virtual void ActivateEffect() override;

protected:
	virtual void BeginPlay() override;
};
