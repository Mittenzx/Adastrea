# Adastrea - Forward Plan: Leveraging Unreal Engine MCP for Trade Simulator MVP Completion

**Generated**: August 9, 2026 | **Based on Analysis**: Repository state as of March 16, 2026 (Week 12 of 12 - Final Polish) | **New Capability**: Unreal Engine MCP Server (23 tools)

---

## 📊 Executive Summary

### Current State (Week 12 of 12)
- **Code Infrastructure**: ✅ Complete (33K+ lines, 22 systems) - Production ready
- **Trading MVP Systems**: ✅ Complete - TradeItemDataAsset, MarketDataAsset, EconomyManager, MarketplaceModule
- **Content Assets**: ⚠️ Partial - 20+ trade items, 5 markets, 10+ ship blueprints exist but need validation
- **Playable Demo**: 🔄 In Progress - Week 12 goal: 30-min polished demo for publishers
- **Documentation**: ✅ Comprehensive (74K+ words) - Well organized in `docs/`

### Critical Gap
The project has **excellent C++ infrastructure** but **minimal validated editor content**. Most systems exist as code but haven't been:
1. Compiled against local UE 5.6 installation
2. Tested in PIE (Play In Editor)
3. Validated with actual Blueprint/Data Asset creation in-editor
4. Packaged as a playable demo

### New Opportunity: Unreal Engine MCP
The `unreal-engine-mcp-server` provides **23 tools** to control Unreal Engine via Remote Control API:
- **Asset Management**: Create/import/manage assets, materials, textures
- **Blueprint Creation**: Create Blueprints/UMG widgets, add components, manipulate graphs
- **Actor Control**: Spawn actors, set transforms, add components, manage tags
- **Editor Control**: Start/stop PIE, viewport camera, console commands, screenshots
- **Level Management**: Load/save levels, configure streaming, build lighting
- **Environment Building**: Landscapes, foliage, procedural terrain, lighting, splines
- **Animation/Physics**: Animation Blueprints, blend spaces, Control Rig/IK
- **Sequences**: Level Sequences for cinematics
- **Inspection**: Read/write UObject properties, export snapshots
- **Audio/VFX**: Niagara, audio components, spatial audio
- **Geometry/PCG**: Procedural meshes, PCG graphs
- **Gameplay**: GAS, Characters, Combat, AI, Inventory, Interaction, Networking

---

## 🎯 Phase 1: UE MCP Setup & Validation (Days 1-2)

### 1.1 Configure Unreal Engine Remote Control
```bash
# In Unreal Editor (Project Settings → Plugins → Remote Control):
# Enable: "Remote Control WebSocket" + "Remote Control HTTP"
# Default ports: HTTP=30010, WS=30020
```
Then verify MCP connection:
```
MCP Tool: manage_tools → list_tools (verify all 23 tools available)
```

### 1.2 Validate Project Compilation
```bash
# Use UE Build Tools (already configured in repo):
./build_with_ue_tools.sh Development Linux
# Or on Windows:
build_with_ue_tools.bat Development Win64
```

### 1.3 Create Baseline Test Level
```
MCP Tool: manage_level → create new level "MVP_TestLevel"
MCP Tool: control_editor → start PIE
MCP Tool: inspect → verify AdastreaGameMode loads correctly
```

---

## 🎯 Phase 2: Content Creation via MCP (Days 3-7)

### 2.1 Create Core Trading Blueprints (Priority: CRITICAL)

**Using MCP `manage_blueprint` tool:**

| Blueprint | Parent Class | Key Components | MCP Actions |
|-----------|--------------|----------------|-------------|
| `BP_TradingShip` | `ASpaceship` | `UCargoComponent`, `UPlayerTraderComponent` | Create BP, add components, set defaults |
| `BP_TradeStation` | `ASpaceStation` | `UDockingBayModule`, `UMarketplaceModule`, `UCargoBayModule` | Create BP, add module classes to DefaultModuleClasses |
| `BP_MarketplaceModule` | `AMarketplaceModule` | MarketDataAsset reference | Create BP, assign MarketDataAsset |
| `WBP_TradingUI` | `UserWidget` | Item list, buy/sell buttons, cargo display | Create UMG widget, build layout |

