// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/AdastreaCharacter.h"
#include "Characters/AdastreaLocomotionAnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "UObject/ConstructorHelpers.h"
#include "AdastreaLog.h"

AAdastreaCharacter::AAdastreaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Human-sized: 176 cm tall, 68 cm across the shoulders.
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);

	// UE5 Mannequin (Manny) is the project's body standard: every later character
	// mesh is built on or retargeted to this skeleton. The mesh carries its own
	// physics asset (PA_Mannequin), which is what the ragdoll simulates.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMesh(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FObjectFinder<UBlendSpace> StandBlendSpace(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run.BS_Idle_Walk_Run"));
	static ConstructorHelpers::FObjectFinder<UBlendSpace> CrouchBlendSpace(
		TEXT("/Game/Characters/Mannequins/Anims/Crouch/BS_MM_CrouchStrafe.BS_MM_CrouchStrafe"));
	StandLocomotion = StandBlendSpace.Object;
	CrouchLocomotion = CrouchBlendSpace.Object;

	if (USkeletalMeshComponent* Body = GetMesh())
	{
		if (MannyMesh.Succeeded())
		{
			Body->SetSkeletalMeshAsset(MannyMesh.Object);
		}
		// Code-driven locomotion (no Animation Blueprint graph): see UAdastreaLocomotionAnimInstance.
		Body->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		Body->SetAnimInstanceClass(UAdastreaLocomotionAnimInstance::StaticClass());
		// Feet on the capsule bottom; Mannequin assets face +Y, actors face +X.
		Body->SetRelativeLocationAndRotation(
			FVector(0.0f, 0.0f, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()),
			FRotator(0.0f, -90.0f, 0.0f));
	}

	// Let unpossessed bodies (crew dummies, later NPCs between AI updates) still
	// fall onto the deck instead of hanging where they were spawned.
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bRunPhysicsWithNoController = true;
	}
}

void AAdastreaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// The mesh is detached while ragdolled; drag the (non-colliding) capsule along
	// with the pelvis so cameras, lights and anything else on the actor follow it.
	if (bIsRagdoll && GetMesh())
	{
		SetActorLocation(GetMesh()->GetSocketLocation(PelvisBone), false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AAdastreaCharacter::EnterRagdoll(FVector Impulse, FName HitBone)
{
	USkeletalMeshComponent* Body = GetMesh();
	if (bIsRagdoll || !Body)
	{
		return;
	}
	if (!Body->GetPhysicsAsset())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("%s: EnterRagdoll but the body has no physics asset."), *GetName());
		return;
	}

	const FVector InheritVelocity = GetRagdollInheritVelocity();
	bIsRagdoll = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		PreRagdollMovementMode = MoveComp->MovementMode;
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Detach so the capsule can follow the pelvis without dragging the bodies.
	MeshRelativeTransform = Body->GetRelativeTransform();
	MeshCollisionProfile = Body->GetCollisionProfileName();
	Body->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// Profile and enabled-state set together: the Ragdoll profile is QueryAndPhysics,
	// but the mesh's CharacterMesh profile is query-only and must not win.
	Body->SetCollisionProfileName(TEXT("Ragdoll"));
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Body->SetAllBodiesSimulatePhysics(true);
	Body->SetSimulatePhysics(true);
	Body->bBlendPhysics = true;
	Body->WakeAllRigidBodies();
	SetZeroGravity(bZeroGravity); // applies gravity + damping to the now-simulating bodies

	Body->SetAllPhysicsLinearVelocity(InheritVelocity);
	if (!Impulse.IsNearlyZero())
	{
		Body->AddImpulse(Impulse, HitBone.IsNone() ? PelvisBone : HitBone, /*bVelChange=*/true);
	}

	OnRagdollChanged(true);
}

