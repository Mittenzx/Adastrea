# Trading and Docking System - Complete Guide

**Last Updated**: 2026-01-16  
**Version**: 1.0  
**Status**: Unified Reference Guide

> **⭐ This is the UNIFIED guide** for all trading and docking functionality in Adastrea. All previous separate guides have been consolidated here.

---

## 📋 Table of Contents

### Part 1: Overview & Architecture
1. [System Overview](#system-overview)
2. [Quick Start](#quick-start)
3. [Architecture](#architecture)
4. [Prerequisites](#prerequisites)

### Part 2: Trading System
5. [Trading Core Concepts](#trading-core-concepts)
6. [Market Data Assets](#market-data-assets)
7. [Trade Items](#trade-items)
8. [Marketplace Modules](#marketplace-modules)
9. [Trading UI](#trading-ui)
10. [Economy Manager](#economy-manager)
11. [Cargo System](#cargo-system)

### Part 3: Docking System
12. [Docking Core Concepts](#docking-core-concepts)
13. [Docking Setup](#docking-setup)
14. [Docking Interaction](#docking-interaction)
15. [Docking UI](#docking-ui)
16. [Docking Points](#docking-points)

### Part 4: Implementation Guides
17. [Creating a Trading Station](#creating-a-trading-station)
18. [Setting Up Player Ship](#setting-up-player-ship)
19. [Implementing Docking Flow](#implementing-docking-flow)
20. [Building Trading UI](#building-trading-ui)

### Part 5: Integration & Testing
21. [Complete Trading Loop](#complete-trading-loop)
22. [Testing Checklist](#testing-checklist)
23. [Common Issues](#common-issues)
24. [Troubleshooting](#troubleshooting)

### Part 6: Advanced Topics
25. [AI Traders](#ai-traders)
26. [Dynamic Pricing](#dynamic-pricing)
27. [Trade Contracts](#trade-contracts)
28. [Market Events](#market-events)

### Part 7: Reference
29. [C++ API Reference](#c-api-reference)
30. [Blueprint Nodes Reference](#blueprint-nodes-reference)
31. [Data Asset Templates](#data-asset-templates)
32. [Related Documentation](#related-documentation)

---

## Part 1: Overview & Architecture

### System Overview

The Adastrea Trading and Docking System provides a complete economic simulation framework for the MVP trading gameplay loop:

**Core Experience**:
```
Dock at Station A → View Market → Buy Cargo (low price) →
Undock → Fly to Station B → Dock → Sell Cargo (high price) →
Calculate Profit → Upgrade Ship → Repeat
```

**Key Features**:
- ✅ **Modular Station Architecture**: Trading via marketplace modules attached to stations
- ✅ **Dynamic Markets**: Supply/demand-based pricing with market data assets
- ✅ **Blueprint-Friendly**: Full C++ exposure for designers
- ✅ **Docking System**: Automatic approach, docking, and undocking with C++ core logic
- ✅ **Cargo Management**: Inventory system with space limits
- ✅ **Economy Simulation**: Price fluctuations, stock levels, market events

**Target Audience**:
- Designers creating trading stations and markets
- Programmers implementing trading logic
- Blueprint creators building UI and interactions

---

### Quick Start

**For Designers (No C++ Required)**:

1. **Create a Market Data Asset**:
   - Content Browser → Right-Click → Data Asset → MarketDataAsset
   - Name: `DA_Market_YourStation`
   - Configure inventory, prices, and markup/markdown

2. **Configure Station with Marketplace**:
   - Open `BP_SpaceStation` Class Defaults
   - Add to `DefaultModuleClasses`:
     - `BP_SpaceStationModule_DockingBay`
     - `BP_SpaceStationModule_Marketplace`
   - In marketplace module, assign your MarketDataAsset

3. **Test**:
   - Drag station into level
   - Press Play
   - Fly close and press F to dock
   - Trading UI opens automatically

**For Programmers (C++ Implementation)**:

```cpp
// Check if station has trading capability
if (Station->HasMarketplace())
{
    AMarketplaceModule* Marketplace = Station->GetMarketplaceModule();
    if (Marketplace->IsAvailableForTrading())
    {
        UMarketDataAsset* MarketData = Marketplace->GetMarketData();
        // Open trading UI with market data
    }
}
```

---

### Architecture

#### Modular Station System

Trading in Adastrea uses a **modular station architecture**:

```
ASpaceStation (Base Station Actor)
├─ ADockingBayModule (Required for docking)
├─ AMarketplaceModule (Required for trading)
│   └─ UMarketDataAsset (Market configuration)
└─ ACargoBayModule (Optional storage)
```

**Key Components**:

1. **ASpaceStation** - Main station actor
   - Spawns modules from `DefaultModuleClasses` array
   - Provides query functions: `HasMarketplace()`, `GetMarketplaceModule()`
   - Manages power, integrity, and module lifecycle

2. **AMarketplaceModule** - Trading facility
   - Attached to station as child actor
   - References `UMarketDataAsset` for inventory/pricing
   - Can be opened/closed (`bIsOpen` property)
   - Multiple marketplaces per station supported

3. **UMarketDataAsset** - Market configuration
   - Defines what items are sold/bought
   - Sets pricing rules (markup/markdown)
   - Manages stock levels, supply/demand
   - Reusable across multiple stations

4. **UTradeItemDataAsset** - Item definition
   - Base price, cargo volume, volatility
   - Category, rarity, description
   - Legal status, faction restrictions

#### Docking System Architecture

Docking has been **refactored to C++** for reliability:

```
ASpaceship (Player Ship)
├─ Docking Logic (C++ in ASpaceship class)
│   ├─ DetectNearbyStations()
│   ├─ RequestDocking()
│   ├─ PerformDockingSequence()
│   └─ Undock()
├─ UCargoComponent (Inventory)
└─ UPlayerTraderComponent (Trading operations)

ASpaceStation
└─ ADockingBayModule (Docking points)
    └─ DockingPoints (USceneComponent array)
```

**Docking Flow**:
1. Player flies within 2000 units of station
2. UI shows "Press F to Dock at [Station Name]"
3. Player presses F → `RequestDocking()` called
4. C++ finds nearest docking point
5. Ship smoothly moves to docking point (3-second curve)
6. Trading UI opens automatically
7. Player trades → Presses "Undock" button
8. Ship smoothly moves away from station

---

### Prerequisites

**Required for Trading**:
- [ ] Unreal Engine 5.6+
- [ ] Adastrea project compiled (C++ classes)
- [ ] At least one `UMarketDataAsset` created
- [ ] At least one `UTradeItemDataAsset` created
- [ ] `BP_SpaceStation` with marketplace module
- [ ] `WBP_TradingUI` widget created

**Required for Docking**:
- [ ] `BP_PlayerShip` with docking configured
- [ ] Input Action `IA_Dock` bound to F key
- [ ] `WBP_DockingPrompt` widget created
- [ ] Station with `ADockingBayModule`
- [ ] Docking curve asset (float curve 0→1, 3 seconds)

**Optional but Recommended**:
- [ ] `UEconomyManager` for dynamic pricing
- [ ] Multiple markets for price comparison
- [ ] `UCargoComponent` for inventory management
- [ ] Trading tutorial UI

---

## Part 2: Trading System

### Trading Core Concepts

**Buy Low, Sell High**:
The core gameplay loop is simple economic arbitrage:
- Find a station with cheap goods (producers)
- Buy goods with available credits
- Fly to station that needs those goods (importers)
- Sell for profit
- Use profits to upgrade cargo capacity

**Market Types**:
```cpp
enum class EMarketType : uint8
{
    OpenMarket,          // General trading (MVP default)
    BlackMarket,         // Illegal goods (post-MVP)
    CommodityExchange,   // Bulk trading (post-MVP)
    LuxuryBazaar,        // High-end goods (post-MVP)
    IndustrialDepot,     // Manufacturing supplies (post-MVP)
    ResearchHub,         // Tech and medicine (post-MVP)
    // ... more types
};
```

**For MVP**: Use `OpenMarket` type exclusively. Advanced types are post-MVP features.

**Market Sizes**:
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

**Pricing Formula**:
```
Player Buy Price = BasePrice × SellPriceMarkup × SupplyFactor × EventMultiplier
Player Sell Price = BasePrice × BuyPriceMarkdown × DemandFactor × EventMultiplier

Supply Factor = 1.0 + (Demand - Supply) / 100
```

**Example**:
```
Water (DA_Item_Water):
- Base Price: 10 credits
- Cargo Volume: 1 unit

Agricultural Station (produces water):
- Sell Markup: 1.1 (player pays 11 credits to buy)
- Buy Markdown: 0.9 (player gets 9 credits when selling)
- Supply: 2.0 (abundant) → Price slightly lower

Industrial Station (needs water):
- Sell Markup: 1.3 (player pays 13 credits to buy)
- Buy Markdown: 0.7 (player gets 7 credits when selling)
- Demand: 2.0 (high need) → Markdown offset by demand

Profit: Buy at 11 (Ag), sell at 10 (Industrial with demand factor) = -1 credit
Better route: Buy at 9 from player selling to Ag, sell to Industrial for 14
Actual profit: 5 credits per unit
```

---

### Market Data Assets

Market Data Assets (`UMarketDataAsset`) define the configuration for each marketplace.

#### Creating a Market Data Asset

**In Editor**:
1. Content Browser → Right-Click
2. Data Asset → MarketDataAsset
3. Name: `DA_Market_[StationType]` (e.g., `DA_Market_Agricultural`)

**Properties to Configure**:

```cpp
// Basic Info
Market Name: "Farming Collective Market"
Market Type: Open Market
Market Size: Medium

// Pricing Configuration
Sell Price Markup: 1.1    // Player pays 10% more when buying
Buy Price Markdown: 0.9    // Player gets 10% less when selling
Transaction Tax Rate: 0.05 // 5% tax on all transactions

// Trading Permissions
Allow Player Buying: true
Allow Player Selling: true
Allow AI Traders: true

// Inventory (add items)
Inventory:
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
      Supply Level: 0.2 (scarce)
      Demand Level: 2.0 (high demand - needs imports)

// Market Events (optional, post-MVP)
Active Events: (empty for MVP)
Random Event Chance: 0.0

// AI Configuration (optional, post-MVP)
AI Trader Count: 0
AI Trade Frequency: 0
```

#### Market Data Asset API

**Blueprint-Callable Functions**:

```cpp
// Get price for an item
UFUNCTION(BlueprintCallable, BlueprintPure)
float GetItemPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying) const;

// Check stock availability
UFUNCTION(BlueprintCallable, BlueprintPure)
bool IsItemInStock(FName ItemID, int32 Quantity) const;

// Get inventory entry
UFUNCTION(BlueprintCallable, BlueprintPure)
bool GetInventoryEntry(FName ItemID, FMarketInventoryEntry& OutEntry) const;

// Filter by category
UFUNCTION(BlueprintCallable)
TArray<FMarketInventoryEntry> GetItemsByCategory(ETradeItemCategory Category) const;

// Market events (post-MVP)
UFUNCTION(BlueprintCallable)
TArray<FMarketEvent> GetActiveEventsForItem(FName ItemID) const;
```

**Usage Example (Blueprint)**:
```
Get Market Data Asset (from marketplace module)
    ↓
For Each: Inventory Entry
    ↓
    Get Item Price (TradeItem, bIsBuying=True)
    ↓
    Add to UI List
```

---

### Trade Items

Trade Items (`UTradeItemDataAsset`) define individual goods that can be bought/sold.

#### Creating a Trade Item

**In Editor**:
1. Content Browser → Right-Click
2. Data Asset → TradeItemDataAsset
3. Name: `DA_Item_[ItemName]` (e.g., `DA_Item_Water`)

**Properties**:

```cpp
// Basic Info
Item Name: "Water"
Description: "Purified water essential for life support systems"
Item ID: "water" (unique identifier)

// Economic Properties
Base Price: 10.0 (credits per unit)
Cargo Volume: 1 (units of space)
Volatility: 0.3 (price fluctuation factor, 0.0-1.0)

// Classification
Category: BasicGoods
Rarity: Common
Legal Status: Legal

// Gameplay
Icon: T_Icon_Water (texture)
3D Model: SM_Cargo_Water (static mesh, optional)
```

**Trade Item Categories**:
```cpp
enum class ETradeItemCategory : uint8
{
    BasicGoods,    // Food, water, fuel
    Luxury,        // Art, entertainment, rare foods
    Industrial,    // Metals, components, machinery
    Medical,       // Medicine, equipment
    Technology,    // Electronics, software, computers
    Illegal,       // Contraband (post-MVP)
    Specialty      // Mission-specific items
};
```

**MVP Item Set** (Recommended):

| Category | Items | Base Prices |
|----------|-------|-------------|
| Basic Goods | Water (10), Food (25), Fuel (50) | Low volatility |
| Luxury | Fine Wine (200), Art (1000), Entertainment (500) | High volatility |
| Industrial | Metals (100), Components (250), Machinery (500) | Medium volatility |
| Medical | Medicine (300), Med Equipment (800) | Low volatility |
| Technology | Electronics (500), Software (1000), AI Cores (5000) | High volatility |

**Total**: 15-20 items recommended for MVP.

---

### Marketplace Modules

Marketplace Modules (`AMarketplaceModule`) are the physical trading facilities attached to stations.

#### Architecture

```cpp
class AMarketplaceModule : public ASpaceStationModule
{
    // Market configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMarketDataAsset* MarketDataAsset;
    
    // Trading availability
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOpen;
    
    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText MarketplaceName;
    
    // Module properties (inherited)
    // ModuleType = "Marketplace"
    // ModulePower = 40.0f (consumption)
    // ModuleGroup = Public
};
```

#### Adding Marketplace to Station

**Method 1: Class Defaults (Recommended for MVP)**:

1. Open `BP_SpaceStation` (or your station Blueprint)
2. Click "Class Defaults" button
3. Find "Station → Configuration"
4. Expand "Default Module Classes" array
5. Add entries:
   ```
   [0] BP_SpaceStationModule_DockingBay
   [1] BP_SpaceStationModule_Marketplace
   [2] BP_CargoBayModule (optional)
   ```
6. Compile and save

7. Open `BP_SpaceStationModule_Marketplace` Class Defaults
8. Set properties:
   ```
   Market Data Asset: DA_Market_YourStation
   Is Open: true
   Marketplace Name: "Central Market"
   ```
9. Compile and save

**What Happens**:
- Station's `BeginPlay()` spawns all modules from array
- Modules automatically attach to station
- Marketplace becomes accessible for trading

**Method 2: Runtime (Post-MVP)**:

```cpp
// Spawn marketplace module at runtime
AMarketplaceModule* NewMarketplace = GetWorld()->SpawnActor<AMarketplaceModule>(
    BP_SpaceStationModule_Marketplace,
    Station->GetActorLocation(),
    FRotator::ZeroRotator
);

// Configure
NewMarketplace->MarketDataAsset = DA_Market_Something;
NewMarketplace->bIsOpen = true;
NewMarketplace->MarketplaceName = FText::FromString("New Market");

// Add to station
Station->AddModule(NewMarketplace);
```

#### Querying Marketplace

**Check if Station Has Trading**:

```cpp
// C++ approach
if (Station->HasMarketplace())
{
    // Station has at least one marketplace
}

// Blueprint approach
Branch: Station → Has Marketplace?
├─ True → Continue
└─ False → Show "No Trading Available"
```

**Get Marketplace Module**:

```cpp
// Get first marketplace
AMarketplaceModule* Marketplace = Station->GetMarketplaceModule();

// Get all marketplaces (if multiple)
TArray<AMarketplaceModule*> Markets = Station->GetMarketplaceModules();

// Check availability
if (Marketplace && Marketplace->IsAvailableForTrading())
{
    // Open trading UI
}
```

**Complete Trading Check (Blueprint)**:

```
Event: Player Requests Trade
    ↓
Get Docked Station Reference
    ↓
Branch: Has Marketplace?
    ├─ False → Show UI: "No Marketplace"
    │
    └─ True:
        ↓
        Get Marketplace Module
        ↓
        Branch: Is Available For Trading?
        ├─ False → Show UI: "Market Closed"
        │   (bIsOpen = false or MarketDataAsset = null)
        │
        └─ True:
            ↓
            Get Market Data
            ↓
            Open Widget: WBP_TradingUI
            ↓
            Initialize UI with Market Data
```

---

### Trading UI

(Trading UI section continues with complete implementation guide...)

