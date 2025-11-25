// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Stations/SpaceStation.h"
#include "StationModuleCatalog.h"
#include "StationEditorManager.generated.h"

// Forward declarations
class AStationBuildPreview;
class UStationGridSystem;

/**
 * Result of a module placement validation check
 */
UENUM(BlueprintType)
enum class EModulePlacementResult : uint8
{
	Success				UMETA(DisplayName="Success"),
	InvalidModule		UMETA(DisplayName="Invalid Module"),
	InvalidPosition		UMETA(DisplayName="Invalid Position"),
	CollisionDetected	UMETA(DisplayName="Collision Detected"),
	InsufficientPower	UMETA(DisplayName="Insufficient Power"),
	InsufficientTech	UMETA(DisplayName="Insufficient Tech Level"),
	InsufficientFunds	UMETA(DisplayName="Insufficient Funds"),
	NoStation			UMETA(DisplayName="No Station Selected"),
	NotEditing			UMETA(DisplayName="Not In Edit Mode")
};

/**
 * Delegate broadcast when editing state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEditingStateChanged, bool, bIsEditing);

/**
 * Delegate broadcast when a module is placed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModulePlaced, ASpaceStationModule*, PlacedModule);

/**
 * Delegate broadcast when a module is removed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModuleRemoved, ASpaceStationModule*, RemovedModule);

/**
 * Delegate broadcast when power balance changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPowerBalanceChanged, float, NewBalance);

/**
 * Station Editor Manager - Core manager class for station editing sessions
 * 
 * This class manages the entire station editing workflow including:
 * - Starting and ending edit sessions
 * - Module placement, removal, and manipulation
 * - Validation of placements (collisions, power, tech level)
 * - Resource and power tracking
 * - Preview system integration
 * 
 * Usage:
 * 1. Create an instance of this manager
 * 2. Assign a ModuleCatalog with available modules
 * 3. Call BeginEditing() with a station to edit
 * 4. Use PlaceModule(), RemoveModule(), etc. to modify the station
 * 5. Call Save() or Cancel() to end the editing session
 * 
 * All major functions are exposed as BlueprintNativeEvents to allow
 * Blueprint customization of behavior.
 * 
 * @see UStationModuleCatalog
 * @see AStationBuildPreview
 */
UCLASS(BlueprintType, Blueprintable)
class STATIONEDITOR_API UStationEditorManager : public UObject
{
	GENERATED_BODY()

public:
	UStationEditorManager();

	/** Default collision radius for module placement validation */
	static constexpr float DefaultCollisionRadius = 200.0f;

	// =====================
	// Configuration
	// =====================

	/** The catalog of available modules for this editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Editor|Configuration")
	UStationModuleCatalog* ModuleCatalog;

	/** Current player's tech level (used for module availability) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Editor|Configuration", meta=(ClampMin=1, ClampMax=10))
	int32 PlayerTechLevel = 1;

	/** Current player's available credits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Editor|Configuration", meta=(ClampMin=0))
	int32 PlayerCredits = 0;

	/** Whether to automatically snap modules to grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Editor|Configuration")
	bool bSnapToGrid = true;

	/** Whether to check for collisions when placing modules */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Editor|Configuration")
	bool bCheckCollisions = true;

	/** Collision radius for module placement validation (world units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Editor|Configuration", meta=(ClampMin=10.0f, UIMin=10.0f))
	float CollisionRadius = DefaultCollisionRadius;

	// =====================
	// State
	// =====================

	/** Whether we are currently in editing mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station Editor|State")
	bool bIsEditing = false;

	/** The station currently being edited */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station Editor|State")
	ASpaceStation* CurrentStation;

	/** The preview actor showing placement preview */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station Editor|State")
	AStationBuildPreview* PreviewActor;

	/** Grid system component for snapping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station Editor|State")
	UStationGridSystem* GridSystem;

	// =====================
	// Events
	// =====================

	/** Called when editing state changes */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnEditingStateChanged OnEditingStateChanged;

	/** Called when a module is successfully placed */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnModulePlaced OnModulePlaced;

	/** Called when a module is removed */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnModuleRemoved OnModuleRemoved;

	/** Called when power balance changes */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnPowerBalanceChanged OnPowerBalanceChanged;

	// =====================
	// Editing Lifecycle
	// =====================

