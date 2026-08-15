# Technical Architecture

**Trade Simulator MVP | Unreal Engine 5.6 | Last Updated: August 9, 2026**

---

## 🏗️ High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        ADASTREA MVP                             │
├─────────────────────────────────────────────────────────────────┤
│  GAMEPLAY LAYER (Blueprints)                                    │
│  ├── BP_TradingShip ← UCargoComponent, UPlayerTraderComponent  │
│  ├── BP_TradeStation ← MarketplaceModule, DockingBayModule     │
│  ├── WBP_TradingUI (UMG)                                        │
│  └── BP_TradingGameMode ← UEconomyManager                       │
├─────────────────────────────────────────────────────────────────┤
│  SYSTEMS LAYER (C++ Modules)                                    │
│  ├── TradingModule    ← TradeItem, Market, Economy, Cargo      │
│  ├── StationModule    ← SpaceStation, Modules, Docking         │
│  ├── ShipModule       ← Spaceship, Controls, Customization     │
│  └── CoreModule       ← GameMode, GameState, Save, Input       │
├─────────────────────────────────────────────────────────────────┤
│  DATA LAYER (DataAssets)                                        │
│  ├── TradeItemDataAsset (15+ items)                            │
│  ├── MarketDataAsset (10 markets)                              │
│  ├── SpaceshipDataAsset (3 ships)                              │
│  └── InputConfigDataAsset                                      │
├─────────────────────────────────────────────────────────────────┤
│  ENGINE LAYER (UE 5.6)                                          │
│  ├── EnhancedInput, UMG, Niagara, GameplayAbilities            │
│  ├── RemoteControl (MCP), PythonScriptPlugin                   │
│  └── Physics, Navigation, AI                                    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📦 Module Structure

### Adastrea.Build.cs Dependencies
```csharp
PublicDependencyModuleNames = {
    "Core", "CoreUObject", "Engine", "InputCore",
    "EnhancedInput", "UMG", "Slate", "SlateCore", "Niagara"
};

PrivateDependencyModuleNames = {
    "AIModule", "NavigationSystem"
};
```

### Enabled Plugins (.uproject)
- EnhancedInput, RemoteControlComponents, WebSocketMessaging
- CommonUI, UIFramework, MVVM (ModelViewViewModel)
- PythonScriptPlugin, EditorScriptingUtilities
- Niagara, GeometryScripting, ModelingToolsEditorMode
- AdastreaDirector (custom plugin)

---

## 🔄 Core Gameplay Systems (MVP Scope)

### 1. Trading System (`Source/Adastrea/Private/Trading/`)

| Class | Responsibility |
|-------|----------------|
| `UTradeItemDataAsset` | Item definition: name, base price, volume, volatility, category |
| `UMarketDataAsset` | Market config: inventory, pricing rules, supply/demand, events |
| `UEconomyManager` | Price simulation, stock refresh, market events (GameInstanceSubsystem) |
| `UCargoComponent` | Inventory management: add/remove cargo, capacity, mass/volume |
| `UPlayerTraderComponent` | Credits, buy/sell transactions, profit tracking |
| `UTradeTransaction` | Transaction record, validation, logging |

**Key Flow:**
```
MarketDataAsset::GetItemPrice(Item, bBuying)
  → Supply/Demand factors (volatility clamped)
  → Market events multiplier
  → Market markup (1.2x buy) / markdown (0.8x sell)
  → Transaction tax (5%)
  → Blueprint override hooks
```

### 2. Station System (`Source/Adastrea/Private/Stations/`)

| Class | Responsibility |
|-------|----------------|
| `ASpaceStation` | Root actor, spawns modules from DefaultModuleClasses |
| `ASpaceStationModule` | Base class: power, integrity, group, type |
| `AMarketplaceModule` | Trading facility: MarketDataAsset, open/close state |
| `ADockingBayModule` | Ship docking: ports, approach vectors, validation |
| `ACargoBayModule` | Station inventory storage |
| `AReactorModule` | Power generation |
| 10+ other modules | Habitation, Barracks, Fabrication, etc. |

**Module Architecture:**
```
ASpaceStation (BeginPlay)
  → For each DefaultModuleClasses:
      Spawn module → Attach → Initialize
  → Power grid calculates total power vs requirements
  → Integrity system tracks damage/repair per module
```

