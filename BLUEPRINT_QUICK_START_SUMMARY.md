# Blueprint Quick Start Summary - Adastrea

> **One-page printable summary for complete beginners**

**📖 Full Guide**: [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md)

---

## 🎯 What You're Creating

**Total**: ~100+ Blueprints organized in 3 phases  
**Time Investment**: 14-16 hours for complete setup  
**Result**: Fully playable space flight game

---

## 📋 Phase 1: Critical (15 Blueprints, 2 hours)

**Goal**: Flying ship with HUD

### Must Create:
1. ✅ **BP_SpaceGameMode** - Makes game start
2. ✅ **BP_GameInstance** - Saves progress
3. ✅ **BP_SpaceshipController** - Controls input
4. ✅ **BP_PlayerShip** - Your spaceship
5. ✅ **WBP_SpaceshipHUD** - UI display
6. ✅ **WBP_MainMenu** - Main menu
7. ✅ **IMC_Spaceship** - Input mapping
8. ✅ **IA_Move** - WASD movement
9. ✅ **IA_Look** - Mouse look
10. ✅ **IA_Boost** - Speed boost
11. ✅ **IA_Fire_Primary** - Shoot weapon
12. ✅ **IA_PauseMenu** - Pause game
13. ✅ **DA_Ship_PlayerScout** - Ship stats
14. ✅ **DA_Way_TradersGuild** - Your starting guild/Way
15. ✅ **DA_InputConfig** - Input setup

**Test**: Can you fly around and see your HUD?

---

## 📋 Phase 2: High Priority (25 Blueprints, 4 hours)

**Goal**: Combat, trading, stations

### Add These Systems:
- **5 Station Blueprints**: Main station + 4 modules
- **5 Combat Blueprints**: 3 weapons + 2 projectiles
- **5 Trading Blueprints**: UI + 4 trade items
- **3 Ship Variants**: Fighter, NPC fighter data
- **7 More Systems**: Inventory, Ways (guilds), quests, personnel

**Test**: Can you dock, trade, and shoot things?

---

## 📋 Phase 3: Complete (60+ Blueprints, 8-10 hours)

**Goal**: Production-ready game

### Polish Everything:
- **10 Station Modules** (all types)
- **15 UI Widgets** (all menus)
- **10 Input Actions** (all controls)
- **40+ Data Assets** (complete content)
- **8 Environment** (planets, asteroids, effects)

**Test**: Full playthrough from start to finish

---

## 🔧 How to Create Each Type

### Blueprint Class (Actors)
```
1. Right-click → Blueprint Class
2. Choose parent (ASpaceship, etc.)
3. Name with BP_ prefix
4. Add components
5. Compile & Save
```

### Widget (UI)
```
1. Right-click → User Interface → Widget Blueprint
2. Name with WBP_ prefix
3. Design UI in Designer tab
4. Add logic in Graph tab
5. Compile & Save
```

### Data Asset (Configuration)
```
1. Right-click → Miscellaneous → Data Asset
2. Choose parent (USpaceshipDataAsset, etc.)
3. Name with DA_ prefix
4. Fill properties
5. Save
```

### Input Action
```
1. Right-click → Input → Input Action
2. Name with IA_ prefix
3. Set value type
4. Save
```

### Input Mapping Context
```
1. Right-click → Input → Input Mapping Context
2. Name with IMC_ prefix
3. Map actions to keys
4. Save
```

---

## 📁 Where to Put Everything

```
Content/
├── Blueprints/
│   ├── Core/           ← BP_GameInstance
│   ├── GameModes/      ← BP_SpaceGameMode
│   ├── Controllers/    ← BP_SpaceshipController
│   ├── Ships/          ← BP_PlayerShip
│   ├── Stations/       ← Station blueprints
│   └── Combat/         ← Weapons, projectiles
│
├── DataAssets/
│   ├── Ships/          ← Ship stats
│   ├── Ways/           ← Way/guild configs
│   ├── Trading/        ← Trade items
│   ├── Weapons/        ← Weapon specs
│   └── Input/          ← Input config
│
├── UI/
│   ├── HUD/            ← WBP_SpaceshipHUD
│   ├── Menus/          ← Main, pause menus
│   └── Trading/        ← Trading interface
│
└── Input/
    ├── IMC_Spaceship   ← Input mapping
    └── Actions/        ← All IA_* files
```

