# Way Network System Guide

## Overview

The Way Network system enables micro-alliances between small guilds (Ways) who share aligned goals and Precepts. This creates emergent political structures where players who earn reputation with one member gain partial reputation with the entire network.

## Key Concepts

### What is a Way Network?

A Way Network is a formal alliance of 2-5 Ways (guilds, schools, or syndicates) that share common Precepts and work together toward mutual goals. Networks are smaller and more focused than traditional faction alliances, creating intimate political relationships.

### Design Philosophy

- **Small and Focused**: Networks typically have 2-5 members (not massive coalitions)
- **Precept-Based**: Networks form around shared philosophical values, not just convenience
- **Dynamic Reputation**: Player actions with one member affect reputation with all members
- **Emergent Gameplay**: Networks create natural quest chains and story opportunities

## Network Components

### Core Identity

Each network has:
- **Network Name**: The public identity (e.g., "The Craftsmen's Compact")
- **Description**: The network's purpose and goals
- **Network ID**: Unique identifier for game systems
- **Network Color**: Visual representation in UI

### Member Ways

Networks consist of 2-5 Ways, each with:
- **Member Way**: Reference to the Way Data Asset
- **Influence Level** (0-100): How much weight this Way has in network decisions
- **Commitment Level** (0-100): How strongly this Way responds to network events
- **Role Description**: Optional flavor text about their role in the network

### Shared Philosophy

Networks define:
- **Shared Precepts**: Core values that unite the network (2-4 typical)
- **Reputation Spillover** (0-100%): How much reputation gain transfers to other members
- **Network Alignment Bonus** (1.0-3.0x): Reputation multiplier for aligned Feats
- **Minimum Reputation Threshold**: Required rep to gain network-wide bonuses

## Network Examples

### The Craftsmen's Compact

**Purpose**: Excellence in ship construction and engineering
**Members**:
- Engineers' Guild (Influence: 70, Commitment: 90)
- Ship-Builders Union (Influence: 80, Commitment: 85)
- Refinery Collective (Influence: 50, Commitment: 70)

**Shared Precepts**:
- Craftsmanship (Importance: 90)
- Mastery (Importance: 85)
- Innovation (Importance: 70)

**Mechanics**:
- Reputation Spillover: 40%
- Network Bonus: 1.8x for crafting-related Feats
- Minimum Threshold: 30 reputation

**Gameplay Impact**:
- Player completing "Master Ship-Builder" Feat gains full reputation with primary Way
- Other network members gain 40% spillover reputation
- All members give 1.8x bonus reputation for future crafting Feats

### The Frontier Alliance

**Purpose**: Exploration and discovery of uncharted space
**Members**:
- Scout's League (Influence: 75, Commitment: 95)
- Cartographer's Society (Influence: 60, Commitment: 80)
- Pioneer Traders (Influence: 65, Commitment: 75)

**Shared Precepts**:
- Discovery (Importance: 95)
- Freedom (Importance: 80)
- Survival (Importance: 70)

**Mechanics**:
- Reputation Spillover: 35%
- Network Bonus: 2.0x for exploration Feats
- Minimum Threshold: 25 reputation

**Gameplay Impact**:
- Player discovering new sectors gains reputation with entire network
- Network provides better exploration missions and discounts
- Members share cartographic data and safe havens

### The Honor Circle

**Purpose**: Upholding warrior traditions and protective services
**Members**:
- Warrior's Guild (Influence: 85, Commitment: 90)
- Duelist Academy (Influence: 70, Commitment: 85)
- Protector's Order (Influence: 75, Commitment: 80)
- Bounty Hunter Collective (Influence: 60, Commitment: 70)

**Shared Precepts**:
- Honor (Importance: 95)
- Strength (Importance: 85)
- Justice (Importance: 75)

**Mechanics**:
- Reputation Spillover: 30%
- Network Bonus: 1.7x for combat/honor Feats
- Minimum Threshold: 35 reputation

**Gameplay Impact**:
- Honorable combat victories boost reputation across all members
- Dishonorable actions hurt reputation with entire network
- Members offer specialized combat training and equipment

## Creating a Way Network

