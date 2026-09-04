// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ships/SpaceshipAvatar.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
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

	// Capsule (movement + collision with the interior volume).
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	// CharacterMovement: normal third-person walking.
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = 300.0f;
	MoveComp->bOrientRotationToMovement = false; // face the look direction (first-person)
	MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	MoveComp->bUseControllerDesiredRotation = false;

	// Camera boom + follow camera.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;  // rotate boom with controller (mouse look)
	CameraBoom->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f)); // slight downward tilt

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // camera stays level relative to boom

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

void ASpaceshipAvatar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Keep the interactable prompt current every frame.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UpdateInteractableScan(PC);
	}
}

void ASpaceshipAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Reliable legacy key bindings so the avatar walks/looks/interacts without
	// requiring (possibly absent) Blueprint-configured input-action assets.
	if (InputComponent)
	{
		// Movement
		InputComponent->BindKey(EKeys::W, IE_Pressed, this, &ASpaceshipAvatar::MoveForward);
		InputComponent->BindKey(EKeys::W, IE_Repeat, this, &ASpaceshipAvatar::MoveForward);
		InputComponent->BindKey(EKeys::S, IE_Pressed, this, &ASpaceshipAvatar::MoveBack);
		InputComponent->BindKey(EKeys::S, IE_Repeat, this, &ASpaceshipAvatar::MoveBack);
		InputComponent->BindKey(EKeys::A, IE_Pressed, this, &ASpaceshipAvatar::MoveLeft);
		InputComponent->BindKey(EKeys::A, IE_Repeat, this, &ASpaceshipAvatar::MoveLeft);
		InputComponent->BindKey(EKeys::D, IE_Pressed, this, &ASpaceshipAvatar::MoveRight);
		InputComponent->BindKey(EKeys::D, IE_Repeat, this, &ASpaceshipAvatar::MoveRight);
		InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ASpaceshipAvatar::Jump);
		// Sprint / couch
		InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &ASpaceshipAvatar::SprintStart);
		InputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &ASpaceshipAvatar::SprintEnd);
		InputComponent->BindKey(EKeys::C, IE_Pressed, this, &ASpaceshipAvatar::CrouchStart);
		InputComponent->BindKey(EKeys::C, IE_Released, this, &ASpaceshipAvatar::CrouchEnd);
		// Look
		InputComponent->BindAxis("Turn", this, &ASpaceshipAvatar::Turn);
		InputComponent->BindAxis("LookUp", this, &ASpaceshipAvatar::LookUp);
		// Worldwide interact (E) and return-to-seat (V)
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ASpaceshipAvatar::Interact);
		InputComponent->BindKey(EKeys::V, IE_Pressed, this, &ASpaceshipAvatar::SitDown);
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceshipAvatar::Move);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceshipAvatar::Look);
		}
		if (SitDownAction)
		{
			EnhancedInput->BindAction(SitDownAction, ETriggerEvent::Started, this, &ASpaceshipAvatar::SitDown);
		}
	}

	// Register the avatar's mapping context on the local player if present.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (AvatarMappingContext)
				{
					Subsystem->AddMappingContext(AvatarMappingContext, 30);
				}
			}
		}
	}
}

void ASpaceshipAvatar::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		const FRotator YawRot(0.0f, PC->GetControlRotation().Yaw, 0.0f);
		const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		AddMovementInput(Forward, Axis.X);
		AddMovementInput(Right, Axis.Y);
	}
}

void ASpaceshipAvatar::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void ASpaceshipAvatar::MoveForward() { AddMovementInput(GetActorForwardVector(), 1.0f); }
void ASpaceshipAvatar::MoveBack()    { AddMovementInput(GetActorForwardVector(), -1.0f); }
void ASpaceshipAvatar::MoveLeft()    { AddMovementInput(GetActorRightVector(), -1.0f); }
void ASpaceshipAvatar::MoveRight()   { AddMovementInput(GetActorRightVector(), 1.0f); }
void ASpaceshipAvatar::Turn(float Value)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->AddYawInput(Value);
	}
}
void ASpaceshipAvatar::LookUp(float Value)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->AddPitchInput(Value);
	}
}

void ASpaceshipAvatar::SprintStart()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed * SprintMultiplier;
	}
}
void ASpaceshipAvatar::SprintEnd()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}
}
void ASpaceshipAvatar::CrouchStart()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed * CrouchMultiplier;
		MoveComp->bWantsToCrouch = true;
	}
}
void ASpaceshipAvatar::CrouchEnd()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
		MoveComp->bWantsToCrouch = false;
	}
}

void ASpaceshipAvatar::SetMovementTuning(float InWalkSpeed, float InSprintMultiplier, float InCrouchMultiplier)
{
	WalkSpeed = InWalkSpeed;
	SprintMultiplier = InSprintMultiplier;
	CrouchMultiplier = InCrouchMultiplier;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}
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