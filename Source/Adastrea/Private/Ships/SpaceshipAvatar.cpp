// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ships/SpaceshipAvatar.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipInterior.h"
#include "Player/AdastreaPlayerController.h"
#include "Player/PlayerInteractableComponent.h"
#include "Player/WorldInteractable.h"
#include "AdastreaHUD.h"
#include "AdastreaLog.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ASpaceshipAvatar::ASpaceshipAvatar()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule: still the collision shape movement sweeps against, and what the
	// interior's FloorCollision/wall colliders block.
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	// Movement is direct swept translation driven from Tick (MoveSafe/SnapToFloor),
	// not CharacterMovementComponent's walking simulation — disable it entirely so
	// it can never tick, apply gravity, or otherwise interfere with manual moves.
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetComponentTickEnabled(false);
		MoveComp->SetMovementMode(MOVE_None);
	}

	// Camera boom + follow camera.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;  // rotate boom with controller (mouse look)
	CameraBoom->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f)); // slight downward tilt

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // camera stays level relative to boom

	// Flashlight: attached directly to the camera (not the boom) so it always
	// points exactly where the player is looking, in both first- and third-person.
	// Off by default — toggled with F, since ship interiors have no other lighting.
	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(FollowCamera);
	Flashlight->Intensity = 5000.0f;
	Flashlight->AttenuationRadius = 1500.0f;
	Flashlight->InnerConeAngle = 15.0f;
	Flashlight->OuterConeAngle = 30.0f;
	Flashlight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f)); // warm white
	Flashlight->CastShadows = true;
	Flashlight->SetVisibility(false);

	bUseControllerRotationYaw = true; // character faces where we look
}

void ASpaceshipAvatar::SetFirstPersonView(bool bEnable)
{
	bFirstPersonView = bEnable;

	if (bEnable)
	{
		// First-person: keep the boom attached to the capsule (so it follows the
		// pawn) but pull the camera to the mount point (arm length 0) at eye height,
		// and let controller rotation steer it. Hide the avatar mesh so it doesn't
		// clip into the near view.
		CameraBoom->TargetArmLength = 0.0f;
		CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 165.0f)); // eye height
		CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
		CameraBoom->bUsePawnControlRotation = true;
		FollowCamera->bUsePawnControlRotation = true;
		if (GetMesh())
		{
			GetMesh()->SetVisibility(false, true);
			GetMesh()->SetHiddenInGame(true, true);
		}
	}
	else
	{
		// Third-person: restore the follow boom behind the avatar.
		CameraBoom->SetRelativeLocation(FVector::ZeroVector);
		CameraBoom->TargetArmLength = 500.0f;
		CameraBoom->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f));
		CameraBoom->bUsePawnControlRotation = true;
		FollowCamera->bUsePawnControlRotation = false;
		if (GetMesh())
		{
			GetMesh()->SetVisibility(true, true);
			GetMesh()->SetHiddenInGame(false, true);
		}
	}
}

void ASpaceshipAvatar::SetFlashlightEnabled(bool bEnable)
{
	bFlashlightOn = bEnable;
	if (Flashlight)
	{
		Flashlight->SetVisibility(bEnable);
	}
}

void ASpaceshipAvatar::UnPossessed()
{
	// Remove our runtime mapping context before the controller lets go of us. It's
	// added at priority 30 (SetupPlayerInputComponent) — above the ship's own
	// priority-10 context — and maps Mouse2D. Left in place, it keeps silently
	// claiming the mouse-look key after the player returns to the ship, so the
	// ship's own Mouse2D->LookAction mapping never sees input again.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (AvatarMappingContext)
				{
					Subsystem->RemoveMappingContext(AvatarMappingContext);
				}
			}
		}
	}

	Super::UnPossessed();
}

void ASpaceshipAvatar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Keep the interactable prompt current every frame.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UpdateInteractableScan(PC);
	}

	// Consume this frame's move input (set by Move() via Enhanced Input's Triggered
	// event, which fires every frame a movement key is held) and clear it — if no
	// new input arrives before the next Tick, movement naturally stops.
	if (!PendingMoveInput.IsNearlyZero())
	{
		float Speed = WalkSpeed;
		if (bSprinting)
		{
			Speed *= SprintMultiplier;
		}
		else if (bCrouchingSpeed)
		{
			Speed *= CrouchMultiplier;
		}

		const FVector Forward = GetActorForwardVector();
		const FVector Right = GetActorRightVector();
		const FVector Delta = (Forward * PendingMoveInput.X + Right * PendingMoveInput.Y) * Speed * DeltaSeconds;
		MoveSafe(Delta);
	}
	PendingMoveInput = FVector2D::ZeroVector;

	if (bFirstPersonView && CurrentInterior)
	{
		SnapToFloor();
	}
}

