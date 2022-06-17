// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactionWars/TargetType.h"
#include "GameFramework/Actor.h"
#include "Ability.generated.h"


#define SPHERE_COL_RAD 32.f

USTRUCT(BlueprintType)
struct FAbilityDetails
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CastTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1000.0"))
	float Distance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ColliderRadius = SPHERE_COL_RAD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifeSpan = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TickEffectTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsInterruptible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsLookingAtTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ETargetType> TargetType = ETargetType::EVERYONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
};

//Forward declarations :
class AFactionWarsCharacter;
class USphereComponent;
class UParticleSystemComponent;
class UAnimMontage;
class UActiveAbilityContainer;
class UBehaviorTree;
class AAbilityEffect;

UCLASS()
class FACTIONWARS_API AAbility : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAbility();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbilityDetails AbilityDetails;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<TSubclassOf<AAbilityEffect>> AbilityEffects;
	
	//Component and root
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* SphereCol = nullptr;

	//Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystemComponent* AbilityParticleSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* Tick_PS = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBehaviorTree* PersonalBehaviorTree = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimerHandle CastingTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimMontage")
	FString CastingAnimMontageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimMontage")
	FString CastAnimMontageName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimMontage")
	FName MontageAnimSection = "EndCastLoop";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimMontage")
	FName MontageNotifyName = "ActivateEffect";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	UActiveAbilityContainer* Container = nullptr;
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	virtual void BeginCasting_NetMulticast();

	UFUNCTION(BlueprintCallable)
	virtual void CastAbility();

	UFUNCTION(BlueprintCallable)
	virtual void CastInterrupted();

	UFUNCTION(BlueprintCallable)
	virtual void ActivateEffect();

	UFUNCTION(BlueprintCallable)
	virtual void TickEffect();

	//Getters Setters
	UFUNCTION(BlueprintCallable)
	void SetCaster(AFactionWarsCharacter* _caster) { Caster = _caster; }

	UFUNCTION(BlueprintCallable)
	AFactionWarsCharacter* GetCaster() { return Caster; }

	UFUNCTION(BlueprintCallable)
	void SetTarget(AFactionWarsCharacter* _target) { Target = _target; }

	UFUNCTION(BlueprintCallable)
	AFactionWarsCharacter* GetTarget() { return Target; }

	UFUNCTION(BlueprintCallable)
	bool GetHasBeenActivated() const { return HasBeenActivated; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FTimerHandle AbilityDestroyTimer;

	// Because we can't give function pointer if the function has arguments...
	UFUNCTION(BlueprintCallable)
	void MyDestroy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PlayAnimMontage_Casting() const;
	virtual void PlayAnimMontage_Cast();

	UFUNCTION()
	virtual void OnAnimMontageNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	virtual void SpawnAbilityEffects();
	
	UPROPERTY(Replicated)
	AFactionWarsCharacter* Caster = nullptr;
	UPROPERTY(Replicated)
	AFactionWarsCharacter* Target = nullptr;

	UPROPERTY()
	FTimerHandle TickEffectTimer;

private:
	void InitSphereCol();
	void InitParticleSys();

	void DestroyFroDelegate() { Destroy(); }

	UPROPERTY(Replicated)
	bool HasBeenActivated = false;
};
