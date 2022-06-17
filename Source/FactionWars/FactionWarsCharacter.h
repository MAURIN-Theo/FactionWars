// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "FactionWarsCharacter.generated.h"

#define HAND_SOCKET "hand_rSocket"
#define MIN_CASTING_SPEED 0.25f

//Forward declaration
class UResourceComponent;
class AAbility;
class AAbilityEffect;
class UActiveAbilityContainer;
class USphereComponent;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInterrupt);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRequestUpdateUI);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetChanged);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectActivated, const AAbilityEffect*, _status);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilitySpawned, const AAbility*, _ability);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTryPlayingMontage, FString, _montageName, float, InPlayRate, FName,
                                               StartSectionName);

UCLASS(config=Game)
class AFactionWarsCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AFactionWarsCharacter();

	virtual void Tick(float DeltaSeconds) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnTryPlayingMontage OnTryPlayingMontage;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnInterrupt OnInterrupt;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnRequestUpdateUI OnRequestUpdateUI;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnTargetChanged OnTargetChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnStatusEffectActivated OnStatusEffectActivated;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnAbilitySpawned OnAbilitySpawned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int InitialTeamID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	bool IsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool IsCasting = false;

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
	void AddCastingSpeed(float _value);

	UFUNCTION(BlueprintCallable)
	float GetCastingSpeed() { return FMath::Max(MIN_CASTING_SPEED, CastingSpeed); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UActiveAbilityContainer*> AbilityContainersArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AFactionWarsCharacter*> PossibleHostileTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AFactionWarsCharacter*> PossibleAllyTargets;

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
	void SetTarget(AFactionWarsCharacter* _target);

	UFUNCTION(BlueprintCallable)
	AFactionWarsCharacter* GetTarget() { return Target; }

	UFUNCTION(BlueprintCallable)
	bool CheckTargetVisibility(AFactionWarsCharacter* _targetToTest = nullptr);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UResourceComponent* HealthRes = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UResourceComponent* ManaRes = nullptr;

	/** Assigns Team Agent to given TeamID */
	UFUNCTION(BlueprintCallable)
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;

	/** Retrieve team identifier in form of FGenericTeamId */
	UFUNCTION(BlueprintCallable)
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	UFUNCTION(BlueprintCallable)
	virtual ETeamAttitude::Type GetTeamAttitudeTowards_Character(const AActor* Other) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* PossibleTargetsSphereCol = nullptr;

	UFUNCTION(BlueprintCallable)
	virtual void SetCantMoveTimer();

	UFUNCTION(BlueprintCallable)
	virtual void ClearCantMoveTimer();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Target)
	AFactionWarsCharacter* Target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	float CastingSpeed = 1.0f;

	virtual void BeginPlay() override;

	static ETeamAttitude::Type CustomTeamAttitudeSolver(FGenericTeamId A, FGenericTeamId B);

	/** Called for forwards/backward input */
	UFUNCTION(BlueprintNativeEvent, Category = FactionWarsCharacter)
	void MoveForward(float Value);

	/** Called for side to side input */
	UFUNCTION(BlueprintNativeEvent, Category = FactionWarsCharacter)
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	UFUNCTION()
	virtual void EventOnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	                                  class AController* InstigatedBy, AActor* DamageCauser);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void PossessedBy(AController* NewController) override;
private:
	void InitAbilityContainers();
	void InitPossibleSphereCol();

	UFUNCTION()
	void SetIsDead() { IsDead = true; }

	UFUNCTION()
	void OnRep_Target();

	UFUNCTION()
	void CustomEventOnCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	UPROPERTY()
	FTimerHandle CantMoveTimer;

	UPROPERTY(VisibleAnywhere, Replicated)
	FGenericTeamId TeamId = 0;
};
