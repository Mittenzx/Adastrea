# Adastrea - MCP Execution Plan: What I'll Do After You Connect UE MCP

**Prerequisite**: You open Unreal Editor with Adastrea project, enable Remote Control plugins (HTTP:30010, WS:30020), and confirm MCP connection works.

---

## 🎯 Mission: Get Playable Trading Demo in 3 Days

---

## DAY 1: Foundation & Verification (Today)

### Step 1: Verify MCP Connection & Project Health
```python
# I'll run:
manage_tools(action="list_tools")                    # Confirm 23 tools available
manage_tools(action="get_status")                    # Check tool states
control_editor(action="console_command", command="version")  # Verify UE version
```

**Expected**: All 23 tools enabled, UE 5.6 confirmed.

### Step 2: Build Verification
```python
# I'll check if project compiles by running a test PIE session
control_editor(action="start_pie", map="")           # Try default map
control_editor(action="stop_pie")
```

**Expected**: Editor loads, PIE starts/stops without crashes.

### Step 3: Create Baseline Test Level
```python
manage_level(action="create", name="MVP_TestLevel")  # Or load existing
control_editor(action="start_pie", map="MVP_TestLevel")
control_editor(action="screenshot", name="baseline_empty_level")
control_editor(action="stop_pie")
```

**Expected**: Clean test level ready for content.

---

## DAY 2: Core Content Creation (Tomorrow)

### Step 4: Validate All Existing Data Assets (Critical Path)
```python
# I'll iterate through all 20+ trade items and 5 markets:
trade_items = [
    "DA_TradeItem_FoodRations", "DA_TradeItem_Water", "DA_TradeItem_MedicalSupplies",
    "DA_TradeItem_Electronics", "DA_TradeItem_ShipComponents", "DA_TradeItem_IronOre",
    "DA_TradeItem_CopperOre", "DA_TradeItem_TitaniumOre", "DA_TradeItem_Platinum",
    "DA_TradeItem_RareEarthElements", "DA_TradeItem_Helium-3", "DA_TradeItem_AntimatterPowerCores",
    "DA_TradeItem_QuantumProcessor", "DA_TradeItem_AICores", "DA_TradeItem_LuxuryBeverages",
    "DA_TradeItem_Artwork", "DA_TradeItem_Jewelry", "DA_TradeItem_ConstructionMaterials",
    "DA_TradeItem_SteelAlloy", "DA_TradeItem_SyntheticProtein"
]

for item in trade_items:
    manage_asset(action="load", path=f"/Game/DataAssets/Trading/Items/{item}")
    inspect(action="get_properties", object=item)
    # Verify: BasePrice > 0, VolumePerUnit > 0, ItemID valid, Category set

markets = ["DA_Market_Agricultural", "DA_Market_Industrial", "DA_Market_Mining", 
           "DA_Market_TechHub", "DA_Market_Luxury"]
# Same validation for each
```

**Expected**: All DataAssets load correctly with valid properties. I'll report any broken ones.

### Step 5: Create Core Blueprints via MCP

**A. Trading Ship Blueprint**
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

**B. Trade Station Blueprint (Modular)**
```python
manage_blueprint(action="create", name="BP_TradeStation", parent="/Script/Adastrea.SpaceStation")
manage_blueprint(action="set_default", blueprint="BP_TradeStation", 
    property="DefaultModuleClasses", value=[
        "/Game/Blueprints/Stations/Modules/BP_SpaceStationModule_DockingBay",
        "/Game/Blueprints/Stations/Modules/BP_SpaceStationModule_Market",
        "/Game/Blueprints/Stations/Modules/BP_CargoBayModule"
    ])
```

**C. Trading UI Widget (UMG)**
```python
manage_blueprint(action="create", name="WBP_TradingUI", parent="/Script/UMG.UserWidget")
# I'll build the widget structure:
# - VerticalBox (root)
#   - TextBlock: "Station Market: {MarketName}"
#   - HorizontalBox: Credits | Cargo Space
#   - ListView: Trade Items (ItemName, BuyPrice, SellPrice, Stock, Volume)
#   - HorizontalBox: Quantity Slider | Buy Button | Sell Button
#   - TextBlock: "Profit: {Profit}" (color: green/red)
```

### Step 6: Wire Trading Logic in Blueprint Graph
```python
# In BP_TradingShip or PlayerController Blueprint:
# I'll add these graph nodes via manage_blueprint:
# Event: OnDockedAtStation(Station)
#   → Cast to BP_TradeStation
#   → Call HasMarketplace()
#   → Branch: True → GetMarketplaceModule() → GetMarketDataAsset() → Open WBP_TradingUI
#   → False → Print "No marketplace at this station"

# In WBP_TradingUI:
# Event: OnBuyClicked(Item, Quantity)
#   → Get PlayerShip → Get CargoComponent → AddCargo(Item, Quantity)
#   → Get PlayerTraderComponent → SpendCredits(BuyPrice * Quantity)
#   → Play SFX_TradeComplete
#   → Refresh Item List

# Event: OnSellClicked(Item, Quantity)
#   → Get PlayerShip → Get CargoComponent → RemoveCargo(Item, Quantity)
#   → Get PlayerTraderComponent → AddCredits(SellPrice * Quantity)
#   → Play SFX_TradeComplete
#   → Refresh Item List
```

