#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
// TODO: Combat system archived - WeaponDataAsset (for damage types) will be reimplemented in MVP
// For now, EDamageType is temporarily defined in IDamageable.h
// #include "Combat/WeaponDataAsset.h"
#include "AdastreaFunctionLibrary.generated.h"

// Forward declarations
// REMOVED: UFactionDataAsset - faction system removed per Trade Simulator MVP
class UDataAsset;
// REMOVED: IFactionMember - faction system removed per Trade Simulator MVP
// Forward declare damage type enum (defined in IDamageable.h)
enum class EDamageType : uint8;

/**
 * Blueprint Function Library for Adastrea
 * 
 * Provides commonly used utility functions for:
 * - Distance and position calculations
 * - Combat calculations (damage, lead targeting)
 * - Data validation helpers
 * - String formatting utilities
 * 
 * REMOVED (Trade Simulator MVP):
 * - Faction relationship queries (removed per MVP scope)
 * 
 * All functions are static and BlueprintCallable for easy use in both C++ and Blueprints.
 * 
 * Usage Example (Blueprint):
 * - Drag from any actor node
 * - Search for "Adastrea" to see all utility functions
 * - Use categorized functions for specific tasks
 * 
 * Usage Example (C++):
 * @code
 * float Distance = UAdastreaFunctionLibrary::GetDistance2D(ActorA, ActorB);
 * bool InRange = UAdastreaFunctionLibrary::IsWithinRange(Ship, Target, 1000.0f);
 * @endcode
 */
