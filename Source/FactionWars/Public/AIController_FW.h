// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIController_FW.generated.h"

/**
 * 
 */
UCLASS()
class FACTIONWARS_API AAIController_FW : public AAIController
{
	GENERATED_BODY()

public:
	AAIController_FW(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int InitialTeamID = 1;
	
	UFUNCTION(BlueprintCallable)
	virtual void SetGenericTeamId(const FGenericTeamId& _TeamID) override;

	UFUNCTION(BlueprintCallable)
	virtual FGenericTeamId GetGenericTeamId() const override;

protected:
	virtual void BeginPlay() override;
};
