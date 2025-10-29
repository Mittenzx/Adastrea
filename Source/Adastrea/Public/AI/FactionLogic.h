#pragma once

#include "CoreMinimal.h"
#include "AI/NPCLogicBase.h"
#include "Factions/FactionDataAsset.h"
#include "FactionLogic.generated.h"

/**
 * Faction-level strategic priority for macro AI decisions
 */
UENUM(BlueprintType)
enum class EFactionPriority : uint8
{
    Survival UMETA(DisplayName = "Survival"),              // Faction under existential threat
    Expansion UMETA(DisplayName = "Expansion"),            // Territory expansion
    Exploration UMETA(DisplayName = "Exploration"),        // Discovery and mapping
    Trade UMETA(DisplayName = "Trade"),                    // Economic development
    Research UMETA(DisplayName = "Research"),              // Technology advancement
    Diplomacy UMETA(DisplayName = "Diplomacy"),            // Alliance building
    Defense UMETA(DisplayName = "Defense"),                // Fortification and security
    Conquest UMETA(DisplayName = "Conquest")               // Military expansion
};

/**
 * Early game priority configuration for peaceful/exploration gameplay
 * Defines what a faction focuses on in the early stages of the game
 */
USTRUCT(BlueprintType)
struct FEarlyGamePriorities
{
    GENERATED_BODY()

    /** Priority level for exploration activities */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Early Game", meta = (ClampMin = "0", ClampMax = "10"))
    int32 ExplorationPriority;

    /** Priority level for peaceful trade activities */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Early Game", meta = (ClampMin = "0", ClampMax = "10"))
    int32 TradePriority;

    /** Priority level for diplomatic relations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Early Game", meta = (ClampMin = "0", ClampMax = "10"))
    int32 DiplomacyPriority;

    /** Priority level for research and technology */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Early Game", meta = (ClampMin = "0", ClampMax = "10"))
    int32 ResearchPriority;

    /** Priority level for territorial expansion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Early Game", meta = (ClampMin = "0", ClampMax = "10"))
    int32 ExpansionPriority;

    /** Priority level for defensive operations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Early Game", meta = (ClampMin = "0", ClampMax = "10"))
    int32 DefensePriority;

    FEarlyGamePriorities()
        : ExplorationPriority(7)  // High priority for early game
        , TradePriority(6)        // Medium-high for economy
        , DiplomacyPriority(5)    // Medium for relationships
        , ResearchPriority(4)     // Medium for tech
        , ExpansionPriority(3)    // Low-medium for territory
        , DefensePriority(2)      // Low for peaceful factions
    {}
};

/**
 * Faction Logic - Handles macro-level AI for entire factions
 * 
 * This class manages faction-wide strategic decisions, including:
 * - Territory expansion and control
 * - Diplomatic relations with other factions
 * - Economic and trade policies
 * - Military strategies
 * - Research priorities
 * 
 * Designed for peaceful/exploration early-game gameplay with support for:
 * - Non-aggressive expansion
 * - Trade-focused economy
 * - Diplomatic relationship building
 * - Scientific discovery priorities
 * 
 * Blueprint Usage:
 * 1. Create a Blueprint based on UFactionLogic
 * 2. Assign a FactionDataAsset reference
 * 3. Configure early game priorities
 * 4. Override Event functions to customize behavior
 * 5. Add to your faction management system
 */
UCLASS(Blueprintable, BlueprintType)
class ADASTREA_API UFactionLogic : public UNPCLogicBase
{
    GENERATED_BODY()

public:
    UFactionLogic();

    // ====================
    // Faction-Specific Properties
    // ====================

    /** Reference to the faction's data asset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction|Data")
    UFactionDataAsset* FactionData;

    /** Early game priorities for this faction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction|Priorities")
    FEarlyGamePriorities EarlyGamePriorities;

    /** Current strategic priority */
    UPROPERTY(BlueprintReadOnly, Category = "Faction|State")
    EFactionPriority CurrentStrategicPriority;

    /** List of faction IDs with active truces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction|Diplomacy")
    TArray<FName> TruceList;

    /** Whether faction is in early game phase (peaceful/exploration focused) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction|State")
    bool bIsEarlyGame;

    /** Number of territories controlled by this faction */
    UPROPERTY(BlueprintReadWrite, Category = "Faction|State")
    int32 TerritoryCount;

    /** Current economic strength (0-100) */
    UPROPERTY(BlueprintReadWrite, Category = "Faction|State", meta = (ClampMin = "0", ClampMax = "100"))
    float EconomicStrength;

    /** Current military power (0-100) */
    UPROPERTY(BlueprintReadWrite, Category = "Faction|State", meta = (ClampMin = "0", ClampMax = "100"))
    float MilitaryPower;

    /** Current diplomatic influence (0-100) */
    UPROPERTY(BlueprintReadWrite, Category = "Faction|State", meta = (ClampMin = "0", ClampMax = "100"))
    float DiplomaticInfluence;

    // ====================
    // BlueprintNativeEvent Hooks - Faction-Specific
    // ====================

