# Blueprint Visual Mockup Guide - Enhanced with Colors and Node Details

> **Realistic Blueprint documentation with colors and detailed node representations**

**Last Updated**: December 29, 2025
**For Version**: 1.0.0-alpha
**Target Engine**: Unreal Engine 5.6

---

## 🎨 Color-Coded Node System

### Node Color Legend

Blueprint nodes in Unreal Engine use specific colors to indicate their type:

| Node Type | Color | Hex Code | Usage |
|-----------|-------|----------|-------|
| **Event** | 🔴 Red | `#C80000` | Entry points (BeginPlay, OnClicked, etc.) |
| **Function** | 🔵 Blue | `#1B4F72` | Function calls and custom functions |
| **Pure Function** | 🟢 Green | `#006400` | Getter functions (no exec pins) |
| **Branch** | ⚪ White | `#FFFFFF` | Conditional logic (if/else) |
| **Flow Control** | 🟠 Orange | `#FF8C00` | Loops, sequences, gates |
| **Variable Get** | 🟦 Light Blue | `#5DADE2` | Getting variable values |
| **Variable Set** | 🟦 Light Blue | `#5DADE2` | Setting variable values |
| **Macro** | 🟣 Purple | `#8E44AD` | Collapsed graphs/macros |
| **Delegate** | 🟥 Dark Red | `#8B0000` | Event dispatchers and binds |

### Pin Type Colors

| Pin Type | Color | Hex Code |
|----------|-------|----------|
| **Exec (White)** | ⚫→⚪ | Flow pins |
| **Boolean** | 🔴 Red | `#DC143C` |
| **Integer** | 🟢 Green | `#00FF00` |
| **Float** | 🟢 Green | `#90EE90` |
| **String** | 🟣 Magenta | `#FF00FF` |
| **Text** | 🟣 Pink | `#FFB6C1` |
| **Vector** | 🟡 Yellow | `#FFD700` |
| **Rotator** | 🟠 Orange | `#FFA500` |
| **Transform** | 🟠 Orange | `#FF8C00` |
| **Object Reference** | 🔵 Cyan | `#00FFFF` |
| **Actor Reference** | 🔵 Cyan | `#40E0D0` |
| **Struct** | 🟤 Blue | `#4682B4` |

---

## 📐 Detailed Node Representations

### Standard Node Format

```
╔══════════════════════════════════════════╗
║ 🔴 Event OnMarketInventoryUpdated        ║
╠══════════════════════════════════════════╣
║                                    ⚪ ═►  ║ Exec Out
╚══════════════════════════════════════════╝
                ↓
╔══════════════════════════════════════════╗
║ 🔵 Clear Children                        ║
╠══════════════════════════════════════════╣
║ ⚪ ═► In Exec                             ║
║ 🔵 Target: ScrollBox_ItemList            ║
║                                    ⚪ ═►  ║ Exec Out
╚══════════════════════════════════════════╝
                ↓
╔══════════════════════════════════════════╗
║ 🔵 Get Available Items                   ║
╠══════════════════════════════════════════╣
║ ⚪ ═► In Exec                             ║
║ 🔵 Target: TradingInterface              ║
║                                    ⚪ ═►  ║ Exec Out
║                      Array<Entry> 🔵 ═►  ║ Return Value
╚══════════════════════════════════════════╝
                ↓
╔══════════════════════════════════════════╗
║ 🟠 ForEach Loop                          ║
╠══════════════════════════════════════════╣
║ ⚪ ═► In Exec                             ║
║ 🔵 ►═ Array: Available Items             ║
║                        Loop Body ⚪ ═►    ║
║                       Completed ⚪ ═►     ║
║          Array Element (Entry) 🔵 ═►     ║
║                  Array Index 🟢 ═►       ║
╚══════════════════════════════════════════╝
                │
                ├─ Loop Body ─┐
                │             ↓
                │   ╔════════════════════════════════════╗
                │   ║ 🔵 Create Widget                   ║
                │   ╠════════════════════════════════════╣
                │   ║ ⚪ ═► In Exec                       ║
                │   ║ 🟣 Class: WBP_ItemListEntry        ║
                │   ║ 🔵 Owning Player: Get Player       ║
                │   ║                          ⚪ ═►     ║ Exec Out
                │   ║         Return Value 🔵 ═►         ║
                │   ╚════════════════════════════════════╝
                │             ↓
                │   ╔════════════════════════════════════╗
                │   ║ 🔵 Set Item Data                   ║
                │   ╠════════════════════════════════════╣
                │   ║ ⚪ ═► In Exec                       ║
                │   ║ 🔵 Target: ItemWidget              ║
                │   ║ 🟣 Item Name: Entry.ItemName       ║
                │   ║ 🟢 Price: Entry.CurrentPrice       ║
                │   ║ 🟢 Stock: Entry.CurrentStock       ║
                │   ║                          ⚪ ═►     ║ Exec Out
                │   ╚════════════════════════════════════╝
                │             ↓
                │   ╔════════════════════════════════════╗
                │   ║ 🔵 Add Child to ScrollBox          ║
                │   ╠════════════════════════════════════╣
                │   ║ ⚪ ═► In Exec                       ║
                │   ║ 🔵 Target: ScrollBox_ItemList      ║
                │   ║ 🔵 Content: ItemWidget             ║
                │   ║                          ⚪ ═►     ║ Exec Out
                │   ╚════════════════════════════════════╝
                │             ↓
                └─────────────┘ (back to loop)
                │
                └─ Completed ─► [Continue]
```

