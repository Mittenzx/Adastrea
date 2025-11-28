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
 * Types of editor actions for undo/redo system
 */
UENUM(BlueprintType)
enum class EEditorActionType : uint8
{
	PlaceModule			UMETA(DisplayName="Place Module"),
	RemoveModule		UMETA(DisplayName="Remove Module"),
	MoveModule			UMETA(DisplayName="Move Module"),
	RotateModule		UMETA(DisplayName="Rotate Module"),
	UpgradeModule		UMETA(DisplayName="Upgrade Module")
};

/**
 * Types of module connections
 */
UENUM(BlueprintType)
enum class EModuleConnectionType : uint8
{
	Power				UMETA(DisplayName="Power"),
	Data				UMETA(DisplayName="Data Network"),
	LifeSupport			UMETA(DisplayName="Life Support")
};

/**
 * Notification severity levels
 */
UENUM(BlueprintType)
enum class ENotificationSeverity : uint8
{
	Info				UMETA(DisplayName="Info"),
	Warning				UMETA(DisplayName="Warning"),
	Error				UMETA(DisplayName="Error"),
	Success				UMETA(DisplayName="Success")
};

/**
 * View modes for the station editor
 */
UENUM(BlueprintType)
enum class EStationEditorViewMode : uint8
{
	Edit				UMETA(DisplayName="Edit Mode"),
	Preview				UMETA(DisplayName="Preview Mode"),
	Connections			UMETA(DisplayName="Connections Mode")
};

/**
 * Record of an editor action for undo/redo
 */
USTRUCT(BlueprintType)
struct STATIONEDITOR_API FEditorAction
{
	GENERATED_BODY()

	/** Type of action performed */
	UPROPERTY(BlueprintReadOnly, Category="Editor Action")
	EEditorActionType ActionType = EEditorActionType::PlaceModule;

	/** Module class involved in the action */
	UPROPERTY(BlueprintReadOnly, Category="Editor Action")
	TSubclassOf<ASpaceStationModule> ModuleClass;

	/** The module instance (may be invalid after undo) */
	UPROPERTY(BlueprintReadOnly, Category="Editor Action")
	ASpaceStationModule* Module = nullptr;

	/** Position before the action */
	UPROPERTY(BlueprintReadOnly, Category="Editor Action")
	FVector PreviousPosition = FVector::ZeroVector;

	/** Position after the action */
	UPROPERTY(BlueprintReadOnly, Category="Editor Action")
	FVector NewPosition = FVector::ZeroVector;

	/** Rotation before the action */
	UPROPERTY(BlueprintReadOnly, Category="Editor Action")
	FRotator PreviousRotation = FRotator::ZeroRotator;

	/** Rotation after the action */
	UPROPERTY(BlueprintReadOnly, Category="Editor Action")
	FRotator NewRotation = FRotator::ZeroRotator;

	/** Timestamp of the action */
	UPROPERTY(BlueprintReadOnly, Category="Editor Action")
	float Timestamp = 0.0f;

	FEditorAction() = default;
};

/**
 * A connection between two modules
 */
USTRUCT(BlueprintType)
struct STATIONEDITOR_API FModuleConnection
{
	GENERATED_BODY()

	/** First module in the connection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Connection")
	ASpaceStationModule* ModuleA = nullptr;

	/** Second module in the connection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Connection")
	ASpaceStationModule* ModuleB = nullptr;

	/** Type of connection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Connection")
	EModuleConnectionType ConnectionType = EModuleConnectionType::Power;

	/** Whether the connection is active and functional */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Connection")
	bool bIsActive = true;

	/** Capacity/flow rate of the connection (e.g., MW for power) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Connection", meta=(ClampMin=0.0f))
	float Capacity = 100.0f;

	/** Current usage percentage (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Connection", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float CurrentUsage = 0.0f;

	FModuleConnection() = default;
};

/**
 * Construction queue item for building modules
 */
USTRUCT(BlueprintType)
struct STATIONEDITOR_API FConstructionQueueItem
{
	GENERATED_BODY()

