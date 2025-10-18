# Trading System - Complete Designer & Programmer Guide

## Overview
The AdAstrea Trading System is a comprehensive, Blueprint-friendly economic simulation framework that provides:
- **Dynamic pricing** based on supply/demand, rarity, and market events
- **AI trading agents** with arbitrage, speculation, and market manipulation
- **Trade contracts** for missions and delivery quests
- **Transaction tracking** for analytics and player history
- **Market events** that affect prices and availability
- **Faction relationships** that influence trade opportunities

All systems are built with **Blueprint extensibility** in mind, using `BlueprintNativeEvent` hooks for designer customization.

---

## Table of Contents
1. [Quick Start](#quick-start)
2. [Core Concepts](#core-concepts)
3. [Trade Items](#trade-items)
4. [Markets](#markets)
5. [Trade Contracts](#trade-contracts)
6. [AI Traders](#ai-traders)
7. [Transaction System](#transaction-system)
8. [Dynamic Pricing](#dynamic-pricing)
9. [Blueprint Integration](#blueprint-integration)
10. [Advanced Features](#advanced-features)
11. [Multiplayer Considerations](#multiplayer-considerations)
12. [Performance Optimization](#performance-optimization)

---

## Quick Start

### For Designers

**Creating Your First Trade Item:**
1. In Content Browser, navigate to `Content/Trading/Items/`
2. Right-click → Miscellaneous → Data Asset
3. Select `TradeItemDataAsset`
4. Name it `DA_TradeItem_[YourItemName]`
5. Configure properties using YAML templates as reference
6. See `Assets/TradingTemplates/TradeItem_*.yaml` for examples

**Setting Up a Market:**
1. Create a new `MarketDataAsset`
2. Configure market type, size, and controlling faction
3. Add inventory entries with trade items
4. Set transaction taxes and price modifiers
5. Configure AI trader count and behavior
6. See `Assets/TradingTemplates/Market_*.yaml` for examples

**Creating Trade Contracts:**
1. Create a new `TradeContractDataAsset`
2. Define cargo requirements and destinations
3. Set rewards and penalties
4. Configure difficulty and special conditions
5. See `Assets/TradingTemplates/Contract_*.yaml` for examples

### For Programmers

**Extending the System in C++:**
```cpp
// Custom trade item with special pricing logic
UCLASS()
class UMyCustomTradeItem : public UTradeItemDataAsset
{
    GENERATED_BODY()
    
protected:
    virtual float OnCalculateCustomPrice_Implementation(
        float Supply, float Demand, 
        float EventMultiplier, float BaseCalculatedPrice) const override
    {
        // Add custom pricing logic here
        float CustomPrice = BaseCalculatedPrice;
        // ... your modifications
        return CustomPrice;
    }
};
```

**Extending in Blueprint:**
1. Create Blueprint class derived from `TradeItemDataAsset`
2. Override `OnCalculateCustomPrice` event
3. Implement custom pricing logic
4. Create Data Asset from your Blueprint class

---

## Core Concepts

### Trade Items
**TradeItemDataAsset** represents any tradeable commodity:
- **Base pricing** with volatility settings
- **Legality status** and faction restrictions
- **Physical properties** (volume, mass)
- **AI behavior tags** for trader decision-making

### Markets
**MarketDataAsset** represents trading locations:
- **Inventory management** with stock levels
- **Dynamic pricing** based on supply/demand
- **Market events** that affect prices
- **AI trader operations**
- **Faction control** and access restrictions

### Trade Contracts
**TradeContractDataAsset** represents missions:
- **Cargo requirements**
- **Delivery destinations**
- **Rewards and penalties**
- **Special conditions** (stealth, combat, etc.)
- **Time limits** and reputation requirements

### AI Traders
**AITraderComponent** provides automated trading:
- **Multiple strategies** (conservative, aggressive, opportunistic)
- **Arbitrage detection** across markets
- **Route planning** for profit optimization
- **Market manipulation** capabilities
- **Event reaction** and player competition

---

## Trade Items

### Properties Reference

#### Basic Info
```
ItemName: Display name shown to players
ItemID: Unique identifier (use naming convention: TradeItem_[Name])
Category: Item category (affects AI behavior and filtering)
Description: Multi-line description of the item
```

#### Pricing
```
BasePrice: Base price in credits
PriceVolatility:
  VolatilityMultiplier: How much prices can fluctuate (1.0 = normal)
  MinPriceDeviation: Minimum price as % of base (0.5 = 50% below)
  MaxPriceDeviation: Maximum price as % of base (2.0 = 200% above)
bAffectedBySupplyDemand: Whether supply/demand affects price
bAffectedByMarketEvents: Whether market events affect price
```

#### Legality
```
LegalityStatus: Legal, Regulated, Restricted, Illegal, Contraband
TradeRestrictions:
  BannedFactions: Factions that prohibit this item
  RequiresPermitFactions: Factions requiring permits
  MinReputationRequired: Minimum reputation to trade
ContrabandFineMultiplier: Fine multiplier if caught (for illegal items)
```

#### Physical Properties
```
VolumePerUnit: Cubic meters per unit
MassPerUnit: Kilograms per unit
StandardLotSize: Typical trading lot size
TypicalMarketStock: Expected market inventory
ReplenishmentRate: Units restocked per day
```

#### AI Behavior
```
AITradePriority: Priority for AI traders (1-10, higher = more desirable)
bAIHoardable: Whether AI will hoard during shortages
bAIArbitrageEnabled: Whether AI can use for arbitrage
BehaviorTags: Tags for AI decision-making
```

### Category Guide

**RawMaterials**: Basic resources (ore, crystals, gases)
- Low price volatility
- High volume trading
- Essential for industry

**RefinedGoods**: Processed materials (ingots, alloys, compounds)
- Medium price volatility
- Medium volume trading
- Required for manufacturing

**Components**: Ship parts and equipment
- Medium-high price volatility
- Low-medium volume
- Used in construction and repairs

**Technology**: Advanced tech items (processors, data cores)
- High price volatility
- Low volume, high value
- Often regulated

**Food**: Consumables and supplies
- Low-medium volatility
- High volume, essential
- Time-sensitive

**Luxury**: High-value non-essential goods
- High volatility
- Low volume, very high value
- Reputation-gated

**Contraband**: Illegal items
- Extreme volatility
- Very high value and risk
- Black market only

**Military**: Weapons and defense systems
- High volatility
- Heavily regulated
- Faction-restricted

**Medical**: Healthcare supplies
- Medium volatility
- Essential during crises
- Emergency demand spikes

**Data**: Information and research
- Variable pricing
- Often unique/one-time trades
- Knowledge-based economy

### Pricing Formula

The default pricing formula is:
```
CalculatedPrice = BasePrice * SupplyFactor * DemandFactor * EventMultiplier * VolatilityMultiplier

Where:
  SupplyFactor = 1.0 / max(Supply, 0.1)
  DemandFactor = Demand
  
Final price is clamped to:
  [BasePrice * MinPriceDeviation, BasePrice * MaxPriceDeviation]
```

You can override this in Blueprint using `OnCalculateCustomPrice`.

### Example Trade Items

**Basic Resource (Iron Ore)**:
```yaml
BasePrice: 50.0
Category: RawMaterials
LegalityStatus: Legal
AITradePriority: 6
TypicalMarketStock: 5000
ReplenishmentRate: 500
```

**High-Value Tech (Quantum Processor)**:
```yaml
BasePrice: 5000.0
Category: Technology
LegalityStatus: Regulated
AITradePriority: 9
TypicalMarketStock: 50
ReplenishmentRate: 5
bAIHoardable: true
```

**Contraband (Neural Stims)**:
```yaml
BasePrice: 1500.0
Category: Contraband
LegalityStatus: Illegal
AITradePriority: 10
ContrabandFineMultiplier: 10.0
bAIHoardable: true
```

---

## Markets

### Market Types

**OpenMarket**: General trading hub
- All items available
- Fair prices and taxes
- Open to all factions (with reputation checks)

**BlackMarket**: Illicit trading
- Contraband and stolen goods
- High prices and taxes
- Restricted access, high risk

**FactionExclusive**: Faction-only market
- Requires faction membership or high reputation
- Specialized goods
- Better prices for faction members

**CommodityExchange**: High-volume trading
- Focus on raw materials and bulk goods
- Lower margins, high volume
- Professional traders only

**LuxuryBazaar**: High-end goods
- Luxury items and rare finds
- Very high prices
- Wealthy clientele

**MilitarySupply**: Military equipment
- Weapons and defense systems
- Faction-restricted
- High security

**ResearchHub**: Scientific goods
- Research materials and data
- Academic focus
- Technology emphasis

**IndustrialDepot**: Manufacturing supplies
- Components and refined goods
- Industrial clients
- Bulk trading

### Market Configuration

```yaml
MarketName: "Display Name"
MarketID: "Market_UniqueID"
MarketType: OpenMarket
MarketSize: Large  # Small, Medium, Large, Megacity, Capital

# Pricing
TransactionTaxRate: 0.05  # 5% tax
SellPriceMarkup: 1.15     # Players pay 15% more
BuyPriceMarkdown: 0.85    # Players sell for 15% less

# Access
bAllowPlayerBuying: true
bAllowPlayerSelling: true
bAllowAITraders: true
MinReputationRequired: -50

# Stock Management
StockRefreshRate: 24.0  # Hours between refresh
```

### Inventory Entries

Each market has inventory entries defining available items:
```yaml
InventoryEntry:
  TradeItem: "DA_TradeItem_IronOre"
  CurrentStock: 5000
  MaxStock: 10000
  SupplyLevel: 1.0   # 1.0 = normal supply
  DemandLevel: 1.0   # 1.0 = normal demand
```

**Supply/Demand Levels**:
- `< 0.5`: Very low
- `0.5 - 0.8`: Low
- `0.8 - 1.2`: Normal
- `1.2 - 2.0`: High
- `> 2.0`: Very high

### Market Events

Market events dynamically affect prices:
```yaml
MarketEvent:
  EventName: "Supply Shortage"
  EventID: "Event_Shortage_IronOre"
  AffectedItemIDs:
    - "TradeItem_IronOre"
  PriceMultiplier: 1.5      # Prices increase 50%
  SupplyMultiplier: 0.5     # Supply drops to 50%
  DemandMultiplier: 1.5     # Demand increases 50%
  DurationHours: 48.0       # Lasts 48 hours
```

**Common Event Types**:
- **Supply Shortage**: Low supply, high prices
- **Surplus**: High supply, low prices
- **Technology Expo**: Increased tech demand
- **War Mobilization**: Military equipment demand spike
- **Plague**: Medical supplies shortage
- **Trade Embargo**: Item restrictions
- **Black Friday Sale**: Temporary price drops
- **Authority Raid**: Black market disruption

### AI Trader Configuration

```yaml
AITraderCount: 15          # Number of AI traders
AITradeFrequency: 20       # Trades per day
bAllowAIPriceManipulation: true
```

AI traders will:
- Buy low, sell high
- Execute arbitrage between markets
- React to market events
- Compete with players
- Manipulate prices (if enabled)

---

## Trade Contracts

### Contract Types

**Delivery**: Simple cargo transport
- Low risk, standard rewards
- Most common contract type

**Procurement**: Acquire and deliver specific goods
- Must purchase cargo
- Variable difficulty based on item rarity

**Trade**: Buy low, sell high mission
- Profit-based rewards
- Teaches trading mechanics

**Escort**: Protect cargo shipment
- Combat expected
- Higher rewards

**Smuggling**: Illegal cargo transport
- High risk, high reward
- Stealth required
- Contraband items

**RareGoods**: Transport unique items
- One-time deliveries
- Very high value
- Often time-sensitive

**Emergency**: Urgent supply delivery
- Short time limits
- Critical supplies
- Reputation impact

**Bulk**: Large-volume transport
- Requires significant cargo space
- Long-term contracts
- Steady income

### Contract Difficulty

**Trivial**: Very easy, minimal reward
**Easy**: Beginner-friendly
**Medium**: Standard difficulty
**Hard**: Experienced players
**VeryHard**: Expert traders
**Extreme**: Near-impossible challenges

Difficulty affects:
- Rewards
- Time limits
- Cargo requirements
- Hostile faction presence
- Failure penalties

### Contract Structure

```yaml
ContractName: "Display Name"
ContractID: "Contract_UniqueID"
ContractType: Delivery
Difficulty: Medium

RequiredCargo:
  - TradeItem: "DA_TradeItem_FoodRations"
    Quantity: 500
    bRequiresPristineCondition: false

OriginLocation:
  LocationName: "Source Station"
  Coordinates: [1000, 2000, 500]
  MarketID: "Market_Source"

DestinationLocation:
  LocationName: "Destination Station"
  Coordinates: [5000, 3000, 1000]
  MarketID: "Market_Dest"

Distance: 5500.0
TimeLimit: 48.0  # 0 = no limit

Rewards:
  Credits: 5000
  ReputationGain: 10
  ExperiencePoints: 100

Penalties:
  CreditPenalty: 1000
  ReputationPenalty: 5
  CollateralDeposit: 500
```

### Special Conditions

```yaml
bRepeatable: true              # Can be taken multiple times
bRequiresStealth: false        # Must avoid detection
bCombatExpected: false         # Likely to encounter hostiles
HostileFactions:               # Factions that will attack
  - "DA_Faction_Pirates"
ContractTags:                  # Tags for filtering
  - "BasicDelivery"
  - "LowRisk"
```

---

## AI Traders

### AI Trader Strategies

**Conservative**:
- Low-risk trades only
- Focuses on steady profits
- Avoids speculation
- Prefers established routes

**Balanced**:
- Mix of safe and risky trades
- Moderate profit goals
- Some speculation
- Default strategy

**Aggressive**:
- High-risk, high-reward
- Active market manipulation
- Speculative buying
- Competes directly with player

**Opportunistic**:
- Reacts to market events
- Exploits player actions
- Adapts to changing conditions
- Event-driven trading

**Specialist**:
- Focuses on specific item categories
- Deep knowledge of niche markets
- Builds monopolies
- Category expert

**Manipulator**:
- Actively manipulates prices
- Creates artificial scarcity
- Market maker behavior
- Requires high capital

### AI Behaviors

AI traders can execute multiple behaviors:

**MarketMaking**: 
- Provide liquidity
- Buy low, sell high
- Stabilize prices

**Arbitrage**:
- Exploit price differences
- Buy cheap, transport, sell expensive
- Route optimization

**RoutePlanning**:
- Calculate most profitable routes
- Optimize cargo space
- Minimize travel time

**SpeculativeBuying**:
- Buy before price increases
- Hoard during events
- Anticipate demand

**EventReaction**:
- React to market events
- Buy during crashes
- Sell during booms

**Hoarding**:
- Create artificial scarcity
- Buy up stock
- Raise prices

**Dumping**:
- Sell large quantities
- Lower prices
- Drive out competition

**ContractTrading**:
- Accept and complete contracts
- Mission-based income
- Reputation building

### AI Configuration

```cpp
// In C++ or Blueprint
AITraderComponent->Strategy = EAITraderStrategy::Aggressive;
AITraderComponent->TradingCapital = 50000;
AITraderComponent->CargoCapacity = 1000.0f;
AITraderComponent->TradingSkill = 8;  // 1-10
AITraderComponent->RiskTolerance = 0.7f;  // 0.0-1.0
AITraderComponent->MinProfitMargin = 0.15f;  // 15%

// Enable behaviors
AITraderComponent->EnabledBehaviors.Add(EAITradeBehavior::Arbitrage);
AITraderComponent->EnabledBehaviors.Add(EAITradeBehavior::MarketMaking);
AITraderComponent->EnabledBehaviors.Add(EAITradeBehavior::EventReaction);

// Permissions
AITraderComponent->bCanManipulatePrices = true;
AITraderComponent->bOperatesInBlackMarkets = false;
AITraderComponent->bAcceptsContracts = true;
```

### AI Relationships

AI traders can have relationships:

**Friendly Traders**:
- Share market information
- Cooperative trading
- Joint ventures
- Price collusion (in black markets)

**Rival Traders**:
- Compete for resources
- Price wars
- Route blocking
- Market manipulation against each other

---

## Transaction System

### Transaction Recording

Every trade is recorded as a `FTradeTransaction`:
```cpp
FTradeTransaction Transaction;
Transaction.TransactionType = ETransactionType::Buy;
Transaction.TradeItem = TradeItem;
Transaction.Quantity = 100;
Transaction.PricePerUnit = 50.0f;
Transaction.TotalValue = 5000;
Transaction.BuyerID = PlayerID;
Transaction.SellerID = MarketID;
Transaction.Market = CurrentMarket;
Transaction.Timestamp = GameTime;

TransactionManager->RecordTransaction(Transaction);
```

### Transaction Analytics

Query transaction history:
```cpp
// Get all trades of an item
TArray<FTradeTransaction> ItemTrades = 
    TransactionManager->GetTransactionsByItem(ItemID);

// Get price history
float AvgPrice = TransactionManager->GetAveragePrice(
    ItemID, StartTime, EndTime);

// Get price trend
float Trend = TransactionManager->GetPriceTrend(ItemID, 24.0f);
// Positive = price increasing, Negative = decreasing

// Get player profit/loss
int32 Profit = TransactionManager->GetPlayerProfitLoss(PlayerID);

// Get most traded items
TArray<FName> TopItems = 
    TransactionManager->GetMostTradedItems(10, StartTime, EndTime);
```

### Transaction Flags

Transactions can be flagged:
- `bFlaggedAsSuspicious`: Unusual trading patterns
- `bInvolvedContraband`: Illegal goods traded
- `bCaughtWithContraband`: Player caught by authorities

Use for:
- Law enforcement systems
- Reputation tracking
- Story triggers
- Economic analytics

---

## Dynamic Pricing

### Price Calculation Flow

1. **Base Price**: From `TradeItemDataAsset.BasePrice`
2. **Supply/Demand**: Market inventory affects price
3. **Market Events**: Active events multiply price
4. **Volatility**: Clamped to min/max deviations
5. **Custom Logic**: `OnCalculateCustomPrice` override
6. **Market Markup**: Buy/sell price adjustments
7. **Transaction Tax**: Final price includes tax

### Supply/Demand Impact

```cpp
// Low supply increases price
SupplyFactor = 1.0 / max(Supply, 0.1)

// High demand increases price
DemandFactor = Demand

Price = BasePrice * SupplyFactor * DemandFactor
```

Example:
- Supply = 0.5 (low), Demand = 1.5 (high)
- SupplyFactor = 1.0 / 0.5 = 2.0
- DemandFactor = 1.5
- Price = 100 * 2.0 * 1.5 = 300 (3x base price)

### Market Event Impact

Events multiply prices:
```yaml
Event:
  PriceMultiplier: 1.5  # +50% price
  SupplyMultiplier: 0.5  # Supply halved
  DemandMultiplier: 2.0  # Demand doubled
```

Multiple events stack multiplicatively:
- Event A: 1.5x price
- Event B: 1.3x price
- Combined: 1.5 * 1.3 = 1.95x price

### Volatility Settings

Control price range:
```yaml
PriceVolatility:
  VolatilityMultiplier: 2.0  # Doubles price swings
  MinPriceDeviation: 0.3     # Can drop to 30% of base
  MaxPriceDeviation: 4.0     # Can rise to 400% of base
```

Use cases:
- **Stable commodities**: Low volatility (food, basic materials)
- **Luxury goods**: Medium volatility
- **Contraband**: Extreme volatility
- **Technology**: High volatility

### Custom Pricing in Blueprint

Override `OnCalculateCustomPrice`:
```
Event OnCalculateCustomPrice
  Inputs:
    - Supply (float)
    - Demand (float)
    - Event Multiplier (float)
    - Base Calculated Price (float)
  
  Logic:
    1. Get base calculated price
    2. Apply your custom modifiers
    3. Return final price
```

Example uses:
- Time-of-day pricing
- Seasonal pricing
- Player reputation discounts
- Bulk order discounts
- Loyalty programs

---

## Blueprint Integration

### Blueprint Event Hooks

All major systems have `BlueprintNativeEvent` hooks:

**Trade Items**:
- `OnCalculateCustomPrice`: Custom pricing logic
- `OnItemTraded`: Post-trade events

**Markets**:
- `OnCalculateCustomMarketPrice`: Market-specific pricing
- `OnStockRefreshed`: Stock refresh events
- `OnMarketEventStarted`: Event start handling
- `OnMarketEventEnded`: Event end handling

**Contracts**:
- `OnContractAccepted`: When player accepts
- `OnContractCompleted`: On successful completion
- `OnContractFailed`: On failure
- `OnContractExpired`: When time runs out

**AI Traders**:
- `OnEvaluateTrade`: Custom trade decision logic
- `OnTradeExecuted`: Post-trade events
- `OnContractCompleted`: AI contract completion

### Blueprint Trading Widget Example

```
Widget BP_TradingInterface:
  - Display market inventory
  - Show current prices
  - Buy/Sell buttons
  - Transaction confirmation
  - Contract listing
  
Variables:
  - CurrentMarket (MarketDataAsset)
  - SelectedItem (TradeItemDataAsset)
  - TransactionManager (TradeTransactionManager)
  
Functions:
  - RefreshInventory()
  - BuyItem(TradeItem, Quantity)
  - SellItem(TradeItem, Quantity)
  - AcceptContract(Contract)
  
Events:
  - OnTransactionComplete
  - OnPriceUpdate
  - OnMarketEventNotification
```

### Blueprint Helper Functions

Common Blueprint patterns:

**Get Current Price**:
```cpp
float Price = Market->GetItemPrice(TradeItem, bIsBuying);
```

**Check Stock Availability**:
```cpp
bool InStock = Market->IsItemInStock(ItemID, Quantity);
```

**Calculate Profit Margin**:
```cpp
float BuyPrice = OriginMarket->GetItemPrice(Item, true);
float SellPrice = DestMarket->GetItemPrice(Item, false);
float Profit = (SellPrice - BuyPrice) * Quantity;
```

**Check Trade Legality**:
```cpp
bool CanTrade = TradeItem->CanBeTradedByFaction(
    FactionID, Reputation);
```

---

## Advanced Features

### Market Manipulation

AI traders with `bCanManipulatePrices` can:
1. Buy large quantities to reduce supply
2. Raise prices through artificial scarcity
3. Dump inventory to crash prices
4. Create monopolies on specific items

Players can detect manipulation:
- Check transaction history
- Look for unusual price spikes
- Monitor AI trader inventory

### Arbitrage Opportunities

Profitable arbitrage requires:
- Price differential > (transport cost + time cost)
- Sufficient demand at destination
- No hostile factions on route
- Available cargo space

AI calculates profitability score:
```cpp
ProfitabilityScore = ProfitPerUnit / TravelTime
```

### Route Planning

Optimal routes consider:
- Distance (travel time)
- Profit per unit
- Cargo capacity utilization
- Fuel costs
- Hostile faction territories
- Market demand trends

AI traders maintain a list of active routes and continuously optimize.

### Event-Driven Economy

Market events create dynamic gameplay:
- **Scarcity events**: Rush to buy before price spikes
- **Surplus events**: Sell before price crashes
- **Emergency events**: High demand for specific items
- **Political events**: Faction restrictions change

Events can be:
- Random (probability-based)
- Scripted (quest/story-driven)
- Player-triggered (actions cause events)

### Faction-Based Trading

Faction relationships affect:
- Access to markets
- Item availability
- Prices (discounts/premiums)
- Contract availability
- Smuggling risks

Example:
- Allied faction: 20% price discount
- Neutral faction: Normal prices
- Hostile faction: No access to markets
- At war: All goods considered contraband

### Black Market Economics

Black markets operate differently:
- Higher taxes and markups
- No legal recourse for fraud
- Contraband items available
- Risk of authority raids
- Reputation required for access
- Price volatility extreme

Authority raid events:
- Temporarily close market
- Confiscate inventory
- Fine traders
- Reset reputation

---

## Multiplayer Considerations

### Shared Economy

For multiplayer:
1. **Central Transaction Manager**: Track all trades
2. **Replicated Market State**: Sync inventory across clients
3. **Event Broadcasting**: Notify all players of market events
4. **AI Trader Synchronization**: One authoritative instance

### Player Competition

Players compete for:
- Limited market inventory
- Profitable trade routes
- Contract opportunities
- AI trader contracts

First-come, first-served for:
- Contract acceptance
- Limited stock purchases
- Market event reactions

### Cooperative Trading

Allow players to:
- Share contract rewards
- Pool cargo space
- Share market information
- Form trading guilds/corporations

### Anti-Exploit Measures

Prevent:
- **Market flooding**: Limit transaction volumes
- **Price manipulation**: Track suspicious patterns
- **Duplication**: Validate all transactions server-side
- **Insider trading**: Delay event notifications
- **Bot trading**: Rate limiting, CAPTCHA for suspicious activity

---

## Performance Optimization

### Memory Management

- **Transaction History**: Prune old records (default 10,000 max)
- **Market Updates**: Update on intervals, not every frame
- **AI Trader Ticks**: Stagger update times
- **Event System**: Pool and reuse event objects

### Update Frequency

Recommended intervals:
- **Market prices**: Update every game day or on significant change
- **Stock refresh**: Every 24-48 game hours
- **AI trader decisions**: Every few seconds (staggered)
- **Transaction recording**: Immediate, async processing

### LOD for AI Traders

- **Near player**: Full AI logic, frequent updates
- **Same sector**: Simplified logic, moderate updates
- **Far away**: Minimal logic, infrequent updates
- **Offline**: Skip entirely or use statistical simulation

### Database Considerations

For persistent economies:
- **Save transaction history**: Export to CSV/JSON
- **Save market state**: Inventory levels, prices
- **Save AI trader state**: Capital, inventory, routes
- **Delta saving**: Only save changes, not full state

---

## Example Workflows

### Designer Workflow: Adding New Trade Item

1. Create YAML template based on category
2. Create Data Asset in Content Browser
3. Copy values from YAML to Data Asset
4. Add to market inventories
5. Test pricing at different supply/demand levels
6. Configure AI trader priority
7. Create related contracts if needed
8. Playtest and balance

### Designer Workflow: Creating Market Event

1. Identify trigger condition (time, quest, random)
2. Choose affected items
3. Set price/supply/demand multipliers
4. Set duration
5. Create Blueprint to trigger event
6. Test impact on economy
7. Balance rewards vs. disruption

### Programmer Workflow: Custom Pricing System

1. Create Blueprint class derived from TradeItemDataAsset
2. Override `OnCalculateCustomPrice` event
3. Implement custom formula
4. Test with various market conditions
5. Create Data Assets from Blueprint class
6. Document for designers

### Programmer Workflow: New AI Behavior

1. Add enum value to `EAITradeBehavior`
2. Implement behavior in `AITraderComponent.cpp`
3. Add behavior check in `MakeTradeDecisions()`
4. Expose configuration to Blueprint
5. Test with different strategies
6. Document behavior for designers

---

## Troubleshooting

### Prices Not Changing

**Check**:
- `bAffectedBySupplyDemand` is true
- Market inventory entry exists
- Supply/Demand levels are not both 1.0
- Volatility multiplier is not 0

### AI Traders Not Trading

**Check**:
- `bAllowAITraders` is true on market
- AI trader has capital
- AI trader knows about market (use `DiscoverMarket()`)
- Enabled behaviors include relevant types
- Min profit margin is not too high

### Contracts Not Completing

**Check**:
- All required cargo is in player inventory
- Player is at correct destination
- Contract has not expired
- `CompleteContract()` is being called

### Market Events Not Working

**Check**:
- Event `bIsActive` is true
- Event has not expired
- Item ID matches exactly
- Market is checking events in pricing

---

## Best Practices

### For Designers

1. **Start simple**: Basic items, standard markets, easy contracts
2. **Balance rewards**: Ensure contracts are worth player time
3. **Test price ranges**: Simulate extreme supply/demand scenarios
4. **Document decisions**: Note why items have specific prices
5. **Iterate based on data**: Use transaction analytics to balance

### For Programmers

1. **Extend, don't modify**: Use Blueprint events, not C++ rewrites
2. **Validate input**: Check for null references, invalid values
3. **Use async for I/O**: Don't block game thread for saves/loads
4. **Profile regularly**: Monitor AI trader performance
5. **Document extensibility**: Help designers customize system

### For Game Balance

1. **Meaningful choices**: Ensure multiple viable strategies
2. **Risk vs. Reward**: Higher risk = higher reward
3. **Progression curve**: Start easy, ramp up complexity
4. **Economic feedback**: Prices react to player actions
5. **Avoid inflation**: Balance money sources/sinks

---

## Appendix: Quick Reference

### Class Hierarchy
```
UPrimaryDataAsset
├── UTradeItemDataAsset
├── UMarketDataAsset
└── UTradeContractDataAsset

UActorComponent
└── UAITraderComponent

UObject
└── UTradeTransactionManager

Structs:
├── FTradeTransaction
├── FMarketEvent
├── FMarketInventoryEntry
├── FTradeRoute
├── FAITraderInventory
├── FContractCargo
├── FContractRewards
├── FContractPenalties
└── FContractLocation
```

### Key Files
```
C++ Headers:
- TradeItemDataAsset.h
- MarketDataAsset.h
- TradeContractDataAsset.h
- AITraderComponent.h
- TradeTransaction.h

YAML Templates:
- Assets/TradingTemplates/TradeItem_*.yaml
- Assets/TradingTemplates/Market_*.yaml
- Assets/TradingTemplates/Contract_*.yaml

Documentation:
- Assets/TradingSystemGuide.md (this file)
- Assets/TradingDesignerWorkflow.md
- Assets/TradingBlueprintExamples.md
- Assets/TradingAPIReference.md
```

### Common Console Commands
```
// Enable debug visualization
trading.debug 1

// Show AI trader paths
trading.showroutes 1

// Log all transactions
trading.logtransactions 1

// Trigger market event
trading.triggerevent EventID

// Spawn AI trader
trading.spawnai Strategy TradingCapital
```

---

## Support & Community

For questions and support:
- **Documentation**: This guide and related docs in `/Assets/`
- **Examples**: `/Assets/TradingTemplates/` for YAML templates
- **Code Comments**: Extensive inline documentation in C++
- **Community**: GitHub issues for bugs and feature requests

**Happy Trading!** 🚀💰
