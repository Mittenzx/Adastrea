# Trading System - Blueprint Integration Guide

## Overview
This guide shows how to integrate the C++ trading system into your Blueprint project. All examples are designed to be implemented directly in the Unreal Editor.

---

## Quick Start: 5-Minute Trading Setup

### Step 1: Create Your First Trade Item (2 minutes)

1. **In Content Browser**:
   - Navigate to `Content/Trading/Items/` (create folder)
   - Right-click → Miscellaneous → Data Asset
   - Select `TradeItemDataAsset`
   - Name: `DA_TradeItem_IronOre`

2. **Configure the item**:
   - Item Name: "Iron Ore"
   - Item ID: "TradeItem_IronOre"
   - Category: Raw Materials
   - Base Price: 50
   - Legal Status: Legal
   - Volume Per Unit: 1.0
   - Mass Per Unit: 2.5

3. **Save the asset**

### Step 2: Create Your First Market (2 minutes)

1. **In Content Browser**:
   - Navigate to `Content/Trading/Markets/`
   - Right-click → Miscellaneous → Data Asset
   - Select `MarketDataAsset`
   - Name: `DA_Market_TestStation`

2. **Configure the market**:
   - Market Name: "Test Trading Station"
   - Market ID: "Market_TestStation"
   - Market Type: Open Market
   - Transaction Tax Rate: 0.05
   - Sell Price Markup: 1.15
   - Buy Price Markdown: 0.85

3. **Add Inventory**:
   - Click "+" on Inventory array
   - Set Trade Item: `DA_TradeItem_IronOre`
   - Current Stock: 1000
   - Max Stock: 5000
   - Supply Level: 1.0
   - Demand Level: 1.0
   - In Stock: ✓

4. **Save the asset**

### Step 3: Create Trading UI Widget (1 minute)

1. **In Content Browser**:
   - Navigate to `Content/UI/Trading/`
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_SimpleTradingUI`

2. **Add basic widgets**:
   - Canvas Panel (root)
   - Text Block: Market name
   - Button: "Buy Iron Ore"
   - Button: "Sell Iron Ore"
   - Text Block: Price display

3. **Add variables**:
   - `CurrentMarket` (MarketDataAsset, Instance Editable)
   - `IronOreItem` (TradeItemDataAsset, Instance Editable)

4. **Implement Buy button**:
```
Event: OnBuyButtonClicked
  ├── Get Item Price
  │   └── Call: CurrentMarket->GetItemPrice(IronOreItem, true)
  ├── Print String (show price)
  └── TODO: Implement actual purchase logic
```

**You now have a working trading system foundation!**

---

## Blueprint Widget Examples

### Example 1: Price Display Widget

**Widget**: `WBP_ItemPriceDisplay`

**Variables**:
```
TradeItem (TradeItemDataAsset, Instance Editable)
CurrentMarket (MarketDataAsset, Instance Editable)
```

**Widget Hierarchy**:
```
Horizontal Box
├── Image: Item Icon
├── Text Block: Item Name
├── Text Block: Price (Colored)
└── Text Block: Supply/Demand Indicator
```

**Event Graph**:
```
Event Construct:
  └── Update Price Display

Custom Event: Update Price Display
  ├── Get Current Price
  │   └── CurrentMarket->GetItemPrice(TradeItem, true)
  ├── Set Price Text
  │   └── Format: "$X.XX"
  ├── Set Price Color
  │   └── If Price > Base Price * 1.2: Red
  │   └── Else If Price < Base Price * 0.8: Green
  │   └── Else: White
  ├── Get Inventory Entry
  │   └── CurrentMarket->GetInventoryEntry(TradeItem->ItemID, OutEntry)
  └── Set Supply/Demand Text
      └── If Supply < 0.8: "Low Stock"
      └── Else If Demand > 1.2: "High Demand"
      └── Else: "Normal"
