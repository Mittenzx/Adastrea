# Testing Widgets Comparison Guide

## Overview

Adastrea provides **two complementary testing widgets** to support different testing workflows:

1. **TestSettingsWidget** - Pre-game configuration screen
2. **TestingDashboardWidget** - In-game testing checklist (NEW!)

## Quick Comparison

| Feature | TestSettingsWidget | TestingDashboardWidget |
|---------|-------------------|------------------------|
| **When Used** | Before starting game | During gameplay |
| **Purpose** | Configure initial test setup | Track testing progress |
| **Visibility** | Shows once at start | Toggle on/off with hotkey |
| **Main Function** | Select ship/difficulty/debug | Check off tested features |
| **Persistence** | One-time configuration | Saves progress across sessions |
| **User Type** | All testers | QA & developers |
| **Complexity** | Simple dropdown menus | Comprehensive dashboard |

## Use Case: Which Widget Do I Need?

### Use TestSettingsWidget When:
- ✅ Starting a new test session
- ✅ Need to configure initial game state
- ✅ Want to set up testing parameters before gameplay
- ✅ Prefer not to see testing UI during gameplay
- ✅ Need simple, quick configuration

### Use TestingDashboardWidget When:
- ✅ Testing multiple features in one session
- ✅ Need to track what's been tested
- ✅ Want to configure tests dynamically during gameplay
- ✅ Need to document testing progress
- ✅ Require detailed testing reports
- ✅ Working on comprehensive QA testing

### Use Both Together When:
- ✅ Starting with initial setup, then tracking progress
- ✅ Need full testing workflow from start to finish
- ✅ Want both pre-configuration and progress tracking
- ✅ Running structured testing sessions

## Workflow Comparison

### TestSettingsWidget Workflow

```
┌─────────────────────┐
│  Launch Game        │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ TestSettingsWidget  │
│ Appears             │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Select:             │
│ - Ship Type         │
│ - Difficulty        │
│ - Debug Mode        │
│ - Auto-spawn        │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Click "Continue"    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Main Menu or        │
│ Start Game          │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Gameplay begins     │
│ with configured     │
│ settings            │
└─────────────────────┘
```

### TestingDashboardWidget Workflow

```
┌─────────────────────┐
│ Game Running        │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Press F12           │
│ Toggle Dashboard    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────────────┐
│ TestingDashboardWidget      │
│ Overlays on screen          │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│ See checklist:              │
│ [ ] Ship Movement           │
│ [ ] Weapon Firing           │
│ [ ] Targeting               │
│ [ ] Autopilot               │
│ [✓] Docking (Tested)        │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│ Test a feature              │
│ (e.g., ship movement)       │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│ Press F12                   │
│ Open Dashboard              │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│ Check off:                  │
│ [✓] Ship Movement           │
│ Add notes if needed         │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│ Press F12                   │
│ Close Dashboard             │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│ Continue testing            │
│ (Repeat as needed)          │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│ Export Results              │
│ Generate Report             │
└─────────────────────────────┘
```

## Combined Workflow (Recommended)

Use both widgets for comprehensive testing:

```
┌──────────────────────────────────┐
│ 1. STARTUP PHASE                 │
│ TestSettingsWidget               │
└──────────┬───────────────────────┘
           │
           ▼
Configure:
  • Ship: "Gunship"
  • Difficulty: "Hard"
  • Debug: "Show Collision"
  • Auto-spawn: Yes
           │
           ▼
┌──────────────────────────────────┐
│ 2. GAMEPLAY PHASE                │
│ TestingDashboardWidget           │
└──────────┬───────────────────────┘
           │
           ▼
Test & Check Off:
  [✓] Ship Movement
  [✓] Weapon Firing
  [ ] Targeting
  [ ] Autopilot
           │
           ▼
On-the-fly actions:
  • Spawn additional ships
  • Change debug modes
  • Enable/disable features
           │
           ▼
Document progress:
  • Add notes to features
  • Track completion %
  • Export results
           │
           ▼
┌──────────────────────────────────┐
│ 3. COMPLETION                    │
│ Results exported                 │
│ Progress saved                   │
└──────────────────────────────────┘
```

## Feature Comparison

### Configuration Options

| Option | TestSettingsWidget | TestingDashboardWidget |
|--------|-------------------|------------------------|
| Ship Selection | ✅ Dropdown at start | ✅ Spawn anytime during gameplay |
| Difficulty | ✅ Set at start | ✅ Change during gameplay |
| Debug Modes | ✅ Enable at start | ✅ Toggle during gameplay |
| Custom Commands | ❌ No | ✅ Execute console commands |

### Progress Tracking

| Feature | TestSettingsWidget | TestingDashboardWidget |
|---------|-------------------|------------------------|
| Feature Checklist | ❌ No | ✅ Yes |
| Progress % | ❌ No | ✅ Yes |
| Testing Notes | ❌ No | ✅ Yes per feature |
| Export Results | ❌ No | ✅ Yes |
| Save/Load | ❌ One-time | ✅ Persistent |

### UI/UX

| Aspect | TestSettingsWidget | TestingDashboardWidget |
|--------|-------------------|------------------------|
| When Visible | Before game starts | Toggle during gameplay |
| Screen Coverage | Full screen | Overlay (partial) |
| Pause Game | N/A (pre-game) | Optional |
| Hotkey Support | ❌ No | ✅ F12 (configurable) |
| Mouse Cursor | Always on | Toggle with dashboard |

