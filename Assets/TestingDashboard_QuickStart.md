# Testing Dashboard - Quick Start Guide

> **💡 Looking for the simplest setup?** See [TestingDashboard_EasySetup.md](TestingDashboard_EasySetup.md) - only 6 steps, 10 minutes to get started!

## What You Asked For ✓

> "We want to check off each part of the game as we test it so configuring each test as I play the game would be best for me"

**This is exactly what the Testing Dashboard provides!**

## What It Does

The Testing Dashboard is an **in-game overlay** that lets you:
1. ✅ **Check off game features** as you test them
2. 🎮 **Configure tests during gameplay** (spawn ships, enable debug modes)
3. 📝 **Add notes** to document what you found
4. 📊 **Track progress** (see percentage complete)
5. 💾 **Auto-saves** your testing progress
6. 📄 **Export results** to share with your team

## How It Looks

```
┌────────────────────────────────────────────────────────┐
│  Testing Dashboard                   Progress: 63% ✓   │
├────────────────────────────────────────────────────────┤
│  Category: [All ▼]    [✓] Show Only Uncompleted        │
├────────────────────────────────────────────────────────┤
│                                                         │
│  CORE SYSTEMS                                          │
│  [✓] Ship Movement        ← Already tested             │
│      Notes: Movement smooth, all axes working          │
│                                                         │
│  [✓] Ship Collision       ← Just tested this!          │
│      Notes: Works correctly                            │
│                                                         │
│  COMBAT SYSTEMS                                        │
│  [✓] Weapon Firing                                     │
│      Notes: Energy weapons perfect                     │
│                                                         │
│  [ ] Targeting System     ← Need to test              │
│      Notes: _______                                    │
│                                                         │
│  [ ] Shields & Damage     ← Not tested yet            │
│      Notes: _______                                    │
│                                                         │
├────────────────────────────────────────────────────────┤
│  Quick Actions:                                        │
│  Ship: [Fighter ▼] [Spawn Ship]                       │
│  Debug: [Show FPS ▼] [Enable]                         │
├────────────────────────────────────────────────────────┤
│  [Export Results]  [Reset Progress]  [Close]          │
└────────────────────────────────────────────────────────┘
```

## Step-by-Step Usage

### Step 1: Open the Dashboard

**During gameplay, press F12** (or your configured hotkey)

The dashboard appears as an overlay - your game keeps running!

### Step 2: Test a Feature

For example, let's test ship movement:

1. **Close the dashboard** (F12) to return to gameplay
2. **Fly your ship** - test all directions, rotation, etc.
3. **Confirm it works** as expected

### Step 3: Check It Off

1. **Open the dashboard** (F12)
2. **Find "Ship Movement"** in the list
3. **Click the checkbox** ✓
4. **Optionally add notes**: "Movement smooth, all axes working"
5. Dashboard **auto-saves** your progress

### Step 4: Configure Next Test

Want to test combat? Spawn an enemy ship:

1. In the dashboard, find the **"Quick Actions"** section
2. Select ship type: **"Enemy Fighter"**
3. Click **"Spawn Ship"**
4. The ship appears in front of you!
5. Close dashboard and start testing combat

### Step 5: Continue Testing

Repeat the cycle:
```
F12 (Open) → Check off feature → Add notes → F12 (Close) → Test → Repeat
```

### Step 6: Export Results

At the end of your session:

1. Open dashboard (F12)
2. Click **"Export Results"**
3. Results saved to: `Saved/TestResults/TestingProgress.txt`
4. Share with your team!

## Example Testing Session

Let's walk through a real testing session:

```
┌─────────────────────────────────────────────────────────┐
│ TIME: 10:00 AM - Start Testing Session                 │
└─────────────────────────────────────────────────────────┘

F12 → Open Dashboard
  Progress: 0% (0/19 features)
  All features unchecked

F12 → Close Dashboard

--- Test ship movement for 5 minutes ---
  ✓ Forward/backward works
  ✓ Strafe left/right works
  ✓ Rotation works

F12 → Open Dashboard
  [✓] Ship Movement
  Notes: "All movement modes work. Rotation feels smooth."
  Progress: 5% (1/19 features)

F12 → Close, continue testing

--- Test collisions ---
  ✓ Hit asteroid - collision detected
  ✓ Hit station - collision detected

F12 → Open Dashboard
  [✓] Ship Collision
  Notes: "Collisions detected properly."
  Progress: 11% (2/19 features)

--- Want to test combat, need enemy ship ---

F12 → Open Dashboard
  Quick Actions:
    Ship Type: "Gunship" ← Select this
    [Spawn Ship] ← Click this

Ship spawns 1000m away!

F12 → Close, test combat

--- Test weapons ---
  ✓ Energy weapons fire
  ✓ Hit detection works
  ✓ Damage applied

F12 → Open Dashboard
  [✓] Weapon Firing
  Notes: "Energy weapons work perfectly"
  Progress: 16% (3/19 features)

--- Continue testing... ---

F12 → Open Dashboard
  [Export Results] ← Click at end of session

Results saved!
  File: Saved/TestResults/Session_2024_11_22.txt

┌─────────────────────────────────────────────────────────┐
│ TIME: 12:00 PM - Session Complete                      │
│ Progress: 63% (12/19 features tested)                  │
└─────────────────────────────────────────────────────────┘
```

## Pre-Configured Features

The dashboard comes with **19 testable features** across **8 categories**:

### ⚙️ Core Systems
- Ship Movement
- Ship Collision

### ⚔️ Combat Systems
- Weapon Firing
- Targeting System
- Shields & Damage

### 🧭 Navigation Systems
- Autopilot
- Sector Map

### 🏭 Station Systems
- Docking
- Station Services

