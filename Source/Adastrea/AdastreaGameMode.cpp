// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaGameMode.h"
#include "SpaceSectorMap.h"
#include "Ships/Spaceship.h"
#include "UI/TestSettingsWidget.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

AAdastreaGameMode::AAdastreaGameMode()
{
	// Set default pawn class to our Blueprinted character
	// This would typically be set to a blueprint class in the editor
	
	// Test settings configuration
	TestSettingsWidgetClass = nullptr; // Must be set in Blueprint or editor to enable
	bShowTestSettingsOnStartup = true; // Show if widget class is set
	
	// Default auto-spawn settings
	bAutoSpawnPlayerShip = true;
	bSpawnAtCenter = false; // Use random position by default for variety
	DefaultSpaceshipClass = nullptr; // Must be set in Blueprint or editor
	FallbackSpawnLocation = FVector::ZeroVector; // Default to world origin
	
	// Initialize widget reference
	TestSettingsWidget = nullptr;
}

void AAdastreaGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Show test settings widget if configured
	if (bShowTestSettingsOnStartup && TestSettingsWidgetClass)
	{
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Showing test settings widget on startup"));
		ShowTestSettingsWidget();
	}
	else
	{
		// No test settings to show - proceed with normal initialization
		OnTestSettingsContinue();
	}
}

void AAdastreaGameMode::SpawnPlayerSpaceship()
{
	// Validate we have a spaceship class to spawn
	if (!DefaultSpaceshipClass)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: Cannot auto-spawn player spaceship - DefaultSpaceshipClass is not set"));
		return;
	}

	// Find the SpaceSectorMap actor in the level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceSectorMap::StaticClass(), FoundActors);

	// Get spawn location - use sector if available, otherwise use fallback
	FVector SpawnLocation = FallbackSpawnLocation;
	
	if (FoundActors.Num() > 0)
	{
		// Use the first sector map found
		if (FoundActors.Num() > 1)
		{
			UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: Multiple SpaceSectorMap actors found (%d), using first one '%s'"), 
				FoundActors.Num(), *FoundActors[0]->GetName());
		}

		ASpaceSectorMap* SectorMap = Cast<ASpaceSectorMap>(FoundActors[0]);
		if (SectorMap)
		{
			// Get spawn location based on configuration
			if (bSpawnAtCenter)
			{
				SpawnLocation = SectorMap->GetSectorCenter();
				UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Spawning player spaceship at sector center: %s"), *SpawnLocation.ToString());
			}
			else
			{
				SpawnLocation = SectorMap->GetRandomPositionInSector();
				UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Spawning player spaceship at random position: %s"), *SpawnLocation.ToString());
			}
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: Failed to cast actor '%s' (class %s) to SpaceSectorMap, using fallback spawn location"), 
				*FoundActors[0]->GetName(), *FoundActors[0]->GetClass()->GetName());
		}
	}
	else
	{
		// No SpaceSectorMap found - use fallback spawn location
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: No SpaceSectorMap found in level, using fallback spawn location: %s"), *FallbackSpawnLocation.ToString());
	}

	// Set spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Name = FName(TEXT("PlayerSpaceship"));

	// Spawn the spaceship
	FRotator SpawnRotation = FRotator::ZeroRotator; // Facing forward
	ASpaceship* PlayerShip = GetWorld()->SpawnActor<ASpaceship>(DefaultSpaceshipClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (!PlayerShip)
	{
		UE_LOG(LogAdastrea, Error, TEXT("AdastreaGameMode: Failed to spawn player spaceship"));
		return;
	}

	UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Successfully spawned player spaceship at %s"), *SpawnLocation.ToString());

	// Get the player controller and possess the spaceship
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->Possess(PlayerShip);
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Player controller possessed spaceship"));
	}
	else
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: No player controller found to possess spaceship"));
	}
}

void AAdastreaGameMode::ShowTestSettingsWidget()
{
	if (!TestSettingsWidgetClass)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: TestSettingsWidgetClass is not set"));
		OnTestSettingsContinue(); // Proceed anyway
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogAdastrea, Error, TEXT("AdastreaGameMode: No player controller found to show test settings widget"));
		OnTestSettingsContinue(); // Proceed anyway
		return;
	}

	// Create the test settings widget (use UUserWidget as template type to support derived classes)
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(PC, TestSettingsWidgetClass);
	TestSettingsWidget = Cast<UTestSettingsWidget>(CreatedWidget);
	if (!TestSettingsWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("AdastreaGameMode: Failed to create test settings widget or cast to UTestSettingsWidget"));
		OnTestSettingsContinue(); // Proceed anyway
		return;
	}

	// Add to viewport with high Z-order to ensure it's on top
	static constexpr int32 TestSettingsWidgetZOrder = 100;
	TestSettingsWidget->AddToViewport(TestSettingsWidgetZOrder);
	
	UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Test settings widget created and added to viewport"));
}

void AAdastreaGameMode::OnTestSettingsContinue()
{
	UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Test settings confirmed, continuing with game initialization"));

	// Clean up test settings widget if it exists
	if (TestSettingsWidget)
	{
		TestSettingsWidget->RemoveFromParent();
		TestSettingsWidget = nullptr;
	}

	// Automatically spawn player spaceship if enabled and configured
	if (bAutoSpawnPlayerShip)
	{
		SpawnPlayerSpaceship();
	}
}
