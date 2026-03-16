# Trading System Guide

## Overview

The Adastrea trading system is a dynamic economy simulation that powers the game's MVP (Minimum Viable Product). This system enables players to buy low, sell high, and build their trading empire across the galaxy.

## Core Components

### 1. Trade Item Data Assets (`DA_Item_*`)
Each trade item is defined by a TradeItemDataAsset with the following properties:

#### Basic Information
- **ItemName**: Display name of the item
- **Description**: Detailed description of the item
- **ItemID**: Unique identifier (lowercase with underscores)
- **Category**: Item category (Technology, Medicine, Food, Materials, etc.)

#### Pricing
- **BasePrice**: Base price in credits
- **PriceVolatility**: Market volatility configuration
  - `VolatilityMultiplier`: How much prices can fluctuate
  - `MinPriceDeviation`: Minimum price deviation multiplier
  - `MaxPriceDeviation`: Maximum price deviation multiplier
- **bAffectedBySupplyDemand**: Whether supply/demand affects price
- **bAffectedByMarketEvents**: Whether market events affect price

#### Legality & Restrictions
- **LegalityStatus**: Legal, Restricted, or Contraband
- **ContrabandFineMultiplier**: Fine multiplier if caught with contraband

#### Trade Properties
- **VolumePerUnit**: Volume in cubic meters per unit
- **MassPerUnit**: Mass in kilograms per unit
- **StandardLotSize**: Standard trading lot size
- **TypicalMarketStock**: Typical stock available in markets
- **ReplenishmentRate**: How quickly stock replenishes

#### AI Trader Behavior
- **AITradePriority**: AI trading priority (1-10)
- **bAIHoardable**: Whether AI traders hoard this item
- **bAIArbitrageEnabled**: Whether AI engages in arbitrage
- **BehaviorTags**: Special behavior tags

### 2. Market Data Assets (`DA_Market_*`)
Each market is defined by a MarketDataAsset with the following properties:

#### Market Information
- **MarketName**: Display name of the market
- **MarketID**: Unique identifier
- **StationType**: Type of station (Agricultural, Industrial, Research, etc.)
- **FactionAffiliation**: Controlling faction

#### Economic Properties
- **BaseDemandMultiplier**: Base demand multiplier
- **BaseSupplyMultiplier**: Base supply multiplier
- **PriceAdjustmentRate**: How quickly prices adjust
- **MaxStockCapacity**: Maximum stock capacity
- **MinStockThreshold**: Minimum stock threshold

#### Trade Restrictions
- **AllowedItemCategories**: Categories allowed in this market
- **RestrictedItems**: Specifically restricted items
- **ContrabandItems**: Items considered contraband
- **ImportTaxRate**: Tax rate for imports
- **ExportTaxRate**: Tax rate for exports

#### Market Events
- **EventProbability**: Probability of market events
- **EventCooldown**: Minimum time between events
- **SupportedEventTypes**: Types of events supported

## Current Trade Items (MVP)

### Technology Category
1. **Advanced Electronics** (`advanced_electronics`)
   - Base Price: 1200 credits
   - Volume: 3.0 m³, Mass: 2.0 kg
   - Volatility: High (1.8x multiplier)

2. **AI Cores** (`ai_cores`)
   - Base Price: 5000 credits
   - Volume: 1.5 m³, Mass: 1.0 kg
   - Volatility: Very High (2.2x multiplier)

3. **Antimatter Cores** (`antimatter_cores`)
   - Base Price: 15000 credits
   - Volume: 2.0 m³, Mass: 3.0 kg
   - Volatility: Extreme (2.5x multiplier)

4. **Basic Computers** (`basic_computers`)
   - Base Price: 800 credits
   - Volume: 2.5 m³, Mass: 1.5 kg
   - Volatility: Medium (1.5x multiplier)

5. **Entertainment Systems** (`entertainment_systems`)
   - Base Price: 600 credits
   - Volume: 4.0 m³, Mass: 3.0 kg
   - Volatility: Low (1.2x multiplier)

6. **Ship Components** (`ship_components`)
   - Base Price: 2000 credits
   - Volume: 5.0 m³, Mass: 4.0 kg
   - Volatility: Medium (1.6x multiplier)

### Medicine Category
7. **Advanced Medicine** (`advanced_medicine`)
   - Base Price: 1800 credits
   - Volume: 1.0 m³, Mass: 0.5 kg
   - Volatility: Medium (1.5x multiplier)

8. **Basic Medicine** (`basic_medicine`)
   - Base Price: 400 credits
   - Volume: 1.5 m³, Mass: 0.8 kg
   - Volatility: Low (1.3x multiplier)