	/** Unique ID for this queue item */
	UPROPERTY(BlueprintReadOnly, Category="Construction Queue")
	int32 QueueId = 0;

	/** Module class to construct */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction Queue")
	TSubclassOf<ASpaceStationModule> ModuleClass;

	/** Target position for the module */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction Queue")
	FVector TargetPosition = FVector::ZeroVector;

	/** Target rotation for the module */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction Queue")
	FRotator TargetRotation = FRotator::ZeroRotator;

	/** Total time required to build (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction Queue", meta=(ClampMin=0.0f))
	float TotalBuildTime = 60.0f;

	/** Time remaining to complete construction (seconds) */
	UPROPERTY(BlueprintReadOnly, Category="Construction Queue")
	float TimeRemaining = 60.0f;

	/** Whether construction is currently in progress */
	UPROPERTY(BlueprintReadOnly, Category="Construction Queue")
	bool bIsBuilding = false;

	/** Whether construction is paused */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction Queue")
	bool bIsPaused = false;

	/**
	 * Get the construction progress (0.0 to 1.0)
	 */
	float GetProgress() const
	{
		if (TotalBuildTime <= 0.0f) return 1.0f;
		return FMath::Clamp(1.0f - (TimeRemaining / TotalBuildTime), 0.0f, 1.0f);
	}

	FConstructionQueueItem() = default;
};

/**
 * Station-wide statistics
 */
USTRUCT(BlueprintType)
struct STATIONEDITOR_API FStationStatistics
{
	GENERATED_BODY()

	/** Total number of modules */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	int32 TotalModules = 0;

	/** Maximum allowed modules */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Statistics", meta=(ClampMin=1))
	int32 MaxModules = 25;

	/** Current population */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	int32 CurrentPopulation = 0;

	/** Maximum population capacity */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	int32 MaxPopulation = 0;

	/** Power generated (MW) */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	float PowerGenerated = 0.0f;

	/** Power consumed (MW) */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	float PowerConsumed = 0.0f;

	/** Defense rating (0-100) */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	float DefenseRating = 0.0f;

	/** Overall efficiency rating (A+ to F) represented as 0.0 to 1.0 */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	float EfficiencyRating = 0.0f;

	/** Total cargo storage capacity */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	float CargoCapacity = 0.0f;

	/** Current cargo usage */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	float CargoUsed = 0.0f;

	/** Data network bandwidth usage (0.0 to 1.0) */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	float DataNetworkUsage = 0.0f;

	/** Life support coverage (0.0 to 1.0) */
	UPROPERTY(BlueprintReadOnly, Category="Station Statistics")
	float LifeSupportCoverage = 1.0f;

	/**
	 * Get power balance (generation - consumption)
	 */
	float GetPowerBalance() const { return PowerGenerated - PowerConsumed; }

	/**
	 * Get efficiency letter grade
	 */
	FString GetEfficiencyGrade() const
	{
		if (EfficiencyRating >= 0.95f) return TEXT("A+");
		if (EfficiencyRating >= 0.9f) return TEXT("A");
		if (EfficiencyRating >= 0.8f) return TEXT("B");
		if (EfficiencyRating >= 0.7f) return TEXT("C");
		if (EfficiencyRating >= 0.6f) return TEXT("D");
		return TEXT("F");
	}

	FStationStatistics() = default;
};

/**
 * Station notification message
 */
USTRUCT(BlueprintType)
struct STATIONEDITOR_API FStationNotification
{
	GENERATED_BODY()

	/** Unique notification ID */
	UPROPERTY(BlueprintReadOnly, Category="Notification")
	int32 NotificationId = 0;

	/** Notification message text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notification")
	FText Message;

	/** Severity level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notification")
	ENotificationSeverity Severity = ENotificationSeverity::Info;

	/** Timestamp when notification was created */
	UPROPERTY(BlueprintReadOnly, Category="Notification")
	float Timestamp = 0.0f;

