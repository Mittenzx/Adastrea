# MVP Quick Reference — Consolidated

**Trade Simulator MVP | One-Page Cheat Sheet | August 2026**

---

## 🎯 Mission

**Build playable trading demo in 10 days using UE MCP (23 tools)**

**Core Loop**: Buy low → Fly → Sell high → Upgrade ship → Repeat
**Target**: 30-min demo, 75% fun, 50% buy, 60 FPS, 20+ playtesters

---

## 📅 10-Day Sprint

| Day | Focus | Key MCP Tools |
|-----|-------|---------------|
| 1 | Verify MCP, build, create test level | `manage_tools`, `control_editor`, `manage_level` |
| 2 | Validate 25 DataAssets, create 3 Blueprints | `manage_asset`, `inspect`, `manage_blueprint` |
| 3 | Wire trading logic, build level, **first PIE test** | `manage_blueprint`, `control_actor`, `build_environment` |
| 4 | Create 5 missing markets + 5 items | `manage_asset(create)` |
| 5 | Place 7 more stations (total 10) | `control_actor(spawn)` |
| 6 | VFX (Niagara), Audio, UI animations | `manage_effect`, `manage_audio`, `manage_blueprint(edit_umg)` |
| 7 | Save/Load test, balance pass | `control_editor`, `inspect` |
| 8 | Performance profile, bug sprint | `system_control(profile/stat)` |
| 9 | Playtest 5→10→20 people | Manual + screenshots |
| 10 | Package Shipping, pitch materials | `build_with_ue_tools.sh Shipping` |

---

## 🔧 MCP Quick Commands

```python
# Verify connection
manage_tools(action="list_tools")

# DataAssets
manage_asset(action="load", path="/Game/DataAssets/Trading/Items/DA_TradeItem_FoodRations")
inspect(action="get_properties", object="DA_TradeItem_FoodRations")
manage_asset(action="create", type="MarketDataAsset", name="DA_Market_Mining", properties={...})

# Blueprints (NOTE: real names — BP_Battleship/BP_PlayerShip for ship, BP_SpaceStation for station.
# BP_TradingShip/BP_TradeStation in older plans are the same assets under old names.)
manage_blueprint(action="set_default", blueprint="BP_Battleship", property="CargoComponent.MaxCapacity", value=50)

# Level & Actors
manage_level(action="create", name="MVP_TestLevel")
control_actor(action="spawn", class="BP_TradeStation", transform={"location":[0,0,500]}, name="Station_Agri")
build_environment(action="add_lighting", type="sky_atmosphere")

# Editor Control
control_editor(action="start_pie", map="MVP_TestLevel")
control_editor(action="console_command", command="stat fps")
control_editor(action="screenshot", name="trade_success")
control_editor(action="stop_pie")

# Performance
system_control(action="profile", duration=60)
system_control(action="stat", commands=["stat fps","stat unit","stat gpu"])
```

---

## 📦 Must-Have DataAssets (25 Total)

### Trade Items (15 Exist, Need 5)
```
Existing: FoodRations, Water, MedicalSupplies, Electronics, ShipComponents,
          IronOre, CopperOre, TitaniumOre, Platinum, RareEarthElements,
          Helium3, AntimatterPowerCores, QuantumProcessor, AICores, LuxuryBeverages,
          Artwork, Jewelry, ConstructionMaterials, SteelAlloy, SyntheticProtein
Need:     [5 more - e.g., HydrogenFuel, Nanomachines, ExoticMatter, ...]
```

### Markets (5 Exist, Need 5)
```
Existing: Agricultural (OpenMarket), Industrial (IndustrialDepot),
          Luxury (LuxuryBazaar), Research (ResearchHub), Central (OpenMarket)
Need:     Mining, Military, Medical, Commodity, Freeport
```

---

## 🎮 3 Core Blueprints to Create

| Blueprint | Parent | Components | Key Properties |
|-----------|--------|------------|----------------|
| `BP_TradingShip` | `ASpaceship` | CargoComponent, PlayerTraderComponent, SpaceshipControlsComponent | MaxCapacity=50, MaxSpeed=2000 |
| `BP_TradeStation` | `ASpaceStation` | (via DefaultModuleClasses) | Modules: DockingBay, Marketplace, CargoBay |
| `WBP_TradingUI` | `UserWidget` | UMG: StationName, Credits, Cargo, ListView(Items), Qty/Buy/Sell, Profit | — |

