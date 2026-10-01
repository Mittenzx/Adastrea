// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ships/SpaceshipAvatar.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
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
#include "Audio/AudioEventLibrary.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ASpaceshipAvatar::ASpaceshipAvatar()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule: still the collision shape movement sweeps against, and what the
	// interior's FloorCollision/wall colliders block. Human-sized: 176 cm tall and
	// 68 cm across the shoulders, so 2 m wide x 2.4 m blast doors read as doors.
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);
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
	CameraBoom->TargetArmLength = ThirdPersonArmLength;
	CameraBoom->bUsePawnControlRotation = true;  // rotate boom with controller (mouse look)
	CameraBoom->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f)); // slight downward tilt
	// Ship-interior walls and walk collision block only the Pawn channel (not Camera),
	// so probe Pawn — otherwise the third-person camera swings straight through them.
	// The spring arm ignores its own actor, so the capsule never pulls it in.
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeChannel = ECC_Pawn;
	CameraBoom->ProbeSize = 10.0f;

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

	// Placeholder suited body built from engine basic shapes, so third-person has
	// something to look at until a real character mesh exists. Sized to the 176 cm
	// capsule (half-height 88): legs, torso, helmet, and a visor showing facing.
	// Purely visual — no collision, and hidden in first-person (SetBodyVisible).
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto AddBodyPart = [this](const TCHAR* Name, UStaticMesh* PartMesh, const FVector& Location, const FVector& Scale)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(GetCapsuleComponent());
		Part->SetStaticMesh(PartMesh);
		Part->SetRelativeLocation(Location);
		Part->SetRelativeScale3D(Scale);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetGenerateOverlapEvents(false);
		Part->SetCanEverAffectNavigation(false);
		Part->SetVisibility(false); // shown when switching to third-person
		BodyParts.Add(Part);
	};
	// Basic shapes are 100 cm across, centred on their origin.
	AddBodyPart(TEXT("BodyLegs"),   CylinderMesh.Object, FVector(0.0f, 0.0f, -46.0f), FVector(0.46f, 0.36f, 0.84f));
	AddBodyPart(TEXT("BodyTorso"),  CylinderMesh.Object, FVector(0.0f, 0.0f, 24.0f),  FVector(0.60f, 0.40f, 0.56f));
	AddBodyPart(TEXT("BodyPack"),   CubeMesh.Object,     FVector(-24.0f, 0.0f, 28.0f), FVector(0.14f, 0.40f, 0.44f));
	AddBodyPart(TEXT("BodyHelmet"), SphereMesh.Object,   FVector(0.0f, 0.0f, 68.0f),  FVector(0.32f));
	AddBodyPart(TEXT("BodyVisor"),  SphereMesh.Object,   FVector(9.0f, 0.0f, 70.0f),  FVector(0.18f, 0.24f, 0.14f));
}

void ASpaceshipAvatar::BeginPlay()
{
	Super::BeginPlay();

	// Tint the placeholder body: light suit, dark visor (BasicShapeMaterial exposes "Color").
	for (UStaticMeshComponent* Part : BodyParts)
	{
		if (UMaterialInstanceDynamic* MID = Part ? Part->CreateDynamicMaterialInstance(0) : nullptr)
		{
			const bool bVisor = Part->GetFName() == TEXT("BodyVisor");
			MID->SetVectorParameterValue(TEXT("Color"), bVisor ? FLinearColor(0.02f, 0.03f, 0.05f) : FLinearColor(0.55f, 0.57f, 0.6f));
		}
	}
	SetBodyVisible(!bFirstPersonView);
}