void ASpaceshipAvatar::MoveSafe(const FVector& WorldDelta)
{
	if (WorldDelta.IsNearlyZero())
	{
		return;
	}

	FHitResult Hit;
	AddActorWorldOffset(WorldDelta, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		// Slide the remaining distance along the surface we hit, once — enough to
		// walk smoothly along a wall instead of stopping dead on first contact.
		const FVector Remaining = WorldDelta * (1.0f - Hit.Time);
		const FVector SlideDelta = FVector::VectorPlaneProject(Remaining, Hit.Normal);
		if (!SlideDelta.IsNearlyZero())
		{
			AddActorWorldOffset(SlideDelta, true);
		}
	}
}

void ASpaceshipAvatar::SnapToFloor()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule || !GetWorld())
	{
		return;
	}

	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.0f, 0.0f, HalfHeight + 50.0f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
	{
		const FVector NewLocation(Start.X, Start.Y, Hit.Location.Z + HalfHeight);
		if (!NewLocation.Equals(Start, 0.1f))
		{
			SetActorLocation(NewLocation, false);
		}
	}
}

void ASpaceshipAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogAdastrea, Error, TEXT("SpaceshipAvatar: PlayerInputComponent is not Enhanced Input — avatar has no controls."));
		return;
	}

	// Build a self-contained runtime mapping context so controls work without a
	// Blueprint-configured Input Action/Mapping Context asset — the same pattern
	// ASpaceship uses for its own controls (EnsureOwnInputActionsAndContext).
	if (!AvatarMappingContext)
	{
		AvatarMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_AvatarRuntime"));

		// Move: W/S = forward/back (X), A/D = strafe (Y), matching Move()'s
		// Axis.X = forward, Axis.Y = right convention.
		if (!MoveAction)
		{
			MoveAction = NewObject<UInputAction>(this, TEXT("IA_AvatarMove_Runtime"));
			MoveAction->ValueType = EInputActionValueType::Axis2D;
		}
		AvatarMappingContext->MapKey(MoveAction, EKeys::W);
		{
			FEnhancedActionKeyMapping& SMapping = AvatarMappingContext->MapKey(MoveAction, EKeys::S);
			SMapping.Modifiers.Add(NewObject<UInputModifierNegate>(AvatarMappingContext));
		}
		{
			FEnhancedActionKeyMapping& DMapping = AvatarMappingContext->MapKey(MoveAction, EKeys::D);
			UInputModifierSwizzleAxis* DSwizzle = NewObject<UInputModifierSwizzleAxis>(AvatarMappingContext);
			DSwizzle->Order = EInputAxisSwizzle::YXZ;
			DMapping.Modifiers.Add(DSwizzle);
		}
		{
			FEnhancedActionKeyMapping& AMapping = AvatarMappingContext->MapKey(MoveAction, EKeys::A);
			UInputModifierSwizzleAxis* ASwizzle = NewObject<UInputModifierSwizzleAxis>(AvatarMappingContext);
			ASwizzle->Order = EInputAxisSwizzle::YXZ;
			AMapping.Modifiers.Add(ASwizzle);
			AMapping.Modifiers.Add(NewObject<UInputModifierNegate>(AvatarMappingContext));
		}

		// Look: mouse XY.
		if (!LookAction)
		{
			LookAction = NewObject<UInputAction>(this, TEXT("IA_AvatarLook_Runtime"));
			LookAction->ValueType = EInputActionValueType::Axis2D;
		}
		AvatarMappingContext->MapKey(LookAction, EKeys::Mouse2D);

		// Sprint / crouch (speed multipliers only — see Tick).
		if (!SprintAction)
		{
			SprintAction = NewObject<UInputAction>(this, TEXT("IA_AvatarSprint_Runtime"));
			SprintAction->ValueType = EInputActionValueType::Boolean;
		}
		AvatarMappingContext->MapKey(SprintAction, EKeys::LeftShift);

		if (!CrouchAction)
		{
			CrouchAction = NewObject<UInputAction>(this, TEXT("IA_AvatarCrouch_Runtime"));
			CrouchAction->ValueType = EInputActionValueType::Boolean;
		}
		AvatarMappingContext->MapKey(CrouchAction, EKeys::C);

		// Worldwide interact (E) and return-to-seat (V).
		if (!InteractAction)
		{
			InteractAction = NewObject<UInputAction>(this, TEXT("IA_AvatarInteract_Runtime"));
			InteractAction->ValueType = EInputActionValueType::Boolean;
		}
		AvatarMappingContext->MapKey(InteractAction, EKeys::E);

		if (!SitDownAction)
		{
			SitDownAction = NewObject<UInputAction>(this, TEXT("IA_AvatarSitDown_Runtime"));
			SitDownAction->ValueType = EInputActionValueType::Boolean;
		}
		AvatarMappingContext->MapKey(SitDownAction, EKeys::V);

		if (!FlashlightAction)
		{
			FlashlightAction = NewObject<UInputAction>(this, TEXT("IA_AvatarFlashlight_Runtime"));
			FlashlightAction->ValueType = EInputActionValueType::Boolean;
		}
		AvatarMappingContext->MapKey(FlashlightAction, EKeys::F);

		// Toggle first-/third-person while walking (entering an interior forces
		// first-person by default, but the player can switch back and forth).
		if (!ToggleViewAction)
		{
			ToggleViewAction = NewObject<UInputAction>(this, TEXT("IA_AvatarToggleView_Runtime"));
			ToggleViewAction->ValueType = EInputActionValueType::Boolean;
		}
		AvatarMappingContext->MapKey(ToggleViewAction, EKeys::T);
	}

	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceshipAvatar::Move);
	EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceshipAvatar::Look);
	EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASpaceshipAvatar::SprintStarted);
	EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASpaceshipAvatar::SprintCompleted);
	EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASpaceshipAvatar::CrouchStarted);
	EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASpaceshipAvatar::CrouchCompleted);
	EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ASpaceshipAvatar::InteractPressed);
	EnhancedInput->BindAction(SitDownAction, ETriggerEvent::Started, this, &ASpaceshipAvatar::SitDownPressed);
	EnhancedInput->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ASpaceshipAvatar::FlashlightPressed);
	EnhancedInput->BindAction(ToggleViewAction, ETriggerEvent::Started, this, &ASpaceshipAvatar::ToggleViewPressed);

	// Register the avatar's mapping context on the local player.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(AvatarMappingContext, 30);
			}
		}
	}
}

