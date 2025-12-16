// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UniverseMapWidget.generated.h"

class ASpaceSectorMap;
class USectorMapWidget;

/**
 * Universe sector information for grid display
 * Contains data about a sector's position in the universe grid
 */
USTRUCT(BlueprintType)
struct FUniverseSectorInfo
{
	GENERATED_BODY()

	/** Reference to the sector actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Universe Info")
	ASpaceSectorMap* Sector;

	/** Grid coordinates for this sector (X, Y, Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Universe Info")
	FIntVector GridCoordinates;

	/** Whether this sector is discovered by the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Universe Info")
	bool bIsDiscovered;

	/** Whether this sector is currently selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Universe Info")
	bool bIsSelected;

	FUniverseSectorInfo()
		: Sector(nullptr)
		, GridCoordinates(FIntVector::ZeroValue)
		, bIsDiscovered(false)
		, bIsSelected(false)
	{
	}
};

/**
 * Universe Map Widget for displaying all sectors in the universe
 * 
 * This widget provides a comprehensive map interface showing:
 * - All sectors in the game world organized in a grid
 * - Player's current location
 * - Sector information and navigation
 * - Universe exploration progress
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class (e.g., WBP_UniverseMap)
 * - Design the universe map layout in the Blueprint designer
 * - Override BlueprintNativeEvents to customize sector display
 * - Bind to selection events for sector navigation
 * - Toggle visibility with M key (default mapping)
 * 
 * Integration:
 * - Works with multiple ASpaceSectorMap actors
 * - Can integrate with SectorMapWidget for detailed sector view
 * - Tracks player exploration and discovery
 * - Designer-friendly with full Blueprint exposure
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UUniverseMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UUniverseMapWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the universe map widget
	 * Scans for all sectors in the world and builds the sector grid
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Universe Map")
	void InitializeUniverseMap();
	virtual void InitializeUniverseMap_Implementation();

	/**
	 * Refresh the universe map data
	 * Rescans for sectors and updates the display
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	void RefreshUniverseMap();

	/**
	 * Update the universe grid display
	 * Called when sector data changes or player discovers new sectors
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Universe Map")
	void UpdateUniverseGrid();
	virtual void UpdateUniverseGrid_Implementation();

	/**
	 * Set the currently selected sector
	 * @param Sector The sector to select, or nullptr to deselect
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	void SetSelectedSector(ASpaceSectorMap* Sector);

	/**
	 * Get the currently selected sector
	 * @return The selected sector, or nullptr if none
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Universe Map")
	ASpaceSectorMap* GetSelectedSector() const { return SelectedSector; }

	/**
	 * Get all sectors in the universe
	 * @return Array of all sector actors in the world
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Universe Map")
	TArray<ASpaceSectorMap*> GetAllSectors() const { return AllSectors; }

	/**
	 * Get the sector at specific grid coordinates
	 * @param GridCoords The grid coordinates to query
	 * @return The sector at those coordinates, or nullptr if none
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	ASpaceSectorMap* GetSectorAtGridCoordinates(const FIntVector& GridCoords) const;

	/**
	 * Get the player's current sector
	 * @return The sector containing the player, or nullptr if not in a sector
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	ASpaceSectorMap* GetPlayerCurrentSector() const;

	/**
	 * Check if a sector has been discovered by the player
	 * @param Sector The sector to check
	 * @return True if the sector is discovered
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Universe Map")
	bool IsSectorDiscovered(ASpaceSectorMap* Sector) const;

	/**
	 * Mark a sector as discovered by the player
	 * @param Sector The sector to mark as discovered
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	void MarkSectorDiscovered(ASpaceSectorMap* Sector);

	/**
	 * Toggle the universe map visibility
	 * @param bVisible Whether the map should be visible
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	void ToggleUniverseMapVisibility(bool bVisible);

	/**
	 * Called when a sector is selected in the map UI
	 * @param Sector The sector that was selected
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Universe Map|Events")
	void OnSectorSelected(ASpaceSectorMap* Sector);
	virtual void OnSectorSelected_Implementation(ASpaceSectorMap* Sector);

	/**
	 * Called when the universe map is opened
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Universe Map|Events")
	void OnMapOpened();
	virtual void OnMapOpened_Implementation();

	/**
	 * Called when the universe map is closed
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Universe Map|Events")
	void OnMapClosed();
	virtual void OnMapClosed_Implementation();

	/**
	 * Get the total number of sectors in the universe
	 * @return Total sector count
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Universe Map")
	int32 GetTotalSectorCount() const { return AllSectors.Num(); }

	/**
	 * Get the number of discovered sectors
	 * @return Number of sectors discovered by the player
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Universe Map")
	int32 GetDiscoveredSectorCount() const;

	/**
	 * Get the exploration progress as a percentage
	 * @return Percentage of sectors discovered (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Universe Map")
	float GetExplorationProgress() const;

	/**
	 * Build universe sector info from all sectors in the world
	 * Organizes sectors into a grid based on their world positions
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	void BuildUniverseSectorGrid();

	/**
	 * Get universe sector info for all sectors
	 * @return Array of sector info structs with grid positions
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Universe Map")
	TArray<FUniverseSectorInfo> GetUniverseSectorInfo() const { return UniverseSectorInfo; }

protected:
	/** Native construction */
	virtual void NativeConstruct() override;

