# Blueprint Mockup Guide - Visual Documentation Best Practices

> **Best practices and templates for documenting Blueprint logic in Adastrea**

**Last Updated**: December 29, 2025  
**For Version**: 1.0.0-alpha  
**Target Engine**: Unreal Engine 5.6

---

## Table of Contents

1. [Overview](#overview)
2. [Visual Format Standards](#visual-format-standards)
3. [Common Node Patterns](#common-node-patterns)
4. [UI Widget Patterns](#ui-widget-patterns)
5. [Actor Blueprint Patterns](#actor-blueprint-patterns)
6. [Data Flow Patterns](#data-flow-patterns)
7. [TradingInterfaceWidget Examples](#tradinginterfacewidget-examples)
8. [Best Practices](#best-practices)
9. [Quick Reference Templates](#quick-reference-templates)

---

## Overview

### Why Visual Blueprint Documentation?

Blueprint visual documentation is **superior to text descriptions** because it:
- ✅ Shows exact node flow and connections
- ✅ Makes data flow immediately clear
- ✅ Easier to translate into actual Blueprint graphs
- ✅ Reduces ambiguity and misinterpretation
- ✅ Serves as both documentation AND implementation guide

### This Guide Provides

- **Standard notation** for Blueprint node graphs
- **Common patterns** used throughout Adastrea
- **Real-world examples** from existing systems
- **Copy-paste templates** for quick documentation

---

## Visual Format Standards

### Basic Node Representation

```
Event Node Name
  ↓
Function Node Name
  ├─ Input: Value
  └─ Return Value
     ↓
Next Node
```

### Node Types

#### 1. Event Nodes (Entry Points)
```
Event [Name]
  Inputs: (if any)
    - Parameter1 (Type)
    - Parameter2 (Type)
  ↓
```

#### 2. Function Nodes (Actions)
```
Function Name
  ├─ Input 1: Value
  ├─ Input 2: Value
  └─ Return: Result
```

#### 3. Branch Nodes (Decisions)
```
Branch (Condition)
  ├─ True → Path if true
  │    ↓
  │  [True branch nodes]
  │
  └─ False → Path if false
       ↓
     [False branch nodes]
```

#### 4. Sequence Nodes (Multiple Outputs)
```
Sequence
  ├─ Then 0 → First action
  ├─ Then 1 → Second action
  └─ Then 2 → Third action
```

#### 5. Loop Nodes
```
ForEach Loop
  ├─ Array: Collection
  └─ Loop Body
       ↓
     [Process each element]
       ↓
     Loop continues
  ↓
Completed
```

#### 6. Select/Switch Nodes
```
Select (Integer)
  ├─ 0 → Option A
  ├─ 1 → Option B
  ├─ 2 → Option C
  └─ Default → Fallback
```

### Connection Symbols

| Symbol | Meaning |
|--------|---------|
| `↓` | Execution flow downward |
| `→` | Execution flow right |
| `├─` | Branch split (first option) |
| `└─` | Branch split (last option) |
| `│` | Vertical continuation |
| `...` | Additional nodes not shown |

### Indentation Rules

- **2 spaces** per indentation level
- Align branches vertically
- Keep related nodes at same level
- Use blank lines between major sections

---

## Common Node Patterns

### Pattern 1: Simple Event Handler

**Use Case**: Handle button click, widget event

```
Event OnButtonClicked
  ↓
Play Sound: ButtonClickSound
  ↓
Show Message: "Button Pressed"
  ↓
Close Widget
```

### Pattern 2: Validated Action

**Use Case**: Perform action only if conditions met

```
Event OnBuyButtonClicked
  ↓
Branch (Has Enough Credits?)
  ├─ True → Continue Purchase
  │    ↓
  │  Deduct Credits
  │    ↓
  │  Add Item to Inventory
  │    ↓
  │  Play Success Sound
  │    ↓
  │  Refresh UI
  │
  └─ False → Show Error
       ↓
     Show Message: "Insufficient Credits"
       ↓
     Play Error Sound
```

### Pattern 3: Get-Calculate-Set

**Use Case**: Retrieve data, process, update display

```
Event RefreshDisplay
  ↓
Get Player Credits
  └─ Return: Credits (int32)
     ↓
Format Text: "{0} CR"
  ├─ Input 0: Credits
  └─ Return: Formatted Text
     ↓
Set Text (TextBlock_Credits)
  └─ Text: Formatted Text
```

### Pattern 4: Null Check Pattern

**Use Case**: Safely access object properties

```
Event UpdateShipInfo
  ↓
Get Player Ship
  └─ Return: Ship Reference
     ↓
Is Valid? (Ship Reference)
  ├─ True → Safe to Use
  │    ↓
  │  Get Ship Name
  │    ↓
  │  Update Display
  │
  └─ False → Handle Null
       ↓
     Set Text: "No Ship"
```

### Pattern 5: Callback Chain

**Use Case**: Sequential async operations

```
Event LoadGame
  ↓
Show Loading Screen
  ↓
Load Save Data (Async)
  ↓
[Wait for completion]
  ↓
On Load Complete
  ├─ Branch (Success?)
  │   ├─ True → Apply Loaded Data
  │   │    ↓
  │   │  Update Game State
  │   │    ↓
  │   │  Hide Loading Screen
  │   │
  │   └─ False → Show Error
  │        ↓
  │      Show Message: "Load Failed"
  │        ↓
  │      Return to Menu
```

### Pattern 6: Loop with Accumulator

**Use Case**: Sum values, find maximum, count items

```
Function CalculateTotalPrice
  Returns: int32
  ↓
Set Total = 0
  ↓
ForEach (Shopping Cart Items)
  ├─ Array: ShoppingCart
  └─ Loop Body
       ↓
     Get Item Price
       ├─ Item: Current Item
       └─ Return: Price
          ↓
     Get Item Quantity
       └─ Return: Quantity
          ↓
     Calculate Subtotal
       └─ Subtotal = Price × Quantity
          ↓
     Add to Total
       └─ Total = Total + Subtotal
          ↓
     [Loop continues]
  ↓
Completed
  ↓
Return Total
```

---

## UI Widget Patterns

### Pattern 1: List Population

**Use Case**: Fill ScrollBox or ListView with items

```
Event OnMarketInventoryUpdated
  ↓
Clear Children (ScrollBox_ItemList)
  ↓
Get Available Items
  └─ Return: Array<MarketInventoryEntry>
     ↓
ForEach Loop
  ├─ Array: Available Items
  └─ Loop Body
       ↓
     Create Widget
       ├─ Class: WBP_ItemListEntry
       └─ Return: Widget
          ↓
     Set Item Data
       ├─ Widget: Created Widget
       ├─ Item: Current Item
       └─ Price: Current Price
          ↓
     Add Child to ScrollBox
       ├─ Parent: ScrollBox_ItemList
       └─ Content: Created Widget
          ↓
     [Loop continues]
```

**Visual Result**:
```
ScrollBox
├── WBP_ItemListEntry (Iron Ore - 50 CR)
├── WBP_ItemListEntry (Gold - 500 CR)
├── WBP_ItemListEntry (Water - 10 CR)
└── ...
```

### Pattern 2: Dynamic Color Feedback

**Use Case**: Change color based on state (prices, health, etc.)

```
Function UpdatePriceDisplay
  Inputs:
    - Current Price (float)
    - Base Price (float)
  ↓
Calculate Price Ratio
  └─ Ratio = Current Price / Base Price
     ↓
Select Color
  ├─ Condition: Ratio > 1.2
  │   └─ Return: Red (High Price)
  ├─ Condition: Ratio < 0.8
  │   └─ Return: Green (Low Price)
  └─ Default
      └─ Return: White (Normal Price)
     ↓
Set Text Color (TextBlock_Price)
  └─ Color: Selected Color
```

### Pattern 3: Progress Bar Update

**Use Case**: Show cargo space, health, loading progress

```
Event UpdateCargoDisplay
  Inputs:
    - Used Space (float)
    - Total Space (float)
  ↓
Calculate Percent
  └─ Percent = Used Space / Total Space
     ↓
Set Percent (ProgressBar_Cargo)
  └─ Percent: Calculated Value
     ↓
Format Display Text
  └─ Text = "{0} / {1} tons"
     ├─ {0}: Used Space
     └─ {1}: Total Space
        ↓
Set Text (TextBlock_CargoInfo)
  └─ Text: Formatted Text
```

**Visual Result**:
```
[████████░░] 80%
"8.0 / 10.0 tons"
```

### Pattern 4: Shopping Cart Interface

**Use Case**: Add/remove items with quantities

```
Event OnAddToCartClicked
  Inputs:
    - Item (TradeItemDataAsset)
    - Quantity (int32)
  ↓
Check if in Cart
  ├─ Contains Key? (ShoppingCart, Item)
  └─ Return: Bool
     ↓
Branch (Already in Cart?)
  ├─ True → Update Quantity
  │    ↓
  │  Get Current Quantity
  │    └─ ShoppingCart[Item]
  │       ↓
  │  Add to Quantity
  │    └─ New Qty = Current + Quantity
  │       ↓
  │  Set Quantity
  │    └─ ShoppingCart[Item] = New Qty
  │
  └─ False → Add New Entry
       ↓
     Add to Map
       ├─ Map: ShoppingCart
       ├─ Key: Item
       └─ Value: Quantity
  ↓
Refresh Cart Display
  ↓
Broadcast OnCartUpdated
```

### Pattern 5: Tab/Page Switching

**Use Case**: Switch between Buy/Sell views, multiple pages

```
Event OnBuySellToggled
  ↓
Toggle Boolean (bShowBuyView)
  └─ New Value: !bShowBuyView
     ↓
Branch (bShowBuyView?)
  ├─ True → Show Buy Interface
  │    ↓
  │  Set Visibility (Panel_Buy)
  │    └─ Visibility: Visible
  │       ↓
  │  Set Visibility (Panel_Sell)
  │    └─ Visibility: Collapsed
  │       ↓
  │  Set Button Text (Button_Toggle)
  │    └─ Text: "Switch to Sell"
  │
  └─ False → Show Sell Interface
       ↓
     Set Visibility (Panel_Buy)
       └─ Visibility: Collapsed
          ↓
     Set Visibility (Panel_Sell)
       └─ Visibility: Visible
          ↓
     Set Button Text (Button_Toggle)
       └─ Text: "Switch to Buy"
  ↓
Refresh Current View
```

---

## Actor Blueprint Patterns

### Pattern 1: Component Interaction

**Use Case**: Get data from components, update state

```
Event BeginPlay
  ↓
Get Component by Class
  ├─ Class: UCargoComponent
  └─ Return: Cargo Component
     ↓
Is Valid? (Cargo Component)
  ├─ True → Initialize Cargo
  │    ↓
  │  Get Max Cargo Space
  │    └─ Return: float
  │       ↓
  │  Set Variable: MaxCargoSpace
  │    ↓
  │  Bind Event (OnCargoChanged)
  │    └─ Function: HandleCargoChanged
  │
  └─ False → Log Error
       ↓
     Print String: "Cargo Component Missing"
```

### Pattern 2: Trigger Volume Interaction

**Use Case**: Docking, interaction zones

```
Event OnOverlapBegin
  Inputs:
    - Overlapped Actor (AActor)
  ↓
Cast to ASpaceship
  ├─ Success → Valid Ship
  │    ↓
  │  Get Player Controller
  │    └─ Cast to AdastreaPlayerController
  │       ↓
  │  Show Interaction Prompt
  │    └─ Text: "Press E to Dock"
  │       ↓
  │  Store Reference
  │    └─ DockingShip = Overlapped Actor
  │
  └─ Failed → Not a Ship
       ↓
     [Do nothing]
```

### Pattern 3: AI Decision Making

**Use Case**: NPC trader, AI ship behavior

```
Event Tick
  ↓
Get Delta Seconds
  ↓
Update Decision Timer
  └─ Timer -= Delta
     ↓
Branch (Timer <= 0?)
  ├─ True → Make Decision
  │    ↓
  │  Reset Timer (5.0 seconds)
  │    ↓
  │  Get Nearby Stations
  │    └─ Return: Array<ASpaceStation>
  │       ↓
  │  Select Best Trade Route
  │    └─ Return: Best Station
  │       ↓
  │  Branch (Valid Station?)
  │    ├─ True → Start Trading
  │    │    ↓
  │    │  Set Destination
  │    │    ↓
  │    │  Navigate to Station
  │    │
  │    └─ False → Idle
  │
  └─ False → Continue Current Action
```

---

## Data Flow Patterns

### Pattern 1: Player → Component → UI

**Use Case**: Update UI from game state

```
[Player Buys Item]
  ↓
PlayerTraderComponent.BuyItem()
  ↓
Broadcast: OnCreditsChanged
  ├─ New Credits: 1500
  └─ Change Amount: -500
     ↓
[Widget Listening]
Event OnPlayerCreditsChanged
  Inputs:
    - New Credits (int32)
    - Change Amount (int32)
  ↓
Update Credits Display
  ├─ Set Text: "1500 CR"
  └─ Play Animation
     ├─ If Change < 0: Red flash (spent)
     └─ If Change > 0: Green flash (gained)
```

### Pattern 2: UI Input → Validation → Component → Feedback

**Use Case**: Complete transaction flow

```
[User Clicks Buy Button]
  ↓
Event OnBuyButtonClicked
  ↓
Get Cart Total
  └─ Return: 5000 CR
     ↓
Get Player Credits
  └─ Return: 3000 CR
     ↓
Validate Transaction
  └─ Has Enough? 3000 >= 5000?
     ├─ False → Show Error
     │    ↓
     │  Show Message: "Insufficient Credits"
     │    ↓
     │  Play Error Sound
     │    ↓
     │  Highlight Credits (Red)
     │
     └─ True → Execute Transaction
          ↓
        Call: PlayerTrader->ExecuteTrade()
          ├─ Success → Update UI
          │    ↓
          │  Clear Cart
          │    ↓
          │  Show Message: "Purchase Complete"
          │    ↓
          │  Play Success Sound
          │    ↓
          │  Refresh Display
          │
          └─ Failed → Handle Error
               ↓
             Show Message: "Transaction Failed"
```

### Pattern 3: Market Update → UI Refresh

**Use Case**: Dynamic price updates

```
[Economy Manager Updates]
EconomyManager.UpdatePrices()
  ↓
Broadcast: OnMarketPricesChanged
  ↓
[Widget Listening]
Event OnMarketInventoryUpdated
  ↓
Get Available Items
  └─ Call: TradingInterface->GetAvailableItems()
     └─ Return: Array<MarketInventoryEntry>
        ↓
ForEach (Item in Available Items)
  ├─ Find Existing Widget
  │    ↓
  │  Update Price Display
  │    ├─ New Price: Item.CurrentPrice
  │    └─ Compare to Base Price
  │       ├─ If Higher: Color Red
  │       └─ If Lower: Color Green
  │
  └─ Loop continues
```

---

## TradingInterfaceWidget Examples

### Example 1: Opening Market Interface

```
Function OpenMarket
  Inputs:
    - Market (MarketDataAsset)
  Returns: bool
  ↓
Branch (Is Valid? Market)
  ├─ False → Early Exit
  │    ↓
  │  Return: false
  │
  └─ True → Initialize
       ↓
     Set CurrentMarket = Market
       ↓
     Get Player Controller
       └─ Cast to AdastreaPlayerController
          ↓
        Get Player Pawn
          └─ Cast to ASpaceship
             ↓
           Get Component (PlayerTraderComponent)
             └─ Store: PlayerTrader
                ↓
           Get Component (CargoComponent)
             └─ Store: PlayerCargo
                ↓
           Get World Subsystem (EconomyManager)
             └─ Store: EconomyManager
                ↓
           Bind Component Events
             ├─ Bind: OnCreditsChanged
             ├─ Bind: OnCargoChanged
             └─ Bind: OnMarketUpdated
                ↓
           Refresh Market Display
             ↓
           Show Widget
             ↓
           Return: true
```

### Example 2: Buying Items

```
Event OnBuyButtonClicked
  Inputs:
    - Selected Item (TradeItemDataAsset)
    - Quantity (int32)
  ↓
Validate Selection
  ├─ Is Valid? (Selected Item)
  └─ Quantity > 0?
     ↓
Get Item Price
  └─ Call: GetItemPrice(Selected Item, Quantity)
     └─ Return: Total Price
        ↓
Get Player Credits
  └─ Call: GetPlayerCredits()
     └─ Return: Credits
        ↓
Check Affordability
  ├─ Credits >= Total Price?
  └─ Branch
     ├─ False → Show Error
     │    ↓
     │  Show Message: "Insufficient Credits"
     │    ├─ Current: Credits
     │    └─ Required: Total Price
     │       ↓
     │  Play Error Sound
     │    ↓
     │  Highlight Credits (Red flash)
     │
     └─ True → Check Cargo Space
          ↓
        Get Required Space
          └─ Space = Item.VolumePerUnit × Quantity
             ↓
        Get Available Cargo Space
          └─ Call: GetCargoSpaceInfo()
             ├─ UsedSpace
             ├─ TotalSpace
             └─ AvailableSpace
                ↓
        Check Space
          ├─ AvailableSpace >= Required Space?
          └─ Branch
             ├─ False → Show Error
             │    ↓
             │  Show Message: "Insufficient Cargo Space"
             │    ├─ Available: AvailableSpace
             │    └─ Required: Required Space
             │       ↓
             │  Play Error Sound
             │
             └─ True → Execute Purchase
                  ↓
                Add to Cart
                  └─ Call: AddToCart(Item, Quantity)
                     ↓
                Update Cart Display
                  ↓
                Play Success Sound
                  ↓
                Show Feedback
                  └─ Text: "Added to cart"
```

### Example 3: Transaction Completion

```
Function ExecuteTrade
  Returns: bool
  ↓
Get Cart Total
  └─ Call: GetCartTotal()
     └─ Return: TotalCost
        ↓
Validate Transaction
  └─ Call: ValidateTransaction(ErrorMessage)
     ├─ OutErrorMessage: Error Text
     └─ Return: IsValid
        ↓
Branch (IsValid?)
  ├─ False → Show Error
  │    ↓
  │  Show Message: ErrorMessage
  │    ↓
  │  Broadcast: OnTradeCompleted
  │    ├─ Success: false
  │    └─ Error: ErrorMessage
  │       ↓
  │  Return: false
  │
  └─ True → Process Transaction
       ↓
     ForEach (Item in ShoppingCart)
       ├─ Get Item
       ├─ Get Quantity
       └─ Loop Body
            ↓
          Call: PlayerTrader->BuyItem(Item, Qty)
            ├─ Deducts credits
            └─ Adds to cargo
               ↓
          Update Market Stock
            └─ Call: EconomyManager->RecordTransaction()
               ↓
          [Loop continues]
       ↓
     Completed
       ↓
     Clear Shopping Cart
       └─ ShoppingCart.Empty()
          ↓
     Refresh All Displays
       ├─ Refresh Market Display
       ├─ Update Credits Display
       └─ Update Cargo Display
          ↓
     Play Success Effects
       ├─ Sound: Transaction Complete
       └─ Visual: Success animation
          ↓
     Show Confirmation
       └─ Text: "Transaction Complete!"
          ├─ Items Purchased: X
          └─ Total Spent: X CR
             ↓
     Broadcast: OnTradeCompleted
       ├─ Success: true
       └─ Error: ""
          ↓
     Return: true
```

### Example 4: Real-time Price Updates

```
Event NativeTick
  Inputs:
    - Delta Time (float)
  ↓
Branch (Is Market Open?)
  ├─ False → Skip update
  │
  └─ True → Update display
       ↓
     Update Timer += Delta Time
       ↓
     Branch (Timer >= 1.0?)
       ├─ False → Skip this frame
       │
       └─ True → Refresh prices
            ↓
          Reset Timer = 0
            ↓
          Get Available Items
            └─ Return: Current Items
               ↓
          ForEach (Item in Current Items)
            └─ Loop Body
                 ↓
               Get Latest Price
                 └─ Call: EconomyManager->GetItemPrice()
                    └─ Return: Current Price
                       ↓
               Find Widget for Item
                 └─ Find in ItemList children
                    └─ Return: Item Widget
                       ↓
               Update Widget Price
                 ├─ Set Text: New Price
                 └─ Set Color: Based on change
                    ├─ If increased: Red
                    └─ If decreased: Green
                       ↓
               [Loop continues]
```

---

## Best Practices

### DO ✅

1. **Use Visual Flow Diagrams**
   - Always show node connections
   - Use consistent symbols (↓, →, ├─, └─)
   - Indent for hierarchy

2. **Include Context**
   - Show input parameters
   - Show return values
   - Label important variables

3. **Document Branches Clearly**
   - Show both True and False paths
   - Explain branch conditions
   - Show where paths merge

4. **Show Real Values**
   ```
   Get Item Price
     └─ Return: 150 CR  ← Actual example value
   ```

5. **Group Related Operations**
   ```
   ── Validation Section ──
   Check Credits
   Check Cargo Space
   Check Item Availability
   ── End Validation ──
   ```

6. **Use Comments for Complex Logic**
   ```
   Calculate Dynamic Price
     // Base price × station multiplier × supply factor
     └─ Price = 100 × 1.2 × 0.85 = 102 CR
   ```

### DON'T ❌

1. **Don't Use Pure Text**
   ❌ Bad: "Then call the GetItemPrice function with the selected item"
   ✅ Good:
   ```
   Get Item Price
     ├─ Item: Selected Item
     └─ Return: Price
   ```

2. **Don't Skip Important Steps**
   ❌ Bad:
   ```
   Event OnBuy
     ↓
   ... (buy logic)
     ↓
   Update UI
   ```
   ✅ Good: Show ALL steps

3. **Don't Mix Styles**
   - Pick one diagram style and stick to it
   - Consistent indentation (2 spaces)
   - Same symbols throughout

4. **Don't Ignore Error Cases**
   - Always show null checks
   - Show validation failures
   - Document error handling

5. **Don't Forget Return Values**
   ```
   ❌ Call Function
   
   ✅ Call Function
        └─ Return: Result
   ```

---

## Quick Reference Templates

### Template 1: Basic Event Handler

```
Event [EventName]
  Inputs:
    - [ParamName] ([Type])
  ↓
[Action Node]
  ├─ Input: [Value]
  └─ Return: [Result]
     ↓
[Next Action]
```

### Template 2: Validated Action

```
Event [ActionName]
  ↓
[Validation Check]
  ↓
Branch (Valid?)
  ├─ True → Execute
  │    ↓
  │  [Main Logic]
  │    ↓
  │  [Success Feedback]
  │
  └─ False → Error
       ↓
     [Error Message]
       ↓
     [Error Feedback]
```

### Template 3: List Population

```
Event [UpdateListEvent]
  ↓
Clear Children ([Container])
  ↓
Get Data Collection
  └─ Return: Array<[Type]>
     ↓
ForEach Loop
  ├─ Array: Data Collection
  └─ Loop Body
       ↓
     Create Widget: [WidgetClass]
       ↓
     Set Data: Current Item
       ↓
     Add to Container
       ↓
     [Loop continues]
```

### Template 4: Complex Transaction

```
Event [TransactionEvent]
  ↓
── Validation Phase ──
  ↓
[Check 1]
  ↓
[Check 2]
  ↓
[Check 3]
  ↓
Branch (All Valid?)
  ├─ False → Handle Error
  │
  └─ True → Continue
       ↓
     ── Execution Phase ──
       ↓
     [Core Logic]
       ↓
     ── Feedback Phase ──
       ↓
     [UI Updates]
       ↓
     [Sound/Animation]
       ↓
     [Event Broadcasting]
```

### Template 5: Component Interaction

```
Event [Initialize/BeginPlay]
  ↓
Get Component by Class
  ├─ Class: [ComponentType]
  └─ Return: Component Reference
     ↓
Is Valid?
  ├─ True → Setup
  │    ↓
  │  [Initialize Component]
  │    ↓
  │  [Bind Events]
  │    ↓
  │  [Store Reference]
  │
  └─ False → Error
       ↓
     Log Error: "Missing Component"
```

---

## Real-World Example: Complete Market UI

```
════════════════════════════════════════════════
WBP_TradingInterface - Complete Flow
════════════════════════════════════════════════

Event Construct
  ↓
Initialize Components
  ├─ Find PlayerTraderComponent
  ├─ Find CargoComponent
  └─ Find EconomyManager
     ↓
Bind Events
  ├─ OnMarketInventoryUpdated
  ├─ OnCartUpdated
  ├─ OnCreditsUpdated
  └─ OnCargoSpaceUpdated
     ↓
Setup UI
  ├─ Initialize Button Events
  ├─ Setup Category Filters
  └─ Set Initial State
     ↓
Ready for user input

────────────────────────────────────────────────

Event OnMarketInventoryUpdated
  ↓
Clear Children (ScrollBox_ItemList)
  ↓
Get Available Items
  └─ Call: GetAvailableItems()
     └─ Return: Array<MarketInventoryEntry>
        ↓
ForEach (Entry in Available Items)
  └─ Loop Body
       ↓
     Create Widget
       ├─ Class: WBP_ItemListEntry
       └─ Return: ItemWidget
          ↓
     Set Item Data
       ├─ Item Name: Entry.TradeItem.ItemName
       ├─ Price: Entry.CurrentPrice
       ├─ Stock: Entry.CurrentStock
       └─ Player Qty: PlayerInventory[Item]
          ↓
     Bind Item Click Event
       └─ Function: OnItemSelected
          ↓
     Add to ScrollBox
       ↓
     [Loop continues]

────────────────────────────────────────────────

Event OnItemSelected
  Inputs:
    - Selected Item (TradeItemDataAsset)
  ↓
Set SelectedItem = Selected Item
  ↓
Update Detail Panel
  ├─ Item Name
  ├─ Item Description
  ├─ Current Price
  ├─ Base Price (comparison)
  └─ Item Stats
     ├─ Volume Per Unit
     ├─ Mass Per Unit
     └─ Legal Status
        ↓
Update Buy/Sell Options
  ├─ Enable Quantity Spinner
  ├─ Update Max Quantity
  │   ├─ Buy: Based on credits & cargo
  │   └─ Sell: Based on inventory
  └─ Show Price Preview

────────────────────────────────────────────────

Event OnAddToCartClicked
  ↓
Get Selected Item
  ↓
Branch (Valid?)
  ├─ False → Show Error
  │    └─ "Please select an item"
  │
  └─ True → Get Quantity
       ↓
     Get Spinner Value
       └─ Return: Quantity
          ↓
     Branch (Quantity > 0?)
       ├─ False → Show Error
       │    └─ "Quantity must be > 0"
       │
       └─ True → Add to Cart
            ↓
          Call: AddToCart(Item, Quantity)
            ├─ Validates credits
            ├─ Validates cargo space
            └─ Return: Success
               ↓
          Branch (Success?)
            ├─ True → Feedback
            │    ├─ Play Sound: Add to Cart
            │    ├─ Show Message: "Added"
            │    └─ Update Cart Display
            │
            └─ False → Error
                 └─ Show validation error

────────────────────────────────────────────────

Event OnConfirmTransactionClicked
  ↓
Get Cart Total
  └─ Call: GetCartTotal()
     └─ Return: TotalCost
        ↓
Show Confirmation Dialog
  ├─ Message: "Confirm purchase?"
  ├─ Items: Cart item count
  ├─ Total: TotalCost
  ├─ Button: "Confirm"
  └─ Button: "Cancel"
     ↓
[Wait for user choice]
  ↓
Branch (User Confirmed?)
  ├─ False → Close Dialog
  │
  └─ True → Execute
       ↓
     Call: ExecuteTrade()
       └─ Return: Success
          ↓
     Branch (Success?)
       ├─ True → Complete
       │    ├─ Clear Cart
       │    ├─ Play Success Animation
       │    ├─ Show Message: "Complete!"
       │    └─ Refresh All Displays
       │
       └─ False → Error
            ├─ Show Error Message
            └─ Keep cart intact

════════════════════════════════════════════════
```

---

## Summary

### Key Takeaways

1. **Visual > Text**: Always use visual flow diagrams
2. **Consistency**: Use standard symbols and formatting
3. **Completeness**: Show all branches and returns
4. **Context**: Include types, values, and examples
5. **Real-World**: Base on actual implementation

### When to Use This Guide

- ✅ Documenting new Blueprint implementations
- ✅ Creating usage guides for C++ classes
- ✅ Explaining complex Blueprint logic
- ✅ Providing examples for designers
- ✅ Code reviews and design discussions

### Related Documentation

- [BLUEPRINT_QUICK_REFERENCE.md](BLUEPRINT_QUICK_REFERENCE.md) - Blueprint creation overview
- [BLUEPRINT_REQUIREMENTS_LIST.md](BLUEPRINT_REQUIREMENTS_LIST.md) - Complete Blueprint list
- [MainMenuWidget_Usage.md](../../Source/Adastrea/Public/UI/MainMenuWidget_Usage.md) - Real example
- [TradingSystemBlueprintGuide.md](../../Blueprints/TradingSystemBlueprintGuide.md) - Trading examples

---

**Last Updated**: December 29, 2025  
**Maintained by**: Adastrea Development Team  
**Feedback**: Please submit improvements via GitHub issues
