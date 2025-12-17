// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Stations/SpaceStationModule.h"
#include "StationManagementWidget.generated.h"

// Forward declarations
class ASpaceStation;

/**
 * Station Management Widget for managing space stations
 * 
 * This widget provides a comprehensive interface for:
 * - Viewing station status and modules
 * - Managing resources and inventory
 * - Configuring station settings
 * - Building and upgrading modules
 * 
 * Usage:
 * 1. Create a Blueprint Widget based on this class
 * 2. Design the UI layout in UMG
 * 3. Bind to station events for real-time updates
 * 4. Call SetManagedStation to link to a station
 * 
 * Example:
 * - Player docks at station
 * - Create and display this widget
 * - Widget shows station status and modules
 * - Player can manage station through UI
 */
UCLASS()
class ADASTREA_API UStationManagementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UStationManagementWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ========================================================================
	// Managed Station
	// ========================================================================

	/** The station being managed */
	UPROPERTY(BlueprintReadOnly, Category = "Station Management")
	ASpaceStation* ManagedStation;

	// ========================================================================
	// UI State
	// ========================================================================

	/** Currently selected module group for filtering */
	UPROPERTY(BlueprintReadWrite, Category = "Station Management")
	EStationModuleGroup SelectedModuleGroup;

	/** Whether the build mode is active */
	UPROPERTY(BlueprintReadWrite, Category = "Station Management")
	bool bBuildModeActive;

	// ========================================================================
	// Blueprint Events
	// ========================================================================

	/**
	 * Event fired when station status is updated
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Station Management")
	void OnStationStatusUpdated();

	/**
	 * Event fired when a module is added to the station
	 * @param NewModule The module that was added
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Station Management")
	void OnModuleAdded(ASpaceStationModule* NewModule);

	/**
	 * Event fired when a module is removed from the station
	 * @param RemovedModule The module that was removed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Station Management")
	void OnModuleRemoved(ASpaceStationModule* RemovedModule);

public:
	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Set the station to manage
	 * @param Station The space station to manage
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Management")
	void SetManagedStation(ASpaceStation* Station);

	/**
	 * Get the currently managed station
	 * @return The managed station
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Station Management")
	ASpaceStation* GetManagedStation() const { return ManagedStation; }

	/**
	 * Refresh all station data displayed in the UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Management")
	void RefreshStationData();

	/**
	 * Get all modules in the station filtered by group
	 * @param Group Module group to filter by (All = all modules, no filtering)
	 * @return Array of modules
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Station Management")
	TArray<ASpaceStationModule*> GetModulesByGroup(EStationModuleGroup Group) const;

	/**
	 * Get total module count
	 * @return Number of modules in the station
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Station Management")
	int32 GetTotalModuleCount() const;

	/**
	 * Get station operational status percentage (0-100)
	 * @return Operational status percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Station Management")
	float GetOperationalStatusPercent() const;

	/**
	 * Check if a module can be built at the current station
	 * @param ModuleClass The module class to check
	 * @return True if the module can be built
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Management")
	bool CanBuildModule(TSubclassOf<ASpaceStationModule> ModuleClass) const;

	/**
	 * Request to build a new module
	 * @param ModuleClass The module class to build
	 * @return True if build request was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Management")
	bool RequestBuildModule(TSubclassOf<ASpaceStationModule> ModuleClass);

	/**
	 * Request to remove a module from the station
	 * @param Module The module to remove
	 * @return True if removal request was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Management")
	bool RequestRemoveModule(ASpaceStationModule* Module);

	/**
	 * Set the selected module group for filtering
	 * @param Group The module group to select
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Management")
	void SetSelectedModuleGroup(EStationModuleGroup Group);

	/**
	 * Toggle build mode on/off
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Management")
	void ToggleBuildMode();

	/**
	 * Close the station management interface
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Management")
	void CloseInterface();

private:
	/** Handle station data changes */
	void HandleStationChanged();

	/** Timer handle for periodic updates */
	FTimerHandle UpdateTimer;
};