---

## 🎯 Real-World Example: Market Inventory Update

### Complete Visual Blueprint

```
════════════════════════════════════════════════════════════════════
                    Market Inventory Update Flow
════════════════════════════════════════════════════════════════════

┌────────────────────────────────────────────────────────────────┐
│ 🔴 EVENT: OnMarketInventoryUpdated                             │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ Called when market prices or inventory changes            │ │
│ └────────────────────────────────────────────────────────────┘ │
│                                                          ⚪ ═► │
└────────────────────────────────────────────────────────────────┘
                            ↓
┌────────────────────────────────────────────────────────────────┐
│ 🔵 FUNCTION: Clear Children                                    │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ Removes all existing item widgets from the list           │ │
│ └────────────────────────────────────────────────────────────┘ │
│ ⚪ ►─ Exec                                                      │
│ 🔵 ►─ Target: ScrollBox_ItemList (ScrollBox Reference)        │
│                                                          ⚪ ═► │
└────────────────────────────────────────────────────────────────┘
                            ↓
┌────────────────────────────────────────────────────────────────┐
│ 🔵 FUNCTION: Get Available Items                               │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ C++ Function - Returns current market inventory           │ │
│ └────────────────────────────────────────────────────────────┘ │
│ ⚪ ►─ Exec                                                      │
│ 🔵 ►─ Target: TradingInterface (Self)                          │
│                                                          ⚪ ═► │ Exec
│                         Return Value (Array<Entry>) 🔵 ─►     │
└────────────────────────────────────────────────────────────────┘
                            ↓
┌────────────────────────────────────────────────────────────────┐
│ 🟠 FLOW CONTROL: ForEach Loop                                  │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ Iterates through each inventory entry                     │ │
│ └────────────────────────────────────────────────────────────┘ │
│ ⚪ ►─ Exec                                                      │
│ 🔵 ►─ Array: Available Items (Array<MarketInventoryEntry>)    │
│                                          Loop Body ⚪ ─►       │
│                                         Completed ⚪ ─►        │
│                    Array Element (MarketInventoryEntry) 🔵 ─► │
│                                      Array Index (int) 🟢 ─►  │
└────────────────────────────────────────────────────────────────┘
        │                                   │
        │ ┌─────────── Loop Body ───────────┘
        │ ↓
        │ ┌──────────────────────────────────────────────────────┐
        │ │ 🔵 FUNCTION: Create Widget                           │
        │ │ ┌──────────────────────────────────────────────────┐ │
        │ │ │ Creates new item list entry widget              │ │
        │ │ └──────────────────────────────────────────────────┘ │
        │ │ ⚪ ►─ Exec                                            │
        │ │ 🟣 ►─ Class: WBP_ItemListEntry (WidgetBlueprint)     │
        │ │ 🔵 ►─ Owning Player: GetOwningPlayer()               │
        │ │                                                ⚪ ═► │
        │ │                    Return Value (Widget) 🔵 ─►      │
        │ └──────────────────────────────────────────────────────┘
        │                     ↓
        │ ┌──────────────────────────────────────────────────────┐
        │ │ 🔵 FUNCTION: Set Item Data                           │
        │ │ ┌──────────────────────────────────────────────────┐ │
        │ │ │ Custom function - Configures widget with data   │ │
        │ │ └──────────────────────────────────────────────────┘ │
        │ │ ⚪ ►─ Exec                                            │
        │ │ 🔵 ►─ Target: Created Widget                          │
        │ │ 🟣 ►─ Item Name: Entry.TradeItem.ItemName (Text)     │
        │ │ 🟢 ►─ Price: Entry.CurrentPrice (float)              │
        │ │ 🟢 ►─ Stock: Entry.CurrentStock (int32)              │
        │ │ 🔴 ►─ In Stock: Entry.bInStock (bool)                │
        │ │                                                ⚪ ═► │
        │ └──────────────────────────────────────────────────────┘
        │                     ↓
        │ ┌──────────────────────────────────────────────────────┐
        │ │ 🔵 FUNCTION: Add Child to Vertical Box              │
        │ │ ┌──────────────────────────────────────────────────┐ │
        │ │ │ Adds widget to the scrollable item list         │ │
        │ │ └──────────────────────────────────────────────────┘ │
        │ │ ⚪ ►─ Exec                                            │
        │ │ 🔵 ►─ Target: ScrollBox_ItemList                     │
        │ │ 🔵 ►─ Content: Created Widget                        │
        │ │                                                ⚪ ═► │
        │ │                        Slot (Panel Slot) 🔵 ─►      │
        │ └──────────────────────────────────────────────────────┘
        │                     ↓
        │                [Loop continues...]
        │
        └─ Completed ─►  ┌────────────────────────────────────┐
                         │ [Continue to next operations...]   │
                         └────────────────────────────────────┘
```

