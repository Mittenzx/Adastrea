// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AdastreaCharacter.h"
#include "SpaceshipAvatar.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USpotLightComponent;
class UStaticMeshComponent;
class UInputAction;
class UInputMappingContext;
class IWorldInteractable;
struct FInputActionValue;

/**
 * The worldwide on-foot avatar the player controls when NOT flying a ship.
 *
 * This is a SHARED third-person character used everywhere the player is on foot:
 * walking a ship interior, a space station's corridors, or on a planetside base.
 * All input is Enhanced Input, built at runtime in code (the same self-contained
 * pattern ASpaceship uses for its own controls) so it works without a Blueprint-
 * configured Input Action/Mapping Context asset.
 *
 * Movement does NOT use CharacterMovementComponent's walking simulation — it's
 * disabled on this class (see constructor) and movement is instead a direct swept
 * capsule translation each Tick, with a short downward trace to hold standing
 * height on the floor. This sidesteps a CharacterMovementComponent initialization
 * issue specific to how this avatar is spawned/possessed outside the normal
 * BeginPlay lifecycle (see PerformMove/SnapToFloor), and is appropriate for the
 * flat, single-level rooms this avatar walks in the once-established.
 */
UCLASS()
class ADASTREA_API ASpaceshipAvatar : public AAdastreaCharacter
{
	GENERATED_BODY()

public:
	ASpaceshipAvatar();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	/** The ship we left, which owns the interior we're standing in. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	TObjectPtr<class ASpaceship> SourceShip;

	/** The interior scene we are currently inside. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	TObjectPtr<class ASpaceshipInterior> CurrentInterior;

	/** True while walking a station interior (no ASpaceshipInterior); still needs floor snapping. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	bool bWalkingStation = false;

	/** Return possession to the ship at its saved cockpit transform. */
	UFUNCTION(BlueprintCallable, Category="Avatar")
	void SitDown();

	/** Switch between the first-person (eye-height) camera and the over-the-shoulder
	 * third-person camera. Toggled with T while on foot in a ship or station. */
	UFUNCTION(BlueprintCallable, Category="Avatar|Camera")
	void SetFirstPersonView(bool bEnable);

	/** True when the avatar is in first-person view (e.g. inside a ship interior). */
	UFUNCTION(BlueprintPure, Category="Avatar|Camera")
	bool IsFirstPersonView() const { return bFirstPersonView; }

	/** Set the movement speed multipliers for walk/sprint/crouch — configurable per biome. */
	UFUNCTION(BlueprintCallable, Category="Avatar|Movement")
	void SetMovementTuning(float InWalkSpeed, float InSprintMultiplier, float InCrouchMultiplier);

	/** Turn the flashlight on/off directly (as opposed to toggling it with F). */
	UFUNCTION(BlueprintCallable, Category="Avatar|Flashlight")
	void SetFlashlightEnabled(bool bEnable);

	/** True while the flashlight is currently on. */
	UFUNCTION(BlueprintPure, Category="Avatar|Flashlight")
	bool IsFlashlightOn() const { return bFlashlightOn; }

	/** The nearest interactable we're currently prompting for (may be null). */
	UFUNCTION(BlueprintPure, Category="Avatar|Interaction")
	class AActor* GetCurrentInteractableActor() const { return CurrentInteractable; }

protected:
	// --- Enhanced Input trigger handlers ---
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintStarted(const FInputActionValue& Value);
	void SprintCompleted(const FInputActionValue& Value);
	void CrouchStarted(const FInputActionValue& Value);
	void CrouchCompleted(const FInputActionValue& Value);
	void InteractPressed(const FInputActionValue& Value);
	void SitDownPressed(const FInputActionValue& Value);
	void FlashlightPressed(const FInputActionValue& Value);
	void ToggleViewPressed(const FInputActionValue& Value);

	/** Scan around the avatar for the nearest interactable actor and store it. */
	void UpdateInteractableScan(APlayerController* PC);

	/** Sweep the capsule by WorldDelta, sliding once along a blocking hit's surface. */
	void MoveSafe(const FVector& WorldDelta);

	/** Hold standing height on the floor directly below via a short downward trace,
	 * so the flat, single-level room floor doesn't need real gravity simulation. */
	void SnapToFloor();

	void Interact();

	// --- Ragdoll (AAdastreaCharacter) ---
	virtual void OnRagdollChanged(bool bRagdoll) override;
	virtual FVector GetRagdollInheritVelocity() const override { return MoveVelocity; }
	virtual FVector GetLocomotionVelocity() const override { return MoveVelocity; }

	/** Crouch (C) is a slower walk here, not CharacterMovement's crouch: pose it too. */
	virtual bool WantsCrouchPose() const override { return bCrouchingSpeed; }

