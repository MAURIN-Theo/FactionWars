// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum ETargetType
{
	SELF_ONLY UMETA(DisplayName = "SelfOnly"),
	ENEMY UMETA(DisplayName = "Enemy"),
	ALLY UMETA(DisplayName = "Ally"),
	ALLY_AND_SELF UMETA(DisplayName = "Ally And Self"),
	EVERYONE UMETA(DisplayName = "Everyone"),
	MAX_TARGET_TYPE
};
