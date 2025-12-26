// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Ships/EngineModuleComponent.h"
#include "Ships/EngineModuleDataAsset.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UEngineModuleComponent::UEngineModuleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	EngineData = nullptr;
	CurrentThrottle = 0.0f;
	CurrentHeat = 0.0f;
	bIsOverheated = false;
	bBoostActive = false;
	bTravelModeActive = false;

	EngineTrailComponent = nullptr;
	BoostEffectComponent = nullptr;
	EngineSoundComponent = nullptr;
}

void UEngineModuleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cast ModuleData to EngineData for convenience
	if (ModuleData)
	{
		EngineData = Cast<UEngineModuleDataAsset>(ModuleData);
	}
}

void UEngineModuleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupEffectComponents();
	Super::EndPlay(EndPlayReason);
}

void UEngineModuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsEnabled && bIsCurrentlyInstalled && EngineData)
	{
		UpdateHeat(DeltaTime);
		UpdateEffects();
	}
}

// ====================
// IShipModule Interface Overrides
// ====================

void UEngineModuleComponent::OnModuleInstalled_Implementation(const FShipModuleSlot& Slot)
{
	Super::OnModuleInstalled_Implementation(Slot);

	InitializeEffectComponents();
}

void UEngineModuleComponent::OnModuleRemoved_Implementation(const FShipModuleSlot& Slot)
{
	CleanupEffectComponents();

	Super::OnModuleRemoved_Implementation(Slot);
}

// ====================
// ENGINE OPERATIONS
// ====================

void UEngineModuleComponent::SetThrottle(float ThrottlePercent)
{
	CurrentThrottle = FMath::Clamp(ThrottlePercent, 0.0f, 100.0f);
}

bool UEngineModuleComponent::ActivateBoost()
{
	if (!CanActivateBoost())
	{
		return false;
	}

	bBoostActive = true;
	OnBoostActivated();
	return true;
}

void UEngineModuleComponent::DeactivateBoost()
{
	if (bBoostActive)
	{
		bBoostActive = false;
		OnBoostDeactivated();
	}
}

bool UEngineModuleComponent::ActivateTravelMode()
{
	if (!CanActivateTravelMode())
	{
		return false;
	}

	bTravelModeActive = true;
	return true;
}

void UEngineModuleComponent::DeactivateTravelMode()
{
	bTravelModeActive = false;
}

float UEngineModuleComponent::GetCurrentThrust() const
{
	if (!EngineData || !bIsEnabled || bIsOverheated)
	{
		return 0.0f;
	}

	return EngineData->GetEffectiveThrust(CurrentThrottle, bBoostActive);
}

float UEngineModuleComponent::GetCurrentMaxSpeed() const
{
	if (!EngineData || !bIsEnabled || bIsOverheated)
	{
		return 0.0f;
	}

	return EngineData->GetEffectiveMaxSpeed(bBoostActive, bTravelModeActive);
}

float UEngineModuleComponent::GetCurrentFuelConsumption() const
{
	if (!EngineData || !bIsEnabled || bIsOverheated)
	{
		return 0.0f;
	}

	return EngineData->GetCurrentFuelConsumption(CurrentThrottle, bBoostActive);
}

