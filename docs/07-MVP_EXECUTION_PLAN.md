# MVP Execution Plan — 3-Day Sprint with UE MCP

**Condensed from MCP_EXECUTION_PLAN.md | Trade Simulator MVP Week 12 | August 2026**

---

## 🎯 Mission: Playable Trading Demo in 3 Days

**Prerequisite**: You have UE 5.6 Editor open with Adastrea project, Remote Control plugins enabled (HTTP:30010, WS:30020), MCP connected.

---

## DAY 1: Foundation & Verification (Today)

### Step 1: Verify MCP Connection
```python
manage_tools(action="list_tools")          # Confirm 23 tools
manage_tools(action="get_status")          # Check all enabled
control_editor(action="console_command", command="version")  # UE 5.6
```

### Step 2: Build & PIE Smoke Test
```bash
./build_with_ue_tools.sh Development Linux  # Or .bat on Windows
```
```python
control_editor(action="start_pie", map="")  # Default map
control_editor(action="stop_pie")
```

### Step 3: Create Test Level
```python
manage_level(action="create", name="MVP_TestLevel")
control_editor(action="start_pie", map="MVP_TestLevel")
control_editor(action="screenshot", name="baseline_empty")
control_editor(action="stop_pie")
```

---

## DAY 2: Core Content Creation (Tomorrow)

### Step 4: Validate All DataAssets (Critical)
```python
# 15 Trade Items - verify each loads with valid properties
trade_items = [
    "DA_TradeItem_FoodRations", "DA_TradeItem_Water", "DA_TradeItem_MedicalSupplies",
    "DA_TradeItem_Electronics", "DA_TradeItem_ShipComponents", "DA_TradeItem_IronOre",
    "DA_TradeItem_CopperOre", "DA_TradeItem_TitaniumOre", "DA_TradeItem_Platinum",
    "DA_TradeItem_RareEarthElements", "DA_TradeItem_Helium-3", "DA_TradeItem_AntimatterPowerCores",
    "DA_TradeItem_QuantumProcessor", "DA_TradeItem_AICores", "DA_TradeItem_LuxuryBeverages",
    "DA_TradeItem_Artwork", "DA_TradeItem_Jewelry", "DA_TradeItem_ConstructionMaterials",
    "DA_TradeItem_SteelAlloy", "DA_TradeItem_SyntheticProtein"
]
# Note: Test shows 15 items, need 5 more

for item in trade_items:
    manage_asset(action="load", path=f"/Game/DataAssets/Trading/Items/{item}")
    inspect(action="get_properties", object=item)
    # Verify: BasePrice > 0, VolumePerUnit > 0, ItemID valid

# 5 Markets - same validation
markets = ["DA_Market_Agricultural", "DA_Market_Industrial", "DA_Market_Mining", 
           "DA_Market_TechHub", "DA_Market_Luxury"]
```

### Step 5: Create 3 Core Blueprints

**A. BP_TradingShip**
```python
manage_blueprint(action="create", name="BP_TradingShip", parent="/Script/Adastrea.Spaceship")
manage_blueprint(action="add_component", blueprint="BP_TradingShip", 
    component="CargoComponent", class="/Script/Adastrea.CargoComponent")
manage_blueprint(action="add_component", blueprint="BP_TradingShip", 
    component="PlayerTraderComponent", class="/Script/Adastrea.PlayerTraderComponent")
manage_blueprint(action="add_component", blueprint="BP_TradingShip", 
    component="SpaceshipControlsComponent", class="/Script/Adastrea.SpaceshipControlsComponent")
manage_blueprint(action="set_default", blueprint="BP_TradingShip", 
    property="CargoComponent.MaxCapacity", value=50)
manage_blueprint(action="set_default", blueprint="BP_TradingShip", 
    property="SpaceshipControlsComponent.MaxSpeed", value=2000.0)
```

