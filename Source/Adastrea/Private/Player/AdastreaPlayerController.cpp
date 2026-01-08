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
#include "UI/InventoryWidget.h"
#include "UI/InventoryComponent.h"
#include "UI/TradingInterfaceWidget.h"
#include "UI/StationManagementWidget.h"
// REMOVED: #include "Factions/FactionDataAsset.h" - faction system removed per Trade Simulator MVP
// REMOVED: #include "Interfaces/IFactionMember.h" - faction system removed per Trade Simulator MVP
#include "TimerManager.h"

AAdastreaPlayerController::AAdastreaPlayerController()
{
	// Set default values
	StationEditorWidgetClass = nullptr;
	ModuleCatalog = nullptr;
	StationSearchRadius = 5000.0f;
	TradingInteractionRadius = 2000.0f;
	StationCheckInterval = 0.5f;
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
	InventoryWidgetClass = nullptr;
	InventoryWidget = nullptr;
	bIsInventoryOpen = false;
	TradingWidgetClass = nullptr;
	TradingWidget = nullptr;
	bIsTradingOpen = false;
	StationManagementWidgetClass = nullptr;
	StationManagementWidget = nullptr;
	bIsStationManagementOpen = false;
	NearbyTradableStation = nullptr;
	bWasNearTradableStation = false;
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

	// Start timer to check for nearby tradable stations
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			StationCheckTimerHandle,
			this,
			&AAdastreaPlayerController::CheckForNearbyTradableStations,
			StationCheckInterval,
			true  // Loop
		);
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaPlayerController: Started nearby station check timer"));
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

void AAdastreaPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Explicitly clear the station check timer to avoid dangling callbacks and keep timer usage maintainable
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StationCheckTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
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

	// Try to initialize as C++ widget (UStationEditorWidgetCpp) with InitializeEditor function
	UFunction* InitializeEditorFunc = StationEditorWidget->FindFunction(FName("InitializeEditor"));
	if (InitializeEditorFunc)
	{
		// Use ProcessEvent to call InitializeEditor(Station, ModuleCatalog)
		struct FInitializeEditorParams
		{
			ASpaceStation* Station;
			UDataAsset* Catalog;
		};
		
		FInitializeEditorParams Params;
		Params.Station = Station;
		Params.Catalog = ModuleCatalog;
		
		StationEditorWidget->ProcessEvent(InitializeEditorFunc, &Params);
		UE_LOG(LogAdastrea, Log, TEXT("ShowStationEditor: Called InitializeEditor on C++ widget"));
	}
	else
	{
		// Fallback to legacy Blueprint widget initialization
		// Initialize the widget using Blueprint-callable functions
		// The widget must implement SetStation() as a Blueprint function
		// This approach avoids circular dependency with StationEditor module
		
		// Use UFunction to call SetStation through reflection (Blueprint-safe)
		UFunction* SetStationFunc = StationEditorWidget->FindFunction(FName("SetStation"));
		if (SetStationFunc)
		{
			// Runtime validation: Ensure function signature matches struct
			if (SetStationFunc->NumParms != 1 || SetStationFunc->ParmsSize != sizeof(FSetStationParams))
			{
				UE_LOG(LogAdastrea, Error, TEXT("ShowStationEditor: SetStation function signature mismatch (expected 1 param, size %d; got %d params, size %d)"),
					sizeof(FSetStationParams), SetStationFunc->NumParms, SetStationFunc->ParmsSize);
				return;
			}
			
			FSetStationParams Params;
			Params.Station = Station;
			
			StationEditorWidget->ProcessEvent(SetStationFunc, &Params);
			UE_LOG(LogAdastrea, Log, TEXT("ShowStationEditor: Called SetStation on widget"));
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("ShowStationEditor: Widget does not have SetStation or InitializeEditor function. Configure in Blueprint."));
		}

		// Set ModuleCatalog through Blueprint property if available
		if (ModuleCatalog)
		{
			// Find and set the ModuleCatalog property if it exists
			if (FProperty* CatalogProp = StationEditorWidget->GetClass()->FindPropertyByName(FName("ModuleCatalog")))
			{
				// Verify it's an object property before setting (type safety)
				if (FObjectProperty* ObjProp = CastField<FObjectProperty>(CatalogProp))
				{
					// Use safe SetPropertyValue_InContainer instead of raw memory copy
					void* PropertyAddress = ObjProp->ContainerPtrToValuePtr<void>(StationEditorWidget);
					ObjProp->SetObjectPropertyValue(PropertyAddress, ModuleCatalog);
					UE_LOG(LogAdastrea, Log, TEXT("ShowStationEditor: Set ModuleCatalog on widget"));
				}
				else
				{
					UE_LOG(LogAdastrea, Warning, TEXT("ShowStationEditor: ModuleCatalog property is not an object property"));
				}
			}
		}
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

// ====================
// Inventory Widget Implementation
// ====================