### Food Category
9. **Luxury Food** (`luxury_food`)
   - Base Price: 300 credits
   - Volume: 2.0 m³, Mass: 1.0 kg
   - Volatility: Low (1.2x multiplier)

10. **Protein Packs** (`protein_packs`)
    - Base Price: 150 credits
    - Volume: 1.0 m³, Mass: 0.5 kg
    - Volatility: Very Low (1.1x multiplier)

### Materials Category
11. **Basic Alloys** (`basic_alloys`)
    - Base Price: 200 credits
    - Volume: 3.0 m³, Mass: 5.0 kg
    - Volatility: Low (1.2x multiplier)

12. **Hydrogen Fuel** (`hydrogen_fuel`)
    - Base Price: 100 credits
    - Volume: 10.0 m³, Mass: 0.7 kg
    - Volatility: Medium (1.4x multiplier)

13. **Refined Metals** (`refined_metals`)
    - Base Price: 450 credits
    - Volume: 2.0 m³, Mass: 8.0 kg
    - Volatility: Medium (1.5x multiplier)

14. **Water (Purified)** (`water_purified`)
    - Base Price: 50 credits
    - Volume: 5.0 m³, Mass: 5.0 kg
    - Volatility: Very Low (1.1x multiplier)

### Luxury Category
15. **Rare Art** (`rare_art`)
    - Base Price: 2500 credits
    - Volume: 2.0 m³, Mass: 1.5 kg
    - Volatility: High (1.8x multiplier)

## Current Markets (MVP)

### Agricultural Station Market
- **Station Type**: Agricultural
- **Specialization**: Food production and basic materials
- **High Demand**: Protein Packs, Basic Medicine, Water
- **High Supply**: Luxury Food, Basic Alloys
- **Restricted**: Advanced Technology, Contraband

### Industrial Station Market
- **Station Type**: Industrial
- **Specialization**: Manufacturing and materials
- **High Demand**: Basic Alloys, Refined Metals, Ship Components
- **High Supply**: Basic Computers, Entertainment Systems
- **Restricted**: Luxury Items, Advanced Medicine

### Research Station Market
- **Station Type**: Research
- **Specialization**: Technology and research materials
- **High Demand**: Advanced Electronics, AI Cores, Antimatter Cores
- **High Supply**: Advanced Medicine, Rare Art
- **Restricted**: Basic Materials, Bulk Commodities

## Trading Mechanics

### Price Calculation
The actual price of an item in a market is calculated as:

```
Actual Price = BasePrice × MarketMultiplier × SupplyDemandMultiplier × EventMultiplier
```

Where:
- **MarketMultiplier**: Station type modifier (0.8-1.2)
- **SupplyDemandMultiplier**: Based on current stock levels (0.5-2.0)
- **EventMultiplier**: Temporary event effects (0.5-3.0)

### Supply and Demand
- **High Supply**: Prices decrease (multiplier 0.5-0.9)
- **Low Supply**: Prices increase (multiplier 1.1-1.5)
- **High Demand**: Prices increase (multiplier 1.2-2.0)
- **Low Demand**: Prices decrease (multiplier 0.8-1.0)

### Market Events
Random events that affect prices and availability:
1. **Shortage**: Supply decreases, prices increase
2. **Surplus**: Supply increases, prices decrease
3. **High Demand**: Demand increases, prices increase
4. **Trade Embargo**: Certain items become restricted
5. **Technological Breakthrough**: Technology prices drop
6. **Medical Emergency**: Medicine prices increase

## Player Trading

### Buying and Selling
1. **Buy Price**: Market's sell price (includes markup)
2. **Sell Price**: Market's buy price (includes markdown)
3. **Spread**: Difference between buy and sell prices (typically 10-30%)

### Cargo Management
- **Cargo Space**: Measured in cubic meters (m³)
- **Mass Limits**: Ships have maximum mass capacity
- **Lot Sizes**: Items typically traded in standard lots

### Profit Calculation
```
Profit = (Sell Price - Buy Price) × Quantity - Transaction Costs
```

**Transaction Costs**:
- Import/Export taxes
- Docking fees
- Brokerage fees (if using brokers)

## AI Trading

### Trader Behavior
AI traders follow these patterns:
1. **Arbitrage**: Buy low in one market, sell high in another
2. **Hording**: Stockpile items during shortages
3. **Market Making**: Provide liquidity by buying and selling
4. **Speculation**: Buy items expected to increase in value

### Trade Routes
AI traders establish profitable trade routes:
1. **Agricultural → Industrial**: Food to materials
2. **Industrial → Research**: Materials to technology
3. **Research → Agricultural**: Technology to food production

