#pragma once

#include "CoreMinimal.h"
#include "AI/NPCLogicBase.h"
#include "Factions/FactionDataAsset.h"
#include "FactionLogic.generated.h"

/**
 * Strategic action categories for faction-level decisions
 */
UENUM(BlueprintType)
enum class EFactionStrategyType : uint8
{
    Expansion UMETA(DisplayName = "Expansion"),           // Territory and influence growth
    Consolidation UMETA(DisplayName = "Consolidation"),   // Strengthen existing holdings
    Diplomacy UMETA(DisplayName = "Diplomacy"),           // Build relationships
    Economic UMETA(DisplayName = "Economic"),             // Focus on trade and resources
    Military UMETA(DisplayName = "Military"),             // Build military strength
    Research UMETA(DisplayName = "Research"),             // Technology advancement
    Intelligence UMETA(DisplayName = "Intelligence"),     // Espionage and information
    Survival UMETA(DisplayName = "Survival")              // Crisis management
};

/**
 * Diplomatic action types for faction interactions
 */
UENUM(BlueprintType)
enum class EDiplomaticActionType : uint8
{
    None UMETA(DisplayName = "None"),
    ProposeAlliance UMETA(DisplayName = "Propose Alliance"),
    DeclareWar UMETA(DisplayName = "Declare War"),
    OfferPeace UMETA(DisplayName = "Offer Peace"),
    OpenTrade UMETA(DisplayName = "Open Trade"),
    CloseTrade UMETA(DisplayName = "Close Trade"),
    SendAid UMETA(DisplayName = "Send Aid"),
    DemandTribute UMETA(DisplayName = "Demand Tribute"),
    ShareIntelligence UMETA(DisplayName = "Share Intelligence")
};

/**
 * Economic action types for faction resource management
 */
UENUM(BlueprintType)
enum class EEconomicActionType : uint8
{
    None UMETA(DisplayName = "None"),
    BuildStation UMETA(DisplayName = "Build Station"),
    ExpandStation UMETA(DisplayName = "Expand Station"),
    EstablishTradeRoute UMETA(DisplayName = "Establish Trade Route"),
    InvestInProduction UMETA(DisplayName = "Invest In Production"),
    StockpileResources UMETA(DisplayName = "Stockpile Resources"),
    SellSurplus UMETA(DisplayName = "Sell Surplus"),
    BuyStrategicGoods UMETA(DisplayName = "Buy Strategic Goods"),
    SubsidizeIndustry UMETA(DisplayName = "Subsidize Industry")
};

/**
 * Military action types for faction military operations
 */
UENUM(BlueprintType)
enum class EMilitaryActionType : uint8
{
    None UMETA(DisplayName = "None"),
    BuildFleet UMETA(DisplayName = "Build Fleet"),
    UpgradeFleet UMETA(DisplayName = "Upgrade Fleet"),
    PatrolTerritory UMETA(DisplayName = "Patrol Territory"),
    RaidEnemyTrade UMETA(DisplayName = "Raid Enemy Trade"),
    DefendAssets UMETA(DisplayName = "Defend Assets"),
    LaunchOffensive UMETA(DisplayName = "Launch Offensive"),
    SecureTerritory UMETA(DisplayName = "Secure Territory"),
    Retreat UMETA(DisplayName = "Retreat")
};

