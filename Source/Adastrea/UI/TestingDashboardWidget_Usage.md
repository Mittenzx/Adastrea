# Testing Dashboard Widget Usage Guide

## Overview

The `TestingDashboardWidget` is an **in-game testing overlay** that allows you to check off game features as you test them during gameplay. Unlike the pre-screen `TestSettingsWidget`, this dashboard stays accessible throughout your testing session and lets you configure tests on-the-fly.

## Key Features

✅ **Interactive Checklist** - Check off features as you test them
✅ **Progress Tracking** - See percentage of features tested
✅ **On-the-Fly Configuration** - Spawn ships, change difficulty, enable debug modes during gameplay
✅ **Notes & Documentation** - Add testing notes to each feature
✅ **Category Filtering** - Filter by system (Combat, Navigation, UI, etc.)
✅ **Auto-Save Progress** - Automatically saves your testing progress
✅ **Export Results** - Generate testing reports
✅ **Toggle Visibility** - Press a hotkey (e.g., F12) to show/hide

## Quick Start

### 1. Create the Widget Blueprint

1. In Unreal Editor, navigate to `Content/UI/Menus/`
2. Right-click → User Interface → Widget Blueprint
3. Name it `WBP_TestingDashboard`
4. Open the Widget Blueprint
5. Go to File → Reparent Blueprint
6. Select `TestingDashboardWidget` as the parent class

### 2. Design the Dashboard Layout

Create a comprehensive testing interface:

```
Canvas Panel (Root)
├── Border (Semi-transparent background)
│   └── Vertical Box (Main Container)
│       ├── Horizontal Box (Header)
│       │   ├── Text - "Testing Dashboard"
│       │   ├── Text - Progress: "X% Complete"
│       │   └── Button - Close (X)
│       │
│       ├── Horizontal Box (Filter Controls)
│       │   ├── ComboBox - Category Filter
│       │   └── CheckBox - "Show Only Uncompleted"
│       │
│       ├── Scroll Box (Feature Checklist)
│       │   └── [Dynamically populated with checkboxes]
│       │
│       ├── Horizontal Box (Quick Actions)
│       │   ├── ComboBox - Ship Type
│       │   ├── Button - "Spawn Ship"
│       │   ├── ComboBox - Debug Mode
│       │   └── Button - "Enable"
│       │
│       └── Horizontal Box (Bottom Buttons)
│           ├── Button - "Export Results"
│           ├── Button - "Reset Progress"
│           └── Button - "Close Dashboard"
```

### 3. Populate the Feature List

In the Event Graph, create a function to display the checklist:

```
Function: RefreshFeatureList
  ↓
Get Testable Features
  ↓
Clear Scroll Box
  ↓
For Each Loop (Features)
    ↓
    Create Widget: WBP_FeatureCheckboxItem
    ↓
    Set Feature Data (Name, Description, Tested Status)
    ↓
    Bind OnCheckChanged → SetFeatureTested
    ↓
    Add to Scroll Box
```

### 4. Add Toggle Input Binding

Create an Input Action to toggle the dashboard:

**In Project Settings → Input:**
1. Create new Action Mapping: `ToggleTestDashboard`
2. Bind to key: `F12` (or your preference)

**In Player Controller Blueprint:**
```
Event ToggleTestDashboard (Input Action)
  ↓
Get All Widgets of Class (TestingDashboardWidget)
  ↓
Get First Element
  ↓
Is Valid?
    ├─ Yes → Get Is Dashboard Visible
    │   ↓
    │   Toggle Dashboard (NOT bVisible)
    │
    └─ No → Create Widget (WBP_TestingDashboard)
        ↓
        Add to Viewport
        ↓
        Toggle Dashboard (true)
```

### 5. Display on Level Start

Optionally create the dashboard when the level loads (hidden by default):

**In Level Blueprint or GameMode:**
```
Event BeginPlay
  ↓
Create Widget (WBP_TestingDashboard)
  ↓
Add to Viewport (Z-Order: 10)
  ↓
Toggle Dashboard (false) // Start hidden
```

## Core Functionality

### Checking Off Features

When you test a feature, simply check it off:

1. Open dashboard (F12)
2. Find the feature in the list
3. Click checkbox to mark as tested
4. Optionally add notes
5. Progress auto-saves

### Adding Test Notes

Each feature can have notes attached:

```
Event OnFeatureNotesChanged (Custom)
  ↓
Get Feature Index
  ↓
Get Notes Text
  ↓
Add Feature Notes (Index, Notes)
```

### Progress Tracking

The dashboard automatically calculates your progress:

```
Progress = (Tested Features / Total Features) * 100

Example: 12 / 19 features = 63% complete
```

### Category Filtering

Filter features by system type:

**Categories:**
- Core (movement, collision)
- Combat (weapons, targeting, shields)
- Navigation (autopilot, sector map)
- Stations (docking, trading, services)
- Factions (reputation, diplomacy)
- AI (NPC behavior, combat AI)
- UI (HUD, menus, inventory)
- Performance (FPS, load times)