void AAdastreaPlayerController::ToggleInventory()
{
	// Toggle inventory state
	if (bIsInventoryOpen)
	{
		// Close the inventory
		HideInventory();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleInventory: Closed inventory"));
	}
	else
	{
		// Open the inventory
		ShowInventory();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleInventory: Opened inventory"));
	}
}

bool AAdastreaPlayerController::IsInventoryOpen() const
{
	return bIsInventoryOpen && InventoryWidget && InventoryWidget->IsInViewport();
}

UInventoryWidget* AAdastreaPlayerController::GetInventoryWidget() const
{
	return InventoryWidget;
}

UInventoryWidget* AAdastreaPlayerController::CreateInventoryWidget()
{
	// Return existing widget if already created
	if (InventoryWidget)
	{
		return InventoryWidget;
	}

	// Check if widget class is assigned
	if (!InventoryWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateInventoryWidget: InventoryWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
	
	if (!InventoryWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateInventoryWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateInventoryWidget: Successfully created inventory widget"));
	
	return InventoryWidget;
}

void AAdastreaPlayerController::ShowInventory()
{
	// Create widget if needed
	if (!CreateInventoryWidget())
	{
		return;
	}

	// Get the player's inventory component
	APawn* ControlledPawn = GetPawn();
	UInventoryComponent* PlayerInventory = nullptr;
	
	if (ControlledPawn)
	{
		PlayerInventory = ControlledPawn->FindComponentByClass<UInventoryComponent>();
	}

	if (!PlayerInventory)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShowInventory: No inventory component found on controlled pawn"));
		// Still show the widget, but it won't have data
	}

	// Initialize with inventory component
	if (PlayerInventory)
	{
		InventoryWidget->InitializeInventory(PlayerInventory);
	}

	// Add widget to viewport
	if (!InventoryWidget->IsInViewport())
	{
		InventoryWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsInventoryOpen = true;
	
	UE_LOG(LogAdastrea, Log, TEXT("ShowInventory: Inventory now visible"));
}

void AAdastreaPlayerController::HideInventory()
{
	if (!InventoryWidget)
	{
		bIsInventoryOpen = false;
		return;
	}

	// Remove widget from viewport
	if (InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsInventoryOpen = false;
	
	UE_LOG(LogAdastrea, Log, TEXT("HideInventory: Inventory hidden"));
}

// ====================
// Trading Widget Implementation
// ====================

// REMOVED: OpenTrading(UFactionDataAsset*) - faction system removed per Trade Simulator MVP
// Use direct market access via trading widget OpenMarket() instead

void AAdastreaPlayerController::CloseTrading()
{
	HideTrading();
	UE_LOG(LogAdastrea, Log, TEXT("CloseTrading: Closed trading interface"));
}

bool AAdastreaPlayerController::IsTradingOpen() const
{
	return bIsTradingOpen && TradingWidget && TradingWidget->IsInViewport();
}

UTradingInterfaceWidget* AAdastreaPlayerController::GetTradingWidget() const
{
	return TradingWidget;
}

UTradingInterfaceWidget* AAdastreaPlayerController::CreateTradingWidget()
{
	// Return existing widget if already created
	if (TradingWidget)
	{
		return TradingWidget;
	}

	// Check if widget class is assigned
	if (!TradingWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateTradingWidget: TradingWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	TradingWidget = CreateWidget<UTradingInterfaceWidget>(this, TradingWidgetClass);
	
	if (!TradingWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateTradingWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateTradingWidget: Successfully created trading widget"));
	
	return TradingWidget;
}

void AAdastreaPlayerController::ShowTrading()
{
	if (!TradingWidget)
	{
		return;
	}

	// Add widget to viewport
	if (!TradingWidget->IsInViewport())
	{
		TradingWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsTradingOpen = true;
	
	UE_LOG(LogAdastrea, Log, TEXT("ShowTrading: Trading interface now visible"));
}

void AAdastreaPlayerController::HideTrading()
{
	if (!TradingWidget)
	{
		bIsTradingOpen = false;
		return;
	}

	// Remove widget from viewport
	if (TradingWidget->IsInViewport())
	{
		TradingWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsTradingOpen = false;
	
	UE_LOG(LogAdastrea, Log, TEXT("HideTrading: Trading interface hidden"));
}

// ====================
// Station Management Widget Implementation
// ====================

void AAdastreaPlayerController::OpenStationManagement(ASpaceStation* Station)
{
	if (!Station)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("OpenStationManagement: Invalid station"));
		return;
	}

	// Create widget if needed
	if (!CreateStationManagementWidget())
	{
		return;
	}

	// Initialize with station
	StationManagementWidget->SetManagedStation(Station);

	// Show the widget
	ShowStationManagement();
	
	UE_LOG(LogAdastrea, Log, TEXT("OpenStationManagement: Opened management for station: %s"), *Station->GetName());
}

void AAdastreaPlayerController::CloseStationManagement()
{
	HideStationManagement();
	UE_LOG(LogAdastrea, Log, TEXT("CloseStationManagement: Closed station management interface"));
}

bool AAdastreaPlayerController::IsStationManagementOpen() const
{
	return bIsStationManagementOpen && StationManagementWidget && StationManagementWidget->IsInViewport();
}

UStationManagementWidget* AAdastreaPlayerController::GetStationManagementWidget() const
{
	return StationManagementWidget;
}

UStationManagementWidget* AAdastreaPlayerController::CreateStationManagementWidget()
{
	// Return existing widget if already created
	if (StationManagementWidget)
	{
		return StationManagementWidget;
	}

	// Check if widget class is assigned
	if (!StationManagementWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationManagementWidget: StationManagementWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	StationManagementWidget = CreateWidget<UStationManagementWidget>(this, StationManagementWidgetClass);
	
	if (!StationManagementWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationManagementWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateStationManagementWidget: Successfully created station management widget"));
	
	return StationManagementWidget;
}

void AAdastreaPlayerController::ShowStationManagement()
{
	if (!StationManagementWidget)
	{
		return;
	}

	// Add widget to viewport
	if (!StationManagementWidget->IsInViewport())
	{
		StationManagementWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsStationManagementOpen = true;
	
	UE_LOG(LogAdastrea, Log, TEXT("ShowStationManagement: Station management interface now visible"));
}

void AAdastreaPlayerController::HideStationManagement()
{
	if (!StationManagementWidget)
	{
		bIsStationManagementOpen = false;
		return;
	}

	// Remove widget from viewport
	if (StationManagementWidget->IsInViewport())
	{
		StationManagementWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsStationManagementOpen = false;
	
	UE_LOG(LogAdastrea, Log, TEXT("HideStationManagement: Station management interface hidden"));
}

// ====================
// Trading Interaction Implementation
// ====================

void AAdastreaPlayerController::CheckForNearbyTradableStations()
{
	// Only check if controlling a spaceship
	if (!IsControllingSpaceship())
	{
		// Clear nearby station if we're not in a spaceship
		if (NearbyTradableStation != nullptr)
		{
			NearbyTradableStation = nullptr;
			bWasNearTradableStation = false;
			OnNearbyTradableStationChanged(false, nullptr);
		}
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	FVector PlayerLocation = ControlledPawn->GetActorLocation();
	
	// Find all space stations in the world
	TArray<AActor*> FoundStations;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceStation::StaticClass(), FoundStations);
	
	ASpaceStation* ClosestStation = nullptr;
	float ClosestDistance = TradingInteractionRadius;
	
	// Find the nearest station within trading interaction radius
	for (AActor* Actor : FoundStations)
	{
		ASpaceStation* Station = Cast<ASpaceStation>(Actor);
		if (!Station)
		{
			continue;
		}
		
		float Distance = FVector::Dist(PlayerLocation, Station->GetActorLocation());
		
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestStation = Station;
		}
	}

	// Check if the nearby station state changed
	bool bIsCurrentlyNear = (ClosestStation != nullptr);
	
	if (bIsCurrentlyNear != bWasNearTradableStation || ClosestStation != NearbyTradableStation)
	{
		// State changed - update and notify
		NearbyTradableStation = ClosestStation;
		bWasNearTradableStation = bIsCurrentlyNear;
		
		OnNearbyTradableStationChanged(bIsCurrentlyNear, ClosestStation);
		
		if (bIsCurrentlyNear)
		{
			UE_LOG(LogAdastrea, Log, TEXT("CheckForNearbyTradableStations: Now near station '%s' at distance %.1f"), 
				*ClosestStation->GetName(), ClosestDistance);
		}
		else
		{
			UE_LOG(LogAdastrea, Log, TEXT("CheckForNearbyTradableStations: Left station trading range"));
		}
	}
}

void AAdastreaPlayerController::AttemptTradeWithNearestStation()
{
	if (!IsControllingSpaceship())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AttemptTradeWithNearestStation: Not controlling a spaceship"));
		return;
	}

	// Get the nearest tradable station
	ASpaceStation* Station = GetNearestTradableStation();
	
	if (!Station)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AttemptTradeWithNearestStation: No station within trading range"));
		return;
	}

	// REMOVED: Faction-based trading logic - faction system removed per Trade Simulator MVP
	// TODO(MVP): Replace with direct market access
	// Should call TradingWidget->OpenMarket(StationMarketDataAsset) instead
	
	UE_LOG(LogAdastrea, Warning, TEXT("AttemptTradeWithNearestStation: Trading requires direct market reference. "
		"Update this function to pass UMarketDataAsset* instead of using faction."));
}

bool AAdastreaPlayerController::IsNearTradableStation() const
{
	return NearbyTradableStation != nullptr;
}

ASpaceStation* AAdastreaPlayerController::GetNearestTradableStation() const
{
	// Return the cached nearby station (updated by timer)
	return NearbyTradableStation;
}
