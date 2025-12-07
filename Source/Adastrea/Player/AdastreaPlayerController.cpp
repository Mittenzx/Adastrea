// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "StationEditor/UI/StationEditorWidget.h"
#include "StationEditor/StationModuleCatalog.h"
#include "AdastreaLog.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "UI/AdastreaHUDWidget.h"
#include "UI/ShipStatusWidget.h"

AAdastreaPlayerController::AAdastreaPlayerController()
{
	// Set default values
	StationEditorWidgetClass = nullptr;
	ModuleCatalog = nullptr;
	StationSearchRadius = 5000.0f;
	StationEditorWidget = nullptr;
	bIsStationEditorOpen = false;
	HUDWidgetClass = nullptr;
	HUDWidget = nullptr;
	ShipStatusWidgetClass = nullptr;
	ShipStatusWidget = nullptr;
	bIsShipStatusOpen = false;
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

	// Toggle editor state
	if (bIsStationEditorOpen)
	{
		// Close the editor
		HideStationEditor();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleStationEditor: Closed station editor"));
		
		// Broadcast the event for backward compatibility with existing Blueprints
		OnStationEditorToggle.Broadcast();
	}
	else
	{
		// Open the editor - find nearest station
		ASpaceStation* NearestStation = FindNearestStation();
		
		if (!NearestStation)
		{
			UE_LOG(LogAdastrea, Warning, TEXT("ToggleStationEditor: No station found within %.0f units"), StationSearchRadius);
			// Don't broadcast event on failure - no state change occurred
			return;
		}

		ShowStationEditor(NearestStation);
		UE_LOG(LogAdastrea, Log, TEXT("ToggleStationEditor: Opened station editor for station: %s"), *NearestStation->GetName());
		
		// Broadcast the event for backward compatibility with existing Blueprints
		OnStationEditorToggle.Broadcast();
	}
}

bool AAdastreaPlayerController::IsStationEditorOpen() const
{
	return bIsStationEditorOpen && StationEditorWidget && StationEditorWidget->IsInViewport();
}

UStationEditorWidget* AAdastreaPlayerController::GetStationEditorWidget() const
{
	return StationEditorWidget;
}

ASpaceStation* AAdastreaPlayerController::FindNearestStation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	FVector PlayerLocation = ControlledPawn->GetActorLocation();
	
	// Find all space stations in the world
	TArray<AActor*> FoundStations;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceStation::StaticClass(), FoundStations);
	
	ASpaceStation* NearestStation = nullptr;
	float NearestDistance = StationSearchRadius;
	
	// GetAllActorsOfClass returns actors of the specified class, so no Cast needed
	for (AActor* Actor : FoundStations)
	{
		ASpaceStation* Station = static_cast<ASpaceStation*>(Actor);
		float Distance = FVector::Dist(PlayerLocation, Station->GetActorLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestStation = Station;
		}
	}
	
	return NearestStation;
}

