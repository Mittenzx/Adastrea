#include "Ships/SpaceshipParticleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Engine/World.h"

USpaceshipParticleComponent::USpaceshipParticleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Initialize default values
	CurrentThrottle = 0.0f;
	bBoostActive = false;
	MinParticleRateMultiplier = 0.2f;
	MaxParticleRateMultiplier = 3.0f;
	RCSThrusterDuration = 0.5f;
	JumpChargeDuration = 3.0f;
	bJumpSequenceActive = false;
	JumpSequenceTimer = 0.0f;
	JumpSequenceStage = 0;
	DamageState = EEngineDamageState::Normal;
	MaxParticlesPerShip = 10000;

	// Initialize component pointers to nullptr
	MainEngineComponent = nullptr;
	AfterburnerComponent = nullptr;
	JumpChargeComponent = nullptr;
	JumpActivationComponent = nullptr;
	JumpTunnelComponent = nullptr;
	JumpExitComponent = nullptr;
	DamageEffectComponent = nullptr;
	EngineMaterialInstance = nullptr;
}

void USpaceshipParticleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize particle systems
	InitializeParticleSystems();

	// Apply initial damage state
	ApplyDamageEffects();
}

void USpaceshipParticleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update engine glow material
	UpdateEngineGlow(DeltaTime);

	// Update jump sequence if active
	if (bJumpSequenceActive)
	{
		UpdateJumpSequence(DeltaTime);
	}
}

