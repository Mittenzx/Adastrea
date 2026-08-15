# Blueprint Quick Reference - Adastrea

> **One-page visual reference for Blueprint creation**

**⭐ New to Blueprints?** See [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md) for step-by-step instructions!

---

## 🎯 Priority Levels

| Symbol | Priority | Description | Count |
|--------|----------|-------------|-------|
| ⚠️ | **CRITICAL** | Game won't run without these | 15 |
| 🔶 | **HIGH** | Core gameplay features | 25 |
| 🔷 | **MEDIUM** | Important functionality | 40 |
| 🟢 | **LOW** | Polish and enhancements | 20+ |

**Total Blueprints**: ~100+

---

## 📊 Blueprint Breakdown by System

```
Game Framework        ⚠️⚠️⚠️⚠️ (5)     Essential core
Ships & Combat        🔶🔶🔶🔶🔷🔷 (15+)  Flying & fighting
Stations              🔶🔶🔷🔷 (12+)     Building & docking
UI & HUD              ⚠️🔶🔶🔷🔷 (15+)  Player interface
Data Assets           ⚠️🔶🔶🔷🔷 (40+)  Game content
Input System          ⚠️⚠️⚠️ (6)       Player controls
Trading               🔶🔷 (8+)         Economy
AI & NPCs             🔷🔷 (10+)        Intelligence
Quests                🔷 (5+)           Missions
Environment           🔷🟢 (8+)         World building
Audio & VFX           🟢 (10+)          Polish
```

---

## 🚀 Getting Started Path

### Phase 1: Minimum Playable (2 hours)
```
1. BP_SpaceGameMode          → Game starts
2. BP_GameInstance           → State management
3. BP_SpaceshipController    → Player control
4. BP_PlayerShip             → Player's ship
5. WBP_SpaceshipHUD          → UI feedback
6. WBP_MainMenu              → Game entry
7. Input Actions (5)         → Controls work
8. DA_Ship_PlayerScout       → Ship stats
9. DA_Faction_PlayerAlliance → Faction system
10. DA_InputConfig           → Input setup

✓ Result: Flying ship with HUD
```

### Phase 2: Core Gameplay (4 hours)
```
Add to Phase 1:
- Station Blueprint + 3 modules
- Combat system (3 weapons, 2 projectiles)
- Trading UI
- Inventory UI
- 2-3 more ship data assets
- 2-3 more faction data assets
- NPC ship blueprint

✓ Result: Combat, trading, stations
```

### Phase 3: Complete Game (8-10 hours)
```
Add to Phase 2:
- All station modules (10)
- All UI widgets (15)
- All data assets (50+)
- Environment actors (8)
- Complete input (15 actions)
- Quests, tutorials, audio

✓ Result: Production-ready
```

---

## 📁 Essential Folder Structure

```
Content/
├── Blueprints/
│   ├── Core/              ← BP_GameInstance
│   ├── GameModes/         ← BP_SpaceGameMode
│   ├── Controllers/       ← BP_SpaceshipController
│   ├── Ships/             ← BP_PlayerShip, NPC ships
│   ├── Stations/          ← Station + modules
│   ├── Combat/            ← Projectiles, weapons
│   └── Environment/       ← Asteroids, planets
│
├── DataAssets/
│   ├── Ships/             ← Ship stats
│   ├── Factions/          ← Faction configs
│   ├── Trading/           ← Trade items, markets
│   ├── Weapons/           ← Weapon specs
│   ├── Quests/            ← Quest definitions
│   ├── Personnel/         ← Crew members
│   └── Input/             ← Input config
│
├── UI/
│   ├── HUD/               ← WBP_SpaceshipHUD
│   ├── Menus/             ← Main, Pause menus
│   ├── Trading/           ← Trading interface
│   ├── Stations/          ← Station management
│   └── Inventory/         ← Inventory UI
│
└── Input/
    ├── IMC_Spaceship      ← Input mapping
    └── Actions/           ← IA_* actions
```

---

## 🎮 Parent Classes Reference

### Actors
- **ASpaceship** → Player/NPC ships
- **ASpaceStation** → Stations
- **ASpaceStationModule** → Station modules
- **APlanet** → Planets
- **AActor** → Generic actors (asteroids, projectiles)
- **AAIController** → NPC AI controllers

### Game Framework
- **AdastreaGameMode** → Game mode
- **AdastreaGameInstance** → Game instance
- **AdastreaPlayerController** → Player controller

### Data Assets (UDataAsset)
- **USpaceshipDataAsset** → Ship stats
- **UFactionDataAsset** → Faction configs
- **UPersonnelDataAsset** → Crew members
- **UTradeItemDataAsset** → Trade items
- **UMarketDataAsset** → Market configs
- **UWeaponDataAsset** → Weapon specs
- **UQuestDataAsset** → Quest definitions
- **UHomeworldDataAsset** → Starting locations
- **UInputConfigDataAsset** → Input config

### UI (UUserWidget)
- **UAdastreaHUDWidget** → HUD
- **UTradingInterfaceWidget** → Trading
- **UStationManagementWidget** → Station UI
- **UInventoryWidget** → Inventory
- **UShipCustomizationWidget** → Ship customization
- **USectorMapWidget** → Map