### Quick Actions

#### Spawn Ship for Testing

```
Event SpawnShipButton.OnClicked
  ↓
Get Selected Ship Type (from ComboBox)
  ↓
Spawn Test Ship (ShipType, Distance: 1000.0)
```

Override `SpawnTestShip` in Blueprint:
```
Event SpawnTestShip (Override)
  ↓
Get Player Pawn Location
  ↓
Calculate Spawn Location (Forward + Distance)
  ↓
Get GameMode
  ↓
Call SpawnShipOfType(ShipType, Location)
```

#### Enable Debug Mode

```
Event DebugModeButton.OnClicked
  ↓
Get Selected Debug Mode
  ↓
Enable Debug Mode (DebugMode)
```

Available debug modes:
- None (disable all)
- Show FPS (`stat fps`)
- Show Collision (`show collision`)
- Show Navigation (`show navigation`)
- Show All Stats (`stat unit`, `stat fps`)
- God Mode (`god`)
- Infinite Resources (custom implementation)
- Show AI Debug (`showdebug ai`)
- Show Physics (`show physics`)
- Show Networking (`stat net`)

### Export Testing Results

Generate a report of your testing session:

```
Event ExportButton.OnClicked
  ↓
Export Test Results ("TestResults/Session_2024_11_22.txt")
```

**Output format:**
```
=== Adastrea Testing Results ===

Total Features: 19
Tested: 12
Progress: 63.2%

=== Core ===
[✓] Ship Movement
    Notes: All movement modes work. Rotation feels smooth.
[✓] Ship Collision
    Notes: Collisions detected properly.

=== Combat ===
[✓] Weapon Firing
    Notes: Energy weapons work. Missiles need testing.
[ ] Targeting System
[ ] Shields & Damage

...
```

## Blueprint Customization

### Override Default Features

Customize the feature list for your game:

```
Event LoadDefaultFeatures (Override)
  ↓
Clear Testable Features
  ↓
Create Testable Feature (Struct)
    Feature Name: "Custom System"
    Description: "Test my custom feature"
    Category: "Custom"
    Is Tested: false
  ↓
Add to Testable Features Array
  ↓
Repeat for all features
```

### Custom Ship Spawning Logic

Implement actual ship spawning:

```
Event SpawnTestShip (Override)
  ↓
Parent: SpawnTestShip (ShipType, SpawnDistance)
  ↓
Branch: ShipType
  ├─ "Fighter" → Spawn BP_Fighter
  ├─ "Scout" → Spawn BP_Scout
  ├─ "Hauler" → Spawn BP_Hauler
  └─ Default → Log Error
```

### Integration with GameMode

Access dashboard from GameMode for automated testing:

```
GameMode: Function AutoTestFeature
  ↓
Get All Widgets of Class (TestingDashboardWidget)
  ↓
Get First Element
  ↓
Set Feature Tested (FeatureIndex, true)
  ↓
Add Feature Notes (FeatureIndex, "Auto-tested OK")
```

## Advanced Features

### Pause Game When Open

Set in Class Defaults:
- **bPauseGameWhenOpen** = `true`

This will pause gameplay when the dashboard is visible, useful for examining detailed stats.

### Show Only Uncompleted

Filter to show only features you haven't tested yet:

```
Event ShowUncompletedCheckbox.OnCheckChanged
  ↓
Set Show Only Uncompleted (Is Checked)
  ↓
Apply Filters
  ↓
Refresh Feature List
```

### Category Quick Filter

Add buttons for quick category switching:

```
Horizontal Box (Quick Categories)
├── Button - "Core"
├── Button - "Combat"
├── Button - "Navigation"
├── Button - "All"

Event CategoryButton.OnClicked
  ↓
Set Current Filter Category (Button Text)
  ↓
Apply Filters
  ↓
Refresh Feature List
```

### Auto-Test Hooks

Create automated test triggers:

```
Function: AutoCheckFeature(FeatureName)
  ↓
For Each Loop (Testable Features)
    ↓
    Branch: Feature Name == FeatureName?
        ├─ Yes → Set Feature Tested (Index, true)
        │   ↓
        │   Break Loop
        │
        └─ No → Continue
```

**Usage in gameplay code:**
```
Event OnWeaponFired
  ↓
Get Testing Dashboard Widget
  ↓
Auto Check Feature ("Weapon Firing")
```

## Testing Workflow Example

### Typical Testing Session

1. **Start Game**
   - Dashboard is hidden
   - Playing normally

2. **Open Dashboard (F12)**
   - Dashboard slides in
   - Shows uncompleted features

3. **Test Movement**
   - Move ship around
   - Check collision
   - Mark "Ship Movement" as tested ✓
   - Add notes: "Movement smooth, all axes working"

4. **Spawn Test Ship**
   - Select "Enemy Fighter" from dropdown
   - Click "Spawn Ship"
   - Enemy appears 1000m away

5. **Test Combat**
   - Engage enemy
   - Test weapons, targeting, shields
   - Mark features as tested ✓✓✓

