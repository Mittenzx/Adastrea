#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DockingSettingsDataAsset.generated.h"

class UUserWidget;
class UCurveFloat;

/**
 * Centralized docking configuration settings
 * 
 * This Data Asset allows configuring docking parameters once and sharing them
 * across all ships, eliminating the need to configure docking on every ship.
 * 
 * Usage:
 * 1. Create a Blueprint based on this class (e.g., DA_DefaultDockingSettings)
 * 2. Configure docking parameters (UI widgets, range, curve)
 * 3. Reference this asset in ship Blueprints instead of configuring individually
 * 
 * Benefits:
 * - Single source of truth for docking configuration
 * - Easy to update all ships at once
 * - Consistent docking behavior across all ships
 * - Designer-friendly configuration
 */
UCLASS(BlueprintType)
class ADASTREA_API UDockingSettingsDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UDockingSettingsDataAsset();

    // ====================
    // UI CONFIGURATION
    // ====================

    /** Widget class for docking prompt UI (shows "Press F to Dock") */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking|UI",
        meta=(ToolTip="Widget shown when player is near a docking point"))
    TSubclassOf<UUserWidget> DockingPromptWidgetClass;

    /** Widget class for trading interface UI (opens when docked) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking|UI",
        meta=(ToolTip="Widget shown when player docks at a station with marketplace"))
    TSubclassOf<UUserWidget> TradingInterfaceClass;

    // ====================
    // DOCKING PARAMETERS
    // ====================

    /** Maximum distance from docking point to allow docking (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking|Parameters",
        meta=(ClampMin="100.0", ClampMax="10000.0", UIMin="500.0", UIMax="5000.0",
        ToolTip="How close the ship needs to be to dock (default: 2000 cm)"))
    float DockingRange;

    /** Animation curve for smooth docking movement (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking|Parameters",
        meta=(ToolTip="Curve controlling docking animation speed (cubic ease recommended)"))
    TObjectPtr<UCurveFloat> DockingCurve;

    /** Time to complete docking animation in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking|Parameters",
        meta=(ClampMin="0.1", ClampMax="10.0", UIMin="0.5", UIMax="3.0",
        ToolTip="How long the docking animation takes (default: 1.0 seconds)"))
    float DockingDuration;

    /** Undock impulse strength (cm/s) - how fast ship moves away when undocking */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking|Parameters",
        meta=(ClampMin="0.0", ClampMax="5000.0", UIMin="100.0", UIMax="1000.0",
        ToolTip="Forward velocity applied when undocking (default: 500 cm/s)"))
    float UndockImpulseStrength;

    // ====================
    // VALIDATION
    // ====================

#if WITH_EDITOR
    /**
     * Validate Data Asset properties
     * Ensures all required widget classes are set and parameters are reasonable
     */
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