void AAdastreaCharacter::ExitRagdoll()
{
	USkeletalMeshComponent* Body = GetMesh();
	if (!bIsRagdoll || !Body)
	{
		return;
	}

	const FVector Pelvis = Body->GetSocketLocation(PelvisBone);
	bIsRagdoll = false;

	Body->SetAllBodiesSimulatePhysics(false);
	Body->SetSimulatePhysics(false);
	Body->bBlendPhysics = false;
	Body->SetCollisionProfileName(MeshCollisionProfile);
	// Rebuild the bodies from the physics asset: drops the ragdoll's gravity and
	// damping overrides so the next ragdoll starts clean.
	Body->RecreatePhysicsState();
	Body->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Body->SetRelativeTransform(MeshRelativeTransform);

	// Stand up over the body: capsule bottom on the floor under the pelvis, if any.
	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector StandLocation = Pelvis + FVector(0.0f, 0.0f, HalfHeight * 0.5f);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AdastreaCharacterStandUp), false, this);
	FHitResult Hit;
	if (GetWorld() && GetWorld()->LineTraceSingleByChannel(Hit, Pelvis + FVector(0.0f, 0.0f, 50.0f),
		Pelvis - FVector(0.0f, 0.0f, 2.0f * HalfHeight), ECC_Pawn, Params))
	{
		StandLocation = Hit.Location + FVector(0.0f, 0.0f, HalfHeight);
	}
	SetActorLocation(StandLocation, false, nullptr, ETeleportType::TeleportPhysics);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// Back to whatever it was doing: walking/falling bodies land on their own, and
		// subclasses that drive movement themselves get MOVE_None back.
		MoveComp->SetMovementMode(PreRagdollMovementMode == MOVE_None ? MOVE_None : MOVE_Falling);
	}

	OnRagdollChanged(false);
}

void AAdastreaCharacter::SetZeroGravity(bool bEnable)
{
	bZeroGravity = bEnable;
	USkeletalMeshComponent* Body = GetMesh();
	if (!bIsRagdoll || !Body)
	{
		return;
	}
	Body->SetEnableGravity(!bEnable);
	for (FBodyInstance* BodyInst : Body->Bodies)
	{
		if (BodyInst && bEnable)
		{
			BodyInst->LinearDamping = FMath::Max(BodyInst->LinearDamping, ZeroGravityLinearDamping);
			BodyInst->UpdateDampingProperties();
		}
	}
	Body->WakeAllRigidBodies();
}

// --- Test commands ---

namespace AdastreaCharacterCommands
{
	/** The player's current pawn, if it's a character body. */
	AAdastreaCharacter* GetPlayerCharacter(UWorld* World)
	{
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		return PC ? Cast<AAdastreaCharacter>(PC->GetPawn()) : nullptr;
	}

	void ToggleRagdoll(AAdastreaCharacter* Character, bool bZeroG, bool bPush)
	{
		if (Character->IsRagdoll())
		{
			Character->ExitRagdoll();
			return;
		}
		Character->SetZeroGravity(bZeroG);
		// A shove from behind (and a little up), so a toggle visibly topples the body.
		const FVector Push = bPush
			? (Character->GetActorForwardVector() * 450.0f + FVector(0.0f, 0.0f, 150.0f))
			: FVector::ZeroVector;
		Character->EnterRagdoll(Push);
	}

	FAutoConsoleCommandWithWorldAndArgs GRagdollCommand(
		TEXT("adastrea.Ragdoll"),
		TEXT("Toggle ragdoll on the player's on-foot body. Usage: adastrea.Ragdoll [all] [zerog] [push] - 'all' toggles every character body in the level"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const bool bAll = Args.Contains(TEXT("all"));
			const bool bZeroG = Args.Contains(TEXT("zerog"));
			const bool bPush = Args.Contains(TEXT("push"));
			if (bAll)
			{
				for (TActorIterator<AAdastreaCharacter> It(World); It; ++It)
				{
					ToggleRagdoll(*It, bZeroG, bPush);
				}
				return;
			}
			if (AAdastreaCharacter* Character = GetPlayerCharacter(World))
			{
				ToggleRagdoll(Character, bZeroG, bPush);
			}
			else
			{
				UE_LOG(LogAdastrea, Warning, TEXT("adastrea.Ragdoll: the player isn't on foot (leave the cockpit first), or use 'all'"));
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs GSpawnCrewDummyCommand(
		TEXT("adastrea.SpawnCrewDummy"),
		TEXT("Spawn uncontrolled character bodies in front of the player (ragdoll/body testing). Usage: adastrea.SpawnCrewDummy [Count=1]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
			APawn* Pawn = PC ? PC->GetPawn() : nullptr;
			if (!Pawn)
			{
				return;
			}
			const int32 Count = Args.IsEmpty() ? 1 : FMath::Clamp(FCString::Atoi(*Args[0]), 1, 20);
			const FVector Forward = Pawn->GetActorForwardVector().GetSafeNormal2D();
			const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			for (int32 i = 0; i < Count; ++i)
			{
				// A row across the player's view, 2.5 m ahead, facing the player.
				const FVector Location = Pawn->GetActorLocation() + Forward * 250.0f
					+ Right * ((i - (Count - 1) * 0.5f) * 90.0f) + FVector(0.0f, 0.0f, 10.0f);
				World->SpawnActor<AAdastreaCharacter>(AAdastreaCharacter::StaticClass(), Location, (-Forward).Rotation(), Params);
			}
		}));
}