/**
 * Faction Logic - Handles strategic faction-level AI
 * 
 * This class manages high-level faction behavior, including:
 * - Strategic planning and goal setting
 * - Diplomatic relations with other factions
 * - Economic development and resource allocation
 * - Military strategy and fleet management
 * - Territory expansion and consolidation
 * 
 * Designed to work with FactionDataAsset for data-driven faction AI
 * 
 * Blueprint Usage:
 * 1. Create a Blueprint based on UFactionLogic
 * 2. Assign a FactionDataAsset reference
 * 3. Configure strategic priorities and behavior
 * 4. Override Event functions to customize faction AI
 * 5. Add to faction management systems or game mode
 * 
 * MVP SCOPE (Trade Simulator):
 * - Economic decisions (trading, pricing, station management)
 * - Basic diplomacy (trade relationships, pricing modifiers)
 * - Territory management (station ownership)
 * 
 * POST-MVP SCOPE:
 * - Military actions (fleet deployment, combat)
 * - Advanced diplomacy (wars, alliances, espionage)
 * - Research and intelligence systems
 * 
 * Integration Points:
 * - Faction system: Uses FactionDataAsset for faction traits and relationships
 * - Trading system: Influences trade routes and economic decisions
 * - Station system: Decides station construction and expansion
 * - POST-MVP Combat system: Manages fleet deployment and military actions
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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction|Data")
    TObjectPtr<UFactionDataAsset> FactionData;

    /** Current strategic priority for the faction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction|Strategy")
    EFactionStrategyType CurrentStrategy;

    /** Military strength rating (0-100, calculated periodically) */
    UPROPERTY(BlueprintReadOnly, Category = "Faction|Status")
    float MilitaryStrength;

    /** Economic strength rating (0-100, calculated periodically) */
    UPROPERTY(BlueprintReadOnly, Category = "Faction|Status")
    float EconomicStrength;

    /** Diplomatic influence rating (0-100, calculated periodically) */
    UPROPERTY(BlueprintReadOnly, Category = "Faction|Status")
    float DiplomaticInfluence;

    /** Territory size rating (0-100, calculated periodically) */
    UPROPERTY(BlueprintReadOnly, Category = "Faction|Status")
    float TerritorySize;

    /** Whether faction is currently at war with any other faction */
    UPROPERTY(BlueprintReadOnly, Category = "Faction|Status")
    bool bIsAtWar;

    /** Whether faction is facing economic crisis */
    UPROPERTY(BlueprintReadOnly, Category = "Faction|Status")
    bool bInEconomicCrisis;

    /** Time until next strategic review (in hours) */
    UPROPERTY(BlueprintReadWrite, Category = "Faction|Planning", meta = (ClampMin = "0"))
    float HoursUntilStrategicReview;

    /** List of faction IDs that are allies */
    UPROPERTY(BlueprintReadWrite, Category = "Faction|Diplomacy")
    TArray<FName> Allies;

    /** List of faction IDs that are enemies */
    UPROPERTY(BlueprintReadWrite, Category = "Faction|Diplomacy")
    TArray<FName> Enemies;

    // ====================
    // BlueprintNativeEvent Hooks - Faction Strategic Decision Making
    // ====================

    /**
     * Decide on diplomatic action toward another faction
     * Override to implement custom diplomatic AI
     * 
     * @param TargetFactionID The faction to evaluate diplomatic action with
     * @param CurrentRelationship Current relationship value (-100 to 100)
     * @return The diplomatic action to take
     * 
     * Example Blueprint usage:
     * - Check relationship strength
     * - Consider faction traits (Aggressive = more war)
     * - Check economic/military balance
     * - Factor in alliances and enemies
     * - Return appropriate diplomatic action
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Diplomacy")
    EDiplomaticActionType DecideDiplomaticAction(FName TargetFactionID, int32 CurrentRelationship);
    virtual EDiplomaticActionType DecideDiplomaticAction_Implementation(FName TargetFactionID, int32 CurrentRelationship);

    /**
     * Decide on economic action for faction development
     * Override to implement custom economic AI
     * 
     * @return The economic action to take
     * 
     * Example Blueprint usage:
     * - Evaluate resource availability
     * - Check trade opportunities
     * - Consider faction economic traits
     * - Balance growth vs stockpiling
     * - Return best economic action
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Economy")
    EEconomicActionType DecideEconomicAction();
    virtual EEconomicActionType DecideEconomicAction_Implementation();

    /**
     * Decide on military action based on strategic situation
     * Override to implement custom military AI
     * 
     * @return The military action to take
     * 
     * Example Blueprint usage:
     * - Assess military strength vs enemies
     * - Check for threats to territory
     * - Consider aggressive/defensive traits
     * - Balance fleet building vs deployment
     * - Return appropriate military action
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Military")
    EMilitaryActionType DecideMilitaryAction();
    virtual EMilitaryActionType DecideMilitaryAction_Implementation();

    /**
     * Evaluate faction's overall strategic situation
     * Override to customize situation assessment
     * 
     * Example Blueprint usage:
     * - Calculate threat levels
     * - Assess economic health
     * - Evaluate diplomatic position
     * - Consider expansion opportunities
     * - Set appropriate strategic priority
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Strategy")
    void EvaluateStrategicSituation();
    virtual void EvaluateStrategicSituation_Implementation();

    /**
     * Determine if faction should change strategic focus
     * Override to implement dynamic strategy switching
     * 
     * @return True if strategy should change
     * 
     * Example Blueprint usage:
     * - Check if under attack -> switch to Military
     * - Check if economy failing -> switch to Economic
     * - Check if opportunities for expansion exist
     * - Consider faction traits when switching
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Strategy")
    bool ShouldChangeStrategy();
    virtual bool ShouldChangeStrategy_Implementation();

    /**
     * Select targets for expansion or military action
     * Override to customize target selection logic
     * 
     * @param ActionType The type of action being planned
     * @param MaxTargets Maximum number of targets to select
     * @param OutTargets Array to fill with selected target faction IDs
     * 
     * Example Blueprint usage:
     * - Evaluate neighbors by relationship
     * - Consider military strength ratio
     * - Factor in resource value
     * - Check for alliance commitments
     * - Return prioritized target list
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Planning")
    void SelectActionTargets(EFactionStrategyType ActionType, int32 MaxTargets, TArray<FName>& OutTargets);
    virtual void SelectActionTargets_Implementation(EFactionStrategyType ActionType, int32 MaxTargets, TArray<FName>& OutTargets);

    /**
     * Handle response to diplomatic event from another faction
     * Override to customize diplomatic reactions
     * 
     * @param SourceFactionID Faction that initiated the action
     * @param Action The diplomatic action taken
     * @param bAccept Whether to accept the action (if applicable)
     * 
     * Example Blueprint usage:
     * - Alliance proposal: check relationship, traits
     * - War declaration: prepare defensive measures
     * - Trade offer: evaluate economic benefit
     * - Update relationship values
     * - Trigger faction events
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faction|Diplomacy")
    void HandleDiplomaticEvent(FName SourceFactionID, EDiplomaticActionType Action, bool bAccept);
    virtual void HandleDiplomaticEvent_Implementation(FName SourceFactionID, EDiplomaticActionType Action, bool bAccept);

    // ====================
    // Blueprint-Callable Utility Functions
    // ====================

    /**
     * Check if faction is allied with another faction
     * @param OtherFactionID The faction to check
     * @return True if they are allies
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Diplomacy")
    bool IsAlliedWith(FName OtherFactionID) const;

    /**
     * Check if faction is at war with another faction
     * @param OtherFactionID The faction to check
     * @return True if they are enemies
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Diplomacy")
    bool IsEnemyOf(FName OtherFactionID) const;

    /**
     * Add another faction as an ally
     * @param OtherFactionID The faction to ally with
     */
    UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
    void AddAlly(FName OtherFactionID);

    /**
     * Remove a faction from allies list
     * @param OtherFactionID The faction to break alliance with
     */
    UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
    void RemoveAlly(FName OtherFactionID);

    /**
     * Add another faction as an enemy
     * @param OtherFactionID The faction to declare war on
     */
    UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
    void AddEnemy(FName OtherFactionID);

    /**
     * Remove a faction from enemies list
     * @param OtherFactionID The faction to make peace with
     */
    UFUNCTION(BlueprintCallable, Category = "Faction|Diplomacy")
    void RemoveEnemy(FName OtherFactionID);

    /**
     * Calculate faction's overall power rating
     * @return Combined rating (0-100) based on military, economic, and diplomatic strength
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Status")
    float CalculateOverallPower() const;

    /**
     * Check if faction is in a strong position
     * @return True if military, economic, and diplomatic ratings are all above 50
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Status")
    bool IsInStrongPosition() const;

    /**
     * Check if faction is vulnerable
     * @return True if any core rating is below 30
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Status")
    bool IsVulnerable() const;

    /**
     * Get relationship value with another faction
     * @param OtherFactionID The faction to check relationship with
     * @return Relationship value (-100 to 100), or 0 if not found
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Diplomacy")
    int32 GetRelationshipWith(FName OtherFactionID) const;

    /**
     * Get faction name from data asset
     * @return Faction name or "Unknown" if no data asset
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Info")
    FText GetFactionName() const;

    /**
     * Get a description of current faction state and strategy
     * @return Text description of faction's current strategic state
     */
    UFUNCTION(BlueprintPure, Category = "Faction|Info")
    FString GetFactionStateDescription() const;

    /**
     * Set the strategic focus for the faction
     * @param NewStrategy The strategic priority to adopt
     */
    UFUNCTION(BlueprintCallable, Category = "Faction|Strategy")
    void SetStrategy(EFactionStrategyType NewStrategy);

    /**
     * Update faction strength ratings
     * Call this periodically to refresh military/economic/diplomatic ratings
     */
    UFUNCTION(BlueprintCallable, Category = "Faction|Status")
    void UpdateStrengthRatings();

    // ====================
    // Overridden from UNPCLogicBase
    // ====================

    virtual void InitializeAI_Implementation() override;
    virtual void OnTickAI_Implementation(float DeltaTime) override;
    virtual EAIPriority EvaluateCurrentPriority_Implementation() override;
    virtual bool DecideNextAction_Implementation(EAIPriority CurrentPriority) override;

private:
    // ====================
    // Internal Helper Functions
    // ====================

    /**
     * Get a loaded faction asset from a soft reference, using cache to avoid repeated loads
     * @param SoftPtr The soft object pointer to load
     * @return The loaded faction asset, or nullptr if not valid
     */
    UFactionDataAsset* GetLoadedFaction(const TSoftObjectPtr<UFactionDataAsset>& SoftPtr) const;

    /**
     * Cache of loaded faction assets to avoid repeated LoadSynchronous calls
     * Maps soft object paths to loaded faction pointers
     * Marked mutable to allow caching in const methods
     */
    UPROPERTY()
    mutable TMap<FSoftObjectPath, TObjectPtr<UFactionDataAsset>> LoadedFactionCache;
};
