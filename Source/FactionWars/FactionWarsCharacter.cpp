// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactionWarsCharacter.h"

#include "ActiveAbilityContainer.h"
#include "AIController_FW.h"
#include "DrawDebugHelpers.h"
#include "ResourceComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// AFactionWarsCharacter

AFactionWarsCharacter::AFactionWarsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	HealthRes = CreateDefaultSubobject<UResourceComponent>(TEXT("HealthResource"));
	ManaRes = CreateDefaultSubobject<UResourceComponent>(TEXT("ManaResource"));

	if (HealthRes != nullptr)
	{
		HealthRes->NeedPerfectConsumption = false;
		HealthRes->OnEmptyResource.AddDynamic(GetCharacterMovement(), &UCharacterMovementComponent::DisableMovement);
	}

	InitAbilityContainers();
	InitPossibleSphereCol();

	OnCharacterMovementUpdated.AddDynamic(this, &AFactionWarsCharacter::CustomEventOnCharacterMovementUpdated);
}

void AFactionWarsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PossibleTargetsSphereCol->IsCollisionEnabled() == false)
	{
		PossibleTargetsSphereCol->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFactionWarsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFactionWarsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFactionWarsCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFactionWarsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFactionWarsCharacter::LookUpAtRate);
}

void AFactionWarsCharacter::SetTarget_Implementation(AFactionWarsCharacter* _target)
{
	if (Target != _target)
	{
		Target = _target;
		OnTargetChanged.Broadcast();
	}
}

bool AFactionWarsCharacter::SetTarget_Validate(AFactionWarsCharacter* _target)
{
	return true;
}

void AFactionWarsCharacter::AddCastingSpeed_Implementation(float _value)
{
	CastingSpeed += _value;
}

bool AFactionWarsCharacter::AddCastingSpeed_Validate(float _value)
{
	return true;
}

bool AFactionWarsCharacter::CheckTargetVisibility(AFactionWarsCharacter* _targetToTest)
{
	AFactionWarsCharacter* target = _targetToTest;

	if (nullptr == target)
	{
		target = GetTarget();
	}

	if (nullptr != target && this != target)
	{
		TArray<FHitResult> Hits;
		FCollisionQueryParams CollisionParameters = FCollisionQueryParams();
		CollisionParameters.AddIgnoredActor(this);

		GetWorld()->LineTraceMultiByChannel(Hits, GetActorLocation(),
		                                    target->GetActorLocation(), ECC_Visibility,
		                                    CollisionParameters);

		DrawDebugLine(GetWorld(), GetActorLocation(), target->GetActorLocation(),
		              FColor::Green, false, 0.0f, 0, 1.f);

		for (const auto Hit : Hits)
		{
			if (Hit.GetActor()->IsRootComponentStatic())
			{
				return false;
			}
		}
	}
	return true;
}

void AFactionWarsCharacter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	this->TeamId = TeamID;
}

ETeamAttitude::Type AFactionWarsCharacter::GetTeamAttitudeTowards_Character(const AActor* Other) const
{
	if (nullptr != Other)
	{
		return IGenericTeamAgentInterface::GetTeamAttitudeTowards(*Other);
	}

	return ETeamAttitude::Neutral;
}

void AFactionWarsCharacter::SetCantMoveTimer()
{
	if (CantMoveTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CantMoveTimer);
	}

	GetCharacterMovement()->DisableMovement();
	GetWorld()->GetTimerManager().SetTimer(CantMoveTimer, this, &AFactionWarsCharacter::ClearCantMoveTimer, 0.5f);
}

void AFactionWarsCharacter::ClearCantMoveTimer()
{
	GetCharacterMovement()->SetDefaultMovementMode();
	if (CantMoveTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CantMoveTimer);
	}
}

void AFactionWarsCharacter::BeginPlay()
{
	Super::BeginPlay();

	FGenericTeamId::SetAttitudeSolver(&CustomTeamAttitudeSolver);

	AAIController_FW* aiController_Fw = Cast<AAIController_FW>(Controller);

	if (nullptr != aiController_Fw)
	{
		aiController_Fw->SetGenericTeamId(FGenericTeamId(InitialTeamID));
	}
	
	OnTakeAnyDamage.AddDynamic(this, &AFactionWarsCharacter::EventOnTakeAnyDamage);

	if (nullptr != HealthRes)
	{
		HealthRes->OnEmptyResource.AddDynamic(this, &AFactionWarsCharacter::SetIsDead);
	}
	
	if (nullptr != ManaRes)
	{
		ManaRes->BeginRegenResource();
	}
}

ETeamAttitude::Type AFactionWarsCharacter::CustomTeamAttitudeSolver(FGenericTeamId A, FGenericTeamId B)
{
	if (A == FGenericTeamId::NoTeam || B == FGenericTeamId::NoTeam)
	{
		return ETeamAttitude::Neutral;
	}

	return A != B ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
}

void AFactionWarsCharacter::MoveForward_Implementation(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AFactionWarsCharacter::MoveRight_Implementation(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AFactionWarsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFactionWarsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AFactionWarsCharacter::EventOnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                                 AController* InstigatedBy, AActor* DamageCauser)
{
	if (HealthRes != nullptr)
	{
		HealthRes->ModifyResource(-Damage);
	}
}

void AFactionWarsCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	APlayerController* PlayerController = Cast<APlayerController>(NewController);
	
	if (nullptr != PlayerController)
	{
		SetGenericTeamId(FGenericTeamId(InitialTeamID));
	}
}

void AFactionWarsCharacter::InitAbilityContainers()
{
	for (int i = 0; i < 8; ++i)
	{
		const FString name = FString::Printf(TEXT("Ability%i"), i);
		UActiveAbilityContainer* container = CreateDefaultSubobject<UActiveAbilityContainer>(ToCStr(name));
		AbilityContainersArray.Add(container);
	}
}

void AFactionWarsCharacter::InitPossibleSphereCol()
{
	PossibleTargetsSphereCol = CreateDefaultSubobject<USphereComponent>(TEXT("PossibleTargetsSphereCollision"));

	if (nullptr != PossibleTargetsSphereCol)
	{
		PossibleTargetsSphereCol->InitSphereRadius(2000);
		PossibleTargetsSphereCol->SetCollisionResponseToAllChannels(ECR_Ignore);
		PossibleTargetsSphereCol->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		PossibleTargetsSphereCol->SetupAttachment(RootComponent);
		PossibleTargetsSphereCol->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AFactionWarsCharacter::OnRep_Target()
{
	OnTargetChanged.Broadcast();
}

void AFactionWarsCharacter::CustomEventOnCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation,
                                                                  FVector OldVelocity)
{
	if (OldVelocity.Size() > 10.0f)
	{
		OnInterrupt.Broadcast();
	}
}

void AFactionWarsCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(AFactionWarsCharacter, TeamId);
	DOREPLIFETIME(AFactionWarsCharacter, Target);
	DOREPLIFETIME(AFactionWarsCharacter, IsCasting);
	DOREPLIFETIME(AFactionWarsCharacter, IsDead);
	DOREPLIFETIME(AFactionWarsCharacter, CastingSpeed);
}
