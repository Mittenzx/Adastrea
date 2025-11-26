// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestGameMode.h"
#include "Public/Ships/SpaceshipDataAsset.h"
#include "Public/Input/InputConfigDataAsset.h"
#include "Ships/Spaceship.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ATestGameMode::ATestGameMode()
{
	// Menu configuration defaults
	MenuWidgetClass = nullptr;
	bShowMenuOnStartup = true;
	MenuWidgetZOrder = 100;
	
	// Spaceship defaults
	DefaultSpaceshipIndex = 0;
	SelectedSpaceship = nullptr;
	
	// Input config defaults
	DefaultInputConfigIndex = 0;
	SelectedInputConfig = nullptr;
	
	// Internal state
	MenuWidgetInstance = nullptr;
	bMenuDisplayed = false;
	
	// Override parent's auto-spawn behavior - we handle spawning after menu confirmation
	bAutoSpawnPlayerShip = false;
}

void ATestGameMode::BeginPlay()
{
	// Call parent's BeginPlay - note that we set bAutoSpawnPlayerShip = false in constructor
	// so the parent won't auto-spawn a ship. We handle spawning after menu confirmation.
	Super::BeginPlay();
	
	// Apply defaults in case we skip the menu
	ApplyDefaults();
	
	// Show configuration menu if enabled and widget class is set
	if (bShowMenuOnStartup && MenuWidgetClass)
	{
		UE_LOG(LogAdastrea, Log, TEXT("TestGameMode: Showing configuration menu on startup"));
		ShowConfigurationMenu();
	}
	else
	{
		// No menu to show - proceed with defaults
		UE_LOG(LogAdastrea, Log, TEXT("TestGameMode: No menu configured, proceeding with default selections"));
		OnGameInitialize();
	}
}

void ATestGameMode::ApplyDefaults()
{
	// Apply default spaceship selection
	if (AvailableSpaceships.Num() > 0 && DefaultSpaceshipIndex >= 0)
	{
		const int32 ClampedIndex = FMath::Clamp(DefaultSpaceshipIndex, 0, AvailableSpaceships.Num() - 1);
		SelectedSpaceship = AvailableSpaceships[ClampedIndex];
		
		if (SelectedSpaceship)
		{
			UE_LOG(LogAdastreaShips, Log, TEXT("TestGameMode: Default spaceship set to '%s'"), 
				*SelectedSpaceship->ShipName.ToString());
		}
	}
	
	// Apply default input config selection
	if (AvailableInputConfigs.Num() > 0 && DefaultInputConfigIndex >= 0)
	{
		const int32 ClampedIndex = FMath::Clamp(DefaultInputConfigIndex, 0, AvailableInputConfigs.Num() - 1);
		SelectedInputConfig = AvailableInputConfigs[ClampedIndex];
		
		if (SelectedInputConfig)
		{
			UE_LOG(LogAdastreaInput, Log, TEXT("TestGameMode: Default input config set"));
		}
	}
}

bool ATestGameMode::SelectSpaceship_Implementation(int32 SpaceshipIndex)
{
	// Validate index
	if (SpaceshipIndex < 0 || SpaceshipIndex >= AvailableSpaceships.Num())
	{
		UE_LOG(LogAdastreaShips, Warning, TEXT("TestGameMode: Invalid spaceship index %d (available: %d)"), 
			SpaceshipIndex, AvailableSpaceships.Num());
		return false;
	}
	
	// Get the spaceship data asset
	USpaceshipDataAsset* NewSelection = AvailableSpaceships[SpaceshipIndex];
	if (!NewSelection)
	{
		UE_LOG(LogAdastreaShips, Warning, TEXT("TestGameMode: Spaceship at index %d is null"), SpaceshipIndex);
		return false;
	}
	
	// Apply selection
	SelectedSpaceship = NewSelection;
	UE_LOG(LogAdastreaShips, Log, TEXT("TestGameMode: Selected spaceship '%s'"), 
		*SelectedSpaceship->ShipName.ToString());
	
	return true;
}

