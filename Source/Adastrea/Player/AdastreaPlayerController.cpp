// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"

AAdastreaPlayerController::AAdastreaPlayerController()
{
	// Set default values
}

void AAdastreaPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AAdastreaPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// TODO: Setup input bindings for spaceship controls
}

void AAdastreaPlayerController::OnPossessSpaceship_Implementation(ASpaceship* NewSpaceship)
{
	// Default implementation
	// Can be overridden in Blueprints
	if (NewSpaceship)
	{
		UE_LOG(LogTemp, Log, TEXT("Player controller possessing spaceship: %s"), *NewSpaceship->GetName());
	}
}

ASpaceship* AAdastreaPlayerController::GetControlledSpaceship() const
{
	return Cast<ASpaceship>(GetPawn());
}

bool AAdastreaPlayerController::IsControllingSpaceship() const
{
	return GetControlledSpaceship() != nullptr;
}
