// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability.h"
#include "Ability_Spell.generated.h"

/**
 * 
 */
UCLASS()
class FACTIONWARS_API AAbility_Spell : public AAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateEffect() override;
};
