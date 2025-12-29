# Blueprint Documentation - Before vs After Examples

> **Demonstrating the improvement from text-based to visual Blueprint documentation**

**Last Updated**: December 29, 2025

---

## Purpose

This document shows side-by-side comparisons of Blueprint documentation styles, demonstrating why **visual flow diagrams are superior** to text descriptions.

---

## Example 1: Market Inventory Update

### ❌ OLD STYLE (Text-Based)

**Text Description:**
> When the market inventory is updated, clear the item list children, then call the GetAvailableItems C++ function. For each item returned, create a text widget, set its text to show the item name and price in the format "[ItemName] - [Price] CR", and add it to the ItemList.

**Problems:**
- Hard to visualize execution flow
- Unclear what happens between steps
- Ambiguous about error handling
- No indication of function parameters
- Difficult to translate to actual Blueprint

### ✅ NEW STYLE (Visual Flow)

```
Event OnMarketInventoryUpdated
  ↓
Clear Children (ScrollBox_ItemList)
  ↓
Get Available Items
  └─ Call: TradingInterface->GetAvailableItems()
     └─ Return: Array<MarketInventoryEntry>
        ↓
ForEach Loop
  ├─ Array: Available Items
  └─ Loop Body
       ↓
     Create Widget
       ├─ Class: WBP_ItemListEntry
       └─ Return: ItemWidget
          ↓
     Set Item Data
       ├─ Item Name: Entry.TradeItem.ItemName
       ├─ Price: Entry.CurrentPrice
       └─ Format: "{0} - {1} CR"
          ↓
     Add Child to ScrollBox
       ├─ Parent: ScrollBox_ItemList
       └─ Content: ItemWidget
          ↓
     [Loop continues]
  ↓
Completed
```

**Benefits:**
- ✅ Clear execution flow
- ✅ Shows exact function calls
- ✅ Parameter and return values visible
- ✅ Loop structure obvious
- ✅ Easy to implement in Blueprint

---

## Example 2: Buy Item Validation

### ❌ OLD STYLE (Text-Based)

**Text Description:**
> When the buy button is clicked, check if the player has enough credits. If yes, check if there's enough cargo space. If both checks pass, add the item to the player's inventory, deduct the credits, and show a success message. Otherwise, show an appropriate error message.

**Problems:**
- Sequential checks not clear
- No indication of which error shows when
- Missing details about actual operations
- No mention of UI feedback
- Unclear order of operations

### ✅ NEW STYLE (Visual Flow)

```
Event OnBuyButtonClicked
  Inputs:
    - Item (TradeItemDataAsset)
    - Quantity (int32)
  ↓
Get Item Price
  └─ Call: GetItemPrice(Item, Quantity)
     └─ Return: TotalPrice (int32)
        ↓
Get Player Credits
  └─ Call: GetPlayerCredits()
     └─ Return: Credits (int32)
        ↓
Branch (Credits >= TotalPrice?)
  ├─ False → Insufficient Credits
  │    ↓
  │  Show Error Message
  │    └─ Text: "Insufficient Credits"
  │       ├─ Have: {Credits} CR
  │       └─ Need: {TotalPrice} CR
  │          ↓
  │  Play Error Sound
  │    └─ Sound: SFX_Error
  │       ↓
  │  Highlight Credits Display (Red)
  │    ↓
  │  END
  │
  └─ True → Check Cargo Space
       ↓
     Get Required Space
       └─ Space = Item.VolumePerUnit × Quantity
          ↓
     Get Available Cargo Space
       └─ Call: GetCargoSpaceInfo()
          ├─ Out: UsedSpace
          ├─ Out: TotalSpace
          └─ Out: AvailableSpace
             ↓
     Branch (AvailableSpace >= RequiredSpace?)
       ├─ False → Insufficient Space
       │    ↓
       │  Show Error Message
       │    └─ Text: "Insufficient Cargo Space"
       │       ├─ Have: {AvailableSpace} tons
       │       └─ Need: {RequiredSpace} tons
       │          ↓
       │  Play Error Sound
       │    ↓
       │  END
       │
       └─ True → Execute Purchase
            ↓
          Deduct Credits
            └─ NewCredits = Credits - TotalPrice
               ↓
          Add to Inventory
            └─ Call: AddItemToInventory(Item, Quantity)
               ↓
          Update Market Stock
            └─ Call: Market->RecordSale(Item, Quantity)
               ↓
          Show Success Message
            └─ Text: "Purchase Complete!"
               ├─ Item: {Item.ItemName}
               ├─ Quantity: {Quantity}
               └─ Spent: {TotalPrice} CR
                  ↓
          Play Success Sound
            └─ Sound: SFX_Purchase
               ↓
          Refresh All Displays
            ├─ Update Credits Display
            ├─ Update Cargo Display
            └─ Update Inventory Display
               ↓
          END
```