### 💰 Trading Systems
- Trading

### 👥 Faction Systems
- Faction Reputation
- Faction Diplomacy

### 🤖 AI Systems
- NPC Ships
- AI Combat

### 🖥️ UI Systems
- HUD Display
- Menus & Settings
- Inventory System

### ⚡ Performance
- Frame Rate
- Load Times

**All customizable!** You can add/remove features as needed.

## Quick Actions

### Spawn Ships During Testing

```
1. Open dashboard (F12)
2. Select ship type from dropdown
3. Click "Spawn Ship"
4. Ship appears in front of you
5. Close dashboard and test!
```

**Available ships:**
- Default Fighter
- Scout Ship
- Cargo Hauler
- Gunship
- Explorer
- Mining Vessel
- Interceptor
- Battlecruiser

### Enable Debug Modes

```
1. Open dashboard (F12)
2. Select debug mode from dropdown
3. Click "Enable"
4. Debug visualization activates!
```

**Available debug modes:**
- None (disable all)
- Show FPS
- Show Collision
- Show Navigation
- Show All Stats
- God Mode
- Infinite Resources
- Show AI Debug
- Show Physics
- Show Networking

## Filter Options

### Show Only Uncompleted

Check this box to **hide features you've already tested**.

Focuses your attention on what still needs testing!

### Category Filter

Select a category to **show only features in that system**:
- All (default)
- Core
- Combat
- Navigation
- Stations
- Trading
- Factions
- AI
- UI
- Performance

Great for focused testing of one system at a time!

## Progress Tracking

The dashboard shows your **overall testing progress**:

```
Progress: 63.2% (12/19 features tested)
```

This updates automatically as you check off features.

## Export Results

Click "Export Results" to generate a report:

```
=== Adastrea Testing Results ===

Total Features: 19
Tested: 12
Progress: 63.2%

=== Core ===
[✓] Ship Movement
    Notes: Movement smooth, all axes working
[✓] Ship Collision
    Notes: Collisions detected properly.

=== Combat ===
[✓] Weapon Firing
    Notes: Energy weapons work perfectly
[ ] Targeting System
[ ] Shields & Damage

...
```

**Saved to:** `Saved/TestResults/TestingProgress.txt`

Perfect for:
- Team status updates
- Bug reports
- QA documentation
- Release checklists

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **F12** | Toggle dashboard on/off |
| **ESC** | Close dashboard |

## Tips & Tricks

### 💡 Pro Tips

1. **Test → Check → Note**
   - Always add notes when checking off features
   - Document any issues you find
   - Notes save automatically

2. **Use Categories**
   - Test one system at a time
   - Use category filter to focus
   - Reduces overwhelm

3. **Spawn Strategically**
   - Spawn ships at different distances
   - Test different ship types
   - Create specific test scenarios

4. **Export Frequently**
   - Export after each session
   - Date your exports: `TestSession_2024_11_22.txt`
   - Build a testing history

5. **Don't Forget Progress Saves**
   - Dashboard auto-saves when you close it
   - Progress persists across game restarts
   - Continue where you left off!

### ⚠️ Common Mistakes

❌ **Don't:** Check off features without testing
❌ **Don't:** Forget to add notes
❌ **Don't:** Reset progress accidentally
❌ **Don't:** Spam ship spawning (lag!)

✅ **Do:** Test thoroughly before checking
✅ **Do:** Document issues you find
✅ **Do:** Export results regularly
✅ **Do:** Use category filters

## Customizing the Dashboard

Want to add your own testable features?

### In Blueprint:

Override `LoadDefaultFeatures`:

```
Event LoadDefaultFeatures (Override)
  ↓
Clear Testable Features Array
  ↓
Create New Feature:
    Name: "My Custom System"
    Description: "Test my custom feature"
    Category: "Custom"
    Tested: False
  ↓
Add to Testable Features Array
  ↓
Repeat for all your features
```

### In C++:

Edit `TestingDashboardWidget.cpp`, function `LoadDefaultFeatures_Implementation()`:

```cpp
Feature.FeatureName = FText::FromString(TEXT("My Feature"));
Feature.Description = FText::FromString(TEXT("Test description"));
Feature.Category = TEXT("MyCategory");
TestableFeatures.Add(Feature);
```

## Integration with Your Workflow

### For Solo Developers

```
1. Open game
2. F12 → Dashboard
3. Check off features as you build them
4. Track development progress!
```

### For QA Teams

```
1. Assign features to testers
2. Each tester uses dashboard
3. Export results at end of day
4. Combine reports for full picture
```

### For Bug Reports

```
When reporting a bug:
1. Note which feature has the bug
2. Add details in dashboard notes
3. Export results
4. Attach to bug report
```

## Getting Help

### Dashboard Won't Open?
- Check F12 key binding
- Verify widget is added to viewport
- Check console for errors

### Progress Not Saving?
- Check Saved folder permissions
- Look for save errors in log
- Verify dashboard closes properly

### Can't Spawn Ships?
- Override `SpawnTestShip` in Blueprint
- Implement actual spawn logic
- Check GameMode has spawn functions

## Next Steps

1. **Launch your game**
2. **Press F12** to open dashboard
3. **Start testing** and checking off features!
4. **Export results** when done

**That's it!** You're ready to start systematic testing with progress tracking.

---

## Summary

The Testing Dashboard gives you:

✅ **Interactive checklist** - Check off features as you test
✅ **Progress tracking** - See what's done, what's left
✅ **Quick actions** - Configure tests during gameplay
✅ **Note taking** - Document findings
✅ **Auto-save** - Never lose progress
✅ **Export** - Share results with team

**Perfect for systematic game testing!**

Press F12 and start testing! 🚀
