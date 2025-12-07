// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "AdastreaLog.h"
#include "Blueprint/UserWidget.h"
#include "UI/AdastreaHUDWidget.h"

AAdastreaPlayerController::AAdastreaPlayerController()
{
	// Set default values
	HUDWidgetClass = nullptr;
	HUDWidget = nullptr;
}

void AAdastreaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Input mapping contexts are now configured by the GameMode through DA_InputConfig
	// This ensures centralized input configuration and prevents conflicts

	// Create HUD widget if class is set
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UAdastreaHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(0);
			HUDWidget->InitializeHUD();
			UE_LOG(LogAdastrea, Log, TEXT("AdastreaPlayerController: Created and initialized HUD widget"));
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("AdastreaPlayerController: Failed to create HUD widget"));
		}
	}
	else
	{
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaPlayerController: No HUD widget class set - HUD will not be displayed"));
	}
}

void AAdastreaPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Input mapping contexts and bindings are configured by the GameMode through DA_InputConfig
	// This ensures centralized input configuration and prevents conflicts between systems
	//
	// Station Editor: Bind StationEditorAction to ToggleStationEditor() in Blueprint
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

void AAdastreaPlayerController::ToggleStationEditor()
{
	// Only allow station editor when controlling a spaceship
	if (!IsControllingSpaceship())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ToggleStationEditor: Not controlling a spaceship - station editor not available"));
		return;
	}

	UE_LOG(LogAdastrea, Log, TEXT("ToggleStationEditor: Broadcasting station editor toggle event"));
	
	// Broadcast the event for Blueprint handling
	OnStationEditorToggle.Broadcast();
}
