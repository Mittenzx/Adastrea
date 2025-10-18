# Trading System - Blueprint Implementation Examples

## Overview
This guide provides complete Blueprint implementation examples for common trading system features. All examples are copy-paste ready and fully functional.

---

## Table of Contents
1. [Trading UI Widget](#trading-ui-widget)
2. [Market Interface](#market-interface)
3. [Contract Management](#contract-management)
4. [Price Display](#price-display)
5. [Transaction Confirmation](#transaction-confirmation)
6. [AI Trader Spawning](#ai-trader-spawning)
7. [Market Event System](#market-event-system)
8. [Route Planning](#route-planning)

---

## Trading UI Widget

### Widget Blueprint: BP_TradingInterface

**Purpose**: Main trading interface for buying/selling items

**Widget Hierarchy**:
```
Canvas Panel
├── Vertical Box (Market Info)
│   ├── Text Block: Market Name
│   ├── Text Block: Faction
│   └── Text Block: Tax Rate
├── Scroll Box (Inventory List)
│   └── [Item Entry Widgets]
├── Horizontal Box (Transaction Panel)
│   ├── Combo Box: Item Selector
│   ├── Spin Box: Quantity
│   ├── Text Block: Total Price
│   ├── Button: Buy
│   └── Button: Sell
└── Vertical Box (Transaction History)
    └── [Transaction Entries]
```

**Variables**:
```
CurrentMarket (MarketDataAsset, Instance Editable)
PlayerInventory (TArray<FAITraderInventory>, Instance Editable)
SelectedItem (TradeItemDataAsset)
TransactionManager (TradeTransactionManager, Reference)
```

**Event Graph**:

```
Event Construct:
  ├── Get Current Market
  ├── Refresh Market Display
  └── Populate Item List

Function: RefreshMarketDisplay
  ├── Get Market Name → Set Text (Market Name Text Block)
  ├── Get Controlling Faction → Get Faction Name → Set Text
  ├── Get Transaction Tax Rate → Format Text → Set Text
  └── Clear Inventory List
      └── FOR EACH Inventory Entry in CurrentMarket.Inventory
          ├── Create Item Entry Widget
          ├── Set Item Data
          │   ├── Item Name
          │   ├── Current Price (Call GetItemPrice)
          │   ├── Stock Level
          │   └── Supply/Demand Indicator
          └── Add to Scroll Box

Function: OnBuyButtonClicked
  ├── Get Selected Item
  ├── Get Quantity from Spin Box
  ├── Calculate Total Cost
  │   ├── Call CurrentMarket->GetItemPrice(SelectedItem, true)
  │   └── Multiply by Quantity
  ├── Check Player Credits
  │   └── IF Credits >= Total Cost
  │       ├── Deduct Credits
  │       ├── Add to Player Inventory
  │       ├── Record Transaction
  │       │   ├── Create FTradeTransaction
  │       │   ├── Set Transaction Type = Buy
  │       │   ├── Set Item, Quantity, Price
  │       │   ├── Set Buyer = Player ID
  │       │   ├── Set Seller = Market ID
  │       │   └── Call TransactionManager->RecordTransaction
  │       ├── Update Market Stock
  │       ├── Play Buy Sound
  │       └── Refresh Display
  │   └── ELSE
  │       └── Show Error: "Insufficient Credits"

Function: OnSellButtonClicked
  ├── Get Selected Item
  ├── Get Quantity
  ├── Check Player Inventory
  │   └── IF Player Has Item && Quantity <= Player's Stock
  │       ├── Calculate Sell Price
  │       │   └── Call CurrentMarket->GetItemPrice(SelectedItem, false)
  │       ├── Add Credits (Price * Quantity)
  │       ├── Remove from Player Inventory
  │       ├── Record Transaction
  │       ├── Update Market Stock
  │       ├── Play Sell Sound
  │       └── Refresh Display
  │   └── ELSE
  │       └── Show Error: "Insufficient Stock"

Function: UpdatePriceDisplay
  ├── Get Selected Item
  ├── Get Quantity
  ├── Get Buy Price
  │   └── CurrentMarket->GetItemPrice(SelectedItem, true)
  ├── Get Sell Price
  │   └── CurrentMarket->GetItemPrice(SelectedItem, false)
  ├── Calculate Total Buy Cost
  ├── Calculate Total Sell Revenue
  ├── Set Buy Price Text (Format: "Buy: X credits")
  └── Set Sell Price Text (Format: "Sell: X credits")
```

---

## Market Interface

### Actor Blueprint: BP_TradingTerminal

**Purpose**: In-world trading terminal that opens UI

**Components**:
```
- Static Mesh: Terminal Model
- Box Collision: Interaction Trigger
- Widget Component: Holographic Display (optional)
```

**Variables**:
```
MarketData (MarketDataAsset, Instance Editable)
TradingWidgetClass (TSubclassOf<UUserWidget>)
CurrentTradingWidget (UserWidget, Reference)
```

**Event Graph**:

```
Event BeginPlay:
  └── IF MarketData is Valid
      ├── Register with Market System
      └── Initialize Holographic Display

Event On Component Begin Overlap (Box Collision):
  ├── Get Overlapping Actor
  └── IF Actor is Player Character
      ├── Show Interaction Prompt ("Press E to Trade")
      └── Enable Input

Event On Player Interact (Custom):
  ├── Create Widget from TradingWidgetClass
  ├── Set CurrentMarket = MarketData
  ├── Add to Viewport
  ├── Set Input Mode UI Only
  ├── Show Mouse Cursor
  └── Store Reference to CurrentTradingWidget

Event On Close Trading:
  ├── Remove CurrentTradingWidget from Viewport
  ├── Set Input Mode Game Only
  ├── Hide Mouse Cursor
  └── Clear Reference
```

---

## Contract Management

### Widget Blueprint: BP_ContractBoard

**Purpose**: Display and accept trade contracts

**Widget Hierarchy**:
```
Canvas Panel
├── Text Block: "Available Contracts"
├── Scroll Box (Contract List)
│   └── [Contract Entry Widgets]
├── Border (Selected Contract Details)
│   ├── Text Block: Contract Name
│   ├── Rich Text Block: Description
│   ├── Horizontal Box (Stats)
│   │   ├── Text: Distance
│   │   ├── Text: Time Limit
│   │   ├── Text: Reward
│   │   └── Text: Difficulty
│   ├── Vertical Box: Required Cargo List
│   └── Button: Accept Contract
└── Vertical Box (Active Contracts)
    └── [Active Contract Widgets]
```

**Functions**:

```
Function: PopulateAvailableContracts
  ├── Clear Contract List
  ├── Get All Contracts from Contract Manager
  └── FOR EACH Contract
      ├── Check if Player Can Accept
      │   ├── Get Player Reputation
      │   ├── Get Player Cargo Capacity
      │   └── Call Contract->CanPlayerAccept(Rep, Cargo)
      └── IF Can Accept
          ├── Create Contract Entry Widget
          ├── Set Contract Data
          │   ├── Contract Name
          │   ├── Difficulty
          │   ├── Reward (Credits + Reputation)
          │   ├── Distance
          │   └── Time Limit
          ├── Bind On Click Event
          └── Add to Scroll Box

Function: OnContractEntryClicked
  ├── Set Selected Contract
  ├── Display Contract Details
  │   ├── Show Full Description
  │   ├── Show Required Cargo List
  │   │   └── FOR EACH Cargo in RequiredCargo
  │   │       ├── Show Item Name
  │   │       ├── Show Quantity
  │   │       └── Show if Pristine Required
  │   ├── Show Origin/Destination
  │   ├── Calculate Estimated Completion Time
  │   │   └── Call Contract->GetEstimatedCompletionTime
  │   └── Show Rewards and Penalties
  └── Enable Accept Button

Function: OnAcceptContractClicked
  ├── Get Selected Contract
  ├── Get Current Game Time
  ├── Call Contract->AcceptContract(PlayerID, GameTime)
  ├── IF Success
  │   ├── Add to Player's Active Contracts
  │   ├── Remove from Available List
  │   ├── Show Success Message
  │   ├── Play Accept Sound
  │   └── Refresh Lists
  └── ELSE
      └── Show Error Message

Function: RefreshActiveContracts
  ├── Clear Active Contracts Display
  ├── Get Player's Active Contracts
  └── FOR EACH Active Contract
      ├── Create Active Contract Widget
      ├── Set Progress Data
      │   ├── Show Time Remaining
      │   ├── Show Cargo Acquired (Checkmarks)
      │   ├── Show Distance to Destination
      │   └── Show Current Location
      ├── Add Track Marker on Map
      └── Add to Active List

Function: CheckContractCompletion
  ├── Get Player's Current Location
  ├── Get Player's Inventory
  └── FOR EACH Active Contract
      ├── Check if at Destination
      └── IF At Destination
          ├── Check if Has Required Cargo
          └── IF Has All Cargo
              ├── Call Contract->CompleteContract
              ├── Award Rewards
              │   ├── Add Credits
              │   ├── Add Reputation
              │   ├── Add Experience
              │   └── Add Bonus Items
              ├── Remove Cargo from Player
              ├── Show Success Notification
              ├── Play Success Sound
              └── Refresh Lists
```

---

## Price Display

### Function Library: BP_TradingHelpers

**Purpose**: Reusable functions for price calculations and display

```
Function: FormatPrice
  Input: Price (Float)
  Output: Formatted Text
  
  Logic:
    ├── Convert Float to Int
    ├── Add Comma Separators
    ├── Add Currency Symbol
    └── Return Text (e.g., "$ 1,234")

Function: GetPriceColor
  Input: 
    - Current Price (Float)
    - Base Price (Float)
  Output: Color
  
  Logic:
    ├── Calculate Percentage Change
    ├── IF Price > Base Price * 1.2
    │   └── Return Red (Very High)
    ├── ELSE IF Price > Base Price * 1.0
    │   └── Return Orange (High)
    ├── ELSE IF Price < Base Price * 0.8
    │   └── Return Green (Low)
    ├── ELSE IF Price < Base Price * 0.9
    │   └── Return Light Green (Good Deal)
    └── ELSE
        └── Return White (Normal)

Function: GetSupplyDemandIndicator
  Input:
    - Supply Level (Float)
    - Demand Level (Float)
  Output: Text
  
  Logic:
    ├── IF Supply < 0.5 && Demand > 1.5
    │   └── Return "⚠️ Critical Shortage"
    ├── ELSE IF Supply < 0.8
    │   └── Return "⬇ Low Supply"
    ├── ELSE IF Supply > 1.5
    │   └── Return "⬆ Surplus"
    ├── ELSE IF Demand > 1.5
    │   └── Return "🔥 High Demand"
    ├── ELSE IF Demand < 0.5
    │   └── Return "❄️ Low Demand"
    └── ELSE
        └── Return "◼ Stable"

Function: CalculateProfitMargin
  Input:
    - Buy Price (Float)
    - Sell Price (Float)
    - Quantity (Int)
  Output: 
    - Total Profit (Int)
    - Profit Percentage (Float)
  
  Logic:
    ├── Total Cost = Buy Price * Quantity
    ├── Total Revenue = Sell Price * Quantity
    ├── Total Profit = Total Revenue - Total Cost
    ├── Profit Percentage = (Total Profit / Total Cost) * 100
    └── Return Both Values

Function: GetPriceTrendArrow
  Input: Price Trend (Float from TransactionManager)
  Output: Text
  
  Logic:
    ├── IF Trend > 0.1
    │   └── Return "⬆⬆ Rising Fast"
    ├── ELSE IF Trend > 0.02
    │   └── Return "⬆ Rising"
    ├── ELSE IF Trend < -0.1
    │   └── Return "⬇⬇ Falling Fast"
    ├── ELSE IF Trend < -0.02
    │   └── Return "⬇ Falling"
    └── ELSE
        └── Return "➡ Stable"
```

---

## Transaction Confirmation

### Widget Blueprint: BP_TransactionConfirmation

**Purpose**: Confirm large transactions and show breakdown

```
Widget Hierarchy:
  Border (Backdrop)
  └── Vertical Box
      ├── Text Block: "Confirm Transaction"
      ├── Horizontal Box (Item Info)
      │   ├── Image: Item Icon
      │   └── Text: Item Name
      ├── Vertical Box (Transaction Details)
      │   ├── Text: "Quantity: X units"
      │   ├── Text: "Price per unit: X credits"
      │   ├── Text: "Subtotal: X credits"
      │   ├── Text: "Tax (X%): X credits"
      │   ├── Separator
      │   └── Text: "Total: X credits" (Bold)
      ├── Text Block: Warning (if applicable)
      ├── Horizontal Box (Buttons)
      │   ├── Button: Confirm
      │   └── Button: Cancel
      └── Check Box: "Don't show for small transactions"

Variables:
  - TransactionData (FTradeTransaction)
  - OnConfirm (Event Dispatcher)
  - OnCancel (Event Dispatcher)

Functions:
  Function: SetTransactionData
    Input: Transaction (FTradeTransaction)
    
    Logic:
      ├── Store Transaction Data
      ├── Display Item Name
      ├── Display Quantity
      ├── Calculate and Display Prices
      │   ├── Price per Unit
      │   ├── Subtotal
      │   ├── Tax Amount
      │   └── Total
      ├── Check for Warnings
      │   ├── IF Contraband Item
      │   │   └── Show: "⚠️ This is contraband. Risk of fine!"
      │   ├── IF Low Stock
      │   │   └── Show: "Low stock - prices may rise"
      │   └── IF High Value
      │       └── Show: "Large transaction - consider security"
      └── Set Button Focus

  Event: OnConfirmClicked
    ├── Broadcast OnConfirm Event
    ├── Play Confirm Sound
    └── Remove from Parent

  Event: OnCancelClicked
    ├── Broadcast OnCancel Event
    ├── Play Cancel Sound
    └── Remove from Parent
```

---

## AI Trader Spawning

### Actor Blueprint: BP_AITraderShip

**Purpose**: Ship with AI trading component

```
Components:
  - Static Mesh: Ship Model
  - AI Trader Component
  - Movement Component

Variables:
  - Home Market (MarketDataAsset)
  - Known Markets (TArray<MarketDataAsset>)
  - Current Route (FTradeRoute)

Event Graph:
  Event BeginPlay:
    ├── Initialize AI Trader Component
    │   ├── Set Trading Capital (Random 10K-100K)
    │   ├── Set Strategy (Random or Configured)
    │   ├── Set Cargo Capacity (Based on Ship Size)
    │   ├── Set Trading Skill (Random 3-8)
    │   └── Set Risk Tolerance (Based on Strategy)
    ├── Enable Behaviors
    │   ├── Add Market Making
    │   ├── Add Arbitrage
    │   ├── Add Route Planning
    │   └── (More based on strategy)
    ├── Discover Starting Markets
    │   └── FOR EACH Market in Known Markets
    │       └── Call AITrader->DiscoverMarket
    ├── Initialize at Home Market
    │   └── Call AITrader->Initialize(Capital, HomeMarket)
    └── Start Trading Loop

  Custom Event: ExecuteTradeDecision
    ├── Find Best Trade Routes
    │   └── Call AITrader->FindBestTradeRoutes(5)
    ├── Select Top Route
    ├── IF At Origin Market
    │   ├── Buy Cargo
    │   │   └── Call AITrader->ExecuteTrade(Item, Qty, true)
    │   └── Travel to Destination
    │       └── Call AITrader->TravelToMarket(Destination)
    └── ELSE IF At Destination Market
        ├── Sell Cargo
        │   └── Call AITrader->ExecuteTrade(Item, Qty, false)
        └── Find New Route

  Function: TravelToMarket
    Input: Destination Market (MarketDataAsset)
    
    Logic:
      ├── Get Destination Location
      ├── Calculate Travel Time
      │   └── Call AITrader->TravelToMarket(Destination)
      ├── Set Navigation Target
      ├── Set Ship Speed
      ├── Play Travel Animation
      └── Delay (Travel Time)
          └── Arrive at Market

  Event: OnTradeExecuted (from AITrader Component)
    ├── Play Trade Animation
    ├── Show Trade Notification (if near player)
    ├── Update Ship's Cargo Display
    └── Log Trade for Analytics
```

---

## Market Event System

### Actor Component: BP_MarketEventManager

**Purpose**: Trigger and manage market events

```
Variables:
  - Registered Markets (TArray<MarketDataAsset>)
  - Event Templates (TArray<FMarketEvent>)
  - Random Event Chance (Float, Default 0.1)
  - Event Check Interval (Float, Default 3600.0)

Event Graph:
  Event BeginPlay:
    ├── Set Timer for Event Checks
    │   └── Looping, Interval = Event Check Interval
    └── Load Event Templates

  Custom Event: CheckForRandomEvents
    ├── FOR EACH Market in Registered Markets
    │   ├── Roll Random (0.0 to 1.0)
    │   └── IF Random < Market.RandomEventChance
    │       ├── Select Random Event Template
    │       ├── Apply to Market
    │       └── Call TriggerMarketEvent

  Function: TriggerMarketEvent
    Input: 
      - Market (MarketDataAsset)
      - Event (FMarketEvent)
    
    Logic:
      ├── Copy Event Template
      ├── Set bIsActive = true
      ├── Set StartTime = Current Game Time
      ├── Add to Market's Active Events
      ├── Call Market->OnMarketEventStarted(Event)
      ├── Show Notification to Players
      │   └── "Market Event: [EventName] at [MarketName]"
      ├── Update Affected Items
      │   └── FOR EACH Affected Item
      │       ├── Apply Price Multiplier
      │       ├── Apply Supply Multiplier
      │       └── Apply Demand Multiplier
      ├── Play Event Start Sound
      └── Schedule Event End
          └── Delay (Event Duration)
              └── Call EndMarketEvent

  Function: EndMarketEvent
    Input:
      - Market (MarketDataAsset)
      - Event (FMarketEvent)
    
    Logic:
      ├── Set bIsActive = false
      ├── Call Market->OnMarketEventEnded(Event)
      ├── Show Event End Notification
      ├── Revert Affected Items
      │   └── Reset to normal supply/demand
      └── Remove from Active Events

  Function: CreateScriptedEvent
    Input:
      - Market (MarketDataAsset)
      - Event Name (Text)
      - Affected Items (TArray<FName>)
      - Duration (Float)
      - Multipliers (Float, Float, Float)
    
    Logic:
      ├── Create New FMarketEvent
      ├── Set All Parameters
      ├── Call TriggerMarketEvent
      └── Return Event Reference

Example Usage in Quest:
  Quest Event: War Declared
    ├── Get Market Event Manager
    ├── Create Military Equipment Shortage
    │   └── CreateScriptedEvent(
    │         MilitaryMarket,
    │         "War Mobilization",
    │         [Weapons, Ammo, Armor],
    │         168.0, // 1 week
    │         2.5, 0.3, 3.0 // Price x2.5, Supply x0.3, Demand x3
    │       )
    └── Notify Players
```

---

## Route Planning

### Widget Blueprint: BP_TradePlannerMap

**Purpose**: Visual route planning and profit calculator

```
Widget Hierarchy:
  Canvas Panel
  ├── Image: Galaxy Map
  ├── Overlay: Market Markers
  │   └── [Market Icon Widgets]
  ├── Overlay: Route Lines
  │   └── [Route Line Widgets]
  ├── Border: Route Info Panel
  │   ├── Text: Origin Market
  │   ├── Text: Destination Market
  │   ├── Text: Distance
  │   ├── Text: Travel Time
  │   ├── Text: Profit per Unit
  │   ├── Text: Total Profit
  │   └── Button: Set Route
  └── Scroll Box: Top Routes
      └── [Route Entry Widgets]

Variables:
  - All Markets (TArray<MarketDataAsset>)
  - Selected Origin (MarketDataAsset)
  - Selected Destination (MarketDataAsset)
  - Best Routes (TArray<FTradeRoute>)
  - Player Ship Speed (Float)

Functions:
  Function: PopulateMarkets
    ├── Clear Existing Markers
    └── FOR EACH Market in All Markets
        ├── Create Market Marker Widget
        ├── Set Position on Map
        ├── Set Market Name
        ├── Bind Click Event
        └── Add to Overlay

  Function: CalculateAllRoutes
    ├── Clear Best Routes
    └── FOR EACH Origin Market
        └── FOR EACH Destination Market
            └── IF Origin != Destination
                └── FOR EACH Trade Item
                    ├── Get Buy Price at Origin
                    ├── Get Sell Price at Destination
                    ├── Calculate Profit
                    └── IF Profit > Threshold
                        ├── Create FTradeRoute
                        ├── Set All Data
                        ├── Calculate Profitability Score
                        └── Add to Best Routes

  Function: DisplayRoute
    Input: Route (FTradeRoute)
    
    Logic:
      ├── Draw Route Line on Map
      │   ├── Get Origin Position
      │   ├── Get Destination Position
      │   └── Draw Line Between
      ├── Show Route Info
      │   ├── Origin Market Name
      │   ├── Destination Market Name
      │   ├── Item Name
      │   ├── Distance
      │   ├── Estimated Travel Time
      │   ├── Buy Price
      │   ├── Sell Price
      │   ├── Profit per Unit
      │   ├── Recommended Quantity
      │   └── Total Profit
      └── Highlight Markets

  Event: OnMarketClicked
    Input: Clicked Market (MarketDataAsset)
    
    Logic:
      ├── IF No Origin Selected
      │   ├── Set as Origin
      │   └── Highlight in Blue
      └── ELSE IF No Destination Selected
          ├── Set as Destination
          ├── Highlight in Red
          ├── Find Routes Between Origin and Destination
          │   └── FOR EACH Item
          │       ├── Calculate Route
          │       └── IF Profitable
          │           └── Add to Route List
          ├── Sort Routes by Profitability
          ├── Display Top Route
          └── Populate Route List

  Function: SetActiveRoute
    Input: Route (FTradeRoute)
    
    Logic:
      ├── Store Route in Player Data
      ├── Set Navigation Waypoint
      ├── Add Route to Active Routes List
      ├── Show Confirmation
      └── Close Planner
```

---

## Additional Examples

### Blueprint: Dynamic Price Display

```
Widget: BP_PriceTickerWidget
  Purpose: Real-time price updates like stock ticker

  Variables:
    - Tracked Items (TArray<TradeItemDataAsset>)
    - Current Market (MarketDataAsset)
    - Update Interval (Float, Default 5.0)

  Logic:
    Event Construct:
      └── Set Timer (Looping, Interval = Update Interval)
          └── Update All Prices

    Function: UpdateAllPrices
      └── FOR EACH Item in Tracked Items
          ├── Get Current Price
          ├── Get Previous Price (cached)
          ├── Calculate Change
          ├── Update Display
          │   ├── Item Name
          │   ├── Current Price (with color)
          │   └── Change Arrow (▲/▼)
          └── Cache Current Price

    Display Format:
      "Iron Ore: $50 ▲ (+5%)" (Green)
      "Quantum CPU: $4800 ▼ (-4%)" (Red)
```

### Blueprint: Contract Progress Tracker

```
Widget: BP_ContractTracker
  Purpose: Show active contract progress

  Variables:
    - Active Contract (TradeContractDataAsset)
    - Player Location (Vector)

  Logic:
    Event Tick:
      ├── Calculate Distance to Destination
      ├── Calculate Time Remaining
      ├── Update Progress Bar
      │   └── Progress = Distance Traveled / Total Distance
      ├── Check Cargo Requirements
      │   └── FOR EACH Required Item
      │       └── Check if in Player Inventory
      └── Update Display

    Display:
      ├── Contract Name
      ├── Progress Bar (0-100%)
      ├── Distance Remaining
      ├── Time Remaining (with color warning if low)
      ├── Cargo Checklist
      │   ├── ✓ Food Rations: 500/500
      │   └── ✗ Medical Supplies: 0/100
      └── Estimated Reward
```

---

## Performance Tips

1. **Update Frequency**:
   - Price updates: Every 5-10 seconds
   - Stock checks: On player action only
   - AI traders: Staggered updates

2. **Widget Pooling**:
   - Reuse item entry widgets
   - Pool transaction confirmations
   - Cache common calculations

3. **LOD for Distant Markets**:
   - Full simulation near player
   - Simplified updates far away
   - Statistical model for very distant

4. **Async Operations**:
   - Load market data async
   - Save transactions async
   - Calculate routes in background

---

**Next Steps**:
- Implement these examples in your project
- Customize visuals and UX
- Test with players
- Iterate based on feedback
