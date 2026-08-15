# Adastrea - Quick Reference: MVP Completion with UE MCP

## 🎯 One-Page Action Plan

### **Current State**: Week 12 of 12 (Final Polish) - March 16, 2026
- **Code**: 33K+ lines, 22 systems ✅ Complete
- **Trading MVP**: All C++ systems implemented ✅
- **Content**: 20 trade items, 5 markets, 10+ ship blueprints ⚠️ Need validation
- **Goal**: 30-min polished demo for publishers by March 22

### **New Capability**: Unreal Engine MCP (23 Tools)
Configured and ready! Connects to UE Remote Control API (ports 30010/30020).

| Category | Tools |
|----------|-------|
| **Assets** | `manage_asset` - Create/import/manage assets, materials, DataAssets |
| **Blueprints** | `manage_blueprint` - Create BPs/UMG, add components, edit graphs |
| **Actors** | `control_actor` - Spawn, transform, components, tags |
| **Editor** | `control_editor` - PIE, viewport, console commands, screenshots |
| **Levels** | `manage_level` - Load/save, streaming, lighting |
| **World** | `build_environment` - Landscape, foliage, lighting, splines |
| **Gameplay** | `manage_character`, `manage_combat`, `manage_ai`, `manage_inventory`, `manage_interaction`, `manage_networking` |
| **VFX/Audio** | `manage_effect` (Niagara), `manage_audio` |
| **Inspection** | `inspect` - Read/write UObject properties |

---

## ⚡ 3-Day Sprint to Playable Demo

### **Day 1: Foundation**
```bash
# 1. Open UE Editor with Adastrea
# 2. Enable Remote Control plugins (Project Settings → Plugins)
# 3. Verify MCP: manage_tools → list_tools (23 tools)
# 4. Build: ./build_with_ue_tools.sh Development Linux
# 5. Create test level via MCP
```

### **Day 2: Core Content**
```python
# Create essential Blueprints via MCP:
manage_blueprint(create, "BP_TradingShip", parent="Spaceship")
manage_blueprint(add_component, "BP_TradingShip", "CargoComponent")
manage_blueprint(add_component, "BP_TradingShip", "PlayerTraderComponent")

manage_blueprint(create, "BP_TradeStation", parent="SpaceStation")
manage_blueprint(set_default, "BP_TradeStation", "DefaultModuleClasses", 
    ["DockingBayModule", "MarketplaceModule", "CargoBayModule"])

manage_blueprint(create, "WBP_TradingUI", parent="UserWidget")
# Build UMG layout: item list, buy/sell buttons, credits, cargo space

# Validate 20 trade items + 5 markets:
for asset in trade_items + markets:
    manage_asset(load, asset)
    inspect(get_properties, asset)
```

### **Day 3: Gameplay Loop**
```python
# Wire trading in Blueprint:
# OnDocked → HasMarketplace() → GetMarketplace() → IsOpen() → Open WBP_TradingUI
# OnTrade → Update CargoComponent → Update Credits → Play SFX/VFX

# Place 3 stations in test level:
control_actor(spawn, "BP_TradeStation", loc=[0,0,0], name="Station_A")
control_actor(spawn, "BP_TradeStation", loc=[50000,0,0], name="Station_B")  
control_actor(spawn, "BP_TradeStation", loc=[25000,43300,0], name="Station_C")

# PIE test:
control_editor(start_pie, "MVP_TestLevel")
# Fly → Dock → Trade → Undock → Fly → Dock → Sell → Profit!
control_editor(stop_pie)
```

---

## 📋 Week 2: Polish & Package (Days 4-10)

| Day | Focus | MCP Tools |
|-----|-------|-----------|
| 4 | Create 10 market DataAssets | `manage_asset` (create MarketDataAsset) |
| 5 | Build full test level (5-10 stations) | `control_actor`, `build_environment` |
| 6 | Economy simulation (supply/demand) | `system_control`, `manage_blueprint` |
| 7 | VFX/Audio (docking, trade confirm) | `manage_effect`, `manage_audio` |
| 8 | UI polish (animations, styling) | `manage_blueprint` (UMG editing) |
| 9 | Save/Load testing | `control_editor` (PIE), `inspect` |
| 10 | Performance optimization | `system_control` (stat fps/unit/gpu) |

---

## 📦 Week 3: Demo & Pitch (Days 11-14)