```

### Example 2: Transaction Widget

**Widget**: `WBP_TransactionDialog`

**Purpose**: Confirm buy/sell transactions

**Variables**:
```
TransactionType (Text) // "Buy" or "Sell"
TradeItem (TradeItemDataAsset)
Quantity (Integer)
PricePerUnit (Float)
TotalPrice (Float)
OnConfirm (Event Dispatcher)
OnCancel (Event Dispatcher)
```

**Event Graph**:
```
Function: Setup Transaction
  Inputs: Item, Qty, Price, IsBuying
  
  ├── Set TransactionType
  │   └── If IsBuying: "Buy" Else: "Sell"
  ├── Set TradeItem = Item
  ├── Set Quantity = Qty
  ├── Set PricePerUnit = Price
  ├── Calculate Total
  │   └── TotalPrice = Price * Qty
  └── Update Display
      ├── Set Item Name Text
      ├── Set Quantity Text: "Quantity: X units"
      ├── Set Price Text: "$X.XX per unit"
      └── Set Total Text: "Total: $X,XXX.XX" (Bold)

Event: On Confirm Button Clicked
  ├── Broadcast OnConfirm
  └── Remove from Parent

Event: On Cancel Button Clicked
  ├── Broadcast OnCancel
  └── Remove from Parent
```

### Example 3: Market Interface Widget

**Widget**: `WBP_MarketInterface`

**Purpose**: Full trading interface

**Variables**:
```
CurrentMarket (MarketDataAsset)
PlayerCredits (Integer)
PlayerInventory (Map<TradeItemDataAsset, Integer>)
SelectedItem (TradeItemDataAsset)
```

**Functions**:

```
Function: Refresh Market Display
  ├── Clear Inventory List
  ├── Get Market Inventory
  │   └── CurrentMarket->Inventory
  └── For Each Entry in Inventory
      ├── Create Item Display Widget
      ├── Set Item Data
      │   ├── Item Name
      │   ├── Current Price
      │   ├── Stock Level
      │   └── Player's Quantity
      └── Add to Scroll Box

Function: Buy Item
  Inputs: Item, Quantity
  
  ├── Calculate Cost
  │   ├── Get Buy Price
  │   │   └── CurrentMarket->GetItemPrice(Item, true)
  │   └── Total Cost = Price * Quantity
  ├── Check Player Credits
  │   └── If Credits >= Total Cost
  │       ├── Deduct Credits
  │       ├── Add to Player Inventory
  │       ├── Play Buy Sound
  │       └── Refresh Display
  │   └── Else
  │       └── Show Error: "Insufficient Credits"

Function: Sell Item
  Inputs: Item, Quantity
  
  ├── Check Player Inventory
  │   └── If Player Has Item && Qty <= Stock
  │       ├── Get Sell Price
  │       │   └── CurrentMarket->GetItemPrice(Item, false)
  │       ├── Add Credits
  │       ├── Remove from Inventory
  │       ├── Play Sell Sound
  │       └── Refresh Display
  │   └── Else
  │       └── Show Error: "Insufficient Stock"
```

---

## Blueprint Actor Examples

### Example 1: Trading Terminal Actor

**Actor**: `BP_TradingTerminal`

**Purpose**: In-world trading access point

**Components**:
```
- Static Mesh (Terminal Model)
- Box Collision (Interaction Trigger)
- Widget Component (Optional holographic display)
```

**Variables**:
```
MarketData (MarketDataAsset, Instance Editable)
TradingUIClass (TSubclassOf<UserWidget>)
CurrentTradingUI (UserWidget, Reference)
```

**Event Graph**:
```
Event Begin Play:
  └── Initialize Terminal
      └── Set Holographic Display Content (if exists)

Event On Component Begin Overlap (Box Collision):
  ├── Cast to Player Character
  └── If Valid
      └── Show Prompt: "Press E to Trade"

Custom Event: Player Interact
  ├── Create Widget
  │   └── Create Widget from TradingUIClass
  ├── Set Market Data
  │   └── Widget->CurrentMarket = MarketData
  ├── Add to Viewport
  ├── Set Input Mode UI Only
  └── Show Mouse Cursor

