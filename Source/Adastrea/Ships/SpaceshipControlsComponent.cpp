#include "Ships/SpaceshipControlsComponent.h"
#include "Ships/Spaceship.h"
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
	, MovementSmoothingSpeed(10.0f)
	, bEnableMovementSmoothing(true)
	, LookSensitivity(50.0f)
	, RotationSmoothingSpeed(5.0f)
	, bEnableRotationSmoothing(true)
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
	, CurrentShipRotation(FRotator::ZeroRotator)
	, TargetShipRotation(FRotator::ZeroRotator)
	, CurrentMovementInput(FVector2D::ZeroVector)
	, TargetMovementInput(FVector2D::ZeroVector)
{
	PrimaryComponentTick.bCanEverTick = true;  // Enable ticking for rotation smoothing
	
	// Ensure component is active and BeginPlay is called
	bAutoActivate = true;
	bWantsInitializeComponent = true;

	// Load Input Action assets EARLY (in constructor) so they're available when SetupPlayerInputComponent is called
	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Constructor - Loading input assets..."));
	CreateInputActions();
	CreateInputMappingContext();
	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Constructor - Assets loaded. MoveAction=%s, SpaceshipMappingContext=%s"),
		MoveAction ? TEXT("Valid") : TEXT("NULL"),
		SpaceshipMappingContext ? TEXT("Valid") : TEXT("NULL"));
}

void USpaceshipControlsComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent::InitializeComponent() called on owner: %s"), 
		GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));

	// Assets are already loaded in constructor, just log status
	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: InitializeComponent - MoveAction=%s, SpaceshipMappingContext=%s"),
		MoveAction ? TEXT("Valid") : TEXT("NULL"),
		SpaceshipMappingContext ? TEXT("Valid") : TEXT("NULL"));
}

void USpaceshipControlsComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent::BeginPlay() called on owner: %s"), 
		*GetOwner()->GetName());

	// Cache weapon component reference
	CachedWeaponComponent = GetWeaponComponent();

	// Get the owning pawn
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Owner is not a Pawn, input will not be bound"));
		return;
	}

	// Initialize smoothing variables to current pawn state
	CurrentShipRotation = OwningPawn->GetActorRotation();
	TargetShipRotation = CurrentShipRotation;
	CurrentMovementInput = FVector2D::ZeroVector;
	TargetMovementInput = FVector2D::ZeroVector;

	// NOTE: InputComponent is set up AFTER BeginPlay in SetupPlayerInputComponent
	// We'll bind input there instead. For now, just log that we're ready.
	UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Initialized on %s, waiting for InputComponent setup"), *GetOwner()->GetName());
}

void USpaceshipControlsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Disable controls to clean up input mapping context
	DisableControls();

	Super::EndPlay(EndPlayReason);
}

void USpaceshipControlsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only tick if rotation smoothing is enabled
	if (!bEnableRotationSmoothing)
	{
		return;
	}

	// Get the owning pawn
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		return;
	}

	// Interpolate ship rotation toward target rotation
	CurrentShipRotation = FMath::RInterpTo(CurrentShipRotation, TargetShipRotation, DeltaTime, RotationSmoothingSpeed);

	// Apply the interpolated rotation to the pawn
	OwningPawn->SetActorRotation(CurrentShipRotation);
}

void USpaceshipControlsComponent::CreateInputActions()
{
	// Load existing Input Action assets from Content/Input/Actions/
	// These are the Blueprint assets configured in the project
	
	// Load Move Action (IA_Move.uasset)
	MoveAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	if (!MoveAction)
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("Failed to load IA_Move asset! Check Content/Input/Actions/IA_Move.uasset exists"));
	}

	// Load Look Action (IA_Look.uasset)
	LookAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	if (!LookAction)
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("Failed to load IA_Look asset! Check Content/Input/Actions/IA_Look.uasset exists"));
	}

	// Load Fire Action (IA_Fire_Primary.uasset)
	FireAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Fire_Primary.IA_Fire_Primary"));
	if (!FireAction)
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("Failed to load IA_Fire_Primary asset! Check Content/Input/Actions/IA_Fire_Primary.uasset exists"));
	}

	// Load Boost Action (IA_Boost.uasset) - using this for speed control
	SpeedAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Boost.IA_Boost"));
	if (!SpeedAction)
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("Failed to load IA_Boost asset! Check Content/Input/Actions/IA_Boost.uasset exists"));
	}

	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Loaded Input Action assets - MoveAction=%s, LookAction=%s"),
		MoveAction ? TEXT("Valid") : TEXT("NULL"),
		LookAction ? TEXT("Valid") : TEXT("NULL"));
}