---

## 🔀 Branch Node Example with Colors

### Buy Item Validation

```
┌────────────────────────────────────────────────────────────────┐
│ 🔴 EVENT: OnBuyButtonClicked                                   │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ Inputs:                                                    │ │
│ │   • Item (TradeItemDataAsset) 🔵                           │ │
│ │   • Quantity (int32) 🟢                                    │ │
│ └────────────────────────────────────────────────────────────┘ │
│                                                          ⚪ ═► │
└────────────────────────────────────────────────────────────────┘
                            ↓
┌────────────────────────────────────────────────────────────────┐
│ 🔵 FUNCTION: Get Item Price                                    │
│ ⚪ ►─ Exec                                                      │
│ 🔵 ►─ Item: Selected Item                                      │
│ 🟢 ►─ Quantity: Quantity                                       │
│                                                          ⚪ ═► │
│                                  Total Price (int) 🟢 ─►      │
└────────────────────────────────────────────────────────────────┘
                            ↓
┌────────────────────────────────────────────────────────────────┐
│ 🟢 PURE: Get Player Credits                                    │
│ 🔵 ►─ Target: PlayerTrader                                     │
│                                         Credits (int) 🟢 ─►    │
└────────────────────────────────────────────────────────────────┘
                            ↓
┌────────────────────────────────────────────────────────────────┐
│ ⚪ BRANCH: >= (Credits >= TotalPrice?)                         │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ Checks if player has enough credits                       │ │
│ └────────────────────────────────────────────────────────────┘ │
│ ⚪ ►─ Exec                                                      │
│ 🔴 ►─ Condition: (Credits >= TotalPrice)                       │
│                                              True ⚪ ─►        │
│                                             False ⚪ ─►         │
└────────────────────────────────────────────────────────────────┘
        │                           │
        │ TRUE                      │ FALSE
        ↓                           ↓
┌─────────────────────┐    ┌──────────────────────────────────┐
│ ✅ SUFFICIENT       │    │ ❌ INSUFFICIENT CREDITS          │
│    CREDITS          │    │                                  │
└─────────────────────┘    ├──────────────────────────────────┤
        ↓                  │ 🔵 FUNCTION: Show Error Message  │
┌─────────────────────┐    │ ⚪ ►─ Exec                        │
│ Check Cargo Space   │    │ 🟣 ►─ Title: "Insufficient Cr"   │
│ [Next validation]   │    │ 🟣 ►─ Message: "Need {0} CR"     │
└─────────────────────┘    │                            ⚪ ═► │
                           ├──────────────────────────────────┤
                           │ 🔵 FUNCTION: Play Error Sound    │
                           │ ⚪ ►─ Exec                        │
                           │ 🔵 ►─ Sound: SFX_Error           │
                           │                            ⚪ ═► │
                           └──────────────────────────────────┘
                                       ↓
                                   [END]
```

