// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AdastreaCharacter.generated.h"

class UBlendSpace;

/**
 * Shared base for every human body in the game: the player's on-foot avatar
 * (ASpaceshipAvatar) and, later, NPCs.
 *
 * Owns the body, not the brain: the skeletal mesh (UE5 Mannequin skeleton — the
 * project standard, so later art and animation retarget onto it), its locomotion
 * animation (UAdastreaLocomotionAnimInstance) and the ragdoll.
 * Input, cameras and AI live in subclasses/controllers.
 *
 * Ragdoll: EnterRagdoll() hands the whole skeleton to physics (PA_Mannequin) and
 * detaches the mesh; the capsule stops colliding and follows the pelvis so cameras
 * and attached components track the body. ExitRagdoll() stands the character back
 * up where the body came to rest (no get-up animation yet — that's a later phase).
 * In zero-g (bZeroGravity) the bodies float instead of dropping.
 */
UCLASS()
class ADASTREA_API AAdastreaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAdastreaCharacter();

	virtual void Tick(float DeltaSeconds) override;

	/** Go limp: the skeleton is simulated by its physics asset from here on.
	 * Impulse (cm/s, mass-independent) is applied at HitBone, or the pelvis if none. */
	UFUNCTION(BlueprintCallable, Category="Character|Ragdoll")
	void EnterRagdoll(FVector Impulse = FVector::ZeroVector, FName HitBone = NAME_None);

	/** Stop simulating and stand back up where the body lies. */
	UFUNCTION(BlueprintCallable, Category="Character|Ragdoll")
	void ExitRagdoll();

	UFUNCTION(BlueprintPure, Category="Character|Ragdoll")
	bool IsRagdoll() const { return bIsRagdoll; }

	/** Bodies float instead of falling while ragdolled (and on entering zero-g mid-ragdoll). */
	UFUNCTION(BlueprintCallable, Category="Character|Ragdoll")
	void SetZeroGravity(bool bEnable);

	UFUNCTION(BlueprintPure, Category="Character|Ragdoll")
	bool IsZeroGravity() const { return bZeroGravity; }

	/** Velocity the body animates to. Subclasses that move themselves (the player
	 * avatar's swept mover) return their own; CharacterMovement's isn't theirs. */
	virtual FVector GetLocomotionVelocity() const { return GetVelocity(); }

	/** True while the body should hold a crouched pose (animation reads this). */
	virtual bool WantsCrouchPose() const { return bIsCrouched; }

	UBlendSpace* GetStandLocomotion() const { return StandLocomotion; }
	UBlendSpace* GetCrouchLocomotion() const { return CrouchLocomotion; }

protected:
	/** Standing locomotion: a Direction x Speed blend space (idle at speed 0). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Animation")
	TObjectPtr<UBlendSpace> StandLocomotion;

	/** Crouched locomotion, same axes. Optional: without it the body never crouches. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Animation")
	TObjectPtr<UBlendSpace> CrouchLocomotion;

	/** Called after the ragdoll state flips, for subclasses (cameras, input, AI). */
	virtual void OnRagdollChanged(bool bRagdoll) {}

	/** Velocity the body should carry into the ragdoll (subclasses with their own mover override). */
	virtual FVector GetRagdollInheritVelocity() const { return GetVelocity(); }

	/** Bone the capsule follows while ragdolled, and the default impulse bone. */
	UPROPERTY(EditDefaultsOnly, Category="Character|Ragdoll")
	FName PelvisBone = TEXT("pelvis");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Ragdoll")
	bool bZeroGravity = false;

	/** Linear damping on ragdoll bodies in zero-g, so a floating body drifts to rest
	 * over a few seconds instead of tumbling forever. */
	UPROPERTY(EditAnywhere, Category="Character|Ragdoll")
	float ZeroGravityLinearDamping = 0.4f;

private:
	bool bIsRagdoll = false;

	/** Movement mode to go back to when standing back up. */
	TEnumAsByte<EMovementMode> PreRagdollMovementMode = MOVE_None;

	/** Mesh placement relative to the capsule, restored when standing back up. */
	FTransform MeshRelativeTransform;
	FName MeshCollisionProfile;
};