	/** View to return to when the ragdoll ends (ragdolling forces third-person). */
	bool bFirstPersonBeforeRagdoll = false;

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Handheld-style light attached to the camera so it always points where the
	 * player is looking — the main way to see inside unlit ship interiors. Off by
	 * default; toggled with F. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Avatar|Flashlight")
	TObjectPtr<USpotLightComponent> Flashlight;

	/** Placeholder suited body (engine basic shapes) so the avatar is visible in
	 * third-person until a real character mesh exists. Hidden in first-person. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Avatar|Body")
	TArray<TObjectPtr<UStaticMeshComponent>> BodyParts;

	/** Show/hide the avatar's body (placeholder parts and any skeletal mesh). */
	void SetBodyVisible(bool bVisible);

	/** True while in first-person (interior) view — the camera rides at eye height. */
	bool bFirstPersonView = false;

	/** True while the flashlight is on. */
	bool bFlashlightOn = false;

	/** This frame's move input (X = forward/back, Y = right/left), applied and
	 * cleared each Tick — Enhanced Input's Triggered event re-sends it every frame
	 * a key is held, so clearing after use naturally stops movement on release. */
	FVector2D PendingMoveInput = FVector2D::ZeroVector;

	bool bSprinting = false;
	bool bCrouchingSpeed = false;

	// --- Input (Enhanced Input, built at runtime — see SetupPlayerInputComponent) ---
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SprintAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SitDownAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> FlashlightAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> ToggleViewAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> AvatarMappingContext;

	// --- Movement tuning ---
	/** Walking pace (cm/s). ~1.9 m/s is a brisk human walk. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Movement")
	float WalkSpeed = 190.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Movement")
	float SprintMultiplier = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Movement")
	float CrouchMultiplier = 0.5f;
	/** How fast the avatar gets up to speed (cm/s^2). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Movement")
	float Acceleration = 900.0f;
	/** How fast the avatar comes to a stop once the keys are released (cm/s^2). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Movement")
	float Deceleration = 1400.0f;

	// --- First-person camera ---
	/** Eye height above the floor when standing (cm). The capsule is 176 cm tall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Camera")
	float EyeHeight = 162.0f;
	/** Eye height above the floor while crouched (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Camera")
	float CrouchEyeHeight = 112.0f;
	/** Head bob at walking pace: vertical dip at each footfall (cm). Scales with speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Camera")
	float HeadBobVertical = 1.4f;
	/** Head bob at walking pace: side-to-side sway over each pair of steps (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Camera")
	float HeadBobLateral = 0.9f;

	// --- Third-person camera ---
	/** Boom length behind the avatar (cm). Short, since decks and corridors are tight;
	 * the boom also pulls in against walls (it probes the Pawn channel they block). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Camera")
	float ThirdPersonArmLength = 260.0f;
	/** Boom mount height above the capsule centre (cm) — roughly shoulder height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Camera")
	float ThirdPersonMountHeight = 55.0f;
	/** Over-the-shoulder offset at the camera end of the boom (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Camera")
	FVector ThirdPersonSocketOffset = FVector(0.0f, 45.0f, 15.0f);

	// --- Interaction ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Interaction")
	float InteractionScanRadius = 400.0f;

	/** The actor (implementing IWorldInteractable) currently in range for the E key. */
	UPROPERTY(BlueprintReadOnly, Category="Avatar|Interaction")
	TObjectPtr<AActor> CurrentInteractable;

	// --- Footsteps (Interior.Footstep.01..04) ---

	/** Distance walked since the last footstep (cm). */
	float FootstepDistance = 0.0f;

	/** Last footstep variation played (1..4), so the same one never plays twice running. */
	int32 LastFootstepIndex = 0;

	/** Accumulate the distance actually moved this frame and play a footstep every stride. */
	void UpdateFootsteps(float MovedDistance, float MoveSpeed);

	/** Stride length for a given speed: longer strides when moving faster. */
	static float GetStrideLength(float MoveSpeed);

	// --- Walk feel (first person) ---

	/** Current planar velocity (cm/s); eased toward the input direction by Acceleration/Deceleration. */
	FVector MoveVelocity = FVector::ZeroVector;

	/** Gait phase: advances by PI per stride, so each footfall lands on a multiple of PI. */
	float GaitPhase = 0.0f;

	/** 0..1 head-bob strength, eased in when moving and out when standing still. */
	float HeadBobBlend = 0.0f;

	/** Current eye height above the floor, eased between EyeHeight and CrouchEyeHeight. */
	float CurrentEyeHeight = 162.0f;

	/** Smoothed world Z of the eye, so stair steps and floor snaps glide instead of popping. */
	float SmoothedEyeWorldZ = 0.0f;
	bool bEyeSmoothingValid = false;

	/** Place the first-person camera: eye height, crouch, stair smoothing and head bob. */
	void UpdateFirstPersonCamera(float DeltaSeconds, float MoveSpeed);
};
