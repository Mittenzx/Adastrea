# Way System - Complete Guide

## Table of Contents

- [Introduction](#introduction)
- [What Are Ways?](#what-are-ways)
- [Way Networks](#way-networks)
- [Core Concepts](#core-concepts)
- [Getting Started](#getting-started)
- [Documentation Structure](#documentation-structure)
- [Integration with Other Systems](#integration-with-other-systems)
- [Quick Links](#quick-links)

## Introduction

The Way System is a unique guild and reputation framework in Adastrea that enables **small specialized guilds** (Ways) to form **micro-alliances** (Way Networks) based on shared values (Precepts). This creates emergent political structures and dynamic relationships where player actions with one guild member can affect reputation with an entire network.

### Design Philosophy

- **Small and Focused**: Ways are specialized guilds (50-1000 members), not massive organizations
- **Value-Driven**: Ways form networks based on shared Precepts (values), not convenience
- **Economic, Not Political**: Ways handle trade, production, and practical governance—not diplomacy or warfare
- **Emergent Gameplay**: Network dynamics create natural quest chains and story opportunities
- **Designer-Friendly**: Everything uses Blueprint Data Assets—no C++ knowledge required

### Key Benefits

- 🎯 **Specialization**: Each Way focuses on specific industries (mining, crafting, exploration, etc.)
- 🤝 **Micro-Alliances**: 2-5 Ways form tight-knit networks with shared goals
- 📈 **Reputation Spillover**: Actions with one member affect the entire network
- 💰 **Economic Integration**: Supply chains and trade agreements between Ways
- 🏛️ **Sector Governance**: Ways participate in practical sector council decisions
- 🎮 **No C++ Required**: Designers create everything through Data Assets and Blueprints

## What Are Ways?

Ways are **small specialized guilds**—groups of people with shared interests, passions, and trade specializations. Each Way focuses on specific industries and builds a quality reputation over time.

### Examples of Ways

- **Dwarven Mining Way** - Hardy miners extracting ore from dangerous asteroids
- **Way of Pure Crafters** - Master artisans creating elite spaceship components
- **Scout's League** - Brave explorers mapping uncharted territories
- **Freight Haulers Union** - Transport specialists moving cargo between stations
- **Medical Services Guild** - Healthcare providers and pharmaceutical experts

### Industry Specializations

Ways can specialize in 15 different industries:

| Industry | Description | Examples |
|----------|-------------|----------|
| **Mining** | Extract raw materials | Asteroid mining, gas harvesting |
| **Refining** | Process raw materials | Ore processing, fuel refining |
| **Manufacturing** | Craft components | Ship parts, modules, equipment |
| **Shipbuilding** | Build and repair ships | Hull construction, retrofitting |
| **Trading** | Buy and sell goods | Merchant services, brokers |
| **Exploration** | Survey and discover | Mapping, reconnaissance |
| **Research** | Develop technology | R&D, innovation |
| **Agriculture** | Produce food/organics | Hydroponics, livestock |
| **Medical** | Healthcare services | Clinics, pharmaceuticals |
| **Security** | Protection services | Escorts, station defense |
| **Salvage** | Recovery and recycling | Wreck salvage, material recovery |
| **Entertainment** | Leisure services | Casinos, theaters, tourism |
| **Transport** | Cargo and passengers | Hauling, ferry services |
| **Engineering** | Technical services | Maintenance, upgrades |
| **Custom** | Unique specialties | Define your own |

### Quality Reputation Tiers

Ways build reputations for the quality of their work:

- **Basic** - Cheap, functional, gets the job done
- **Standard** - Average quality, fair price
- **Quality** - Above average, reliable ⭐
- **Premium** - High quality, expensive ⭐⭐
- **Elite** - Exceptional quality, very expensive ⭐⭐⭐
- **Legendary** - Best in class, priceless reputation ⭐⭐⭐⭐

## Way Networks

Way Networks are **micro-alliances** of 2-5 Ways that share common Precepts (values) and work together toward mutual goals. Networks create emergent political structures smaller and more focused than traditional faction alliances.

### Why Networks Matter

When a player earns reputation with one network member, they gain **partial reputation** with all other members through **spillover**. This creates:

- **Interconnected Relationships**: Actions ripple through networks
- **Network Bonuses**: Aligned Feats earn reputation multipliers
- **Shared Benefits**: Trading discounts, exclusive missions, joint resources
- **Political Drama**: Competing networks with opposing values
- **Emergent Stories**: Network dynamics create natural quest chains

### Example Networks

#### The Craftsmen's Compact (Engineering)
**Purpose**: Excellence in ship construction and engineering  
**Members**: Engineers' Guild, Ship-Builders Union, Refinery Collective  
**Shared Precepts**: Craftsmanship (90), Mastery (85), Innovation (70)  
**Mechanics**: 40% spillover, 1.8x alignment bonus, 30 reputation threshold

#### The Frontier Alliance (Exploration)
**Purpose**: Exploration and discovery of uncharted space  
**Members**: Scout's League, Cartographer's Society, Pioneer Traders  
**Shared Precepts**: Discovery (95), Freedom (80), Survival (70)  
**Mechanics**: 35% spillover, 2.0x alignment bonus, 25 reputation threshold

#### The Honor Circle (Combat)
**Purpose**: Upholding warrior traditions and protective services  
**Members**: Warrior's Guild, Duelist Academy, Protector's Order, Bounty Hunters  
**Shared Precepts**: Honor (95), Strength (85), Justice (75)  
**Mechanics**: 30% spillover, 1.7x alignment bonus, 35 reputation threshold

## Core Concepts

### 1. Precepts (Values)

Precepts are the philosophical values that define what a Way stands for. Ways and networks align based on shared Precepts:

- **Craftsmanship** - Excellence in creation
- **Mastery** - Skill perfection
- **Innovation** - Technological advancement
- **Discovery** - Exploration and learning
- **Honor** - Ethical conduct
- **Strength** - Physical/mental power
- **Prosperity** - Wealth and success
- **Freedom** - Independence and autonomy
- **Survival** - Endurance and resilience
- **Justice** - Fairness and righteousness
- Plus many more...

Each Precept has an **Importance Value** (0-100) that defines how much the Way values it.

### 2. Feats (Legendary Accomplishments)

Feats are legendary deeds that players accomplish. When you complete a Feat, you gain reputation with Ways whose Precepts align with the Feat's values:

```
Example: "Master Ship Builder" Feat
  Aligned Precepts: Craftsmanship (90), Mastery (85)
  
  Player completes this Feat:
  → Gain 100 reputation with Engineers' Guild (aligned)
  → Gain 40 reputation with other Craftsmen's Compact members (spillover)
  → Gain 1.8x bonus on future crafting Feats (network bonus)
```

### 3. Supply Chains

Ways trade with each other to create economic supply chains:

```
Dwarven Mining Way (produces iron ore)
    ↓ supplies to ↓
Way of Pure Crafters (consumes ore, produces ship parts)
    ↓ supplies to ↓
Shipbuilders Guild (consumes parts, produces ships)
```

Supply chains define:
- **What each Way produces** (output resources)
- **What each Way consumes** (input resources)
- **Trade agreements** between Ways (pricing, quantities)
- **Supply relationships** (partnerships, exclusivity)

### 4. Sector Councils

Each game sector has a **council** where Way representatives make practical decisions about:

- **Tax rates** (security, infrastructure, administration)
- **Budget allocation** (how collected taxes are spent)
- **Policies** (trade rules, patrol schedules, resource management)

This is **NOT politics or diplomacy**—it's practical administration and governance.

#### Council Mechanics

- Each Way with sector presence gets a representative
- Voting weight based on Way's influence and member count
- Decisions made by weighted voting (51% threshold typical)
- Players can join councils if they lead or represent a Way

### 5. Reputation Spillover

When networks are active, reputation gains transfer between members:

```
Player completes Feat aligned with Engineers' Guild
  ↓
Engineers' Guild: +100 reputation (direct)
  ↓
Craftsmen's Compact Network: 40% spillover active
  ↓
Ship-Builders Union: +40 reputation (spillover)
Refinery Collective: +40 reputation (spillover)
```

Spillover percentages:
- **20-30%**: Loose alliance
- **30-50%**: Standard network (most common)
- **50-70%**: Very tight alliance
- **70-100%**: Essentially one faction

### 6. Network Bonuses

Networks apply reputation multipliers for aligned Feats:

```
Network Alignment Bonus: 1.8x

Player completes crafting Feat:
  Base reputation gain: +50
  With network bonus: +50 × 1.8 = +90 reputation

Applies to all Feats matching network's Shared Precepts
```

Bonus multipliers:
- **1.2-1.5x**: Minor encouragement
- **1.5-2.0x**: Standard bonus (recommended)
- **2.0-3.0x**: Major incentive
- **3.0x+**: Extremely powerful (use sparingly)

## Getting Started

### For Players

1. **Explore Ways** - Discover specialized guilds in different sectors
2. **Complete Feats** - Accomplish deeds aligned with Way Precepts
3. **Build Reputation** - Gain standing through aligned actions
4. **Join Networks** - Reach reputation thresholds to access network bonuses
5. **Participate in Councils** - Eventually lead or represent your own Way

### For Designers

1. **Create Ways** - Define specialized guilds with Data Assets
2. **Define Industries** - Set what they produce and consume
3. **Establish Supply Chains** - Connect Ways economically
4. **Form Networks** - Create micro-alliances based on shared Precepts
5. **Configure Councils** - Set up sector governance with voting weights

### For Developers

1. **Use Data Assets** - All Way configuration through UWayDataAsset
2. **Integrate VerseSubsystem** - Register networks and track reputation
3. **Record Feats** - Use RecordFeatWithNetworkEffects for spillover
4. **Query Reputation** - Check player standing with Ways and networks
5. **Apply Benefits** - Grant bonuses based on network membership

## Documentation Structure

The Way System documentation is organized into specialized guides:

### Getting Started
- **[WaySystemOverview.md](WaySystemOverview.md)** - High-level introduction
- **[WaySystemQuickReference.md](WaySystemQuickReference.md)** - Quick lookup
- **[WayCreationGuide.md](WayCreationGuide.md)** - Step-by-step Way creation

### Way Networks
- **[WayNetworkGuide.md](WayNetworkGuide.md)** - Complete network documentation
- **[WayNetworkQuickStart.md](WayNetworkQuickStart.md)** - 5-minute network setup
- **[WayNetworkWorkflow.md](WayNetworkWorkflow.md)** - Designer workflow

### Blueprint Integration
- **[WayNetworkBlueprintExamples.md](../Blueprints/WayNetworkBlueprintExamples.md)** - Copy-paste Blueprint examples

### Templates
- **[WayNetworkTemplates/](WayNetworkTemplates/)** - Ready-to-use YAML templates

### Developer Reference
- **[WaySystemAPIReference.md](WaySystemAPIReference.md)** - C++ and Blueprint API

## Integration with Other Systems

### Faction System

- Ways can belong to larger Factions
- Networks operate at micro-level within Factions
- Faction diplomacy affects network relationships
- Ways inherit some faction characteristics

### Trading System

- Ways produce and consume resources
- Supply chains connect Way economies
- Network members may share trade routes
- Better prices when trading with network members
- Joint contracts available to high-reputation players

### Station System

- Stations can be owned or operated by Ways
- Way faction assignment through `ASpaceStation::Faction`
- Network bonuses apply at all member stations
- Special network-exclusive services available

### Personnel System

- Personnel can belong to Ways
- Skills and roles align with Way industries
- Career progression through Way reputation
- Network connections provide opportunities

### AI System

- Way-level AI for strategic decisions
- Network members coordinate behaviors
- Shared threat assessment and resource allocation
- Collective response to player actions

### Quest System

- Network quests benefit entire alliance
- Feat completion with network spillover
- Quest chains connecting network members
- Network-exclusive missions and rewards

## Quick Links

### Essential Reading
1. [Way System Overview](WaySystemOverview.md) - Start here
2. [Way Creation Guide](WayCreationGuide.md) - Create your first Way
3. [Way Network Quick Start](WayNetworkQuickStart.md) - Create your first network

### Deep Dives
- [Way Network Guide](WayNetworkGuide.md) - Complete network documentation
- [Way Network Workflow](WayNetworkWorkflow.md) - Designer workflow
- [Way Network Blueprint Examples](../Blueprints/WayNetworkBlueprintExamples.md) - Code examples

### Templates and References
- [Way Network Templates](WayNetworkTemplates/) - YAML configurations
- [Way System API Reference](WaySystemAPIReference.md) - Developer reference
- [Way System Quick Reference](WaySystemQuickReference.md) - Quick lookup

### Related Systems
- [Faction System Guide](BlueprintFactionAssetGuide.md)
- [Trading System Guide](TradingSystemGuide.md)
- [Personnel System Guide](PersonnelSystemGuide.md)
- [Station System](../Content/Blueprints/STATION_EDITOR_README.md)

## Summary

The Way System provides:
- ✅ **Small specialized guilds** focused on specific industries
- ✅ **Micro-alliances** of 2-5 Ways with shared values
- ✅ **Reputation spillover** creating interconnected relationships
- ✅ **Network bonuses** rewarding aligned actions
- ✅ **Economic gameplay** through supply chains and trading
- ✅ **Practical governance** via sector councils
- ✅ **Emergent stories** from network dynamics
- ✅ **Designer-friendly** Data Asset workflow

Start with [WaySystemOverview.md](WaySystemOverview.md) for introduction, then follow [WayCreationGuide.md](WayCreationGuide.md) to create your first Way!

---

**Version**: 1.0  
**Last Updated**: 2025-11-10  
**Maintained by**: Adastrea Development Team