void ASpaceshipAvatar::Move(const FInputActionValue& Value)
{
	PendingMoveInput = Value.Get<FVector2D>();
}

void ASpaceshipAvatar::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void ASpaceshipAvatar::SprintStarted(const FInputActionValue& Value)   { bSprinting = true; }
void ASpaceshipAvatar::SprintCompleted(const FInputActionValue& Value) { bSprinting = false; }
void ASpaceshipAvatar::CrouchStarted(const FInputActionValue& Value)   { bCrouchingSpeed = true; }
void ASpaceshipAvatar::CrouchCompleted(const FInputActionValue& Value) { bCrouchingSpeed = false; }
void ASpaceshipAvatar::InteractPressed(const FInputActionValue& Value) { Interact(); }
void ASpaceshipAvatar::SitDownPressed(const FInputActionValue& Value)  { SitDown(); }
void ASpaceshipAvatar::FlashlightPressed(const FInputActionValue& Value) { SetFlashlightEnabled(!bFlashlightOn); }
void ASpaceshipAvatar::ToggleViewPressed(const FInputActionValue& Value) { SetFirstPersonView(!bFirstPersonView); }

void ASpaceshipAvatar::SetMovementTuning(float InWalkSpeed, float InSprintMultiplier, float InCrouchMultiplier)
{
	WalkSpeed = InWalkSpeed;
	SprintMultiplier = InSprintMultiplier;
	CrouchMultiplier = InCrouchMultiplier;
}

void ASpaceshipAvatar::UpdateInteractableScan(APlayerController* PC)
{
	AActor* Best = nullptr;
	float BestDist = InteractionScanRadius;
	const FVector AvatarLoc = GetActorLocation();

	// Iterate the world for actors that are interactable: either they implement
	// IWorldInteractable directly, or they carry a UPlayerInteractableComponent.
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Candidate = *It;
		if (!Candidate || Candidate == this)
		{
			continue;
		}

		float CandidateDist = TNumericLimits<float>::Max();
		if (Candidate->Implements<UWorldInteractable>())
		{
			CandidateDist = FVector::Dist(AvatarLoc, Candidate->GetActorLocation());
		}
		else if (UPlayerInteractableComponent* Comp = Candidate->FindComponentByClass<UPlayerInteractableComponent>())
		{
			CandidateDist = FVector::Dist(AvatarLoc, Comp->GetInteractionWorldPoint());
		}
		else
		{
			continue;
		}

		if (CandidateDist < BestDist)
		{
			BestDist = CandidateDist;
			Best = Candidate;
		}
	}

	// Notify the HUD when the interactable under the cursor changes.
	if (Best != CurrentInteractable)
	{
		CurrentInteractable = Best;
		if (AAdastreaHUD* HUD = PC->GetHUD<AAdastreaHUD>())
		{
			HUD->SetCurrentInteractable(Best);
		}
	}
}

void ASpaceshipAvatar::Interact()
{
	if (!CurrentInteractable)
	{
		return;
	}
	if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(GetController()))
	{
		// Prefer a UPlayerInteractableComponent if present, else use the actor-level interface.
		if (UPlayerInteractableComponent* Comp = CurrentInteractable->FindComponentByClass<UPlayerInteractableComponent>())
		{
			Comp->Interact_Implementation(PC);
		}
		else if (CurrentInteractable->Implements<UWorldInteractable>())
		{
			IWorldInteractable::Execute_Interact(CurrentInteractable, PC);
		}
	}
}

void ASpaceshipAvatar::SitDown()
{
	// Return possession to the ship at its saved cockpit transform.
	if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(GetController()))
	{
		if (SourceShip)
		{
			PC->ExitShipInterior(SourceShip);
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("SpaceshipAvatar: SitDown but no SourceShip to return to."));
		}
	}
}
