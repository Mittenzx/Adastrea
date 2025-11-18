#include "Combat/ShieldVFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UShieldVFXComponent::UShieldVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Set default colors
	FullShieldColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.8f);     // Green
	RechargingColor = FLinearColor(0.0f, 0.5f, 1.0f, 0.8f);    // Blue
	ShieldDownColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.8f);    // Red

	// Set default pulse parameters
	RechargePulseSpeed = 2.0f;
	MinPulseIntensity = 0.3f;
	MaxPulseIntensity = 1.5f;

	// Initialize state tracking
	bWereAllShieldsDown = false;
	bWereShieldsFull = false;

	// Set component defaults
	ShieldBaseMaterial = nullptr;
	ShieldEffectTemplate = nullptr;
	CombatHealthComponent = nullptr;
}

void UShieldVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	// Auto-detect combat health component if not set
	if (!CombatHealthComponent)
	{
		CombatHealthComponent = GetOwner()->FindComponentByClass<UCombatHealthComponent>();
	}

	InitializeShieldVisuals();
}

void UShieldVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateShieldVisuals();
}

void UShieldVFXComponent::InitializeShieldVisuals()
{
	if (!ShieldBaseMaterial || !ShieldEffectTemplate)
	{
		return;
	}

	// Initialize material instances and effect components for each facing
	// This would be expanded with actual component creation in a full implementation
	// For now, we set up the data structures
	
	ForwardFacing.State = EShieldVisualState::Full;
	AftFacing.State = EShieldVisualState::Full;
	PortFacing.State = EShieldVisualState::Full;
	StarboardFacing.State = EShieldVisualState::Full;
	OmnidirectionalShield.State = EShieldVisualState::Full;
}

void UShieldVFXComponent::UpdateShieldVisuals()
{
	if (!CombatHealthComponent)
	{
		return;
	}

	bool bUseDirectional = CombatHealthComponent->bUseDirectionalShields;
	
	if (bUseDirectional)
	{
		// Update each facing independently
		FShieldFacingData ForwardData = CombatHealthComponent->GetShieldFacingData(EShieldFacing::Forward);
		FShieldFacingData AftData = CombatHealthComponent->GetShieldFacingData(EShieldFacing::Aft);
		FShieldFacingData PortData = CombatHealthComponent->GetShieldFacingData(EShieldFacing::Port);
		FShieldFacingData StarboardData = CombatHealthComponent->GetShieldFacingData(EShieldFacing::Starboard);

		float DeltaTime = GetWorld()->GetDeltaSeconds();

		UpdateFacingVisual(ForwardFacing, EShieldFacing::Forward, ForwardData.CurrentStrength / FMath::Max(ForwardData.MaxStrength, 1.0f), 
			ForwardData.RechargeDelay > 0.0f, DeltaTime);
		UpdateFacingVisual(AftFacing, EShieldFacing::Aft, AftData.CurrentStrength / FMath::Max(AftData.MaxStrength, 1.0f), 
			AftData.RechargeDelay > 0.0f, DeltaTime);
		UpdateFacingVisual(PortFacing, EShieldFacing::Port, PortData.CurrentStrength / FMath::Max(PortData.MaxStrength, 1.0f), 
			PortData.RechargeDelay > 0.0f, DeltaTime);
		UpdateFacingVisual(StarboardFacing, EShieldFacing::Starboard, StarboardData.CurrentStrength / FMath::Max(StarboardData.MaxStrength, 1.0f), 
			StarboardData.RechargeDelay > 0.0f, DeltaTime);
	}
	else
	{
		// Update omnidirectional shield
		float ShieldPercentage = CombatHealthComponent->GetShieldPercentage();
		bool bIsRecharging = CombatHealthComponent->IsRecharging();
		float DeltaTime = GetWorld()->GetDeltaSeconds();

		UpdateFacingVisual(OmnidirectionalShield, EShieldFacing::Omnidirectional, ShieldPercentage, bIsRecharging, DeltaTime);
	}

	// Check for state transitions
	bool bAllShieldsDown = CombatHealthComponent->GetShieldPercentage() <= 0.0f;
	bool bShieldsFull = CombatHealthComponent->GetShieldPercentage() >= 1.0f;

	if (bAllShieldsDown && !bWereAllShieldsDown)
	{
		OnAllShieldsDown();
	}

	if (bShieldsFull && !bWereShieldsFull)
	{
		OnShieldsFullyRecharged();
	}

	bWereAllShieldsDown = bAllShieldsDown;
	bWereShieldsFull = bShieldsFull;
}

