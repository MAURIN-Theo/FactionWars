// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactionWars/TargetType.h"
#include "GameFramework/Actor.h"
#include "AbilityEffect.generated.h"

USTRUCT(BlueprintType)
struct FAbilityEffectDetails
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "10000.0"))
	float ColliderRadius = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "10000.0"))
	float LifeSpan = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "10000.0"))
	float TickEffectTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* Tick_PS = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsTick_PS_Attached = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ETargetType> TargetType = ETargetType::EVERYONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
};

//Forward declarations :
class AFactionWarsCharacter;
class USphereComponent;

UCLASS()
class FACTIONWARS_API AAbilityEffect : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAbilityEffect();
	
	UFUNCTION(BlueprintCallable)
	virtual void TickEffect();
	
	UFUNCTION(BlueprintCallable)
	void ClearDuplicates(AFactionWarsCharacter* _target);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbilityEffectDetails AbilityEffectDetails;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* SphereCol = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystemComponent* AbilityParticleSystem = nullptr;

	UFUNCTION(BlueprintCallable)
	void Setup(AFactionWarsCharacter* _caster, AFactionWarsCharacter* _target);
	
	UPROPERTY(Replicated)
	AFactionWarsCharacter* Caster = nullptr;

	UPROPERTY(Replicated)
	AFactionWarsCharacter* Target = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY()
	FTimerHandle TickEffectTimer;

private:
	void InitSphereCol();

	void InitParticleSys();
};
