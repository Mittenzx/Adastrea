#include "UI/TestingDashboardWidget.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UTestingDashboardWidget::UTestingDashboardWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsDashboardVisible(false)
	, bPauseGameWhenOpen(false)
	, bShowOnlyUncompleted(false)
	, CurrentFilterCategory(TEXT(""))
	, TestResultsPath(TEXT("Saved/TestResults/TestingProgress.txt"))
{
	// Initialize default ship types
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

	// Initialize debug modes
	AvailableDebugModes = {
		TEXT("None"),
		TEXT("Show FPS"),
		TEXT("Show Collision"),
		TEXT("Show Navigation"),
		TEXT("Show All Stats"),
		TEXT("God Mode"),
		TEXT("Infinite Resources"),
		TEXT("Show AI Debug"),
		TEXT("Show Physics"),
		TEXT("Show Networking")
	};
}

void UTestingDashboardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	InitializeDashboard();
	
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Dashboard constructed"));
}

void UTestingDashboardWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	// Save progress before destruction
	SaveProgress();
	
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Dashboard destructed"));
}

void UTestingDashboardWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Dashboard-specific tick logic can go here
	// For example, updating real-time stats
}

void UTestingDashboardWidget::InitializeDashboard_Implementation()
{
	// Load default testable features
	LoadDefaultFeatures();
	
	// Load saved progress if available
	LoadProgress();
	
	// Initially hide the dashboard
	bIsDashboardVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
	
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Dashboard initialized with %d features"), TestableFeatures.Num());
}

void UTestingDashboardWidget::ToggleDashboard(bool bVisible)
{
	bIsDashboardVisible = bVisible;
	
	if (bVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
		
		// Show mouse cursor
		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->bShowMouseCursor = true;
			PC->bEnableClickEvents = true;
			PC->bEnableMouseOverEvents = true;
			
			// Optionally pause game
			if (bPauseGameWhenOpen)
			{
				UGameplayStatics::SetGamePaused(this, true);
			}
		}
		
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Dashboard opened"));
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
		
		// Restore game input
		if (APlayerController* PC = GetOwningPlayer())
		{
			// Don't hide cursor if user wants it visible
			// PC->bShowMouseCursor = false;
			
			// Resume game if it was paused
			if (bPauseGameWhenOpen)
			{
				UGameplayStatics::SetGamePaused(this, false);
			}
		}
		
		// Save progress when closing
		SaveProgress();
		
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Dashboard closed"));
	}
}

void UTestingDashboardWidget::SetFeatureTested(int32 FeatureIndex, bool bTested)
{
	if (TestableFeatures.IsValidIndex(FeatureIndex))
	{
		TestableFeatures[FeatureIndex].bIsTested = bTested;
		
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Feature '%s' marked as %s"),
			*TestableFeatures[FeatureIndex].FeatureName.ToString(),
			bTested ? TEXT("TESTED") : TEXT("NOT TESTED"));
		
		// Auto-save progress
		SaveProgress();
	}
	else
	{
		UE_LOG(LogAdastrea, Warning, TEXT("TestingDashboardWidget: Invalid feature index %d"), FeatureIndex);
	}
}

void UTestingDashboardWidget::AddFeatureNotes(int32 FeatureIndex, const FString& Notes)
{
	if (TestableFeatures.IsValidIndex(FeatureIndex))
	{
		TestableFeatures[FeatureIndex].TestNotes = Notes;
		
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Added notes to feature '%s': %s"),
			*TestableFeatures[FeatureIndex].FeatureName.ToString(),
			*Notes);
		
		// Auto-save progress
		SaveProgress();
	}
}

TArray<FTestableFeature> UTestingDashboardWidget::GetFeaturesByCategory(const FString& Category) const
{
	TArray<FTestableFeature> FilteredFeatures;
	
	for (const FTestableFeature& Feature : TestableFeatures)
	{
		if (Category.IsEmpty() || Feature.Category == Category)
		{
			FilteredFeatures.Add(Feature);
		}
	}
	
	return FilteredFeatures;
}

