# Marketplace Module - Complete Setup Guide

**Module Type**: `AMarketplaceModule`  
**Blueprint Class**: `BP_SpaceStationModule_Marketplace`  
**Power Consumption**: 40 units  
**Module Group**: Public  
**Last Updated**: 2026-01-18

---

## Overview

The Marketplace Module provides commercial trading facilities for buying, selling, and trading goods. It includes vendor stalls, trading terminals, and auction systems with configurable market data assets for inventory and pricing.

**Key Features:**
- Configurable market data asset for inventory and pricing
- Can be opened/closed for trading
- Supports multiple marketplaces per station
- Blueprint-accessible trading API
- Dynamic pricing support

---

## Quick Start

### Add as Child Actor Component

1. Open your station Blueprint (`BP_SpaceStation`)
2. Go to **Components** panel
3. Click **+ Add** → Search for **Child Actor Component**
4. Rename to "Marketplace"
5. In Details panel:
   - **Child Actor Class**: `BP_SpaceStationModule_Marketplace`
6. Position in viewport relative to your station
7. Compile and save

> **Note:** This is the recommended method as it provides visual feedback at design-time. You can see and position your marketplace module in the editor viewport.

---

## Detailed Configuration

### Step 1: Create Market Data Asset

Market Data Assets define what the marketplace sells and buys.

1. **Content Browser** → Right-Click → **Data Asset**
2. Select **MarketDataAsset**
3. Name: `DA_Market_[StationType]` (e.g., `DA_Market_Agricultural`)
4. Open the asset to configure

**Basic Properties:**

```
Market Name: "Farming Collective Market"
Market Type: Open Market
Market Size: Medium

Sell Price Markup: 1.1     (player pays 10% more when buying)
Buy Price Markdown: 0.9     (player receives 10% less when selling)
Transaction Tax Rate: 0.05  (5% tax on transactions)

Allow Player Buying: true
Allow Player Selling: true
Allow AI Traders: false (MVP)
```

**Inventory Configuration:**

Add items to the **Inventory** array:

```
[0] Trade Item: DA_Item_Water
    Current Stock: 10000
    Max Stock: 50000
    Supply Level: 2.0 (abundant - station produces)
    Demand Level: 0.5 (low need)

[1] Trade Item: DA_Item_Food
    Current Stock: 5000
    Max Stock: 20000
    Supply Level: 1.5 (good supply)
    Demand Level: 0.8 (moderate demand)

[2] Trade Item: DA_Item_Electronics
    Current Stock: 100
    Max Stock: 500
    Supply Level: 0.2 (scarce - needs imports)
    Demand Level: 2.0 (high demand)
```

### Step 2: Create/Configure Marketplace Blueprint

If `BP_SpaceStationModule_Marketplace` doesn't exist:

1. **Content Browser** → Right-Click → **Blueprint Class**
2. Search for and select **MarketplaceModule** as parent
3. Name: `BP_SpaceStationModule_Marketplace`
4. Open the Blueprint

### Step 3: Assign Market Data Asset

In **Class Defaults → Marketplace**:

- **Market Data Asset**: `DA_Market_Agricultural` (your asset)
- **Is Open**: ✓ true (marketplace is open for trading)
- **Marketplace Name**: "Central Market" (display name)

### Step 4: Add to Station

Follow Quick Start method 1 or 2 above.

---

## Component Hierarchy Example

```
BP_SpaceStationModule_Marketplace (Self)
├─ StaticMesh_Marketplace (Static Mesh)
│  └─ Mesh: SM_Marketplace_01
├─ SignageLight (Point Light, optional)
│  └─ Color: Blue (open), Gray (closed)
└─ TradingTerminals (Static Mesh, optional)
   └─ Mesh: SM_TradingTerminal_01
```

---

## Properties Reference

### Market Data Asset

**Type**: `UMarketDataAsset*`  
**Access**: EditAnywhere, BlueprintReadWrite  
**Category**: Marketplace

