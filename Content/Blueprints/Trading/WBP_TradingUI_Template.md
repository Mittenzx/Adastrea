# Trading UI Widget - Starter Template

Widget Blueprint template for the trading interface in Trade Simulator MVP.

## WBP_TradingUI - Widget Structure

### Widget Hierarchy

```
Canvas Panel (Root)
├─ Overlay (Background)
│  └─ Image (Dark background, 80% opacity)
│
├─ Vertical Box (Main Content)
│  ├─ Horizontal Box (Header)
│  │  ├─ Text Block: Station Name
│  │  └─ Button: Close (X)
│  │
│  ├─ Horizontal Box (Player Info)
│  │  ├─ Text Block: "Credits:"
│  │  ├─ Text Block: PlayerCreditsDisplay (bound)
│  │  ├─ Spacer
│  │  ├─ Text Block: "Cargo:"
│  │  ├─ Text Block: CargoSpaceDisplay (bound)
│  │  └─ Progress Bar: CargoBar (bound)
│  │
│  ├─ Scroll Box (Item List)
│  │  └─ [Dynamic - populated with WBP_TradeItemRow widgets]
│  │
│  └─ Horizontal Box (Footer)
│     ├─ Text Block: "Total Selected:"
│     └─ Text Block: TotalCostDisplay
│
└─ (Spacer for margins)
```

## Widget Variables

### References (Set from Trading Ship)

#### Market Data
```
Variable Name: MarketData
Type: UMarketDataAsset (Object Reference)
Category: Trading
Instance Editable: false
Expose on Spawn: true
```

#### Cargo Component
```
Variable Name: CargoComponent
Type: UCargoComponent (Object Reference)
Category: Trading
Instance Editable: false
Expose on Spawn: true
```

#### Player Credits Reference
```
Variable Name: PlayerCreditsRef
Type: Float (Reference)
Category: Trading
Instance Editable: false
Expose on Spawn: true
```

### Internal State

#### Current Station Name
```
Variable Name: StationName
Type: Text
Category: Display
Default Value: "Unknown Station"
```

#### Item List
```
Variable Name: ItemList
Type: TArray<UUserWidget*>
Category: Internal
Default Value: Empty Array
```

## Widget Events

### Event Construct (Initialization)

```
Event Construct
  → Branch: Is Valid? (MarketData)
     ├─ True:
     │   → Set StationName = MarketData->MarketName
     │   → Populate Item List
     │   → Update Display
     └─ False:
        → Print String: "ERROR: No Market Data provided"
```

### Event: Close Button Clicked

```
Event: OnClicked (Close Button)
  → Get Owning Player
  → Get Controlled Pawn
  → Cast to BP_TradingShip_Starter
  → Call: CloseTradingUI()
```

### Function: PopulateItemList

```
Function: PopulateItemList

Flow:
  1. Clear Children (Scroll Box)
  2. Get Inventory from MarketData
  3. For Each: InventoryEntry
     ├─ Create Widget: WBP_TradeItemRow
     ├─ Set Item Data:
     │  - Trade Item
     │  - Current Stock
     │  - Buy Price (MarketData->GetItemPrice(Item, true))
     │  - Sell Price (MarketData->GetItemPrice(Item, false))
     │  - Player Cargo Quantity
     ├─ Bind to OnBuyClicked event
     ├─ Bind to OnSellClicked event
     └─ Add Child to Scroll Box
```

### Function: UpdateDisplay

```
Function: UpdateDisplay

Flow:
  1. Set Text (PlayerCreditsDisplay):
     Format: "{PlayerCreditsRef} cr"
  
  2. Get Cargo Space:
     - Available = CargoComponent->GetAvailableCargoSpace()
     - Capacity = CargoComponent->CargoCapacity
     - Used = Capacity - Available
  
  3. Set Text (CargoSpaceDisplay):
     Format: "{Used} / {Capacity} m³"
  
  4. Set Progress Bar (CargoBar):
     Percent = Used / Capacity
  
  5. For Each: Item in ItemList
     → Call UpdateItemRow()
```

### Function: OnItemBuyClicked

```
Function: OnItemBuyClicked
Inputs:
  - Item (UTradeItemDataAsset*)
  - Quantity (int32)

Flow:
  1. Get Buy Price = MarketData->GetItemPrice(Item, true)
  2. Get Owning Player Pawn
  3. Cast to BP_TradingShip_Starter
  4. Call: BuyItem(Item, Quantity, BuyPrice)
  5. Branch: Purchase Successful?
     ├─ True:
     │   → Update Display
     │   → Play Sound: Purchase Success
     └─ False:
        → Play Sound: Purchase Failed
```

### Function: OnItemSellClicked

```
Function: OnItemSellClicked
Inputs:
  - Item (UTradeItemDataAsset*)
  - Quantity (int32)

Flow:
  1. Get Sell Price = MarketData->GetItemPrice(Item, false)
  2. Get Owning Player Pawn
  3. Cast to BP_TradingShip_Starter
  4. Call: SellItem(Item, Quantity, SellPrice)
  5. Branch: Sale Successful?
     ├─ True:
     │   → Update Display
     │   → Play Sound: Sale Success
     └─ False:
        → Play Sound: Sale Failed
```

## Binding Functions (Property Bindings)

### Get Player Credits Text

```
Function: GetPlayerCreditsText
Returns: Text

Flow:
  → Format Text: "{PlayerCreditsRef} cr"
  → Return formatted text
```

Bind this to PlayerCreditsDisplay → Text property

