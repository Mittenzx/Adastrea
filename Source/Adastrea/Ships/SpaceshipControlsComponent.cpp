#include "Ships/SpaceshipControlsComponent.h"
#include "Combat/WeaponComponent.h"
#include "AdastreaLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

USpaceshipControlsComponent::USpaceshipControlsComponent()
	: MovementSpeed(1.0f)
	, LookSensitivity(1.0f)
	, bInvertLookY(false)
	, InputMappingPriority(0)
	, CurrentSpeed(1.0f)
	, SpeedStep(0.25f)
	, MinSpeed(0.25f)
	, MaxSpeed(3.0f)
	, MoveAction(nullptr)
	, LookAction(nullptr)
	, FireAction(nullptr)
	, SpeedAction(nullptr)
	, SpaceshipMappingContext(nullptr)
	, bControlsEnabled(false)
	, CachedWeaponComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USpaceshipControlsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create input actions and mapping context
	CreateInputActions();
	CreateInputMappingContext();

	// Cache weapon component reference
	CachedWeaponComponent = GetWeaponComponent();

	// Get the owning pawn
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Owner is not a Pawn, input will not be bound"));
		return;
	}

	// Bind to pawn's input component setup
	if (OwningPawn->InputComponent)
	{
		UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(OwningPawn->InputComponent);
		if (EnhancedInputComp)
		{
			SetupInputBindings(EnhancedInputComp);
		}
		else
		{
			UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: InputComponent is not an EnhancedInputComponent, input bindings will not be set up"));
		}
	}
	else
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Pawn has no InputComponent, input bindings will not be set up"));
	}

	// Enable controls by default
	EnableControls();

	UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Initialized on %s"), *GetOwner()->GetName());
}

void USpaceshipControlsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Disable controls to clean up input mapping context
	DisableControls();

	Super::EndPlay(EndPlayReason);
}

void USpaceshipControlsComponent::CreateInputActions()
{
	// Create Move Action (2D Axis for WASD)
	MoveAction = NewObject<UInputAction>(this, TEXT("IA_SpaceshipMove"));
	if (MoveAction)
	{
		MoveAction->ValueType = EInputActionValueType::Axis2D;
	}

	// Create Look Action (2D Axis for Mouse)
	LookAction = NewObject<UInputAction>(this, TEXT("IA_SpaceshipLook"));
	if (LookAction)
	{
		LookAction->ValueType = EInputActionValueType::Axis2D;
	}

	// Create Fire Action (Digital for LMB)
	FireAction = NewObject<UInputAction>(this, TEXT("IA_SpaceshipFire"));
	if (FireAction)
	{
		FireAction->ValueType = EInputActionValueType::Boolean;
	}

	// Create Speed Action (1D Axis for Mouse Wheel)
	SpeedAction = NewObject<UInputAction>(this, TEXT("IA_SpaceshipSpeed"));
	if (SpeedAction)
	{
		SpeedAction->ValueType = EInputActionValueType::Axis1D;
	}

	UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Created input actions"));
}

