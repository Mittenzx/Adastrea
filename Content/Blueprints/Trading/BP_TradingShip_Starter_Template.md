# Trading Ship Blueprint - Starter Template

This is a starter template for creating a trading ship Blueprint in Unreal Engine for the Trade Simulator MVP.

## BP_TradingShip_Starter - Class Configuration

### Parent Class
`ASpaceship` (or `APawn` if starting from scratch)

### Required Components

#### 1. CargoComponent
```
Component Name: CargoComponent
Class: UCargoComponent
```

**Properties** (Class Defaults):
- `CargoCapacity`: 10.0 (starter ship)

**Purpose**: Manages cargo inventory and space calculation

#### 2. PlayerTraderComponent  
```
Component Name: PlayerTraderComponent
Class: UPlayerTraderComponent
```

**Properties** (Class Defaults):
- (Uses default values)

**Purpose**: Handles buy/sell transactions with markets

#### 3. StaticMeshComponent (Ship Visual)
```
Component Name: ShipMesh
Class: UStaticMeshComponent
```

**Properties**:
- Mesh: Select your ship model
- Collision: Block All (for physics)

### Blueprint Variables

Add these variables in the Blueprint:

#### Player Credits
```
Variable Name: PlayerCredits
Type: Float
Category: Trading
Default Value: 1000.0
Instance Editable: false
Tooltip: "Player's current credit balance"
```

#### Current Docked Station
```
Variable Name: DockedStation
Type: ASpaceStation (Object Reference)
Category: Trading
Default Value: None
Tooltip: "Station the player is currently docked at"
```

#### Trading UI Widget
```
Variable Name: TradingUIWidget
Type: UWBPTradingUI (Widget Class Reference)
Category: UI
Default Value: WBP_TradingUI
Tooltip: "Trading interface widget class"
```

#### Active Trading Widget
```
Variable Name: ActiveTradingWidget
Type: UUserWidget (Object Reference)
Category: UI
Default Value: None
Tooltip: "Currently displayed trading UI instance"
```

## Event Graph - Core Functions

### Event BeginPlay

```
Event BeginPlay
  → Print String: "Trading Ship Initialized"
  → Print String: "Starting Credits: {PlayerCredits}"
```

**Purpose**: Initialize ship and display starting state

### Input Action: OpenTrade (Key: T)

```
Event: InputAction OpenTrade
  → Branch: Is Valid? (DockedStation)
     ├─ True:
     │   → Get Marketplace Module (from DockedStation)
     │   → Branch: Has Marketplace?
     │      ├─ True:
     │      │   → Branch: IsAvailableForTrading?
     │      │      ├─ True:
     │      │      │   → Open Trading UI
     │      │      │   → Pass: PlayerCredits, CargoComponent, MarketData
     │      │      └─ False:
     │      │         → Print String: "Market is closed"
     │      └─ False:
     │         → Print String: "No marketplace at this station"
     └─ False:
        → Print String: "Not docked at any station"
```

**Inputs**: None  
**Outputs**: Opens trading UI or displays error message

### Function: OpenTradingUI

```
Function: OpenTradingUI
Inputs:
  - MarketData (UMarketDataAsset*)

Flow:
  1. Create Widget (class: TradingUIWidget)
  2. Set ActiveTradingWidget = created widget
  3. Add to Viewport (Z-Order: 10)
  4. Set Input Mode: UI Only
  5. Show Mouse Cursor: true
  6. Initialize Widget with:
     - Player Credits
     - Cargo Component
     - Market Data
```

**Returns**: None

### Function: CloseTradingUI

```
Function: CloseTradingUI

Flow:
  1. Branch: Is Valid? (ActiveTradingWidget)
     ├─ True:
     │   → Remove from Parent (ActiveTradingWidget)
     │   → Set ActiveTradingWidget = None
     │   → Set Input Mode: Game Only
     │   → Show Mouse Cursor: false
     └─ False:
        → (Do nothing)
```

**Returns**: None

### Function: OnDockAtStation

```
Function: OnDockAtStation
Inputs:
  - Station (ASpaceStation*)

Flow:
  1. Set DockedStation = Station
  2. Print String: "Docked at {Station Name}"
  3. Print String: "Press T to trade"
```

**Purpose**: Called when ship successfully docks

### Function: OnUndockFromStation

```
Function: OnUndockFromStation

Flow:
  1. Branch: Is Valid? (ActiveTradingWidget)
     ├─ True: Close Trading UI
     └─ False: (Continue)
  2. Set DockedStation = None
  3. Print String: "Undocked from station"
```