---

## 🔄 Trading Logic (Blueprint Graph)

```
OnDockedAtStation(Station)
  → Cast to BP_TradeStation
  → HasMarketplace() → Branch True
  → GetMarketplaceModule() → GetMarketDataAsset()
  → CreateWidget(WBP_TradingUI) → SetMarketData → AddToViewport

WBP_TradingUI:
  OnBuyClicked(Item, Qty)
    → PlayerShip.CargoComponent.AddCargo(Item, Qty)
    → PlayerTraderComponent.SpendCredits(BuyPrice * Qty)
    → Play SFX → Refresh List
  
  OnSellClicked(Item, Qty)
    → PlayerShip.CargoComponent.RemoveCargo(Item, Qty)
    → PlayerTraderComponent.AddCredits(SellPrice * Qty)
    → Play SFX → Refresh List
```

---

## 🌍 Test Level Layout

```
3 Stations in equilateral triangle (~50km sides = ~1 min flight)
  Station_Agri (0, 0, 500)        → Market: Agricultural (cheap food)
  Station_Industrial (50000, 0, 500) → Market: Industrial (cheap materials)  
  Station_Mining (25000, 43300, 500) → Market: Mining (cheap ore)

PlayerStart: (0, -5000, 500) in BP_TradingShip
Environment: SkyAtmosphere + DirectionalLight + ExponentialHeightFog
```

---

## ✅ Day 3 Success Criteria (First PIE Test)

- [ ] MCP connects, 23 tools available
- [ ] All 20 DataAssets load with valid properties
- [ ] 3 Blueprints created & wired
- [ ] Trading logic functional in Blueprint graphs
- [ ] 3 stations placed in level with correct markets
- [ ] **You can: Fly → Dock → Buy → Fly → Dock → Sell → See Profit**
- [ ] 60+ FPS, no errors in Output Log

---

## 🎨 Polish Checklist (Days 4-7)

| Item | MCP Tool | Details |
|------|----------|---------|
| Docking VFX | `manage_effect` | Niagara ring pulse on dock |
| Trade VFX | `manage_effect` | Currency sparkle on confirm |
| Docking SFX | `manage_audio` | Chime on dock |
| Trade SFX | `manage_audio` | Cash register on buy/sell |
| UI Profit Pulse | `manage_blueprint(edit_umg)` | Green flash on profit |
| UI Button Feedback | `manage_blueprint(edit_umg)` | Press animation |
| Save/Load | `control_editor` | Test credits/cargo/position persist |

---

## 📊 Performance Targets

| Metric | Target | Command |
|--------|--------|---------|
| FPS | 60 | `stat fps` |
| Frame Time | <16.6ms | `stat unit` |
| GPU Time | <12ms | `stat gpu` |
| Draw Calls | <2000 | `stat scenerendering` |

---

## 🚨 Fallback If MCP Fails

| Issue | Manual Alternative |
|-------|-------------------|
| MCP disconnect | Keep Editor open; I give exact click steps |
| Blueprint graph editing fails | I create structure via MCP; you wire nodes manually |
| PIE won't start | Check Output Log via `control_editor(console_command)` |
| DataAssets won't load | Verify `/Game/DataAssets/Trading/Items/` path |

---

## 📞 Communication

| You Say | I Do |
|---------|------|
| "MCP ready" | Start Step 1 |
| "done" / "next" | Next step |
| "error: [log]" | Diagnose & fix |
| "playing now" | Capture screenshots during PIE |

---

## 📁 Key File Paths

| System | Path |
|--------|------|
| Trading C++ | `Source/Adastrea/Private/Trading/` |
| Station C++ | `Source/Adastrea/Private/Stations/` |
| Ship C++ | `Source/Adastrea/Private/Ships/` |
| Trade Items | `Content/DataAssets/Trading/Items/` |
| Markets | `Content/DataAssets/Trading/Markets/` |
| Ship Blueprints | `Content/Blueprints/Ships/` |
| Station Blueprints | `Content/Blueprints/Stations/Modules/` |
| Trading UI | `Content/Blueprints/Trading/` |

---

**Ready? Open UE Editor → Enable Remote Control → Say "MCP ready"**