**Benefits:**
- ✅ Both validation paths clearly shown
- ✅ Error messages specified
- ✅ Sound effects documented
- ✅ UI updates explicit
- ✅ Order of operations unambiguous

---

## Example 3: Simple Function Call

### ❌ OLD STYLE (Text-Based)

**Text Description:**
> Call the CalculateTotalPrice function to get the total price of all items in the shopping cart.

**Problems:**
- No context about when this happens
- Missing return value handling
- No indication of what to do with result
- Too vague for implementation

### ✅ NEW STYLE (Visual Flow)

```
Function GetCartTotal
  Returns: int32
  ↓
Set Total = 0
  ↓
ForEach (Item, Quantity in ShoppingCart)
  ├─ Map: ShoppingCart<TradeItemDataAsset*, int32>
  └─ Loop Body
       ↓
     Get Item Price
       ├─ Item: Current Item
       ├─ Quantity: Current Quantity
       └─ Return: ItemTotal (int32)
          ↓
     Add to Total
       └─ Total = Total + ItemTotal
          ↓
     [Loop continues]
  ↓
Completed
  ↓
Return Total
```

**Benefits:**
- ✅ Clear function signature
- ✅ Shows loop structure
- ✅ Accumulator pattern visible
- ✅ Return value explicit

---

## Example 4: Event Binding

### ❌ OLD STYLE (Text-Based)

**Text Description:**
> In the BeginPlay event, get the PlayerTraderComponent and bind to its OnCreditsChanged event so the UI can update when credits change.

**Problems:**
- No null checking mentioned
- Missing bind function details
- Unclear what the callback does
- No error handling

### ✅ NEW STYLE (Visual Flow)

```
Event BeginPlay
  ↓
Get Owner
  └─ Return: Actor Reference
     ↓
Cast to ASpaceship
  ├─ Success → Valid Ship
  │    ↓
  │  Get Component by Class
  │    ├─ Class: UPlayerTraderComponent
  │    └─ Return: TraderComponent
  │       ↓
  │  Is Valid? (TraderComponent)
  │    ├─ True → Bind Event
  │    │    ↓
  │    │  Store Reference
  │    │    └─ PlayerTrader = TraderComponent
  │    │       ↓
  │    │  Bind Event to Delegate
  │    │    ├─ Event: OnCreditsChanged
  │    │    ├─ Object: self
  │    │    └─ Function: HandleCreditsChanged
  │    │       ↓
  │    │  Log: "Trader component bound"
  │    │
  │    └─ False → Error
  │         ↓
  │       Log Error: "PlayerTraderComponent not found"
  │         ↓
  │       Show Warning UI
  │
  └─ Failed → Not a Ship
       ↓
     Log Error: "Owner is not a Spaceship"

────────────────────────────────────────

Function HandleCreditsChanged (Callback)
  Inputs:
    - NewCredits (int32)
    - ChangeAmount (int32)
  ↓
Update Credits Display
  ├─ Set Text: "{NewCredits} CR"
  └─ Format: Add commas for thousands
     ↓
Animate Change
  ├─ Branch (ChangeAmount < 0)
  │   ├─ True → Play "Spent" Animation
  │   │    └─ Color: Red flash
  │   └─ False → Play "Gained" Animation
  │        └─ Color: Green flash
  │
  └─ Play Sound
       ├─ If Spent: SFX_MoneyOut
       └─ If Gained: SFX_MoneyIn
```

