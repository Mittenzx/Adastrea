# Blueprint Creation Roadmap - Visual Guide

> **Visual progression map for creating all Blueprints in Adastrea**

**📖 Full Guide**: [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md)

---

## 🗺️ Your Journey Map

```
START HERE
    ↓
┌─────────────────────────────────────────┐
│  PHASE 1: CRITICAL FOUNDATION           │
│  15 Blueprints | 2 hours                │
│  Goal: Flying ship with HUD             │
└─────────────────────────────────────────┘
    ↓
    ├─→ Game Framework (5)
    │   ├─ BP_SpaceGameMode
    │   ├─ BP_GameInstance
    │   ├─ BP_SpaceshipController
    │   ├─ BP_PlayerShip
    │   └─ WBP_SpaceshipHUD
    │
    ├─→ Enhanced Input (6)
    │   ├─ IMC_Spaceship
    │   ├─ IA_Move
    │   ├─ IA_Look
    │   ├─ IA_Boost
    │   ├─ IA_Fire_Primary
    │   └─ IA_PauseMenu
    │
    ├─→ Data Assets (3)
    │   ├─ DA_Ship_PlayerScout
    │   ├─ DA_Faction_PlayerAlliance
    │   └─ DA_InputConfig
    │
    └─→ UI (1)
        └─ WBP_MainMenu
    ↓
✅ TEST: Can you fly?
    ↓
┌─────────────────────────────────────────┐
│  PHASE 2: CORE GAMEPLAY                 │
│  25 Blueprints | 4 hours                │
│  Goal: Combat, trading, stations        │
└─────────────────────────────────────────┘
    ↓
    ├─→ Stations (5)
    │   ├─ BP_SpaceStation_Main
    │   ├─ BP_Module_Docking
    │   ├─ BP_Module_Power
    │   ├─ BP_Module_Storage
    │   └─ BP_Module_Defence
    │
    ├─→ Combat (5)
    │   ├─ DA_Weapon_Laser
    │   ├─ DA_Weapon_Cannon
    │   ├─ DA_Weapon_Missile
    │   ├─ BP_Projectile_Laser
    │   └─ BP_Projectile_Cannon
    │
    ├─→ Ships (3)
    │   ├─ DA_Ship_PlayerFighter
    │   ├─ DA_Ship_NPC_Fighter
    │   └─ BP_Ship_NPC_Fighter
    │
    ├─→ Trading (5)
    │   ├─ WBP_TradingInterface
    │   ├─ DA_TradeItem_Food
    │   ├─ DA_TradeItem_Minerals
    │   ├─ DA_TradeItem_Technology
    │   └─ DA_Market_Station1
    │
    └─→ Systems (7)
        ├─ WBP_InventoryWidget
        ├─ DA_Faction_Merchants
        ├─ DA_Faction_Pirates
        ├─ DA_Quest_Tutorial_FirstFlight
        ├─ DA_Homeworld_EarthStation
        ├─ DA_Personnel_Captain
        └─ DA_Personnel_Pilot
    ↓
✅ TEST: Can you dock, trade, fight?
    ↓
┌─────────────────────────────────────────┐
│  PHASE 3: COMPLETE SYSTEMS              │
│  60+ Blueprints | 8-10 hours            │
│  Goal: Production-ready game            │
└─────────────────────────────────────────┘
    ↓
    ├─→ All Station Modules (6 more)
    │   ├─ BP_Module_Habitation
    │   ├─ BP_Module_Command
    │   ├─ BP_Module_Industrial
    │   ├─ BP_Module_Trade
    │   ├─ BP_Module_Research
    │   └─ BP_Module_Entertainment
    │
    ├─→ More Ships (5)
    │   ├─ DA_Ship_PlayerTrader
    │   ├─ DA_Ship_NPC_Trader
    │   ├─ DA_Ship_NPC_Hauler
    │   ├─ DA_Ship_NPC_Scout
    │   └─ BP_Ship_NPC_Trader
    │
    ├─→ Complete UI (10)
    │   ├─ WBP_PauseMenu
    │   ├─ WBP_SettingsMenu
    │   ├─ WBP_StationManagement
    │   ├─ WBP_StationEditor
    │   ├─ WBP_ShipCustomization
    │   ├─ WBP_QuestLog
    │   ├─ WBP_SectorMap
    │   ├─ WBP_CharacterCreation
    │   ├─ WBP_DialogueInterface
    │   └─ WBP_TutorialOverlay
    │
    ├─→ Complete Input (10 more)
    │   └─ (TargetNext, Fire_Secondary, Autopilot, etc.)
    │
    ├─→ More Data Assets (30+)
    │   ├─ 5 More Factions
    │   ├─ 20 More Personnel Roles
    │   ├─ 10 More Trade Items
    │   └─ 10+ More Quests, etc.
    │
    └─→ Environment (8)
        ├─ BP_Asteroid
        ├─ BP_Planet
        ├─ BP_Nebula
        ├─ BP_Explosion
        ├─ BP_EngineTrail
        ├─ BP_ShieldEffect
        ├─ BP_WarpEffect
        └─ BP_DamageEffect
    ↓
✅ TEST: Full playthrough
    ↓
┌─────────────────────────────────────────┐
│  🎉 COMPLETE!                           │
│  ~100+ Blueprints Created               │
│  Production-Ready Game                  │
└─────────────────────────────────────────┘
```