Custom Event: Close Trading UI
  ├── Remove Widget
  ├── Set Input Mode Game Only
  └── Hide Mouse Cursor
```

### Example 2: AI Trader Ship

**Actor**: `BP_AITraderShip`

**Purpose**: Automated trading ship

**Components**:
```
- Static Mesh (Ship Model)
- AI Trader Component
- Movement Component
```

**Variables**:
```
HomeMarket (MarketDataAsset)
KnownMarkets (Array<MarketDataAsset>)
CurrentRoute (FTradeRoute)
TradingActive (Boolean)
```

**Event Graph**:
```
Event Begin Play:
  ├── Setup AI Trader
  │   ├── Get AI Trader Component
  │   ├── Set Strategy
  │   │   └── Random or: Conservative/Balanced/Aggressive
  │   ├── Set Capital
  │   │   └── Random: 10,000 - 100,000
  │   ├── Set Cargo Capacity
  │   │   └── Based on ship size
  │   └── Enable Behaviors
  │       ├── Add: Market Making
  │       ├── Add: Arbitrage
  │       └── Add: Route Planning
  ├── Discover Markets
  │   └── For Each in KnownMarkets
  │       └── AITrader->DiscoverMarket
  ├── Initialize
  │   └── AITrader->Initialize(Capital, HomeMarket)
  └── Start Trading Loop
      └── Set Timer: Execute Trade (Repeating, 30 seconds)

Custom Event: Execute Trade
  ├── Find Best Routes
  │   └── AITrader->FindBestTradeRoutes(5)
  ├── Select Route
  │   └── Get First Element
  ├── Check Current Location
  └── If At Origin
  │   ├── Buy Cargo
  │   │   └── AITrader->ExecuteTrade(Item, Qty, true)
  │   └── Travel to Destination
  │       └── Set Destination = Route->DestinationMarket
  └── Else If At Destination
      ├── Sell Cargo
      │   └── AITrader->ExecuteTrade(Item, Qty, false)
      └── Find New Route
```

### Example 3: Market Event Manager

**Actor Component**: `BP_MarketEventManager`

**Purpose**: Trigger and manage market events

**Variables**:
```
RegisteredMarkets (Array<MarketDataAsset>)
EventCheckInterval (Float) = 3600.0 (1 hour)
RandomEventChance (Float) = 0.1
```

**Event Graph**:
```
Event Begin Play:
  └── Set Timer: Check Random Events (Repeating, EventCheckInterval)

Custom Event: Check Random Events
  └── For Each Market in RegisteredMarkets
      ├── Random Float (0.0 to 1.0)
      └── If Random < Market->RandomEventChance
          ├── Select Random Event Type
          └── Trigger Event

Function: Trigger Market Event
  Inputs: Market, EventName, AffectedItems, Duration, Multipliers
  
  ├── Create Market Event Struct
  │   ├── Set Event Name
  │   ├── Set Affected Items
  │   ├── Set Price/Supply/Demand Multipliers
  │   ├── Set Duration
  │   └── Set bIsActive = true
  ├── Add to Market's Active Events
  ├── Show Notification
  │   └── "Market Event: [Name] at [Market]"
  └── Set Timer: End Event (Once, Duration)

Function: End Market Event
  Inputs: Market, Event
  
  ├── Set Event bIsActive = false
  ├── Remove from Active Events
  └── Show Notification
      └── "Event Ended: [Name]"
```

---

## Blueprint Function Library

**Blueprint**: `BPL_TradingHelpers`

**Purpose**: Reusable trading utility functions

```
Function: Format Price
  Input: Price (Float)
  Output: Text
  
  ├── Convert to Int
  ├── Convert to String
  ├── Add Thousands Separators
  ├── Prepend "$"
  └── Return Text

Function: Get Price Color
  Input: CurrentPrice (Float), BasePrice (Float)
  Output: Color
  
  ├── Calculate Ratio = CurrentPrice / BasePrice
  └── If Ratio > 1.2: Return Red
  └── Else If Ratio > 1.0: Return Orange
  └── Else If Ratio < 0.8: Return Green
  └── Else: Return White

