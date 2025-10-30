#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Public/Factions/FactionDataAsset.h"
#include "FactionDiplomacyManager.generated.h"

// Forward declarations
class UFactionRuntimeState;

/**
 * Diplomatic event types for triggering gameplay responses
 */
UENUM(BlueprintType)
enum class EDiplomaticEvent : uint8
{
    WarDeclared         UMETA(DisplayName = "War Declared"),
    PeaceMade           UMETA(DisplayName = "Peace Made"),
    AllianceFormed      UMETA(DisplayName = "Alliance Formed"),
    AllianceBroken      UMETA(DisplayName = "Alliance Broken"),
    ReputationIncreased UMETA(DisplayName = "Reputation Increased"),
    ReputationDecreased UMETA(DisplayName = "Reputation Decreased"),
    TerritoryChanged    UMETA(DisplayName = "Territory Changed")
};

/**
 * Diplomatic event data passed to listeners
 */
USTRUCT(BlueprintType)
struct FDiplomaticEventData
{
    GENERATED_BODY()

    /** Type of diplomatic event */
    UPROPERTY(BlueprintReadOnly, Category="Diplomacy")
    EDiplomaticEvent EventType;

    /** Primary faction involved */
    UPROPERTY(BlueprintReadOnly, Category="Diplomacy")
    FName FactionA;

    /** Secondary faction involved (if applicable) */
    UPROPERTY(BlueprintReadOnly, Category="Diplomacy")
    FName FactionB;

    /** Numeric value associated with event (e.g., reputation change) */
    UPROPERTY(BlueprintReadOnly, Category="Diplomacy")
    int32 Value;

    /** Optional text description of the event */
    UPROPERTY(BlueprintReadOnly, Category="Diplomacy")
    FText Description;

    FDiplomaticEventData()
        : EventType(EDiplomaticEvent::ReputationIncreased)
        , FactionA(NAME_None)
        , FactionB(NAME_None)
        , Value(0)
        , Description(FText::GetEmpty())
    {}
};

/**
 * Game instance subsystem for managing faction diplomacy across the game.
 * Coordinates faction relationships, wars, alliances, and diplomatic events.
 * 
 * This is a centralized manager that:
 * - Tracks all active faction runtime states
 * - Manages diplomatic actions between factions
 * - Broadcasts diplomatic events to interested systems
 * - Handles cascading diplomatic effects (e.g., ally of my enemy)
 * 
 * Usage:
 * - Automatically created as a game instance subsystem
 * - Access via UGameInstance::GetSubsystem<UFactionDiplomacyManager>()
 * - Register faction runtime states
 * - Call diplomacy functions to trigger events
 * 
 * Example:
 * ```cpp
 * UFactionDiplomacyManager* Manager = GetGameInstance()->GetSubsystem<UFactionDiplomacyManager>();
 * Manager->DeclareWar(FactionA, FactionB);
 * ```
 */
