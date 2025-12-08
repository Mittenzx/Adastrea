#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ITargetable.generated.h"

/**
 * UInterface class for ITargetable
 * Required by Unreal's reflection system
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UTargetable : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that can be targeted by weapons and sensors
 * 
 * Implement this interface on:
 * - Spaceships (ASpaceship)
 * - Space stations (ASpaceStation)
 * - Asteroids and space objects
 * - Any entity that should appear in targeting systems
 * 
 * Benefits:
 * - Standardized targeting queries across all systems
 * - Easy integration with AI targeting algorithms
 * - Consistent UI display for target information
 * - Support for stealth and ECM mechanics
 * 
 * Usage Example (C++):
 * @code
 * if (PotentialTarget->Implements<UTargetable>())
 * {
 *     ITargetable* TargetInterface = Cast<ITargetable>(PotentialTarget);
 *     if (TargetInterface->Execute_CanBeTargeted(PotentialTarget))
 *     {
 *         FVector AimPoint = TargetInterface->Execute_GetAimPoint(PotentialTarget);
 *         // Aim weapon at this point
 *     }
 * }
 * @endcode
 * 
 * Usage Example (Blueprint):
 * - Check "Does Implement Interface" for ITargetable
 * - Call GetTargetDisplayName for UI
 * - Call GetAimPoint for weapon aiming
 */
class ADASTREA_API ITargetable
{
    GENERATED_BODY()

public:
    /**
     * Check if this actor can currently be targeted
     * 
     * Should return false if:
     * - Actor is cloaked/stealthed beyond detection range
     * - Actor is in a safe zone with targeting disabled
     * - Actor is friendly and friendly fire is disabled
     * - Actor is out of sensor range
     * 
     * @return True if the actor can be targeted, false otherwise
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    bool CanBeTargeted() const;

    /**
     * Get targeting priority for this actor
     * 
     * Higher values indicate higher threat priority. Used by AI to determine
     * which targets to engage first.
     * 
     * Priority guidelines:
     * - 0-25: Low priority (cargo ships, miners, damaged targets)
     * - 26-50: Medium priority (standard fighters, escorts)
     * - 51-75: High priority (bombers, armed freighters)
     * - 76-100: Critical priority (capital ships, mission objectives)
     * 
     * @return Priority value (0-100, higher = more important)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    int32 GetTargetPriority() const;

    /**
     * Get display name for this target
     * 
     * Used in:
     * - Targeting HUD
     * - Sensor displays
     * - Combat logs
     * - Mission objectives
     * 
     * @return Localized display name for the target
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    FText GetTargetDisplayName() const;

    /**
     * Get icon texture for this target type
     * 
     * Used in:
     * - Radar/minimap displays
     * - Target selection UI
     * - Tactical overview
     * 
     * @return Icon texture for UI display (can be nullptr)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    UTexture2D* GetTargetIcon() const;

    /**
     * Get optimal aim point on this target
     * 
     * This is where weapons should aim for best results. Typically:
     * - Center of mass for ships
     * - Command module for stations
     * - Weak points for armored targets
     * - Adjusted for target size and hitbox
     * 
     * @return World space location to aim at
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    FVector GetAimPoint() const;

    /**
     * Get target signature strength
     * 
     * Affects detection range and lock-on difficulty:
     * - 0.0: Completely stealthed (undetectable)
     * - 0.5: Reduced signature (harder to lock)
     * - 1.0: Normal signature
     * - 2.0+: Enhanced signature (easier to detect, military transponder active)
     * 
     * @return Signature multiplier (0.0 to 2.0+)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    float GetTargetSignature() const;

    /**
     * Get distance to this target from specified location
     * 
     * Helper function for targeting calculations
     * 
     * @param FromLocation Location to measure from
     * @return Distance in world units
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    float GetDistanceFromLocation(FVector FromLocation) const;

    /**
     * Check if this target is hostile to the specified actor
     * 
     * Used for IFF (Identification Friend or Foe) systems
     * 
     * @param Observer Actor checking hostility
     * @return True if target is hostile to observer
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    bool IsHostileToActor(AActor* Observer) const;
};
