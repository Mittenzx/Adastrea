# Blueprint Patterns Library

> **Common Blueprint patterns and solutions used in Adastrea**

**Last Updated**: January 17, 2026  
**Purpose**: Reusable Blueprint patterns for common tasks  
**Audience**: Intermediate Blueprint developers

---

## 🎯 Purpose

This library contains proven Blueprint patterns you'll use repeatedly in Adastrea. Each pattern includes:
- Visual diagram
- When to use it
- How to implement it
- Real examples from the project

---

## 📚 Pattern Categories

1. [Initialization Patterns](#initialization-patterns)
2. [Validation Patterns](#validation-patterns)
3. [UI Update Patterns](#ui-update-patterns)
4. [Event Communication Patterns](#event-communication-patterns)
5. [Data Management Patterns](#data-management-patterns)
6. [Performance Patterns](#performance-patterns)

---

## Initialization Patterns

### Pattern 1: Basic Initialization

**When to Use**: Setting up initial state when actor spawns

**Blueprint:**
```
Event BeginPlay
  ├─→ Initialize Variables
  ├─→ Setup Components
  ├─→ Register Events
  └─→ Print "Initialization Complete"
```

**Example - Trading Ship:**
```
Event BeginPlay
  ├─→ Set PlayerCredits = 1000
  ├─→ Set CargoCapacity = 10
  ├─→ Set CurrentCargoWeight = 0
  ├─→ Create Widget (WBP_ShipHUD)
  │     └─→ Add to Viewport
  ├─→ Setup Input Mapping
  └─→ Print "Ship Ready"
```

**Adastrea Usage:**
- `BP_TradingShip` - Initialize ship stats
- `BP_TradingStation` - Setup market inventory
- `BP_GameMode` - Initialize economy

---

### Pattern 2: Delayed Initialization

**When to Use**: When you need to wait for other systems to be ready

**Blueprint:**
```
Event BeginPlay
  → Delay (0.1 seconds)
    → Check if Systems Ready
      → Branch
        ├─ True → Initialize
        └─ False → Delay and Check Again
```

**Example - Wait for Game Mode:**
```
Event BeginPlay
  → Delay (0.1)
    → Get Game Mode
      → Is Valid?
        → Branch
          ├─ True
          │   → Get Economy Manager
          │   → Register with Economy
          │   → Initialize Market
          └─ False
              → Print "Waiting for Game Mode..."
              → Delay (0.5)
              → (Try again)
```

---

## Validation Patterns

### Pattern 3: Simple Validation

**When to Use**: Check one condition before proceeding

**Blueprint:**
```
Event OnAction
  → Get Condition
    → Branch
      ├─ True → Proceed with Action
      └─ False → Show Error Message
```

**Example - Buy Item:**
```
Event OnBuyButtonClicked
  → Get Player Credits
    → Greater Than (Item Price)
      → Branch
        ├─ True
        │   → Complete Purchase
        │   → Update UI
        └─ False
            → Print "Not enough credits!"
            → Play Error Sound
```

---

### Pattern 4: Multi-Condition Validation

**When to Use**: Check multiple conditions before proceeding

**Blueprint:**
```
Event OnAction
  → Check Condition 1 → Bool1
  → Check Condition 2 → Bool2
  → Check Condition 3 → Bool3
  → Bool1 AND Bool2 AND Bool3 → AllValid
  → Branch (AllValid)
    ├─ True → Proceed
    └─ False → Determine Which Failed
```

**Example - Complete Purchase Validation:**
```
Event OnBuyButtonClicked
  ├─→ Get Player Credits
  │     → Greater Than (Item Price) → CanAfford
  ├─→ Get Cargo Space
  │     → Greater Than (Item Volume) → HasSpace
  └─→ Get Is Docked → IsDocked

  → CanAfford AND HasSpace AND IsDocked → AllowPurchase

  → Branch (AllowPurchase)
    ├─ True
    │   → Complete Purchase
    │   → Update All Systems
    └─ False
        → Branch (CanAfford)
          ├─ False → "Not enough credits"
          └─ True
              → Branch (HasSpace)
                ├─ False → "Cargo full"
                └─ True
                    → Branch (IsDocked)
                      ├─ False → "Must be docked"
                      └─ True → "Unknown error"
```

---

### Pattern 5: Safe Reference Check

**When to Use**: Before using any object reference to prevent crashes

**Blueprint:**
```
Get Object Reference
  → Is Valid?
    → Branch
      ├─ True → Use Object
      └─ False → Handle Null Case
```

**Example - Get Market Data:**
```
Get Docked Station
  → Is Valid?
    → Branch
      ├─ True
      │   → Get Marketplace Module
      │     → Is Valid?
      │       → Branch
      │         ├─ True
      │         │   → Get Market Data
      │         │   → Open Trading UI
      │         └─ False
      │             → Print "Station has no marketplace"
      └─ False
          → Print "Not docked at any station"
```

---

## UI Update Patterns

### Pattern 6: Single Value Update

**When to Use**: Update one UI element when value changes

**Blueprint:**
```
Event OnValueChanged
  → Get New Value
    → Convert to Display Format
      → Update UI Element
```

**Example - Update Credits Display:**
```
Function: UpdateCreditsDisplay

Parameters: NewCredits (Integer)

Flow:
  → To String (NewCredits) → CreditsText
  → Append ("Credits: ", CreditsText) → DisplayText
  → Set Text (CreditsTextBlock, DisplayText)
```

**Usage:**
```
Event OnPurchaseComplete
  → Get Player Credits
  → Call: UpdateCreditsDisplay
```

---

### Pattern 7: Bulk UI Refresh

**When to Use**: Update entire UI section with new data

**Blueprint:**
```
Event OnDataChanged
  → Get All Data
    → Clear UI Container
      → ForEachLoop (All Items)
        └─ Create UI Entry
          → Populate with Data
          → Add to Container
```

**Example - Refresh Market Display:**
```
Custom Event: RefreshMarketDisplay

Flow:
  → Get Market Inventory → ItemsArray
  → Clear Children (MarketScrollBox)
  → ForEachLoop (ItemsArray)
    └─ Loop Body
      → Get Array Element → CurrentItem
      → Create Widget (WBP_MarketItemEntry)
      → Set Item Name
      → Set Item Price
      → Set Buy Button OnClick → OnItemBuyClicked
      → Add Child to Scroll Box (Widget)
  → Completed
    → Print "Market Display Updated"
```

---

### Pattern 8: Conditional UI Update

**When to Use**: Update UI only if values actually changed

**Blueprint:**
```
Variable: LastValue

Event OnValueChanged (NewValue)
  → NewValue != LastValue → Changed
  → Branch (Changed)
    ├─ True
    │   → Update UI
    │   → Set LastValue = NewValue
    └─ False
        → (Do nothing - no update needed)
```

**Example - Efficient Cargo Display:**
```
Variable: LastCargoWeight = 0

Event OnCargoChanged
  → Get Current Cargo Weight → NewWeight
  → NewWeight != LastCargoWeight → Changed
  → Branch (Changed)
    ├─ True
    │   → Update Cargo Display (NewWeight)
    │   → Set LastCargoWeight = NewWeight
    └─ False
        → (Skip update - no change)
```

---

## Event Communication Patterns

### Pattern 9: Direct Event Call

**When to Use**: Communicate between components in same actor

**Blueprint:**
```
Component A:
  Event OnSomethingHappened
    → Call Custom Event on Component B

Component B:
  Custom Event: DoSomething
    → Execute Logic
```

**Example - Ship Cargo to UI:**
```
BP_TradingShip (Cargo Component):
  Function: AddCargo
    → Update Internal Cargo Array
    → Call Custom Event: OnCargoChanged

BP_TradingShip (Event Graph):
  Custom Event: OnCargoChanged
    → Get HUD Widget
      → Call: RefreshCargoDisplay
```

---

### Pattern 10: Event Dispatcher Pattern

**When to Use**: Communicate between separate actors

**Blueprint:**
```
Actor A:
  Event Dispatcher: OnEventHappened

Actor B (Event Graph):
  Event BeginPlay
    → Get Reference to Actor A
      → Bind Event to OnEventHappened
        → Custom Function: HandleEvent

Actor A:
  When Something Happens
    → Call Event Dispatcher: OnEventHappened
      → All bound listeners receive event
```

**Example - Station to Ship:**
```
BP_TradingStation:
  Event Dispatcher: OnMarketInventoryChanged

BP_TradingShip:
  Event OnDocked (at Station)
    → Get Station
      → Bind Event: OnMarketInventoryChanged
        → Function: RefreshMarketDisplay

BP_TradingStation:
  Function: UpdateInventory
    → Modify Inventory Array
    → Call Dispatcher: OnMarketInventoryChanged
      → (All docked ships refresh their displays)
```

---

## Data Management Patterns

### Pattern 11: Safe Array Access

**When to Use**: Get item from array without crashing

**Blueprint:**
```
Get Array
  → Get Array Length → Length
  → Index < Length → IsValidIndex
  → Branch (IsValidIndex)
    ├─ True
    │   → Get Array Element (Index)
    │   → Use Element
    └─ False
        → Print "Invalid array index"
```

**Example - Get Market Item:**
```
Function: GetMarketItemAtIndex

Parameters: Index (Integer)
Returns: ItemData (Struct or null)

Flow:
  → Get Market Inventory Array → Items
  → Get Array Length → Length
  → Index >= 0 AND Index < Length → Valid
  → Branch (Valid)
    ├─ True
    │   → Get Array Element (Index)
    │   → Return Element
    └─ False
        → Print "Invalid index"
        → Return null
```

---

### Pattern 12: Find Item in Array

**When to Use**: Search array for specific item

**Blueprint:**
```
Get Array
  → ForEachLoop
    └─ Loop Body
      → Get Array Element
      → Check if Matches Criteria
        → Branch
          ├─ True
          │   → Found Item!
          │   → Break Loop
          │   → Return Item
          └─ False
              → Continue Loop
```

**Example - Find Item by Name:**
```
Function: FindItemByName

Parameters: ItemName (String)
Returns: ItemData or null

Flow:
  → Get Cargo Array
  → ForEachLoop
    └─ Loop Body
      → Get Array Element → CurrentItem
      → Get CurrentItem.Name → Name
      → Name == ItemName → Match
      → Branch (Match)
        ├─ True
        │   → Break (exit loop)
        │   → Return CurrentItem
        └─ False
            → (Continue to next item)
  → Completed (if reached, not found)
    → Return null
```

---

## Performance Patterns

### Pattern 13: Cache Expensive References

**When to Use**: Avoid repeated expensive lookups

**❌ Bad Pattern:**
```
Event Tick
  → Get All Actors of Class (Expensive!)
    → Do Something
```

**✅ Good Pattern:**
```
Variable: CachedActors (Array)

Event BeginPlay
  → Get All Actors of Class
  → Set CachedActors = Result

Event Tick (or better: Timer)
  → Get CachedActors
    → Do Something
```

**Example - Cache Station Reference:**
```
BP_TradingShip:

Variable: DockedStation (Reference)

Event OnDockingComplete (Station)
  → Set DockedStation = Station
  → (Cache the reference)

Function: OpenTradingUI
  → Get DockedStation (from cache)
    → Is Valid?
      → Get Market Data
      → Show UI
```

---

### Pattern 14: Timer Instead of Tick

**When to Use**: Periodic updates that don't need to run every frame

**❌ Bad Pattern:**
```
Event Tick
  → Update Economy Prices (Heavy calculation!)
```

**✅ Good Pattern:**
```
Event BeginPlay
  → Set Timer by Event
    (Function: UpdateEconomy, Time: 5.0, Looping: true)

Function: UpdateEconomy
  → Calculate New Prices
  → Update Markets
```

**Example - Market Price Updates:**
```
BP_TradingGameMode:

Event BeginPlay
  → Set Timer by Function Name
    (Function: "UpdateMarketPrices")
    (Time: 60.0 seconds)
    (Looping: true)

Function: UpdateMarketPrices
  → Get All Trading Stations
  → ForEachLoop
    └─ Update Station Market Prices
  → Broadcast Price Change Event
```

---

### Pattern 15: Event-Driven Updates

**When to Use**: Update only when something changes, not constantly

**❌ Bad Pattern:**
```
Event Tick
  → Get Current Credits
  → Update UI Display
```

**✅ Good Pattern:**
```
Custom Event: OnCreditsChanged (NewCredits)
  → Update UI Display (NewCredits)

Function: ModifyCredits (Amount)
  → CurrentCredits += Amount
  → Call Event: OnCreditsChanged
```

**Example - Cargo Weight Display:**
```
BP_TradingShip:

Custom Event: OnCargoWeightChanged (NewWeight)
  → Update HUD Cargo Display
  → Check if Overweight Warning

Function: AddCargo (Item)
  → Add to Cargo Array
  → Calculate New Total Weight
  → Call Event: OnCargoWeightChanged
```

---

## Quick Reference

### When to Use Each Pattern

| Pattern | Use When | Example |
|---------|----------|---------|
| Basic Init | Setting up actor on spawn | Ship initialization |
| Delayed Init | Waiting for other systems | Economy manager ready |
| Simple Validation | One condition check | Can afford item? |
| Multi Validation | Multiple conditions | Complete purchase check |
| Safe Reference | Using object references | Get market data |
| Single UI Update | One value changed | Credits display |
| Bulk UI Refresh | Multiple items changed | Market inventory |
| Conditional Update | Maybe value changed | Cargo weight |
| Direct Event | Same actor communication | Component to component |
| Event Dispatcher | Different actors | Station to ship |
| Safe Array Access | Getting array element | Get item by index |
| Find in Array | Searching array | Find item by name |
| Cache Reference | Avoid expensive lookups | Store station ref |
| Timer Updates | Periodic tasks | Price updates |
| Event-Driven | Changes trigger updates | Cargo changed |

---

## Best Practices

### Pattern Selection

1. **Start Simple** - Use simplest pattern that works
2. **Optimize Later** - Don't prematurely optimize
3. **Test First** - Verify pattern works before moving on
4. **Consistent Style** - Use same patterns throughout project

### Common Mistakes

1. **Over-Engineering** - Don't use complex patterns for simple tasks
2. **Ignoring Safety** - Always validate references and indices
3. **Performance Ignorance** - Avoid Tick when possible
4. **Poor Organization** - Group related patterns with comments

---

## Next Steps

### Practice These Patterns

1. **Implement Each Pattern** - Create test Blueprints
2. **Combine Patterns** - Build real features
3. **Study Examples** - Look at Adastrea Blueprints
4. **Contribute** - Share your pattern discoveries

### Real Projects

Apply these patterns in:
- [Trading UI Guide](../../Blueprints/TradingSystemBlueprintGuide_SIMPLIFIED.md)
- [Trading Ship Guide](../../Blueprints/BP_TradingShip_GUIDE.md)
- [Market Display Guide](../../Blueprints/WBP_MarketDisplay_GUIDE.md)

---

**Remember**: Patterns are solutions to common problems. Learn them, but adapt them to your specific needs!
