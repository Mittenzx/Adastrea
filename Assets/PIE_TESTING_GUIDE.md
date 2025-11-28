# PIE (Play In Editor) Testing Guide

> **Quick Start**: Press `Alt+P` or click "Play" in the Unreal Editor to start PIE testing with the pre-configured TestLevel map.

## Overview

PIE (Play In Editor) is Unreal Engine's built-in testing mode that allows you to test your game directly within the editor. This guide covers the Adastrea-specific PIE configuration and best practices for efficient testing.

## PIE Configuration

The Adastrea project includes pre-configured PIE settings in `Config/DefaultEditor.ini` and `Config/DefaultEngine.ini`.

### Key Settings

| Setting | Value | Description |
|---------|-------|-------------|
| EditorStartupMap | /Game/Maps/TestLevel | Default map loaded for testing |
| PlayNetMode | PIE_Standalone | Single-player testing mode |
| ClientWindowWidth | 1280 | PIE window width |
| ClientWindowHeight | 720 | PIE window height |
| bAutoRecompileBlueprints | True | Auto-recompile Blueprints on changes |
| bPlayInEditorSoundEnabled | True | Enable audio during PIE |

## Starting PIE

### Method 1: Keyboard Shortcut
- **Alt + P**: Play in viewport
- **Alt + Shift + P**: Play in new window
- **Alt + S**: Simulate mode (no player control)

### Method 2: Toolbar
1. Click the **Play** button in the editor toolbar
2. Use the dropdown arrow for additional options:
   - Play in Viewport
   - Play in New Window
   - Simulate
   - Standalone Game

### Method 3: Custom Launch
```
Editor Menu → Play → Advanced Settings
```

## PIE Testing Modes

### 1. Play in Viewport (Default)
Best for quick testing. The game runs in the current viewport.

**Pros:**
- Fastest startup
- No context switching
- Direct editor access

**Cons:**
- Limited screen space
- May have editor UI overlap

### 2. Play in New Window
Opens a separate window for testing.

**Pros:**
- Full screen experience
- Better for UI testing
- More representative of final game

**Cons:**
- Slightly longer startup
- Requires window management

### 3. Standalone Game
Launches a completely separate process.

**Pros:**
- Most accurate to shipping build
- Best for performance testing
- No editor overhead

**Cons:**
- Longest startup time
- No live editing
- Must restart for changes

### 4. Simulate Mode
Runs game logic without player control.

**Pros:**
- Good for AI testing
- Can edit while running
- Camera control maintained

**Cons:**
- No player interaction
- Limited gameplay testing

## TestLevel Map

The `TestLevel.umap` is the default PIE startup map, configured specifically for testing:

### Features
- Pre-placed spawn points
- Test spaceship spawners
- Debug visualization actors
- Performance monitoring widgets

### Map Configuration
Located at: `Content/Maps/TestLevel.umap`

Set in: `Config/DefaultEngine.ini`
```ini
[/Script/EngineSettings.GameMapsSettings]
EditorStartupMap=/Game/Maps/TestLevel
```

## TestGameMode Integration

During PIE, the `TestGameMode` class provides:

### Pre-Game Configuration Menu
- Spaceship selection
- Input configuration
- Debug mode options

### Runtime Features
- Ship spawning control
- Input mapping application
- Debug visualization toggles

### Usage
1. PIE starts with TestLevel
2. TestGameMode shows configuration menu (if enabled)
3. Configure settings
4. Click "Continue" to start gameplay
5. Test features interactively

## Debug Features During PIE