float UTestingDashboardWidget::GetTestingProgress() const
{
	if (TestableFeatures.Num() == 0)
	{
		return 0.0f;
	}
	
	int32 TestedCount = 0;
	for (const FTestableFeature& Feature : TestableFeatures)
	{
		if (Feature.bIsTested)
		{
			TestedCount++;
		}
	}
	
	return (static_cast<float>(TestedCount) / TestableFeatures.Num()) * 100.0f;
}

void UTestingDashboardWidget::ResetAllProgress()
{
	for (FTestableFeature& Feature : TestableFeatures)
	{
		Feature.bIsTested = false;
		Feature.TestNotes = TEXT("");
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: All testing progress reset"));
	
	SaveProgress();
}

void UTestingDashboardWidget::SpawnTestShip_Implementation(const FString& ShipType, float SpawnDistance)
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Spawning test ship - Type: %s, Distance: %.2f"),
		*ShipType, SpawnDistance);
	
	// This is a placeholder - Blueprint can override to implement actual spawning
	// Or it can call GameMode's spawn functions
	
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->GetPawn())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("TestingDashboardWidget: Cannot spawn ship - no player pawn"));
		return;
	}
	
	// Get spawn location in front of player
	FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
	FVector PlayerForward = PC->GetPawn()->GetActorForwardVector();
	FVector SpawnLocation = PlayerLocation + (PlayerForward * SpawnDistance);
	
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Would spawn at location: %s"), *SpawnLocation.ToString());
	
	// Blueprint implementation should handle actual spawning
}

void UTestingDashboardWidget::EnableDebugMode_Implementation(const FString& DebugMode)
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Enabling debug mode: %s"), *DebugMode);
	
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}
	
	// Apply debug modes via console commands
	if (DebugMode == TEXT("Show FPS"))
	{
		PC->ConsoleCommand(TEXT("stat fps"));
	}
	else if (DebugMode == TEXT("Show Collision"))
	{
		PC->ConsoleCommand(TEXT("show collision"));
	}
	else if (DebugMode == TEXT("Show Navigation"))
	{
		PC->ConsoleCommand(TEXT("show navigation"));
	}
	else if (DebugMode == TEXT("Show All Stats"))
	{
		PC->ConsoleCommand(TEXT("stat unit"));
		PC->ConsoleCommand(TEXT("stat fps"));
	}
	else if (DebugMode == TEXT("God Mode"))
	{
		PC->ConsoleCommand(TEXT("god"));
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: God Mode toggled"));
	}
	else if (DebugMode == TEXT("Infinite Resources"))
	{
		// This would need to be implemented per-game
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Infinite Resources - implement in Blueprint"));
	}
	else if (DebugMode == TEXT("Show AI Debug"))
	{
		PC->ConsoleCommand(TEXT("showdebug ai"));
	}
	else if (DebugMode == TEXT("Show Physics"))
	{
		PC->ConsoleCommand(TEXT("show physics"));
	}
	else if (DebugMode == TEXT("Show Networking"))
	{
		PC->ConsoleCommand(TEXT("stat net"));
	}
	else if (DebugMode == TEXT("None"))
	{
		// Disable all debug modes
		PC->ConsoleCommand(TEXT("stat none"));
		PC->ConsoleCommand(TEXT("show collision off"));
		PC->ConsoleCommand(TEXT("show navigation off"));
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: All debug modes disabled"));
	}
}

void UTestingDashboardWidget::SetDifficulty_Implementation(const FString& DifficultyLevel)
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Setting difficulty to: %s"), *DifficultyLevel);
	
	// This would typically interact with game instance or game mode
	// Blueprint can override to implement actual difficulty changes
	
	// For now, just log it
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Difficulty change - implement in GameMode or Blueprint"));
}

void UTestingDashboardWidget::ExecuteTestCommand_Implementation(const FString& Command)
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Executing test command: %s"), *Command);
	
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->ConsoleCommand(Command);
	}
}

