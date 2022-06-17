// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActiveAbilityContainer.generated.h"


//Forward declarations:
class AAbility;
class AFactionWarsCharacter;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeginCasting);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilityCasted);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTIONWARS_API UActiveAbilityContainer : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UActiveAbilityContainer();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AAbility> AbilityContained;

	UFUNCTION()
	bool PreCheckCanBeActivated() const;

	UFUNCTION(BlueprintCallable)
	bool CanAbilityBeCasted(AFactionWarsCharacter* _targetToTest = nullptr);

	UFUNCTION(BlueprintCallable)
	AFactionWarsCharacter* ComputeTarget_Ability(AFactionWarsCharacter* _targetToCompute = nullptr);

	UFUNCTION(BlueprintCallable)
	bool CheckTargetDistance(AFactionWarsCharacter* _targetToTest);

	//ONLINE DELEGATE
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnBeginCasting OnBeginCasting;

	UFUNCTION(NetMulticast, Reliable)
	void CallOnBeginCasting_NetMulticast();
	//END ONLINE DELEGATE

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnAbilityCasted OnAbilityCasted;
	
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
	virtual void UseAbilityContained_Server();

	UFUNCTION(BlueprintCallable)
	virtual void BeginCooldown();

	UFUNCTION(BlueprintCallable)
	virtual void EndCooldown();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FTimerHandle AbilityCooldownTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool IsInCooldown = false;

	UPROPERTY()
	AFactionWarsCharacter* OwnerChara = nullptr;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
