// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ships/SpaceshipAvatar.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
#include "AdastreaLog.h"

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
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	MoveComp->bUseControllerDesiredRotation = false;

	// Camera boom + follow camera.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 380.0f;
	CameraBoom->bUsePawnControlRotation = true;  // rotate boom with controller (mouse look)
	CameraBoom->SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f)); // slight downward tilt

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // camera stays level relative to boom

	bUseControllerRotationYaw = true; // character faces where we look
}

void ASpaceshipAvatar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASpaceshipAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Reliable legacy key bindings so the avatar walks/looks without requiring
	// (possibly absent) Blueprint-configured input-action assets. This mirrors
	// how the ship core controls (Tab/M/map/trading) are bound directly.
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
		// Look
		InputComponent->BindAxis("Turn", this, &ASpaceshipAvatar::Turn);
		InputComponent->BindAxis("LookUp", this, &ASpaceshipAvatar::LookUp);
		// Sit down / return to cockpit
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

void ASpaceshipAvatar::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	// Walk relative to the controller's facing (forward/right).
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