Market configuration defining what this marketplace sells and buys. Contains inventory, pricing rules, and trading permissions.

**Must be assigned** for marketplace to function.

### Is Open

**Type**: `bool`  
**Default**: true  
**Access**: EditAnywhere, BlueprintReadWrite  
**Category**: Marketplace

Whether this marketplace is currently open for trading. Can be toggled at runtime to open/close the market.

### Marketplace Name

**Type**: `FText`  
**Default**: (empty)  
**Access**: EditAnywhere, BlueprintReadWrite  
**Category**: Marketplace

Display name for this marketplace (e.g., "Central Market", "Black Market", "Trade Exchange").

---

## Blueprint Functions

### GetMarketData()

**Type**: BlueprintCallable, BlueprintPure  
**Returns**: `UMarketDataAsset*` - The market data asset or nullptr

Get the market data asset for this marketplace.

### IsAvailableForTrading()

**Type**: BlueprintCallable, BlueprintPure  
**Returns**: `bool` - True if marketplace can be used

Check if this marketplace is open and has a valid market configuration.

```
Available = bIsOpen && (MarketDataAsset != nullptr)
```

---

## Viewing Module Information in Station Blueprint

**NEW**: When you add marketplace modules to a station, you can see their information directly in the station Blueprint!

### Station-Level Functions

The station Blueprint provides functions to query marketplace information:

#### GetOpenMarketplaceCount()

**Type**: BlueprintCallable, BlueprintPure  
**Category**: Station|Trading  
**Returns**: `int32` - Number of marketplaces currently open for trading

Shows the count of marketplaces that are open and available for trading.

**Example Usage:**
```
In BP_SpaceStation Class Defaults or Event Graph:
  Get Open Marketplace Count → Print String
  
Output: "This station has 2 open marketplaces"
```

#### GetTotalMarketplaceCount()

**Type**: BlueprintCallable, BlueprintPure  
**Category**: Station|Trading  
**Returns**: `int32` - Total number of marketplace modules

Shows the total count of all marketplace modules (open or closed).

**Example Usage:**
```
In BP_SpaceStation:
  Get Total Marketplace Count → Print String
  
Output: "This station has 3 total marketplaces"
```

#### GetMarketplaceNames()

**Type**: BlueprintCallable  
**Category**: Station|Trading  
**Returns**: `TArray<FText>` - Array of marketplace display names

Gets the names of all marketplaces on the station. Useful for displaying a list of available markets in UI.

**Example Usage:**
```
In BP_SpaceStation:
  Get Marketplace Names → For Each Loop
    → Print String (Loop Body)
  
Output: "Central Market", "Industrial Exchange", "Luxury Bazaar"
```

#### GetMarketplaceModule() / GetMarketplaceModules()

**Type**: BlueprintCallable  
**Category**: Station|Trading  
**Returns**: Single marketplace or array of all marketplaces

Access marketplace modules directly to query their properties.

### How to View in Editor

**Method 1: Event Graph (Design-Time Preview)**

1. Open your station Blueprint (`BP_SpaceStation`)
2. Go to **Event Graph**
3. Add node: **Event Construction Script**
4. Call **Get Open Marketplace Count**
5. Connect to **Print String**
6. When you add/remove marketplace modules in the Components panel, the print will update

**Method 2: Custom Display Variables**

Create display-only variables in your station Blueprint:

```
In BP_SpaceStation Class Defaults:

1. Add variable: "OpenMarketplaceCountDisplay" (int32)
2. Set to: VisibleAnywhere, BlueprintReadOnly
3. Category: "Station Info"

In Event Construction Script:
  Get Open Marketplace Count → Set OpenMarketplaceCountDisplay
```

Now you'll see the count in the Details panel!

### Example Blueprint Setup

```
Event Construction Script
    ↓
Get Open Marketplace Count (Self) → Set OpenMarketplaceCountDisplay
    ↓
Get Total Marketplace Count (Self) → Set TotalMarketplaceCountDisplay
    ↓
Get Marketplace Names (Self) → Array Length → Set MarketplaceNamesCountDisplay
```

