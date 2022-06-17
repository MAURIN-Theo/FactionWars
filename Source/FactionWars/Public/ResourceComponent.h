// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceComponent.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRequestUpdateResource);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmptyResource);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFullResource);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLostResource);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGainedResource);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTIONWARS_API UResourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UResourceComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	//ONLINE DELEGATE
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnRequestUpdateResource OnRequestUpdateResource;

	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
	void OnRequestUpdateResource_NetMulticast();
	//END ONLINE DELEGATE

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnEmptyResource OnEmptyResource;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnFullResource OnFullResource;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnLostResource OnLostResource;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGainedResource OnGainedResource;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	float Max = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	float Min = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	float Current = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	float RegenStrength = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	float RegenTickRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	bool NeedPerfectConsumption = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool IsRegenerating = false;

	UFUNCTION(BlueprintCallable)
	virtual bool CanConsumeResource(float _cost);

	UFUNCTION(BlueprintCallable)
	virtual void ModifyResource(float _value);

	UFUNCTION(BlueprintCallable)
	virtual float GetNormalizedResource() { return (Current - Min) / (Max - Min); };

	UFUNCTION(BlueprintCallable)
	virtual void SetValueFromNormalized(float _valueNormalized);

	UFUNCTION(BlueprintCallable)
	virtual void BeginRegenResource();

	UFUNCTION(BlueprintCallable)
	virtual void StopRegenResource();

	UFUNCTION()
	virtual void RegenResource();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	FTimerHandle RegenTimerHandle;
};