| Day | Task | Validation |
|-----|------|------------|
| 11 | External playtest (5-10 testers) | Fun rating, confusion points |
| 12 | Bug fix sprint | Zero critical bugs |
| 13 | Final packaging (Shipping build) | `build_with_ue_tools.sh Shipping` |
| 14 | Pitch materials (screenshots, video, deck) | Publisher-ready package |

---

## 🎯 Success Metrics (Week 12 Targets)

| Metric | Target | How to Measure |
|--------|--------|----------------|
| Fun Rating | 75%+ | Post-playtest survey (20+ testers) |
| Buy Intent | 50%+ | Survey question: "Would you buy this?" |
| Critical Bugs | 0 | Automated tests + manual testing |
| Performance | 60 FPS | `stat fps` on GTX 1060/RX 580 class |
| Demo Length | 30 min | Timed guided playthrough |
| Stability | No crashes | 1-hour stress test |

---

## 🔑 Key MCP Commands Cheat Sheet

```bash
# List all tools
manage_tools(action="list_tools")

# Create Blueprint
manage_blueprint(action="create", name="BP_Name", parent="/Script/Module.Class")

# Add component to Blueprint
manage_blueprint(action="add_component", blueprint="BP_Name", 
    component="CompName", class="/Script/Module.ComponentClass")

# Set default property
manage_blueprint(action="set_default", blueprint="BP_Name", 
    property="PropertyName", value=Value)

# Create DataAsset
manage_asset(action="create", type="MarketDataAsset", name="DA_Name", 
    properties={...})

# Spawn actor in level
control_actor(action="spawn", class="BP_Class", transform={location:[x,y,z]}, name="ActorName")

# Start PIE
control_editor(action="start_pie", map="LevelName")

# Console command
control_editor(action="console_command", command="stat fps")

# Screenshot
control_editor(action="screenshot", name="test_name")

# Profile performance
system_control(action="profile", duration=60)
system_control(action="stat", commands=["stat fps", "stat unit", "stat gpu"])
```

---

## 📁 Critical File Paths

| System | Path |
|--------|------|
| Project Config | `Adastrea.uproject` |
| Trading Items | `Source/Adastrea/Private/Trading/TradeItemDataAsset.cpp` |
| Markets | `Source/Adastrea/Private/Trading/MarketDataAsset.cpp` |
| Economy | `Source/Adastrea/Private/Trading/EconomyManager.cpp` |
| Cargo | `Source/Adastrea/Private/Trading/CargoComponent.cpp` |
| Player Trading | `Source/Adastrea/Private/Trading/PlayerTraderComponent.cpp` |
| Stations | `Source/Adastrea/Private/Stations/SpaceStation.cpp` |
| Marketplace Module | `Source/Adastrea/Private/Stations/MarketplaceModule.cpp` |
| Docking | `Source/Adastrea/Private/Stations/DockingBayModule.cpp` |
| Ships | `Source/Adastrea/Private/Ships/Spaceship.cpp` |
| Ship Controls | `Source/Adastrea/Private/Ships/SpaceshipControlsComponent.cpp` |
| Trade Items (Content) | `Content/DataAssets/Trading/Items/*.uasset` |
| Station Blueprints | `Content/Blueprints/Stations/Modules/*.uasset` |
| Trading Blueprints | `Content/Blueprints/Trading/*.uasset` |
| MVP Docs | `docs/mvp/README.md` |
| MVP Spec | `.github/instructions/trade-simulator-mvp.instructions.md` |

---

## ⚠️ Top Risks & Mitigations

| Risk | Mitigation |
|------|------------|
| MCP connection drops | Keep UE Editor open; re-run `mcp_config_set` if ports change |
| Blueprint graph complexity | Use MCP for structure; wire complex logic manually in Editor |
| Content creation bottleneck | Use `manage_asset` loops for repetitive DataAsset creation |
| Playtester shortage | Start recruiting NOW (Discord, Reddit r/IndieDev, r/PlayMyGame) |
| UE 5.6 build issues | Use `build_with_ue_tools` scripts; check `MODULE_DEPENDENCY_FIX.md` |

---

## 🚀 START NOW

1. **Open UE Editor** → Adastrea project
2. **Enable Remote Control** → HTTP:30010, WS:30020
3. **Run**: `manage_tools(action="list_tools")` - verify 23 tools
4. **Execute Day 1 plan** above

**Full Plan**: See `MVP_FORWARD_PLAN.md` for detailed 14-day breakdown with MCP automation scripts.