### 3. Ship System (`Source/Adastrea/Private/Ships/`)

| Class | Responsibility |
|-------|----------------|
| `ASpaceship` | Base pawn: mesh, movement, components |
| `USpaceshipControlsComponent` | Flight input, thrust, rotation, damping |
| `UShipCustomizationComponent` | Module slots, upgrades, visual changes |
| `UShipModuleComponent` | Equipment: engines, shields, weapons |
| `UCargoComponent` | (Shared) Cargo inventory on ship |
| `UPlayerTraderComponent` | (Shared) Trading logic on ship |

**Flight Model:**
- Newtonian-style with damping (X4-inspired)
- Forward/Strafe/Vertical thrust
- Yaw/Pitch/Roll torque
- Boost, brake, auto-dampening

---

## 🎮 Data-Driven Design

### Primary Data Assets (Asset Manager Registered)

```cpp
// TradeItemDataAsset
UCLASS() class UTradeItemDataAsset : public UPrimaryDataAsset {
    FText ItemName;
    FName ItemID;                    // Stable ID for saves
    ETradeItemCategory Category;
    float BasePrice;
    FFloatRange PriceVolatility;     // Min/Max deviation
    float VolumePerUnit;             // Cargo space
    float MassPerUnit;               // Physics mass
    int32 StandardLotSize;           // Default trade quantity
    int32 TypicalMarketStock;        // Baseline supply
    int32 ReplenishmentRate;         // Per refresh cycle
    int32 AITradePriority;           // 1-10
    TArray<FName> BehaviorTags;      // "Perishable", "Illegal", etc.
};

// MarketDataAsset  
UCLASS() class UMarketDataAsset : public UPrimaryDataAsset {
    FText MarketName;
    FName MarketID;
    EMarketType MarketType;          // OpenMarket, LuxuryBazaar, IndustrialDepot, ResearchHub
    EMarketSize MarketSize;          // Small, Medium, Large, Massive
    float SellPriceMarkup;           // Player buys: 1.2 = +20%
    float BuyPriceMarkdown;          // Player sells: 0.8 = -20%
    float TransactionTaxRate;        // 0.05 = 5%
    TArray<FMarketInventoryEntry> Inventory;  // Per-item supply/demand/stock
    float StockRefreshRate;          // Hours
    TArray<FMarketEvent> ActiveEvents;        // Price modifiers
};

// SpaceshipDataAsset
UCLASS() class USpaceshipDataAsset : public UPrimaryDataAsset {
    FText ShipName;
    TSubclassOf<ASpaceship> ShipClass;
    float BaseMaxSpeed;
    float BaseCargoCapacity;
    int32 ModuleSlots;
    TArray<FShipModuleSlot> DefaultModules;
    // ... visual, physics, upgrade paths
};
```

---

## 💾 Persistence Architecture

### Save System (`SaveGameSubsystem`, `AdastreaSaveGame`)

```
Player Save:
├── Identity (name, ID, faction)
├── Transform (location, rotation)
├── Credits
├── Ship (class, modules, cargo, upgrades)
├── Progress (XP, level, unlocks)
├── Reputation (per faction)
├── Active Quests
└── Statistics

World Save:
├── Station states (modules, integrity, power)
├── Market states (stock, prices, events)
├── Player-built structures
└── Encounter progress
```

**Key Principle**: Save stable IDs (DataAsset names), not UObject pointers. Resolve at load.

---

## 🌐 Networking (Deferred - Post-MVP)

| Aspect | Status |
|--------|--------|
| Replication | Basic (position, health) |
| RPCs | Trading transactions (server-authoritative) |
| Authority | Server validates all trades |
| Session | Listen server + direct IP join |
| Backend | None (Steam/EOS post-MVP) |

**MVP Decision**: Single-player only. Multiplayer code exists but disabled.

---

## 🧪 Testing Architecture

### Python Test Suite (CI-Ready)
```bash
python test_trading_system.py      # DataAsset validation, price math
python test_station_systems.py     # Integrity, power, trading efficiency
python test_damage_calculations.py # 5 damage types × armor values
python test_todo_verification.py   # Documentation standards
```