### Console Commands
Press **`** (tilde/backtick) to open the console:

| Command | Description |
|---------|-------------|
| `stat fps` | Show FPS counter |
| `stat unit` | Show frame time breakdown |
| `stat game` | Show game thread stats |
| `show collision` | Toggle collision visualization |
| `show navigation` | Toggle navigation mesh |
| `viewmode wireframe` | Wireframe rendering |
| `viewmode unlit` | Unlit rendering |

### Adastrea-Specific Commands
```
// Enable verbose logging
log LogAdastrea Verbose
log LogAdastreaShips Verbose
log LogAdastreaInput Verbose
```

> **Note:** The following custom Adastrea console commands are **planned for future releases** and are not yet implemented:
> - `adastrea.spawn_ship` - Spawn ships of various types
> - `adastrea.set_speed` - Set ship speed
> - `adastrea.debug_collision` - Toggle collision debugging
> - `adastrea.debug_navigation` - Toggle navigation debugging
>
> Use the TestingDashboardWidget (F12) for ship spawning and debug mode toggles in the meantime.

### Visual Debugging
- **G**: Toggle game UI
- **F1-F12**: Custom debug views (defined in TestingDashboardWidget)
- **F12**: Toggle Testing Dashboard

## Testing Dashboard Integration

During PIE, the TestingDashboardWidget provides:

### Interactive Checklist
- Track tested features
- Add notes per feature
- Export test results

### Quick Actions
- Spawn test ships
- Enable debug modes
- Change difficulty

### Progress Tracking
- Real-time completion percentage
- Auto-save/load progress
- Export reports

**Toggle**: Press `F12` during gameplay

## Best Practices

### 1. Use Hot Reload
- Make C++ changes
- Press `Ctrl + Alt + F11` to compile
- Changes apply without restarting PIE

### 2. Blueprint Debugging
- Set breakpoints in Blueprints
- Use "Print String" nodes
- Check Output Log for messages

### 3. Performance Testing
```
stat fps
stat unit
stat game
profilegpu
```

### 4. Memory Testing
```
stat memory
memreport -full
```

### 5. Quick Iteration
- Keep PIE running
- Make changes in editor
- Eject (F8) to test changes
- Possess (F8) to continue playing

## Common Testing Scenarios

### Scenario 1: Ship Flight Testing
1. Start PIE (Alt + P)
2. Skip or configure TestSettings menu
3. Test ship controls:
   - WASD for movement
   - Mouse for looking
   - Mouse wheel for throttle
4. Use F12 to check off "Ship Movement" in dashboard

### Scenario 2: Combat Testing
1. Start PIE
2. Open Testing Dashboard (F12)
3. Spawn enemy ship
4. Test weapons and targeting
5. Check off combat features
6. Add notes for any issues

### Scenario 3: Station Docking
1. Start PIE with station in level
2. Approach station
3. Test docking sequence
4. Verify UI feedback
5. Test undocking

### Scenario 4: Performance Profiling
1. Start PIE in Standalone mode
2. Run `stat fps` and `stat unit`
3. Navigate to performance-critical areas
4. Capture frame data with `profilegpu`
5. Analyze in GPU Visualizer

## Troubleshooting

### PIE Won't Start
1. Check Output Log for errors
2. Verify project compiles successfully
3. Check for missing assets in TestLevel

### Poor Performance in PIE
1. Disable editor features (Realtime rendering)
2. Use "Standalone Game" mode
3. Check for infinite loops in Blueprints

### Input Not Working
1. Click inside the viewport to focus
2. Check Input Mode settings
3. Verify Enhanced Input contexts

### No Sound
1. Check `bPlayInEditorSoundEnabled` in settings
2. Verify audio devices
3. Check volume sliders in game

### Crashes on Start
1. Check Output Log for crash info
2. Verify game mode class exists
3. Check for null pointer issues in BeginPlay

## Integration with CI/CD

### Automated PIE Testing
```bash
# Run PIE tests from command line
UnrealEditor-Cmd.exe "Adastrea.uproject" -ExecCmds="Automation RunTests Adastrea" -unattended

# Run with specific map
UnrealEditor-Cmd.exe "Adastrea.uproject" -ExecCmds="Automation RunTests Adastrea.PIE" -Map=/Game/Maps/TestLevel
```

### Smoke Testing
See `SmokeTest.py` for automated map loading and validation.

### Screenshot Testing
See `ScreenshotTester.py` for visual regression testing.

## Related Documentation

- [Testing_System_Overview.md](Testing_System_Overview.md) - Complete testing system documentation
- [TestSettingsFlow.md](TestSettingsFlow.md) - Pre-game configuration flow
- [TestingDashboard_QuickStart.md](TestingDashboard_QuickStart.md) - In-game testing dashboard
- [TESTING_AUTOMATION_GUIDE.md](../TESTING_AUTOMATION_GUIDE.md) - Automated testing

## Version History

- **1.0** (2025-11-28): Initial PIE configuration setup
  - Added PIE settings to DefaultEditor.ini
  - Added world settings to DefaultEngine.ini
  - Created PIE testing documentation

---

**Maintained by**: Adastrea Development Team  
**Last Updated**: 2025-11-28
