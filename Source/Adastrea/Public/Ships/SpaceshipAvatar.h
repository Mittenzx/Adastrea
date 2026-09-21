// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceshipAvatar.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USpotLightComponent;
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
class ADASTREA_API ASpaceshipAvatar : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceshipAvatar();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void UnPossessed() override;

	/** The ship we left, which owns the interior we're standing in. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	TObjectPtr<class ASpaceship> SourceShip;

	/** The interior scene we are currently inside. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	TObjectPtr<class ASpaceshipInterior> CurrentInterior;

	/** Return possession to the ship at its saved cockpit transform. */
	UFUNCTION(BlueprintCallable, Category="Avatar")
	void SitDown();

	/** Switch to a first-person (eyes-height) camera inside an interior. */
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Movement")
	float WalkSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Movement")
	float SprintMultiplier = 1.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Movement")
	float CrouchMultiplier = 0.45f;

	// --- Interaction ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Avatar|Interaction")
	float InteractionScanRadius = 400.0f;

	/** The actor (implementing IWorldInteractable) currently in range for the E key. */
	UPROPERTY(BlueprintReadOnly, Category="Avatar|Interaction")
	TObjectPtr<AActor> CurrentInteractable;
};