---

## 🎨 Variable Nodes with Colors

### Getting and Setting Variables

```
┌────────────────────────────────────────────────────────────────┐
│ 🟦 VARIABLE GET: Player Credits                                │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ Type: int32                                                │ │
│ │ Category: Trading                                          │ │
│ └────────────────────────────────────────────────────────────┘ │
│                                    Player Credits 🟢 ─►       │
└────────────────────────────────────────────────────────────────┘


┌────────────────────────────────────────────────────────────────┐
│ 🟦 VARIABLE SET: Player Credits                                │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ Updates player's credit balance                            │ │
│ └────────────────────────────────────────────────────────────┘ │
│ ⚪ ►─ Exec                                                      │
│ 🟢 ►─ Player Credits: New Value                                │
│                                                          ⚪ ═► │
│                            Player Credits (out) 🟢 ─►         │
└────────────────────────────────────────────────────────────────┘
```

---

## 🔄 Loop Structures with Details

### ForEach Loop with Index

```
╔════════════════════════════════════════════════════════════════╗
║ 🟠 FLOW CONTROL: ForEach Loop                                  ║
╠════════════════════════════════════════════════════════════════╣
║ ┌────────────────────────────────────────────────────────────┐ ║
║ │ Iterates through shopping cart items                       │ ║
║ │ Updates total price for each item                          │ ║
║ └────────────────────────────────────────────────────────────┘ ║
║                                                                ║
║ INPUT PINS:                                                    ║
║   ⚪ ►─ Exec                                                   ║
║   🔵 ►─ Array: ShoppingCart (Map Keys)                         ║
║                                                                ║
║ OUTPUT PINS:                                                   ║
║   Loop Body ⚪ ─►  [Execute for each item]                    ║
║   Completed ⚪ ─►  [Execute when loop finishes]               ║
║                                                                ║
║ LOOP VARIABLES:                                                ║
║   Array Element (TradeItemDataAsset*) 🔵 ─►                   ║
║   Array Index (int32) 🟢 ─►  [Current iteration]              ║
╚════════════════════════════════════════════════════════════════╝
                │                                  │
                │ Loop Body                        │ Completed
                ↓                                  ↓
        [Process each item]                    [Continue]
```

### While Loop with Break

```
╔════════════════════════════════════════════════════════════════╗
║ 🟠 FLOW CONTROL: While Loop                                    ║
╠════════════════════════════════════════════════════════════════╣
║ ⚪ ►─ Exec                                                      ║
║ 🔴 ►─ Condition: (Counter < MaxAttempts)                       ║
║                                           Loop Body ⚪ ─►       ║
║                                          Completed ⚪ ─►        ║
╚════════════════════════════════════════════════════════════════╝
                │
                │ Loop Body
                ↓
        ┌───────────────────────┐
        │ [Retry logic]         │
        │         ↓              │
        │   ╔═══════════════╗   │
        │   ║ 🟠 BREAK      ║   │ ← Exits loop early
        │   ╠═══════════════╣   │
        │   ║ ⚪ ►─ Exec     ║   │
        │   ╚═══════════════╝   │
        └───────────────────────┘
```

---

## 🎭 Macro Nodes

### Custom Macro Example