	/** Whether the notification has been read/acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notification")
	bool bIsRead = false;

	/** Related module (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notification")
	ASpaceStationModule* RelatedModule = nullptr;

	FStationNotification() = default;
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
 * Delegate broadcast when undo/redo state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUndoRedoStateChanged, bool, bCanUndo, bool, bCanRedo);

/**
 * Delegate broadcast when a notification is added
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationAdded, const FStationNotification&, Notification);

/**
 * Delegate broadcast when construction queue changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConstructionQueueChanged);

/**
 * Delegate broadcast when station statistics are updated
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatisticsUpdated, const FStationStatistics&, Statistics);

/**
 * Delegate broadcast when view mode changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViewModeChanged, EStationEditorViewMode, NewViewMode);

/**
 * Delegate broadcast when a connection is added or removed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionChanged, const FModuleConnection&, Connection);

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

	/** Maximum size of the notification history */
	static constexpr int32 MaxNotifications = 50;

	/** Default build time when catalog entry is not found (seconds) */
	static constexpr float DefaultBuildTime = 60.0f;

	/** Default upgrade cost multiplier (0.5 = 50% of build cost) */
	static constexpr float DefaultUpgradeCostMultiplier = 0.5f;

	/** Efficiency penalty for power deficit */
	static constexpr float PowerDeficitEfficiencyPenalty = 0.3f;

	/** Efficiency penalty for power over-production */
	static constexpr float PowerOverProductionEfficiencyPenalty = 0.1f;

	/** Power load warning threshold (0.8 = 80% capacity) */
	static constexpr float PowerLoadWarningThreshold = 0.8f;

	/** Population capacity warning threshold (0.9 = 90% capacity) */
	static constexpr float PopulationWarningThreshold = 0.9f;

	/** Default cargo capacity per storage module (tons) */
	static constexpr float DefaultCargoCapacityPerModule = 500.0f;

	/** Default population capacity per habitation module */
	static constexpr int32 DefaultPopulationCapacityPerModule = 100;

	/** Default defense rating per defense module */
	static constexpr float DefaultDefenseRatingPerModule = 15.0f;

	/** Default connection capacity values by type */
	static constexpr float DefaultPowerConnectionCapacity = 100.0f;   // MW
	static constexpr float DefaultDataConnectionCapacity = 1000.0f;   // Mbps
	static constexpr float DefaultLifeSupportConnectionCapacity = 50.0f;  // Crew capacity

	/** Distance threshold for module matching during undo/redo (squared) */
	static constexpr float ModuleMatchDistanceSquared = 100.0f;

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

	/** Called when undo/redo state changes */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnUndoRedoStateChanged OnUndoRedoStateChanged;

	/** Called when a notification is added */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnNotificationAdded OnNotificationAdded;

	/** Called when construction queue changes */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnConstructionQueueChanged OnConstructionQueueChanged;

	/** Called when station statistics are updated */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnStatisticsUpdated OnStatisticsUpdated;

	/** Called when view mode changes */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnViewModeChanged OnViewModeChanged;

	/** Called when a connection is added or removed */
	UPROPERTY(BlueprintAssignable, Category="Station Editor|Events")
	FOnConnectionChanged OnConnectionChanged;

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

	// =====================
	// Undo/Redo System
	// =====================

