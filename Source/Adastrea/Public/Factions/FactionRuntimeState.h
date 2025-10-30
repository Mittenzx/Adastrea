#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Factions/FactionDataAsset.h"
#include "FactionRuntimeState.generated.h"

/**
 * Runtime state component for managing dynamic faction data.
 * Bridges the static FactionDataAsset templates with runtime gameplay state.
 * 
 * This component stores dynamic data such as:
 * - Current reputation values with player
 * - Dynamic relationship changes with other factions
 * - Runtime trait additions/removals
 * - Faction territorial control
 * - Active diplomatic states
 * 
 * Usage:
 * - Attach to a faction manager actor or game mode
 * - Initialize with a FactionDataAsset
 * - Use Blueprint functions to modify runtime state
 */
UCLASS(BlueprintType, ClassGroup=(Faction), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UFactionRuntimeState : public UActorComponent
{
    GENERATED_BODY()

public:
    UFactionRuntimeState();

    // ====================
    // Core Properties
    // ====================

    /** The static faction data asset this runtime state is based on */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction")
    UFactionDataAsset* FactionTemplate;

    /** Current reputation with the player (-100 to 100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction|Reputation", meta=(ClampMin="-100", ClampMax="100"))
    int32 PlayerReputation;

    /** Dynamic relationships with other factions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction|Diplomacy")
    TArray<FFactionRelationship> DynamicRelationships;

    /** Traits that have been added at runtime (not in the template) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction|Traits")
    TArray<FFactionTrait> RuntimeTraits;

    /** Territory sectors controlled by this faction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction|Territory")
    TArray<FName> ControlledSectors;

    /** Whether this faction is currently at war with the player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction|Diplomacy")
    bool bAtWarWithPlayer;

    /** Whether this faction is currently allied with the player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction|Diplomacy")
    bool bAlliedWithPlayer;

    // ====================
    // Initialization
    // ====================

    /**
     * Initialize runtime state from a faction template
     * @param InFactionTemplate The static faction data asset to base state on
     */
    UFUNCTION(BlueprintCallable, Category="Faction")
    void InitializeFromTemplate(UFactionDataAsset* InFactionTemplate);

    // ====================
    // Reputation System
    // ====================

    /**
     * Modify player reputation with this faction
     * @param Delta Amount to change reputation by (can be negative)
     * @param bClamp Whether to clamp the result to -100/+100 range
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Reputation")
    void ModifyPlayerReputation(int32 Delta, bool bClamp = true);

    /**
     * Set player reputation to a specific value
     * @param NewReputation The new reputation value
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Reputation")
    void SetPlayerReputation(int32 NewReputation);

    /**
     * Get current player reputation
     * @return Current reputation value (-100 to 100)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Reputation")
    int32 GetPlayerReputation() const;

    /**
     * Check if player reputation meets a threshold
     * @param Threshold The reputation threshold to check
     * @return True if current reputation >= threshold
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Reputation")
    bool IsReputationAtLeast(int32 Threshold) const;

    // ====================
    // Dynamic Relationships
    // ====================

    /**
     * Update relationship with another faction at runtime
     * @param OtherFactionID The faction to update relationship with
     * @param NewValue New relationship value
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Diplomacy")
    void UpdateFactionRelationship(FName OtherFactionID, int32 NewValue);

    /**
     * Get current relationship value with another faction (checks both template and runtime)
     * @param OtherFactionID The faction to check
     * @return Relationship value, or 0 if not found
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Diplomacy")
    int32 GetFactionRelationshipValue(FName OtherFactionID) const;

    /**
     * Declare war with another faction
     * @param OtherFactionID The faction to declare war on
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Diplomacy")
    void DeclareWar(FName OtherFactionID);

    /**
     * Make peace with another faction
     * @param OtherFactionID The faction to make peace with
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Diplomacy")
    void MakePeace(FName OtherFactionID);

    /**
     * Form alliance with another faction
     * @param OtherFactionID The faction to ally with
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Diplomacy")
    void FormAlliance(FName OtherFactionID);

    /**
     * Break alliance with another faction
     * @param OtherFactionID The faction to break alliance with
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Diplomacy")
    void BreakAlliance(FName OtherFactionID);

    // ====================
    // Runtime Traits
    // ====================

    /**
     * Add a trait at runtime
     * @param NewTrait The trait to add
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Traits")
    void AddRuntimeTrait(const FFactionTrait& NewTrait);

    /**
     * Remove a runtime trait by ID
     * @param TraitID The ID of the trait to remove
     * @return True if trait was found and removed
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Traits")
    bool RemoveRuntimeTrait(FName TraitID);

    /**
     * Get all active traits (template + runtime)
     * @return Combined array of all active traits
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Traits")
    TArray<FFactionTrait> GetAllActiveTraits() const;

    /**
     * Check if faction has a specific trait (checks both template and runtime)
     * @param TraitID The trait ID to check for
     * @return True if trait is active
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Traits")
    bool HasActiveTrait(FName TraitID) const;

    // ====================
    // Territory Management
    // ====================

    /**
     * Add a sector to faction control
     * @param SectorID The sector to add
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Territory")
    void AddControlledSector(FName SectorID);

    /**
     * Remove a sector from faction control
     * @param SectorID The sector to remove
     * @return True if sector was controlled and removed
     */
    UFUNCTION(BlueprintCallable, Category="Faction|Territory")
    bool RemoveControlledSector(FName SectorID);

    /**
     * Check if faction controls a specific sector
     * @param SectorID The sector to check
     * @return True if faction controls the sector
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Territory")
    bool ControlsSector(FName SectorID) const;

    /**
     * Get count of controlled sectors
     * @return Number of sectors under faction control
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Territory")
    int32 GetControlledSectorCount() const;

protected:
    /**
     * Find or create a dynamic relationship entry
     * @param OtherFactionID The faction to find/create relationship for
     * @return Pointer to the relationship entry
     */
    FFactionRelationship* FindOrCreateRelationship(FName OtherFactionID);
};
