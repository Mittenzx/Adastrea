// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
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
	MainMenuWidgetClass = nullptr;
	MainMenuWidget = nullptr;
	bIsMainMenuOpen = false;
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

UUserWidget* AAdastreaPlayerController::GetStationEditorWidget() const
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

UUserWidget* AAdastreaPlayerController::CreateStationEditorWidget()
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

	// Create the widget as a generic UUserWidget to avoid StationEditor dependency
	StationEditorWidget = CreateWidget<UUserWidget>(this, StationEditorWidgetClass);
	
	if (!StationEditorWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationEditorWidget: Failed to create widget from class"));
		return nullptr;
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

	// Create widget if needed (no StationEditor-specific logic)
	if (!CreateStationEditorWidget())
	{
		return;
	}

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

void AAdastreaPlayerController::ToggleMainMenu()
{
	// Toggle main menu state
	if (bIsMainMenuOpen)
	{
		// Close the main menu
		HideMainMenu();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleMainMenu: Closed main menu"));
	}
	else
	{
		// Open the main menu
		ShowMainMenu();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleMainMenu: Opened main menu"));
	}
}

bool AAdastreaPlayerController::IsMainMenuOpen() const
{
	return bIsMainMenuOpen && MainMenuWidget && MainMenuWidget->IsInViewport();
}

UUserWidget* AAdastreaPlayerController::GetMainMenuWidget() const
{
	return MainMenuWidget;
}

UUserWidget* AAdastreaPlayerController::CreateMainMenuWidget()
{
	// Return existing widget if already created
	if (MainMenuWidget)
	{
		return MainMenuWidget;
	}

	// Check if widget class is assigned
	if (!MainMenuWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateMainMenuWidget: MainMenuWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
	
	if (!MainMenuWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateMainMenuWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateMainMenuWidget: Successfully created main menu widget"));
	
	return MainMenuWidget;
}

void AAdastreaPlayerController::ShowMainMenu()
{
	// Create widget if needed
	if (!CreateMainMenuWidget())
	{
		return;
	}

	// Add widget to viewport
	if (!MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->AddToViewport();
	}

	// Switch to UI input mode with focus on the menu
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	bIsMainMenuOpen = true;
	
	UE_LOG(LogAdastrea, Log, TEXT("ShowMainMenu: Main menu now visible"));
}

void AAdastreaPlayerController::HideMainMenu()
{
	if (!MainMenuWidget)
	{
		// No widget to hide, but ensure state is consistent
		if (bIsMainMenuOpen)
		{
			UE_LOG(LogAdastrea, Warning, TEXT("HideMainMenu: Menu marked as open but widget is null - resetting state"));
		}
		bIsMainMenuOpen = false;
		return;
	}

	// Remove widget from viewport
	if (MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsMainMenuOpen = false;
	
	UE_LOG(LogAdastrea, Log, TEXT("HideMainMenu: Main menu hidden"));
}