**B. BP_TradeStation**
```python
manage_blueprint(action="create", name="BP_TradeStation", parent="/Script/Adastrea.SpaceStation")
manage_blueprint(action="set_default", blueprint="BP_TradeStation", 
    property="DefaultModuleClasses", value=[
        "/Game/Blueprints/Stations/Modules/BP_SpaceStationModule_DockingBay",
        "/Game/Blueprints/Stations/Modules/BP_SpaceStationModule_Market",
        "/Game/Blueprints/Stations/Modules/BP_CargoBayModule"
    ])
```

**C. WBP_TradingUI (UMG)**
```python
manage_blueprint(action="create", name="WBP_TradingUI", parent="/Script/UMG.UserWidget")
# Structure: VerticalBox root → StationName | Credits/Cargo | ListView(Items) | Qty/Buy/Sell | Profit
```

### Step 6: Wire Trading Logic (Blueprint Graphs)
```python
# In BP_TradingShip or PlayerController:
# Event OnDockedAtStation(Station)
#   → Cast to BP_TradeStation
#   → HasMarketplace() → Branch True
#   → GetMarketplaceModule() → GetMarketDataAsset()
#   → CreateWidget WBP_TradingUI → Set MarketData → AddToViewport

# In WBP_TradingUI:
# Event OnBuyClicked(Item, Qty)
#   → Get PlayerShip → CargoComponent.AddCargo(Item, Qty)
#   → PlayerTraderComponent.SpendCredits(BuyPrice * Qty)
#   → Play SFX → Refresh List

# Event OnSellClicked(Item, Qty)
#   → Get PlayerShip → CargoComponent.RemoveCargo(Item, Qty)
#   → PlayerTraderComponent.AddCredits(SellPrice * Qty)
#   → Play SFX → Refresh List
```

---

## DAY 3: Level Build & Playtest (Day After)

### Step 7: Build Test Level (3 Stations)
```python
# Equilateral triangle ~50km sides = ~1 min flight each
control_actor(action="spawn", class="BP_TradeStation", 
    transform={"location": [0, 0, 500]}, name="Station_Agri")
control_actor(action="spawn", class="BP_TradeStation", 
    transform={"location": [50000, 0, 500]}, name="Station_Industrial")
control_actor(action="spawn", class="BP_TradeStation", 
    transform={"location": [25000, 43300, 500]}, name="Station_Mining")

# Assign different MarketDataAssets to each station's MarketplaceModule
# (via level instance editing or Blueprint defaults)

# Player start
control_actor(action="spawn", class="BP_TradingShip", 
    transform={"location": [0, -5000, 500]}, name="PlayerStart")

# Environment
build_environment(action="add_lighting", type="sky_atmosphere")
build_environment(action="add_lighting", type="directional_light", 
    properties={"intensity": 5.0, "color": [1.0, 0.95, 0.9]})
build_environment(action="add_fog", type="exponential_height_fog")
```

### Step 8: First PIE Test — YOU PLAY
```python
control_editor(action="start_pie", map="MVP_TestLevel")
# YOU manually:
# 1. Fly to Station_Agri (WASD + mouse)
# 2. Dock at docking bay
# 3. Trading UI opens → Buy FoodRations (cheap)
# 4. Undock → Fly to Station_Industrial (~1 min)
# 5. Dock → Sell FoodRations (higher price)
# 6. Buy SteelAlloy → Fly to Station_Mining → Sell
# 7. Verify: Credits up, cargo updated, no errors
control_editor(action="screenshot", name="first_trade_success")
control_editor(action="stop_pie")
```

### Step 9: Automated Validation
```python
control_editor(action="console_command", command="Adastrea.TestTradingLoop")
control_editor(action="console_command", command="stat fps")
control_editor(action="console_command", command="stat unit")
control_editor(action="console_command", command="stat gpu")
# Target: 60+ FPS, no errors
```

---

## WEEK 2: Scale & Polish (Days 4-7)