bool ATestGameMode::SelectInputConfig_Implementation(int32 InputConfigIndex)
{
	// Validate index
	if (InputConfigIndex < 0 || InputConfigIndex >= AvailableInputConfigs.Num())
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("TestGameMode: Invalid input config index %d (available: %d)"), 
			InputConfigIndex, AvailableInputConfigs.Num());
		return false;
	}
	
	// Get the input config data asset
	UInputConfigDataAsset* NewSelection = AvailableInputConfigs[InputConfigIndex];
	if (!NewSelection)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("TestGameMode: Input config at index %d is null"), InputConfigIndex);
		return false;
	}
	
	// Apply selection
	SelectedInputConfig = NewSelection;
	UE_LOG(LogAdastreaInput, Log, TEXT("TestGameMode: Selected input configuration"));
	
	return true;
}

void ATestGameMode::OnMenuConfirm_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestGameMode: Menu confirmed, validating selections"));
	
	// Validate selections
	if (!AreSelectionsValid())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("TestGameMode: Selections are not valid, applying defaults"));
		ApplyDefaults();
	}
	
	// Hide the menu
	HideConfigurationMenu();
	
	// Proceed with game initialization
	OnGameInitialize();
}

void ATestGameMode::OnMenuCancel_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestGameMode: Menu cancelled"));
	
	// Hide the menu
	HideConfigurationMenu();
	
	// Default behavior: could quit game or go to main menu
	// For now, just proceed with defaults
	ApplyDefaults();
	OnGameInitialize();
}

TArray<FText> ATestGameMode::GetAvailableSpaceshipNames() const
{
	TArray<FText> Names;
	Names.Reserve(AvailableSpaceships.Num());
	
	for (const USpaceshipDataAsset* Ship : AvailableSpaceships)
	{
		if (Ship)
		{
			Names.Add(Ship->ShipName);
		}
		else
		{
			Names.Add(FText::FromString(TEXT("<Invalid>")));
		}
	}
	
	return Names;
}

TArray<FText> ATestGameMode::GetAvailableInputConfigNames() const
{
	TArray<FText> Names;
	Names.Reserve(AvailableInputConfigs.Num());
	
	for (int32 i = 0; i < AvailableInputConfigs.Num(); ++i)
	{
		const UInputConfigDataAsset* Config = AvailableInputConfigs[i];
		if (Config)
		{
			// Use a descriptive name - InputConfigDataAsset doesn't have a display name,
			// so we generate one based on index or use the asset name
			FString AssetName = Config->GetName();
			Names.Add(FText::FromString(AssetName));
		}
		else
		{
			Names.Add(FText::FromString(TEXT("<Invalid>")));
		}
	}
	
	return Names;
}

int32 ATestGameMode::GetSelectedSpaceshipIndex() const
{
	if (!SelectedSpaceship)
	{
		return INDEX_NONE;
	}
	
	return AvailableSpaceships.IndexOfByKey(SelectedSpaceship);
}

int32 ATestGameMode::GetSelectedInputConfigIndex() const
{
	if (!SelectedInputConfig)
	{
		return INDEX_NONE;
	}
	
	return AvailableInputConfigs.IndexOfByKey(SelectedInputConfig);
}

bool ATestGameMode::AreSelectionsValid() const
{
	// Check spaceship selection
	if (!SelectedSpaceship)
	{
		return false;
	}
	
	// Input config is optional - some setups may use default input
	// So we only require a valid spaceship
	return true;
}

void ATestGameMode::ShowConfigurationMenu_Implementation()
{
	if (!MenuWidgetClass)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("TestGameMode: MenuWidgetClass is not set"));
		OnGameInitialize(); // Proceed without menu
		return;
	}
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogAdastrea, Error, TEXT("TestGameMode: No player controller found to show menu"));
		OnGameInitialize(); // Proceed without menu
		return;
	}
	
	// Create the menu widget
	MenuWidgetInstance = CreateWidget<UUserWidget>(PC, MenuWidgetClass);
	if (!MenuWidgetInstance)
	{
		UE_LOG(LogAdastrea, Error, TEXT("TestGameMode: Failed to create menu widget"));
		OnGameInitialize(); // Proceed without menu
		return;
	}
	
	// Add to viewport with configurable Z-order
	MenuWidgetInstance->AddToViewport(MenuWidgetZOrder);
	bMenuDisplayed = true;
	
	// Set input mode to UI only
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MenuWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
	
	UE_LOG(LogAdastrea, Log, TEXT("TestGameMode: Configuration menu displayed"));
}