Result in Details panel:
```
Station Info:
  Open Marketplace Count Display: 2
  Total Marketplace Count Display: 3
  Marketplace Names Count Display: 3
```

**Combined Station Information:**

You can combine docking bay and marketplace information:

```
Event Construction Script
    ↓
Branch: Has Docking Capability?
    ├─ True: Get Total Docking Points → Set display variable
    └─ False: Set display to 0
    ↓
Branch: Has Marketplace?
    ├─ True: Get Open Marketplace Count → Set display variable
    └─ False: Set display to 0
```

---

## Market Data Asset Configuration

### Market Types

```cpp
enum class EMarketType : uint8
{
    OpenMarket,          // General trading (MVP default)
    BlackMarket,         // Illegal goods (post-MVP)
    CommodityExchange,   // Bulk trading (post-MVP)
    LuxuryBazaar,        // High-end goods (post-MVP)
    IndustrialDepot,     // Manufacturing supplies (post-MVP)
    ResearchHub          // Tech and medicine (post-MVP)
};
```

**For MVP**: Use `OpenMarket` exclusively.

### Market Sizes

```cpp
enum class EMarketSize : uint8
{
    Small,    // Outpost (limited stock)
    Medium,   // Standard station
    Large,    // Major hub
    Megacity, // Massive exchange (post-MVP)
    Capital   // Planetary market (post-MVP)
};
```

### Pricing Formula

```
Player Buy Price = BasePrice × SellPriceMarkup × SupplyFactor × EventMultiplier
Player Sell Price = BasePrice × BuyPriceMarkdown × DemandFactor × EventMultiplier

Supply Factor = 1.0 + (Demand - Supply) / 100
```

### Example Market Configurations

#### Agricultural Station

```yaml
Market Name: "Farming Collective Market"
Market Type: Open Market
Market Size: Medium

Produces: Water, Food (cheap)
Imports: Electronics, Technology (expensive)

Sell Price Markup: 1.1  # 10% markup
Buy Price Markdown: 0.9  # 10% markdown

Inventory:
  - Water: Stock 10000, Supply 2.0, Demand 0.5
  - Food: Stock 5000, Supply 1.5, Demand 0.8
  - Electronics: Stock 100, Supply 0.2, Demand 2.0
```

#### Industrial Station

```yaml
Market Name: "Industrial Exchange"
Market Type: Industrial Depot
Market Size: Large

Produces: Metals, Components (cheap)
Imports: Food, Medicine (expensive)

Sell Price Markup: 1.3  # 30% markup
Buy Price Markdown: 0.7  # 30% markdown

Inventory:
  - Metals: Stock 20000, Supply 2.5, Demand 0.3
  - Components: Stock 8000, Supply 2.0, Demand 0.5
  - Food: Stock 500, Supply 0.3, Demand 2.5
  - Medicine: Stock 200, Supply 0.2, Demand 3.0
```

#### Luxury Station

```yaml
Market Name: "Opulent Bazaar"
Market Type: Luxury Bazaar
Market Size: Small

Produces: Art, Entertainment (expensive)
Imports: Luxury Food, Rare Goods (very expensive)

Sell Price Markup: 2.0  # 100% markup (double price)
Buy Price Markdown: 0.8  # 20% markdown

Inventory:
  - Art: Stock 50, Supply 0.5, Demand 1.0
  - Entertainment: Stock 100, Supply 0.8, Demand 1.2
  - Luxury Food: Stock 200, Supply 0.3, Demand 2.0
```

---

## Trade Item Configuration

Create Trade Item Data Assets for each good:

1. **Content Browser** → Right-Click → **Data Asset**
2. Select **TradeItemDataAsset**
3. Name: `DA_Item_[Name]` (e.g., `DA_Item_Water`)

**Properties:**

