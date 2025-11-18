# Faction System Quick Reference

This guide provides quick examples and usage patterns for the faction system, which handles political diplomacy, warfare, and large-scale military operations.

**System Scope:**
- **Factions** = Large political/military organizations handling diplomacy, warfare, and political power
- **Ways** = Small specialized guilds (50-1000 members) handling trade, production, and economic cooperation

For economic activities, trade networks, guild management, and industry specialization, see the [Way System Guide](WaySystemGuide.md). Ways can belong to factions but operate independently for economic matters through "Way Networks" (micro-alliances of 2-5 guilds).

---

## Table of Contents

1. [Faction Runtime State](#faction-runtime-state)
2. [Player Reputation](#player-reputation)
3. [Diplomacy Manager](#diplomacy-manager)
4. [Trait Categories](#trait-categories)
5. [Integration Examples](#integration-examples)

---

## Faction Runtime State

**Component:** `UFactionRuntimeState`  
**Location:** `Source/Adastrea/Factions/FactionRuntimeState.h`

### Basic Setup

```cpp
// In your faction manager actor or game mode
UFactionRuntimeState* FactionState = NewObject<UFactionRuntimeState>(this);
FactionState->RegisterComponent();
FactionState->InitializeFromTemplate(FactionDataAsset);
```

### Blueprint Usage

```
Create Component → Faction Runtime State
↓
Initialize From Template (FactionDataAsset)
↓
Ready for runtime modifications
```

### Common Operations

**Player Reputation:**
```cpp
// Modify player reputation
FactionState->ModifyPlayerReputation(10); // +10 reputation

// Check reputation
int32 CurrentRep = FactionState->GetPlayerReputation();
bool IsFriendly = FactionState->IsReputationAtLeast(25);
```

**Dynamic Relationships:**
```cpp
// Update relationship with another faction
FactionState->UpdateFactionRelationship(FName("OtherFaction"), 50);

// Declare war
FactionState->DeclareWar(FName("EnemyFaction"));

// Form alliance
FactionState->FormAlliance(FName("AllyFaction"));
```

**Runtime Traits:**
```cpp
// Add a trait at runtime
FFactionTrait NewTrait;
NewTrait.TraitID = FName("BonusTrade");
NewTrait.TraitName = FText::FromString("Trade Bonus");
NewTrait.Category = EFactionTraitCategory::Economic;
NewTrait.ModifierValue = 1.2f;
FactionState->AddRuntimeTrait(NewTrait);

// Remove a trait
FactionState->RemoveRuntimeTrait(FName("BonusTrade"));
```

**Territory Control:**
```cpp
// Add controlled sector
FactionState->AddControlledSector(FName("AlphaSector"));

// Check control
bool bControls = FactionState->ControlsSector(FName("AlphaSector"));
```

---

## Player Reputation

**Component:** `UPlayerReputationComponent`  
**Location:** `Source/Adastrea/Player/PlayerReputationComponent.h`

### Setup

Attach to player pawn or player controller:

```cpp
// C++
UPlayerReputationComponent* ReputationComp = CreateDefaultSubobject<UPlayerReputationComponent>(TEXT("Reputation"));

// Blueprint: Add Component → Player Reputation Component
```

### Reputation Management

```cpp
// Modify reputation with a faction
ReputationComp->ModifyReputation(FName("SolarisUnion"), 15);

// Set specific value
ReputationComp->SetReputation(FName("SolarisUnion"), 50);

// Get current reputation
int32 Rep = ReputationComp->GetReputation(FName("SolarisUnion"));

// Get reputation tier
EReputationTier Tier = ReputationComp->GetReputationTier(FName("SolarisUnion"));
// Returns: Hated, Hostile, Unfriendly, Neutral, Friendly, Honored, or Revered
```

### Gameplay Effects

```cpp
// Trade price modifier
float PriceModifier = ReputationComp->GetTradePriceModifier(FName("SolarisUnion"));
// Returns: 0.7-1.5 (0.7 = 30% discount, 1.5 = 50% markup)

// Check hostility
bool bHostile = ReputationComp->ShouldBeHostile(FName("Pirates"));
// Returns true if reputation < -25

// Access control
bool bCanAccess = ReputationComp->CanAccessRestrictedAreas(FName("SolarisUnion"));
// Returns true if reputation >= 26

// Mission quality
int32 QualityLevel = ReputationComp->GetMissionQualityLevel(FName("SolarisUnion"));
// Returns: 0-5 (higher = better missions)
```

### Mission Tracking

```cpp
// Record completed mission
ReputationComp->RecordMissionCompleted(FName("SolarisUnion"));

// Check missions completed
int32 MissionCount = ReputationComp->GetMissionsCompleted(FName("SolarisUnion"));

// Mark storyline complete
ReputationComp->MarkStorylineCompleted(FName("SolarisUnion"));
```

---

## Diplomacy Manager

**Subsystem:** `UFactionDiplomacyManager`  
**Location:** `Source/Adastrea/Factions/FactionDiplomacyManager.h`

### Access

```cpp
// Get the diplomacy manager
UFactionDiplomacyManager* DiplomacyMgr = GetGameInstance()->GetSubsystem<UFactionDiplomacyManager>();
```

### Faction Registration

```cpp
// Register faction runtime states
DiplomacyMgr->RegisterFaction(FName("SolarisUnion"), SolarisRuntimeState);
DiplomacyMgr->RegisterFaction(FName("NovaVanguard"), NovaRuntimeState);

// Check if registered
bool bRegistered = DiplomacyMgr->IsFactionRegistered(FName("SolarisUnion"));

// Get faction state
UFactionRuntimeState* State = DiplomacyMgr->GetFactionState(FName("SolarisUnion"));
```

### Diplomatic Actions

```cpp
// Declare war between two factions
DiplomacyMgr->DeclareWar(FName("SolarisUnion"), FName("Pirates"));

// Make peace
DiplomacyMgr->MakePeace(FName("SolarisUnion"), FName("Collective"));

// Form alliance
DiplomacyMgr->FormAlliance(FName("SolarisUnion"), FName("NovaVanguard"));

// Break alliance
DiplomacyMgr->BreakAlliance(FName("SolarisUnion"), FName("OldAlly"));

// Modify relationship
DiplomacyMgr->ModifyRelationship(FName("SolarisUnion"), FName("Traders"), 20);
```

### Queries

```cpp
// Check if factions are at war
bool bAtWar = DiplomacyMgr->AreFactionsAtWar(FName("SolarisUnion"), FName("Pirates"));

// Check if factions are allied
bool bAllied = DiplomacyMgr->AreFactionsAllied(FName("SolarisUnion"), FName("NovaVanguard"));

// Get relationship value
int32 RelValue = DiplomacyMgr->GetRelationshipValue(FName("SolarisUnion"), FName("Traders"));

// Get all warring factions
TArray<FName> Enemies = DiplomacyMgr->GetWarringFactions(FName("SolarisUnion"));

// Get all allied factions
TArray<FName> Allies = DiplomacyMgr->GetAlliedFactions(FName("SolarisUnion"));
```

---

## Trait Categories

**Enhancement:** Trait system now supports categories for better organization.

### Trait Categories

- **Military**: Combat and defense bonuses
- **Economic**: Trade and resource bonuses
- **Scientific**: Research and technology bonuses
- **Diplomatic**: Relationship modifiers
- **Special**: Unique faction abilities

### Creating Categorized Traits

```cpp
FFactionTrait MilitaryTrait;
MilitaryTrait.TraitID = FName("WarMachine");
MilitaryTrait.TraitName = FText::FromString("War Machine");
MilitaryTrait.TraitDescription = FText::FromString("Increased military strength");
MilitaryTrait.Category = EFactionTraitCategory::Military;
MilitaryTrait.ModifierValue = 1.5f;

FFactionTrait EconomicTrait;
EconomicTrait.TraitID = FName("TradeHub");
EconomicTrait.TraitName = FText::FromString("Trade Hub");
EconomicTrait.Category = EFactionTraitCategory::Economic;
EconomicTrait.ModifierValue = 1.3f;
```

### Using Category Filters

```cpp
// Get all military traits
TArray<FFactionTrait> MilitaryTraits = FactionDataAsset->GetTraitsByCategory(EFactionTraitCategory::Military);

// Check if faction has economic traits
bool bHasEconomic = FactionDataAsset->HasTraitInCategory(EFactionTraitCategory::Economic);

// Get total modifier for all diplomatic traits
float DiplomaticBonus = FactionDataAsset->GetCategoryModifierTotal(EFactionTraitCategory::Diplomatic);
```

---

## Integration Examples

### Example 1: Trading System Integration

**Note:** For actual trading operations, use the [Way System](WaySystemGuide.md) which provides detailed supply chains, guild-specific pricing, and Way Network bonuses. Faction reputation affects high-level access but Ways handle the specifics.

```cpp
// Basic faction reputation check for trading access
float CalculateBaseTradingAccess(UPlayerReputationComponent* Reputation, FName FactionID)
{
    // Faction reputation determines if you can trade at all
    float ReputationModifier = Reputation->GetTradePriceModifier(FactionID);
    
    // For detailed pricing, supply/demand, and guild bonuses, 
    // integrate with the Way System (see WaySystemAPIReference.md)
    
    return ReputationModifier;
}

// For actual trade pricing, use Way System:
// float finalPrice = WayNetwork->CalculatePrice(item, playerVerse);
```

### Example 2: NPC Hostility Check

```cpp
// In your NPC AI
bool ShouldAttackPlayer(UPlayerReputationComponent* Reputation, FName NPCFaction)
{
    // Check base hostility
    if (Reputation->ShouldBeHostile(NPCFaction))
    {
        return true;
    }
    
    // Check if faction is at war with player's allies
    UFactionDiplomacyManager* DiplomacyMgr = GetGameInstance()->GetSubsystem<UFactionDiplomacyManager>();
    
    // More complex logic here...
    
    return false;
}
```

### Example 3: Mission Availability

```cpp
// In your mission system
TArray<UMissionDataAsset*> GetAvailableMissions(UPlayerReputationComponent* Reputation, FName FactionID)
{
    TArray<UMissionDataAsset*> AvailableMissions;
    
    // Get mission quality level based on reputation
    int32 QualityLevel = Reputation->GetMissionQualityLevel(FactionID);
    
    // Filter missions by quality level
    for (UMissionDataAsset* Mission : AllFactionMissions)
    {
        if (Mission->RequiredReputationTier <= QualityLevel)
        {
            AvailableMissions.Add(Mission);
        }
    }
    
    return AvailableMissions;
}
```

### Example 4: Dynamic Territory Control

```cpp
// In your territory system
void UpdateTerritoryControl(UFactionRuntimeState* FactionState, FName SectorID, bool bCaptured)
{
    if (bCaptured)
    {
        FactionState->AddControlledSector(SectorID);
        
        // Trigger event
        OnTerritoryChanged.Broadcast(FactionState->FactionTemplate->FactionID, SectorID, true);
    }
    else
    {
        FactionState->RemoveControlledSector(SectorID);
        
        // Trigger event
        OnTerritoryChanged.Broadcast(FactionState->FactionTemplate->FactionID, SectorID, false);
    }
}
```

---

## Blueprint Examples

### Example: Simple Reputation Reward

```
Event: OnMissionComplete
↓
Get Player Reputation Component
↓
Modify Reputation (FactionID: "SolarisUnion", Delta: 10)
↓
Get Reputation Tier (FactionID: "SolarisUnion")
↓
Branch (Tier >= Friendly?)
  → True: Show "Unlocked Faction Store!" message
  → False: Continue
```

### Example: Check Faction War Status

```
Event: OnEnterSector
↓
Get Diplomacy Manager (Game Instance Subsystem)
↓
Are Factions At War? (FactionA: "SolarisUnion", FactionB: "Pirates")
↓
Branch (At War?)
  → True: Spawn hostile patrols
  → False: Spawn neutral traders
```

---

## Common Patterns

### Pattern 1: Faction Initialization

```cpp
void InitializeFaction(UFactionDataAsset* Template)
{
    // 1. Create runtime state
    UFactionRuntimeState* RuntimeState = NewObject<UFactionRuntimeState>(this);
    RuntimeState->RegisterComponent();
    
    // 2. Initialize from template
    RuntimeState->InitializeFromTemplate(Template);
    
    // 3. Register with diplomacy manager
    UFactionDiplomacyManager* DiplomacyMgr = GetGameInstance()->GetSubsystem<UFactionDiplomacyManager>();
    DiplomacyMgr->RegisterFaction(Template->FactionID, RuntimeState);
    
    // 4. Store reference
    ActiveFactions.Add(Template->FactionID, RuntimeState);
}
```

### Pattern 2: Reputation-Based Content Gating

```cpp
bool CanAccessContent(UPlayerReputationComponent* Reputation, FName FactionID, int32 RequiredReputation)
{
    return Reputation->IsReputationAtLeast(FactionID, RequiredReputation);
}
```

### Pattern 3: Dynamic Trait Application

```cpp
void ApplyRuntimeTrait(UFactionRuntimeState* FactionState, FName TraitID, float Modifier)
{
    FFactionTrait NewTrait;
    NewTrait.TraitID = TraitID;
    NewTrait.ModifierValue = Modifier;
    // Set other properties...
    
    FactionState->AddRuntimeTrait(NewTrait);
}
```

---

## Troubleshooting

### Common Issues

**Q: Reputation changes aren't affecting trade prices**  
A: Ensure you're calling `GetTradePriceModifier()` and applying the multiplier to prices.

**Q: Faction state isn't persisting**  
A: Runtime states need to be saved/loaded separately. Implement save game serialization.

**Q: Diplomatic events aren't triggering**  
A: Event listener system is a stub. Implement listeners for `BroadcastDiplomaticEvent()`.

**Q: Traits aren't affecting gameplay**  
A: Integration with gameplay systems is pending. Use trait modifiers in your systems.

---

## Next Steps

1. **Integrate with existing systems** (combat, missions, political storylines)
2. **Create content** (faction data assets with political/military traits)
3. **Implement UI** (reputation display, diplomacy panel, war declarations)
4. **Add persistence** (save/load faction states)
5. **Extend event system** (implement diplomatic event listeners)
6. **Integrate with Way System** (link factions to their constituent Ways for economic activities)

---

## Related Documentation

### Faction System (Political/Military)
- [Assets/IterationGuide.md](Assets/IterationGuide.md) - Complete faction system guide
- [Assets/FactionSetupGuide.md](Assets/FactionSetupGuide.md) - Faction creation guide
- [Assets/BlueprintFactionAssetGuide.md](Assets/BlueprintFactionAssetGuide.md) - Blueprint integration
- [CHANGELOG.md](../CHANGELOG.md) - Implementation status

### Way System (Economic/Guild)
- [Assets/WAY_SYSTEM_INDEX.md](Assets/WAY_SYSTEM_INDEX.md) - Way system documentation index
- [Assets/WaySystemGuide.md](Assets/WaySystemGuide.md) - Complete Way system guide
- [Assets/WayCreationGuide.md](Assets/WayCreationGuide.md) - Creating guilds and networks
- [Assets/WaySystemAPIReference.md](Assets/WaySystemAPIReference.md) - Programming API

_Last updated: 2025-11-18_