```
╔════════════════════════════════════════════════════════════════╗
║ 🟣 MACRO: Calculate Profit Margin                              ║
╠════════════════════════════════════════════════════════════════╣
║ ┌────────────────────────────────────────────────────────────┐ ║
║ │ Collapsed graph containing profit calculation logic        │ ║
║ │ Double-click to expand and view internal nodes             │ ║
║ └────────────────────────────────────────────────────────────┘ ║
║                                                                ║
║ INPUT PINS:                                                    ║
║   ⚪ ►─ Exec                                                   ║
║   🟢 ►─ Buy Price (float)                                      ║
║   🟢 ►─ Sell Price (float)                                     ║
║   🟢 ►─ Quantity (int32)                                       ║
║                                                                ║
║ OUTPUT PINS:                                                   ║
║   ⚪ ─► Exec                                                   ║
║   🟢 ─► Profit (float)                                         ║
║   🟢 ─► Margin Percent (float)                                 ║
╚════════════════════════════════════════════════════════════════╝
```

---

## 📞 Event Dispatcher / Delegate Nodes

### Binding to Event

```
╔════════════════════════════════════════════════════════════════╗
║ 🟥 DELEGATE: Bind Event to OnCreditsChanged                    ║
╠════════════════════════════════════════════════════════════════╣
║ ┌────────────────────────────────────────────────────────────┐ ║
║ │ Binds a function to be called when credits change          │ ║
║ └────────────────────────────────────────────────────────────┘ ║
║ ⚪ ►─ Exec                                                      ║
║ 🔵 ►─ Target: PlayerTraderComponent                            ║
║ 🟥 ►─ Event: HandleCreditsChanged (Function)                   ║
║                                                          ⚪ ═► ║
╚════════════════════════════════════════════════════════════════╝
```

### Broadcasting Event

```
╔════════════════════════════════════════════════════════════════╗
║ 🟥 DELEGATE: Call OnTradeCompleted                             ║
╠════════════════════════════════════════════════════════════════╣
║ ┌────────────────────────────────────────────────────────────┐ ║
║ │ Broadcasts event to all bound listeners                    │ ║
║ └────────────────────────────────────────────────────────────┘ ║
║ ⚪ ►─ Exec                                                      ║
║ 🔴 ►─ Success (bool)                                            ║
║ 🟣 ►─ Error Message (Text)                                      ║
║                                                          ⚪ ═► ║
╚════════════════════════════════════════════════════════════════╝
```

---

## 🎯 Complete Real-World Example: Transaction System

