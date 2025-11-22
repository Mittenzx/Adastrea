// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "AdastreaLog.h"

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
	
	// Note: Input bindings are configured via Enhanced Input system using InputConfigDataAsset
	// See Source/Adastrea/Input/InputConfigDataAsset.h and Assets/EnhancedInputImplementation.md
	// To use: Create a Data Asset based on InputConfigDataAsset (DA_InputConfig) and assign
	// all Input Actions and Input Mapping Contexts in the Unreal Editor.
	// Then reference the Data Asset in your Blueprint derived from this controller.
	// For manual setup instructions, see ENHANCED_INPUT_GUIDE.md
}

void AAdastreaPlayerController::OnPossessSpaceship_Implementation(ASpaceship* NewSpaceship)
{
	// Default implementation
	// Can be overridden in Blueprints
	if (NewSpaceship)
	{
		UE_LOG(LogAdastrea, Log, TEXT("Player controller possessing spaceship: %s"), *NewSpaceship->GetName());
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
