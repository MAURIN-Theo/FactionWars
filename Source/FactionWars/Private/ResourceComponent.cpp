// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UResourceComponent::UResourceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


bool UResourceComponent::CanConsumeResource(float _cost)
{
	if (NeedPerfectConsumption && _cost > Current)
	{
		return false;
	}
	return true;
}

void UResourceComponent::ModifyResource(float _value)
{
	if (_value == 0.0f)
	{
		return;
	}

	if (_value < 0.0f && CanConsumeResource(-_value))
	{
		Current += _value;
		OnLostResource.Broadcast();
	}
	else if (_value > 0.0f)
	{
		Current += _value;
		OnGainedResource.Broadcast();
	}

	Current = FMath::Clamp(Current, Min, Max);

	OnRequestUpdateResource_NetMulticast();
	
	if (Current >= Max)
	{
		OnFullResource.Broadcast();
	}
	else if (Current <= Min)
	{
		OnEmptyResource.Broadcast();
	}
}

void UResourceComponent::SetValueFromNormalized(float _valueNormalized)
{
	Current = Max * _valueNormalized;
	OnRequestUpdateResource_NetMulticast();
}

void UResourceComponent::BeginRegenResource()
{
	GetWorld()->GetTimerManager().SetTimer(RegenTimerHandle, this, &UResourceComponent::RegenResource, RegenTickRate, true);
	IsRegenerating = true;
}

void UResourceComponent::StopRegenResource()
{
	if (RegenTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
	}
	
	IsRegenerating = false;
}

void UResourceComponent::RegenResource()
{
	ModifyResource(RegenStrength);
}

void UResourceComponent::OnRequestUpdateResource_NetMulticast_Implementation()
{
	OnRequestUpdateResource.Broadcast();
}

// Called when the game starts
void UResourceComponent::BeginPlay()
{
	Super::BeginPlay();

	Current = Max;
	OnRequestUpdateResource_NetMulticast();
}


// Called every frame
void UResourceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UResourceComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(UResourceComponent, Max);
	DOREPLIFETIME(UResourceComponent, Min);
	DOREPLIFETIME(UResourceComponent, Current);
	DOREPLIFETIME(UResourceComponent, RegenStrength);
	DOREPLIFETIME(UResourceComponent, RegenTickRate);
	DOREPLIFETIME(UResourceComponent, NeedPerfectConsumption);
	DOREPLIFETIME(UResourceComponent, IsRegenerating);
}

