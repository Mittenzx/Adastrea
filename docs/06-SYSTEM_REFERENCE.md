# System Reference — All 22 Systems

**Trade Simulator MVP | Active vs Deferred | Last Updated: August 9, 2026**

---

## 🟢 MVP Active Systems (5 Critical)

| System | Location | Status | Key Classes |
|--------|----------|--------|-------------|
| **Trading** | `Source/Adastrea/Private/Trading/` | ✅ C++ Complete | TradeItemDataAsset, MarketDataAsset, EconomyManager, CargoComponent, PlayerTraderComponent |
| **Stations** | `Source/Adastrea/Private/Stations/` | ✅ C++ Complete | SpaceStation, SpaceStationModule, MarketplaceModule, DockingBayModule, CargoBayModule |
| **Ships** | `Source/Adastrea/Private/Ships/` | ✅ C++ Complete | Spaceship, SpaceshipControlsComponent, ShipCustomizationComponent, ShipModuleComponent |
| **Economy** | `Source/Adastrea/Private/Trading/EconomyManager.cpp` | ✅ C++ Complete | UEconomyManager (GameInstanceSubsystem) |
| **Input/Controls** | `Source/Adastrea/Private/Input/` | ✅ C++ Complete | InputConfigDataAsset, EnhancedInput integration |

---

## 🟡 MVP Supporting Systems (3 - Simplify for MVP)

| System | Location | MVP Approach |
|--------|----------|--------------|
| **Save/Load** | `Source/Adastrea/Private/Player/SaveGameSubsystem.cpp` | Basic: credits, cargo, position, ship only |
| **UI/HUD** | `Source/Adastrea/Private/UI/` | Trading UI only (WBP_TradingUI), no full HUD |
| **Performance** | `Source/Adastrea/Private/Performance/` | Profiling tools only, no optimization yet |

---

## 🔴 Deferred Systems (14 - Post-MVP Only)

| System | Location | Lines | Notes |
|--------|----------|-------|-------|
| **Combat** | `Source/Adastrea/Private/Combat/` | 5,700+ | Complete, disabled |
| **Navigation/Autopilot** | `Source/Adastrea/Private/Navigation/` | 865 | Complete, disabled |
| **Quest System** | `Source/Adastrea/Private/Quest/` | 1,327 | Complete, disabled |
| **Faction Diplomacy** | `Source/Adastrea/Private/Factions/` | 2,000+ | Complete, disabled |
| **Personnel/Crew** | `Source/Adastrea/Private/Characters/` | 3,000+ | Complete, disabled |
| **Advanced AI** | `Source/Adastrea/Private/AI/` | 4,000+ | Complete, disabled |
| **Exploration/Scanning** | `Source/Adastrea/Private/Exploration/` | 3,500+ | Partial |
| **Way Network** | `Source/Adastrea/Private/Way/` | 2,800+ | Complete, disabled |
| **Rivals/Antagonists** | `Source/Adastrea/Private/Rivals/` | 2,200+ | Complete, disabled |
| **Battle Pets** | `Source/Adastrea/Private/` | 1,800+ | Complete, disabled |
| **Building/Crafting** | `Source/Adastrea/Private/Stations/` (modules) | 3,000+ | Complete, disabled |
| **Tutorial System** | `Source/Adastrea/Private/Tutorial/` | 1,200+ | Complete, disabled |
| **Audio System** | `Source/Adastrea/Archive/Source/Audio/` | 2,000+ | In Archive, disabled |
| **Multiplayer** | Not implemented | 0 | Planned only |

---

## 📦 Module Dependency Graph

```
Adastrea (Core Module)
├── Public Dependencies:
│   ├── Core, CoreUObject, Engine, InputCore
│   ├── EnhancedInput, UMG, Slate, SlateCore, Niagara
│
├── Private Dependencies:
│   ├── AIModule, NavigationSystem
│
├── Systems (Private):
│   ├── Trading     ← DEPENDS ON: Economy, Cargo, PlayerTrader
│   ├── Stations    ← DEPENDS ON: Trading (MarketDataAsset)
│   ├── Ships       ← DEPENDS ON: Trading (CargoComponent)
│   ├── Player      ← DEPENDS ON: Trading, Ships, Stations
│   └── UI          ← DEPENDS ON: All above
│
└── Plugins:
    └── AdastreaDirector (custom, separate module)
```