6. **Enable Debug Mode**
   - Select "Show FPS"
   - Check performance
   - Mark "Frame Rate" as tested ✓

7. **Review Progress**
   - Check progress bar: 45% complete
   - Export results for documentation

8. **Close Dashboard (F12)**
   - Continue gameplay
   - Progress auto-saved

## Properties Reference

### Configuration Properties

```cpp
// Editable in Blueprint Class Defaults

TArray<FTestableFeature> TestableFeatures
// The list of all testable features

TArray<FString> AvailableShipTypes
// Ships available for spawning

TArray<FString> AvailableDebugModes
// Debug modes that can be enabled

bool bPauseGameWhenOpen
// Whether to pause game when dashboard is visible

bool bShowOnlyUncompleted
// Filter to show only uncompleted features

FString TestResultsPath
// Where to save exported results
```

### State Properties (Read Only)

```cpp
bool bIsDashboardVisible
// Whether dashboard is currently visible

FString CurrentFilterCategory
// Currently active category filter
```

### Feature Struct

```cpp
struct FTestableFeature
{
    FText FeatureName;      // Display name
    FText Description;      // What to test
    bool bIsTested;         // Tested status
    FString Category;       // Feature category
    FString TestNotes;      // Testing notes
}
```

## Functions Reference

### Display & Control

```cpp
// Toggle dashboard visibility
ToggleDashboard(bool bVisible)

// Get testing progress percentage
GetTestingProgress() → float (0-100)

// Reset all testing progress
ResetAllProgress()
```

### Feature Management

```cpp
// Mark feature as tested
SetFeatureTested(int32 FeatureIndex, bool bTested)

// Add notes to feature
AddFeatureNotes(int32 FeatureIndex, FString Notes)

// Get all features
GetTestableFeatures() → TArray<FTestableFeature>

// Get features by category
GetFeaturesByCategory(FString Category) → TArray<FTestableFeature>
```

### Quick Actions

```cpp
// Spawn ship for testing
SpawnTestShip(FString ShipType, float SpawnDistance)

// Enable debug visualization
EnableDebugMode(FString DebugMode)

// Change difficulty on-the-fly
SetDifficulty(FString DifficultyLevel)

// Execute custom console command
ExecuteTestCommand(FString Command)
```

### Data Management

```cpp
// Export results to file
ExportTestResults(FString FilePath)

// Save testing progress
SaveProgress()

// Load testing progress
LoadProgress()
```

## Integration Examples

### With Automated Testing

```cpp
// In your automation test class
void FMyAutomationTest::RunTest()
{
    UTestingDashboardWidget* Dashboard = GetTestingDashboard();
    
    // Test ship movement
    TestShipMovement();
    Dashboard->SetFeatureTested(0, true);
    
    // Test combat
    TestCombatSystem();
    Dashboard->SetFeatureTested(2, true);
    Dashboard->AddFeatureNotes(2, "Automated test passed");
    
    // Export results
    Dashboard->ExportTestResults("AutoTest_Results.txt");
}
```

### With CI/CD Pipeline

```bash
# In your test script
python run_unreal_tests.py --export-dashboard-results
```

### With Bug Tracking

```
When reporting a bug:
1. Open Testing Dashboard
2. Find related feature
3. Add notes with bug details
4. Export results
5. Attach to bug report
```

## Best Practices

### DO:
✓ Check off features as you test them
✓ Add detailed notes for issues found
✓ Export results at end of session
✓ Use category filters for focused testing
✓ Spawn ships to test different scenarios
✓ Enable debug modes when needed

### DON'T:
✗ Forget to toggle dashboard closed during gameplay
✗ Mark features as tested without actually testing
✗ Ignore the notes field - document issues!
✗ Reset progress accidentally
✗ Spam ship spawning (can cause performance issues)

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| F12 | Toggle Dashboard Visibility |
| ESC | Close Dashboard (if configured) |

## Troubleshooting

### Dashboard won't open
- Check input binding is configured
- Verify widget is added to viewport
- Check console for errors

### Features not saving
- Check file permissions in Saved folder
- Verify SaveProgress is being called
- Check logs for save errors

### Ship spawning doesn't work
- Override SpawnTestShip in Blueprint
- Implement actual spawn logic
- Check GameMode spawn functions

### Progress shows 0%
- Verify testable features array is populated
- Check LoadDefaultFeatures was called
- Confirm feature list isn't empty

## Version History

- **v1.0** (Current): Initial implementation
  - Interactive checklist system
  - Progress tracking
  - Quick actions (spawn, debug)
  - Export functionality
  - Save/Load progress

## See Also

- [TestSettingsWidget.h](TestSettingsWidget.h) - Pre-game configuration widget
- [MainMenuWidget.h](MainMenuWidget.h) - Main menu widget
- [TESTING_AUTOMATION_GUIDE.md](/TESTING_AUTOMATION_GUIDE.md) - Automated testing
- [BLUEPRINT_CREATION_GUIDE.md](/BLUEPRINT_CREATION_GUIDE.md) - Blueprint setup