void ASpaceshipAvatar::SetBodyVisible(bool bVisible)
{
	// The placeholder only stands in until a real body exists: once the character
	// mesh has a skeletal mesh assigned, it is the body and the basic shapes stay hidden.
	const bool bHasRealBody = GetMesh() && GetMesh()->GetSkeletalMeshAsset();
	for (UStaticMeshComponent* Part : BodyParts)
	{
		if (Part)
		{
			Part->SetVisibility(bVisible && !bHasRealBody);
			Part->SetHiddenInGame(!bVisible || bHasRealBody);
		}
	}
	if (GetMesh())
	{
		GetMesh()->SetVisibility(bVisible, true);
		GetMesh()->SetHiddenInGame(!bVisible, true);
	}
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
		// The boom mounts at the capsule centre, so eye height is measured up from
		// the capsule's bottom; UpdateFirstPersonCamera adds crouch, bob and smoothing.
		CameraBoom->TargetArmLength = 0.0f;
		CurrentEyeHeight = bCrouchingSpeed ? CrouchEyeHeight : EyeHeight;
		bEyeSmoothingValid = false;
		HeadBobBlend = 0.0f;
		CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, CurrentEyeHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
		CameraBoom->bUsePawnControlRotation = true;
		CameraBoom->SocketOffset = FVector::ZeroVector;
		CameraBoom->bEnableCameraLag = false;
		FollowCamera->bUsePawnControlRotation = true;
		SetBodyVisible(false);
	}
	else
	{
		// Third-person: a short over-the-shoulder boom from shoulder height, so the
		// camera fits ship corridors; it pulls in against walls (ProbeChannel = Pawn).
		// A little positional lag smooths stair steps and floor snaps.
		CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, ThirdPersonMountHeight));
		CameraBoom->TargetArmLength = ThirdPersonArmLength;
		CameraBoom->SocketOffset = ThirdPersonSocketOffset;
		CameraBoom->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f));
		CameraBoom->bUsePawnControlRotation = true;
		CameraBoom->bEnableCameraLag = true;
		CameraBoom->CameraLagSpeed = 15.0f;
		FollowCamera->bUsePawnControlRotation = false;
		FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);
		SetBodyVisible(true);
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

void ASpaceshipAvatar::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetActorHiddenInGame(false);
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

	CurrentInteractable = nullptr;
	// The avatar is parked (not destroyed) when the player returns to the helm, and
	// its third-person body would otherwise be left standing where the deck was.
	SetActorHiddenInGame(true);
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
	// Read the held move value straight from Enhanced Input rather than relying only on
	// the Triggered event having fired before this Tick this frame (tick order vs. input
	// processing) — otherwise walking drops out on frames that also carry mouse-look.
	if (const UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{
			const FVector2D Held = EIC->GetBoundActionValue(MoveAction).Get<FVector2D>();
			if (Held.SizeSquared() > PendingMoveInput.SizeSquared())
			{
				PendingMoveInput = Held;
			}
		}
	}

	// Ease the planar velocity toward the input direction instead of starting and
	// stopping dead: a person takes a step or two to reach walking pace and to stop.
	float Speed = WalkSpeed;
	if (bSprinting)
	{
		Speed *= SprintMultiplier;
	}
	else if (bCrouchingSpeed)
	{
		Speed *= CrouchMultiplier;
	}
	FVector InputDir = GetActorForwardVector() * PendingMoveInput.X + GetActorRightVector() * PendingMoveInput.Y;
	InputDir.Z = 0.0f;
	InputDir = InputDir.GetClampedToMaxSize(1.0f); // diagonals are no faster
	const FVector TargetVelocity = InputDir * Speed;
	const float Rate = TargetVelocity.IsNearlyZero() || FVector::DotProduct(TargetVelocity, MoveVelocity) < 0.0f
		? Deceleration : Acceleration;
	MoveVelocity = FMath::VInterpConstantTo(MoveVelocity, TargetVelocity, DeltaSeconds, Rate);

	float MovedSpeed = 0.0f;
	if (!MoveVelocity.IsNearlyZero(1.0f) && DeltaSeconds > 0.0f)
	{
		const FVector Before = GetActorLocation();
		MoveSafe(MoveVelocity * DeltaSeconds);
		// Horizontal distance actually covered (walking into a wall makes no steps).
		const float Moved = FVector::Dist2D(Before, GetActorLocation());
		MovedSpeed = Moved / DeltaSeconds;
		// Pressed against a wall: drop the blocked part of the velocity so the avatar
		// doesn't carry phantom momentum when it slides off the end of the wall.
		if (MovedSpeed < MoveVelocity.Size() * 0.5f)
		{
			MoveVelocity = MoveVelocity.GetSafeNormal() * MovedSpeed;
		}
		UpdateFootsteps(Moved, MoveVelocity.Size());
	}
	else
	{
		MoveVelocity = FVector::ZeroVector;
		// Standing still: the next step starts a fresh stride.
		FootstepDistance = 0.0f;
	}
	PendingMoveInput = FVector2D::ZeroVector;

	// Hold the deck in either view — third-person walks the same stairs and ramps.
	if (CurrentInterior || bWalkingStation)
	{
		SnapToFloor();
	}

	if (bFirstPersonView)
	{
		UpdateFirstPersonCamera(DeltaSeconds, MovedSpeed);
	}
}