---

## 🔑 Key DataAsset Types (Asset Manager Must Register)

| DataAsset | Module | Count (Current/Target) | Purpose |
|-----------|--------|------------------------|---------|
| `UTradeItemDataAsset` | Trading | 15 / 20 | Tradeable goods definitions |
| `UMarketDataAsset` | Trading | 5 / 10 | Station market configurations |
| `USpaceshipDataAsset` | Ships | 3 / 3 | Ship class definitions |
| `UInputConfigDataAsset` | Input | 1 / 1 | Control schemes |
| `UMaterialDataAsset` | Materials | N/A | Deferred |
| `UPersonnelDataAsset` | Characters | N/A | Deferred |
| `UHomeworldDataAsset` | Player | N/A | Deferred |
| `UAchievementDataAsset` | Player | N/A | Deferred |

---

## 🎮 Blueprint Requirements (MVP)

> **Note**: Actual blueprint names differ from the original design docs. The
> trading ship is `BP_Battleship`/`BP_PlayerShip`, the station is `BP_SpaceStation`,
> the marketplace module is `BP_SpaceStationModule_Market`, and the game mode is
> `BP_SpaceGameMode`. Docs below use real names.

| Blueprint | Parent | Components Required | Status |
|-----------|--------|---------------------|--------|
| `BP_Battleship` / `BP_PlayerShip` | `ASpaceship` | CargoComponent, PlayerTraderComponent (C++ base), SpaceshipControlsComponent | ✅ Wired |
| `BP_SpaceStation` | `ASpaceStation` | DefaultModuleClasses: [DockingBay, Marketplace, CargoBay] | ✅ Wired |
| `BP_SpaceStationModule_Market` | `AMarketplaceModule` | MarketDataAsset reference | ✅ Wired |
| `WBP_TradingUI` | `TradingInterfaceWidget` (C++) | Item list, buy/sell buttons, credits, cargo, profit | ✅ Wired |
| `BP_SpaceGameMode` | `AAdastreaGameMode` | EconomyManager subsystem, DefaultPawn=BP_Battleship | ✅ Wired |

### Other Ship Blueprints
| Blueprint | Parent | Notes |
|-----------|--------|-------|
| `BP_Ship_Corvette/Cruiser/Destroyer/Fighter/Freighter` | `ASpaceship` | Converted from generic Pawn (Aug 2026). Unused templates, ready for use |
| `BP_CommandXL`, `BP_Super` | `ASpaceship` | Ship variants |
| `BP_Import` | `ASpaceship` | Used by BP_TestGameMode |
| `BP_ImportAI` | `ASpaceship` | AI ship variant |

---

## 🔌 Plugin: AdastreaDirector

**Location**: `Plugins/AdastreaDirector/`
**Purpose**: MCP integration, LLM-driven editor automation, asset management
**Key Classes**:
- `AdastreaMCPServer` — Remote Control bridge
- `AdastreaToolSystem` — Tool execution framework
- `AdastreaAssetService` — Asset CRUD via MCP
- `AdastreaScriptService` — Python/JS script execution
- `AdastreaLLMClient` — LLM integration

**MCP Tools Exposed**: 23 tools (manage_asset, manage_blueprint, control_actor, control_editor, etc.)

---

## 📊 System Health Summary

| System | Code | Tests | DataAssets | Blueprints | PIE Tested |
|--------|------|-------|------------|------------|------------|
| Trading | ✅ | ✅ | 15/20 | ⚠️ | ❌ |
| Stations | ✅ | ✅ | 5/10 | ⚠️ | ❌ |
| Ships | ✅ | ❌ | 3/3 | ✅ | ❌ |
| Economy | ✅ | ✅ | N/A | ❌ | ❌ |
| Input | ✅ | ❌ | 1/1 | ✅ | ❌ |
| Save/Load | ✅ | ❌ | N/A | ❌ | ❌ |
| UI | ⚠️ | ❌ | N/A | ⚠️ | ❌ |

**Legend**: ✅ Complete | ⚠️ Partial | ❌ Not Started

---

*Back to [INDEX.md](INDEX.md) | Next: [07-MVP_EXECUTION_PLAN.md](07-MVP_EXECUTION_PLAN.md)*