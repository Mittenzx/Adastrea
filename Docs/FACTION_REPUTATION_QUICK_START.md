# Faction & Reputation System Quick Start

## Overview

The Faction and Reputation system provides the foundation for political relationships, diplomacy, and faction-based gameplay in Adastrea.

## Core Classes

### UFactionDataAsset

Data Asset for defining factions in the game.

**Location:** `Source/Adastrea/Public/Factions/FactionDataAsset.h`

**Key Properties:**
- `FactionName` (FText) - Display name of the faction
- `Description` (FText) - Faction description and lore
- `FactionID` (FName) - Unique identifier
- `InitialReputation` (int32) - Starting reputation with player (-100 to 100)
- `bIsHostileByDefault` (bool) - Whether faction starts hostile
- `bIsMajorFaction` (bool) - Major faction flag

**Usage:**
1. Create a Blueprint based on UFactionDataAsset
2. Configure properties in the editor
3. Reference from stations, NPCs, quests, and trading systems

### UPlayerReputationComponent

Actor component for tracking player reputation with all factions.

**Location:** `Source/Adastrea/Public/Player/PlayerReputationComponent.h`

**Key Features:**
- Tracks reputation with all factions (-100 to 100 scale)
- 7 reputation tiers: Hostile, Unfriendly, Neutral, Friendly, Honored, Revered, Exalted
- Trade price modifiers based on reputation
- Save/load integration via FPlayerFactionReputation struct

**Key Functions:**
- `InitializeFactionReputation(FName FactionID, int32 InitialReputation)` - Initialize reputation
- `ModifyReputation(FName FactionID, int32 Amount)` - Change reputation
- `GetReputation(FName FactionID)` - Query current reputation
- `GetReputationTier(FName FactionID)` - Get tier (Hostile, Friendly, etc.)
- `ShouldBeHostile(FName FactionID)` - Check if faction is hostile
- `GetTradePriceModifier(FName FactionID)` - Get price modifier (0.7 to 1.3)

**Usage:**
1. Add component to player pawn/controller
2. Initialize factions via `InitializeFromFactionData(UFactionDataAsset*)`
3. Modify reputation based on player actions
4. Query reputation to affect gameplay (trading, AI behavior, quests)

## Integration Points

### Space Stations
- `ASpaceStationModule` has `ModuleFaction` property (UFactionDataAsset*)
- Station ownership affects available services and prices

### Trading System
- `UMarketDataAsset` references faction
- `UTradeContractDataAsset` has `ContractGiver` faction
- `UAITraderComponent` has `TraderFaction` affiliation
- Prices adjusted via `GetTradePriceModifier()`

### Save System
- `UAdastreaSaveGame` stores `TArray<FPlayerFactionReputation> FactionReputations`
- Persists reputation across sessions

### UI Systems
- `UTradingInterfaceWidget` displays faction information
- `UStationEditorWidget` shows faction-specific options

## Reputation Tiers

| Tier | Range | Trade Modifier (actual) | Description |
|------|-------|-------------------------|-------------|
| Hostile | -100 to -60 | 1.30x – 1.18x (30–18% more expensive) | Will attack on sight |
| Unfriendly | -59 to -20 | 1.177x – 1.06x (17.7–6% more expensive) | Distrustful, limited access |
| Neutral | -19 to 19 | 1.057x – 0.943x (5.7% more expensive – 5.7% discount) | Default state |
| Friendly | 20 to 59 | 0.94x – 0.823x (6–17.7% discount) | Helpful, better services |
| Honored | 60 to 89 | 0.82x – 0.733x (18–26.7% discount) | Respected member |
| Revered | 90 to 99 | 0.73x – 0.703x (27–29.7% discount) | Highly honored |
| Exalted | 100 | 0.70x (30% discount) | Maximum reputation |

> **Note:** The trade price modifier is calculated as `1.0 - (Reputation * 0.003)`, where Reputation ranges from -100 to +100. The modifier is applied linearly across the reputation scale.

## Example: Creating a Faction

### In C++
```cpp
// Initialize faction reputation
UPlayerReputationComponent* RepComp = Player->FindComponentByClass<UPlayerReputationComponent>();
if (RepComp && FactionData)
{
    RepComp->InitializeFromFactionData(FactionData);
}

// Modify reputation when player completes quest
RepComp->ModifyReputation(FName("SolarisUnion"), 10); // +10 reputation

// Check if should be hostile
bool bHostile = RepComp->ShouldBeHostile(FName("SolarisUnion"));
```

### In Blueprints
1. Get Player Reputation Component
2. Call "Initialize From Faction Data" with your faction asset
3. Use "Modify Reputation" to change based on player actions
4. Query "Get Reputation" or "Get Reputation Tier" for gameplay

## Example: Faction-Based Trading
```cpp
// In trading system
float BasePrice = TradeItem->GetBasePrice();
float ReputationModifier = PlayerReputation->GetTradePriceModifier(MarketFaction->GetFactionID());
float FinalPrice = BasePrice * ReputationModifier;
```

## Future Enhancements

The current implementation provides the core foundation. Future updates may include:
- Faction traits and gameplay modifiers
- Inter-faction relationships and diplomacy
- Faction-specific quests and storylines
- Dynamic faction wars and alliances
- Faction territory control
- Faction AI for strategic decisions

## Related Documentation

- `Assets/AISystemIntegration.md` - AI integration with factions
- `Assets/TradingSystemGuide.md` - Trading system details
- `Source/Adastrea/Player/AdastreaSaveGame.h` - Save system integration

## Notes

- This is a minimal, production-ready implementation created to resolve build errors
- All systems using factions now have access to these types
- The implementation follows Adastrea's Data Asset pattern and coding standards
- Full Blueprint integration for designer-friendly workflow
