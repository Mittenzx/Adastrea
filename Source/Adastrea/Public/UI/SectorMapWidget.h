// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SectorMapWidget.generated.h"

class ASpaceSectorMap;

/**
 * Sector information structure for display
 * Contains data about a sector that can be displayed in the UI
 */
USTRUCT(BlueprintType)
struct FSectorDisplayInfo
{
	GENERATED_BODY()

	/** Name of the sector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector Info")
	FText SectorName;

	/** Description of the sector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector Info")
	FText Description;

	/** Center position of the sector in world space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector Info")
	FVector SectorCenter;

	/** Size of the sector in Unreal Units (centimeters) - 200km = 20,000,000 cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector Info")
	float SectorSize;

	/** Number of objects in the sector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector Info")
	int32 ObjectCount;

	FSectorDisplayInfo()
		: SectorName(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, SectorCenter(FVector::ZeroVector)
		, SectorSize(0.0f)
		, ObjectCount(0)
	{
	}
};

/**
 * Sector Map Widget for displaying sector information
 * 
 * This widget provides a user interface for viewing space sector information:
 * - Sector name and description
 * - Sector boundaries and size (200km x 200km = 20,000,000 x 20,000,000 Unreal Units)
 * - Objects within the sector (stations, ships, asteroids)
 * - Navigation and selection functionality
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class (e.g., WBP_SectorMap)
 * - Design the sector map layout in the Blueprint designer
 * - Override BlueprintNativeEvents to customize behavior
 * - Use Blueprint functions to update sector display
 * - Bind to the sector selection delegate for navigation
 * 
 * Integration:
 * - Works with ASpaceSectorMap actors for sector data
 * - Can be integrated into the main HUD or opened as a separate screen
 * - Supports procedural generation workflows
 * - Designer-friendly with full Blueprint exposure
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API USectorMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USectorMapWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the sector map widget
	 * Called when the widget is first created and added to viewport
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sector Map")
	void InitializeSectorMap();
	virtual void InitializeSectorMap_Implementation();

	/**
	 * Update the sector information display
	 * @param SectorInfo The sector information to display
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sector Map")
	void UpdateSectorInfo(const FSectorDisplayInfo& SectorInfo);
	virtual void UpdateSectorInfo_Implementation(const FSectorDisplayInfo& SectorInfo);

	/**
	 * Update the list of objects in the current sector
	 * @param ObjectNames Array of object names to display
	 * @param ObjectCount Total count of objects in sector
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sector Map")
	void UpdateObjectList(const TArray<FText>& ObjectNames, int32 ObjectCount);
	virtual void UpdateObjectList_Implementation(const TArray<FText>& ObjectNames, int32 ObjectCount);

	/**
	 * Set the target sector to display
	 * @param Sector The ASpaceSectorMap actor to display information for
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	void SetTargetSector(ASpaceSectorMap* Sector);

	/**
	 * Get the currently targeted sector
	 * @return The current sector being displayed, or nullptr if none
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Map")
	ASpaceSectorMap* GetTargetSector() const { return CurrentSector; }

	/**
	 * Refresh the sector data from the current sector
	 * Call this to update the display when sector data changes
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	void RefreshSectorData();

	/**
	 * Called when a sector is selected in the UI
	 * @param SelectedSector The sector that was selected
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sector Map|Events")
	void OnSectorSelected(ASpaceSectorMap* SelectedSector);
	virtual void OnSectorSelected_Implementation(ASpaceSectorMap* SelectedSector);

	/**
	 * Clear all sector information from the display
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sector Map")
	void ClearSectorInfo();
	virtual void ClearSectorInfo_Implementation();

	/**
	 * Check if a valid sector is currently displayed
	 * @return True if a sector is being displayed
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Map")
	bool HasValidSector() const { return CurrentSector != nullptr; }

	/**
	 * Get the current sector display information
	 * @return The current sector display info struct
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Map")
	FSectorDisplayInfo GetCurrentSectorInfo() const { return CurrentSectorInfo; }

	/**
	 * Toggle the visibility of the sector map widget
	 * @param bVisible Whether the widget should be visible
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	void ToggleSectorMapVisibility(bool bVisible);

	/**
	 * Get a random spawn position within the current sector
	 * Useful for determining ship spawn locations
	 * @return A valid spawn position, or zero vector if no sector
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map|Spawning")
	FVector GetRandomSpawnPosition() const;

	/**
	 * Check if a world position is within the current sector
	 * @param WorldPosition The position to check
	 * @return True if the position is within sector bounds
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Map")
	bool IsPositionInCurrentSector(const FVector& WorldPosition) const;

	/**
	 * Get all sectors in the world
	 * Useful for universe-level navigation
	 * @return Array of all sector actors
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	TArray<ASpaceSectorMap*> GetAllSectors() const;

	/**
	 * Get neighboring sectors (adjacent grid positions)
	 * @return Array of sectors adjacent to current sector
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	TArray<ASpaceSectorMap*> GetNeighboringSectors() const;

	/**
	 * Calculate the distance to another sector
	 * @param OtherSector The sector to measure distance to
	 * @return Distance in Unreal Units, or -1 if no current sector
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Map")
	float GetDistanceToSector(ASpaceSectorMap* OtherSector) const;

protected:
	/** Native construction */
	virtual void NativeConstruct() override;