## Integration with Other Systems

### Ship Systems
- **Cargo Capacity**: Determines how much can be carried
- **Jump Range**: Affects which markets are accessible
- **Fuel Consumption**: Trading costs include fuel

### Faction System
- **Reputation**: Affects prices and access
- **Trade Agreements**: Special pricing for allies
- **Embargoes**: Cannot trade with enemies

### Quest System
- **Delivery Quests**: Transport specific items
- **Procurement Quests**: Acquire rare items
- **Market Manipulation**: Affect prices for factions

## Data Asset Structure

### Trade Item Data Asset Example
```yaml
# DA_Item_AdvancedElectronics
Class: /Script/Adastrea.TradeItemDataAsset
Asset Name: DA_Item_AdvancedElectronics

ItemName: Advanced Electronics
Description: High-tech electronic components and systems.
ItemID: advanced_electronics
Category: Technology

BasePrice: 1200
PriceVolatility:
  VolatilityMultiplier: 1.8
  MinPriceDeviation: 0.4
  MaxPriceDeviation: 3.0

bAffectedBySupplyDemand: true
bAffectedByMarketEvents: true

LegalityStatus: Legal
ContrabandFineMultiplier: 1.0

VolumePerUnit: 3.0
MassPerUnit: 2.0
StandardLotSize: 5
TypicalMarketStock: 500
ReplenishmentRate: 40

AITradePriority: 5
bAIHoardable: false
bAIArbitrageEnabled: true
BehaviorTags: []
```

### Market Data Asset Example
```yaml
# DA_Market_AgriculturalStationMarket
Class: /Script/Adastrea.MarketDataAsset
Asset Name: DA_Market_AgriculturalStationMarket

MarketName: Agricultural Station Market
MarketID: agricultural_station_market
StationType: Agricultural
FactionAffiliation: Neutral

BaseDemandMultiplier: 1.2
BaseSupplyMultiplier: 0.8
PriceAdjustmentRate: 0.1
MaxStockCapacity: 10000
MinStockThreshold: 100

AllowedItemCategories: [Food, Medicine, Materials]
RestrictedItems: [advanced_electronics, ai_cores]
ContrabandItems: []
ImportTaxRate: 0.05
ExportTaxRate: 0.03

EventProbability: 0.1
EventCooldown: 3600
SupportedEventTypes: [Shortage, Surplus, HighDemand]
```

## Implementation Notes

### Performance Considerations
1. **Price Caching**: Cache calculated prices to avoid recomputation
2. **Event Cooldowns**: Limit frequency of market events
3. **AI Optimization**: Batch AI trading decisions
4. **Network Sync**: Minimal data sync for multiplayer

### Extensibility
1. **New Item Categories**: Add new categories via enum expansion
2. **Custom Events**: Create custom market event types
3. **Dynamic Markets**: Markets can change based on gameplay
4. **Mod Support**: Expose data assets for modding

### Testing
1. **Unit Tests**: Price calculation, supply/demand logic
2. **Integration Tests**: Full trading scenarios
3. **Balance Tests**: Ensure profitable trading is possible
4. **Performance Tests**: Handle many concurrent traders

## Future Enhancements

### Planned Features
1. **Commodity Exchanges**: Futures and options trading
2. **Player-Owned Markets**: Set up your own trading posts
3. **Dynamic Economy**: Player actions affect global prices
4. **Trade Wars**: Compete with other traders
5. **Smuggling Mechanics**: Illegal trading with risks

### Advanced Systems
1. **Automated Trading**: Set up automated trade routes
2. **Market Analysis Tools**: Predict price movements
3. **Trade Network**: Connect with other player traders
4. **Economic Warfare**: Sabotage competitors' trades

## Troubleshooting

### Common Issues
1. **Prices Not Updating**: Check event cooldowns and update frequency
2. **AI Not Trading**: Verify AI priority settings and available credits
3. **Market Stock Issues**: Check replenishment rates and capacity limits
4. **Performance Problems**: Reduce number of active markets or traders

### Debug Commands
```cpp
// Force price update for all items
UpdateAllMarketPrices();

// Trigger specific market event
TriggerMarketEvent(EMarketEventType::Shortage);

// Reset market to default state
ResetMarket(MarketID);

// Dump market state to log
DumpMarketState(MarketID);
```

## Conclusion

The trading system is the core of Adastrea's MVP, providing engaging gameplay through economic simulation. By understanding the data assets, market mechanics, and integration points, you can create compelling trading experiences and expand the system as the game grows.

For implementation details, see the C++ source code in `Source/Adastrea/Trading/` and the example data assets in `Content/DataAssets/Trading/MVP/`.