**MCP Workflow Example:**
```python
# Pseudocode for MCP automation
manage_blueprint(action="create", name="BP_TradingShip", parent="/Script/Adastrea.Spaceship")
manage_blueprint(action="add_component", blueprint="BP_TradingShip", component="CargoComponent", class="/Script/Adastrea.CargoComponent")
manage_blueprint(action="add_component", blueprint="BP_TradingShip", component="PlayerTraderComponent", class="/Script/Adastrea.PlayerTraderComponent")
manage_blueprint(action="set_default", blueprint="BP_TradingShip", property="CargoComponent.MaxCapacity", value=50)
```

### 2.2 Validate Existing Data Assets
**Using MCP `manage_asset` tool:**

```python
# Verify all 20+ trade items load correctly
for item in trade_items:
    manage_asset(action="load", path=f"/Game/DataAssets/Trading/Items/{item}")
    inspect(action="get_properties", object=item)
    # Validate: BasePrice > 0, VolumePerUnit > 0, ItemID not None
```

### 2.3 Create Market Data Assets for 10 Stations
**Per MVP spec: 10 stations with varied economies**

| Station | Type | MarketDataAsset | Key Imports | Key Exports |
|---------|------|----------------|-------------|-------------|
| Agricultural | OpenMarket | DA_Market_Agricultural | Technology, Machinery | Food, Water |
| Industrial | OpenMarket | DA_Market_Industrial | Raw Materials, Food | Steel, Components |
| Mining | OpenMarket | DA_Market_Mining | Food, Equipment | Ore, Rare Earth |
| Tech Hub | OpenMarket | DA_Market_TechHub | Rare Earth, Components | Electronics, AI Cores |
| Luxury Port | LuxuryBazaar | DA_Market_Luxury | Art, Jewelry | Luxury Beverages, Jewelry |
| ... | ... | ... | ... | ... |

**MCP Automation:**
```python
manage_asset(action="create", type="MarketDataAsset", name="DA_Market_Agricultural", properties={
    "MarketName": "Agricultural Exchange",
    "MarketType": "OpenMarket",
    "MarketSize": "Medium",
    "SellPriceMarkup": 1.1,
    "BuyPriceMarkdown": 0.9,
    "Inventory": [/* 5-10 trade items with supply/demand */]
})
```

### 2.4 Build Test Level with Stations
**Using MCP `control_actor` and `build_environment`:**

```python
# Create level with 3 stations in a triangle (5-min flight each)
manage_level(action="load", level="MVP_TestLevel")
control_actor(action="spawn", class="BP_TradeStation", transform={location: [0,0,0]}, name="Station_Agri")
control_actor(action="spawn", class="BP_TradeStation", transform={location: [50000,0,0]}, name="Station_Industrial")
control_actor(action="spawn", class="BP_TradeStation", transform={location: [25000,43300,0]}, name="Station_Mining")
# Add player start, skybox, basic lighting
build_environment(action="add_lighting", type="sky_atmosphere")
build_environment(action="add_lighting", type="directional_light")
```

---

## 🎯 Phase 3: Gameplay Integration & Testing (Days 8-12)

### 3.1 Wire Trading Loop in Blueprint
**Using MCP `manage_blueprint` for graph manipulation:**

```python
# In BP_TradingShip or PlayerController:
# Event: OnDockedAtStation
#   → Get Station reference
#   → Check HasMarketplace()
#   → Get MarketplaceModule()
#   → Check IsAvailableForTrading()
#   → Get MarketDataAsset()
#   → Open WBP_TradingUI with MarketData
# Event: OnTradeComplete
#   → Update CargoComponent
#   → Update Credits
#   → Play purchase sound/VFX
```

### 3.2 Add Economy Simulation
**Using MCP to configure EconomyManager:**

```python
# In GameMode or GameInstance:
manage_blueprint(action="set_default", blueprint="BP_SpaceGameMode", 
    property="EconomyManager.StockRefreshRate", value=24.0)
manage_blueprint(action="set_default", blueprint="BP_SpaceGameMode",
    property="EconomyManager.TransactionTaxRate", value=0.05)
```

### 3.3 PIE Testing & Iteration
**Using MCP `control_editor`:**

```python
control_editor(action="start_pie", map="MVP_TestLevel")
# Automated test sequence:
control_editor(action="console_command", command="Adastrea.TestTradingLoop")
# Or manual: navigate ship, dock, trade, undock, fly, repeat
control_editor(action="screenshot", name="trading_ui_test")
control_editor(action="stop_pie")
```

### 3.4 Performance Validation
**Using MCP `system_control`:**

```python
system_control(action="profile", duration=60, map="MVP_TestLevel")
system_control(action="stat", commands=["stat fps", "stat unit", "stat gpu"])
# Target: 60 FPS on mid-range PC (GTX 1060 / RX 580 class)
```