	/** Native destruction */
	virtual void NativeDestruct() override;

	// ====================
	// STATE PROPERTIES
	// ====================

	/** Reference to the current sector being displayed */
	UPROPERTY(BlueprintReadOnly, Category="Sector Map|State")
	ASpaceSectorMap* CurrentSector;

	/** Current sector display information */
	UPROPERTY(BlueprintReadOnly, Category="Sector Map|State")
	FSectorDisplayInfo CurrentSectorInfo;

	/** List of object names in the current sector */
	UPROPERTY(BlueprintReadOnly, Category="Sector Map|State")
	TArray<FText> SectorObjectNames;

	/** Whether the sector map is currently visible */
	UPROPERTY(BlueprintReadOnly, Category="Sector Map|State")
	bool bIsSectorMapVisible;

	// ====================
	// BOUND UI WIDGETS (Optional - will be created in C++ if not bound)
	// ====================

	/** Text display for sector name - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Sector Map|UI", meta=(BindWidgetOptional))
	class UTextBlock* Text_SectorName;

	/** Text display for sector description - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Sector Map|UI", meta=(BindWidgetOptional))
	class UTextBlock* Text_SectorDescription;

	/** Text display for object count - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Sector Map|UI", meta=(BindWidgetOptional))
	class UTextBlock* Text_ObjectCount;

	/** List view for sector objects - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Sector Map|UI", meta=(BindWidgetOptional))
	class UScrollBox* ObjectListScrollBox;

	// ====================
	// CONFIGURATION
	// ====================

	/** Whether to automatically create UI widgets in C++ if not bound in Blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sector Map|Config")
	bool bAutoCreateMissingWidgets;

	// ====================
	// HELPER FUNCTIONS
	// ====================

	/**
	 * Build sector display info from a sector actor
	 * @param Sector The sector to build info from
	 * @return Populated FSectorDisplayInfo struct
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	FSectorDisplayInfo BuildSectorDisplayInfo(ASpaceSectorMap* Sector) const;

	/**
	 * Create default UI widgets programmatically if they weren't bound in Blueprint
	 * This allows the widget to work with zero Blueprint setup
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	void CreateDefaultUIWidgets();

	/**
	 * Get detailed statistics about the current sector
	 * @return String with sector statistics
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	FString GetSectorStatistics() const;

	/**
	 * Update object tracking within the sector
	 * Refreshes the list of actors in the sector
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	void UpdateObjectTracking();

	/**
	 * Get the player's distance to sector center
	 * @return Distance in Unreal Units, or -1 if no sector or no player
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Map")
	float GetPlayerDistanceToSectorCenter() const;

	/**
	 * Check if the player is currently in this sector
	 * @return True if player is within sector bounds
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Map")
	bool IsPlayerInSector() const;

	/**
	 * Get navigation direction to sector center from player
	 * @return Normalized direction vector, or zero if no sector/player
	 */
	UFUNCTION(BlueprintCallable, Category="Sector Map")
	FVector GetNavigationDirectionToCenter() const;

	/**
	 * Calculate estimated travel time to sector from player position
	 * @param TravelSpeed Speed in units per second
	 * @return Estimated time in seconds, or -1 if cannot calculate
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Map")
	float CalculateTravelTimeToSector(float TravelSpeed) const;
};