```
═══════════════════════════════════════════════════════════════════════
                        Execute Trade Transaction
═══════════════════════════════════════════════════════════════════════

┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ 🔴 EVENT: OnConfirmTransactionClicked                           ┃
┃ ┌──────────────────────────────────────────────────────────────┐ ┃
┃ │ User confirmed the transaction in the UI                    │ ┃
┃ └──────────────────────────────────────────────────────────────┘ ┃
┃                                                            ⚪ ═► ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
                              ↓
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ 🔵 FUNCTION: Get Cart Total                                     ┃
┃ ┌──────────────────────────────────────────────────────────────┐ ┃
┃ │ Calculates total cost of all items in shopping cart         │ ┃
┃ └──────────────────────────────────────────────────────────────┘ ┃
┃ ⚪ ►─ Exec                                                        ┃
┃                                                            ⚪ ═► ┃
┃                                    Total Cost (int) 🟢 ─►       ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
                              ↓
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ 🔵 FUNCTION: Validate Transaction                               ┃
┃ ┌──────────────────────────────────────────────────────────────┐ ┃
┃ │ Checks credits, cargo space, and item availability          │ ┃
┃ └──────────────────────────────────────────────────────────────┘ ┃
┃ ⚪ ►─ Exec                                                        ┃
┃                                                            ⚪ ═► ┃
┃                                     Is Valid (bool) 🔴 ─►       ┃
┃                               Error Message (Text) 🟣 ─►        ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
                              ↓
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ ⚪ BRANCH: Validation Result                                     ┃
┃ ⚪ ►─ Exec                                                        ┃
┃ 🔴 ►─ Condition: Is Valid                                        ┃
┃                                                True ⚪ ─►        ┃
┃                                               False ⚪ ─►         ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
        │                                   │
        │ ✅ TRUE                           │ ❌ FALSE
        ↓                                   ↓
┌───────────────────────────┐     ┌────────────────────────────────┐
│ EXECUTE TRANSACTION       │     │ HANDLE ERROR                   │
└───────────────────────────┘     └────────────────────────────────┘
        ↓                                   ↓
╔═══════════════════════════╗     ╔════════════════════════════════╗
║ 🟠 ForEach: Cart Items    ║     ║ 🔵 Show Error Message          ║
╠═══════════════════════════╣     ╠════════════════════════════════╣
║ ⚪ ►─ Exec                 ║     ║ ⚪ ►─ Exec                      ║
║ 🔵 ►─ Array: Shopping Cart║     ║ 🟣 ►─ Title: "Error"           ║
║         Loop Body ⚪ ─►    ║     ║ 🟣 ►─ Message: Error Message   ║
║        Completed ⚪ ─►     ║     ║                          ⚪ ═► ║
║  Array Element 🔵 ─►      ║     ╚════════════════════════════════╝
╚═══════════════════════════╝               ↓
        │            │               ╔════════════════════════════════╗
        │            │               ║ 🔵 Play Error Sound            ║
        │ Loop Body  │               ╠════════════════════════════════╣
        │      ↓     │               ║ ⚪ ►─ Exec                      ║
        │   ┌────────────────┐      ║ 🔵 ►─ Sound: SFX_Error         ║
        │   │ 🔵 Buy Item    │      ║                          ⚪ ═► ║
        │   ├────────────────┤      ╚════════════════════════════════╝
        │   │ ⚪ ►─ Exec      │               ↓
        │   │ 🔵 ►─ Item     │          [Return false]
        │   │ 🟢 ►─ Quantity │
        │   │          ⚪ ═► │
        │   └────────────────┘
        │         ↓
        │   [Loop continues]
        │
        └─ Completed ─►
                ↓
        ╔════════════════════════════════╗
        ║ 🔵 Clear Shopping Cart         ║
        ╠════════════════════════════════╣
        ║ ⚪ ►─ Exec                      ║
        ║                          ⚪ ═► ║
        ╚════════════════════════════════╝
                ↓
        ╔════════════════════════════════╗
        ║ 🔵 Play Success Sound          ║
        ╠════════════════════════════════╣
        ║ ⚪ ►─ Exec                      ║
        ║ 🔵 ►─ Sound: SFX_Success       ║
        ║                          ⚪ ═► ║
        ╚════════════════════════════════╝
                ↓
        ╔════════════════════════════════╗
        ║ 🔵 Show Success Message        ║
        ╠════════════════════════════════╣
        ║ ⚪ ►─ Exec                      ║
        ║ 🟣 ►─ Title: "Success!"        ║
        ║ 🟣 ►─ Message: "Trade Complete"║
        ║                          ⚪ ═► ║
        ╚════════════════════════════════╝
                ↓
        ╔════════════════════════════════╗
        ║ 🟥 Broadcast: OnTradeCompleted ║
        ╠════════════════════════════════╣
        ║ ⚪ ►─ Exec                      ║
        ║ 🔴 ►─ Success: true            ║
        ║ 🟣 ►─ Error: ""                ║
        ║                          ⚪ ═► ║
        ╚════════════════════════════════╝
                ↓
           [Return true]
```

---

## 📋 Quick Reference: Node Colors

### Copy-Paste Color Codes

```markdown
🔴 Red     - Events, Boolean pins
🔵 Blue    - Functions, Object references
🟢 Green   - Pure functions, Numeric pins (int/float)
⚪ White   - Branch nodes, Exec pins
🟠 Orange  - Flow control (loops, sequences)
🟦 Light Blue - Variables (get/set)
🟣 Purple  - Macros, String/Text pins
🟥 Dark Red - Delegates/Events
🟡 Yellow  - Vector/Location pins
🟤 Brown   - Struct pins
```

### Pin Symbols

```
⚪ ►─  Input Exec pin (white arrow going in)
⚪ ═►  Output Exec pin (white double line going out)
🔵 ►─  Input Data pin (colored, type-specific)
🔵 ─►  Output Data pin (colored, type-specific)
```

---

## 💡 Best Practices

### DO ✅