**Benefits:**
- ✅ Null safety checks shown
- ✅ Error handling documented
- ✅ Both success and fail paths clear
- ✅ Callback implementation included
- ✅ UI feedback specified

---

## Example 5: Widget Designer Layout

### ❌ OLD STYLE (Text-Based)

**Text Description:**
> Create a widget with a vertical box containing a text block for the title, a horizontal box with buttons for Buy and Sell, and a scroll box for the item list.

**Problems:**
- No hierarchy clear
- Widget names not specified
- Properties not mentioned
- No binding information

### ✅ NEW STYLE (Visual Layout + Properties)

```
═══════════════════════════════════════
Widget: WBP_TradingInterface
Parent: UTradingInterfaceWidget
═══════════════════════════════════════

WIDGET HIERARCHY:
─────────────────────────────────────
Canvas Panel (Root)
├── Image_Background
│   ├── Anchors: Fill Screen
│   ├── Color: (0, 0, 0, 0.85)
│   └── Z-Order: 0
│
├── VerticalBox_Main
│   ├── Alignment: Center
│   ├── Padding: 50
│   │
│   ├── TextBlock_Title
│   │   ├── Text: "Trading Interface"
│   │   ├── Font Size: 48
│   │   ├── Color: White
│   │   ├── Is Variable: ✓
│   │   └── Alignment: Center
│   │
│   ├── HorizontalBox_Controls
│   │   ├── Padding: 10
│   │   │
│   │   ├── Button_BuyMode
│   │   │   ├── Text: "Buy"
│   │   │   ├── Size: 200 x 60
│   │   │   ├── Is Variable: ✓
│   │   │   └── OnClicked: Event_OnBuyModeClicked
│   │   │
│   │   ├── Spacer (Width: 20)
│   │   │
│   │   └── Button_SellMode
│   │       ├── Text: "Sell"
│   │       ├── Size: 200 x 60
│   │       ├── Is Variable: ✓
│   │       └── OnClicked: Event_OnSellModeClicked
│   │
│   ├── ScrollBox_ItemList
│   │   ├── Style: Modern
│   │   ├── Orientation: Vertical
│   │   ├── Is Variable: ✓ (Important!)
│   │   └── Contains: Dynamic child widgets
│   │
│   └── HorizontalBox_Footer
│       ├── TextBlock_Credits
│       │   ├── Text: "Credits: 0 CR"
│       │   ├── Font Size: 24
│       │   └── Is Variable: ✓
│       │
│       ├── Spacer
│       │
│       └── TextBlock_CargoSpace
│           ├── Text: "Cargo: 0/100 tons"
│           ├── Font Size: 24
│           └── Is Variable: ✓

REQUIRED VARIABLES:
─────────────────────────────────────
✓ TextBlock_Title (TextBlock*)
✓ Button_BuyMode (Button*)
✓ Button_SellMode (Button*)
✓ ScrollBox_ItemList (ScrollBox*)
✓ TextBlock_Credits (TextBlock*)
✓ TextBlock_CargoSpace (TextBlock*)

WIDGET BINDINGS:
─────────────────────────────────────
Event Graph:
  ├── Button_BuyMode.OnClicked
  │   → ToggleBuySellView()
  │
  └── Button_SellMode.OnClicked
      → ToggleBuySellView()
```

**Benefits:**
- ✅ Complete hierarchy visible
- ✅ Widget names specified
- ✅ Properties documented
- ✅ Variables marked
- ✅ Event bindings listed

---

## Example 6: Complex State Machine

### ❌ OLD STYLE (Text-Based)

**Text Description:**
> The trading state machine has three states: Browsing, CartReview, and Checkout. When in Browsing state, users can add items to cart. In CartReview, they can modify quantities. In Checkout, the transaction is processed.

**Problems:**
- State transitions unclear
- No trigger events specified
- Actions per state vague
- Exit conditions missing

### ✅ NEW STYLE (Visual State Machine)

