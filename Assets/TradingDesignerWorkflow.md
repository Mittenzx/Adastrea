# Trading System - Designer Workflow Guide

## Overview
This guide provides step-by-step workflows for game designers to create and configure trading content without writing C++ code. All examples use Blueprint and Data Assets.

**Important:** The trading system integrates with the [Way System](WaySystemGuide.md), where specialized guilds (Ways) and their networks manage day-to-day economic activities, supply chains, and trade relationships. This guide covers the technical setup of trade items and markets.

---

## Table of Contents
1. [Creating Trade Items](#creating-trade-items)
2. [Setting Up Markets](#setting-up-markets)
3. [Designing Contracts](#designing-contracts)
4. [Configuring AI Traders](#configuring-ai-traders)
5. [Creating Market Events](#creating-market-events)
6. [Balancing Economy](#balancing-economy)
7. [Testing & Iteration](#testing--iteration)

---

## Creating Trade Items

### Workflow 1: Basic Trade Item (Raw Material)

**Goal**: Create iron ore as a basic tradeable resource

**Steps**:

1. **Open Content Browser**
   - Navigate to `Content/Trading/Items/`
   - Create folder if it doesn't exist

2. **Create Data Asset**
   - Right-click → Miscellaneous → Data Asset
   - Select `TradeItemDataAsset`
   - Name: `DA_TradeItem_IronOre`

3. **Configure Basic Info**
   ```
   Item Name: "Iron Ore"
   Item ID: "TradeItem_IronOre"
   Category: Raw Materials
   Description: "Raw iron ore extracted from asteroids.
                 Essential for metal production."
   ```

4. **Set Pricing**
   ```
   Base Price: 50
   Price Volatility:
     - Volatility Multiplier: 1.2
     - Min Price Deviation: 0.6
     - Max Price Deviation: 1.8
   Affected By Supply Demand: ✓
   Affected By Market Events: ✓
   ```

5. **Configure Legality**
   ```
   Legality Status: Legal
   Min Reputation Required: -100
   ```

6. **Set Physical Properties**
   ```
   Volume Per Unit: 1.0
   Mass Per Unit: 2.5
   Standard Lot Size: 100
   Typical Market Stock: 5000
   Replenishment Rate: 500
   ```

7. **Configure AI Behavior**
   ```
   AI Trade Priority: 6
   AI Hoardable: ☐
   AI Arbitrage Enabled: ✓
   Behavior Tags: BasicResource, Industrial, HighVolume
   ```

8. **Save and Test**
   - Save the Data Asset
   - Add to a test market
   - Check pricing calculations

### Workflow 2: High-Value Tech Item

**Goal**: Create quantum processor as rare, valuable technology

**Steps**:

1. **Create Data Asset**
   - `DA_TradeItem_QuantumProcessor`

2. **Configure Basic Info**
   ```
   Item Name: "Quantum Processor"
   Item ID: "TradeItem_QuantumProcessor"
   Category: Technology
   Description: "Advanced quantum computing processor."
   ```

3. **Set Premium Pricing**
   ```
   Base Price: 5000
   Price Volatility:
     - Volatility Multiplier: 2.5
     - Min Price Deviation: 0.4
     - Max Price Deviation: 3.0
   ```

4. **Add Restrictions**
   ```
   Legality Status: Regulated
   Banned Factions:
     - DA_Faction_PirateAlliance
   Requires Permit Factions:
     - DA_Faction_SolarisUnion
   Min Reputation Required: 25
   ```

5. **Set Low Volume Properties**
   ```
   Volume Per Unit: 0.1
   Mass Per Unit: 0.5
   Standard Lot Size: 10
   Typical Market Stock: 50
   Replenishment Rate: 5
   ```

6. **High-Priority AI Behavior**
   ```
   AI Trade Priority: 9
   AI Hoardable: ✓
   AI Arbitrage Enabled: ✓
   Behavior Tags: HighValue, Technology, Rare, Regulated
   ```

### Workflow 3: Contraband Item

**Goal**: Create illegal drugs for black market trading

**Steps**:

1. **Create Data Asset**
   - `DA_TradeItem_NeuralStims`

2. **Configure as Contraband**
   ```
   Item Name: "Neural Stims"
   Item ID: "TradeItem_NeuralStims"
   Category: Contraband
   Legality Status: Illegal
   ```

3. **Extreme Pricing**
   ```
   Base Price: 1500
   Price Volatility:
     - Volatility Multiplier: 3.0
     - Min Price Deviation: 0.3
     - Max Price Deviation: 5.0
   ```

4. **Set Bans and Fines**
   ```
   Banned Factions:
     - DA_Faction_SolarisUnion
     - DA_Faction_TerranFederation
   Contraband Fine Multiplier: 10.0
   ```

5. **Black Market Properties**
   ```
   Volume Per Unit: 0.05
   Mass Per Unit: 0.1
   Typical Market Stock: 20
   Replenishment Rate: 2
   ```

6. **High-Risk AI Behavior**
   ```
   AI Trade Priority: 10
   AI Hoardable: ✓
   Behavior Tags: HighValue, Contraband, HighRisk, BlackMarket
   ```

---

## Setting Up Markets

### Workflow 4: Open Trading Hub

**Goal**: Create a general-purpose trading station

**Steps**:

1. **Create Market Data Asset**
   - Navigate to `Content/Trading/Markets/`
   - Create `MarketDataAsset`
   - Name: `DA_Market_FreePort`

2. **Configure Basic Info**
   ```
   Market Name: "Free Port Station"
   Market ID: "Market_FreePort"
   Market Type: Open Market
   Market Size: Large
   Controlling Faction: DA_Faction_FreeTraders
   ```

3. **Set Pricing Parameters**
   ```
   Transaction Tax Rate: 0.05 (5%)
   Sell Price Markup: 1.15 (players pay 15% more)
   Buy Price Markdown: 0.85 (players sell for 15% less)
   ```

4. **Configure Access**
   ```
   Allow Player Buying: ✓
   Allow Player Selling: ✓
   Allow AI Traders: ✓
   Min Reputation Required: -50
   ```

5. **Add Inventory**
   - Click "+" on Inventory array
   - For each item:
     ```
     Trade Item: DA_TradeItem_[ItemName]
     Current Stock: [Initial quantity]
     Max Stock: [Maximum quantity]
     Supply Level: 1.0
     Demand Level: 1.0
     In Stock: ✓
     ```

6. **Stock Management**
   ```
   Stock Refresh Rate: 24.0 (hours)
   ```

7. **Configure AI Traders**
   ```
   AI Trader Count: 15
   AI Trade Frequency: 20
   Allow AI Price Manipulation: ✓
   ```

8. **Add Market Events** (optional)
   - Configure in Event System (see Workflow 8)

### Workflow 5: Black Market

**Goal**: Create hidden market for contraband

**Steps**:

1. **Create Market Data Asset**
   - `DA_Market_ShadowHaven`

2. **Configure as Black Market**
   ```
   Market Name: "Shadow Haven"
   Market ID: "Market_ShadowHaven"
   Market Type: Black Market
   Market Size: Small
   Controlling Faction: DA_Faction_PirateAlliance
   ```

3. **High Fees**
   ```
   Transaction Tax Rate: 0.20 (20%)
   Sell Price Markup: 1.5 (50% markup)
   Buy Price Markdown: 0.6 (40% markdown)
   ```

4. **Restricted Access**
   ```
   Min Reputation Required: -25 (pirates only)
   Random Event Chance: 0.25 (25% - higher risk)
   ```

5. **Contraband Inventory**
   - Focus on illegal items
   - High prices, low stock
   - Fast refresh for hot goods

6. **Aggressive AI**
   ```
   AI Trader Count: 5
   AI Trade Frequency: 15
   Allow AI Price Manipulation: ✓
   ```

### Workflow 6: Faction-Exclusive Market

**Goal**: Create market requiring faction membership

**Steps**:

1. **Create Market Data Asset**
   - `DA_Market_MilitaryDepot`

2. **Configure Restrictions**
   ```
   Market Type: Faction Exclusive
   Controlling Faction: DA_Faction_SolarisUnion
   Min Reputation Required: 50 (faction member)
   ```

3. **Specialized Inventory**
   - Military equipment only
   - High-tech items
   - Faction-specific goods

4. **Better Prices for Members**
   ```
   Transaction Tax Rate: 0.02 (2% for members)
   Sell Price Markup: 1.05 (small markup)
   Buy Price Markdown: 0.95 (fair buyback)
   ```

---

## Designing Contracts

### Workflow 7: Basic Delivery Contract

**Goal**: Create simple cargo delivery mission

**Steps**:

1. **Create Contract Data Asset**
   - Navigate to `Content/Trading/Contracts/`
   - Create `TradeContractDataAsset`
   - Name: `DA_Contract_SupplyDelivery_001`

2. **Configure Basic Info**
   ```
   Contract Name: "Routine Supply Delivery"
   Contract ID: "Contract_SupplyDelivery_001"
   Contract Type: Delivery
   Difficulty: Easy
   Contract Giver: DA_Faction_FreeTraders
   ```

3. **Set Cargo Requirements**
   - Click "+" on Required Cargo array
   - Add entries:
     ```
     Entry 1:
       Trade Item: DA_TradeItem_FoodRations
       Quantity: 500
       Requires Pristine Condition: ☐
     
     Entry 2:
       Trade Item: DA_TradeItem_MedicalSupplies
       Quantity: 100
       Requires Pristine Condition: ✓
     ```

4. **Configure Locations**
   ```
   Origin Location:
     Location Name: "Free Port Station"
     Coordinates: X=1000, Y=2000, Z=500
     Market ID: "Market_FreePort"
   
   Destination Location:
     Location Name: "Mining Colony Alpha"
     Coordinates: X=5000, Y=3000, Z=1000
     Market ID: "Market_MiningColony"
   ```

5. **Calculate Distance**
   ```
   Distance: 5500.0
   ```
   *(Use: sqrt((x2-x1)² + (y2-y1)² + (z2-z1)²))*

6. **Set Time Limit**
   ```
   Time Limit: 48.0 (hours, 0 = no limit)
   Min Reputation Required: 0
   ```

7. **Configure Rewards**
   ```
   Rewards:
     Credits: 5000
     Reputation Gain: 10
     Experience Points: 100
   ```

8. **Set Penalties**
   ```
   Penalties:
     Credit Penalty: 1000
     Reputation Penalty: 5
     Collateral Deposit: 500
   ```

9. **Special Conditions**
   ```
   Repeatable: ✓
   Requires Stealth: ☐
   Combat Expected: ☐
   Hostile Factions: (none)
   ```

10. **Add Tags**
    ```
    Contract Tags: BasicDelivery, LowRisk, Repeatable
    ```

### Workflow 8: Smuggling Contract

**Goal**: Create high-risk contraband delivery

**Steps**:

1. **Create Contract Data Asset**
   - `DA_Contract_Smuggling_NeuralStims`

2. **Configure as Smuggling**
   ```
   Contract Name: "Shadow Run: Neural Stim Delivery"
   Contract Type: Smuggling
   Difficulty: Very Hard
   Contract Giver: DA_Faction_PirateAlliance
   ```

3. **Contraband Cargo**
   ```
   Required Cargo:
     Trade Item: DA_TradeItem_NeuralStims
     Quantity: 50
     Requires Pristine Condition: ✓
   ```

4. **Dangerous Route**
   ```
   Origin: Shadow Haven (pirate space)
   Destination: Corporate Megalopolis (corporate space)
   Distance: 18500.0
   Time Limit: 72.0
   ```

5. **High Rewards**
   ```
   Rewards:
     Credits: 75000
     Reputation Gain: 25
     Experience Points: 500
     Bonus Items: DA_TradeItem_SmugglerBadge
   ```

6. **Severe Penalties**
   ```
   Penalties:
     Credit Penalty: 50000
     Reputation Penalty: 50
     Collateral Deposit: 25000
   ```

7. **Special Conditions**
   ```
   Repeatable: ☐ (one-time)
   Requires Stealth: ✓
   Combat Expected: ✓
   Hostile Factions:
     - DA_Faction_SolarisUnion
     - DA_Faction_AuthorityPatrol
   Contract Tags: Smuggling, HighRisk, HighReward, Contraband
   ```

---

## Configuring AI Traders

### Workflow 9: Conservative Trader

**Goal**: Create safe, steady profit-seeking AI

**Steps**:

1. **Add AI Trader Component to Actor**
   - Select station or ship actor
   - Add Component → AI Trader Component

2. **Configure Strategy**
   ```
   Trader Faction: DA_Faction_FreeTraders
   Strategy: Conservative
   Trading Capital: 25000
   Cargo Capacity: 500.0
   Travel Speed: 100.0
   ```

3. **Set Skills and Risk**
   ```
   Trading Skill: 5 (average)
   Risk Tolerance: 0.2 (very risk-averse)
   Min Profit Margin: 0.20 (20% minimum)
   ```

4. **Enable Safe Behaviors**
   ```
   Enabled Behaviors:
     ✓ Market Making
     ✓ Route Planning
     ☐ Speculative Buying
     ☐ Market Manipulation
   ```

5. **Permissions**
   ```
   Can Manipulate Prices: ☐
   Operates In Black Markets: ☐
   Accepts Contracts: ✓
   Trade Frequency: 10 (per day)
   ```

6. **Initialize in Blueprint**
   - Create BeginPlay event
   - Call `Initialize` with starting capital and market

### Workflow 10: Aggressive Speculator

**Goal**: Create high-risk, high-reward AI trader

**Steps**:

1. **Add AI Trader Component**

2. **Configure Aggressive Strategy**
   ```
   Strategy: Aggressive
   Trading Capital: 100000
   Cargo Capacity: 1500.0
   Trading Skill: 8 (expert)
   Risk Tolerance: 0.8 (risk-seeking)
   Min Profit Margin: 0.30 (30% minimum)
   ```

3. **Enable All Behaviors**
   ```
   Enabled Behaviors:
     ✓ Market Making
     ✓ Arbitrage
     ✓ Route Planning
     ✓ Speculative Buying
     ✓ Event Reaction
     ✓ Hoarding
     ✓ Market Manipulation
   ```

4. **High-Risk Permissions**
   ```
   Can Manipulate Prices: ✓
   Operates In Black Markets: ✓
   Accepts Contracts: ✓
   Trade Frequency: 25
   ```

5. **Set Relationships** (optional)
   - Add rival traders (competes against)
   - Add friendly traders (cooperates with)

---

## Creating Market Events

### Workflow 11: Supply Shortage Event

**Goal**: Create temporary shortage of raw materials

**Steps**:

1. **Open Market Data Asset**
   - Select market that will experience event

2. **Add to Active Events Array**
   - Click "+" on Active Events

3. **Configure Event**
   ```
   Event Name: "Iron Ore Shortage"
   Event ID: "Event_Shortage_IronOre"
   Event Description: "Mining operations disrupted by
                       asteroid storm. Iron ore scarce."
   ```

4. **Set Affected Items**
   ```
   Affected Item IDs:
     - TradeItem_IronOre
     - TradeItem_SteelIngot (processed from iron)
   ```

5. **Configure Impact**
   ```
   Price Multiplier: 1.5 (50% price increase)
   Supply Multiplier: 0.5 (supply halved)
   Demand Multiplier: 1.5 (increased demand)
   Duration Hours: 48.0
   ```

6. **Trigger Event in Blueprint**
   ```
   Event Graph:
     - When condition met (quest, timer, random)
     - Set bIsActive = true
     - Set StartTime = Current Game Time
     - Call OnMarketEventStarted
   ```

### Workflow 12: Technology Expo Event

**Goal**: Create demand spike for tech items

**Steps**:

1. **Configure Event**
   ```
   Event Name: "Galactic Tech Expo"
   Event ID: "Event_TechExpo"
   ```

2. **Affect Tech Items**
   ```
   Affected Item IDs:
     - TradeItem_QuantumProcessor
     - TradeItem_DataCore
     - TradeItem_AIChip
   ```

3. **Boost Demand**
   ```
   Price Multiplier: 1.3
   Supply Multiplier: 1.2 (vendors stock up)
   Demand Multiplier: 1.8 (high interest)
   Duration Hours: 72.0 (3-day event)
   ```

---

## Balancing Economy

### Balancing Checklist

**Price Ranges**:
- [ ] Basic materials: 10-100 credits
- [ ] Refined goods: 100-500 credits
- [ ] Components: 500-2000 credits
- [ ] Technology: 2000-10000 credits
- [ ] Luxury: 5000-50000 credits
- [ ] Contraband: 1000-20000 credits

**Profit Margins**:
- [ ] Low-risk trades: 10-20% profit
- [ ] Medium-risk: 20-40% profit
- [ ] High-risk: 40-100% profit
- [ ] Contraband: 100-300% profit

**Contract Rewards**:
- [ ] Easy: 1000-5000 credits
- [ ] Medium: 5000-20000 credits
- [ ] Hard: 20000-50000 credits
- [ ] Very Hard: 50000-100000 credits
- [ ] Extreme: 100000+ credits

**Stock Levels**:
- [ ] Common items: 1000-10000 units
- [ ] Uncommon: 100-1000 units
- [ ] Rare: 10-100 units
- [ ] Very rare: 1-10 units

### Testing Scenarios

**Test 1: Basic Trading Loop**
1. Buy iron ore at mining station (cheap)
2. Transport to industrial hub
3. Sell for profit
4. Verify profit is 15-25%

**Test 2: Arbitrage**
1. Identify price difference between markets
2. Calculate profit after transport costs
3. Execute trade
4. Verify AI traders also exploit opportunity

**Test 3: Market Event**
1. Trigger shortage event
2. Verify prices increase
3. Verify stock depletes
4. Confirm event expires after duration

**Test 4: Contraband**
1. Buy contraband at black market
2. Attempt to sell at legal market
3. Verify rejection or fine
4. Sell at another black market for profit

**Test 5: AI Competition**
1. Spawn aggressive AI trader
2. Monitor same markets as AI
3. Verify AI competes for opportunities
4. Check if prices are manipulated

---

## Testing & Iteration

### Debug Tools

**Enable Debug Mode**:
```
In Blueprint:
  - Add debug display for prices
  - Show AI trader decisions
  - Log all transactions
  - Display market events
```

**Test Commands** (if implemented):
```
trading.debug 1
trading.showroutes 1
trading.logtransactions 1
```

### Analytics to Monitor

1. **Price Stability**
   - Track price over time
   - Identify excessive volatility
   - Balance volatility multipliers

2. **Player Profit**
   - Track total player earnings
   - Ensure positive but not excessive
   - Balance against other income sources

3. **AI Performance**
   - Monitor AI trader profits
   - Verify AI makes logical decisions
   - Balance AI competitiveness

4. **Market Activity**
   - Transaction volume per item
   - Most/least traded items
   - Market event frequency

5. **Contract Completion**
   - Success rate by difficulty
   - Average profit per contract
   - Time to completion

### Iteration Workflow

1. **Gather Data**
   - Playtest sessions
   - Transaction logs
   - Player feedback

2. **Identify Issues**
   - Overpowered strategies
   - Underused systems
   - Balance problems

3. **Make Changes**
   - Adjust prices
   - Modify event frequency
   - Rebalance rewards

4. **Test Changes**
   - Focused playtests
   - Verify fixes
   - Check for new issues

5. **Document Changes**
   - Update balance spreadsheet
   - Note design decisions
   - Track iterations

---

## Common Patterns

### Pattern 1: Tiered Item Progression

Create item chains with increasing value:
```
Iron Ore (50) → Steel Ingot (150) → Ship Hull Plate (500) → 
Complete Hull (2000)
```

Each step adds value through processing.

### Pattern 2: Regional Specialization

Different markets specialize:
- Mining stations: Cheap raw materials
- Industrial hubs: Refined goods
- Tech centers: Advanced components
- Luxury stations: High-end items

Creates natural trade routes.

### Pattern 3: Event Chains

Events trigger follow-up events:
```
Mining Accident → Ore Shortage → Metal Price Spike → 
Component Shortage → Ship Price Increase
```

Creates economic ripple effects.

### Pattern 4: Seasonal Economy

Rotating events create seasons:
- Spring: Agriculture boom (food cheap)
- Summer: Tech expo (tech demand)
- Fall: Military exercises (weapons demand)
- Winter: Fuel shortage (energy prices up)

### Pattern 5: Quest-Driven Economy

Quests affect economy:
- War quest → Military equipment demand
- Plague quest → Medical supplies shortage
- Discovery quest → Research materials wanted

---

## Appendix: Quick Reference Tables

### Trade Item Categories

| Category | Base Price Range | Volume | Legality | AI Priority |
|----------|-----------------|--------|----------|-------------|
| Raw Materials | 10-100 | High | Legal | 5-6 |
| Refined Goods | 100-500 | Medium | Legal | 6-7 |
| Components | 500-2000 | Medium | Legal | 7-8 |
| Technology | 2000-10000 | Low | Regulated | 8-9 |
| Food | 10-200 | High | Legal | 5-6 |
| Luxury | 5000-50000 | Very Low | Legal | 7-9 |
| Contraband | 1000-20000 | Low | Illegal | 9-10 |
| Military | 2000-20000 | Low | Restricted | 8-9 |
| Medical | 100-5000 | Medium | Legal | 6-8 |

### Market Type Comparison

| Type | Tax | Markup | Access | Items |
|------|-----|--------|--------|-------|
| Open Market | 5% | 15% | Easy | All Legal |
| Black Market | 20% | 50% | Hard | All Including Illegal |
| Faction Exclusive | 2% | 5% | Faction Only | Specialized |
| Commodity Exchange | 1% | 5% | Easy | Raw Materials |
| Luxury Bazaar | 10% | 30% | Reputation | Luxury |
| Military Supply | 5% | 10% | Restricted | Military |

### AI Trader Strategies

| Strategy | Risk | Behaviors | Capital | Profit Target |
|----------|------|-----------|---------|---------------|
| Conservative | Low | Market Making, Route Planning | 25K | 10-20% |
| Balanced | Medium | All Basic | 50K | 20-30% |
| Aggressive | High | All Including Manipulation | 100K | 30-50% |
| Opportunistic | Variable | Event-Driven | 75K | Variable |
| Specialist | Medium | Category-Focused | 60K | 25-40% |
| Manipulator | High | Price Manipulation | 150K | 40-100% |

---

**Next Steps**: 
- Review [TradingSystemGuide.md](TradingSystemGuide.md) for complete reference
- Check [TradingBlueprintExamples.md](TradingBlueprintExamples.md) for Blueprint examples
- See [TradingTemplates/](TradingTemplates/) for YAML configuration templates