---

## ⚡ Essential Settings

### After Creating Phase 1, Configure:

**Edit → Project Settings → Maps & Modes**:
- Default GameMode: `BP_SpaceGameMode`
- Game Default Map: `MainMenu`

**Edit → Project Settings → Input**:
- Default Player Input Class: `EnhancedPlayerInput`
- Default Input Component Class: `EnhancedInputComponent`

**Edit → Project Settings → Engine**:
- Game Instance Class: `BP_GameInstance`

---

## 🚨 Common Problems & Solutions

| Problem | Solution |
|---------|----------|
| **Ship won't spawn** | Check Game Mode → Default Pawn Class |
| **Input doesn't work** | Add IMC_Spaceship in PlayerController BeginPlay |
| **HUD not visible** | Add widget to viewport in PlayerController |
| **Data Asset values ignored** | Assign Data Asset reference to Blueprint |
| **Game crashes** | Check Output Log for errors |
| **Blueprint won't compile** | Read error message, check parent class |

---

## 📝 Naming Rules (VERY IMPORTANT!)

| Type | Prefix | Example |
|------|--------|---------|
| Blueprint Actor | BP_ | BP_PlayerShip |
| Widget | WBP_ | WBP_MainMenu |
| Data Asset | DA_ | DA_Ship_Scout |
| Input Action | IA_ | IA_Move |
| Input Mapping | IMC_ | IMC_Spaceship |

**Why?** Easy to find and organized!

---

## ✅ Testing Checkpoints

### Phase 1 Check:
- [ ] Game starts from main menu
- [ ] Can press "Start Game"
- [ ] Ship spawns in level
- [ ] WASD moves ship
- [ ] Mouse controls camera
- [ ] HUD shows on screen
- [ ] Can pause with Esc

### Phase 2 Check:
- [ ] Can approach station
- [ ] Trading UI opens
- [ ] Can fire weapons
- [ ] Projectiles spawn
- [ ] Damage works
- [ ] Inventory updates

### Phase 3 Check:
- [ ] All menus work
- [ ] Quests track
- [ ] Save/load works
- [ ] NPCs behave correctly
- [ ] No critical bugs

---

## 🎓 Learn More

**For complete instructions**: [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md)  
**For quick checklist**: [BLUEPRINT_CHECKLIST.md](BLUEPRINT_CHECKLIST.md)  
**For detailed specs**: [BLUEPRINT_REQUIREMENTS_LIST.md](BLUEPRINT_REQUIREMENTS_LIST.md)  
**For visual reference**: [BLUEPRINT_QUICK_REFERENCE.md](BLUEPRINT_QUICK_REFERENCE.md)

---

## 💡 Pro Tips

1. ✅ **Test after each phase** - Don't wait until the end
2. ✅ **Save frequently** - Ctrl+S is your friend
3. ✅ **Check Output Log** - Shows all errors
4. ✅ **Use templates** - Copy from Assets/ folder YAML files
5. ✅ **Compile often** - Catch errors early
6. ✅ **Follow naming** - Makes finding things easy
7. ✅ **Organize folders** - Use the structure above
8. ✅ **Start small** - Phase 1 first, then expand

---

## 🚀 Ready to Start?

1. **Open Unreal Engine 5.6**
2. **Load Adastrea.uproject**
3. **Open** [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
4. **Follow Phase 1** (15 Blueprints)
5. **Test your game**
6. **Continue to Phase 2**

**You've got this! 🎮**

---

**Last Updated**: November 18, 2025  
**For**: Adastrea 1.0.0-alpha  
**Engine**: Unreal Engine 5.6
