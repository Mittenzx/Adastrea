#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IFactionMember.generated.h"

// Forward declarations
class UFactionDataAsset;

/**
 * UInterface class for IFactionMember
 * Required by Unreal's reflection system
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UFactionMember : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that belong to or are affiliated with a faction
 * 
 * Implement this interface on:
 * - Spaceships (ASpaceship)
 * - Space stations (ASpaceStation)
 * - NPCs and personnel
 * - AI-controlled entities
 * - Quest givers and traders
 * 
 * Benefits:
 * - Standardized faction relationship queries
 * - Easy IFF (Identification Friend or Foe) checks
 * - Simplified AI decision-making based on alliances
 * - Consistent faction-based gameplay mechanics
 * 
 * Usage Example (C++):
 * @code
 * if (Actor->Implements<UFactionMember>())
 * {
 *     IFactionMember* FactionMemberInterface = Cast<IFactionMember>(Actor);
 *     UFactionDataAsset* Faction = FactionMemberInterface->Execute_GetFaction(Actor);
 *     
 *     if (FactionMemberInterface->Execute_IsHostileTo(Actor, MyFactionInterface))
 *     {
 *         // Engage in combat
 *     }
 * }
 * @endcode
 * 
 * Usage Example (Blueprint):
 * - Check "Does Implement Interface" for IFactionMember
 * - Call GetFaction to retrieve faction data
 * - Call IsAlliedWith or IsHostileTo for relationship checks
 */
class ADASTREA_API IFactionMember
{
    GENERATED_BODY()

public:
    /**
     * Get the faction this actor belongs to
     * 
     * @return Faction Data Asset, or nullptr if actor is independent/neutral
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    UFactionDataAsset* GetFaction() const;

    /**
     * Check if this actor is allied with another faction member
     * 
     * Alliance is determined by:
     * - Same faction = always allied
     * - Different factions = check diplomatic relations (positive = allied)
     * - Independent actors = never allied unless explicitly set
     * 
     * @param Other Another actor implementing IFactionMember
     * @return True if allied, false otherwise
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    bool IsAlliedWith(const TScriptInterface<IFactionMember>& Other) const;

    /**
     * Check if this actor is hostile to another faction member
     * 
     * Hostility is determined by:
     * - Same faction = never hostile (unless explicitly overridden)
     * - Different factions = check diplomatic relations (negative = hostile)
     * - Independent actors = check individual relationships
     * 
     * @param Other Another actor implementing IFactionMember
     * @return True if hostile, false otherwise
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    bool IsHostileTo(const TScriptInterface<IFactionMember>& Other) const;

    /**
     * Get faction relationship value with another faction member
     * 
     * Relationship scale:
     * - -100 to -51: At war / extreme hostility
     * - -50 to -26: Hostile / unfriendly
     * - -25 to +25: Neutral
     * - +26 to +50: Friendly / cordial
     * - +51 to +100: Allied / close cooperation
     * 
     * @param Other Another actor implementing IFactionMember
     * @return Relationship value from -100 (war) to +100 (allied)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    int32 GetRelationshipWith(const TScriptInterface<IFactionMember>& Other) const;

    /**
     * Check if this actor is neutral (no faction affiliation)
     * 
     * @return True if independent/neutral, false if affiliated with a faction
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    bool IsNeutral() const;

    /**
     * Get faction display name for UI
     * 
     * Shows:
     * - Faction name if affiliated
     * - "Independent" if neutral
     * - Custom name if overridden
     * 
     * @return Display name for UI elements
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    FText GetFactionDisplayName() const;

    /**
     * Check if this actor can engage in hostile actions
     * 
     * Returns false if:
     * - Actor is docked at a safe station
     * - Actor is in a no-combat zone
     * - Actor is protected by quest mechanics
     * 
     * @return True if can attack, false if restricted
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    bool CanEngageInCombat() const;

    /**
     * Get reputation modifier for trading prices
     * 
     * Positive relationships = better prices
     * Negative relationships = worse prices or refusal to trade
     * 
     * @param TraderFaction Faction of the trader
     * @return Price modifier multiplier (0.5 to 2.0, where 1.0 is base price)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    float GetTradePriceModifier(UFactionDataAsset* TraderFaction) const;
};