### Step 1: Define Member Ways

First, create the individual Way Data Assets for each guild:

```yaml
# Example: Engineers' Guild
WayName: "Engineers' Guild"
WayID: "EngineersGuild"
bIsSchool: true
CorePrecepts:
  - Precept: Craftsmanship
    ImportanceValue: 90
    Description: "Excellence in all things built"
  - Precept: Mastery
    ImportanceValue: 85
    Description: "Perfect your skills"
  - Precept: Innovation
    ImportanceValue: 75
    Description: "Push technological boundaries"
```

### Step 2: Create Network Data Asset

1. **In Unreal Editor**:
   - Content Browser → Right-click → Blueprint → Blueprint Class
   - Parent Class: WayNetworkDataAsset
   - Name: `DA_Network_CraftsmenCompact`

2. **Configure Basic Info**:
   - Network Name: "The Craftsmen's Compact"
   - Network ID: `CraftsmenCompact`
   - Description: "An alliance of master craftsmen and engineers"
   - Network Color: Choose a representative color

3. **Add Member Ways**:
   - Add array element for each member
   - Set Member Way reference to the Way Data Asset
   - Set Influence Level (how much weight they have)
   - Set Commitment Level (how strongly they react)
   - Add Role Description (optional flavor)

4. **Define Shared Precepts**:
   - Add the 2-4 Precepts all members share
   - Set importance values for each

5. **Configure Mechanics**:
   - Reputation Spillover Percent: 30-50% typical
   - Network Alignment Bonus: 1.5-2.5x typical
   - Minimum Reputation Threshold: 20-40 typical

6. **Set Network Status**:
   - bIsActive: true (or false if dissolved)
   - Formation Date: Set for historical flavor

### Step 3: Register Network with VerseSubsystem

In your game initialization (Blueprint or C++):

```cpp
// C++ example
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the VerseSubsystem
    UVerseSubsystem* VerseSystem = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
    
    // Load your network asset
    UWayNetworkDataAsset* CraftsmenNetwork = LoadObject<UWayNetworkDataAsset>(
        nullptr, 
        TEXT("/Game/DataAssets/Networks/DA_Network_CraftsmenCompact")
    );
    
    // Register it
    if (VerseSystem && CraftsmenNetwork)
    {
        VerseSystem->RegisterNetwork(CraftsmenNetwork);
    }
}
```

Blueprint alternative:
1. Get Game Instance → Get Subsystem (VerseSubsystem)
2. Load Data Asset (your network)
3. Call "Register Network" with your network asset

## Using Networks in Gameplay

### Checking Network Membership

```cpp
// Check if a Way belongs to any networks
UVerseSubsystem* VerseSystem = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
TArray<UWayNetworkDataAsset*> Networks = VerseSystem->GetNetworksForWay(SomeWay);

if (Networks.Num() > 0)
{
    // This Way is part of one or more networks
}
```

### Calculating Network Reputation

```cpp
// Get player's overall standing with a network
float NetworkScore = VerseSystem->GetNetworkVerseScore(CraftsmenNetwork);

// Check if player qualifies for network bonuses
bool QualifiesForBonuses = VerseSystem->QualifiesForNetworkBonuses(CraftsmenNetwork);
```

### Recording Feats with Network Effects

```cpp
// Award a Feat with automatic network spillover
VerseSystem->RecordFeatWithNetworkEffects(MasterCraftsmanFeat, true);

// This will:
// 1. Record the Feat for the player
// 2. Calculate alignment with all registered networks
// 3. Apply reputation spillover to network members
// 4. Apply network alignment bonuses where applicable
```

## Blueprint Integration

### Network Information Widget

Create a UI widget to display network information:

```
Network Name: [NetworkName]
Members: [MemberCount]
Your Standing: [GetNetworkVerseScore]
Bonuses Active: [QualifiesForNetworkBonuses]

Member List:
- [MemberWay.WayName] (Influence: [InfluenceLevel])
- [MemberWay.WayName] (Influence: [InfluenceLevel])
```

### Network Quest Generator

Use networks to create interconnected quest chains:

```cpp
// Find all networks a Way belongs to
TArray<UWayNetworkDataAsset*> MemberNetworks = GetNetworksForWay(QuestGiver);

// Generate quest that benefits the entire network
if (MemberNetworks.Num() > 0)
{
    // Quest rewards spillover to all network members
    // Quest may involve coordinating with other network Ways
}
```

## Advanced Features

### Network Dissolution

Networks can dissolve due to political changes:

```cpp
// Deactivate a network
CraftsmenNetwork->bIsActive = false;

// Or completely unregister it
VerseSystem->UnregisterNetwork(CraftsmenNetwork);
```

### Dynamic Network Formation

Create systems where new networks form based on player actions:

```cpp
// Check if multiple Ways have high reputation with player
// and share common Precepts
if (CanFormNetwork(Way1, Way2, Way3))
{
    UWayNetworkDataAsset* NewNetwork = CreateDynamicNetwork(Way1, Way2, Way3);
    VerseSystem->RegisterNetwork(NewNetwork);
}
```

### Network Conflicts

Networks with opposing Precepts can create interesting conflicts:

```cpp
// The Craftsmen's Compact values Craftsmanship
// The Profit Cartel values Prosperity and Efficiency
// Player must choose which network to support

if (PlayerSupports(CraftsmenNetwork))
{
    // Lose reputation with Profit Cartel
}
```

## Best Practices

### Network Size

- **2-3 members**: Tight-knit, strong identity
- **4-5 members**: Diverse perspectives, more complex dynamics
- **6+ members**: Too large, consider splitting into multiple networks

### Precept Alignment

- All members should share at least 2 common Precepts
- Precept importance values should be similar across members
- Avoid networks with contradictory Precepts

### Spillover Percentages

- **20-30%**: Loose alliance, minimal cooperation
- **30-50%**: Standard network, good cooperation
- **50-70%**: Tight alliance, very strong cooperation
- **70-100%**: Effectively a single faction (not a true network)

### Network Bonus Multipliers

- **1.2-1.5x**: Minor bonus, encourages participation
- **1.5-2.0x**: Standard bonus, clear benefit
- **2.0-3.0x**: Major bonus, creates strong incentive
- **3.0x+**: Extremely powerful, use sparingly

## Integration with Existing Systems

### Faction System

- Ways can belong to larger Factions
- Networks operate at micro-level within Factions
- Faction diplomacy affects network relationships

### Trading System

- Network members may share trade routes
- Better prices when trading with network members
- Joint contracts available to high-reputation players

### Station System

- Stations can be owned by network Ways
- Network bonuses apply at all member stations
- Special network-exclusive services

### AI System

- Network members coordinate AI behaviors
- Shared threat assessment and resource allocation
- Collective response to player actions

## Troubleshooting

### Network Not Applying Bonuses

Check:
- Is network registered with VerseSubsystem?
- Is network active (bIsActive = true)?
- Does player have minimum reputation with any member?
- Are Feat Precepts aligned with network Precepts?

### Spillover Not Working

Verify:
- RecordFeatWithNetworkEffects is being called
- bApplyNetworkEffects parameter is true
- Network has spillover percentage > 0
- Member Ways are properly configured

### Network Score Always Zero

Ensure:
- Member Ways have Precept values configured
- Player has completed Feats with aligned Precepts
- Network's shared Precepts match member Precepts

## Example YAML Templates

See the `/Assets/WayNetworkTemplates/` directory for ready-to-use templates:

- `CraftsmenCompact.yaml` - Engineering and crafting alliance
- `FrontierAlliance.yaml` - Exploration and discovery network
- `HonorCircle.yaml` - Warrior guilds network
- `MerchantCoalition.yaml` - Trade and commerce alliance
- `ScholarsSyndicate.yaml` - Knowledge and research network

## Summary

Way Networks create micro-alliances between small guilds with shared Precepts:
- **2-5 member Ways** form focused networks
- **Reputation spillover** creates interconnected relationships
- **Network bonuses** reward alignment with shared values
- **Dynamic gameplay** emerges from network politics

Use networks to create rich, emergent storytelling opportunities where player choices reverberate through interconnected communities of shared values.
