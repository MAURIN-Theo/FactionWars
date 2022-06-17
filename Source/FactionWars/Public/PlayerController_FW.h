// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_FW.generated.h"

UCLASS()
class FACTIONWARS_API APlayerController_FW : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void UseThisAbility(const int _abilityIndex);
};