```
═══════════════════════════════════════════════════
TRADING STATE MACHINE
═══════════════════════════════════════════════════

STATE: Browsing
─────────────────────────────────────
Allowed Actions:
  ├── View Items
  ├── Add to Cart
  ├── Remove from Cart
  └── View Item Details

Transitions:
  ├── [Cart Not Empty] + [View Cart Button]
  │   → STATE: CartReview
  │
  └── [Close Button]
      → Exit Interface

Entry Actions:
  ├── Refresh Item List
  ├── Update Filters
  └── Show All UI Elements

Exit Actions:
  └── Save current scroll position

─────────────────────────────────────

STATE: CartReview
─────────────────────────────────────
Allowed Actions:
  ├── Modify Quantity
  ├── Remove Item
  ├── View Item Details
  └── Calculate Total

Transitions:
  ├── [Back Button]
  │   → STATE: Browsing
  │
  ├── [Checkout Button] + [Valid Transaction]
  │   → STATE: Checkout
  │
  └── [Cart Empty] (auto)
      → STATE: Browsing

Entry Actions:
  ├── Display Cart Summary
  ├── Calculate Totals
  │   ├── Subtotal
  │   ├── Tax
  │   └── Total Cost
  └── Show Checkout Button

Exit Actions:
  └── Clear temporary calculations

Validation:
  ├── Check Credits >= Total Cost
  ├── Check Cargo Space >= Required
  └── Check All Items In Stock

─────────────────────────────────────

STATE: Checkout
─────────────────────────────────────
Allowed Actions:
  ├── Confirm Transaction
  └── Cancel

Transitions:
  ├── [Confirm Button] + [Success]
  │   → STATE: Complete → Exit
  │
  ├── [Confirm Button] + [Failed]
  │   → STATE: CartReview (show error)
  │
  └── [Cancel Button]
      → STATE: CartReview

Entry Actions:
  ├── Disable UI interaction
  ├── Show Confirmation Dialog
  │   ├── Transaction Summary
  │   ├── Final Total
  │   └── Confirm/Cancel buttons
  └── Start Timer (auto-cancel)

Processing:
  ├── Lock Market Prices
  ├── Reserve Cargo Space
  ├── Execute Transaction
  │   ├── Deduct Credits
  │   ├── Add Items to Cargo
  │   └── Update Market Stock
  ├── Broadcast Events
  │   └── OnTradeCompleted
  └── Cleanup

Exit Actions:
  ├── Release Price Lock
  ├── Release Cargo Reservation
  ├── Clear Cart
  └── Show Result Message

─────────────────────────────────────

STATE: Complete (Transient)
─────────────────────────────────────
Actions:
  ├── Display Success Message
  ├── Play Success Animation
  ├── Play Success Sound
  └── Auto-close (2 seconds)

Transitions:
  └── [Timer Complete]
      → Exit Interface
```

**Benefits:**
- ✅ All states documented
- ✅ Transitions explicit with conditions
- ✅ Actions per state listed
- ✅ Entry/exit actions clear
- ✅ Validation rules shown

---

## Example 7: Error Handling Pattern

### ❌ OLD STYLE (Text-Based)

**Text Description:**
> Handle errors appropriately when the transaction fails.

**Problems:**
- "Appropriately" is vague
- No specific errors listed
- Recovery actions unclear
- User feedback not specified

### ✅ NEW STYLE (Visual Error Handling)