---

## DAY 3: Level Build & Playtest (Day After Tomorrow)

### Step 7: Build Test Level with 3 Stations
```python
control_actor(action="spawn", class="BP_TradeStation", 
    transform={"location": [0, 0, 500], "rotation": [0, 0, 0]}, name="Station_Agri")
control_actor(action="spawn", class="BP_TradeStation", 
    transform={"location": [50000, 0, 500], "rotation": [0, 0, 0]}, name="Station_Industrial")
control_actor(action="spawn", class="BP_TradeStation", 
    transform={"location": [25000, 43300, 500], "rotation": [0, 0, 0]}, name="Station_Mining")

# Assign different MarketDataAssets to each station's MarketplaceModule
# (via Blueprint defaults or level instance edits)

# Add player start
control_actor(action="spawn", class="BP_TradingShip", 
    transform={"location": [0, -5000, 500]}, name="PlayerStart")

# Basic environment
build_environment(action="add_lighting", type="sky_atmosphere")
build_environment(action="add_lighting", type="directional_light", 
    properties={"intensity": 5.0, "color": [1.0, 0.95, 0.9]})
build_environment(action="add_fog", type="exponential_height_fog")
```

### Step 8: First PIE Test - Full Trading Loop
```python
control_editor(action="start_pie", map="MVP_TestLevel")
# I'll guide you through manual test:
# 1. Fly to Station_Agri (WASD + mouse)
# 2. Dock (approach docking bay)
# 3. Trading UI opens automatically
# 4. Buy FoodRations (cheap at agricultural)
# 5. Undock, fly to Station_Industrial (~1 min flight)
# 6. Dock, sell FoodRations (higher price at industrial)
# 7. Buy SteelAlloy (cheap at industrial)
# 8. Fly to Station_Mining, sell SteelAlloy
# 9. Verify: Credits increased, cargo updated, no errors
control_editor(action="screenshot", name="first_successful_trade")
control_editor(action="stop_pie")
```

### Step 9: Automated Validation Test
```python
# Run console command to verify systems
control_editor(action="console_command", command="Adastrea.TestTradingLoop")
control_editor(action="console_command", command="stat fps")
control_editor(action="console_command", command="stat unit")
```

**Expected**: 60+ FPS, no errors in output log, trading loop completes cleanly.

---

## DAY 4+: Polish & Scale (If Time Permits)

### Step 10: Create 7 More Markets (Total 10)
```python
# Using manage_asset to create DataAssets programmatically:
market_configs = [
    {"name": "DA_Market_Research", "type": "OpenMarket", "size": "Small", 
     "markup": 1.15, "markdown": 0.85, "focus": ["Electronics", "AICores"]},
    {"name": "DA_Market_Military", "type": "OpenMarket", "size": "Large", 
     "markup": 1.25, "markdown": 0.75, "focus": ["ShipComponents", "Weapons"]},
    # ... 5 more
]
```

### Step 11: Add Polish
```python
# VFX for docking confirmation
manage_effect(action="create_niagara", name="NS_DockingRing", template="RingPulse")
# Audio for trade confirmation  
manage_audio(action="import", file="UI_Confirm.wav", name="SFX_TradeConfirm")
# UI animations (fade in/out, profit pulse)
```

### Step 12: Performance & Package
```python
system_control(action="profile", duration=120, map="MVP_TestLevel")
# Build shipping
# control_editor(action="console_command", command="BuildCookRun -targetplatform=Win64 -configuration=Shipping")
```

---

## 📋 What I Need From You

| When | Action |
|------|--------|
| **Now** | Open UE Editor → Adastrea project → Enable Remote Control plugins (HTTP:30010, WS:30020) |
| **After Step 1** | Confirm "MCP connected successfully" |
| **During Step 8** | Manually fly/dock/trade in PIE while I watch via screenshots |
| **Anytime** | Tell me if something breaks or behaves unexpectedly |

---

## 🚨 Fallback Plans (If MCP Issues)

| Issue | Fallback |
|-------|----------|
| MCP connection drops | I'll give you exact manual Editor steps; you execute, I guide |
| Blueprint graph editing fails via MCP | I'll create Blueprint structure via MCP, you wire graphs manually |
| PIE won't start | Check output log via `control_editor(console_command)`; fix compile errors |
| DataAssets won't load | Check `/Game/DataAssets/Trading/Items/` path; may need to reimport |

---

## ✅ Success Criteria for This Plan

| Milestone | Done When |
|-----------|-----------|
| MCP verified | Step 1 complete |
| All 25 DataAssets validated | Step 4 complete |
| 3 core Blueprints created | Step 5 complete |
| Trading logic wired | Step 6 complete |
| 3-station test level built | Step 7 complete |
| **Full trade loop works in PIE** | **Step 8 complete** ← **MAIN GOAL** |
| 60 FPS, no errors | Step 9 complete |

---

**Ready when you are.** Just say "MCP connected" or "UE Editor ready" and I'll start with Step 1.