Function: Calculate Profit
  Inputs: BuyPrice, SellPrice, Quantity
  Output: TotalProfit, ProfitPercentage
  
  ├── Cost = BuyPrice * Quantity
  ├── Revenue = SellPrice * Quantity
  ├── Profit = Revenue - Cost
  ├── Percentage = (Profit / Cost) * 100
  └── Return Both

Function: Get Supply Demand Text
  Inputs: Supply (Float), Demand (Float)
  Output: Text
  
  └── If Supply < 0.5 && Demand > 1.5: "Critical Shortage"
  └── Else If Supply < 0.8: "Low Supply"
  └── Else If Supply > 1.5: "Surplus"
  └── Else If Demand > 1.5: "High Demand"
  └── Else: "Stable"
```

---

## Integration Checklist

### Phase 1: Basic Trading
- [ ] Create 5-10 trade items
- [ ] Create 2-3 markets
- [ ] Create trading UI widget
- [ ] Create trading terminal actor
- [ ] Test buy/sell loop
- [ ] Verify pricing calculations

### Phase 2: Advanced Features
- [ ] Add AI trader ships
- [ ] Create contracts
- [ ] Implement contract board UI
- [ ] Add market events
- [ ] Create event manager
- [ ] Test AI trading

### Phase 3: Polish
- [ ] Add sound effects
- [ ] Add visual feedback
- [ ] Implement transaction history
- [ ] Add price trend displays
- [ ] Create route planning UI
- [ ] Balance economy

### Phase 4: Integration
- [ ] Connect to faction system
- [ ] Connect to ship system
- [ ] Connect to station system
- [ ] Add save/load support
- [ ] Test multiplayer (if applicable)
- [ ] Performance optimization

---

## Common Issues and Solutions

### Issue: Prices not updating
**Solution**: 
- Ensure `bAffectedBySupplyDemand` is true
- Check that market has inventory entry for item
- Verify supply/demand values are not both 1.0

### Issue: AI traders not trading
**Solution**:
- Check `bAllowAITraders` on market
- Verify AI has trading capital
- Ensure AI knows about markets (use DiscoverMarket)
- Check enabled behaviors include relevant types

### Issue: Contract not completing
**Solution**:
- Verify player is at destination location
- Check all required cargo is in inventory
- Ensure contract hasn't expired
- Call CompleteContract explicitly

### Issue: Market events not affecting prices
**Solution**:
- Verify event `bIsActive` is true
- Check event hasn't expired
- Ensure item ID matches exactly
- Confirm `bAffectedByMarketEvents` is true on item

---

## Performance Tips

1. **Update Frequency**: Don't update prices every frame
   - Use timers (5-10 second intervals)
   - Update on player action only

2. **Widget Pooling**: Reuse widgets instead of creating new
   - Pool item entry widgets
   - Cache expensive calculations

3. **AI Trader LOD**: Reduce complexity for distant traders
   - Full logic near player
   - Simplified logic far away
   - Statistical simulation very far

4. **Async Operations**: Use async for expensive operations
   - Route calculations
   - Transaction history queries
   - Save/load operations

---

## Next Steps

1. **Read Full Documentation**:
   - `Assets/TradingSystemGuide.md` - Complete reference
   - `Assets/TradingDesignerWorkflow.md` - Detailed workflows
   - `Assets/TradingAPIReference.md` - API documentation

2. **Review Examples**:
   - `Assets/TradingBlueprintExamples.md` - Blueprint patterns
   - `Assets/TradingTemplates/` - YAML templates

3. **Implement and Test**:
   - Start with basic trading
   - Add complexity gradually
   - Test and balance

4. **Iterate**:
   - Gather feedback
   - Adjust prices and rewards
   - Expand content

---

**Happy Trading!** 🚀💰

For questions or issues, refer to the main documentation in `Assets/TradingSystemGuide.md`