void ATestGameMode::HideConfigurationMenu_Implementation()
{
	if (MenuWidgetInstance)
	{
		MenuWidgetInstance->RemoveFromParent();
		MenuWidgetInstance = nullptr;
	}
	
	bMenuDisplayed = false;
	
	// Restore input mode to game
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("TestGameMode: Configuration menu hidden"));
}

void ATestGameMode::ApplyInputConfiguration_Implementation()
{
	if (!SelectedInputConfig)
	{
		UE_LOG(LogAdastreaInput, Log, TEXT("TestGameMode: No input configuration selected, using defaults"));
		return;
	}
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("TestGameMode: No player controller found to apply input config"));
		return;
	}
	
	// Get the Enhanced Input Local Player Subsystem
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("TestGameMode: No local player found"));
		return;
	}
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		UE_LOG(LogAdastreaInput, Warning, TEXT("TestGameMode: Enhanced Input subsystem not available"));
		return;
	}
	
	// Clear existing mapping contexts first
	InputSubsystem->ClearAllMappings();
	
	// Add spaceship mapping context if available
	UInputMappingContext* SpaceshipContext = SelectedInputConfig->GetSpaceshipMappingContext();
	if (SpaceshipContext)
	{
		InputSubsystem->AddMappingContext(SpaceshipContext, SelectedInputConfig->SpaceshipContextPriority);
		UE_LOG(LogAdastreaInput, Log, TEXT("TestGameMode: Added spaceship mapping context with priority %d"), 
			SelectedInputConfig->SpaceshipContextPriority);
	}
	
	// Add menu mapping context if available
	UInputMappingContext* MenuContext = SelectedInputConfig->GetMenuMappingContext();
	if (MenuContext)
	{
		InputSubsystem->AddMappingContext(MenuContext, SelectedInputConfig->MenuContextPriority);
		UE_LOG(LogAdastreaInput, Log, TEXT("TestGameMode: Added menu mapping context with priority %d"), 
			SelectedInputConfig->MenuContextPriority);
	}
	
	UE_LOG(LogAdastreaInput, Log, TEXT("TestGameMode: Input configuration applied successfully"));
}

void ATestGameMode::SpawnSelectedSpaceship_Implementation()
{
	// Validate we have a selected spaceship
	if (!SelectedSpaceship)
	{
		UE_LOG(LogAdastreaShips, Warning, TEXT("TestGameMode: No spaceship selected, cannot spawn"));
		return;
	}
	
	// Validate we have a default spaceship class to spawn
	if (!DefaultSpaceshipClass)
	{
		UE_LOG(LogAdastreaShips, Warning, TEXT("TestGameMode: DefaultSpaceshipClass is not set, cannot spawn"));
		return;
	}
	
	UE_LOG(LogAdastreaShips, Log, TEXT("TestGameMode: Spawning spaceship '%s'"), 
		*SelectedSpaceship->ShipName.ToString());
	
	// Use parent's spawn logic which handles sector/fallback positioning
	SpawnPlayerSpaceship();
	
	// Configure the spawned ship with selected data asset
	// Find the player's possessed pawn (should be the newly spawned ship)
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		ASpaceship* PlayerShip = Cast<ASpaceship>(PC->GetPawn());
		if (PlayerShip)
		{
			// Apply the selected data asset to the ship
			PlayerShip->ShipDataAsset = SelectedSpaceship;
			
			// Update ship properties from data asset
			PlayerShip->MaxHullIntegrity = SelectedSpaceship->HullStrength;
			PlayerShip->CurrentHullIntegrity = SelectedSpaceship->HullStrength;
			
			// Update movement properties from data asset
			PlayerShip->DefaultMaxSpeed = SelectedSpaceship->MaxSpeed;
			PlayerShip->DefaultAcceleration = SelectedSpaceship->Acceleration;
			
			UE_LOG(LogAdastreaShips, Log, TEXT("TestGameMode: Applied data asset '%s' to spawned ship"), 
				*SelectedSpaceship->ShipName.ToString());
		}
	}
}

void ATestGameMode::OnGameInitialize_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestGameMode: Initializing game with selected configuration"));
	
	// Apply input configuration first
	ApplyInputConfiguration();
	
	// Spawn the selected spaceship
	SpawnSelectedSpaceship();
	
	UE_LOG(LogAdastrea, Log, TEXT("TestGameMode: Game initialization complete"));
}