---

## 🎯 Phase 4: Polish & Demo Packaging (Days 13-16)

### 4.1 Visual Polish via MCP
```python
# Add VFX for docking, trading confirmation
manage_effect(action="create_niagara", name="NS_DockingConfirmation", template="DockingRing")
manage_effect(action="create_niagara", name="NS_TradeComplete", template="CurrencySparkle")
# Attach to Blueprint events
```

### 4.2 Audio Integration
```python
manage_audio(action="import", file="docking_chime.wav", name="SFX_Docking")
manage_audio(action="import", file="cash_register.wav", name="SFX_TradeComplete")
# Add to Blueprint events via manage_blueprint
```

### 4.3 UI Polish
```python
manage_blueprint(action="edit_umg", widget="WBP_TradingUI", 
    actions=[
        {"type": "style", "widget": "BuyButton", "properties": {"BackgroundColor": "Green"}},
        {"type": "style", "widget": "SellButton", "properties": {"BackgroundColor": "Red"}},
        {"type": "animation", "widget": "ProfitText", "anim": "FadeInOut"}
    ])
```

### 4.4 Save/Load System
```python
# Verify SaveGameSubsystem works
manage_blueprint(action="test", blueprint="BP_SpaceGameMode",
    test="SaveGameSubsystem.SaveAndLoadCharacter")
```

---

## 🎯 Phase 5: Demo Build & Validation (Days 17-18)

### 5.1 Package Demo Build
```bash
# Using existing build scripts
./build_with_ue_tools.sh Shipping Linux
# Or UE Editor: File → Package Project → Windows/Linux
```

### 5.2 Automated Validation Suite
```python
# Run comprehensive tests
control_editor(action="run_automation", test_suite="Adastrea.MVPValidation")
# Includes: smoke tests, trading loop test, save/load test, performance test
```

### 5.3 Create Demo Package
- 30-minute guided experience
- Clear progression: Starter ship → 3 trades → Upgrade → Advanced route
- No crashes, no critical bugs
- 60 FPS target met

---

## 📋 MCP Tool Mapping for Adastrea Systems

| Adastrea System | MCP Tool(s) | Key Actions |
|----------------|-------------|-------------|
| **Trading UI** | `manage_blueprint` (UMG), `manage_asset` (DataAssets) | Create WBP_TradingUI, wire MarketDataAsset |
| **Ships** | `manage_blueprint`, `control_actor` | Create BP_TradingShip, spawn in level |
| **Stations** | `manage_blueprint`, `control_actor`, `build_environment` | Create modular station BPs, place in level |
| **Markets** | `manage_asset` (MarketDataAsset), `inspect` | Create/validate 10 market configs |
| **Economy** | `system_control`, `manage_blueprint` | Configure EconomyManager, test simulation |
| **VFX/Audio** | `manage_effect`, `manage_audio` | Docking, trade confirmation, UI sounds |
| **Testing** | `control_editor` (PIE), `system_control` (profile) | Automated playtesting, performance validation |
| **Packaging** | `control_editor` (commandline) | Build shipping configuration |

---

## 🚀 Immediate Next Steps (This Week)

### Day 1 (Today)
1. [ ] **Start Unreal Editor** with Adastrea project
2. [ ] **Enable Remote Control** plugins (HTTP:30010, WS:30020)
3. [ ] **Verify MCP Connection** - run `manage_tools → list_tools`
4. [ ] **Build Project** - verify compilation with `build_with_ue_tools`
5. [ ] **Create Test Level** - `MVP_TestLevel` via MCP

### Day 2
1. [ ] **Validate All Data Assets** - load 20 trade items + 5 markets via MCP
2. [ ] **Create Core Blueprints** - BP_TradingShip, BP_TradeStation, WBP_TradingUI
3. [ ] **Wire Basic Trading** - dock → trade → undock loop in Blueprint
4. [ ] **First PIE Test** - manual playthrough of one trade route

### Day 3-4
1. [ ] **Create 10 Station Markets** - using MCP asset creation
2. [ ] **Build Full Test Level** - 3-5 stations in triangle formation
3. [ ] **Add Economy Simulation** - configure supply/demand dynamics

### Day 5-7
1. [ ] **Complete Trading Loop** - buy → fly → sell → upgrade → repeat
2. [ ] **Add Polish** - VFX, audio, UI animations
3. [ ] **Save/Load Testing** - verify persistence works