void USpaceshipControlsComponent::CreateInputMappingContext()
{
	SpaceshipMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_SpaceshipControls"));
	if (!SpaceshipMappingContext)
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("SpaceshipControlsComponent: Failed to create Input Mapping Context"));
		return;
	}

	// Map W key (Up strafe - positive Y)
	if (MoveAction)
	{
		FEnhancedActionKeyMapping& WMapping = SpaceshipMappingContext->MapKey(MoveAction, EKeys::W);
		UInputModifierSwizzleAxis* WSwizzle = NewObject<UInputModifierSwizzleAxis>(SpaceshipMappingContext);
		WSwizzle->Order = EInputAxisSwizzle::YXZ;
		WMapping.Modifiers.Add(WSwizzle);

		// Map S key (Down strafe - negative Y)
		FEnhancedActionKeyMapping& SMapping = SpaceshipMappingContext->MapKey(MoveAction, EKeys::S);
		UInputModifierSwizzleAxis* SSwizzle = NewObject<UInputModifierSwizzleAxis>(SpaceshipMappingContext);
		SSwizzle->Order = EInputAxisSwizzle::YXZ;
		SMapping.Modifiers.Add(SSwizzle);
		UInputModifierNegate* SNegate = NewObject<UInputModifierNegate>(SpaceshipMappingContext);
		SMapping.Modifiers.Add(SNegate);

		// Map D key (Right strafe - positive X)
		SpaceshipMappingContext->MapKey(MoveAction, EKeys::D);

		// Map A key (Left strafe - negative X)
		FEnhancedActionKeyMapping& AMapping = SpaceshipMappingContext->MapKey(MoveAction, EKeys::A);
		UInputModifierNegate* ANegate = NewObject<UInputModifierNegate>(SpaceshipMappingContext);
		AMapping.Modifiers.Add(ANegate);
	}

	// Map Mouse XY (Look)
	if (LookAction)
	{
		SpaceshipMappingContext->MapKey(LookAction, EKeys::Mouse2D);
	}

	// Map Left Mouse Button (Fire)
	if (FireAction)
	{
		SpaceshipMappingContext->MapKey(FireAction, EKeys::LeftMouseButton);
	}

	// Map Mouse Wheel (Speed adjustment)
	if (SpeedAction)
	{
		SpaceshipMappingContext->MapKey(SpeedAction, EKeys::MouseWheelAxis);
	}

	UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Created input mapping context with key bindings"));
}

void USpaceshipControlsComponent::SetupInputBindings(UEnhancedInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Invalid input component"));
		return;
	}

	// Bind Move action
	if (MoveAction)
	{
		PlayerInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &USpaceshipControlsComponent::HandleMove);
	}

	// Bind Look action
	if (LookAction)
	{
		PlayerInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &USpaceshipControlsComponent::HandleLook);
	}

	// Bind Fire action
	if (FireAction)
	{
		PlayerInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &USpaceshipControlsComponent::HandleFirePressed);
		PlayerInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &USpaceshipControlsComponent::HandleFireReleased);
	}

	// Bind Speed action
	if (SpeedAction)
	{
		PlayerInputComponent->BindAction(SpeedAction, ETriggerEvent::Triggered, this, &USpaceshipControlsComponent::HandleSpeed);
	}

	UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Input bindings set up"));
}

UEnhancedInputLocalPlayerSubsystem* USpaceshipControlsComponent::GetEnhancedInputSubsystem() const
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		return nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!PC)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

UWeaponComponent* USpaceshipControlsComponent::GetWeaponComponent() const
{
	if (!GetOwner())
	{
		return nullptr;
	}

	return GetOwner()->FindComponentByClass<UWeaponComponent>();
}

void USpaceshipControlsComponent::EnableControls()
{
	if (bControlsEnabled)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (Subsystem && SpaceshipMappingContext)
	{
		Subsystem->AddMappingContext(SpaceshipMappingContext, InputMappingPriority);
		bControlsEnabled = true;
		OnControlsEnabled.Broadcast();
		UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Controls enabled"));
	}
}

void USpaceshipControlsComponent::DisableControls()
{
	if (!bControlsEnabled)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (Subsystem && SpaceshipMappingContext)
	{
		Subsystem->RemoveMappingContext(SpaceshipMappingContext);
		bControlsEnabled = false;
		OnControlsDisabled.Broadcast();
		UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Controls disabled"));
	}
}

bool USpaceshipControlsComponent::AreControlsEnabled() const
{
	return bControlsEnabled;
}

void USpaceshipControlsComponent::SetMovementSpeed(float NewSpeed)
{
	MovementSpeed = FMath::Clamp(NewSpeed, 0.1f, 10.0f);
}

void USpaceshipControlsComponent::SetLookSensitivity(float NewSensitivity)
{
	LookSensitivity = FMath::Clamp(NewSensitivity, 0.1f, 10.0f);
}

void USpaceshipControlsComponent::ToggleInvertLookY()
{
	bInvertLookY = !bInvertLookY;
	UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Look Y invert set to %s"), bInvertLookY ? TEXT("true") : TEXT("false"));
}

float USpaceshipControlsComponent::GetCurrentSpeed() const
{
	return CurrentSpeed;
}