void USpaceshipControlsComponent::CreateInputMappingContext()
{
	// Load the existing Input Mapping Context asset from Content/Input/
	SpaceshipMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Spaceship.IMC_Spaceship"));
	if (!SpaceshipMappingContext)
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("SpaceshipControlsComponent: Failed to load IMC_Spaceship asset! Check Content/Input/IMC_Spaceship.uasset exists"));
		return;
	}
	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Loaded IMC_Spaceship mapping context"));

	// NOTE: The IMC_Spaceship Blueprint asset should already have all key mappings configured.
	// We no longer need to create them dynamically here. If WASD doesn't work, check the
	// IMC_Spaceship asset in Unreal Editor to ensure it has proper WASD -> IA_Move mappings.
	
	/* REMOVED: Manual key mapping (now done in Blueprint asset)
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
	*/

	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Using key mappings from IMC_Spaceship Blueprint asset"));
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
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: BOUND HandleMove to MoveAction"));
	}
	else
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("SpaceshipControlsComponent: MoveAction is NULL, cannot bind!"));
	}

	// Bind Look action
	if (LookAction)
	{
		PlayerInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &USpaceshipControlsComponent::HandleLook);
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: BOUND HandleLook to LookAction"));
	}
	else
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("SpaceshipControlsComponent: LookAction is NULL, cannot bind!"));
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

void USpaceshipControlsComponent::InitializeInputBindings(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Cannot initialize - PlayerInputComponent is null"));
		return;
	}

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComp)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: PlayerInputComponent is not an EnhancedInputComponent"));
		return;
	}

	// Bind input actions
	SetupInputBindings(EnhancedInputComp);

	// Enable controls (add mapping context)
	EnableControls();

	UE_LOG(LogAdastreaInput, Log, TEXT("SpaceshipControlsComponent: Input bindings initialized and controls enabled"));
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
		UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent::EnableControls() - Controls already enabled, skipping"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (!Subsystem)
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("SpaceshipControlsComponent::EnableControls() - Failed to get EnhancedInputSubsystem!"));
		return;
	}
	
	if (!SpaceshipMappingContext)
	{
		UE_LOG(LogAdastreaInput, Error, TEXT("SpaceshipControlsComponent::EnableControls() - SpaceshipMappingContext is null!"));
		return;
	}
	
	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent::EnableControls() - Adding mapping context '%s' with priority %d"), 
		*SpaceshipMappingContext->GetName(), InputMappingPriority);
	
	Subsystem->AddMappingContext(SpaceshipMappingContext, InputMappingPriority);
	bControlsEnabled = true;
	OnControlsEnabled.Broadcast();
	
	UE_LOG(LogAdastreaInput, Warning, TEXT("SpaceshipControlsComponent: Controls enabled successfully"));
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
	FVector2D RawValue = Value.Get<FVector2D>();
	UE_LOG(LogAdastreaInput, Warning, TEXT("HandleMove called! Raw=(%.2f, %.2f), MovementSpeed=%.2f, CurrentSpeed=%.2f"),
		RawValue.X, RawValue.Y, MovementSpeed, CurrentSpeed);

	// Update target movement input
	// Only scale forward/backward (X) by throttle, strafing (Y,Z) acts as thrusters independent of speed
	FVector2D ScaledValue = RawValue * MovementSpeed;
	ScaledValue.X *= CurrentSpeed;  // Forward/backward scaled by throttle
	// ScaledValue.Y remains unscaled (left/right strafe)
	// Note: Z axis (up/down) is not in this 2D vector, handled separately in Spaceship.cpp
	TargetMovementInput = ScaledValue;

	// Apply movement smoothing if enabled
	if (bEnableMovementSmoothing)
	{
		// Interpolate current movement toward target
		CurrentMovementInput = FMath::Vector2DInterpTo(CurrentMovementInput, TargetMovementInput, GetWorld()->GetDeltaSeconds(), MovementSmoothingSpeed);
	}
	else
	{
		// No smoothing - use target directly
		CurrentMovementInput = TargetMovementInput;
	}

	UE_LOG(LogAdastreaInput, Warning, TEXT("HandleMove: Final MoveValue=(%.2f, %.2f)"), CurrentMovementInput.X, CurrentMovementInput.Y);
	OnMoveInput(CurrentMovementInput);
}

