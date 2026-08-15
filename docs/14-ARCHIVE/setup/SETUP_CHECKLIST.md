# Setup Checklist for Testing - Quick Reference

**For detailed information, see [WHATS_STILL_NEEDED.md](WHATS_STILL_NEEDED.md)**

## Critical Setup Items

### Must Have (Blocking Basic Testing)

- [ ] **None** - All systems are testable with existing assets

### Should Have (Enhanced Testing Experience)

- [ ] **TestMode.umap** - Create test level (workaround: use TestLevel.umap)
  - Location: `Content/Maps/TestMode.umap`
  - Guide: `Content/Blueprints/BP_TestModeSetup_Guide.txt`
  - Spec: `Content/Maps/TestMode.umap.txt`

- [ ] **Input Config Data Asset** - Create and configure enhanced input
  - Location: `Content/DataAssets/Input/DA_InputConfig`
  - Base Class: `InputConfigDataAsset`
  - Assign 30+ Input Actions
  - Guide: `Assets/EnhancedInputImplementation.md`

### Nice to Have (Future Enhancements)

- [ ] **System Integration TODOs** - Connect related systems
  - See WHATS_STILL_NEEDED.md for full list (8 integration points)
  - Priority: LOW - Individual systems work independently

## Quick Test Setup (5 Minutes)

1. **Open Project**
   ```
   Launch Adastrea.uproject in Unreal Engine 5.6
   ```

2. **Load Test Map**
   ```
   Content Browser → Maps → TestLevel.umap
   Double-click to open
   ```

3. **Configure Game Mode**
   ```
   Window → World Settings
   Game Mode Override → BP_SpaceGameMode
   ```

4. **Test Basic Systems**
   ```
   Press Play (Alt+P)
   WASD - Movement
   Mouse - Look
   Test flight controls
   ```

5. **Verify Systems Work**
   ```
   - Spaceship spawns
   - Controls respond
   - Camera follows
   - No errors in Output Log
   ```

## What's Already Set Up ✅

- ✅ BP_PlayerShip - Player spaceship Blueprint
- ✅ BP_SpaceGameMode - Game mode with auto-spawn
- ✅ TestLevel.umap - Basic test environment
- ✅ MainMenu.umap - Menu map
- ✅ 100+ Data Assets - Example content
- ✅ Enhanced Input System - C++ implementation complete
- ✅ 22 Major Game Systems - All implemented and testable

## What's NOT Blocking Testing 🟢

These are future content placeholders (intentional):

- 🟢 Sector_A_Placeholder.umap.txt
- 🟢 Sector_B_Placeholder.umap.txt
- 🟢 BP_SectorPlaceholder.txt
- 🟢 BP_SpaceStationPlaceholder.txt
- 🟢 BP_SpaceStationModulePlaceholder.txt
- 🟢 BP_PlanetPlaceholder.txt
- 🟢 SectorMapWidgetPlaceholder.txt
- 🟢 Various template placeholders

## Code TODOs - Summary

**Total**: 37 TODOs found
**Blocking Testing**: 0
**System Integrations**: 8 (low priority)
**Future Features**: 29 (planned enhancements)

See WHATS_STILL_NEEDED.md for detailed breakdown.

## Recommended Test Order

### Day 1: Basic Systems
1. ✅ Spaceship flight controls
2. ✅ Camera and controls
3. ✅ HUD display
4. ✅ Game Mode spawn logic

### Day 2: Combat & Navigation
1. ✅ Weapon systems
2. ✅ Targeting
3. ✅ Damage and shields
4. ✅ Autopilot

### Day 3: Economy & Crew
1. ✅ Trading system
2. ✅ Faction relationships
3. ✅ Personnel management
4. ✅ Quest system

### Day 4: Advanced Features
1. ✅ Save/Load
2. ✅ Tutorial system
3. ✅ Audio system
4. ✅ Performance profiling

## Getting Help

- **Setup Issues**: See [WHATS_STILL_NEEDED.md](WHATS_STILL_NEEDED.md)
- **Testing Guide**: See [TESTING_AUTOMATION_GUIDE.md](TESTING_AUTOMATION_GUIDE.md)
- **System Docs**: See `Assets/[System]Guide.md` files
- **Quick Reference**: See [CHEATSHEET.md](CHEATSHEET.md)
- **Contributing**: See [CONTRIBUTING.md](CONTRIBUTING.md)

## Status Summary

✅ **Ready for Testing**: All 22 major systems
⚠️ **Optional Setup**: TestMode.umap (can use TestLevel.umap)
🟢 **Not Blocking**: 11 placeholder files (future content)
📝 **TODOs**: 37 (none blocking basic testing)

---

**Bottom Line**: You can start testing immediately with existing assets. TestMode.umap creation is optional. All systems are functional and testable.