### Day 8-10
1. [ ] **External Playtest** - recruit 5-10 testers (Week 12 requirement: 20+)
2. [ ] **Bug Fix Sprint** - address all critical issues
3. [ ] **Performance Optimization** - hit 60 FPS target

### Day 11-12
1. [ ] **Final Demo Package** - build shipping, create distribution
2. [ ] **Publisher Pitch Materials** - screenshots, video, pitch deck
3. [ ] **Documentation Finalization** - update all docs with final status

---

## 🎯 Success Criteria (Week 12 Targets)

| Metric | Target | Validation Method |
|--------|--------|-------------------|
| Fun Rating | 75%+ "had fun" | External playtest survey (20+ testers) |
| Buy Intent | 50%+ "would buy" | Post-playtest survey |
| Critical Bugs | 0 | Automated test suite + manual testing |
| Performance | 60 FPS | `stat fps` on mid-range hardware |
| Demo Length | 30 minutes | Timed playthrough |
| Stability | No crashes | 1-hour stress test |

---

## ⚠️ Risk Mitigation

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| MCP connection issues | Medium | High | Fallback: manual Blueprint creation in Editor |
| UE 5.6 compilation errors | Low | High | Use `build_with_ue_tools` scripts; check `MODULE_DEPENDENCY_FIX.md` |
| Blueprint wiring complexity | Medium | Medium | Use MCP `manage_blueprint` for graph manipulation; keep logic simple |
| Content creation bottleneck | High | High | MCP automation for repetitive asset creation; procedural generators |
| Playtester recruitment | Medium | High | Start recruiting NOW; use Discord, Reddit, indie dev communities |

---

## 🔗 Key Files to Reference

### Project Config
- `/c/Users/akuma/Adastrea/Adastrea.uproject` - Engine 5.6, modules, plugins
- `/c/Users/akuma/Adastrea/Source/Adastrea/Adastrea.Build.cs` - Dependencies

### Trading System (MVP Core)
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Trading/TradeItemDataAsset.cpp/h`
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Trading/MarketDataAsset.cpp/h`
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Trading/EconomyManager.cpp/h`
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Trading/CargoComponent.cpp/h`
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Trading/PlayerTraderComponent.cpp/h`

### Station System
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Stations/SpaceStation.cpp/h`
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Stations/MarketplaceModule.cpp/h`
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Stations/DockingBayModule.cpp/h`

### Ship System
- `/c/Users/akuma/akuma/Adastrea/Source/Adastrea/Private/Ships/Spaceship.cpp/h`
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Ships/SpaceshipControlsComponent.cpp/h`
- `/c/Users/akuma/Adastrea/Source/Adastrea/Private/Ships/ShipCustomizationComponent.cpp/h`

### Data Assets (Content)
- `/c/Users/akuma/Adastrea/Content/DataAssets/Trading/Items/` - 20 trade items
- `/c/Users/akuma/Adastrea/Content/Blueprints/Trading/` - Trading Blueprints
- `/c/Users/akuma/Adastrea/Content/Blueprints/Stations/Modules/` - Station modules

### Documentation
- `/c/Users/akuma/Adastrea/docs/mvp/README.md` - MVP documentation hub
- `/c/Users/akuma/Adastrea/docs/mvp/TRADING_MVP_GUIDE.md` - Complete 12-week roadmap
- `/c/Users/akuma/Adastrea/.github/instructions/trade-simulator-mvp.instructions.md` - MVP spec
- `/c/Users/akuma/Adastrea/ROADMAP.md` - Current week status

---

## 📝 Notes for Future Sessions

1. **MCP Server Persistence**: The Unreal Engine MCP server needs UE Editor running with Remote Control enabled. If Editor restarts, re-verify connection.

2. **Port Configuration**: Default ports HTTP=30010, WS=30020. If changed in UE Project Settings, update MCP config via `mcp_config_set`.

3. **Blueprint Graph Manipulation**: The `manage_blueprint` tool supports adding nodes, connections, and setting properties. Complex logic may require multiple calls.

4. **Async Operations**: Some MCP tools (PIE start, asset import) are async. Use `control_editor` with appropriate waits.

5. **Cross-Platform**: Build scripts support Windows/Linux/Mac. Test on target platform before demo packaging.

---

**Status**: READY TO EXECUTE - All infrastructure in place, MCP configured, clear 12-day plan to Week 12 demo.

**Next Action**: Start Unreal Editor, enable Remote Control, verify MCP connection, begin Phase 1.