	/**
	 * Undo the last action
	 * @return True if undo was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Undo Redo")
	bool Undo();

	/**
	 * Redo the last undone action
	 * @return True if redo was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Undo Redo")
	bool Redo();

	/**
	 * Check if undo is available
	 * @return True if there are actions to undo
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Undo Redo")
	bool CanUndo() const;

	/**
	 * Check if redo is available
	 * @return True if there are actions to redo
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Undo Redo")
	bool CanRedo() const;

	/**
	 * Clear the undo/redo history
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Undo Redo")
	void ClearUndoHistory();

	/**
	 * Get the number of undoable actions
	 * @return Number of actions in undo stack
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Undo Redo")
	int32 GetUndoCount() const;

	/**
	 * Get the number of redoable actions
	 * @return Number of actions in redo stack
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Undo Redo")
	int32 GetRedoCount() const;

	// =====================
	// Module Connections
	// =====================

	/**
	 * Add a connection between two modules
	 * @param ModuleA First module
	 * @param ModuleB Second module
	 * @param ConnectionType Type of connection to create
	 * @return True if connection was created successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Connections")
	bool AddConnection(ASpaceStationModule* ModuleA, ASpaceStationModule* ModuleB, EModuleConnectionType ConnectionType);

	/**
	 * Remove a connection between two modules
	 * @param ModuleA First module
	 * @param ModuleB Second module
	 * @param ConnectionType Type of connection to remove (optional, removes all if not specified)
	 * @return True if connection was removed
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Connections")
	bool RemoveConnection(ASpaceStationModule* ModuleA, ASpaceStationModule* ModuleB, EModuleConnectionType ConnectionType);

	/**
	 * Get all connections for a module
	 * @param Module The module to query
	 * @return Array of connections involving this module
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Connections")
	TArray<FModuleConnection> GetModuleConnections(ASpaceStationModule* Module) const;

	/**
	 * Get all connections of a specific type
	 * @param ConnectionType The type to filter by
	 * @return Array of connections of the specified type
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Connections")
	TArray<FModuleConnection> GetConnectionsByType(EModuleConnectionType ConnectionType) const;

	/**
	 * Get all connections in the station
	 * @return Array of all connections
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Connections")
	TArray<FModuleConnection> GetAllConnections() const;

	/**
	 * Auto-generate connections for a module based on adjacency
	 * @param Module The newly placed module
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Connections")
	void AutoGenerateConnections(ASpaceStationModule* Module);

	/**
	 * Check if a module is connected to the power grid
	 * @param Module The module to check
	 * @return True if module has power connection path to a reactor
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Connections")
	bool IsConnectedToPower(const ASpaceStationModule* Module) const;

	/**
	 * Check if a module has life support
	 * @param Module The module to check
	 * @return True if module has life support connection
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Connections")
	bool HasLifeSupport(const ASpaceStationModule* Module) const;

	// =====================
	// Construction Queue
	// =====================

	/**
	 * Add a module to the construction queue
	 * @param ModuleClass The module class to build
	 * @param Position Target position
	 * @param Rotation Target rotation
	 * @return Queue item ID, or -1 if failed
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Construction")
	int32 QueueConstruction(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation);

	/**
	 * Remove an item from the construction queue
	 * @param QueueId The ID of the item to remove
	 * @return True if item was removed
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Construction")
	bool CancelConstruction(int32 QueueId);

	/**
	 * Pause or resume a construction item
	 * @param QueueId The ID of the item
	 * @param bPause True to pause, false to resume
	 * @return True if state was changed
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Construction")
	bool SetConstructionPaused(int32 QueueId, bool bPause);

	/**
	 * Move an item up or down in the queue
	 * @param QueueId The ID of the item
	 * @param MoveUp True to move up (higher priority), false to move down
	 * @return True if item was moved
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Construction")
	bool ReorderConstruction(int32 QueueId, bool MoveUp);

	/**
	 * Get the current construction queue
	 * @return Array of queued construction items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Construction")
	TArray<FConstructionQueueItem> GetConstructionQueue() const;

	/**
	 * Get the currently building item
	 * @param OutItem The item currently under construction
	 * @return True if there is an item being built
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Construction")
	bool GetCurrentConstruction(FConstructionQueueItem& OutItem) const;

	/**
	 * Update construction progress (call each frame or timer)
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Construction")
	void UpdateConstruction(float DeltaTime);

	// =====================
	// Station Statistics
	// =====================

	/**
	 * Get comprehensive station statistics
	 * @return Current station statistics
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Statistics")
	FStationStatistics GetStationStatistics() const;

	/**
	 * Recalculate all station statistics
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Statistics")
	void RecalculateStatistics();

	/**
	 * Get total population capacity from habitation modules
	 * @return Maximum population the station can support
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Statistics")
	int32 GetPopulationCapacity() const;

	/**
	 * Get defense rating based on defense modules
	 * @return Defense rating (0-100)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Statistics")
	float GetDefenseRating() const;

	/**
	 * Get overall efficiency rating
	 * @return Efficiency as a value from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Statistics")
	float GetEfficiencyRating() const;

	// =====================
	// Notifications
	// =====================

	/**
	 * Add a notification
	 * @param Message The notification message
	 * @param Severity Severity level
	 * @param RelatedModule Optional module related to the notification
	 * @return Notification ID
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Notifications")
	int32 AddNotification(FText Message, ENotificationSeverity Severity, ASpaceStationModule* RelatedModule = nullptr);

	/**
	 * Mark a notification as read
	 * @param NotificationId The notification to mark
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Notifications")
	void MarkNotificationRead(int32 NotificationId);

	/**
	 * Clear all notifications
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Notifications")
	void ClearNotifications();

	/**
	 * Get all notifications
	 * @return Array of all notifications
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Notifications")
	TArray<FStationNotification> GetNotifications() const;

	/**
	 * Get unread notification count
	 * @return Number of unread notifications
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Notifications")
	int32 GetUnreadNotificationCount() const;

	// =====================
	// View Mode
	// =====================

	/**
	 * Set the current view mode
	 * @param NewMode The view mode to switch to
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|View")
	void SetViewMode(EStationEditorViewMode NewMode);

	/**
	 * Get the current view mode
	 * @return Current view mode
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|View")
	EStationEditorViewMode GetViewMode() const;

	// =====================
	// Module Upgrade
	// =====================

	/**
	 * Check if a module can be upgraded
	 * @param Module The module to check
	 * @return True if the module has an available upgrade
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Upgrade")
	bool CanUpgradeModule(ASpaceStationModule* Module) const;

	/**
	 * Get the upgrade cost for a module
	 * @param Module The module to check
	 * @param OutCost The upgrade cost
	 * @return True if module has an upgrade available
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Upgrade")
	bool GetUpgradeCost(ASpaceStationModule* Module, FStationBuildCost& OutCost) const;

	/**
	 * Upgrade a module to its next level
	 * @param Module The module to upgrade
	 * @return True if upgrade was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Station Editor|Upgrade")
	bool UpgradeModule(ASpaceStationModule* Module);

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

	/**
	 * Record an action for undo/redo
	 */
	void RecordAction(const FEditorAction& Action);