	/** Native destruction */
	virtual void NativeDestruct() override;

	// ====================
	// STATE PROPERTIES
	// ====================

	/** All sectors in the universe */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|State")
	TArray<ASpaceSectorMap*> AllSectors;

	/** Currently selected sector */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|State")
	ASpaceSectorMap* SelectedSector;

	/** Whether the universe map is currently visible */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|State")
	bool bIsUniverseMapVisible;

	/** Set of discovered sector actors */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|State")
	TSet<ASpaceSectorMap*> DiscoveredSectors;

	/** Universe sector information with grid coordinates */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|State")
	TArray<FUniverseSectorInfo> UniverseSectorInfo;

	// ====================
	// BOUND UI WIDGETS (Optional - will be created in C++ if not bound)
	// ====================

	/** Main container panel for the sector grid - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|UI", meta=(BindWidgetOptional))
	class UCanvasPanel* SectorGridPanel;

	/** Text display for selected sector name - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|UI", meta=(BindWidgetOptional))
	class UTextBlock* Text_SectorName;

	/** Text display for selected sector description - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|UI", meta=(BindWidgetOptional))
	class UTextBlock* Text_SectorDescription;

	/** Progress bar showing exploration completion - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|UI", meta=(BindWidgetOptional))
	class UProgressBar* ProgressBar_Exploration;

	/** Text display for exploration percentage - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|UI", meta=(BindWidgetOptional))
	class UTextBlock* Text_ExplorationPercent;

	/** Close button - created automatically if not bound */
	UPROPERTY(BlueprintReadOnly, Category="Universe Map|UI", meta=(BindWidgetOptional))
	class UButton* Button_Close;

	// ====================
	// CONFIGURATION
	// ====================

	/** Whether to automatically discover sectors the player visits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Universe Map|Config")
	bool bAutoDiscoverVisitedSectors;

	/** Whether to show undiscovered sectors on the map (as blank/unknown) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Universe Map|Config")
	bool bShowUndiscoveredSectors;

	/** Whether to automatically create UI widgets in C++ if not bound in Blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Universe Map|Config")
	bool bAutoCreateMissingWidgets;

	// ====================
	// HELPER FUNCTIONS
	// ====================

	/**
	 * Calculate grid coordinates for a sector based on its world position
	 * @param Sector The sector to calculate grid coordinates for
	 * @return Grid coordinates (X, Y, Z)
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	FIntVector CalculateGridCoordinates(ASpaceSectorMap* Sector) const;

	/**
	 * Find all sectors in the current world
	 * @return Array of all ASpaceSectorMap actors
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	TArray<ASpaceSectorMap*> FindAllSectorsInWorld() const;

	/**
	 * Create default UI widgets programmatically if they weren't bound in Blueprint
	 * This allows the widget to work with zero Blueprint setup
	 */
	UFUNCTION(BlueprintCallable, Category="Universe Map")
	void CreateDefaultUIWidgets();

	/**
	 * Setup close button click handler
	 */
	UFUNCTION()
	void OnCloseButtonClicked();
};
