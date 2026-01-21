# Adastrea Architecture - Trade Simulator MVP

**Current Phase**: Trade Simulator MVP (Week 3 of 12)  
**Last Updated**: January 21, 2026  
**Status**: Hardcoded Prototype Development

---

## 🎯 Overview

Adastrea is currently building a **space trading game MVP** to validate core gameplay before expanding to the full vision. This document describes the MVP architecture only.

**MVP Goal**: Playable 30-minute trading demo by March 2026

**Core Loop**: Buy low → Sell high → Upgrade ship → Repeat with better margins

---

## 🏗️ MVP Architecture

### System Architecture (MVP Only)

```
┌─────────────────────────────────────────────────────────────┐
│                     Trade Simulator MVP                      │
└─────────────────────────────────────────────────────────────┘

┌─────────────┐      ┌──────────────┐      ┌─────────────┐
│   Player    │─────▶│   Station    │─────▶│  Trading    │
│   Ship      │      │   + Docking  │      │     UI      │
│             │      │              │      │             │
│ • Movement  │      │ • Marketplace│      │ • Buy/Sell  │
│ • Cargo     │      │ • Inventory  │      │ • Prices    │
│ • Credits   │      │              │      │             │
└─────────────┘      └──────────────┘      └─────────────┘
       │                     │                     │
       │                     │                     │
       └─────────────────────┴─────────────────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │ Economy Manager │
                    │                 │
                    │ • Price Updates │
                    │ • Supply/Demand │
                    └─────────────────┘
```

### Core Components

#### 1. Player Ship (`BP_SimpleTradingShip`)
- **Purpose**: Player's trading vessel
- **Features**:
  - 3D space flight (FloatingPawnMovement)
  - Cargo storage (simple inventory)
  - Credits tracking
  - Docking capability
- **Properties**:
  - Cargo capacity (upgradeable)
  - Flight speed
  - Current location

#### 2. Trading Stations (`BP_SimpleStation_A`, `BP_SimpleStation_B`)
- **Purpose**: Trading locations with markets
- **Features**:
  - Docking bays for ships
  - Marketplace with inventory
  - Buy/sell prices per item
  - Visual distinction between stations
- **Components**:
  - Collision volume for docking trigger
  - Market inventory (hardcoded for prototype)
  - Station mesh

#### 3. Trading UI (`WBP_SimpleTradingUI`)
- **Purpose**: Interface for buying/selling cargo
- **Features**:
  - Item list with prices
  - Buy/sell buttons
  - Quantity selector
  - Player credits display
  - Cargo space indicator
  - Transaction feedback
- **Shows**:
  - Available items at station
  - Player's current cargo
  - Total transaction value
  - Profit/loss calculations

#### 4. Economy Manager (`BP_SimpleEconomy`)
- **Purpose**: Manages market prices and supply/demand
- **Features**:
  - Price updates based on player trading
  - Supply tracking per station
  - Simple demand simulation
  - Price fluctuations
- **Algorithm** (simplified):
  ```
  New Price = Base Price × Station Multiplier × Supply Factor
  Supply Factor = 1.0 + (Recent Sales - Recent Purchases) / 100
  ```

#### 5. HUD (`WBP_SimpleHUD`)
- **Purpose**: Shows essential info during flight
- **Displays**:
  - Current credits
  - Cargo space (used/total)
  - Distance to nearest station
  - Docking prompt when near station
  - Tutorial hints

---

## 📊 Data Flow

### Trading Loop Flow

```
1. Player flies to Station A
   └─▶ Ship movement component
   
2. Player approaches station
   └─▶ Docking trigger fires
   └─▶ "Press F to dock" prompt appears
   
3. Player presses F
   └─▶ Ship movement disabled
   └─▶ Trading UI opens
   └─▶ Market inventory loaded
   
4. Player buys cargo
   └─▶ Credits deducted
   └─▶ Cargo added to ship
   └─▶ Station supply decreased
   └─▶ Price increases slightly
   
5. Player closes UI (undocks)
   └─▶ Ship movement enabled
   └─▶ Player flies to Station B
   
6. Player docks at Station B
   └─▶ Trading UI opens
   └─▶ Different prices displayed
   
7. Player sells cargo
   └─▶ Credits added
   └─▶ Cargo removed from ship
   └─▶ Station supply increased
   └─▶ Price decreases slightly
   └─▶ Profit calculated and displayed
   
8. Repeat with better margins
```

### Price Update Flow

```
Player Transaction
    ↓
Update Station Supply
    ↓
Recalculate Price
    ↓
Apply Supply Factor
    ↓
Update UI Display
    ↓
Store New Price
```

---

## 🎮 MVP Technical Implementation

### Phase 1: Hardcoded Prototype (Current)

**Approach**: Everything hardcoded in Blueprints for speed

**Items** (hardcoded):
```
Water:    Station A: 10 credits  |  Station B: 15 credits
Food:     Station A: 25 credits  |  Station B: 35 credits  
Fuel:     Station A: 50 credits  |  Station B: 65 credits
```

**Ship Stats** (hardcoded):
```
Cargo Capacity: 10 units
Starting Credits: 1000
Flight Speed: 600 units/sec
```

**No Data Assets Yet**: Coming in Phase 2 after validation

### Phase 2: Structured Version (Weeks 5-8)