void USpaceshipParticleComponent::InitializeParticleSystems()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Create main engine particle component
	if (MainEngineTemplate)
	{
		MainEngineComponent = NewObject<UParticleSystemComponent>(Owner);
		if (MainEngineComponent)
		{
			MainEngineComponent->SetTemplate(MainEngineTemplate);
			MainEngineComponent->RegisterComponent();
			MainEngineComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			MainEngineComponent->SetRelativeLocation(FVector(-200.0f, 0.0f, 0.0f)); // Behind ship
			MainEngineComponent->Activate(true);
		}
	}

	// Create afterburner particle component
	if (AfterburnerTemplate)
	{
		AfterburnerComponent = NewObject<UParticleSystemComponent>(Owner);
		if (AfterburnerComponent)
		{
			AfterburnerComponent->SetTemplate(AfterburnerTemplate);
			AfterburnerComponent->RegisterComponent();
			AfterburnerComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			AfterburnerComponent->SetRelativeLocation(FVector(-250.0f, 0.0f, 0.0f)); // Behind main engine
			AfterburnerComponent->Deactivate();
		}
	}

	// Create RCS thruster components for each axis
	if (RCSThrusterTemplate)
	{
		// Forward RCS (front of ship)
		UParticleSystemComponent* ForwardRCS = NewObject<UParticleSystemComponent>(Owner);
		if (ForwardRCS)
		{
			ForwardRCS->SetTemplate(RCSThrusterTemplate);
			ForwardRCS->RegisterComponent();
			ForwardRCS->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			ForwardRCS->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));
			ForwardRCS->Deactivate();
			RCSThrusterComponents.Add(ERCSThrusterAxis::Forward, ForwardRCS);
		}

		// Backward RCS (rear of ship)
		UParticleSystemComponent* BackwardRCS = NewObject<UParticleSystemComponent>(Owner);
		if (BackwardRCS)
		{
			BackwardRCS->SetTemplate(RCSThrusterTemplate);
			BackwardRCS->RegisterComponent();
			BackwardRCS->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			BackwardRCS->SetRelativeLocation(FVector(-200.0f, 0.0f, 0.0f));
			BackwardRCS->Deactivate();
			RCSThrusterComponents.Add(ERCSThrusterAxis::Backward, BackwardRCS);
		}

		// Left RCS
		UParticleSystemComponent* LeftRCS = NewObject<UParticleSystemComponent>(Owner);
		if (LeftRCS)
		{
			LeftRCS->SetTemplate(RCSThrusterTemplate);
			LeftRCS->RegisterComponent();
			LeftRCS->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			LeftRCS->SetRelativeLocation(FVector(0.0f, -100.0f, 0.0f));
			LeftRCS->Deactivate();
			RCSThrusterComponents.Add(ERCSThrusterAxis::Left, LeftRCS);
		}

		// Right RCS
		UParticleSystemComponent* RightRCS = NewObject<UParticleSystemComponent>(Owner);
		if (RightRCS)
		{
			RightRCS->SetTemplate(RCSThrusterTemplate);
			RightRCS->RegisterComponent();
			RightRCS->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			RightRCS->SetRelativeLocation(FVector(0.0f, 100.0f, 0.0f));
			RightRCS->Deactivate();
			RCSThrusterComponents.Add(ERCSThrusterAxis::Right, RightRCS);
		}

		// Up RCS
		UParticleSystemComponent* UpRCS = NewObject<UParticleSystemComponent>(Owner);
		if (UpRCS)
		{
			UpRCS->SetTemplate(RCSThrusterTemplate);
			UpRCS->RegisterComponent();
			UpRCS->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			UpRCS->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
			UpRCS->Deactivate();
			RCSThrusterComponents.Add(ERCSThrusterAxis::Up, UpRCS);
		}

		// Down RCS
		UParticleSystemComponent* DownRCS = NewObject<UParticleSystemComponent>(Owner);
		if (DownRCS)
		{
			DownRCS->SetTemplate(RCSThrusterTemplate);
			DownRCS->RegisterComponent();
			DownRCS->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			DownRCS->SetRelativeLocation(FVector(0.0f, 0.0f, -100.0f));
			DownRCS->Deactivate();
			RCSThrusterComponents.Add(ERCSThrusterAxis::Down, DownRCS);
		}
	}

	// Create jump drive effect components
	if (JumpChargeTemplate)
	{
		JumpChargeComponent = NewObject<UParticleSystemComponent>(Owner);
		if (JumpChargeComponent)
		{
			JumpChargeComponent->SetTemplate(JumpChargeTemplate);
			JumpChargeComponent->RegisterComponent();
			JumpChargeComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			JumpChargeComponent->Deactivate();
		}
	}

	if (JumpActivationTemplate)
	{
		JumpActivationComponent = NewObject<UParticleSystemComponent>(Owner);
		if (JumpActivationComponent)
		{
			JumpActivationComponent->SetTemplate(JumpActivationTemplate);
			JumpActivationComponent->RegisterComponent();
			JumpActivationComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			JumpActivationComponent->Deactivate();
		}
	}

	if (JumpTunnelTemplate)
	{
		JumpTunnelComponent = NewObject<UParticleSystemComponent>(Owner);
		if (JumpTunnelComponent)
		{
			JumpTunnelComponent->SetTemplate(JumpTunnelTemplate);
			JumpTunnelComponent->RegisterComponent();
			JumpTunnelComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			JumpTunnelComponent->Deactivate();
		}
	}

	if (JumpExitTemplate)
	{
		JumpExitComponent = NewObject<UParticleSystemComponent>(Owner);
		if (JumpExitComponent)
		{
			JumpExitComponent->SetTemplate(JumpExitTemplate);
			JumpExitComponent->RegisterComponent();
			JumpExitComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			JumpExitComponent->Deactivate();
		}
	}

	// Create damage effect component
	DamageEffectComponent = NewObject<UParticleSystemComponent>(Owner);
	if (DamageEffectComponent)
	{
		DamageEffectComponent->RegisterComponent();
		DamageEffectComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageEffectComponent->SetRelativeLocation(FVector(-150.0f, 0.0f, 0.0f));
		DamageEffectComponent->Deactivate();
	}
}

void USpaceshipParticleComponent::UpdateThrottle(float Throttle)
{
	CurrentThrottle = FMath::Clamp(Throttle, 0.0f, 1.0f);

	// Update main engine particle spawn rate based on throttle
	if (MainEngineComponent && MainEngineComponent->IsActive())
	{
		float SpawnRateMultiplier = FMath::Lerp(MinParticleRateMultiplier, MaxParticleRateMultiplier, CurrentThrottle);
		MainEngineComponent->SetFloatParameter(FName("SpawnRate"), SpawnRateMultiplier);
	}
}

