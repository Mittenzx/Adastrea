// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceshipAvatar.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
class IWorldInteractable;
struct FInputActionValue;

/**
 * The worldwide on-foot avatar the player controls when NOT flying a ship.
 *
 * This is a SHARED third-person character used everywhere the player is on foot:
 * walking a ship interior, a space station's corridors, or on a planetside base.
 * Controls are consistent regardless of location:
 *   - WASD walk, mouse look, Space jump, Shift sprint, C/LeftCtrl crouch.
 *   - E interacts with the nearest IWorldInteractable (ship seat, station terminal,
 *     airlock, door, NPC). The prompt is shown on the HUD.
 */
UCLASS()
class ADASTREA_API ASpaceshipAvatar : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceshipAvatar();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** The ship we left, which owns the interior we're standing in. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	TObjectPtr<class ASpaceship> SourceShip;

	/** The interior scene we are currently inside. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	TObjectPtr<class ASpaceshipInterior> CurrentInterior;

	/** Return possession to the ship at its saved cockpit transform. */
	UFUNCTION(BlueprintCallable, Category="Avatar")
	void SitDown();

	/** Set the movement speed multipliers for walk/sprint/crouch — configurable per biome. */
	UFUNCTION(BlueprintCallable, Category="Avatar|Movement")
	void SetMovementTuning(float InWalkSpeed, float InSprintMultiplier, float InCrouchMultiplier);

	/** The nearest interactable we're currently prompting for (may be null). */
	UFUNCTION(BlueprintPure, Category="Avatar|Interaction")
	class AActor* GetCurrentInteractableActor() const { return CurrentInteractable; }

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void MoveForward();
	void MoveBack();
	void MoveLeft();
	void MoveRight();
	void Turn(float Value);
	void LookUp(float Value);
	void SprintStart();
	void SprintEnd();
	void CrouchStart();
	void CrouchEnd();
	void Interact();

	/** Scan around the avatar for the nearest interactable actor and store it. */
	void UpdateInteractableScan(APlayerController* PC);

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- Input ---
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SitDownAction;
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

private:
	bool bIsOnFoot = false;
};