# Configuration Directory

This directory contains Unreal Engine configuration files that control various aspects of the game and editor.

## Typical Configuration Files

When this project has a `.uproject` file, the following configuration files will be generated:

### Engine Configuration
- **DefaultEngine.ini** - Core engine settings (physics, rendering, networking)
- **DefaultEditor.ini** - Editor preferences and settings
- **DefaultGame.ini** - Game-specific settings (game mode, player start, etc.)
- **DefaultInput.ini** - Input bindings and controls

### Platform-Specific Configuration
- **Windows/WindowsEngine.ini** - Windows-specific engine settings
- **Mac/MacEngine.ini** - macOS-specific settings
- **Linux/LinuxEngine.ini** - Linux-specific settings

### Custom Configuration
You can add custom configuration categories as needed.

## Configuration Best Practices

### 1. Version Control
- **DO commit**: DefaultEngine.ini, DefaultGame.ini, DefaultInput.ini
- **DO commit**: Platform-specific .ini files
- **DON'T commit**: User-specific settings (*.user.ini)

### 2. Organization
```ini
[CategoryName]
SettingName=Value
AnotherSetting=Value

[AnotherCategory]
Setting=Value
```

### 3. Comments
```ini
; This is a comment explaining the setting below
[/Script/Engine.Engine]
bSmoothFrameRate=True
```

### 4. Common Settings

#### Performance Settings (DefaultEngine.ini)
```ini
[/Script/Engine.RendererSettings]
r.DefaultFeature.AutoExposure=True
r.DefaultFeature.MotionBlur=True
r.DefaultFeature.AntiAliasing=2

[/Script/Engine.Engine]
bSmoothFrameRate=True
SmoothedFrameRateRange=(LowerBound=(Type=Inclusive,Value=30),UpperBound=(Type=Exclusive,Value=120))
```

#### Input Settings (DefaultInput.ini)
```ini
[/Script/Engine.InputSettings]
DefaultViewportMouseCaptureMode=CapturePermanently_IncludingInitialMouseDown
bDefaultViewportMouseLock=True
```

#### Game Settings (DefaultGame.ini)
```ini
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Game/Maps/MainMenu
EditorStartupMap=/Game/Maps/TestLevel
GlobalDefaultGameMode=/Script/Adastrea.AdastreaGameMode
```

## Project-Specific Notes

### Current Status
This project currently has an empty Config directory. Configuration files will be created when:
1. The .uproject file is created/updated
2. Project settings are modified in the Unreal Editor
3. Custom configurations are added by developers

### Planned Configurations
Once the project is fully set up, we expect to configure:
- **Physics**: Custom collision channels for ships, stations, and projectiles
- **Input**: Control schemes for ship flight, station management, and UI
- **Rendering**: Optimized settings for space environments
- **Networking**: If multiplayer features are added
- **Game Mode**: Custom game modes for different play scenarios

## Modifying Settings

### Via Unreal Editor
1. Open Unreal Engine Editor
2. Go to **Edit > Project Settings**
3. Navigate to the category you want to modify
4. Make changes
5. Settings auto-save to appropriate .ini files

### Via Text Editor
1. Open the .ini file in any text editor
2. Locate the section you need
3. Add or modify settings
4. Save the file
5. Restart the editor to apply changes

## Debugging Configuration Issues

### Common Problems

**Problem**: Settings not applying
- **Solution**: Check if settings are overridden in higher-priority .ini files
- Check for typos in section names or setting names

**Problem**: Settings reset after restart
- **Solution**: Ensure you're editing DefaultXXX.ini, not UserXXX.ini
- Verify the .ini file is not read-only

**Problem**: Can't find a setting
- **Solution**: Use the editor's search function in Project Settings
- Check Unreal Engine documentation for the correct .ini section

## Resources

- [Unreal Engine Configuration Files](https://docs.unrealengine.com/5.0/en-US/configuration-files-in-unreal-engine/)
- [Project Settings Reference](https://docs.unrealengine.com/5.0/en-US/project-settings-in-unreal-engine/)
- [Input Settings](https://docs.unrealengine.com/5.0/en-US/input-in-unreal-engine/)

## Contributing

When adding configuration changes:
1. Document the reason for the change
2. Test the change thoroughly
3. Update this README if adding new custom sections
4. Commit configuration changes with clear commit messages
5. Note any platform-specific implications

---

**Note**: This directory will be populated automatically when the project is properly configured in Unreal Engine.