UCLASS(BlueprintType)
class ADASTREA_API UFactionDiplomacyManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFactionDiplomacyManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ====================
    // Faction Registration
    // ====================

    /**
     * Register a faction runtime state with the manager
     * @param FactionID Unique identifier for the faction
     * @param RuntimeState The runtime state component to register
     */
    UFUNCTION(BlueprintCallable, Category="Diplomacy")
    void RegisterFaction(FName FactionID, UFactionRuntimeState* RuntimeState);

    /**
     * Unregister a faction from the manager
     * @param FactionID The faction to unregister
     */
    UFUNCTION(BlueprintCallable, Category="Diplomacy")
    void UnregisterFaction(FName FactionID);

    /**
     * Get a faction's runtime state
     * @param FactionID The faction to get
     * @return The runtime state, or nullptr if not found
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Diplomacy")
    UFactionRuntimeState* GetFactionState(FName FactionID) const;

    /**
     * Check if a faction is registered
     * @param FactionID The faction to check
     * @return True if faction is registered
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Diplomacy")
    bool IsFactionRegistered(FName FactionID) const;

    // ====================
    // Diplomatic Actions
    // ====================

    /**
     * Declare war between two factions
     * @param FactionA First faction
     * @param FactionB Second faction
     * @param bBroadcastEvent Whether to broadcast the event to listeners
     */
    UFUNCTION(BlueprintCallable, Category="Diplomacy|Actions")
    void DeclareWar(FName FactionA, FName FactionB, bool bBroadcastEvent = true);

    /**
     * Make peace between two factions
     * @param FactionA First faction
     * @param FactionB Second faction
     * @param bBroadcastEvent Whether to broadcast the event to listeners
     */
    UFUNCTION(BlueprintCallable, Category="Diplomacy|Actions")
    void MakePeace(FName FactionA, FName FactionB, bool bBroadcastEvent = true);

    /**
     * Form alliance between two factions
     * @param FactionA First faction
     * @param FactionB Second faction
     * @param bBroadcastEvent Whether to broadcast the event to listeners
     */
    UFUNCTION(BlueprintCallable, Category="Diplomacy|Actions")
    void FormAlliance(FName FactionA, FName FactionB, bool bBroadcastEvent = true);

    /**
     * Break alliance between two factions
     * @param FactionA First faction
     * @param FactionB Second faction
     * @param bBroadcastEvent Whether to broadcast the event to listeners
     */
    UFUNCTION(BlueprintCallable, Category="Diplomacy|Actions")
    void BreakAlliance(FName FactionA, FName FactionB, bool bBroadcastEvent = true);

    /**
     * Modify relationship between two factions
     * @param FactionA First faction
     * @param FactionB Second faction
     * @param Delta Amount to change relationship by
     * @param bBroadcastEvent Whether to broadcast the event to listeners
     */
    UFUNCTION(BlueprintCallable, Category="Diplomacy|Actions")
    void ModifyRelationship(FName FactionA, FName FactionB, int32 Delta, bool bBroadcastEvent = true);

    // ====================
    // Diplomatic Queries
    // ====================

    /**
     * Check if two factions are at war
     * @param FactionA First faction
     * @param FactionB Second faction
     * @return True if at war
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Diplomacy|Queries")
    bool AreFactionsAtWar(FName FactionA, FName FactionB) const;

    /**
     * Check if two factions are allied
     * @param FactionA First faction
     * @param FactionB Second faction
     * @return True if allied
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Diplomacy|Queries")
    bool AreFactionsAllied(FName FactionA, FName FactionB) const;

    /**
     * Get relationship value between two factions
     * @param FactionA First faction
     * @param FactionB Second faction
     * @return Relationship value (-100 to 100), or 0 if not found
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Diplomacy|Queries")
    int32 GetRelationshipValue(FName FactionA, FName FactionB) const;

    /**
     * Get all factions at war with a specific faction
     * @param FactionID The faction to check
     * @return Array of faction IDs at war
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Diplomacy|Queries")
    TArray<FName> GetWarringFactions(FName FactionID) const;

    /**
     * Get all factions allied with a specific faction
     * @param FactionID The faction to check
     * @return Array of allied faction IDs
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Diplomacy|Queries")
    TArray<FName> GetAlliedFactions(FName FactionID) const;

    // ====================
    // Event System (Stub for Future Implementation)
    // ====================

    /**
     * Broadcast a diplomatic event to interested listeners
     * Note: Event listener system to be implemented
     * @param EventData The event data to broadcast
     */
    UFUNCTION(BlueprintCallable, Category="Diplomacy|Events")
    void BroadcastDiplomaticEvent(const FDiplomaticEventData& EventData);

protected:
    /** Map of faction IDs to their runtime states */
    UPROPERTY()
    TMap<FName, UFactionRuntimeState*> RegisteredFactions;

    /**
     * Apply diplomatic action to both factions (bidirectional)
     * @param FactionA First faction
     * @param FactionB Second faction
     * @param Action Lambda function to apply to both faction states
     */
    void ApplyBidirectionalAction(FName FactionA, FName FactionB, 
        TFunction<void(UFactionRuntimeState*, FName)> Action);

    /**
     * Create a diplomatic event data struct
     * @param EventType Type of event
     * @param FactionA Primary faction
     * @param FactionB Secondary faction
     * @param Value Associated value
     * @param Description Event description
     * @return The event data struct
     */
    FDiplomaticEventData CreateEventData(EDiplomaticEvent EventType, FName FactionA, 
        FName FactionB, int32 Value, const FText& Description) const;
};
