// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/AdastreaLocomotionAnimInstance.h"
#include "Characters/AdastreaCharacter.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimationAsset.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequenceBase.h"
#include "AnimationRuntime.h"
#include "Animation/AnimSyncScope.h"

namespace
{
	/** Index of the blend space axis with this display name, or INDEX_NONE. */
	int32 FindAxis(const UBlendSpace* BlendSpace, const TCHAR* Name)
	{
		for (int32 Axis = 0; Axis < 3; ++Axis)
		{
			if (BlendSpace->GetBlendParameter(Axis).DisplayName.Equals(Name, ESearchCase::IgnoreCase))
			{
				return Axis;
			}
		}
		return INDEX_NONE;
	}
}

// --- Game thread ---

void UAdastreaLocomotionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AAdastreaCharacter* Character = Cast<AAdastreaCharacter>(TryGetPawnOwner());
	if (!Character || Character->IsRagdoll())
	{
		return; // physics owns the pose while limp
	}

	const FVector Velocity = Character->GetLocomotionVelocity();
	Speed = Velocity.Size2D();
	if (Speed > 5.0f)
	{
		// Keep the last heading while (nearly) stopped, so idle doesn't snap around.
		const FVector Local = Character->GetActorRotation().UnrotateVector(FVector(Velocity.X, Velocity.Y, 0.0f));
		Direction = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));
	}
	CrouchAlpha = FMath::FInterpTo(CrouchAlpha, Character->WantsCrouchPose() ? 1.0f : 0.0f, DeltaSeconds, CrouchBlendSpeed);

	StandBlendSpace = Character->GetStandLocomotion();
	CrouchBlendSpace = Character->GetCrouchLocomotion();

	// The proxy's parallel update starts after this returns, so writing it here is safe.
	FAdastreaLocomotionProxy& Proxy = GetProxyOnGameThread<FAdastreaLocomotionProxy>();
	SetLayerInput(Proxy.Stand, StandBlendSpace, StandStride);
	SetLayerInput(Proxy.Crouch, CrouchBlendSpace, CrouchStride);
	Proxy.CrouchAlpha = CrouchBlendSpace ? CrouchAlpha : 0.0f;
}

float UAdastreaLocomotionAnimInstance::FStride::GetClipSpeed(float InDirection) const
{
	float Best = 0.0f;
	float BestAngle = TNumericLimits<float>::Max();
	for (const TPair<float, float>& Entry : ClipSpeeds)
	{
		const float Angle = FMath::Abs(FMath::FindDeltaAngleDegrees(InDirection, Entry.Key));
		if (Angle < BestAngle)
		{
			BestAngle = Angle;
			Best = Entry.Value;
		}
	}
	return Best;
}

void UAdastreaLocomotionAnimInstance::MeasureStride(FStride& Stride, const UBlendSpace* BlendSpace)
{
	Stride = FStride();
	Stride.BlendSpace = BlendSpace;
	Stride.SpeedAxis = FindAxis(BlendSpace, TEXT("Speed"));
	Stride.DirectionAxis = FindAxis(BlendSpace, TEXT("Direction"));
	if (Stride.SpeedAxis == INDEX_NONE)
	{
		return;
	}

	// The walk: the slowest non-zero speed any sample was placed at.
	for (const FBlendSample& Sample : BlendSpace->GetBlendSamples())
	{
		const float Value = Sample.SampleValue[Stride.SpeedAxis];
		if (Value > KINDA_SMALL_NUMBER && (Stride.AxisValue == 0.0f || Value < Stride.AxisValue))
		{
			Stride.AxisValue = Value;
		}
	}
	if (Stride.AxisValue == 0.0f)
	{
		return;
	}

	// Each walk clip's real pace, from its root motion; fall back to reading the axis as cm/s.
	for (const FBlendSample& Sample : BlendSpace->GetBlendSamples())
	{
		if (!FMath::IsNearlyEqual(Sample.SampleValue[Stride.SpeedAxis], Stride.AxisValue, 0.5f))
		{
			continue;
		}
		float ClipSpeed = Stride.AxisValue;
		if (const UAnimSequenceBase* Clip = Sample.Animation)
		{
			const double Length = Clip->GetPlayLength();
			if (Length > UE_KINDA_SMALL_NUMBER)
			{
				const FVector Travel = Clip->ExtractRootMotionFromRange(0.0, Length, FAnimExtractContext()).GetTranslation();
				if (Travel.Size2D() > 1.0)
				{
					ClipSpeed = static_cast<float>(Travel.Size2D() / Length);
				}
			}
		}
		const float SampleDirection = Stride.DirectionAxis != INDEX_NONE ? Sample.SampleValue[Stride.DirectionAxis] : 0.0f;
		Stride.ClipSpeeds.Emplace(SampleDirection, ClipSpeed);
	}
}

