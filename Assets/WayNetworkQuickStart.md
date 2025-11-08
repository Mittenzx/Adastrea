# Way Network System - Quick Start Guide

## What Are Way Networks?

Way Networks are micro-alliances of 2-5 small guilds (Ways) who share common Precepts and work together. When you earn reputation with one member, you gain partial reputation with the entire network.

## Quick Setup (5 Minutes)

### 1. Create Member Ways (if not existing)

Create 2-5 Way Data Assets with shared Precepts:

```yaml
# Example: Engineers' Guild
WayName: "Engineers' Guild"
WayID: "EngineersGuild"
bIsSchool: true
CorePrecepts:
  - Precept: Craftsmanship (90)
  - Precept: Mastery (85)
  - Precept: Innovation (70)
```

### 2. Create Network Data Asset

**In Unreal Editor:**
1. Content Browser → Right-click → Blueprint Class
2. Parent: `WayNetworkDataAsset`
3. Name: `DA_Network_YourNetwork`

**Configure:**
- Network Name: "The Craftsmen's Compact"
- Network ID: `CraftsmenCompact`
- Add 2-5 Member Ways with Influence/Commitment levels
- Set Shared Precepts (copy from member Ways)
- Configure spillover: 30-50% typical
- Set alignment bonus: 1.5-2.0x typical

### 3. Register Network in Game

**Blueprint:**
```
Game Instance → Get Subsystem (VerseSubsystem)
→ Register Network (your network asset)
```

**C++:**
```cpp
UVerseSubsystem* Verse = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
Verse->RegisterNetwork(NetworkAsset);
```

### 4. Award Feats with Network Effects

```cpp
// Player completes a Feat
Verse->RecordFeatWithNetworkEffects(FeatAsset, true);

// Automatic:
// - Reputation with aligned Way
// - Spillover to other network members
// - Network bonus multiplier applied
```

## Example Networks

### The Craftsmen's Compact (Engineering)
- **Members**: Engineers Guild, Ship-Builders Union, Refinery Collective
- **Precepts**: Craftsmanship (90), Mastery (85), Innovation (70)
- **Spillover**: 40% | **Bonus**: 1.8x | **Threshold**: 30

### The Frontier Alliance (Exploration)
- **Members**: Scout's League, Cartographers Society, Pioneer Traders
- **Precepts**: Discovery (95), Freedom (80), Survival (70)
- **Spillover**: 35% | **Bonus**: 2.0x | **Threshold**: 25

### The Honor Circle (Combat)
- **Members**: Warrior's Guild, Duelist Academy, Protector's Order
- **Precepts**: Honor (95), Strength (85), Justice (75)
- **Spillover**: 30% | **Bonus**: 1.7x | **Threshold**: 35

## Key Functions

### Check Network Membership
```cpp
TArray<UWayNetworkDataAsset*> Networks = Verse->GetNetworksForWay(SomeWay);
```

### Get Network Reputation
```cpp
float NetworkScore = Verse->GetNetworkVerseScore(Network);
bool QualifiesForBonuses = Verse->QualifiesForNetworkBonuses(Network);
```

### Award Feat with Network Effects
```cpp
Verse->RecordFeatWithNetworkEffects(Feat, true);
```

## Configuration Guidelines

### Network Size
- **2-3 members**: Tight-knit, strong identity
- **4-5 members**: Diverse, complex dynamics
- **6+ members**: Too large, split into multiple networks

### Reputation Spillover
- **20-30%**: Loose alliance
- **30-50%**: Standard network (recommended)
- **50-70%**: Very tight alliance
- **70-100%**: Essentially one faction

### Network Bonus
- **1.2-1.5x**: Minor encouragement
- **1.5-2.0x**: Standard bonus (recommended)
- **2.0-3.0x**: Major incentive
- **3.0x+**: Extremely powerful (use sparingly)

### Minimum Threshold
- **10-20**: Easy access, low commitment
- **20-40**: Standard requirement (recommended)
- **40-60**: Significant investment
- **60+**: Elite status only

## Common Use Cases

### 1. Interconnected Quest Chains
- Quest from one guild affects entire network
- Completing network objectives unlocks bonuses
- Betraying one member hurts network reputation

### 2. Shared Resources
- Stations owned by network members
- Shared trade routes and discounts
- Joint training programs and equipment

### 3. Political Drama
- Competing networks with opposing values
- Player must choose alliances carefully
- Network conflicts create meaningful choices

### 4. Emergent Storytelling
- Player actions ripple through networks
- Rival networks respond to player choices
- Dynamic political landscape

## Blueprint Workflow

### Create Network UI Widget
```
Get Network Name → Display Network Title
Get Member Count → Show "X member guilds"
Get Network Verse Score → Show reputation bar
Qualifies For Network Bonuses → Enable special offers
Get Shared Precepts → Display network values
```

### Network-Based Quest System
```
Find Networks For Way (quest giver) →
If in network: Generate network-wide quest
Reward applies to all members with spillover
```

### Network Benefits Shop
```
Check Qualifies For Network Bonuses →
If true: Enable network-exclusive items/services
Apply network member discounts
```

## Templates Available

See `/Assets/WayNetworkTemplates/` for ready-to-use YAML configs:
- `CraftsmenCompact.yaml` - Engineering alliance
- `FrontierAlliance.yaml` - Exploration network
- `HonorCircle.yaml` - Warrior guilds
- `MerchantCoalition.yaml` - Trade network
- `ScholarsSyndicate.yaml` - Research alliance

## Troubleshooting

**Network not applying bonuses?**
- Check network is registered with VerseSubsystem
- Verify bIsActive = true
- Ensure player has minimum reputation
- Confirm Feat Precepts align with network

**Spillover not working?**
- Use RecordFeatWithNetworkEffects (not RecordFeat)
- Set bApplyNetworkEffects = true
- Verify spillover percentage > 0

**Network score always zero?**
- Ensure member Ways have configured Precepts
- Player must complete Feats with aligned Precepts
- Check shared Precepts match member Precepts

## Next Steps

1. **Full Documentation**: See `WayNetworkGuide.md` for complete details
2. **YAML Templates**: Copy from `/WayNetworkTemplates/` directory
3. **Integration**: Connect networks to quests, trading, stations
4. **Testing**: Award test Feats and verify spillover effects

## Summary

Way Networks create micro-alliances between 2-5 guilds with shared values:
- **Reputation spillover** links network members
- **Network bonuses** reward aligned Feats
- **Dynamic gameplay** from interconnected relationships
- **Emergent stories** from political networks

Start with one of the example networks, customize it, and expand from there!
