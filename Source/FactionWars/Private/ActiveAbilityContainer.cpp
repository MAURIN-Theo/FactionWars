// Fill out your copyright notice in the Description page of Project Settings.


#include "ActiveAbilityContainer.h"
#include "Ability.h"
#include "ResourceComponent.h"
#include "FactionWars/FactionWarsCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UActiveAbilityContainer::UActiveAbilityContainer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UActiveAbilityContainer::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerChara = Cast<AFactionWarsCharacter>(GetOwner());
	
	if (nullptr == OwnerChara)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("No owner in BeginPlay of UActiveAbilityContainer"));
		}
	}
	else if (nullptr != AbilityContained && AbilityContained.GetDefaultObject()->AbilityDetails.IsInterruptible)
	{
		OnBeginCasting.AddUniqueDynamic(OwnerChara, &AFactionWarsCharacter::SetCantMoveTimer);
	}

	OnAbilityCasted.AddDynamic(this, &UActiveAbilityContainer::BeginCooldown);
	OnAbilityCasted.AddUniqueDynamic(OwnerChara, &AFactionWarsCharacter::ClearCantMoveTimer);
}

// Called every frame
void UActiveAbilityContainer::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UActiveAbilityContainer::PreCheckCanBeActivated() const
{
	if (nullptr == AbilityContained || nullptr == OwnerChara || nullptr == OwnerChara->ManaRes)
	{
		return false;
	}

	if (OwnerChara->IsDead == true)
	{
		return false;
	}
	
	if (IsInCooldown == true || OwnerChara->IsCasting == true)
	{
		return false;
	}

	if (OwnerChara->ManaRes->CanConsumeResource(AbilityContained.GetDefaultObject()->AbilityDetails.Cost) == false)
	{
		return false;
	}

	return true;
}

bool UActiveAbilityContainer::CanAbilityBeCasted(AFactionWarsCharacter* _targetToTest)
{
	if (PreCheckCanBeActivated() == false)
	{
		return false;
	}

	AFactionWarsCharacter* computedTarget = ComputeTarget_Ability(_targetToTest);

	if (nullptr != computedTarget)
	{
		if (CheckTargetDistance(computedTarget) == false)
		{
			return false;
		}

		if (OwnerChara->CheckTargetVisibility(computedTarget) == false)
		{
			return false;
		}

		return true;
	}

	return false;
}

AFactionWarsCharacter* UActiveAbilityContainer::ComputeTarget_Ability(AFactionWarsCharacter* _targetToCompute)
{
	if (nullptr == AbilityContained || nullptr == OwnerChara)
	{
		return nullptr;
	}

	ETeamAttitude::Type attitude = ETeamAttitude::Type::Neutral;
	AFactionWarsCharacter* target = _targetToCompute;

	if (nullptr == target)
	{
		target = OwnerChara->GetTarget();
	}

	if (nullptr != target)
	{
		if (target->GetGenericTeamId() != FGenericTeamId::NoTeam)
		{
			attitude = OwnerChara->GetTeamAttitudeTowards(*(target));
		}
	}

	switch (AbilityContained.GetDefaultObject()->AbilityDetails.TargetType)
	{
	case ETargetType::SELF_ONLY:
		{
			return OwnerChara;
		}
	case ETargetType::ALLY:
		{
			if (attitude == ETeamAttitude::Friendly && OwnerChara != target)
			{
				return target;
			}
			break;
		}
	case ETargetType::ENEMY:
		{
			if (attitude == ETeamAttitude::Hostile)
			{
				return target;
			}
			break;
		}
	case ETargetType::ALLY_AND_SELF:
		{
			if (attitude == ETeamAttitude::Friendly || OwnerChara == target)
			{
				if (nullptr == target)
				{
					return OwnerChara;
				}
				return target;
			}
			break;
		}
	case ETargetType::EVERYONE:
		{
			if (nullptr == target)
			{
				return OwnerChara;
			}
			return target;
		}
	default:
		{
			return nullptr;
		}
	}

	return nullptr;
}

bool UActiveAbilityContainer::CheckTargetDistance(AFactionWarsCharacter* _targetToTest)
{
	if (nullptr == OwnerChara || nullptr == _targetToTest)
	{
		return false;
	}

	const float distToTarget = FVector::Dist(OwnerChara->GetActorLocation(),
	                                         _targetToTest->GetActorLocation());

	const float AbilityMaxDist = AbilityContained.GetDefaultObject()->AbilityDetails.Distance;

	if (OwnerChara != _targetToTest && distToTarget > AbilityMaxDist)
	{
		return false;
	}

	return true;
}

void UActiveAbilityContainer::CallOnBeginCasting_NetMulticast_Implementation()
{
	OnBeginCasting.Broadcast();
}

void UActiveAbilityContainer::UseAbilityContained_Server_Implementation()
{
	if (CanAbilityBeCasted() == false || nullptr == OwnerChara)
	{
		return;
	}
	
	AAbility* ability = GetWorld()->SpawnActor<AAbility>(AbilityContained, OwnerChara->GetActorLocation(), FRotator());

	if (nullptr != ability)
	{
		ability->SetTarget(ComputeTarget_Ability());
		ability->Container = this;
		ability->BeginCasting_NetMulticast();
	}
}

bool UActiveAbilityContainer::UseAbilityContained_Server_Validate()
{
	return true;
}

void UActiveAbilityContainer::BeginCooldown()
{
	if (nullptr == OwnerChara)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Error BeginCooldown"));
		}
		return;
	}

	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow,
		                                 TEXT("Cooldown begins for" + AbilityContained.GetDefaultObject()->
			                                 AbilityDetails.Name));
	}*/

	OwnerChara->IsCasting = false;
	IsInCooldown = true;
	const float cooldownTime = AbilityContained.GetDefaultObject()->AbilityDetails.Cooldown;

	if (cooldownTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(AbilityCooldownTimer, this, &UActiveAbilityContainer::EndCooldown,
		                                       cooldownTime, false);
	}
	else
	{
		EndCooldown();
	}
}

void UActiveAbilityContainer::EndCooldown()
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow,
		                                 TEXT("Cooldown ends for" + AbilityContained.GetDefaultObject()->
			                                 AbilityDetails.Name));
	}*/

	IsInCooldown = false;

	GetWorld()->GetTimerManager().ClearTimer(AbilityCooldownTimer);
}

void UActiveAbilityContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(UActiveAbilityContainer, IsInCooldown);
}