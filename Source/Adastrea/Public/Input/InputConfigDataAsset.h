#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputConfigDataAsset.generated.h"

/**
 * Data Asset for configuring Enhanced Input system
 * 
 * Stores references to Input Actions and Input Mapping Contexts
 * for easy access and organization.
 * 
 * Usage:
 * - Create a Blueprint Data Asset based on UInputConfigDataAsset
 * - Assign Input Actions created in editor
 * - Assign Input Mapping Context
 * - Reference in player controller for input setup
 */
UCLASS(BlueprintType)
class ADASTREA_API UInputConfigDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UInputConfigDataAsset();

    // ====================
    // INPUT MAPPING CONTEXT
    // ====================

    /** Primary input mapping context for spaceship controls */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Mapping")
    UInputMappingContext* SpaceshipMappingContext;

    /** Input mapping context for UI/menu navigation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Mapping")
    UInputMappingContext* MenuMappingContext;

    /** Default priority for spaceship context */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Mapping", meta=(ClampMin="0"))
    int32 SpaceshipContextPriority;

    /** Default priority for menu context */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Mapping", meta=(ClampMin="0"))
    int32 MenuContextPriority;

    // ====================
    // MOVEMENT INPUT ACTIONS
    // ====================

    /** Input action for movement (WASD/Left Stick) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* MoveAction;

    /** Input action for look/camera (Mouse/Right Stick) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* LookAction;

    /** Input action for boost/afterburner */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* BoostAction;

    /** Input action for brake/reverse */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* BrakeAction;

    /** Input action for roll */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* RollAction;

    /** Input action for throttle up (increase target velocity) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* ThrottleUpAction;

    /** Input action for throttle down (decrease target velocity) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* ThrottleDownAction;

    /** Input action for toggle flight assist mode */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* ToggleFlightAssistAction;

    /** Input action for toggle travel mode (high-speed cruise) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Movement")
    UInputAction* ToggleTravelModeAction;

    // ====================
    // COMBAT INPUT ACTIONS
    // ====================

    /** Input action for primary weapon fire */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Combat")
    UInputAction* FirePrimaryAction;

    /** Input action for secondary weapon fire */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Combat")
    UInputAction* FireSecondaryAction;

    /** Input action for target lock/cycle target */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Combat")
    UInputAction* TargetLockAction;

    /** Input action for next target */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Combat")
    UInputAction* NextTargetAction;

    /** Input action for previous target */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Combat")
    UInputAction* PreviousTargetAction;

    /** Input action for nearest target */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Combat")
    UInputAction* NearestTargetAction;

    // ====================
    // INTERACTION INPUT ACTIONS
    // ====================

    /** Input action for interact with objects/stations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Interaction")
    UInputAction* InteractAction;

    /** Input action for dock with station */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Interaction")
    UInputAction* DockAction;

    /** Input action for board/capture ship */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Interaction")
    UInputAction* BoardAction;

    // ====================
    // NAVIGATION INPUT ACTIONS
    // ====================

    /** Input action for autopilot toggle */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Navigation")
    UInputAction* AutopilotAction;

    /** Input action for set waypoint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Navigation")
    UInputAction* SetWaypointAction;

    /** Input action for clear waypoint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Navigation")
    UInputAction* ClearWaypointAction;

    /** Input action for next waypoint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Navigation")
    UInputAction* NextWaypointAction;

    // ====================
    // SYSTEM INPUT ACTIONS
    // ====================

    /** Input action for open menu/pause */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|System")
    UInputAction* MenuAction;

    /** Input action for open map */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|System")
    UInputAction* MapAction;

    /** Input action for open inventory */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|System")
    UInputAction* InventoryAction;

    /** Input action for open quest log */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|System")
    UInputAction* QuestLogAction;

    /** Input action for screenshot */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|System")
    UInputAction* ScreenshotAction;

    // ====================
    // CAMERA INPUT ACTIONS
    // ====================

    /** Input action for cycle camera view */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Camera")
    UInputAction* CycleCameraAction;

    /** Input action for zoom in */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Camera")
    UInputAction* ZoomInAction;

    /** Input action for zoom out */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Actions|Camera")
    UInputAction* ZoomOutAction;

    // ====================
    // QUERY FUNCTIONS
    // ====================

    /**
     * Get spaceship mapping context
     * @return Spaceship input mapping context
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Input Config")
    UInputMappingContext* GetSpaceshipMappingContext() const;

    /**
     * Get menu mapping context
     * @return Menu input mapping context
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Input Config")
    UInputMappingContext* GetMenuMappingContext() const;

    /**
     * Check if input config is valid (all required actions assigned)
     * @return True if config has all essential input actions
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Input Config")
    bool IsValid() const;

    /**
     * Get list of unassigned input actions
     * @return Array of action names that are not assigned
     */
    UFUNCTION(BlueprintCallable, Category="Input Config")
    TArray<FString> GetUnassignedActions() const;
};