### AI Logic
- **UFactionLogic** → Faction AI
- **UPersonnelLogic** → Crew AI
- **UNPCLogicBase** → Generic NPC AI

---

## 🔧 Common Blueprint Patterns

### Pattern 1: Data Asset Setup
```
1. Right-click → Data Asset
2. Choose parent (e.g., USpaceshipDataAsset)
3. Name with prefix (DA_Ship_Name)
4. Fill in properties
5. Save
```

### Pattern 2: Actor Blueprint Setup
```
1. Right-click → Blueprint Class
2. Choose parent (e.g., ASpaceship)
3. Name with prefix (BP_Ship_Name)
4. Add components (mesh, camera, etc.)
5. Set variables
6. Compile & save
```

### Pattern 3: Widget Setup
```
1. Right-click → Widget Blueprint
2. Choose parent (e.g., UAdastreaHUDWidget)
3. Name with prefix (WBP_Name)
4. Design UI layout
5. Add event graph logic
6. Compile & save
```

### Pattern 4: Input Action Setup
```
1. Right-click → Input Action
2. Name with prefix (IA_ActionName)
3. Set value type (Boolean, Vector2D, etc.)
4. Save
5. Add to IMC_Spaceship mapping context
```

---

## ⚡ Quick Commands

### In Editor
- **Create Blueprint**: Right-click → Blueprint Class
- **Create Data Asset**: Right-click → Data Asset
- **Create Widget**: Right-click → User Interface → Widget Blueprint
- **Create Input Action**: Right-click → Input → Input Action
- **Compile Blueprint**: Toolbar → Compile
- **Save**: Ctrl+S
- **Play**: Alt+P

### Project Settings
- **Maps & Modes**: Edit → Project Settings → Maps & Modes
- **Input**: Edit → Project Settings → Input
- **Engine**: Edit → Project Settings → Engine → Game Instance Class

---

## 📝 Naming Conventions

| Type | Prefix | Example |
|------|--------|---------|
| Blueprint Actor | BP_ | BP_PlayerShip |
| Widget | WBP_ | WBP_SpaceshipHUD |
| Data Asset | DA_ | DA_Ship_PlayerScout |
| Input Action | IA_ | IA_Move |
| Input Mapping Context | IMC_ | IMC_Spaceship |
| Static Mesh | SM_ | SM_Ship_Hull |
| Material | M_ | M_Ship_Paint |
| Texture | T_ | T_Ship_Diffuse |

---

## 🎯 Testing Checkpoints

### After Phase 1 (15 Blueprints)
- [ ] Game starts from main menu
- [ ] Can press "Start Game"
- [ ] Player ship spawns
- [ ] WASD moves ship
- [ ] Mouse looks around
- [ ] HUD shows info
- [ ] Can pause (Esc)

### After Phase 2 (40 Blueprints)
- [ ] Can dock at station
- [ ] Trading interface works
- [ ] Can fire weapons
- [ ] Projectiles spawn and move
- [ ] Damage is dealt
- [ ] Inventory updates
- [ ] Multiple factions exist

### After Phase 3 (100+ Blueprints)
- [ ] All UI functional
- [ ] Quests track properly
- [ ] NPCs spawn and behave
- [ ] Station editor works
- [ ] Save/load works
- [ ] Audio plays
- [ ] No critical bugs

---

## 🚨 Common Mistakes

| ❌ Don't | ✅ Do |
|---------|-------|
| Forget parent class | Always set correct parent |
| Skip naming conventions | Use BP_, WBP_, DA_, IA_ prefixes |
| Forget to compile | Compile after every change |
| Not test incrementally | Test after each Blueprint |
| Miss Project Settings | Configure Game Mode, Game Instance |
| Skip folder organization | Use recommended folder structure |
| Create without planning | Follow the phase order |
| Ignore errors | Check Output Log |

---

## 📚 Documentation Links

- **Complete List**: [BLUEPRINT_REQUIREMENTS_LIST.md](BLUEPRINT_REQUIREMENTS_LIST.md)
- **Checklist**: [BLUEPRINT_CHECKLIST.md](BLUEPRINT_CHECKLIST.md)
- **Creation Guide**: [BLUEPRINT_CREATION_GUIDE.md](BLUEPRINT_CREATION_GUIDE.md)
- **Quick Start**: [CONTENT_CREATION_QUICKSTART.md](CONTENT_CREATION_QUICKSTART.md)
- **Designer Guide**: [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md)

---

## 💡 Pro Tips

1. **Start small**: Create Phase 1 first, test, then expand
2. **Use templates**: Reference YAML templates in Assets/
3. **Save often**: Ctrl+S is your friend
4. **Check logs**: Output Log shows errors
5. **Test early**: Don't wait to test
6. **Follow naming**: Consistent names = easier to find
7. **Organize folders**: Clean structure = happy life
8. **Document as you go**: Add comments and tooltips
9. **Backup before major changes**: Copy assets
10. **Ask for help**: See README.md for resources

---

**Time Investment**:
- Phase 1 (Minimum): ~2 hours → Basic playable
- Phase 2 (Core): ~4 hours → Full gameplay
- Phase 3 (Complete): ~8-10 hours → Production-ready

**Total**: ~14-16 hours for complete Blueprint setup

---

**Last Updated**: November 16, 2025
**For**: Adastrea 1.0.0-alpha
**Engine**: Unreal Engine 5.6