```
Item Name: "Water"
Description: "Purified water essential for life support"
Item ID: "water" (unique identifier)

Base Price: 10.0 (credits per unit)
Cargo Volume: 1.0 (units of space)
Volatility: 0.3 (price fluctuation, 0.0-1.0)

Category: BasicGoods
Rarity: Common
Legal Status: Legal

Icon: T_Icon_Water (texture)
3D Model: SM_Cargo_Water (optional)
```

### Recommended MVP Items (15-20 total)

**Basic Goods** (low prices, low volatility):
- Water: 10 credits
- Food: 25 credits
- Fuel: 50 credits

**Luxury Items** (high prices, high volatility):
- Fine Wine: 200 credits
- Art: 1000 credits
- Entertainment: 500 credits

**Industrial** (medium prices, medium volatility):
- Metals: 100 credits
- Components: 250 credits
- Machinery: 500 credits

**Medical** (high prices, low volatility):
- Medicine: 300 credits
- Med Equipment: 800 credits

**Technology** (very high prices, high volatility):
- Electronics: 500 credits
- Software: 1000 credits
- AI Cores: 5000 credits

---

## Testing Checklist

### Pre-Test Setup

- [ ] Market Data Asset created and configured
- [ ] Trade Item Data Assets created for all goods
- [ ] Marketplace Module added to station
- [ ] Market Data Asset assigned to marketplace
- [ ] Is Open = true
- [ ] Marketplace Name set
- [ ] All Blueprints compiled and saved

### In-Game Testing

1. **Test Market Availability**
   - Dock at station
   - Verify marketplace detected: `Station->HasMarketplace()`
   - Verify marketplace available: `Marketplace->IsAvailableForTrading()`

2. **Test Trading UI**
   - Trading UI should open after docking
   - Market items display with correct prices
   - Can add items to shopping cart
   - Cart total calculates correctly

3. **Test Buying**
   - Select item and quantity
   - Verify sufficient credits check
   - Verify sufficient cargo space check
   - Complete purchase
   - Credits decrease correctly
   - Cargo increases correctly

4. **Test Selling**
   - Have items in cargo
   - Select item and quantity
   - Verify item in cargo check
   - Complete sale
   - Credits increase correctly
   - Cargo decreases correctly

5. **Test Market Closure**
   - Set `bIsOpen = false` at runtime
   - Verify trading disabled
   - Verify proper error message

---

## Common Issues & Solutions

### Issue: Trading UI Won't Open

**Likely Cause**: MarketDataAsset not assigned  
**Solution**:
- Open marketplace Blueprint Class Defaults
- Verify Market Data Asset field is populated
- Assign your Market Data Asset
- Compile and save

### Issue: No Items in Market

**Likely Cause**: Market inventory empty  
**Solution**:
- Open Market Data Asset
- Add entries to Inventory array
- Assign Trade Item Data Assets
- Set stock levels > 0
- Save asset

### Issue: Prices Seem Wrong

**Likely Cause**: Markup/markdown not configured  
**Solution**:
- Check Sell Price Markup (1.1-1.5 typical)
- Check Buy Price Markdown (0.7-0.9 typical)
- Formula: Buy = Base × Markup, Sell = Base × Markdown
- Test with console: `GetItemPrice`

### Issue: Can't Buy Items

**Possible Causes**:
1. Insufficient credits
   - Solution: Check player credits balance
2. Insufficient cargo space
   - Solution: Check cargo capacity and current usage
3. Item out of stock
   - Solution: Check Current Stock in market inventory
4. Market closed
   - Solution: Verify bIsOpen = true

### Issue: Multiple Markets Interfere

**Cause**: Same MarketDataAsset used on multiple marketplaces  
**Solution**:
- Create unique MarketDataAsset for each marketplace
- Or: Intentionally share if markets should have shared inventory
- Note: Shared asset = shared stock levels

---

## Blueprint Examples

### Check Market Availability (Blueprint)