UStationEditorWidget* AAdastreaPlayerController::CreateStationEditorWidget()
{
	// Return existing widget if already created
	if (StationEditorWidget)
	{
		return StationEditorWidget;
	}

	// Check if widget class is assigned
	if (!StationEditorWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationEditorWidget: StationEditorWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	StationEditorWidget = CreateWidget<UStationEditorWidget>(this, StationEditorWidgetClass);
	
	if (!StationEditorWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationEditorWidget: Failed to create widget from class"));
		return nullptr;
	}

	// Initialize the editor manager once when widget is created
	// This ensures consistent state across multiple open/close cycles
	// Use PlayerController as outer for clearer ownership semantics
	if (!StationEditorWidget->EditorManager)
	{
		StationEditorWidget->EditorManager = NewObject<UStationEditorManager>(this);
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateStationEditorWidget: Successfully created station editor widget"));
	
	return StationEditorWidget;
}

void AAdastreaPlayerController::ShowStationEditor(ASpaceStation* Station)
{
	if (!Station)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShowStationEditor: Invalid station pointer"));
		return;
	}

	// Create widget if needed (also creates editor manager)
	if (!CreateStationEditorWidget())
	{
		return;
	}

	// Configure the editor manager
	if (StationEditorWidget->EditorManager)
	{
		// Assign the module catalog
		if (ModuleCatalog)
		{
			StationEditorWidget->EditorManager->ModuleCatalog = ModuleCatalog;
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("ShowStationEditor: No ModuleCatalog assigned. Module placement may not work correctly."));
		}

		// Begin editing the station
		if (!StationEditorWidget->EditorManager->BeginEditing(Station))
		{
			UE_LOG(LogAdastrea, Error, TEXT("ShowStationEditor: Failed to begin editing station"));
			return;
		}
	}

	// Set the station reference
	StationEditorWidget->SetStation(Station);

	// Add widget to viewport
	if (!StationEditorWidget->IsInViewport())
	{
		StationEditorWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsStationEditorOpen = true;
	
	UE_LOG(LogAdastrea, Log, TEXT("ShowStationEditor: Station editor now visible"));
}

void AAdastreaPlayerController::HideStationEditor()
{
	if (!StationEditorWidget)
	{
		bIsStationEditorOpen = false;
		return;
	}

	// End editing session if active
	if (StationEditorWidget->EditorManager && StationEditorWidget->EditorManager->bIsEditing)
	{
		// Save changes when closing (or could call Cancel() to discard)
		if (!StationEditorWidget->EditorManager->Save())
		{
			UE_LOG(LogAdastrea, Error, TEXT("HideStationEditor: Failed to save changes to station. Changes may be lost!"));
			// Note: Still proceed with closing the editor, but user is notified of the issue
		}
		else
		{
			UE_LOG(LogAdastrea, Log, TEXT("HideStationEditor: Successfully saved station changes"));
		}
	}

	// Remove widget from viewport
	if (StationEditorWidget->IsInViewport())
	{
		StationEditorWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsStationEditorOpen = false;
	
	UE_LOG(LogAdastrea, Log, TEXT("HideStationEditor: Station editor hidden"));
}

void AAdastreaPlayerController::ToggleShipStatus()
{
	// Only allow ship status when controlling a spaceship
	if (!IsControllingSpaceship())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ToggleShipStatus: Not controlling a spaceship - ship status not available"));
		return;
	}

	// Toggle ship status state
	if (bIsShipStatusOpen)
	{
		// Close the ship status screen
		HideShipStatus();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleShipStatus: Closed ship status screen"));
	}
	else
	{
		// Open the ship status screen
		ShowShipStatus();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleShipStatus: Opened ship status screen"));
	}
}

bool AAdastreaPlayerController::IsShipStatusOpen() const
{
	return bIsShipStatusOpen && ShipStatusWidget && ShipStatusWidget->IsInViewport();
}

UShipStatusWidget* AAdastreaPlayerController::GetShipStatusWidget() const
{
	return ShipStatusWidget;
}

UShipStatusWidget* AAdastreaPlayerController::CreateShipStatusWidget()
{
	// Return existing widget if already created
	if (ShipStatusWidget)
	{
		return ShipStatusWidget;
	}

	// Check if widget class is assigned
	if (!ShipStatusWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateShipStatusWidget: ShipStatusWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	ShipStatusWidget = CreateWidget<UShipStatusWidget>(this, ShipStatusWidgetClass);
	
	if (!ShipStatusWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateShipStatusWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateShipStatusWidget: Successfully created ship status widget"));
	
	return ShipStatusWidget;
}

void AAdastreaPlayerController::ShowShipStatus()
{
	// Create widget if needed
	if (!CreateShipStatusWidget())
	{
		return;
	}

	// Get the current spaceship
	ASpaceship* CurrentShip = GetControlledSpaceship();
	if (!CurrentShip)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShowShipStatus: No spaceship to display"));
		return;
	}

	// Initialize with current spaceship
	ShipStatusWidget->InitializeWithSpaceship(CurrentShip);

	// Add widget to viewport
	if (!ShipStatusWidget->IsInViewport())
	{
		ShipStatusWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsShipStatusOpen = true;
	
	UE_LOG(LogAdastrea, Log, TEXT("ShowShipStatus: Ship status screen now visible"));
}

void AAdastreaPlayerController::HideShipStatus()
{
	if (!ShipStatusWidget)
	{
		bIsShipStatusOpen = false;
		return;
	}

	// Remove widget from viewport
	if (ShipStatusWidget->IsInViewport())
	{
		ShipStatusWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsShipStatusOpen = false;
	
	UE_LOG(LogAdastrea, Log, TEXT("HideShipStatus: Ship status screen hidden"));
}