float ASpaceshipAvatar::GetStrideLength(float MoveSpeed)
{
	// Stride grows with speed, so steps come faster when sprinting but not linearly:
	// crouch ~1.3 steps/s, walk ~1.9, sprint ~2.5 at the default speeds.
	return FMath::Clamp(MoveSpeed * 0.28f + 45.0f, 60.0f, 150.0f);
}

void ASpaceshipAvatar::UpdateFirstPersonCamera(float DeltaSeconds, float MoveSpeed)
{
	const float HalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;

	// Crouch lowers the eye over about a quarter second.
	CurrentEyeHeight = FMath::FInterpTo(CurrentEyeHeight, bCrouchingSpeed ? CrouchEyeHeight : EyeHeight, DeltaSeconds, 10.0f);
	const float EyeOffset = CurrentEyeHeight - HalfHeight;

	// Stair steps lift the capsule a riser at a time (MoveSafe/SnapToFloor); let the
	// eye follow smoothly so climbing reads as walking, not a string of pops. Big
	// jumps (spawn, teleport, deck change) snap straight through.
	const float TargetEyeZ = GetActorLocation().Z + EyeOffset;
	if (!bEyeSmoothingValid || FMath::Abs(TargetEyeZ - SmoothedEyeWorldZ) > 80.0f)
	{
		SmoothedEyeWorldZ = TargetEyeZ;
		bEyeSmoothingValid = true;
	}
	else
	{
		SmoothedEyeWorldZ = FMath::FInterpTo(SmoothedEyeWorldZ, TargetEyeZ, DeltaSeconds, 12.0f);
	}

	// Head bob, locked to the footstep stride: the head is lowest just after each
	// footfall and highest mid-stride, and sways once to each side per pair of steps.
	const bool bOnDeck = CurrentInterior || bWalkingStation;
	const bool bMoving = bOnDeck && MoveSpeed > 20.0f;
	HeadBobBlend = FMath::FInterpTo(HeadBobBlend, bMoving ? 1.0f : 0.0f, DeltaSeconds, bMoving ? 4.0f : 6.0f);
	if (bMoving)
	{
		GaitPhase = FMath::Fmod(GaitPhase + PI * MoveSpeed * DeltaSeconds / GetStrideLength(MoveSpeed), 2.0f * PI);
	}
	else if (HeadBobBlend < 0.01f)
	{
		GaitPhase = 0.0f;
	}
	const float SpeedScale = WalkSpeed > 0.0f ? FMath::Clamp(MoveSpeed / WalkSpeed, 0.4f, 1.8f) : 1.0f;
	const float BobZ = HeadBobVertical * SpeedScale * (FMath::Abs(FMath::Sin(GaitPhase)) - 0.5f) * 2.0f;
	const float BobY = HeadBobLateral * SpeedScale * FMath::Sin(GaitPhase);

	CameraBoom->SetRelativeLocation(FVector(
		0.0f,
		BobY * HeadBobBlend,
		EyeOffset + (SmoothedEyeWorldZ - TargetEyeZ) + BobZ * HeadBobBlend));
}