	/**
	 * Begin editing a station
	 * @param Station The station to edit
	 * @return True if editing started successfully
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Lifecycle")
	bool BeginEditing(ASpaceStation* Station);

	/**
	 * End editing without saving changes
	 * Reverts all changes made during the editing session
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Lifecycle")
	void Cancel();

	/**
	 * Save changes and end editing
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Lifecycle")
	bool Save();

	/**
	 * End editing (internal cleanup, called by Save and Cancel)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Lifecycle")
	void EndEditing();

	// =====================
	// Module Operations
	// =====================

	/**
	 * Place a module at the specified position and rotation
	 * @param ModuleClass The class of module to place
	 * @param Position World position for the module
	 * @param Rotation Rotation for the module
	 * @return The placed module, or nullptr if placement failed
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Modules")
	ASpaceStationModule* PlaceModule(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation);

	/**
	 * Remove a module from the station
	 * @param Module The module to remove
	 * @return True if removal was successful
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Modules")
	bool RemoveModule(ASpaceStationModule* Module);

	/**
	 * Move a module to a new position
	 * @param Module The module to move
	 * @param NewPosition New world position
	 * @return True if move was successful
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Modules")
	bool MoveModule(ASpaceStationModule* Module, FVector NewPosition);

	/**
	 * Rotate a module to a new orientation
	 * @param Module The module to rotate
	 * @param NewRotation New rotation
	 * @return True if rotation was successful
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Modules")
	bool RotateModule(ASpaceStationModule* Module, FRotator NewRotation);

	// =====================
	// Validation
	// =====================

	/**
	 * Check if a module can be placed at the specified position
	 * @param ModuleClass The class of module to check
	 * @param Position Position to check
	 * @param Rotation Rotation to check
	 * @return Result enum indicating success or reason for failure
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Station Editor|Validation")
	EModulePlacementResult CanPlaceModule(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation);

	/**
	 * Check if there are any collisions at the specified position
	 * @param ModuleClass The module class to check collision for
	 * @param Position Position to check
	 * @param Rotation Rotation to check
	 * @return True if there is a collision
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Validation")
	bool CheckCollision(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation) const;

	/**
	 * Check if player has sufficient tech level for a module
	 * @param ModuleClass The module class to check
	 * @return True if player meets tech requirements
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Validation")
	bool HasSufficientTechLevel(TSubclassOf<ASpaceStationModule> ModuleClass) const;

	/**
	 * Check if player can afford to build a module
	 * @param ModuleClass The module class to check
	 * @return True if player has sufficient credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Validation")
	bool CanAffordModule(TSubclassOf<ASpaceStationModule> ModuleClass) const;

	// =====================
	// Resource Tracking
	// =====================

	/**
	 * Get total power consumed by all modules
	 * @return Total power consumption (positive values)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Resources")
	float GetTotalPowerConsumption() const;

	/**
	 * Get total power generated by all modules
	 * @return Total power generation (positive values)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Resources")
	float GetTotalPowerGeneration() const;

	/**
	 * Get current power balance (generation - consumption)
	 * @return Power balance (positive = surplus, negative = deficit)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Resources")
	float GetPowerBalance() const;

	/**
	 * Check if station has sufficient power
	 * @return True if power generation >= consumption
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Resources")
	bool HasSufficientPower() const;

	/**
	 * Check if adding a module would cause power deficit
	 * @param ModuleClass The module class to check
	 * @return True if adding would cause deficit
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Resources")
	bool WouldCausePowerDeficit(TSubclassOf<ASpaceStationModule> ModuleClass) const;

	// =====================
	// Preview System
	// =====================

	/**
	 * Show the placement preview for a module class
	 * @param ModuleClass The class to preview
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Preview")
	void ShowPreview(TSubclassOf<ASpaceStationModule> ModuleClass);

	/**
	 * Update the preview position
	 * @param Position New position for preview
	 * @param Rotation New rotation for preview
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Preview")
	void UpdatePreview(FVector Position, FRotator Rotation);

	/**
	 * Hide the placement preview
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Preview")
	void HidePreview();

	// =====================
	// Utility
	// =====================

	/**
	 * Get list of modules that can be built with current tech level
	 * @return Array of available module entries
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Utility")
	TArray<FStationModuleEntry> GetAvailableModules() const;

	/**
	 * Get the build cost for a module class
	 * @param ModuleClass The class to get cost for
	 * @param OutCost The output build cost (only valid if returns true)
	 * @return True if module was found in catalog
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Utility")
	bool GetModuleBuildCost(TSubclassOf<ASpaceStationModule> ModuleClass, FStationBuildCost& OutCost) const;

protected:
	/** Modules that were added during this editing session (for undo on cancel) */
	UPROPERTY()
	TArray<ASpaceStationModule*> ModulesAddedThisSession;

	/** Original positions of moved modules (for undo on cancel) */
	UPROPERTY()
	TMap<ASpaceStationModule*, FTransform> OriginalModuleTransforms;

	/**
	 * Internal function to revert changes on cancel
	 */
	void RevertChanges();

	/**
	 * Update power balance and broadcast event if changed
	 */
	void NotifyPowerBalanceChanged();

private:
	/** Cached last power balance for change detection */
	float LastPowerBalance = 0.0f;
};