void UTestingDashboardWidget::ExportTestResults(const FString& FilePath)
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Exporting test results to: %s"), *FilePath);
	
	FString ResultText = TEXT("=== Adastrea Testing Results ===\n\n");
	ResultText += FString::Printf(TEXT("Total Features: %d\n"), TestableFeatures.Num());
	ResultText += FString::Printf(TEXT("Tested: %d\n"), 
		TestableFeatures.FilterByPredicate([](const FTestableFeature& F) { return F.bIsTested; }).Num());
	ResultText += FString::Printf(TEXT("Progress: %.1f%%\n\n"), GetTestingProgress());
	
	// Group by category
	TMap<FString, TArray<FTestableFeature>> FeaturesByCategory;
	for (const FTestableFeature& Feature : TestableFeatures)
	{
		FeaturesByCategory.FindOrAdd(Feature.Category).Add(Feature);
	}
	
	// Export each category
	for (const auto& CategoryPair : FeaturesByCategory)
	{
		ResultText += FString::Printf(TEXT("=== %s ===\n"), *CategoryPair.Key);
		
		for (const FTestableFeature& Feature : CategoryPair.Value)
		{
			ResultText += FString::Printf(TEXT("[%s] %s\n"),
				Feature.bIsTested ? TEXT("✓") : TEXT(" "),
				*Feature.FeatureName.ToString());
			
			if (!Feature.TestNotes.IsEmpty())
			{
				ResultText += FString::Printf(TEXT("    Notes: %s\n"), *Feature.TestNotes);
			}
		}
		
		ResultText += TEXT("\n");
	}
	
	// Save to file
	FString FullPath = FPaths::ProjectSavedDir() / FilePath;
	if (FFileHelper::SaveStringToFile(ResultText, *FullPath))
	{
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Results exported successfully to: %s"), *FullPath);
	}
	else
	{
		UE_LOG(LogAdastrea, Error, TEXT("TestingDashboardWidget: Failed to export results to: %s"), *FullPath);
	}
}

void UTestingDashboardWidget::LoadDefaultFeatures_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Loading default testable features"));
	
	TestableFeatures.Empty();
	
	// Core Systems
	FTestableFeature Feature;
	
	Feature.FeatureName = FText::FromString(TEXT("Ship Movement"));
	Feature.Description = FText::FromString(TEXT("Test forward, backward, strafe, and rotation"));
	Feature.Category = TEXT("Core");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Ship Collision"));
	Feature.Description = FText::FromString(TEXT("Test collisions with objects, stations, and other ships"));
	Feature.Category = TEXT("Core");
	TestableFeatures.Add(Feature);
	
	// Combat Systems
	Feature.FeatureName = FText::FromString(TEXT("Weapon Firing"));
	Feature.Description = FText::FromString(TEXT("Test all weapon types - energy, projectile, missiles"));
	Feature.Category = TEXT("Combat");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Targeting System"));
	Feature.Description = FText::FromString(TEXT("Test target lock, switching, and tracking"));
	Feature.Category = TEXT("Combat");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Shields & Damage"));
	Feature.Description = FText::FromString(TEXT("Test shield absorption, recharge, and hull damage"));
	Feature.Category = TEXT("Combat");
	TestableFeatures.Add(Feature);
	
	// Navigation Systems
	Feature.FeatureName = FText::FromString(TEXT("Autopilot"));
	Feature.Description = FText::FromString(TEXT("Test autopilot to waypoints and stations"));
	Feature.Category = TEXT("Navigation");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Sector Map"));
	Feature.Description = FText::FromString(TEXT("Test map display, zoom, and navigation"));
	Feature.Category = TEXT("Navigation");
	TestableFeatures.Add(Feature);
	
	// Station Systems
	Feature.FeatureName = FText::FromString(TEXT("Docking"));
	Feature.Description = FText::FromString(TEXT("Test docking request, approach, and completion"));
	Feature.Category = TEXT("Stations");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Trading"));
	Feature.Description = FText::FromString(TEXT("Test buying, selling, and market prices"));
	Feature.Category = TEXT("Trading");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Station Services"));
	Feature.Description = FText::FromString(TEXT("Test repairs, refueling, and ship upgrades"));
	Feature.Category = TEXT("Stations");
	TestableFeatures.Add(Feature);
	
	// Faction Systems
	Feature.FeatureName = FText::FromString(TEXT("Faction Reputation"));
	Feature.Description = FText::FromString(TEXT("Test reputation changes from actions"));
	Feature.Category = TEXT("Factions");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Faction Diplomacy"));
	Feature.Description = FText::FromString(TEXT("Test faction relationships and alliances"));
	Feature.Category = TEXT("Factions");
	TestableFeatures.Add(Feature);
	
	// AI Systems
	Feature.FeatureName = FText::FromString(TEXT("NPC Ships"));
	Feature.Description = FText::FromString(TEXT("Test NPC ship behavior and pathfinding"));
	Feature.Category = TEXT("AI");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("AI Combat"));
	Feature.Description = FText::FromString(TEXT("Test AI targeting, tactics, and flee behavior"));
	Feature.Category = TEXT("AI");
	TestableFeatures.Add(Feature);
	
	// UI Systems
	Feature.FeatureName = FText::FromString(TEXT("HUD Display"));
	Feature.Description = FText::FromString(TEXT("Test HUD elements - health, shields, speed, etc."));
	Feature.Category = TEXT("UI");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Menus & Settings"));
	Feature.Description = FText::FromString(TEXT("Test all menus, settings, and options"));
	Feature.Category = TEXT("UI");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Inventory System"));
	Feature.Description = FText::FromString(TEXT("Test inventory management and item usage"));
	Feature.Category = TEXT("UI");
	TestableFeatures.Add(Feature);
	
	// Performance
	Feature.FeatureName = FText::FromString(TEXT("Frame Rate"));
	Feature.Description = FText::FromString(TEXT("Check FPS in various scenarios"));
	Feature.Category = TEXT("Performance");
	TestableFeatures.Add(Feature);
	
	Feature.FeatureName = FText::FromString(TEXT("Load Times"));
	Feature.Description = FText::FromString(TEXT("Test level loading and transition times"));
	Feature.Category = TEXT("Performance");
	TestableFeatures.Add(Feature);
	
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Loaded %d default features"), TestableFeatures.Num());
}