1. **Use colored node boxes** to match Blueprint editor
2. **Show pin types** with color coding
3. **Include node descriptions** in comment boxes
4. **Display all relevant pins** (in and out)
5. **Use proper exec flow symbols** (⚪ for white exec pins)

### DON'T ❌

1. **Don't use plain text** without color indicators
2. **Don't omit pin types** - they're crucial for implementation
3. **Don't forget exec pins** - they show execution flow
4. **Don't skip node borders** - boxes help distinguish nodes
5. **Don't use unclear symbols** - stick to the legend

---

## 🎨 ASCII Art Node Library

### Event Node Template

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ 🔴 EVENT: [Event Name]                ┃
┃ ┌────────────────────────────────────┐ ┃
┃ │ [Description of when this fires]   │ ┃
┃ └────────────────────────────────────┘ ┃
┃ [Input pins if any]                    ┃
┃                                  ⚪ ═► ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

### Function Node Template

```
╔════════════════════════════════════════╗
║ 🔵 FUNCTION: [Function Name]           ║
╠════════════════════════════════════════╣
║ ┌────────────────────────────────────┐ ║
║ │ [What this function does]          │ ║
║ └────────────────────────────────────┘ ║
║ ⚪ ►─ Exec                              ║
║ [Type] ►─ [Input Pin Name]: [Value]   ║
║                              ⚪ ═►     ║
║                [Return Name] ─►  [Type]║
╚════════════════════════════════════════╝
```

### Branch Node Template

```
╔════════════════════════════════════════╗
║ ⚪ BRANCH: [Condition Description]     ║
╠════════════════════════════════════════╣
║ ⚪ ►─ Exec                              ║
║ 🔴 ►─ Condition: [Boolean expression]  ║
║                        True ⚪ ─►      ║
║                       False ⚪ ─►       ║
╚════════════════════════════════════════╝
```

---

## 📸 Visual Comparison

### Before (Simple Text)

```
Event OnButtonClicked
  ↓
Get Player Credits
  ↓
Show Message
```

### After (Enhanced with Colors and Details)

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ 🔴 EVENT: OnButtonClicked              ┃
┃                                  ⚪ ═► ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
                ↓
╔════════════════════════════════════════╗
║ 🟢 PURE: Get Player Credits            ║
╠════════════════════════════════════════╣
║ 🔵 ►─ Target: PlayerTrader             ║
║                 Credits (int) 🟢 ─►    ║
╚════════════════════════════════════════╝
                ↓
╔════════════════════════════════════════╗
║ 🔵 FUNCTION: Show Message              ║
╠════════════════════════════════════════╣
║ ⚪ ►─ Exec                              ║
║ 🟣 ►─ Message: Credits value           ║
║                              ⚪ ═►     ║
╚════════════════════════════════════════╝
```

**Key Improvements:**
- ✅ Color-coded node types (🔴 red event, 🟢 green pure, 🔵 blue function)
- ✅ Pin types shown with colors (⚪ exec, 🔵 object, 🟢 numeric, 🟣 text)
- ✅ Exec flow clearly marked (►─ and ═►)
- ✅ Node borders distinguish different nodes
- ✅ Matches actual Blueprint appearance

---

## 🔗 Integration with Existing Documentation

This enhanced format supplements `BLUEPRINT_MOCKUP_GUIDE.md` with visual accuracy closer to actual Unreal Engine Blueprints.

**Use this format when:**
- Creating implementation guides for designers
- Documenting complex Blueprint logic
- Training new team members on Blueprint systems
- Reviewing Blueprint changes in PRs

**Related Documentation:**
- [BLUEPRINT_MOCKUP_GUIDE.md](BLUEPRINT_MOCKUP_GUIDE.md) - Original format guide
- [BLUEPRINT_DOCUMENTATION_EXAMPLES.md](BLUEPRINT_DOCUMENTATION_EXAMPLES.md) - Before/after examples
- [BLUEPRINT_QUICK_REFERENCE.md](BLUEPRINT_QUICK_REFERENCE.md) - Blueprint overview

---

**Last Updated**: December 29, 2025
**Maintained by**: Adastrea Development Team
**Feedback**: Submit improvements via GitHub issues