### Automation (Editor)
- Smoke tests: Load all maps, check missing assets
- Screenshot tests: Visual regression vs golden masters
- `AutomationRunner.py` — CLI for overnight runs

---

## 🔌 MCP Integration Points

The `unreal-engine-mcp-server` (23 tools) connects via Remote Control API:

| MCP Tool | Adastrea Use Case |
|----------|-------------------|
| `manage_asset` | Create/validate 25 DataAssets |
| `manage_blueprint` | Create 3 core BPs, wire graphs |
| `control_actor` | Spawn 3 stations in test level |
| `control_editor` | PIE start/stop, screenshots, console |
| `build_environment` | Sky, lighting, fog for space |
| `inspect` | Verify DataAsset properties at runtime |
| `system_control` | Profile FPS, stat unit, stat gpu |

---

## 📁 Source Tree (Relevant Parts)

```
Source/Adastrea/
├── Adastrea.Build.cs
├── Adastrea.Target.cs
├── Public/
│   ├── Adastrea.h
│   ├── Trading/
│   │   ├── TradeItemDataAsset.h
│   │   ├── MarketDataAsset.h
│   │   ├── EconomyManager.h
│   │   ├── CargoComponent.h
│   │   ├── PlayerTraderComponent.h
│   │   └── TradeTransaction.h
│   ├── Stations/
│   │   ├── SpaceStation.h
│   │   ├── SpaceStationModule.h
│   │   ├── MarketplaceModule.h
│   │   └── DockingBayModule.h
│   └── Ships/
│       ├── Spaceship.h
│       ├── SpaceshipControlsComponent.h
│       └── ShipCustomizationComponent.h
└── Private/
    ├── Adastrea.cpp
    ├── Trading/          # All .cpp implementations
    ├── Stations/         # All .cpp implementations
    ├── Ships/            # All .cpp implementations
    ├── Player/           # GameMode, GameState, SaveGame, Progression
    ├── UI/               # HUD, Inventory, Menu widgets
    ├── AI/               # NPC, Personnel logic
    ├── Navigation/       # Sector map, pathfinding
    ├── Performance/      # LOD, Profiler, Benchmark
    └── Materials/        # MaterialDataAsset
```

---

## ⚡ Performance Targets

| Metric | Target | Measurement |
|--------|--------|-------------|
| FPS | 60 | `stat fps` on GTX 1060 / RX 580 |
| Frame Time | <16.6ms | `stat unit` |
| GPU Time | <12ms | `stat gpu` |
| Draw Calls | <2000 | `stat scenerendering` |
| Memory | <4GB | Task Manager |
| Load Time | <10s | Level load + asset streaming |

---

## 🔧 Build Configuration

### Development Editor (Iteration)
```bash
./build_with_ue_tools.sh Development Linux
# Or VS: Development Editor, Win64
```

### Shipping (Demo Build)
```bash
./build_with_ue_tools.sh Shipping Linux
# UE Editor: File → Package Project → Windows/Linux
```

### Key Settings
- `bWarningsAsErrors = true` (Adastrea.Build.cs)
- C++20 standard
- PCH: ExplicitOrShared
- No circular dependencies (StationEditor removed from Adastrea)

---

## 📋 Deferred Systems (Post-MVP)

These exist in C++ but are **not wired for MVP**:

| System | Lines | Status |
|--------|-------|--------|
| Combat | 5,700+ | Complete, disabled |
| Navigation/Autopilot | 865 | Complete, disabled |
| Quest System | 1,327 | Complete, disabled |
| Faction Diplomacy | 2,000+ | Complete, disabled |
| Personnel/Crew | 3,000+ | Complete, disabled |
| Save/Load (full) | 2,500+ | Partial |
| Advanced AI | 4,000+ | Complete, disabled |
| Exploration/Scanning | 3,500+ | Partial |
| Way Network | 2,800+ | Complete, disabled |
| Rivals/Antagonists | 2,200+ | Complete, disabled |
| Battle Pets | 1,800+ | Complete, disabled |
| Building/Crafting | 3,000+ | Complete, disabled |
| Multiplayer | 0 | Planned only |

**Total Deferred**: ~35K lines (50% of codebase)

---

*Back to [INDEX.md](INDEX.md) | Next: [05-ROADMAP.md](05-ROADMAP.md)*