void ASpaceshipAvatar::UpdateFootsteps(float MovedDistance, float MoveSpeed)
{
	// Only on a deck (ship interior or station), and only for the local player.
	if (!(CurrentInterior || bWalkingStation) || !IsLocallyControlled() || !IsPlayerControlled())
	{
		return;
	}

	const float StrideLength = GetStrideLength(MoveSpeed);

	FootstepDistance += MovedDistance;
	if (FootstepDistance < StrideLength)
	{
		return;
	}
	FootstepDistance = FMath::Fmod(FootstepDistance, StrideLength);

	// Random variation 1..4, never the same one twice in a row.
	int32 Index = FMath::RandRange(1, LastFootstepIndex > 0 ? 3 : 4);
	if (LastFootstepIndex > 0 && Index >= LastFootstepIndex)
	{
		++Index;
	}
	LastFootstepIndex = Index;

	const float HalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;
	const FName EventId(*FString::Printf(TEXT("Interior.Footstep.%02d"), Index));
	UAudioEventLibrary::PlayEventAtLocation(this, EventId, GetActorLocation() - FVector(0.0f, 0.0f, HalfHeight));
}

void ASpaceshipAvatar::MoveSafe(const FVector& WorldDelta)
{
	if (WorldDelta.IsNearlyZero())
	{
		return;
	}

	// Interiors with stairs/dais steps (CurrentInterior->WalkStepHeight > 0): lift by
	// the step height, move, then sweep back down by the lift plus one more step, so a
	// riser or ramp up to that height is climbed and the capsule follows stairs and
	// ramps down instead of hovering off their lower end.
	const float StepUp = (CurrentInterior && FMath::IsNearlyZero(WorldDelta.Z)) ? CurrentInterior->WalkStepHeight : 0.0f;
	const FVector Start = GetActorLocation();
	float Lifted = 0.0f;
	if (StepUp > 0.0f)
	{
		AddActorWorldOffset(FVector(0.0f, 0.0f, StepUp), true);
		Lifted = GetActorLocation().Z - Start.Z;
	}

	FHitResult Hit;
	AddActorWorldOffset(WorldDelta, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		// Slide the remaining distance along the surface we hit, once — enough to
		// walk smoothly along a wall instead of stopping dead on first contact.
		const FVector Remaining = WorldDelta * (1.0f - Hit.Time);
		FVector SlideDelta = FVector::VectorPlaneProject(Remaining, Hit.Normal);
		if (StepUp > 0.0f)
		{
			// Climbing is the lift's job only: an upward slide off a ledge corner
			// would otherwise ratchet the capsule up faces taller than a step.
			SlideDelta.Z = FMath::Min(SlideDelta.Z, 0.0f);
		}
		if (!SlideDelta.IsNearlyZero())
		{
			AddActorWorldOffset(SlideDelta, true);
		}
	}

	if (StepUp > 0.0f)
	{
		AddActorWorldOffset(FVector(0.0f, 0.0f, -(Lifted + StepUp)), true);
		if (GetActorLocation().Z - Start.Z > StepUp + 1.0f)
		{
			SetActorLocation(Start, false); // never gain more than one step per move
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

		// Toggle first-/third-person while on foot in a ship or station (entering
		// starts in first-person; T switches back and forth).
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
		if (bWalkingStation)
		{
			PC->ExitStationInterior(false);
		}
		else if (SourceShip)
		{
			PC->ExitShipInterior(SourceShip);
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("SpaceshipAvatar: SitDown but no SourceShip to return to."));
		}
	}
}