UCLASS()
class ADASTREA_API UAdastreaFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ====================
    // DISTANCE & POSITION UTILITIES
    // ====================

    /**
     * Get 2D distance between two actors (ignoring Z axis)
     * Useful for:
     * - Planar distance calculations
     * - Top-down distance checks
     * - Minimizing vertical distance influence
     * 
     * @param ActorA First actor
     * @param ActorB Second actor
     * @return Distance in world units, or -1.0 if either actor is invalid
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Position", meta=(Keywords="distance 2d flat planar"))
    static float GetDistance2D(AActor* ActorA, AActor* ActorB);

    /**
     * Check if two actors are within specified range
     * 
     * @param ActorA First actor
     * @param ActorB Second actor
     * @param Range Maximum distance threshold
     * @return True if actors are within range, false otherwise
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Position", meta=(Keywords="distance range near close"))
    static bool IsWithinRange(AActor* ActorA, AActor* ActorB, float Range);

    /**
     * Get the closest actor to origin from an array of actors
     * 
     * @param Origin Reference point to measure from
     * @param Actors Array of actors to search
     * @return Closest actor, or nullptr if array is empty
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Position", meta=(Keywords="closest nearest find"))
    static AActor* GetClosestActor(AActor* Origin, const TArray<AActor*>& Actors);

    /**
     * Get all actors within a sphere radius
     * 
     * @param WorldContextObject World context for the query
     * @param Center Center of the sphere
     * @param Radius Sphere radius
     * @param ActorClass Optional class filter (nullptr = all actors)
     * @return Array of actors within the sphere
     */
    UFUNCTION(BlueprintCallable, Category="Adastrea|Utilities|Position", meta=(WorldContext="WorldContextObject", Keywords="sphere radius area"))
    static TArray<AActor*> GetActorsWithinSphere(
        const UObject* WorldContextObject,
        FVector Center,
        float Radius,
        TSubclassOf<AActor> ActorClass = nullptr
    );

    /**
     * Calculate direction vector from one actor to another
     * 
     * @param From Source actor
     * @param To Target actor
     * @return Normalized direction vector, or zero vector if invalid
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Position", meta=(Keywords="direction vector aim"))
    static FVector GetDirectionToActor(AActor* From, AActor* To);

    // ====================
    // FACTION UTILITIES
    // ====================

    // REMOVED: Faction utility functions - faction system removed per Trade Simulator MVP
    // - AreFactionsAllied()
    // - AreFactionsHostile()
    // - GetFactionRelationship()
    // - GetActorFaction()

    // ====================
    // COMBAT UTILITIES
    // ====================

    /**
     * Calculate damage after armor reduction
     * Formula considers armor penetration and damage type effectiveness
     * 
     * @param RawDamage Base damage value
     * @param ArmorValue Target's armor rating
     * @param DamageType Type of damage being dealt
     * @return Final damage after armor mitigation
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Combat", meta=(Keywords="damage armor penetration"))
    static float CalculateDamageAfterArmor(float RawDamage, float ArmorValue, EDamageType DamageType);

    /**
     * Calculate optimal firing solution (lead target calculation)
     * Predicts where target will be and returns aim point
     * 
     * @param ShooterLocation Current weapon/shooter location
     * @param TargetLocation Current target location
     * @param TargetVelocity Target's velocity vector
     * @param ProjectileSpeed Speed of the projectile
     * @return World position to aim at for intercept
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Combat", meta=(Keywords="lead target aim predict intercept"))
    static FVector CalculateLeadTarget(
        FVector ShooterLocation,
        FVector TargetLocation,
        FVector TargetVelocity,
        float ProjectileSpeed
    );

    /**
     * Check if target is within weapon's firing arc
     * 
     * @param ShooterLocation Weapon mount location
     * @param ShooterRotation Weapon mount rotation
     * @param TargetLocation Target location to check
     * @param ArcAngleDegrees Firing arc angle in degrees (full cone angle)
     * @return True if target is within arc, false otherwise
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Combat", meta=(Keywords="firing arc angle cone"))
    static bool IsInFiringArc(
        FVector ShooterLocation,
        FRotator ShooterRotation,
        FVector TargetLocation,
        float ArcAngleDegrees
    );

    /**
     * Calculate time to impact for a projectile
     * 
     * @param Distance Distance to target
     * @param ProjectileSpeed Projectile speed
     * @return Time in seconds until impact
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Combat", meta=(Keywords="time impact projectile"))
    static float CalculateTimeToImpact(float Distance, float ProjectileSpeed);

    // ====================
    // DATA VALIDATION UTILITIES
    // ====================

    /**
     * Validate that a Data Asset has required properties set
     * Basic validation for null checks and empty values
     * 
     * @param DataAsset Data Asset to validate
     * @param OutErrorMessage Detailed error message if validation fails
     * @return True if valid, false if validation errors found
     */
    UFUNCTION(BlueprintCallable, Category="Adastrea|Utilities|Validation", meta=(Keywords="validate check data asset"))
    static bool ValidateDataAsset(UDataAsset* DataAsset, FString& OutErrorMessage);

    /**
     * Clamp a value to specified range (min/max)
     * 
     * @param Value Value to clamp
     * @param Min Minimum allowed value
     * @param Max Maximum allowed value
     * @return Clamped value
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Math", meta=(Keywords="clamp min max range"))
    static float ClampValue(float Value, float Min, float Max);

    /**
     * Clamp an integer value to specified range
     * 
     * @param Value Value to clamp
     * @param Min Minimum allowed value
     * @param Max Maximum allowed value
     * @return Clamped value
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Math", meta=(Keywords="clamp min max range"))
    static int32 ClampValueInt(int32 Value, int32 Min, int32 Max);

    // ====================
    // STRING UTILITIES
    // ====================

    /**
     * Format large number with thousands separators
     * Example: 1000 -> "1,000", 1000000 -> "1,000,000"
     * 
     * @param Number Number to format
     * @return Formatted string with commas
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|String", meta=(Keywords="format number comma thousand"))
    static FString FormatLargeNumber(int32 Number);

    /**
     * Format floating point number with thousands separators
     * Example: 1000.5 -> "1,000.5"
     * 
     * @param Number Number to format
     * @param DecimalPlaces Number of decimal places to display
     * @return Formatted string with commas
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|String", meta=(Keywords="format number comma decimal"))
    static FString FormatLargeNumberFloat(float Number, int32 DecimalPlaces = 2);

    /**
     * Format percentage value for display
     * Example: 0.75 -> "75%", 0.5 -> "50%"
     * 
     * @param Percentage Percentage value (0.0 to 1.0)
     * @param DecimalPlaces Number of decimal places to show
     * @return Formatted percentage string
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|String", meta=(Keywords="format percentage percent"))
    static FString FormatPercentage(float Percentage, int32 DecimalPlaces = 0);

    /**
     * Format time duration in seconds to readable format
     * Example: 90 -> "1m 30s", 3665 -> "1h 1m 5s"
     * 
     * @param Seconds Time duration in seconds
     * @return Formatted time string
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|String", meta=(Keywords="format time duration"))
    static FString FormatDuration(float Seconds);

    // ====================
    // MATH UTILITIES
    // ====================

    /**
     * Remap a value from one range to another
     * Example: RemapValue(5, 0, 10, 0, 100) = 50
     * 
     * @param Value Value to remap
     * @param InMin Input range minimum
     * @param InMax Input range maximum
     * @param OutMin Output range minimum
     * @param OutMax Output range maximum
     * @return Remapped value
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Math", meta=(Keywords="remap scale range"))
    static float RemapValue(float Value, float InMin, float InMax, float OutMin, float OutMax);

    /**
     * Calculate percentage of value within range
     * Example: GetPercentageInRange(5, 0, 10) = 0.5 (50%)
     * 
     * @param Value Current value
     * @param Min Range minimum
     * @param Max Range maximum
     * @return Percentage from 0.0 to 1.0
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Math", meta=(Keywords="percentage ratio range"))
    static float GetPercentageInRange(float Value, float Min, float Max);
};
