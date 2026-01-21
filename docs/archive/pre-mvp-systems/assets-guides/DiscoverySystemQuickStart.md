# Discovery System Quick Start Guide

**Version**: 1.0.0  
**Last Updated**: November 21, 2025  
**Prerequisites**: Scanner System (Phase 1)

---

## Overview

The Discovery System (Phase 2) extends the Exploration System with:
- **Anomalies**: Spatial phenomena with rewards
- **Discoveries**: Archaeological sites, derelicts, artifacts
- **Tracking**: Global discovery database and statistics
- **Rewards**: Credits, XP, items, reputation
- **Interactions**: Harvest, explore, salvage, research

---

## Quick Setup (5 Minutes)

### 1. Create an Anomaly

**In Editor:**
1. Content Browser → Right-click → Blueprint → Data Asset
2. Choose `AnomalyDataAsset`
3. Name it: `DA_Anomaly_GravitySingularity`

**Configure:**
```
Basic Info:
- Anomaly Name: "Gravitational Singularity"
- Description: "An intense gravitational distortion"
- Anomaly Type: Gravitational Anomaly
- Anomaly ID: "Anomaly_GravSingularity_001"

Detection:
- Signal Strength: 85
- Required Scanner Type: Scientific
- Requires Deep Scan: true

Rarity:
- Rarity Tier: Very Rare
- Discovery Value: 5000

Rewards:
- Credits Reward: 10000
- Experience Reward: 2000
- Reputation Reward: 50

Interaction:
- Is Hostile: true
- Damage Per Second: 100
- Interaction Radius: 50000 (500m)
```

### 2. Create a Discoverable Location

**In Editor:**
1. Content Browser → Right-click → Blueprint → Data Asset
2. Choose `DiscoveryDataAsset`
3. Name it: `DA_Discovery_AncientRuins`

**Configure:**
```
Basic Info:
- Discovery Name: "Ancient Ruins"
- Description: "Remains of a lost civilization"
- Discovery Type: Archaeological Site
- Discovery ID: "Discovery_Ruins_001"

Detection:
- Signal Strength: 45
- Required Scanner Type: Exploration
- Requires Deep Scan: true

Rarity:
- Rarity Tier: Rare
- Discovery Value: 10000

Rewards:
- Credits Reward: 15000
- Experience Reward: 3000

Interaction:
- Can Be Explored: true
- Exploration Time: 300 (5 minutes)
- Can Be Salvaged: true
- Salvage Value: 5000
```

### 3. Add Components to Actor

**Create Actor Blueprint:**
1. Create `BP_Anomaly_Singularity`
2. Add Components:
   - `ScannableObjectComponent`
   - `DiscoveryComponent`

**Configure ScannableObjectComponent:**
```
- Display Name: "Unknown Signal"
- Signal Type: Anomaly
- Signature Strength: 85
- Object Size: 100000 (1km)
```

**Configure DiscoveryComponent:**
```
- Anomaly Data: DA_Anomaly_GravitySingularity
- Is Unique: true
- Persistent: true
```

### 4. Place in Level

1. Drag `BP_Anomaly_Singularity` into level
2. Position in space
3. Test with player ship scanner

---

## Discovery Flow

```
Player → Scan Object → Reaches Required Detail Level
    ↓
DiscoveryComponent.MarkAsDiscovered()
    ↓
Registers with DiscoveryManagerSubsystem
    ↓
Rewards Given to Player
    ↓
UI Updated, Events Fired
    ↓
Player Can Interact (Harvest/Explore/etc.)
```

---

## Blueprint Usage

### Check if Discovery Exists

```cpp
// Get Discovery Manager
UDiscoveryManagerSubsystem* Manager = GetGameInstance()->GetSubsystem<UDiscoveryManagerSubsystem>();

// Check if discovered
if (Manager->IsDiscovered("Anomaly_GravSingularity_001"))
{
    // Already found
}
```

### Get All Rare Discoveries

```cpp
TArray<FDiscoveryRecord> RareFinds = Manager->GetDiscoveriesByRarity(ERarityTier::Rare);

for (const FDiscoveryRecord& Record : RareFinds)
{
    UE_LOG(LogTemp, Log, TEXT("Rare Discovery: %s"), *Record.DiscoveryName.ToString());
}
```

### Get Discovery Statistics

```cpp
FDiscoveryStatistics Stats = Manager->GetStatistics();

UE_LOG(LogTemp, Log, TEXT("Total Discoveries: %d"), Stats.TotalDiscoveries);
UE_LOG(LogTemp, Log, TEXT("Anomalies: %d"), Stats.AnomaliesDiscovered);
UE_LOG(LogTemp, Log, TEXT("Locations: %d"), Stats.LocationsDiscovered);
UE_LOG(LogTemp, Log, TEXT("Credits Earned: %d"), Stats.TotalCreditsEarned);
```

### Start Interaction