void UAdastreaLocomotionAnimInstance::SetLayerInput(FAdastreaLocomotionProxy::FLayer& Layer, UBlendSpace* BlendSpace, FStride& Stride) const
{
	Layer.BlendSpace = BlendSpace;
	if (!BlendSpace)
	{
		return;
	}
	if (Stride.BlendSpace != BlendSpace)
	{
		MeasureStride(Stride, BlendSpace);
	}

	float InputSpeed = 0.0f;
	float PlayRate = 1.0f;
	const float ClipSpeed = Stride.GetClipSpeed(Direction);
	if (ClipSpeed > 0.0f)
	{
		if (Speed < ClipSpeed)
		{
			// Below the stride clip's pace, hold that clip and slow it down to the real
			// speed (a stride blended with idle reads as shuffling and slides the feet).
			// Fade it in over the first StrideFadeInSpeed so starting off isn't a pop.
			const float FadeIn = StrideFadeInSpeed > 0.0f ? FMath::Clamp(Speed / StrideFadeInSpeed, 0.0f, 1.0f) : 1.0f;
			InputSpeed = Stride.AxisValue * FadeIn;
			PlayRate = Speed > 1.0f ? FMath::Clamp(Speed / ClipSpeed, 0.25f, 1.0f) : 1.0f;
		}
		else
		{
			// Faster: move up the axis in the space's own units (walk -> jog).
			InputSpeed = Stride.AxisValue * Speed / ClipSpeed;
		}
	}

	FVector Input = FVector::ZeroVector;
	if (Stride.SpeedAxis != INDEX_NONE)
	{
		Input[Stride.SpeedAxis] = InputSpeed;
	}
	if (Stride.DirectionAxis != INDEX_NONE)
	{
		Input[Stride.DirectionAxis] = Direction;
	}
	Layer.Input = Input;
	Layer.PlayRate = PlayRate;
}

// --- Worker thread ---

void FAdastreaLocomotionProxy::UpdateAnimationNode(const FAnimationUpdateContext& InContext)
{
	AdvanceLayer(Stand, 1.0f - CrouchAlpha, InContext);
	AdvanceLayer(Crouch, CrouchAlpha, InContext);
}

void FAdastreaLocomotionProxy::AdvanceLayer(FLayer& Layer, float Weight, const FAnimationUpdateContext& InContext)
{
	if (!Layer.BlendSpace)
	{
		return;
	}
	if (Layer.PreparedFor != Layer.BlendSpace)
	{
		Layer.Samples.Reset();
		Layer.Markers.Reset();
		Layer.Time = 0.0f;
		Layer.BlendSpace->InitializeFilter(&Layer.Filter);
		Layer.PreparedFor = Layer.BlendSpace;
	}
	// Hand the engine a tick record, as a blend space player node does: its sync
	// scope ticks the samples (marker-synced feet) once the update finishes.
	// Each layer is its own ungrouped leader; both keep ticking so a cross-fade
	// picks up mid-stride.
	FAnimTickRecord TickRecord(Layer.BlendSpace, Layer.Input, Layer.Samples, Layer.Filter, /*bInLooping=*/true,
		Layer.PlayRate, /*bShouldTeleportToTime=*/false, /*bInIsEvaluator=*/false,
		FMath::Max(Weight, UE_KINDA_SMALL_NUMBER), Layer.Time, Layer.Markers);
	TickRecord.DeltaTimeRecord = &Layer.DeltaTimeRecord;
	InContext.GetMessageChecked<UE::Anim::FAnimSyncGroupScope>().AddTickRecord(TickRecord);
}

void FAdastreaLocomotionProxy::EvaluateLayer(FLayer& Layer, FPoseContext& Output)
{
	if (!Layer.BlendSpace || Layer.Samples.Num() == 0)
	{
		Output.ResetToRefPose();
		return;
	}
	FAnimationPoseData PoseData(Output);
	const FAnimExtractContext ExtractionContext(static_cast<double>(Layer.Time), /*bExtractRootMotion=*/false, FDeltaTimeRecord(), /*bLooping=*/true);
	Layer.BlendSpace->GetAnimationPose(Layer.Samples, ExtractionContext, PoseData);
}

bool FAdastreaLocomotionProxy::Evaluate(FPoseContext& Output)
{
	if (CrouchAlpha <= ZERO_ANIMWEIGHT_THRESH)
	{
		EvaluateLayer(Stand, Output);
	}
	else if (CrouchAlpha >= 1.0f - ZERO_ANIMWEIGHT_THRESH)
	{
		EvaluateLayer(Crouch, Output);
	}
	else
	{
		EvaluateLayer(Stand, Output);
		FPoseContext CrouchPose(Output);
		EvaluateLayer(Crouch, CrouchPose);
		FAnimationPoseData StandData(Output);
		const FAnimationPoseData CrouchData(CrouchPose);
		FAnimationRuntime::BlendTwoPosesTogetherInPlace(StandData, CrouchData, 1.0f - CrouchAlpha);
	}
	return true;
}
