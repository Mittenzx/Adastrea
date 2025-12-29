# Economy & Trading Roadmap

**Part of**: [Main Project Roadmap](../../ROADMAP.md)  
**Last Updated**: December 29, 2025  
**Status**: 🎯 **PRIORITY #1** - Trade Simulator MVP Focus

> **⚠️ MVP PIVOT (Dec 24, 2025)**: This roadmap now focuses exclusively on Trade Simulator MVP content. All non-MVP features are deferred until post-validation.

---

## 🎯 Trade Simulator MVP Focus (12 Weeks)

**Current Week**: Week 1 of 12 (Dec 29 - Jan 4)  
**Goal**: Playable 30-minute trading demo by Week 12  
**Scope**: Buy low, sell high, upgrade ship, repeat

### MVP Content Requirements

**Phase 1 (Weeks 1-4)**: Hardcoded Prototype
- 1 ship (hardcoded stats)
- 2 stations (Agricultural, Industrial)
- 3 trade goods (Water, Food, Fuel)
- Basic trading UI
- Hardcoded prices

**Phase 2 (Weeks 5-8)**: Structured Version
- 3 ships (small, medium, large)
- 5-10 stations
- 10-20 trade goods
- Economy manager integration
- Data Asset-based

**Phase 3 (Weeks 9-12)**: Polished Demo
- Tutorial/instructions
- Visual polish
- Sound effects (basic)
- 30-minute experience

### What's Deferred (Post-MVP)

- ❌ Trade contracts (quest system)
- ❌ AI traders (autonomous trading)
- ❌ Faction diplomacy effects (beyond pricing)
- ❌ Way system (guild networks)
- ❌ Complex market simulation
- ❌ Trade route optimization

---

## Overview

This roadmap covers the economic systems including Trading, Markets, Factions, and Way networks (guilds). 

**Current Status**: 
- Code: 85% Complete (MVP subset identified via Phase 2.1 analysis)
- MVP Content: 10% Complete (Week 1 in progress)
- Post-MVP Content: 0% (Deferred)

---

## Table of Contents