```cpp
UDiscoveryComponent* Discovery = Actor->FindComponentByClass<UDiscoveryComponent>();

FText Reason;
if (Discovery->CanInteract(EInteractionType::Harvest, Reason))
{
    Discovery->StartInteraction(EInteractionType::Harvest, PlayerController);
}
```

### Listen to Events

**In Blueprint:**

1. Get Discovery Manager reference
2. Bind to `OnDiscoveryRegistered` event:

```cpp
OnDiscoveryRegistered.AddDynamic(this, &AMyHUD::HandleNewDiscovery);

void AMyHUD::HandleNewDiscovery(const FDiscoveryRecord& Record)
{
    // Show notification
    DisplayDiscoveryNotification(Record.DiscoveryName, Record.RarityTier);
}
```

---

## Rarity System

| Rarity | Multiplier | Color | Use Case |
|--------|-----------|-------|----------|
| **Common** | 1.0x | Gray | Frequent finds, tutorial content |
| **Uncommon** | 1.5x | Green | Regular exploration rewards |
| **Rare** | 2.5x | Blue | Special locations, quest tie-ins |
| **Very Rare** | 4.0x | Purple | Endgame content, unique finds |
| **Legendary** | 5.0x | Orange | Once-per-playthrough discoveries |

**Design Tips:**
- Common: 50% spawn chance
- Uncommon: 30% spawn chance
- Rare: 15% spawn chance
- Very Rare: 4% spawn chance
- Legendary: 1% spawn chance or hand-placed

---

## Interaction Types

| Type | Default Time | Use Case |
|------|-------------|----------|
| **Scan** | 30s | Basic analysis |
| **Harvest** | Configurable | Extract resources from anomalies |
| **Explore** | Configurable | Enter and investigate locations |
| **Salvage** | 120s | Recover materials from derelicts |
| **Research** | 60s | Scientific analysis |
| **None** | - | No interaction possible |

---

## YAML Templates

See `Assets/ExplorationTemplates/`:

**AnomalyTemplates.yaml** - 10 complete templates:
1. Gravitational Singularity
2. Temporal Distortion Field
3. Plasma Energy Vortex
4. Stable Wormhole Aperture
5. High-Radiation Nebula
6. Quantum Foam Anomaly
7. Dark Matter Concentration
8. Ionized Plasma Storm
9. Subspace Tear
10. Unidentified Spatial Anomaly

**DiscoveryTemplates.yaml** - 10 complete templates:
1. Precursor Archaeological Site
2. Abandoned Battlecruiser
3. Frontier Station Echo
4. Classified Research Facility
5. Colony Ship Wreck
6. Unmarked Cargo Cache
7. Xenoarchaeological Monolith
8. Battle Debris Field
9. Secret Military Installation
10. The Silent Expanse

**Copy and customize** these templates for your game!

---

## Common Patterns

### Quest-Linked Discovery

```
1. Create DiscoveryDataAsset
2. Set "Triggered Quest ID" = "Quest_MyQuest"
3. When discovered → quest starts automatically
4. Optionally set "Unlocked Quest IDs" for quest chains
```

### Hostile Location

```
1. Set "Has Hostiles" = true
2. Set "Hostile Difficulty" = 1-10
3. Spawn enemies on interaction
4. Player must clear before exploring
```

### One-Time Discovery

```
1. Set "Is Unique" = true
2. Set "Can Revisit" = false
3. First discovery registers globally
4. Subsequent finds rejected
```

### Reusable Resource Node

```
1. Set "Is Reusable" = true
2. Set "Interaction Cooldown" = 600 (10 min)
3. Player can harvest multiple times
4. Cooldown prevents exploitation
```

---

## Troubleshooting

**Discovery not registering:**
- ✓ Verify DiscoveryID is unique
- ✓ Check scan reached MinimumScanLevel
- ✓ Ensure DiscoveryManagerSubsystem exists

**Can't interact:**
- ✓ Must be discovered first (bDiscovered = true)
- ✓ Check interaction type is allowed
- ✓ Verify not on cooldown
- ✓ Check DiscoveryState != Depleted

**No rewards:**
- ✓ Set reward values > 0 in Data Asset
- ✓ Verify GiveRewards() is called
- ✓ Check player systems are available

---

## Performance Tips

1. **Limit Active Discoveries**: 50-100 per sector max
2. **Use LOD**: Disable tick when far from player
3. **Cache Queries**: Don't call GetAllDiscoveries() every frame
4. **Unique IDs**: Use descriptive, collision-free IDs
5. **Persistence**: Only mark as Persistent if truly unique

---

## Next Steps

- Read [ExplorationSystemGuide.md](ExplorationSystemGuide.md) for full documentation
- Explore [AnomalyTemplates.yaml](ExplorationTemplates/AnomalyTemplates.yaml) for examples
- Explore [DiscoveryTemplates.yaml](ExplorationTemplates/DiscoveryTemplates.yaml) for examples
- Integrate with Quest System for narrative content
- Create custom visual effects for rare discoveries

---

**Happy Exploring! 🚀**

*"The universe is full of mysteries. Go find them."*