void UTestingDashboardWidget::ApplyFilters_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Applying filters - Category: '%s', Show Only Uncompleted: %s"),
		*CurrentFilterCategory,
		bShowOnlyUncompleted ? TEXT("Yes") : TEXT("No"));
	
	// Blueprint should implement UI filtering logic
	// This is just for logging
}

void UTestingDashboardWidget::SaveProgress_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Saving testing progress"));
	
	// Simple text-based save for now
	// Blueprint can override to use SaveGame system
	
	FString SaveData = TEXT("");
	for (int32 i = 0; i < TestableFeatures.Num(); i++)
	{
		const FTestableFeature& Feature = TestableFeatures[i];
		SaveData += FString::Printf(TEXT("%d|%s|%d|%s\n"),
			i,
			*Feature.FeatureName.ToString(),
			Feature.bIsTested ? 1 : 0,
			*Feature.TestNotes);
	}
	
	FString FullPath = FPaths::ProjectSavedDir() / TEXT("TestingProgress.txt");
	if (FFileHelper::SaveStringToFile(SaveData, *FullPath))
	{
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Progress saved to: %s"), *FullPath);
	}
}

void UTestingDashboardWidget::LoadProgress_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Loading testing progress"));
	
	FString FullPath = FPaths::ProjectSavedDir() / TEXT("TestingProgress.txt");
	FString LoadData;
	
	if (FFileHelper::LoadFileToString(LoadData, *FullPath))
	{
		TArray<FString> Lines;
		LoadData.ParseIntoArrayLines(Lines);
		
		for (const FString& Line : Lines)
		{
			TArray<FString> Parts;
			Line.ParseIntoArray(Parts, TEXT("|"));
			
			if (Parts.Num() >= 3)
			{
				int32 Index = FCString::Atoi(*Parts[0]);
				bool bTested = FCString::Atoi(*Parts[2]) != 0;
				FString Notes = Parts.Num() > 3 ? Parts[3] : TEXT("");
				
				if (TestableFeatures.IsValidIndex(Index))
				{
					TestableFeatures[Index].bIsTested = bTested;
					TestableFeatures[Index].TestNotes = Notes;
				}
			}
		}
		
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: Progress loaded from: %s"), *FullPath);
	}
	else
	{
		UE_LOG(LogAdastrea, Log, TEXT("TestingDashboardWidget: No saved progress found, starting fresh"));
	}
}
