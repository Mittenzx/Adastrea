#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Stations/SpaceStation.h"
#include "Factions/FactionDataAsset.h"
#include "../StationEditorManager.h"
#include "StationEditorWidget.generated.h"

// Forward declarations
class ASpaceStationModule;

/**
 * Station Editor Widget - UI for managing space station modules
 * This class provides C++ hooks for Blueprint UI implementation
 * 
 * This is a runtime, player-facing UI widget that allows players to
 * construct and modify space stations in-game.
 * 
 * Features:
 * - Module placement, removal, and movement
 * - Faction integration
 * - Undo/Redo support
 * - Construction queue management
 * - Module connections visualization
 * - Station statistics display
 * - Notification system
 */
UCLASS()
class STATIONEDITOR_API UStationEditorWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UStationEditorWidget(const FObjectInitializer& ObjectInitializer);

    // Reference to the station being edited
    UPROPERTY(BlueprintReadWrite, Category="Station Editor")
    ASpaceStation* CurrentStation;

    // Reference to the editor manager
    UPROPERTY(BlueprintReadWrite, Category="Station Editor")
    UStationEditorManager* EditorManager;

    // Add a module to the current station
    // @param ModuleClass - The class of module to spawn
    // @param RelativeLocation - Location relative to the station center
    // @return The newly added module, or nullptr if failed
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    ASpaceStationModule* AddModule(TSubclassOf<ASpaceStationModule> ModuleClass, FVector RelativeLocation);

    // Remove a module from the current station
    // @param Module - The module to remove
    // @return true if the module was successfully removed
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    bool RemoveModule(ASpaceStationModule* Module);

    // Move a module to a new location within the station
    // @param Module - The module to move
    // @param NewRelativeLocation - New location relative to the station center
    // @return true if the module was successfully moved
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);

    // Get a list of all modules currently attached to the station
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    TArray<ASpaceStationModule*> GetAllModules();

    // Set the station to be edited
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    void SetStation(ASpaceStation* Station);

    // Helper function to validate module placement
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    bool IsValidPlacement(FVector Location);

    // ====================
    // Faction Integration Hooks
    // ====================

    /**
     * Set the faction for the current station
     * @param NewFaction The faction to assign to the station
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Faction")
    void SetStationFaction(UFactionDataAsset* NewFaction);

    /**
     * Get available module types that match the station's faction technology level
     * @return Array of module types that can be built
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Faction")
    TArray<FString> GetAvailableModuleTypes() const;

    /**
     * Check if a module can be added based on faction restrictions
     * @param ModuleClass The module class to check
     * @return True if the module can be added
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Faction")
    bool CanAddModuleForFaction(TSubclassOf<ASpaceStationModule> ModuleClass) const;

    // ====================
    // Undo/Redo
    // ====================

    /**
     * Undo the last editor action
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

    // ====================
    // Construction Queue
    // ====================

    /**
     * Add a module to the construction queue
     * @param ModuleClass Module class to build
     * @param Position Target position
     * @return Queue item ID, or -1 if failed
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Construction")
    int32 QueueConstruction(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position);

    /**
     * Cancel a construction item
     * @param QueueId ID of the item to cancel
     * @return True if cancelled successfully
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Construction")
    bool CancelConstruction(int32 QueueId);

    /**
     * Get the construction queue
     * @return Array of queued items
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Construction")
    TArray<FConstructionQueueItem> GetConstructionQueue() const;

    // ====================
    // Statistics
    // ====================

    /**
     * Get station statistics
     * @return Current station statistics
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Statistics")
    FStationStatistics GetStationStatistics() const;

    /**
     * Get power balance
     * @return Power balance (positive = surplus)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Statistics")
    float GetPowerBalance() const;

    /**
     * Get defense rating
     * @return Defense rating (0-100)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Statistics")
    float GetDefenseRating() const;

    // ====================
    // Connections
    // ====================

    /**
     * Get all module connections
     * @return Array of all connections
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Connections")
    TArray<FModuleConnection> GetAllConnections() const;

    /**
     * Get connections for a specific module
     * @param Module The module to query
     * @return Array of connections involving this module
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Connections")
    TArray<FModuleConnection> GetModuleConnections(ASpaceStationModule* Module) const;

    // ====================
    // Notifications
    // ====================

    /**
     * Get all notifications
     * @return Array of notifications
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Notifications")
    TArray<FStationNotification> GetNotifications() const;

    /**
     * Get unread notification count
     * @return Number of unread notifications
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Notifications")
    int32 GetUnreadNotificationCount() const;

    /**
     * Mark a notification as read
     * @param NotificationId ID of the notification
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Notifications")
    void MarkNotificationRead(int32 NotificationId);

    // ====================
    // View Mode
    // ====================

    /**
     * Set the view mode
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

    // ====================
    // Module Upgrade
    // ====================

    /**
     * Check if a module can be upgraded
     * @param Module The module to check
     * @return True if upgrade is available
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Editor|Upgrade")
    bool CanUpgradeModule(ASpaceStationModule* Module) const;

    /**
     * Upgrade a module
     * @param Module The module to upgrade
     * @return True if upgrade was successful
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Upgrade")
    bool UpgradeModule(ASpaceStationModule* Module);

protected:
    // Called when the widget is constructed
    virtual void NativeConstruct() override;

    // Called every frame
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    /**
     * Ensure EditorManager exists and is valid
     */
    void EnsureEditorManager();
};