    /**
     * Evaluate faction's strategic priorities
     * Override to implement custom strategic decision making
     * 
     * @return The current top strategic priority
     * 
     * Example Blueprint usage:
     * - Check territory count vs neighbors
     * - Evaluate economic needs
     * - Consider diplomatic situation
     * - Return appropriate priority
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Strategy")
    EFactionPriority EvaluateStrategicPriority();
    virtual EFactionPriority EvaluateStrategicPriority_Implementation();

    /**
     * Handle diplomatic interactions with another faction
     * Override to customize diplomatic behavior
     * 
     * @param OtherFactionID The faction to interact with
     * @param bInitiatedByUs Whether we initiated the interaction
     * 
     * Example Blueprint usage:
     * - Check relationship with target faction
     * - Consider strategic priorities
     * - Decide on diplomatic action (trade, alliance, etc.)
     * - Update relationships accordingly
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Diplomacy")
    void HandleDiplomaticInteraction(FName OtherFactionID, bool bInitiatedByUs);
    virtual void HandleDiplomaticInteraction_Implementation(FName OtherFactionID, bool bInitiatedByUs);

    /**
     * Called when faction discovers a new territory
     * Override to customize exploration response
     * 
     * @param TerritoryName Name of the discovered territory
     * @param TerritoryValue Strategic value of the territory (0-100)
     * 
     * Example Blueprint usage:
     * - Evaluate territory value vs current needs
     * - Decide whether to claim or ignore
     * - Consider diplomatic implications
     * - Update exploration priorities
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Exploration")
    void OnTerritoryDiscovered(const FString& TerritoryName, float TerritoryValue);
    virtual void OnTerritoryDiscovered_Implementation(const FString& TerritoryName, float TerritoryValue);

    /**
     * Evaluate whether to initiate trade with another faction
     * Override to customize trade behavior
     * 
     * @param OtherFactionID The potential trade partner
     * @return True if should initiate trade
     * 
     * Example Blueprint usage:
     * - Check relationship with target faction
     * - Verify not on truce list (if truce prevents trade)
     * - Evaluate economic benefit
     * - Consider trade priorities
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Economy")
    bool ShouldInitiateTrade(FName OtherFactionID);
    virtual bool ShouldInitiateTrade_Implementation(FName OtherFactionID);

    /**
     * Determine if faction should pursue peaceful expansion
     * Override for custom expansion logic
     * 
     * @return True if should expand peacefully
     * 
     * Example Blueprint usage:
     * - Check if in early game phase
     * - Verify peaceful behavior mode
     * - Check expansion priority
     * - Ensure no aggressive neighbors
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Strategy")
    bool ShouldPursuepeacefulExpansion();
    virtual bool ShouldPursuepeacefulExpansion_Implementation();

    /**
     * Update faction strategic goals based on current state
     * Override to implement custom goal adjustment
     * 
     * Example Blueprint usage:
     * - Re-evaluate priorities based on game state
     * - Adjust early game priorities as game progresses
     * - Update truce list based on relationships
     * - Shift behavior mode if needed
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Strategy")
    void UpdateStrategicGoals();
    virtual void UpdateStrategicGoals_Implementation();

    // ====================
    // Blueprint-Callable Utility Functions
    // ====================

    /**
     * Check if faction has a truce with another faction
     * @param OtherFactionID The faction to check
     * @return True if there's an active truce
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Diplomacy")
    bool HasTruceWith(FName OtherFactionID) const;

    /**
     * Add a faction to the truce list
     * @param OtherFactionID The faction to add truce with
     */
    UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
    void AddTruce(FName OtherFactionID);

    /**
     * Remove a faction from the truce list
     * @param OtherFactionID The faction to remove truce with
     */
    UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
    void RemoveTruce(FName OtherFactionID);

    /**
     * Check if faction can peacefully interact with another faction
     * Considers truces, relationships, and behavior mode
     * @param OtherFactionID The faction to check
     * @return True if peaceful interaction is possible
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Diplomacy")
    bool CanPeacefullyInteract(FName OtherFactionID) const;

    /**
     * Get the highest priority early game action
     * @return The priority type with highest value
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Strategy")
    EFactionPriority GetTopEarlyGamePriority() const;

    /**
     * Check if faction is focused on exploration
     * @return True if exploration is a top priority
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Strategy")
    bool IsExplorationFocused() const;

    /**
     * Check if faction is focused on trade
     * @return True if trade is a top priority
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Strategy")
    bool IsTradeFocused() const;

    /**
     * Check if faction is focused on diplomacy
     * @return True if diplomacy is a top priority
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Strategy")
    bool IsDiplomacyFocused() const;

    /**
     * Get faction name from data asset
     * @return Faction name or "Unknown" if no data asset
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Info")
    FText GetFactionName() const;

    /**
     * Get a description of current faction state
     * @return Text description of faction's current state and priorities
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Info")
    FString GetFactionStateDescription() const;

    // ====================
    // Overridden from UNPCLogicBase
    // ====================

    virtual void InitializeAI_Implementation() override;
    virtual void OnTickAI_Implementation(float DeltaTime) override;
    virtual EAIPriority EvaluateCurrentPriority_Implementation() override;
};