| Day | Task | MCP Tools |
|-----|------|-----------|
| 4 | Create 5 missing MarketDataAssets | `manage_asset(create)` |
| 4 | Create 5 missing TradeItemDataAssets | `manage_asset(create)` |
| 5 | Place 7 more stations (total 10) | `control_actor(spawn)` |
| 5 | Wire EconomyManager in GameMode | `manage_blueprint(set_default)` |
| 6 | Add VFX: Docking ring, trade sparkle | `manage_effect(create_niagara)` |
| 6 | Add Audio: Docking chime, cash register | `manage_audio(import)` |
| 6 | UI animations: Profit pulse, button feedback | `manage_blueprint(edit_umg)` |
| 7 | Save/Load test in PIE | `control_editor(start_pie)` |
| 7 | Balance pass on all prices | Manual + `inspect` |

---

## WEEK 3: Validate & Package (Days 8-10)

| Day | Task | Validation |
|-----|------|------------|
| 8 | Performance profile (120s) | `system_control(profile)` |
| 8 | Bug fix sprint | Zero critical bugs |
| 9 | Recruit playtesters (5→10→20) | Discord, Reddit, indie dev |
| 9 | Playtest sessions | 75% fun, 50% buy |
| 10 | Package Shipping build | `build_with_ue_tools.sh Shipping` |
| 10 | Create pitch materials | Screenshots, 2-min video, deck |

---

## 🎯 Success Criteria Checklist

### Day 3 (First PIE Test)
- [ ] MCP connects, 23 tools
- [ ] All 20 DataAssets load valid
- [ ] 3 Blueprints created
- [ ] Trading logic wired
- [ ] 3 stations in level
- [ ] **You can fly → dock → buy → fly → dock → sell → profit**
- [ ] 60 FPS, no errors

### Day 7 (Content Complete)
- [ ] 10 stations with unique markets
- [ ] 20 trade items across 5 categories
- [ ] 3 ships wired
- [ ] VFX/Audio for docking & trade
- [ ] UI polished (animations, feedback)
- [ ] Save/Load works

### Day 10 (Demo Ready)
- [ ] 60 FPS on GTX 1060
- [ ] 30-minute guided experience
- [ ] No crashes in 1-hour test
- [ ] 20+ playtesters: 75% fun, 50% buy
- [ ] Pitch package: screenshots, video, deck

---

## 🚨 Fallback If MCP Fails

| Issue | Manual Fallback |
|-------|-----------------|
| MCP disconnects | Keep Editor open; I give exact click-by-click steps |
| `manage_blueprint` graph editing fails | I create BP structure via MCP; you wire nodes manually |
| `control_editor` PIE fails | Check Output Log via console command; fix compile errors |
| DataAssets won't load | Verify `/Game/DataAssets/Trading/Items/` path; reimport |

---

## 📋 Quick MCP Command Reference

```python
# Assets
manage_asset(action="load", path="/Game/...")
manage_asset(action="create", type="MarketDataAsset", name="DA_Name", properties={...})
inspect(action="get_properties", object="AssetName")

# Blueprints
manage_blueprint(action="create", name="BP_Name", parent="/Script/Module.Class")
manage_blueprint(action="add_component", blueprint="BP_Name", component="Comp", class="/Script/Module.Class")
manage_blueprint(action="set_default", blueprint="BP_Name", property="Prop", value=Value)

# Actors & Level
control_actor(action="spawn", class="BP_Class", transform={location:[x,y,z]}, name="ActorName")
manage_level(action="create", name="LevelName")
build_environment(action="add_lighting", type="sky_atmosphere")

# Editor Control
control_editor(action="start_pie", map="LevelName")
control_editor(action="stop_pie")
control_editor(action="console_command", command="stat fps")
control_editor(action="screenshot", name="test_name")

# Performance
system_control(action="profile", duration=60)
system_control(action="stat", commands=["stat fps", "stat unit", "stat gpu"])
```

---

## 📞 Communication Protocol

| Situation | You Do |
|-----------|--------|
| MCP connected | Say "MCP ready" |
| PIE running (Step 8) | Play manually; narrate what you see |
| Error occurs | Paste Output Log error |
| Need manual step | I give exact clicks; you execute |
| Done with step | Say "done" or "next" |

---

**Ready?** Open UE Editor, enable Remote Control, say **"MCP ready"** and I'll start with Step 1.