Convert to Data Assets:
- `UTradeItemDataAsset` - Trade goods
- `UMarketDataAsset` - Station markets
- `USpaceshipDataAsset` - Ship configurations

### Phase 3: Polish (Weeks 9-12)

Add polish and content:
- More ships (3 total)
- More stations (10 total)
- More items (20 total)
- Save/load system
- Sound effects and VFX

---

## 📦 File Structure (MVP)

```
Content/
├── Blueprints/
│   ├── MVP/
│   │   ├── BP_SimpleTradingShip.uasset     # Player ship
│   │   ├── BP_SimpleStation_A.uasset       # Trading station 1
│   │   ├── BP_SimpleStation_B.uasset       # Trading station 2
│   │   └── BP_SimpleEconomy.uasset         # Economy manager
│   │
│   └── GameModes/
│       └── BP_TradingGameMode.uasset       # MVP game mode
│
├── UI/
│   ├── MVP/
│   │   ├── WBP_SimpleTradingUI.uasset      # Trading interface
│   │   ├── WBP_SimpleHUD.uasset            # Flight HUD
│   │   └── WBP_DockingPrompt.uasset        # "Press F" prompt
│   │
│   └── Textures/
│       └── ... (UI icons and backgrounds)
│
└── Maps/
    └── MVP_TestLevel.umap                   # Test environment
```

---

## 🔧 Technical Details

### Blueprint Communication

**Event-Driven Architecture**:
- Docking triggers `OnStationEntered` event
- Trading UI calls `OnItemPurchased` event
- Economy updates via `OnPriceChanged` event
- HUD updates via `OnCreditsChanged` event

### Key Blueprint Interfaces

**`BP_SimpleTradingShip`**:
- `GetCurrentCredits()` → float
- `GetCargoSpace()` → int (used/total)
- `AddCargo(ItemName, Quantity)` → bool
- `RemoveCargo(ItemName, Quantity)` → bool
- `AddCredits(Amount)` → void
- `RemoveCredits(Amount)` → bool

**`BP_SimpleStation`**:
- `GetMarketInventory()` → Map<ItemName, Quantity>
- `GetItemPrice(ItemName, IsBuying)` → float
- `UpdateSupply(ItemName, QuantityChange)` → void

**`BP_SimpleEconomy`**:
- `CalculatePrice(Station, Item, IsBuying)` → float
- `RecordTransaction(Station, Item, Quantity, IsBuying)` → void
- `UpdateAllPrices()` → void

---

## ⚠️ MVP Scope Boundaries

### ✅ IN SCOPE (MVP)
- Basic 3D flight
- 2 stations minimum
- 3-5 trade items
- Simple buy/sell UI
- Basic economy simulation
- Cargo management
- Ship upgrades (cargo capacity)
- Profit tracking

### 🚫 OUT OF SCOPE (Post-MVP)
- ❌ Combat system
- ❌ Exploration mechanics
- ❌ Faction diplomacy (beyond pricing)
- ❌ Crew management
- ❌ Quest system
- ❌ Multiplayer
- ❌ Advanced AI
- ❌ Character walking
- ❌ Procedural generation
- ❌ Complex progression

**Rationale**: Focus on ONE validated gameplay loop before expanding.

---

## 🎯 Success Criteria

### Week 4 Validation Gate
- [ ] Can fly between stations (1-2 minutes)
- [ ] Can dock at stations
- [ ] Can buy and sell cargo
- [ ] Prices differ between stations
- [ ] Can track profit/loss
- [ ] 60%+ playtesters say "fun"

**If NOT met**: Pivot or redesign core loop

### Week 12 Demo Goals
- [ ] 30-minute playable demo
- [ ] 75%+ "had fun" rating
- [ ] 50%+ "would buy" interest
- [ ] No critical bugs
- [ ] Stable 60 FPS

**If met**: Ready for publisher pitch

---

## 📚 Additional Documentation

For comprehensive architecture details (deferred systems), see:
- **[Full Architecture](docs/development/FULL_ARCHITECTURE.md)** - Complete system architecture
- **[MVP Guide](docs/mvp/README.md)** - MVP development guide
- **[System Classification](docs/mvp/SYSTEM_CLASSIFICATION.md)** - Per-system status

For development standards:
- **[Code Style](docs/development/CODE_STYLE.md)** - C++ and Blueprint standards
- **[Contributing](docs/development/CONTRIBUTING.md)** - Contribution guidelines

---

## 🚀 Future Architecture (Post-MVP)

If MVP succeeds (75%+ fun rating, funding secured), expand to:

1. **Combat System** - Weapons, targeting, damage
2. **Exploration** - Scanning, discovery, anomalies
3. **Faction System** - Full diplomacy and relationships
4. **Personnel System** - Crew management and AI
5. **Quest System** - Missions and contracts
6. **Advanced Trading** - Complex supply chains

**See [Full Architecture](docs/development/FULL_ARCHITECTURE.md) for complete vision.**

---

**Remember**: This is a focused MVP to validate market interest. Full vision comes AFTER successful validation.

**Build it. Ship it. Validate it. Then iterate.**

---

**Last Updated**: January 21, 2026  
**Version**: MVP 1.0  
**Phase**: Week 3 of 12 - Trade Simulator MVP