---

## 🎯 Priority Visualization

```
CRITICAL (Phase 1)     ⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️  [15]
Game won't run without these
Must complete first

HIGH (Phase 2)         🔶🔶🔶🔶🔶🔶🔶🔶🔶🔶🔶🔶🔶...  [25]
Core gameplay features
Combat, trading, stations

MEDIUM (Phase 3)       🔷🔷🔷🔷🔷🔷🔷🔷🔷...          [40+]
Important functionality
Polish and completeness

LOW (Phase 3)          🟢🟢🟢🟢🟢                     [20+]
Enhancement and polish
Visual effects, audio
```

---

## 📊 Time Investment Breakdown

```
Phase 1: CRITICAL
████████ (2 hours)
├─ Setup project settings: 15 min
├─ Create game framework: 30 min
├─ Create player ship: 30 min
├─ Setup input system: 30 min
└─ Test and debug: 15 min

Phase 2: HIGH PRIORITY
████████████████ (4 hours)
├─ Station system: 1 hour
├─ Combat system: 1 hour
├─ Trading system: 1 hour
├─ Additional systems: 45 min
└─ Test and debug: 15 min

Phase 3: COMPLETE
████████████████████████████████ (8-10 hours)
├─ All modules: 2 hours
├─ All UI widgets: 2 hours
├─ All data assets: 3 hours
├─ Environment & VFX: 1 hour
├─ Testing & polish: 2 hours
```

**Total Time**: ~14-16 hours for complete setup

---

## 🔄 Creation Workflow

```
For Each Blueprint:

1. CREATE
   ├─ Navigate to folder
   ├─ Right-click
   ├─ Choose type
   └─ Set parent class

2. CONFIGURE
   ├─ Add components (if actor)
   ├─ Set variables
   ├─ Design UI (if widget)
   └─ Fill properties (if data asset)

3. CONNECT
   ├─ Wire up events
   ├─ Link to other Blueprints
   └─ Reference data assets

4. COMPILE
   ├─ Click Compile
   ├─ Check for errors
   └─ Fix if needed

5. TEST
   ├─ Play in editor
   ├─ Verify functionality
   └─ Debug if needed

6. SAVE
   └─ Ctrl+S

Repeat for next Blueprint!
```

---

## 🎨 Blueprint Type Distribution

```
Total: ~100 Blueprints

Data Assets (40%)        ████████████████████████████████████████
├─ Ship configs
├─ Faction configs
├─ Trade items
├─ Weapon specs
├─ Personnel roles
└─ Quest definitions

Actors (25%)             █████████████████████████
├─ Ships
├─ Stations
├─ Projectiles
└─ Environment

UI Widgets (20%)         ████████████████████
├─ HUD
├─ Menus
├─ Trading interface
└─ Management screens

Input System (10%)       ██████████
├─ Input actions
└─ Mapping contexts

Game Framework (5%)      █████
├─ Game Mode
├─ Game Instance
└─ Controllers
```