### Get Cargo Space Text

```
Function: GetCargoSpaceText
Returns: Text

Flow:
  1. Get Available = CargoComponent->GetAvailableCargoSpace()
  2. Get Capacity = CargoComponent->CargoCapacity
  3. Calculate Used = Capacity - Available
  4. Format Text: "{Used} / {Capacity} m³"
  5. Return formatted text
```

Bind this to CargoSpaceDisplay → Text property

### Get Cargo Bar Percent

```
Function: GetCargoBarPercent
Returns: Float

Flow:
  1. Get Available = CargoComponent->GetAvailableCargoSpace()
  2. Get Capacity = CargoComponent->CargoCapacity
  3. Calculate Used = Capacity - Available
  4. Calculate Percent = Used / Capacity
  5. Return Percent
```

Bind this to CargoBar → Percent property

## Styling Guidelines

### Colors

```
Background: Dark Gray (R:0.1, G:0.1, B:0.1, A:0.9)
Header: Medium Gray (R:0.2, G:0.2, B:0.2)
Text (Primary): White (R:1.0, G:1.0, B:1.0)
Text (Secondary): Light Gray (R:0.7, G:0.7, B:0.7)
Text (Credits): Gold (R:1.0, G:0.84, B:0.0)
Text (Cargo): Cyan (R:0.0, G:1.0, B:1.0)
Button (Buy): Green (R:0.0, G:0.8, B:0.0)
Button (Sell): Orange (R:1.0, G:0.6, B:0.0)
Button (Close): Red (R:0.8, G:0.0, B:0.0)
```

### Fonts

```
Header: Size 24, Bold
Body Text: Size 16, Normal
Small Text: Size 12, Normal
```

### Spacing

```
Padding (Outer): 20px
Padding (Inner): 10px
Margin (Between Elements): 5px
```

## Child Widget: WBP_TradeItemRow

This widget represents one item in the trade list.

### Structure

```
Horizontal Box (Root)
├─ Image: Item Icon (64x64)
├─ Vertical Box: Item Info
│  ├─ Text Block: Item Name
│  ├─ Text Block: Category
│  └─ Text Block: Volume/Unit
├─ Vertical Box: Prices
│  ├─ Text Block: Buy Price
│  └─ Text Block: Sell Price
├─ Vertical Box: Stock
│  ├─ Text Block: Market Stock
│  └─ Text Block: Player Cargo
├─ Horizontal Box: Actions
│  ├─ Button: Buy -10
│  ├─ Button: Buy -1
│  ├─ Button: Sell +1
│  └─ Button: Sell +10
└─ Spacer
```

### Variables

```
TradeItem: UTradeItemDataAsset*
CurrentStock: int32
BuyPrice: float
SellPrice: float
PlayerQuantity: int32
```

### Events

```
OnBuyClicked(Item, Quantity) - Dispatched to parent
OnSellClicked(Item, Quantity) - Dispatched to parent
```

## Testing the Widget

### In-Editor Preview

1. Open WBP_TradingUI in Widget Designer
2. Set Preview values:
   - PlayerCreditsRef = 5000
   - CargoComponent = (mock component)
   - MarketData = DA_Market_TradeHubMarket
3. Verify layout looks correct
4. Test button hover states

### In-Game Testing

1. Launch game with BP_TradingShip_Starter
2. Dock at station
3. Press T to open trading UI
4. Verify:
   - [ ] Station name displays
   - [ ] Credits show correctly
   - [ ] Cargo bar updates
   - [ ] Items list populates
   - [ ] Buy buttons work
   - [ ] Sell buttons work
   - [ ] Close button works
   - [ ] UI updates after transactions

## Common Issues

### "Items don't show in list"
**Check**:
1. MarketData has Inventory entries
2. PopulateItemList is called in Construct
3. WBP_TradeItemRow widget exists

### "Buttons don't respond"
**Check**:
1. Button OnClicked events are bound
2. Functions are public (not private)
3. Events are dispatched correctly

### "Cargo/Credits don't update"
**Check**:
1. Property bindings are set correctly
2. UpdateDisplay is called after transactions
3. References (CargoComponent, PlayerCreditsRef) are valid

### "UI doesn't close"
**Check**:
1. Close button calls CloseTradingUI on ship
2. Input mode is restored correctly
3. Widget is removed from viewport

## Visual Reference

See generated diagram: `docs/reference/images/blueprints/wbp_trading_ui_layout.svg`

## Optimization Tips

### For Better Performance

1. **Don't Update Every Frame**: Use Event-driven updates
2. **Cache Item Rows**: Don't recreate on every refresh
3. **Limit Bindings**: Use manual updates instead of constant bindings
4. **Pool Widgets**: Reuse WBP_TradeItemRow instances

### For Better UX

1. **Show Profit Margin**: Display buy vs sell price difference
2. **Highlight Affordable Items**: Green text for items player can buy
3. **Show Volume Requirements**: Warn when item won't fit
4. **Sort Options**: Allow sorting by price, name, category
5. **Search/Filter**: Add text filter for large item lists

## Next Steps

1. Create WBP_TradingUI widget in Unreal Editor
2. Create WBP_TradeItemRow child widget
3. Set up bindings and events
4. Style with colors and fonts
5. Test with BP_TradingShip_Starter
6. Iterate based on usability testing

---

**Last Updated**: January 17, 2026  
**For**: Trade Simulator MVP (Weeks 1-4)  
**Complexity**: Intermediate (requires UMG knowledge)