void USpaceshipParticleComponent::SetBoostActive(bool bActivate)
{
	bBoostActive = bActivate;

	if (AfterburnerComponent)
	{
		if (bBoostActive)
		{
			AfterburnerComponent->Activate(true);
		}
		else
		{
			AfterburnerComponent->Deactivate();
		}
	}
}

void USpaceshipParticleComponent::UpdateEngineGlow(float DeltaTime)
{
	// Update engine glow material parameter if available
	if (EngineMaterialInstance)
	{
		// Calculate glow intensity based on throttle and boost
		float GlowIntensity = CurrentThrottle;
		if (bBoostActive)
		{
			GlowIntensity = FMath::Min(GlowIntensity * 1.5f, 1.0f);
		}

		// Apply damage state modulation
		if (DamageState == EEngineDamageState::Damaged)
		{
			// Flickering effect for damaged state
			UWorld* World = GetWorld();
			if (World)
			{
				float FlickerAmount = FMath::Sin(World->GetTimeSeconds() * 10.0f) * 0.3f;
				GlowIntensity *= (0.7f + FlickerAmount);
			}
		}
		else if (DamageState == EEngineDamageState::Critical)
		{
			GlowIntensity *= 0.3f; // Very dim for critical damage
		}

		EngineMaterialInstance->SetScalarParameterValue(FName("EmissiveIntensity"), GlowIntensity);
	}
}

void USpaceshipParticleComponent::ActivateRCSThruster(ERCSThrusterAxis Axis, float Intensity)
{
	UParticleSystemComponent** ComponentPtr = RCSThrusterComponents.Find(Axis);
	if (ComponentPtr && *ComponentPtr)
	{
		UParticleSystemComponent* RCSComponent = *ComponentPtr;
		
		// Activate the thruster
		RCSComponent->SetFloatParameter(FName("Intensity"), FMath::Clamp(Intensity, 0.0f, 1.0f));
		RCSComponent->Activate(true);

		// Auto-deactivate after duration using weak pointer to safely handle component destruction
		UWorld* World = GetWorld();
		if (World)
		{
			FTimerHandle TimerHandle;
			TWeakObjectPtr<UParticleSystemComponent> WeakRCSComponent = RCSComponent;
			World->GetTimerManager().SetTimer(
				TimerHandle,
				[WeakRCSComponent]()
				{
					if (WeakRCSComponent.IsValid())
					{
						WeakRCSComponent->Deactivate();
					}
				},
				RCSThrusterDuration,
				false
			);
		}
	}
}

void USpaceshipParticleComponent::TriggerJumpSequence(float ChargeDuration)
{
	if (bJumpSequenceActive)
	{
		return; // Already in progress
	}

	bJumpSequenceActive = true;
	JumpSequenceStage = 0;
	JumpSequenceTimer = 0.0f;

	// Use provided duration or default
	if (ChargeDuration > 0.0f)
	{
		JumpChargeDuration = ChargeDuration;
	}

	// Start charge-up effect
	if (JumpChargeComponent)
	{
		JumpChargeComponent->Activate(true);
	}
}

