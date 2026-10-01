// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimationAsset.h"
#include "AdastreaLocomotionAnimInstance.generated.h"

class UBlendSpace;

/**
 * Worker-thread side of UAdastreaLocomotionAnimInstance: plays the standing and the
 * crouched locomotion blend spaces and cross-fades between them. The instance writes
 * each frame's inputs straight into it from NativeUpdateAnimation (game thread).
 */
struct FAdastreaLocomotionProxy : public FAnimInstanceProxy
{
	FAdastreaLocomotionProxy() = default;
	explicit FAdastreaLocomotionProxy(UAnimInstance* InAnimInstance) : FAnimInstanceProxy(InAnimInstance) {}

	/** One blend space and its playback state. */
	struct FLayer
	{
		UBlendSpace* BlendSpace = nullptr;
		FVector Input = FVector::ZeroVector;
		float PlayRate = 1.0f;
		float Time = 0.0f;
		TArray<FBlendSampleData> Samples;
		FBlendFilter Filter;
		FMarkerTickRecord Markers;
		FDeltaTimeRecord DeltaTimeRecord;
		/** Blend space the filter and samples were set up for. */
		UBlendSpace* PreparedFor = nullptr;
	};

	FLayer Stand;
	FLayer Crouch;
	/** 0 = standing pose, 1 = crouched pose. */
	float CrouchAlpha = 0.0f;

protected:
	virtual void UpdateAnimationNode(const FAnimationUpdateContext& InContext) override;
	virtual bool Evaluate(FPoseContext& Output) override;

private:
	void AdvanceLayer(FLayer& Layer, float Weight, const FAnimationUpdateContext& InContext);
	void EvaluateLayer(FLayer& Layer, FPoseContext& Output);
};

/**
 * Locomotion for every character body (player avatar and NPCs), written in C++ so
 * it needs no Animation Blueprint graph: a direction x speed blend space for
 * standing and another for crouching, cross-faded by crouch state.
 *
 * Reads the owning AAdastreaCharacter: GetLocomotionVelocity() for speed and
 * direction (the player avatar returns its own swept-mover velocity), WantsCrouchPose(), and
 * the blend spaces it names (StandLocomotion/CrouchLocomotion).
 *
 * Blend spaces are expected to have "Direction" (-180..180) and "Speed" axes, as
 * the UE5 Mannequin ones do. The Speed axis need not be cm/s (the Mover crouch
 * space uses 0..100 for a 300 cm/s clip): each space's stride clip is measured
 * from its root motion. Characters move slower than the clips were authored for
 * (190 cm/s walk vs 300), so below that the clip is held and slowed to match:
 * real strides, no foot sliding.
 *
 * Not yet: montage slot, foot IK, turn-in-place (later phases).
 */
UCLASS(Transient, NotBlueprintable)
class ADASTREA_API UAdastreaLocomotionAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** Ground speed (cm/s). */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion")
	float Speed = 0.0f;

	/** Movement direction relative to facing (degrees, -180..180; +90 = strafing right). */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion")
	float Direction = 0.0f;

	/** Eased crouch blend (0 standing .. 1 crouched). */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion")
	float CrouchAlpha = 0.0f;

	/** How quickly the body sinks into / rises out of a crouch. */
	UPROPERTY(EditDefaultsOnly, Category="Locomotion")
	float CrouchBlendSpeed = 8.0f;

	/** Speed (cm/s) over which a standing body fades into its stride. */
	UPROPERTY(EditDefaultsOnly, Category="Locomotion")
	float StrideFadeInSpeed = 60.0f;

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return new FAdastreaLocomotionProxy(this); }

private:
	/** A blend space's slowest moving samples (the walk): where they sit on the
	 * Speed axis and how fast each direction's clip really travels (cm/s, from root
	 * motion; a backward walk is usually slower than a forward one). */
	struct FStride
	{
		TWeakObjectPtr<const UBlendSpace> BlendSpace;
		int32 SpeedAxis = INDEX_NONE;
		int32 DirectionAxis = INDEX_NONE;
		float AxisValue = 0.0f;
		/** (direction in degrees, clip speed) per walk sample. */
		TArray<TPair<float, float>> ClipSpeeds;

		/** Clip speed of the walk sample nearest this direction (0 if none). */
		float GetClipSpeed(float InDirection) const;
	};

	static void MeasureStride(FStride& Stride, const UBlendSpace* BlendSpace);

	/** Fill a layer's blend input and play rate from Speed/Direction. */
	void SetLayerInput(FAdastreaLocomotionProxy::FLayer& Layer, UBlendSpace* BlendSpace, FStride& Stride) const;

	FStride StandStride;
	FStride CrouchStride;

	/** Keep the blend spaces referenced (the proxy holds raw pointers). */
	UPROPERTY(Transient)
	TObjectPtr<UBlendSpace> StandBlendSpace;
	UPROPERTY(Transient)
	TObjectPtr<UBlendSpace> CrouchBlendSpace;
};