**Purpose**: Called when ship undocks

### Function: BuyItem

```
Function: BuyItem
Inputs:
  - Item (UTradeItemDataAsset*)
  - Quantity (int32)
  - UnitPrice (float)

Flow:
  1. Calculate Total Cost = UnitPrice × Quantity
  2. Branch: PlayerCredits >= TotalCost?
     ├─ True:
     │   → Get Cargo Component
     │   → Branch: HasSpaceFor(Item, Quantity)?
     │      ├─ True:
     │      │   → Subtract Credits: PlayerCredits -= TotalCost
     │      │   → Add Cargo: CargoComponent->AddCargo(Item, Quantity)
     │      │   → Print String: "Bought {Quantity}x {Item Name} for {TotalCost}cr"
     │      │   → Return: true
     │      └─ False:
     │         → Print String: "Not enough cargo space"
     │         → Return: false
     └─ False:
        → Print String: "Not enough credits"
        → Return: false
```

**Returns**: bool (success/failure)

### Function: SellItem

```
Function: SellItem
Inputs:
  - Item (UTradeItemDataAsset*)
  - Quantity (int32)
  - UnitPrice (float)

Flow:
  1. Get Cargo Component
  2. Get Current Quantity: CargoComponent->GetItemQuantity(Item)
  3. Branch: CurrentQuantity >= Quantity?
     ├─ True:
     │   → Remove Cargo: CargoComponent->RemoveCargo(Item, Quantity)
     │   → Calculate Total Profit = UnitPrice × Quantity
     │   → Add Credits: PlayerCredits += TotalProfit
     │   → Print String: "Sold {Quantity}x {Item Name} for {TotalProfit}cr"
     │   → Return: true
     └─ False:
        → Print String: "Don't have {Quantity}x {Item Name}"
        → Return: false
```

**Returns**: bool (success/failure)

## Collision Detection (Docking)

### Add Sphere Collision Component

```
Component Name: DockingDetectionSphere
Class: USphereComponent
Radius: 5000.0 (units)
Collision Preset: Overlap All Dynamic
```

### Event: OnComponentBeginOverlap (DockingDetectionSphere)

```
Event: OnComponentBeginOverlap (DockingDetectionSphere)
  → Get Other Actor
  → Cast to ASpaceStation
  → Branch: Cast Succeeded?
     ├─ True:
     │   → Set DockedStation = Casted Station
     │   → Call: OnDockAtStation(Station)
     └─ False:
        → (Ignore - not a station)
```

### Event: OnComponentEndOverlap (DockingDetectionSphere)

```
Event: OnComponentEndOverlap (DockingDetectionSphere)
  → Get Other Actor
  → Cast to ASpaceStation
  → Branch: Cast Succeeded AND DockedStation == Station?
     ├─ True:
     │   → Call: OnUndockFromStation()
     └─ False:
        → (Ignore)
```

## Testing Checklist

- [ ] Ship spawns in level
- [ ] Starting credits show: 1000
- [ ] Can fly near station
- [ ] "Docked at..." message appears
- [ ] Press T opens trading UI
- [ ] Can see player credits in UI
- [ ] Can see cargo space in UI
- [ ] Can buy items (credits decrease, cargo fills)
- [ ] Can sell items (credits increase, cargo empties)
- [ ] Flying away closes UI automatically
- [ ] Can dock at multiple stations

## Common Issues

### "Trading UI doesn't open"
**Check**:
1. TradingUIWidget class is set correctly
2. Station has MarketplaceModule
3. Marketplace bIsOpen = true
4. MarketDataAsset is assigned

### "Can't afford anything"
**Fix**: Increase PlayerCredits default value or lower item prices

### "Cargo space fills instantly"
**Fix**: Increase CargoCapacity or reduce item VolumePerUnit

### "Docking doesn't trigger"
**Check**:
1. DockingDetectionSphere collision is set to Overlap
2. Station has collision enabled
3. Sphere radius is large enough (5000+ units)

## Blueprint Visual Reference

See generated diagram: `docs/reference/images/blueprints/bp_trading_ship_starter.svg`

## Next Steps

1. Create this Blueprint in Unreal Editor
2. Assign ship mesh and materials
3. Create trading UI widget (see WBP_TradingUI_Template.md)
4. Test with 2-station setup
5. Iterate based on playtesting

---

**Last Updated**: January 17, 2026  
**For**: Trade Simulator MVP (Weeks 1-4)  
**Complexity**: Beginner-friendly (hardcoded prototype)