void USpaceshipControlsComponent::HandleLook(const FInputActionValue& Value)
{
	FVector2D LookValue = Value.Get<FVector2D>() * LookSensitivity;

	// Apply Y axis inversion if enabled
	if (bInvertLookY)
	{
		LookValue.Y = -LookValue.Y;
	}

	// Get the owning pawn
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		return;
	}

	// Update target rotation based on input
	FRotator CurrentRotation = OwningPawn->GetActorRotation();
	TargetShipRotation = CurrentRotation;

	// Apply yaw (left/right look)
	TargetShipRotation.Yaw += LookValue.X;

	// Apply pitch (up/down look) with clamping to prevent flipping
	float NewPitch = FMath::Clamp(TargetShipRotation.Pitch + LookValue.Y, -89.0f, 89.0f);
	TargetShipRotation.Pitch = NewPitch;

	// If rotation smoothing is disabled, apply immediately
	if (!bEnableRotationSmoothing)
	{
		OwningPawn->SetActorRotation(TargetShipRotation);
		CurrentShipRotation = TargetShipRotation;
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
	ASpaceship* OwningShip = Cast<ASpaceship>(GetOwner());
	if (!OwningShip)
	{
		return;
	}

	// Debug logging
	UE_LOG(LogAdastreaInput, Warning, TEXT("OnMoveInput: X=%.2f, Y=%.2f"), MoveValue.X, MoveValue.Y);

	// Call the spaceship's movement functions directly
	// MoveValue.X = horizontal strafe (A/D)
	// MoveValue.Y = forward/backward (W/S)
	
	// Forward/backward movement (W/S)
	if (!FMath::IsNearlyZero(MoveValue.Y, 0.01f))
	{
		OwningShip->MoveForward(MoveValue.Y);
		UE_LOG(LogAdastreaInput, Warning, TEXT("OnMoveInput: Called MoveForward(%.2f)"), MoveValue.Y);
	}
	
	// Left/right strafe (A/D)
	if (!FMath::IsNearlyZero(MoveValue.X, 0.01f))
	{
		OwningShip->MoveRight(MoveValue.X);
		UE_LOG(LogAdastreaInput, Warning, TEXT("OnMoveInput: Called MoveRight(%.2f)"), MoveValue.X);
	}
}

void USpaceshipControlsComponent::OnLookInput_Implementation(FVector2D LookValue)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		return;
	}

	// Debug logging
	UE_LOG(LogAdastreaInput, Warning, TEXT("OnLookInput: X=%.2f, Y=%.2f"), LookValue.X, LookValue.Y);

	// Apply rotation directly to the actor for spaceship controls
	// Mouse X = horizontal movement = Yaw (left/right turning)
	// Mouse Y = vertical movement = Pitch (up/down angling)
	// Apply sensitivity and optional Y-axis inversion
	float Yaw = LookValue.X * LookSensitivity;
	float Pitch = LookValue.Y * LookSensitivity * (bInvertLookY ? -1.0f : 1.0f);
	
	FRotator DeltaRotation = FRotator(Pitch, Yaw, 0.0f);
	OwningPawn->AddActorWorldRotation(DeltaRotation);
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