- [Trading System](#trading-system)
- [Market System](#market-system)
- [Faction System](#faction-system)
- [Way System (Guilds)](#way-system-guilds)
- [Economic Simulation](#economic-simulation)
- [Integration Status](#integration-status)

---

## Trading System

**Location**: `Source/Adastrea/Trading/`  
**Status**: 🎯 **MVP PRIORITY** - 46% Function Reduction Complete (Phase 2.1)  
**Documentation**: 
- `docs/systems/TRADING_ARCHITECTURE.md`
- `docs/mvp/TRADING_MVP_GUIDE.md`
- `docs/development/PHASE2_TRADING_SYSTEM_CATEGORIZATION.md`

### MVP-Critical Components (Keep)

#### ✅ PlayerTraderComponent (11 functions - Perfect Scope)
- Buy/sell cargo
- Credits and profit tracking
- Transaction validation
- **Status**: No changes needed, perfectly scoped for MVP

#### ✅ EconomyManager (7 functions - Clean Design)
- Market registration
- Supply/demand updates
- Price calculations
- **Status**: Clean subsystem design, no changes needed

#### ✅ MarketDataAsset (6 functions - Well-Balanced)
- Market configuration
- Inventory management
- Price queries
- **Status**: Well-balanced, no changes needed

#### ✅ CargoComponent (13 → 6-8 functions)
- Add/remove cargo (MVP-critical)
- Capacity checks (MVP-critical)
- ~~5-7 convenience wrappers~~ (removed Dec 27)
- **Status**: Cleanup complete

#### ⚠️ TradeItemDataAsset (9 → 2-3 functions)
- Item data (MVP-critical)
- ~~Logic functions~~ (removed Dec 27, moved to Market)
- **Status**: Cleanup complete, now data-focused

### Post-MVP Components (Deferred)

#### ⏸️ TradeContractDataAsset (11 functions)
- Quest system integration
- **Deferred**: No contracts in MVP

#### ⏸️ AITraderComponent (6-7 functions)
- Autonomous AI trading
- **Deferred**: Economy simulation only in MVP

#### ⏸️ TradeTransaction (3-4 functions)
- Detailed analytics
- **Deferred**: Basic tracking only in MVP

### Current Week Focus (Week 1)

**Hardcoded Prototype Goals**:
- [ ] Create 3 hardcoded trade goods (Water, Food, Fuel)
- [ ] Hardcode prices (Agricultural station cheap, Industrial expensive)
- [ ] Test buy/sell with PlayerTraderComponent
- [ ] Verify profit calculation
- [ ] No Data Assets yet (Week 5+)

### Roadmap

#### Phase 1: Basic Trade Content (Week 1-2) - Priority: CRITICAL

**Trade Items** (10 essential commodities)
- [ ] `DA_TradeItem_Food`
  - Category: Consumables
  - Base price: 100 credits/ton
  - Production: Agricultural stations
  - Consumption: All stations
- [ ] `DA_TradeItem_Water`
  - Category: Consumables
  - Base price: 50 credits/ton
  - Production: Ice mining
- [ ] `DA_TradeItem_Fuel`
  - Category: Resources
  - Base price: 200 credits/ton
  - Production: Refinery stations
- [ ] `DA_TradeItem_Minerals`
  - Category: Raw Materials
  - Base price: 150 credits/ton
  - Production: Mining stations
- [ ] `DA_TradeItem_OrePrecious`
  - Category: Raw Materials
  - Base price: 500 credits/ton
  - Production: Rare asteroid fields
- [ ] `DA_TradeItem_Technology`
  - Category: Manufactured Goods
  - Base price: 1000 credits/unit
  - Production: Research stations
- [ ] `DA_TradeItem_Electronics`
  - Category: Manufactured Goods
  - Base price: 800 credits/unit
  - Production: Industrial stations
- [ ] `DA_TradeItem_Weapons`
  - Category: Military
  - Base price: 2000 credits/unit
  - Production: Military stations
  - Restrictions: Illegal in some sectors
- [ ] `DA_TradeItem_Medicine`
  - Category: Consumables
  - Base price: 400 credits/unit
  - Production: Medical stations
- [ ] `DA_TradeItem_Luxury`
  - Category: Luxury Goods
  - Base price: 3000 credits/unit
  - Production: Core worlds
  - Demand: Wealthy stations

**Market Data Assets** (3 starter markets)
- [ ] `DA_Market_TradeHub_Alpha`
  - Type: Major trade hub
  - Sells: All basic commodities
  - Buys: All commodities
  - High volume, competitive prices
- [ ] `DA_Market_Mining_Station`
  - Type: Mining outpost
  - Sells: Minerals, ore, fuel
  - Buys: Food, water, equipment
  - Specialized market
- [ ] `DA_Market_Industrial_Complex`
  - Type: Industrial center
  - Sells: Technology, electronics, weapons
  - Buys: Raw materials, fuel
  - Manufacturing hub

**Trade UI** (1 widget)
- [ ] `WBP_TradingInterface`
  - Parent: `UTradingInterfaceWidget` (C++)
  - Layout: Buy/Sell split screen
  - Features:
    - Item list with prices
    - Quantity slider
    - Profit calculator
    - Market price trends
    - Transaction confirmation
  - Reference: `Assets/TradingSystemGuide.md`

**Test Implementation**
- [ ] Add TradingComponent to BP_SpaceStation_Basic
- [ ] Configure market inventory
- [ ] Test buy/sell transactions
- [ ] Verify price calculations
- [ ] Test cargo capacity limits

**SUCCESS MILESTONE**: Can dock and trade goods at station

---

#### Phase 2: Trade Expansion (Week 3-4) - Priority: HIGH

**More Trade Items** (10 additional commodities)
- [ ] Chemicals (industrial)
- [ ] Plastics (manufactured)
- [ ] Textiles (consumer goods)
- [ ] Machinery (equipment)
- [ ] Medical Supplies (specialized)
- [ ] Construction Materials (industrial)
- [ ] Rare Elements (precious)
- [ ] Biological Samples (research)
- [ ] Art & Artifacts (luxury)
- [ ] Contraband (illegal, high profit)

**More Markets** (7 specialized markets)
- [ ] Agricultural station market
- [ ] Military station market
- [ ] Research station market
- [ ] Pirate black market
- [ ] Border station market
- [ ] Core world market
- [ ] Frontier outpost market

**Trade Contracts** (10 contracts)
- [ ] `DA_Contract_Delivery_Food_Emergency`
  - Type: Urgent delivery
  - Cargo: Food (50 tons)
  - Destination: Remote station
  - Reward: 5000 credits
  - Time limit: 30 minutes
- [ ] `DA_Contract_Bulk_Minerals`
  - Type: Bulk haul
  - Cargo: Minerals (200 tons)
  - Reward: 3000 credits
  - No time limit
- [ ] Create 8 more contract variants
  - Mix of delivery, transport, and procurement
  - Various difficulty levels
  - Different time constraints
  - Faction-specific contracts

**Trade Routes** (5 profitable routes)
- [ ] Configure trade route data
  - Route A: Industrial → Mining → Trading Hub
  - Route B: Agricultural → Core World → Frontier
  - Route C: Mining → Industrial → Military
  - Route D: Research → Trading Hub → Core World
  - Route E: Luxury circuit (high-risk, high-reward)
- [ ] Document route profitability
- [ ] Add route markers to maps

**SUCCESS MILESTONE**: Multiple profitable trade routes available

---

#### Phase 3: Advanced Trading (Week 5-6) - Priority: MEDIUM

**Economic Simulation**
- [ ] Configure supply/demand parameters
- [ ] Set price volatility ranges
- [ ] Balance production/consumption rates
- [ ] Tune distance price modifiers
- [ ] Test market stability

**AI Trader NPCs**
- [ ] Create `BP_NPC_Ship_Trader` with AI
- [ ] Configure autonomous trading behavior
- [ ] Set trade route preferences
- [ ] Test NPC market impact

**Trade Quests** (10 quests)
- [ ] Trade tutorial quest
- [ ] "Become a Merchant" quest chain (5 quests)
- [ ] Faction trading quests (1 per faction)
- [ ] Trade guild advancement quests

**Advanced Features**
- [ ] Smuggling mechanics (illegal goods)
- [ ] Customs inspections
- [ ] Trade insurance
- [ ] Market manipulation detection
- [ ] Trade guild ranks and bonuses

---

## Market System

**Status**: ✅ 80% Complete (Code)  
**Integration**: Tied to Trading System

### Current State

Dynamic market simulation with:
- Supply/demand tracking
- Price fluctuations
- Distance-based pricing
- Faction relationship modifiers
- Production/consumption simulation

### Content Needs

**Market Configurations** (Per station type)
- [ ] Define inventory for each station type
- [ ] Set base prices for each market
- [ ] Configure supply/demand rates
- [ ] Balance refresh rates
- [ ] Test market stability

**Market Events** (10 events)
- [ ] Market crash (oversupply)
- [ ] Resource shortage (undersupply)
- [ ] Trade embargo (faction conflict)
- [ ] Trade boom (increased demand)
- [ ] Pirate raids (supply disruption)
- [ ] New trade route opening
- [ ] Station expansion (capacity increase)
- [ ] Military requisition (specific demand)
- [ ] Natural disaster (production loss)
- [ ] Technology breakthrough (new products)

---

## Faction System

**Location**: `Source/Adastrea/Factions/`  
**Status**: ✅ 95% Complete (Code), ❌ 10% Complete (Content)  
**Documentation**: [Faction Setup Guide](Assets/FactionSetupGuide.md)

### Current State

#### ✅ Completed (C++)
- FactionDataAsset for all factions
- UFactionDiplomacyManager for relationships
- UPlayerReputationComponent for player standing
- Trait system for faction modifiers
- Relationship system (-100 to +100 scale)
- Dynamic diplomacy system
- Faction AI integration

#### ❌ Missing (Content)
- Faction Data Assets (need 10)
- Faction territory definitions
- Faction fleet compositions
- Faction storylines
- Faction-specific rewards

### Roadmap

#### Phase 1: Core Factions (Week 1-2) - Priority: CRITICAL

**Essential Factions** (5 factions)
- [ ] `DA_Faction_Player`
  - Independent faction
  - Neutral starting reputation with all
  - Use template: `Assets/FactionSetupGuide.md`
- [ ] `DA_Faction_TradersUnion`
  - Peaceful merchant faction
  - Controls trade hubs
  - Positive toward traders
  - Traits: +10% trade prices
- [ ] `DA_Faction_MilitaryAlliance`
  - Organized military power
  - Controls military stations
  - Neutral toward civilians
  - Traits: +20% combat effectiveness
- [ ] `DA_Faction_Pirates`
  - Hostile raiders
  - Controls border regions
  - Aggressive toward all
  - Traits: +15% loot
- [ ] `DA_Faction_Explorers`
  - Scientific faction
  - Controls research stations
  - Friendly toward explorers
  - Traits: +10% scan range

**Faction Relationships** (Configure initial relations)
- [ ] Set starting reputation matrix
  - Traders ↔ Military: Neutral (0)
  - Traders ↔ Pirates: Hostile (-50)
  - Traders ↔ Explorers: Friendly (+30)
  - Military ↔ Pirates: War (-80)
  - Military ↔ Explorers: Allied (+40)
  - Explorers ↔ Pirates: Hostile (-30)

**Reputation Gains/Losses** (Define for each faction)
- [ ] Configure reputation events
  - Trading with faction: +5 rep
  - Completing faction quests: +10-50 rep
  - Attacking faction ships: -20 rep
  - Destroying faction ships: -50 rep
  - Smuggling: +10 pirate rep, -10 others
  - Research missions: +20 explorer rep

**SUCCESS MILESTONE**: Faction system functional with reputation tracking

---

#### Phase 2: Additional Factions (Week 3-4) - Priority: HIGH

**More Factions** (5 more)
- [ ] `DA_Faction_Miners`
  - Resource extraction guild
  - Controls mining stations
- [ ] `DA_Faction_CoreWorlds`
  - Central government
  - Controls core systems
- [ ] `DA_Faction_FrontierColonists`
  - Independent settlers
  - Controls frontier outposts
- [ ] `DA_Faction_CorporateSyndicate`
  - Mega-corporations
  - Controls industrial complexes
- [ ] `DA_Faction_Outlaws`
  - Organized crime
  - Controls black markets

**Faction Quests** (10+ quests)
- [ ] Create faction-specific quest chains
  - 2 quests per faction minimum
  - Reputation-gated quests
  - Faction storylines

**Faction Fleets** (Configure)
- [ ] Define fleet compositions per faction
  - Military: Heavy fighters, capital ships
  - Traders: Cargo haulers, escort fighters
  - Pirates: Fast raiders, hit-and-run ships
  - Explorers: Survey ships, science vessels

---

#### Phase 3: Faction Dynamics (Week 5-6) - Priority: MEDIUM

**Dynamic Diplomacy**
- [ ] Configure faction conflict events
- [ ] Set up territory control mechanics
- [ ] Implement faction war conditions
- [ ] Create peace treaty system
- [ ] Test faction AI decision-making

**Faction Benefits** (Per faction)
- [ ] Faction-specific ship access
- [ ] Faction-specific equipment
- [ ] Trade discounts
- [ ] Safe passage permits
- [ ] Special mission access

---

## Way System (Guilds)

**Location**: `Source/Adastrea/Way/`  
**Status**: ✅ 95% Complete (Code), ❌ 5% Complete (Content)  
**Documentation**: [Way System Guide](Assets/WaySystemGuide.md), [Way Network Guide](Assets/WayNetworkGuide.md)

### Current State

#### ✅ Completed (C++)
- WayDataAsset for guild definitions
- WayNetworkDataAsset for alliances
- UVerseSubsystem for network management
- Way reputation and Feats system
- Antagonist/rival system
- Way Network alliances
- Cross-guild interactions

#### ❌ Missing (Content)
- Way Data Assets (need 7+)
- Way Network Data Assets (need 3+)
- Way-specific quests
- Guild advancement trees
- Guild perks and bonuses

### Roadmap

#### Phase 1: Essential Ways (Week 1-2) - Priority: HIGH

**Core Guilds** (5 Ways)
- [ ] `DA_Way_TradersGuild`
  - Purpose: Merchant network
  - Benefits: Trade bonuses, market access
  - Advancement: Trade volume milestones
  - Use template: `Assets/WayNetworkTemplates/TradersGuild.yaml`
- [ ] `DA_Way_Freelancers`
  - Purpose: Independent contractors
  - Benefits: Quest variety, flexible reputation
  - Advancement: Quest completion
- [ ] `DA_Way_Explorers`
  - Purpose: Discovery network
  - Benefits: Scan bonuses, discovery rewards
  - Advancement: Exploration achievements
- [ ] `DA_Way_Bounty_Hunters`
  - Purpose: Combat contracts
  - Benefits: Bounty access, combat rewards
  - Advancement: Bounty completion
- [ ] `DA_Way_Miners`
  - Purpose: Resource extraction
  - Benefits: Mining efficiency, ore prices
  - Advancement: Mining volume

**Way Networks** (2 alliances)
- [ ] `DA_WayNetwork_Verse_Trade`
  - Members: Traders Guild, Freelancers
  - Shared reputation pool
  - Cross-guild benefits
- [ ] `DA_WayNetwork_Verse_Frontier`
  - Members: Explorers, Miners, Freelancers
  - Frontier support network
  - Shared discoveries

**Way Features**
- [ ] Configure Feat requirements per Way
  - Feats = achievement milestones
  - Unlock perks and ranks
- [ ] Set up reputation gains
  - Activity-based reputation
  - Cross-guild reputation sharing
- [ ] Define rank advancement
  - Novice → Apprentice → Journeyman → Expert → Master

**SUCCESS MILESTONE**: Can join guilds and gain reputation

---

#### Phase 2: Expanded Ways (Week 3-4) - Priority: MEDIUM

**More Ways** (3 additional)
- [ ] `DA_Way_Engineers`
  - Ship repair and upgrades
- [ ] `DA_Way_Smugglers`
  - Illegal goods transport
- [ ] `DA_Way_Mercenaries`
  - Military contracts

**Way Quests** (15 quests)
- [ ] Create Way-specific quest chains
  - 2-3 quests per Way
  - Rank-gated quests
  - Guild storylines

**Guild Perks** (Configure per Way)
- [ ] Rank 1 perks (Novice)
- [ ] Rank 3 perks (Journeyman)
- [ ] Rank 5 perks (Master)
- [ ] Cross-guild synergies

---

#### Phase 3: Way Dynamics (Week 5-6) - Priority: LOW

**Advanced Features**
- [ ] Rival/antagonist generation
  - Personal nemesis system
  - Rivalry events
  - Nemesis encounters
- [ ] Guild politics
  - Internal faction struggles
  - Leadership changes
  - Policy votes
- [ ] Guild territories
  - Controlled space
  - Jurisdiction rules
  - Turf wars

---

## Economic Simulation

**Status**: ⚠️ 40% Complete (Framework only)

### Phase 1: Basic Simulation (Q2 2026)

**Supply Chain Simulation**
- [ ] Implement C++ production chains
  - Raw materials → Processing → Products
  - Station production/consumption
  - Transport delays
- [ ] Configure production rates
- [ ] Balance consumption rates
- [ ] Test economic stability

**Price Simulation**
- [ ] Implement price volatility
- [ ] Add seasonal variations
- [ ] Include crisis events
- [ ] Test market responses

**AI Economic Activity**
- [ ] NPC trader routes
- [ ] NPC market participation
- [ ] Fleet economic impact
- [ ] Station economic growth

---

## Integration Status

### System Dependencies

```
Trading System
  ├─→ Requires: Stations, Ships, Cargo system
  └─→ Enables: Economy gameplay, Trade routes

Market System
  ├─→ Requires: Trading system, Faction system
  └─→ Enables: Dynamic economy, Price discovery

Faction System
  ├─→ Requires: Reputation system, Diplomacy manager
  └─→ Enables: Political gameplay, Faction conflicts

Way System
  ├─→ Requires: Reputation, Quest system
  └─→ Enables: Guild gameplay, Specializations

Economic Simulation
  ├─→ Requires: All economic systems
  └─→ Enables: Emergent economy, Market dynamics
```

### Critical Path (Q1 2026)

**Week 1-2**: Basic Trading
1. Create 10 trade items
2. Create 3 markets
3. Create trading UI
4. Test buy/sell

**Week 3-4**: Factions & Ways
1. Create 5 factions
2. Create 5 Ways
3. Configure reputations
4. Test faction/guild systems

**Week 5-6**: Content Expansion
1. Create more trade items (10+)
2. Create more markets (7+)
3. Create more factions (5+)
4. Create more Ways (3+)

**Week 7-8**: Integration & Balance
1. Balance prices
2. Balance reputation gains
3. Test economic loops
4. Polish UI

---

## Success Metrics

### Q1 2026 Goals
- [ ] Can buy and sell 20+ commodities
- [ ] Can trade at 10+ different markets
- [ ] Reputation system functional with 10 factions
- [ ] Can join and advance in 5+ Ways
- [ ] Profitable trade routes established

### Q2 2026 Goals
- [ ] Dynamic economy simulation running
- [ ] AI traders affect markets
- [ ] Faction conflicts impact economy
- [ ] Guild perks provide meaningful benefits
- [ ] Economic gameplay loop polished

---

## Related Documentation

- [Main Roadmap](ROADMAP.md)
- [Core Systems Roadmap](ROADMAP_CORE_SYSTEMS.md)
- [Content Creation Roadmap](ROADMAP_CONTENT_CREATION.md)
- [Trading System Guide](Assets/TradingSystemGuide.md)
- [Faction Setup Guide](Assets/FactionSetupGuide.md)
- [Way System Guide](Assets/WaySystemGuide.md)
- [Way Network Guide](Assets/WayNetworkGuide.md)

---

**Last Updated**: December 20, 2025  
**Next Review**: January 2026
