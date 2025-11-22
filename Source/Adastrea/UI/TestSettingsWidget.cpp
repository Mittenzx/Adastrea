#include "UI/TestSettingsWidget.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"

UTestSettingsWidget::UTestSettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bShowMainMenuAfter(true)
	, SelectedShipType(TEXT("Default Fighter"))
	, SelectedDifficulty(TEXT("Normal"))
	, SelectedDebugMode(TEXT("None"))
	, bAutoSpawnShip(true)
	, bIsSettingsVisible(false)
{
	// Initialize default available options
	AvailableShipTypes = {
		TEXT("Default Fighter"),
		TEXT("Scout Ship"),
		TEXT("Cargo Hauler"),
		TEXT("Gunship"),
		TEXT("Explorer"),
		TEXT("Mining Vessel"),
		TEXT("Interceptor"),
		TEXT("Battlecruiser")
	};

	AvailableDifficulties = {
		TEXT("Very Easy"),
		TEXT("Easy"),
		TEXT("Normal"),
		TEXT("Hard"),
		TEXT("Very Hard")
	};

	AvailableDebugModes = {
		TEXT("None"),
		TEXT("Show FPS"),
		TEXT("Show Collision"),
		TEXT("Show Navigation"),
		TEXT("Show All Stats"),
		TEXT("God Mode"),
		TEXT("Infinite Resources")
	};
}

void UTestSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Initialize the settings when constructed
	InitializeTestSettings();
	
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Widget constructed"));
}

void UTestSettingsWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Widget destructed"));
}

void UTestSettingsWidget::InitializeTestSettings_Implementation()
{
	// Set widget as visible
	bIsSettingsVisible = true;
	
	// Show mouse cursor for menu interaction
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		
		// Set input mode to UI only
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	
	// Load default settings
	LoadDefaultSettings();
	
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Settings initialized"));
	UE_LOG(LogAdastrea, Log, TEXT("  - Ship Type: %s"), *SelectedShipType);
	UE_LOG(LogAdastrea, Log, TEXT("  - Difficulty: %s"), *SelectedDifficulty);
	UE_LOG(LogAdastrea, Log, TEXT("  - Debug Mode: %s"), *SelectedDebugMode);
	UE_LOG(LogAdastrea, Log, TEXT("  - Auto Spawn: %s"), bAutoSpawnShip ? TEXT("Yes") : TEXT("No"));
}

void UTestSettingsWidget::OnContinueClicked_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Continue clicked"));
	
	// Apply settings before continuing
	ApplySettings();
	
	// Transition to main menu or directly to game
	if (bShowMainMenuAfter)
	{
		TransitionToMainMenu();
	}
	else
	{
		// Remove widget and start game directly
		RemoveFromParent();
	}
}

void UTestSettingsWidget::OnResetClicked_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Reset clicked"));
	
	// Reset to default settings
	LoadDefaultSettings();
}

void UTestSettingsWidget::OnShipTypeChanged_Implementation(const FString& SelectedShip, int32 SelectionIndex)
{
	SelectedShipType = SelectedShip;
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Ship type changed to: %s (index: %d)"), *SelectedShip, SelectionIndex);
}

void UTestSettingsWidget::OnDifficultyChanged_Implementation(const FString& SelectedDiff, int32 SelectionIndex)
{
	SelectedDifficulty = SelectedDiff;
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Difficulty changed to: %s (index: %d)"), *SelectedDiff, SelectionIndex);
}

void UTestSettingsWidget::OnDebugModeChanged_Implementation(const FString& SelectedMode, int32 SelectionIndex)
{
	SelectedDebugMode = SelectedMode;
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Debug mode changed to: %s (index: %d)"), *SelectedMode, SelectionIndex);
	
	// Apply debug mode changes immediately if needed
	// This can be extended in Blueprint or by game instance
}

void UTestSettingsWidget::SetAutoSpawnEnabled(bool bEnabled)
{
	bAutoSpawnShip = bEnabled;
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Auto-spawn set to: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UTestSettingsWidget::TransitionToMainMenu_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Transitioning to main menu"));
	
	// Remove this widget from viewport
	RemoveFromParent();
	
	// Create and display main menu widget
	// This can be overridden in Blueprint to use a specific main menu widget class
	// For now, we just log the transition - Blueprint implementation will handle the actual widget creation
	
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Override this function in Blueprint to create main menu widget"));
}

void UTestSettingsWidget::ApplySettings_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Applying settings..."));
	
	// Store settings in a way that can be accessed by GameMode or PlayerController
	// This can be extended to store in GameInstance, SaveGame, or custom settings object
	
	// For now, we just log the settings - they can be read by other systems via getter functions
	UE_LOG(LogAdastrea, Log, TEXT("  Applied Settings:"));
	UE_LOG(LogAdastrea, Log, TEXT("    - Ship Type: %s"), *SelectedShipType);
	UE_LOG(LogAdastrea, Log, TEXT("    - Difficulty: %s"), *SelectedDifficulty);
	UE_LOG(LogAdastrea, Log, TEXT("    - Debug Mode: %s"), *SelectedDebugMode);
	UE_LOG(LogAdastrea, Log, TEXT("    - Auto Spawn: %s"), bAutoSpawnShip ? TEXT("Yes") : TEXT("No"));
	
	// Blueprint can override this to add custom application logic
}

void UTestSettingsWidget::LoadDefaultSettings_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Loading default settings"));
	
	// Set to default values from available options
	if (AvailableShipTypes.Num() > 0)
	{
		SelectedShipType = AvailableShipTypes[0];
	}
	
	// Find "Normal" difficulty or use middle option as fallback
	if (AvailableDifficulties.Num() > 0)
	{
		int32 NormalIndex = AvailableDifficulties.IndexOfByPredicate([](const FString& Difficulty) {
			return Difficulty.Contains(TEXT("Normal"));
		});
		
		if (NormalIndex != INDEX_NONE)
		{
			SelectedDifficulty = AvailableDifficulties[NormalIndex];
		}
		else if (AvailableDifficulties.Num() > 2)
		{
			SelectedDifficulty = AvailableDifficulties[2]; // Use middle option
		}
		else
		{
			SelectedDifficulty = AvailableDifficulties[0]; // Use first as fallback
		}
	}
	
	if (AvailableDebugModes.Num() > 0)
	{
		SelectedDebugMode = AvailableDebugModes[0]; // "None"
	}
	
	bAutoSpawnShip = true;
	
	UE_LOG(LogAdastrea, Log, TEXT("  Default settings loaded:"));
	UE_LOG(LogAdastrea, Log, TEXT("    - Ship Type: %s"), *SelectedShipType);
	UE_LOG(LogAdastrea, Log, TEXT("    - Difficulty: %s"), *SelectedDifficulty);
	UE_LOG(LogAdastrea, Log, TEXT("    - Debug Mode: %s"), *SelectedDebugMode);
}