void UShieldVFXComponent::UpdateFacingVisual(FShieldFacingVisual& FacingVisual, EShieldFacing Facing, float ShieldStrength, bool IsRecharging, float DeltaTime)
{
	// Determine new state
	EShieldVisualState OldState = FacingVisual.State;
	EShieldVisualState NewState;

	if (ShieldStrength <= 0.0f)
	{
		NewState = EShieldVisualState::Down;
	}
	else if (IsRecharging)
	{
		NewState = EShieldVisualState::Recharging;
	}
	else if (ShieldStrength >= 1.0f)
	{
		NewState = EShieldVisualState::Full;
	}
	else
	{
		NewState = EShieldVisualState::Recharging;
	}

	// Update state and trigger event if changed
	if (OldState != NewState)
	{
		FacingVisual.State = NewState;
		OnShieldStateChanged(Facing, OldState, NewState);
	}

	// Update pulse animation
	if (NewState == EShieldVisualState::Recharging)
	{
		FacingVisual.PulseTime += DeltaTime * RechargePulseSpeed;
		if (FacingVisual.PulseTime > 1.0f)
		{
			FacingVisual.PulseTime -= 1.0f;
		}
	}
	else
	{
		FacingVisual.PulseTime = 0.0f;
	}

	// Update glow intensity
	FLinearColor CurrentColor = GetColorForState(NewState, FacingVisual.PulseTime);
	FacingVisual.GlowIntensity = CurrentColor.A;

	// Apply to material instance
	if (FacingVisual.ShieldMaterial)
	{
		FacingVisual.ShieldMaterial->SetVectorParameterValue(FName("ShieldColor"), CurrentColor);
		FacingVisual.ShieldMaterial->SetScalarParameterValue(FName("Intensity"), FacingVisual.GlowIntensity);
	}

	// Update Niagara effect
	if (FacingVisual.EffectComponent)
	{
		FacingVisual.EffectComponent->SetColorParameter(FName("ShieldColor"), CurrentColor);
		FacingVisual.EffectComponent->SetFloatParameter(FName("Intensity"), FacingVisual.GlowIntensity);
	}
}

FLinearColor UShieldVFXComponent::GetColorForState(EShieldVisualState State, float PulseTime) const
{
	switch (State)
	{
	case EShieldVisualState::Full:
		return FullShieldColor;

	case EShieldVisualState::Recharging:
		{
			// Pulse between min and max intensity
			float PulseAlpha = FMath::Sin(PulseTime * PI * 2.0f) * 0.5f + 0.5f;
			float Intensity = FMath::Lerp(MinPulseIntensity, MaxPulseIntensity, PulseAlpha);
			FLinearColor Color = RechargingColor;
			Color.A *= Intensity;
			return Color;
		}

	case EShieldVisualState::Down:
		return ShieldDownColor;

	case EShieldVisualState::Disabled:
	default:
		return FLinearColor::Transparent;
	}
}

void UShieldVFXComponent::SetFacingState(EShieldFacing Facing, EShieldVisualState NewState)
{
	FShieldFacingVisual& FacingVisual = GetFacingVisual(Facing);
	EShieldVisualState OldState = FacingVisual.State;
	
	if (OldState != NewState)
	{
		FacingVisual.State = NewState;
		OnShieldStateChanged(Facing, OldState, NewState);
	}
}

EShieldVisualState UShieldVFXComponent::GetFacingState(EShieldFacing Facing) const
{
	return GetFacingVisual(Facing).State;
}

void UShieldVFXComponent::PlayShieldImpact(FVector ImpactLocation, FVector ImpactNormal, EShieldFacing Facing)
{
	if (!ShieldEffectTemplate)
	{
		return;
	}

	// Spawn impact effect at location
	UNiagaraComponent* ImpactEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ShieldEffectTemplate,
		ImpactLocation,
		ImpactNormal.Rotation()
	);

	if (ImpactEffect)
	{
		// Set parameters based on facing
		FLinearColor ImpactColor = GetColorForState(GetFacingState(Facing), 0.0f);
		ImpactEffect->SetColorParameter(FName("ImpactColor"), ImpactColor);
	}
}

FShieldFacingVisual& UShieldVFXComponent::GetFacingVisual(EShieldFacing Facing)
{
	switch (Facing)
	{
	case EShieldFacing::Forward:
		return ForwardFacing;
	case EShieldFacing::Aft:
		return AftFacing;
	case EShieldFacing::Port:
		return PortFacing;
	case EShieldFacing::Starboard:
		return StarboardFacing;
	case EShieldFacing::Omnidirectional:
	default:
		return OmnidirectionalShield;
	}
}

const FShieldFacingVisual& UShieldVFXComponent::GetFacingVisual(EShieldFacing Facing) const
{
	switch (Facing)
	{
	case EShieldFacing::Forward:
		return ForwardFacing;
	case EShieldFacing::Aft:
		return AftFacing;
	case EShieldFacing::Port:
		return PortFacing;
	case EShieldFacing::Starboard:
		return StarboardFacing;
	case EShieldFacing::Omnidirectional:
	default:
		return OmnidirectionalShield;
	}
}

void UShieldVFXComponent::OnShieldStateChanged_Implementation(EShieldFacing Facing, EShieldVisualState OldState, EShieldVisualState NewState)
{
	// Default implementation - can be overridden in Blueprint
}

void UShieldVFXComponent::OnShieldsFullyRecharged_Implementation()
{
	// Default implementation - can be overridden in Blueprint
}

void UShieldVFXComponent::OnAllShieldsDown_Implementation()
{
	// Default implementation - can be overridden in Blueprint
}