## Code Comparison

### TestSettingsWidget - Simple Setup

```cpp
// Usage: One-time configuration
UTestSettingsWidget* Settings = CreateWidget<UTestSettingsWidget>(...);
Settings->AddToViewport();

// Later: Read settings
FString ShipType = Settings->GetSelectedShipType();
FString Difficulty = Settings->GetSelectedDifficulty();
bool bAutoSpawn = Settings->GetAutoSpawnEnabled();
```

### TestingDashboardWidget - Continuous Testing

```cpp
// Usage: Create once, toggle multiple times
UTestingDashboardWidget* Dashboard = CreateWidget<UTestingDashboardWidget>(...);
Dashboard->AddToViewport();

// Toggle visibility
Dashboard->ToggleDashboard(true);   // Show
Dashboard->ToggleDashboard(false);  // Hide

// Track progress
Dashboard->SetFeatureTested(0, true);
Dashboard->AddFeatureNotes(0, "All movement directions work");

// Get progress
float Progress = Dashboard->GetTestingProgress(); // 63.2%

// Export results
Dashboard->ExportTestResults("TestSession_2024_11_22.txt");
```

## Integration Patterns

### Pattern 1: Sequential (TestSettings → Dashboard)

```
1. Show TestSettingsWidget on game start
2. Configure initial test parameters
3. Transition to game
4. TestingDashboardWidget available during gameplay
5. Track feature testing progress
```

**Best For:**
- Structured testing sessions
- QA team workflows
- Comprehensive feature coverage

### Pattern 2: Dashboard Only

```
1. Skip TestSettingsWidget
2. Start game normally
3. Use TestingDashboardWidget for all testing
4. Configure everything on-the-fly
```

**Best For:**
- Quick ad-hoc testing
- Debugging specific features
- Developer testing during development

### Pattern 3: Settings Only

```
1. Use TestSettingsWidget for configuration
2. Skip TestingDashboardWidget
3. Test manually without progress tracking
```

**Best For:**
- Simple testing scenarios
- Performance testing
- One-off test runs

## Blueprint Example: Using Both

```
┌────────────────────────────────────┐
│ Level Blueprint                    │
└────────────────────────────────────┘

Event BeginPlay
  ↓
Branch: Is First Run?
  ├─ Yes → Create TestSettingsWidget
  │   ↓
  │   Add to Viewport
  │   ↓
  │   Wait for Continue Button
  │   ↓
  │   Remove TestSettingsWidget
  │   ↓
  │   Store configured settings
  │   ↓
  │   Start gameplay
  │
  └─ No → Skip configuration
      ↓
      Start gameplay directly

During Gameplay:
  ↓
Create TestingDashboardWidget
  ↓
Add to Viewport (Hidden)
  ↓
Bind F12 Key → Toggle Dashboard

When F12 Pressed:
  ↓
Toggle Dashboard Visibility
  ↓
Update feature list display
  ↓
Show current progress
```

## Recommended Setup

### For Development Testing

```
1. Add TestingDashboardWidget to GameMode
2. Bind F12 to toggle visibility
3. Skip TestSettingsWidget (use defaults)
4. Test features and check them off
```

### For QA Testing

```
1. Show TestSettingsWidget on level load
2. Configure test scenario
3. Add TestingDashboardWidget for progress tracking
4. Export results at end of session
```

### For Playtesting

```
1. Hide both widgets
2. Only enable if tester is also QA
3. Or use TestSettingsWidget for difficulty selection only
```

## File Locations

### TestSettingsWidget
```
Source/Adastrea/UI/TestSettingsWidget.h
Source/Adastrea/UI/TestSettingsWidget.cpp
Source/Adastrea/UI/TestSettingsWidget_Usage.md
Assets/TestSettingsWidget_Blueprint_Example.md
Content/UI/Menus/TestSettings_README.md
```

### TestingDashboardWidget
```
Source/Adastrea/UI/TestingDashboardWidget.h
Source/Adastrea/UI/TestingDashboardWidget.cpp
Source/Adastrea/UI/TestingDashboardWidget_Usage.md
```

## Summary

| Your Need | Use This Widget |
|-----------|----------------|
| "I want to configure test settings at startup" | **TestSettingsWidget** |
| "I want to track my testing progress" | **TestingDashboardWidget** |
| "I want to check off features as I test them" | **TestingDashboardWidget** |
| "I want to spawn ships during gameplay" | **TestingDashboardWidget** |
| "I want one-time difficulty selection" | **TestSettingsWidget** |
| "I want to generate testing reports" | **TestingDashboardWidget** |
| "I want comprehensive testing workflow" | **Both Widgets** |

## Getting Started

### Minimal Setup (5 minutes)
1. Create `WBP_TestingDashboard` widget
2. Add basic checklist UI
3. Bind F12 key
4. Start testing!

### Full Setup (30 minutes)
1. Create `WBP_TestSettings` widget
2. Create `WBP_TestingDashboard` widget
3. Configure both in level blueprint
4. Set up input bindings
5. Customize feature list
6. Test the full workflow

### Production Setup
1. Disable both widgets for shipping builds
2. Enable only for development/QA builds
3. Use preprocessor directives: `#if !UE_BUILD_SHIPPING`

---

**Choose the widget(s) that fit your testing workflow best!**