float UEngineModuleComponent::GetHeatPercentage() const
{
	if (!EngineData || EngineData->MaxHeat <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentHeat / EngineData->MaxHeat;
}

bool UEngineModuleComponent::CanActivateBoost() const
{
	return EngineData && bIsEnabled && !bIsOverheated && !bBoostActive;
}

bool UEngineModuleComponent::CanActivateTravelMode() const
{
	return EngineData && EngineData->bSupportsTravelMode && bIsEnabled && !bIsOverheated && !bTravelModeActive;
}

// ====================
// BLUEPRINT NATIVE EVENTS
// ====================

void UEngineModuleComponent::OnModuleEnabled_Implementation()
{
	Super::OnModuleEnabled_Implementation();

	InitializeEffectComponents();
}

void UEngineModuleComponent::OnModuleDisabled_Implementation()
{
	Super::OnModuleDisabled_Implementation();

	// Deactivate special modes
	DeactivateBoost();
	DeactivateTravelMode();
	CurrentThrottle = 0.0f;

	CleanupEffectComponents();
}

void UEngineModuleComponent::UpdateModule_Implementation(float DeltaTime)
{
	Super::UpdateModule_Implementation(DeltaTime);

	// Module-specific updates are handled in TickComponent
}

void UEngineModuleComponent::OnEngineOverheated_Implementation()
{
	// Override in Blueprint for overheat effects
}

void UEngineModuleComponent::OnEngineCooledDown_Implementation()
{
	// Override in Blueprint for cooldown effects
}

void UEngineModuleComponent::OnBoostActivated_Implementation()
{
	// Override in Blueprint for boost activation effects
	if (BoostEffectComponent)
	{
		BoostEffectComponent->Activate();
	}
}

void UEngineModuleComponent::OnBoostDeactivated_Implementation()
{
	// Override in Blueprint for boost deactivation effects
	if (BoostEffectComponent)
	{
		BoostEffectComponent->Deactivate();
	}
}

// ====================
// PRIVATE HELPERS
// ====================

void UEngineModuleComponent::UpdateHeat(float DeltaTime)
{
	if (!EngineData)
	{
		return;
	}

	bool bWasOverheated = bIsOverheated;

	// Generate heat based on throttle
	float HeatGenerated = EngineData->HeatGeneration * (CurrentThrottle / 100.0f) * DeltaTime;
	
	if (bBoostActive)
	{
		HeatGenerated *= EngineData->BoostHeatMultiplier;
	}

	CurrentHeat += HeatGenerated;

	// Apply cooling
	CurrentHeat -= EngineData->CoolingRate * DeltaTime;
	
	// Clamp heat with configurable overrun
	float MaxHeatWithOverrun = EngineData->MaxHeat * (1.0f + EngineData->MaxHeatOverrunPercent / 100.0f);
	CurrentHeat = FMath::Clamp(CurrentHeat, 0.0f, MaxHeatWithOverrun);

	// Check overheat status
	bIsOverheated = CurrentHeat >= EngineData->MaxHeat;

	// Trigger events
	if (bIsOverheated && !bWasOverheated)
	{
		OnEngineOverheated();
		
		// Auto-deactivate boost if overheated
		if (bBoostActive)
		{
			DeactivateBoost();
		}
	}
	else if (!bIsOverheated && bWasOverheated)
	{
		OnEngineCooledDown();
	}
}

void UEngineModuleComponent::UpdateEffects()
{
	if (!EngineData)
	{
		return;
	}

	// Update trail effect intensity based on throttle
	if (EngineTrailComponent && EngineTrailComponent->IsActive())
	{
		float Intensity = CurrentThrottle / 100.0f;
		EngineTrailComponent->SetFloatParameter(FName("Intensity"), Intensity);
	}

	// Update engine sound pitch/volume based on throttle
	if (EngineSoundComponent && EngineSoundComponent->IsPlaying())
	{
		float ThrottleNormalized = CurrentThrottle / 100.0f;
		
		// Interpolate pitch based on data asset settings
		float PitchMultiplier = FMath::Lerp(EngineData->EngineSoundMinPitch, EngineData->EngineSoundMaxPitch, ThrottleNormalized);
		EngineSoundComponent->SetPitchMultiplier(PitchMultiplier);
		
		// Interpolate volume based on data asset settings
		float VolumeMultiplier = FMath::Lerp(EngineData->EngineSoundMinVolume, EngineData->EngineSoundMaxVolume, ThrottleNormalized);
		EngineSoundComponent->SetVolumeMultiplier(VolumeMultiplier);
	}
}

void UEngineModuleComponent::InitializeEffectComponents()
{
	if (!EngineData || !GetOwner())
	{
		return;
	}

	AActor* Owner = GetOwner();

	// Create engine trail effect
	if (EngineData->EngineTrailEffect && !EngineTrailComponent)
	{
		EngineTrailComponent = NewObject<UNiagaraComponent>(Owner, UNiagaraComponent::StaticClass());
		if (EngineTrailComponent)
		{
			EngineTrailComponent->RegisterComponent();
			EngineTrailComponent->AttachToComponent(ModuleMeshComponent ? ModuleMeshComponent : Owner->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform);
			Owner->AddInstanceComponent(EngineTrailComponent);
			EngineTrailComponent->SetAsset(EngineData->EngineTrailEffect);
			EngineTrailComponent->Activate();
		}
	}

	// Create boost effect
	if (EngineData->BoostEffect && !BoostEffectComponent)
	{
		BoostEffectComponent = NewObject<UNiagaraComponent>(Owner, UNiagaraComponent::StaticClass());
		if (BoostEffectComponent)
		{
			BoostEffectComponent->RegisterComponent();
			BoostEffectComponent->AttachToComponent(ModuleMeshComponent ? ModuleMeshComponent : Owner->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform);
			Owner->AddInstanceComponent(BoostEffectComponent);
			BoostEffectComponent->SetAsset(EngineData->BoostEffect);
			BoostEffectComponent->Deactivate(); // Start inactive
		}
	}

	// Create engine sound
	if (EngineData->EngineSound && !EngineSoundComponent)
	{
		EngineSoundComponent = NewObject<UAudioComponent>(Owner, UAudioComponent::StaticClass());
		if (EngineSoundComponent)
		{
			EngineSoundComponent->RegisterComponent();
			EngineSoundComponent->AttachToComponent(ModuleMeshComponent ? ModuleMeshComponent : Owner->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform);
			Owner->AddInstanceComponent(EngineSoundComponent);
			EngineSoundComponent->SetSound(EngineData->EngineSound);
			EngineSoundComponent->Play();
		}
	}
}

void UEngineModuleComponent::CleanupEffectComponents()
{
	if (EngineTrailComponent)
	{
		EngineTrailComponent->DestroyComponent();
		EngineTrailComponent = nullptr;
	}

	if (BoostEffectComponent)
	{
		BoostEffectComponent->DestroyComponent();
		BoostEffectComponent = nullptr;
	}

	if (EngineSoundComponent)
	{
		EngineSoundComponent->Stop();
		EngineSoundComponent->DestroyComponent();
		EngineSoundComponent = nullptr;
	}
}