void USpaceshipControlsComponent::SetCurrentSpeed(float NewSpeed)
{
	float OldSpeed = CurrentSpeed;
	CurrentSpeed = FMath::Clamp(NewSpeed, MinSpeed, MaxSpeed);
	
	if (!FMath::IsNearlyEqual(OldSpeed, CurrentSpeed))
	{
		float Delta = CurrentSpeed - OldSpeed;
		OnSpeedChanged(CurrentSpeed, Delta);
		OnSpeedChangedEvent.Broadcast(CurrentSpeed, Delta);
		UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Speed set to %.2f"), CurrentSpeed);
	}
}

void USpaceshipControlsComponent::IncreaseSpeed()
{
	SetCurrentSpeed(CurrentSpeed + SpeedStep);
}

void USpaceshipControlsComponent::DecreaseSpeed()
{
	SetCurrentSpeed(CurrentSpeed - SpeedStep);
}

void USpaceshipControlsComponent::HandleMove(const FInputActionValue& Value)
{
	FVector2D MoveValue = Value.Get<FVector2D>() * MovementSpeed * CurrentSpeed;
	OnMoveInput(MoveValue);
}

void USpaceshipControlsComponent::HandleLook(const FInputActionValue& Value)
{
	FVector2D LookValue = Value.Get<FVector2D>() * LookSensitivity;
	
	// Apply Y axis inversion if enabled
	if (bInvertLookY)
	{
		LookValue.Y = -LookValue.Y;
	}

	OnLookInput(LookValue);
}

void USpaceshipControlsComponent::HandleFirePressed(const FInputActionValue& Value)
{
	OnFirePressed();
}

void USpaceshipControlsComponent::HandleFireReleased(const FInputActionValue& Value)
{
	OnFireReleased();
}

void USpaceshipControlsComponent::HandleSpeed(const FInputActionValue& Value)
{
	float ScrollValue = Value.Get<float>();
	
	// Early return for zero values to avoid unnecessary processing
	if (FMath::IsNearlyZero(ScrollValue))
	{
		return;
	}

	if (ScrollValue > 0.0f)
	{
		IncreaseSpeed();
	}
	else
	{
		DecreaseSpeed();
	}
}

void USpaceshipControlsComponent::OnMoveInput_Implementation(FVector2D MoveValue)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		return;
	}

	// Get pawn's right and up vectors for strafe movement
	// X component = horizontal strafe (A/D), Y component = vertical strafe (W/S)
	const FVector RightVector = OwningPawn->GetActorRightVector();
	const FVector UpVector = OwningPawn->GetActorUpVector();

	// Calculate movement direction
	// MoveValue.X = horizontal strafe (positive = right, negative = left)
	// MoveValue.Y = vertical strafe (positive = up, negative = down)
	const FVector MoveDirection = (RightVector * MoveValue.X) + (UpVector * MoveValue.Y);

	// Apply movement input
	OwningPawn->AddMovementInput(MoveDirection, 1.0f);
}

void USpaceshipControlsComponent::OnLookInput_Implementation(FVector2D LookValue)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		return;
	}

	// Apply rotation input to the controller
	// LookValue.X = yaw (left/right), LookValue.Y = pitch (up/down)
	OwningPawn->AddControllerYawInput(LookValue.X);
	OwningPawn->AddControllerPitchInput(LookValue.Y);
}

void USpaceshipControlsComponent::OnFirePressed_Implementation()
{
	// Try to fire weapon component
	if (!CachedWeaponComponent)
	{
		CachedWeaponComponent = GetWeaponComponent();
	}

	if (CachedWeaponComponent)
	{
		CachedWeaponComponent->Fire();
		UE_LOG(LogAdastreaInput, Verbose, TEXT("SpaceshipControlsComponent: Primary weapon fired"));
	}
}

void USpaceshipControlsComponent::OnFireReleased_Implementation()
{
	// Default implementation does nothing
	// Override in Blueprint for continuous fire weapons that need release handling
}

void USpaceshipControlsComponent::OnSpeedChanged_Implementation(float NewSpeed, float Delta)
{
	// Default implementation logs the speed change
	// Override in Blueprint for custom speed change effects (UI updates, engine sounds, etc.)
	UE_LOG(LogAdastreaInput, Verbose, TEXT("SpaceshipControlsComponent: Speed changed to %.2f (delta: %.2f)"), NewSpeed, Delta);
}