void USpaceshipParticleComponent::UpdateJumpSequence(float DeltaTime)
{
	JumpSequenceTimer += DeltaTime;

	switch (JumpSequenceStage)
	{
		case 0: // Charging
			if (JumpSequenceTimer >= JumpChargeDuration)
			{
				// Move to activation stage
				JumpSequenceStage = 1;
				JumpSequenceTimer = 0.0f;

				// Deactivate charge, activate jump flash
				if (JumpChargeComponent)
				{
					JumpChargeComponent->Deactivate();
				}
				if (JumpActivationComponent)
				{
					JumpActivationComponent->Activate(true);
				}
			}
			break;

		case 1: // Activation flash (brief)
			if (JumpSequenceTimer >= 0.5f)
			{
				// Move to tunnel stage
				JumpSequenceStage = 2;
				JumpSequenceTimer = 0.0f;

				// Activate tunnel effect
				if (JumpTunnelComponent)
				{
					JumpTunnelComponent->Activate(true);
				}
			}
			break;

		case 2: // Jump tunnel (duration varies)
			if (JumpSequenceTimer >= 2.0f)
			{
				// Move to exit stage
				JumpSequenceStage = 3;
				JumpSequenceTimer = 0.0f;

				// Deactivate tunnel, activate exit flash
				if (JumpTunnelComponent)
				{
					JumpTunnelComponent->Deactivate();
				}
				if (JumpExitComponent)
				{
					JumpExitComponent->Activate(true);
				}
			}
			break;

		case 3: // Exit flash
			if (JumpSequenceTimer >= 0.5f)
			{
				// Complete sequence
				JumpSequenceStage = 4;
				JumpSequenceTimer = 0.0f;

				// Deactivate all jump effects
				if (JumpActivationComponent)
				{
					JumpActivationComponent->Deactivate();
				}
				if (JumpExitComponent)
				{
					JumpExitComponent->Deactivate();
				}

				bJumpSequenceActive = false;

				// Fire completion event
				OnJumpSequenceComplete();
			}
			break;

		default:
			break;
	}
}

void USpaceshipParticleComponent::SetDamageState(EEngineDamageState NewState)
{
	if (DamageState == NewState)
	{
		return; // No change
	}

	DamageState = NewState;
	ApplyDamageEffects();
}

void USpaceshipParticleComponent::ApplyDamageEffects()
{
	if (!DamageEffectComponent)
	{
		return;
	}

	// Deactivate any existing damage effect
	DamageEffectComponent->Deactivate();

	switch (DamageState)
	{
		case EEngineDamageState::Normal:
			// No damage effects
			break;

		case EEngineDamageState::Damaged:
			if (DamagedEngineTemplate)
			{
				DamageEffectComponent->SetTemplate(DamagedEngineTemplate);
				DamageEffectComponent->Activate(true);
			}
			break;

		case EEngineDamageState::Critical:
			if (CriticalDamageTemplate)
			{
				DamageEffectComponent->SetTemplate(CriticalDamageTemplate);
				DamageEffectComponent->Activate(true);
			}
			break;

		default:
			break;
	}
}

int32 USpaceshipParticleComponent::GetCurrentParticleCount() const
{
	int32 TotalCount = 0;

	// Count main engine particles
	if (MainEngineComponent && MainEngineComponent->IsActive())
	{
		TotalCount += MainEngineComponent->GetNumActiveParticles();
	}

	// Count afterburner particles
	if (AfterburnerComponent && AfterburnerComponent->IsActive())
	{
		TotalCount += AfterburnerComponent->GetNumActiveParticles();
	}

	// Count RCS particles
	for (const auto& Pair : RCSThrusterComponents)
	{
		if (Pair.Value && Pair.Value->IsActive())
		{
			TotalCount += Pair.Value->GetNumActiveParticles();
		}
	}

	// Count jump effect particles
	if (JumpChargeComponent && JumpChargeComponent->IsActive())
	{
		TotalCount += JumpChargeComponent->GetNumActiveParticles();
	}
	if (JumpActivationComponent && JumpActivationComponent->IsActive())
	{
		TotalCount += JumpActivationComponent->GetNumActiveParticles();
	}
	if (JumpTunnelComponent && JumpTunnelComponent->IsActive())
	{
		TotalCount += JumpTunnelComponent->GetNumActiveParticles();
	}
	if (JumpExitComponent && JumpExitComponent->IsActive())
	{
		TotalCount += JumpExitComponent->GetNumActiveParticles();
	}

	// Count damage effect particles
	if (DamageEffectComponent && DamageEffectComponent->IsActive())
	{
		TotalCount += DamageEffectComponent->GetNumActiveParticles();
	}

	return TotalCount;
}
