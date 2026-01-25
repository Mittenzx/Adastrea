# Centralized Docking Configuration - Quick Reference

**Last Updated**: 2026-01-25  
**Status**: Implementation Complete (C++ + Documentation)

## Overview

Docking configuration can now be centralized using a shared Data Asset instead of configuring every ship individually.

## Problem Solved

**Before (Old Way):**
```
BP_PlayerShip → Configure docking
BP_NPCShip → Configure docking
BP_TraderShip → Configure docking
BP_MiningShip → Configure docking
...repeat for every ship
```

**After (New Way):**
```
DA_DefaultDockingSettings → Configure once
  ↓
All ships reference this asset
  ↓
Done!
```

## Quick Setup (3 Steps)

### Step 1: Create Docking Settings Asset (One Time)

1. Open Unreal Editor
2. Content Browser → Right-Click → **Data Asset** → **DockingSettingsDataAsset**
3. Name it: `DA_DefaultDockingSettings`
4. Double-click to open
5. Configure:
   ```
   Docking | UI:
     - Docking Prompt Widget Class: WBP_DockingPrompt
     - Trading Interface Class: WBP_TradingInterface
   
   Docking | Parameters:
     - Docking Range: 2000.0
   
   Docking | Parameters (Future):
     - Docking Duration: 1.0 (not yet implemented - safe to leave at default)
     - Undock Impulse Strength: 500.0 (not yet implemented - safe to leave at default)
     - Docking Curve: (Optional, not yet implemented)
   ```
   **Note**: Parameters in the "Future" category are placeholders for upcoming features and do not affect current gameplay.
6. Save

### Step 2: Update Ship Blueprints

For each ship (BP_PlayerShip, BP_NPCShip, etc.):

1. Open ship Blueprint
2. Click **Class Defaults** button
3. Find **Docking** section
4. Set **Docking Settings**: `DA_DefaultDockingSettings`
5. Compile and Save

### Step 3: Test

1. Play in Editor
2. Fly to a station with docking bay
3. Press F to dock
4. Verify docking prompt appears
5. Verify trading UI opens after docking

## Benefits

✅ **Single Source of Truth** - Change one asset, update all ships  
✅ **Consistency** - All ships use same docking behavior  
✅ **Less Work** - No need to configure every ship  
✅ **Easy Updates** - Update docking range globally in seconds  
✅ **Backward Compatible** - Old ships still work without changes  

## Advanced: Multiple Docking Profiles

Create different profiles for different ship types:

**Note**: Duration and Impulse parameters are placeholders for future features and do not currently affect gameplay. Only DockingRange is actively used.

**Small/Fast Ships:**
- Asset: `DA_DockingSettings_Small`
- Range: 1500.0 (tighter approach)
- Duration: 0.5 (quick docking) [FUTURE FEATURE - not yet active]
- Impulse: 800.0 (fast exit) [FUTURE FEATURE - not yet active]

**Large/Slow Ships:**
- Asset: `DA_DockingSettings_Large`
- Range: 3000.0 (wider approach)
- Duration: 2.0 (slow, careful) [FUTURE FEATURE - not yet active]
- Impulse: 300.0 (slow exit) [FUTURE FEATURE - not yet active]

**VIP Ships:**
- Asset: `DA_DockingSettings_VIP`
- Custom trading UI: `WBP_VIPTradingUI`
- Special effects and animations [FUTURE FEATURE - not yet active]

## Migration from Old System

**If you have existing ships with individual docking properties:**

1. Keep the old properties (they act as fallback)
2. Add `DockingSettings` reference
3. Ship will automatically use settings if present
4. Old properties ignored when settings is set
5. No breaking changes!

**Property Priority:**
```
If DockingSettings is set:
  ├─ Use DockingSettings.DockingRange ✅
  ├─ Use DockingSettings.DockingPromptWidgetClass ✅
  └─ Use DockingSettings.TradingInterfaceClass ✅

If DockingSettings is NOT set:
  ├─ Use Ship.DockingRange (fallback) ✅
  ├─ Use Ship.DockingPromptWidgetClass (fallback) ✅
  └─ Use Ship.TradingInterfaceClass (fallback) ✅
```

## API Reference

**C++ Helper Functions (automatically called):**
```cpp
// Get effective docking range (from settings or fallback)
float GetEffectiveDockingRange() const;

// Get effective widget class (from settings or fallback)
TSubclassOf<UUserWidget> GetEffectiveDockingPromptWidgetClass() const;
TSubclassOf<UUserWidget> GetEffectiveTradingInterfaceClass() const;
```

**Blueprint Nodes:**
- All existing docking nodes work unchanged
- No Blueprint changes required
- Helper functions are called automatically

## Troubleshooting

**Docking prompt doesn't show:**
- Check `DockingSettings.DockingPromptWidgetClass` is set
- Or check ship's fallback `DockingPromptWidgetClass`
- Verify widget class exists in Content Browser

**Trading UI doesn't open:**
- Check `DockingSettings.TradingInterfaceClass` is set
- Or check ship's fallback `TradingInterfaceClass`
- Verify widget class exists in Content Browser

**Docking range seems wrong:**
- Check `DockingSettings.DockingRange` value
- Verify settings asset is referenced in ship
- Check console logs for effective range being used

## Files Changed

**C++ Files:**
- `Source/Adastrea/Public/Ships/DockingSettingsDataAsset.h` (NEW)
- `Source/Adastrea/Private/Ships/DockingSettingsDataAsset.cpp` (NEW)
- `Source/Adastrea/Public/Ships/Spaceship.h` (MODIFIED)
- `Source/Adastrea/Private/Ships/Spaceship.cpp` (MODIFIED)

**Documentation:**
- `docs/mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md` (UPDATED)
- `docs/reference/DOCKING_CENTRALIZED_CONFIG.md` (THIS FILE)

## Related Documentation

- [Trading and Docking Complete Guide](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md) - Full system documentation
- [Simple Docking Setup](SIMPLE_DOCKING_SETUP.md) - Basic docking configuration
- [Docking System Quick Reference](DOCKING_QUICK_REFERENCE.md) - API reference

---

**Questions?** See the full guide at [TRADING_AND_DOCKING_COMPLETE_GUIDE.md](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md) section "Centralized Docking Configuration"