```
Event: Player Requests Trade
    ↓
Get Docked Station Reference
    ↓
Branch: Has Marketplace?
    ├─ False → Show "No Marketplace Available"
    │
    └─ True:
        ↓
        Get Marketplace Module
        ↓
        Branch: Is Available For Trading?
        ├─ False → Show "Market Closed"
        │
        └─ True:
            ↓
            Get Market Data
            ↓
            Open Widget: WBP_TradingUI
            ↓
            Initialize UI with Market Data
```

### Get Item Price (Blueprint)

```
Function: GetItemPriceForPlayer
Input: Trade Item (UTradeItemDataAsset), Is Buying (bool)
    ↓
Get Market Data Asset
    ↓
Call: Get Item Price (TradeItem, IsBuying)
    ↓
Return: Price (float)
```

### Execute Trade (Blueprint)

```
Function: ExecutePlayerTrade
Input: Items (Array), Quantities (Array), Is Buying (bool)
    ↓
For Each: Item in Items
    ↓
    Calculate Total Price
    ↓
    Validate Transaction:
        - Sufficient Credits?
        - Sufficient Cargo Space?
        - Item In Stock?
    ↓
    If Valid:
        ↓
        Update Player Credits
        ↓
        Update Player Cargo
        ↓
        Update Market Stock
        ↓
        Fire OnTradeCompleted Event
    ↓
Return: Success (bool)
```

---

## Integration with Station

### Complete Trading Station Example

Your station needs:
1. **Docking Bay Module** (for ship docking)
2. **Marketplace Module** (this module)
3. **Market Data Asset** (inventory and pricing)
4. **Trade Item Data Assets** (individual goods)

```
BP_TradingStation_Agricultural
├─ StaticMesh_StationCore (Static Mesh)
├─ DockingBay (Child Actor Component)
│  └─ Child Actor Class: BP_SpaceStationModule_DockingBay
├─ Marketplace (Child Actor Component)
│  └─ Child Actor Class: BP_SpaceStationModule_Marketplace
│      └─ Market Data Asset: DA_Market_Agricultural
└─ DetectionRange (Sphere Component)
```

---

## Advanced Topics

### Dynamic Market Events (Post-MVP)

```cpp
struct FMarketEvent
{
    EventName: "Supply Shortage"
    AffectedItems: ["Water", "Food"]
    PriceMultiplier: 1.5
    DurationHours: 24.0
};
```

**Example Events:**
- Drought → Water prices +50%
- Tech Expo → Electronics demand spikes
- Authority Raid → Black market disrupted
- Surplus → Food prices -30%

### Multiple Marketplaces Per Station

A station can have multiple marketplace modules:

```
BP_MajorHub_Station
├─ Marketplace_Central (Child Actor)
│  └─ Market Data: DA_Market_General
├─ Marketplace_Industrial (Child Actor)
│  └─ Market Data: DA_Market_Industrial
└─ Marketplace_Luxury (Child Actor)
   └─ Market Data: DA_Market_Luxury
```

Access via:
- `Station->GetMarketplaceModule()` - Gets first marketplace
- `Station->GetMarketplaceModules()` - Gets all marketplaces

---

## Performance Considerations

- **Market Updates**: Update prices every 30 seconds (configurable)
- **Stock Tracking**: Update only when transactions occur
- **Power Consumption**: 40 units per marketplace
- **UI Refresh**: Only when trading UI is open
- **AI Traders**: Disabled for MVP (post-MVP feature)

---

## Related Documentation

- [Complete Trading and Docking Guide](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)
- [Docking Bay Module Guide](DOCKING_BAY_MODULE.md)
- [Cargo Bay Module Guide](CARGO_BAY_MODULE.md)
- [Trade Simulator MVP Instructions](../.github/instructions/trade-simulator-mvp.instructions.md)

---

## Version History

**v1.0** (2026-01-18)
- Initial documentation
- Comprehensive setup guide
- Market configuration examples
- Blueprint integration guide

---

**Maintained by**: Adastrea Development Team  
**For Support**: See main project README.md