```
Function ExecuteTrade
  Returns: bool
  ↓
Validate Transaction
  └─ Call: ValidateTransaction(OutErrorMessage)
     ├─ Out: ErrorMessage (FText)
     └─ Return: IsValid (bool)
        ↓
Branch (IsValid?)
  ├─ False → Handle Validation Error
  │    ↓
  │  ┌────────────────────────────────┐
  │  │   ERROR HANDLING SECTION       │
  │  └────────────────────────────────┘
  │    ↓
  │  Determine Error Type
  │    ↓
  │  Switch (Error Type)
  │    ├─ InsufficientCredits
  │    │    ├─ Show Message
  │    │    │   └─ "Not enough credits"
  │    │    ├─ Highlight Credits (Red)
  │    │    ├─ Play Sound: SFX_Error
  │    │    ├─ Suggest Actions
  │    │    │   └─ "Sell items or reduce quantity"
  │    │    └─ Focus Credits Display
  │    │
  │    ├─ InsufficientCargoSpace
  │    │    ├─ Show Message
  │    │    │   └─ "Not enough cargo space"
  │    │    ├─ Highlight Cargo Bar (Red)
  │    │    ├─ Play Sound: SFX_Error
  │    │    ├─ Suggest Actions
  │    │    │   └─ "Sell cargo or upgrade ship"
  │    │    └─ Show Space Calculator
  │    │
  │    ├─ ItemOutOfStock
  │    │    ├─ Show Message
  │    │    │   └─ "Item no longer available"
  │    │    ├─ Remove from Cart
  │    │    ├─ Play Sound: SFX_Warning
  │    │    ├─ Refresh Market Display
  │    │    └─ Suggest Alternatives
  │    │
  │    ├─ PriceChanged
  │    │    ├─ Show Message
  │    │    │   └─ "Prices have changed"
  │    │    ├─ Display Price Comparison
  │    │    │   ├─ Old Price: {OldPrice}
  │    │    │   └─ New Price: {NewPrice}
  │    │    ├─ Play Sound: SFX_Warning
  │    │    ├─ Update Cart Prices
  │    │    └─ Ask for Confirmation
  │    │
  │    └─ NetworkError
  │         ├─ Show Message
  │         │   └─ "Connection lost"
  │         ├─ Play Sound: SFX_Error
  │         ├─ Preserve Cart
  │         ├─ Show Retry Button
  │         └─ Disable Transaction
  │    ↓
  │  Log Error
  │    ├─ Category: LogTrading
  │    ├─ Level: Warning
  │    └─ Message: ErrorMessage
  │       ↓
  │  Broadcast Event
  │    └─ OnTradeCompleted
  │       ├─ Success: false
  │       └─ Error: ErrorMessage
  │          ↓
  │  Return: false
  │
  └─ True → Execute Transaction
       ↓
     [Transaction logic...]
       ↓
     Return: true
```

**Benefits:**
- ✅ All error types handled
- ✅ Specific user feedback per error
- ✅ Recovery suggestions provided
- ✅ Visual feedback specified
- ✅ Logging included

---

## Key Differences Summary

| Aspect | ❌ Text-Based | ✅ Visual Flow |
|--------|--------------|---------------|
| **Clarity** | Ambiguous, requires interpretation | Clear, unambiguous execution path |
| **Implementation** | Hard to translate to Blueprints | Direct 1:1 mapping |
| **Error Handling** | Often omitted or vague | Explicitly shown with branches |
| **Data Flow** | Implied, not visible | Clear inputs/outputs shown |
| **Complexity** | Gets confusing quickly | Scales well with complexity |
| **Updates** | Easy to become outdated | Structure forces accuracy |
| **Learning Curve** | Requires reading comprehension | Visual pattern recognition |
| **Detail Level** | Either too vague or too verbose | Balanced, appropriate detail |

---

## Usage Guidelines

### When Creating New Documentation

1. **Start with visual flow diagram**
2. Add text explanations only where needed
3. Include input/output types
4. Show error handling branches
5. Document UI feedback
6. Add real example values

### When Updating Existing Documentation

1. Identify text-heavy sections
2. Convert to visual flow diagrams
3. Preserve important details
4. Test readability
5. Get feedback from users

---

## Related Documentation

- [BLUEPRINT_MOCKUP_GUIDE.md](BLUEPRINT_MOCKUP_GUIDE.md) - Complete visual format guide
- [BLUEPRINT_QUICK_REFERENCE.md](BLUEPRINT_QUICK_REFERENCE.md) - Blueprint overview
- [TradingSystemBlueprintGuide.md](../../Blueprints/TradingSystemBlueprintGuide.md) - Trading examples

---

**Conclusion**: Visual flow diagrams make Blueprint documentation **clearer, more accurate, and easier to implement**. Always prefer visual over text-only descriptions.

---

**Last Updated**: December 29, 2025  
**Maintained by**: Adastrea Development Team