	/**
	 * Execute an action (for redo)
	 */
	bool ExecuteAction(const FEditorAction& Action);

	/**
	 * Reverse an action (for undo)
	 */
	bool ReverseAction(const FEditorAction& Action);

	/**
	 * Notify undo/redo state changed
	 */
	void NotifyUndoRedoStateChanged();

	/**
	 * Complete a construction item and spawn the module
	 */
	void CompleteConstruction(FConstructionQueueItem& Item);

	/**
	 * Generate a notification based on current station state
	 */
	void GenerateStatusNotifications();

	/**
	 * Internal helper to recalculate statistics (const-safe for mutable cache)
	 */
	void RecalculateStatisticsInternal() const;

private:
	/** Cached last power balance for change detection */
	float LastPowerBalance = 0.0f;

	/** Undo stack for editor actions */
	UPROPERTY()
	TArray<FEditorAction> UndoStack;

	/** Redo stack for editor actions */
	UPROPERTY()
	TArray<FEditorAction> RedoStack;

	/** Maximum size of undo/redo stacks */
	static constexpr int32 MaxUndoStackSize = 50;

	/** Module connections */
	UPROPERTY()
	TArray<FModuleConnection> Connections;

	/** Construction queue */
	UPROPERTY()
	TArray<FConstructionQueueItem> ConstructionQueue;

	/** Next queue item ID */
	int32 NextQueueId = 1;

	/** Notifications */
	UPROPERTY()
	TArray<FStationNotification> Notifications;

	/** Next notification ID */
	int32 NextNotificationId = 1;

	/** Current view mode */
	UPROPERTY()
	EStationEditorViewMode CurrentViewMode = EStationEditorViewMode::Edit;

	/** Cached station statistics (mutable for lazy recalculation in const getters) */
	mutable FStationStatistics CachedStatistics;

	/** Whether statistics need recalculation (mutable for lazy recalculation in const getters) */
	mutable bool bStatisticsDirty = true;

	/** Current time (for timestamps) */
	float CurrentTime = 0.0f;
};