---

## 🔗 Dependency Chain

```
Must Create First:
┌───────────────────┐
│ Game Framework    │ ← Start here
└─────────┬─────────┘
          │
          ├─→ Controls spawning
          │
┌─────────▼─────────┐
│ Player Ship       │
└─────────┬─────────┘
          │
          ├─→ Needs stats
          │
┌─────────▼─────────┐
│ Ship Data Assets  │
└─────────┬─────────┘
          │
          ├─→ Needs input
          │
┌─────────▼─────────┐
│ Input System      │
└─────────┬─────────┘
          │
          ├─→ Needs display
          │
┌─────────▼─────────┐
│ HUD Widget        │
└───────────────────┘

Can Create in Parallel:
┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│  Stations   │  │  Combat     │  │  Trading    │
└─────────────┘  └─────────────┘  └─────────────┘
       │                │                 │
       └────────────────┴─────────────────┘
                        │
                ┌───────▼────────┐
                │  Environment   │
                └────────────────┘
```

---

## 📈 Milestone Checkpoints

```
✅ Checkpoint 1: Game Boots
   ├─ Main menu appears
   ├─ Can click Start Game
   └─ Level loads

✅ Checkpoint 2: Player Control
   ├─ Ship spawns
   ├─ WASD moves ship
   ├─ Mouse controls camera
   └─ HUD displays

✅ Checkpoint 3: Core Loop
   ├─ Can dock at station
   ├─ Trading works
   ├─ Combat functional
   └─ Inventory updates

✅ Checkpoint 4: Systems Complete
   ├─ All menus work
   ├─ Quests track
   ├─ Save/load works
   └─ NPCs behave

✅ Checkpoint 5: Polish Complete
   ├─ All VFX present
   ├─ Audio plays
   ├─ No critical bugs
   └─ Ready for players!
```

---

## 🛠️ Tools & Resources

```
Templates:                  Documentation:
├─ SpaceshipTemplates/     ├─ BLUEPRINT_GUIDE_FOR_BEGINNERS.md
├─ FactionSetupGuide.md    ├─ BLUEPRINT_REQUIREMENTS_LIST.md
├─ PersonnelTemplates/     ├─ BLUEPRINT_CHECKLIST.md
└─ TradingTemplates/       └─ BLUEPRINT_QUICK_REFERENCE.md

Python Scripts:             Unreal Editor:
├─ YAMLtoDataAsset.py      ├─ Content Browser
├─ ProceduralGenerators.py ├─ Blueprint Editor
├─ SetupCheck.py           ├─ Widget Designer
└─ TemplateGenerator.py    └─ Output Log (debug)
```

---

## 💡 Success Strategy

```
Week 1: Foundation
├─ Mon-Tue: Phase 1 (Critical)
├─ Wed: Test and debug
├─ Thu: Phase 2 start (Stations)
└─ Fri: Phase 2 continue (Combat)

Week 2: Core Systems
├─ Mon: Phase 2 finish (Trading)
├─ Tue: Test full gameplay loop
├─ Wed: Phase 3 start (UI)
├─ Thu: Phase 3 continue (Data assets)
└─ Fri: Phase 3 finish (Environment)

Week 3: Polish
├─ Mon-Tue: Complete remaining Blueprints
├─ Wed: Full testing
├─ Thu: Bug fixes
└─ Fri: Final polish

Result: Production-ready game! 🎉
```

---

## 📞 Need Help?

**Stuck?** Check these resources:

1. **[BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md)** - Complete step-by-step guide
2. **[Troubleshooting Section](BLUEPRINT_GUIDE_FOR_BEGINNERS.md#troubleshooting-common-issues)** - Common problems & solutions
3. **Output Log** - Window → Developer Tools → Output Log (shows all errors)
4. **README.md** - Project overview and links
5. **GitHub Issues** - Ask the community

**Remember**: Everyone starts as a beginner. Take it one Blueprint at a time! 🚀

---

**Last Updated**: November 18, 2025  
**For**: Adastrea 1.0.0-alpha  
**Engine**: Unreal Engine 5.6
