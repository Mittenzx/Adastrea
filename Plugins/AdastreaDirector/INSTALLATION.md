# Adastrea Director Plugin - Installation Guide

This guide covers the installation and setup of the Adastrea Director plugin for Unreal Engine.

## Prerequisites

### Required
- **Unreal Engine 5.0 or higher**
  - Recommended: UE 5.6+ (latest)
  - Also compatible: UE 5.3, 5.4, 5.5 (LTS), 5.1, 5.2
  
- **Platform Support**
  - Windows 10/11 (x64)
  - macOS 10.15+ (Intel or Apple Silicon)
  - Linux (Ubuntu 20.04+ or equivalent)

- **Development Tools**
  - Visual Studio 2019/2022 (Windows)
  - Xcode 13+ (macOS)
  - GCC 9+ or Clang 10+ (Linux)

### Optional
- **Python 3.9+** (for backend AI features - will be integrated in Week 2+)
- **Git** (for version control and updates)

## Installation Methods

### Method 1: Manual Installation (Recommended for Development)

#### Step 1: Get the Plugin Files

**Option A: Clone from Repository**
```bash
git clone https://github.com/Mittenzx/Adastrea-Director.git
```

**Option B: Download ZIP**
1. Go to https://github.com/Mittenzx/Adastrea-Director
2. Click "Code" → "Download ZIP"
3. Extract the archive

#### Step 2: Copy Plugin to Your Project

1. Navigate to your Unreal Engine project directory
2. Create a `Plugins` folder if it doesn't exist:
   ```
   YourProject/
   ├── Content/
   ├── Config/
   ├── Plugins/          ← Create this if missing
   └── YourProject.uproject
   ```

3. Copy the plugin folder:
   ```bash
   # From the downloaded repository
   cp -r Adastrea-Director/Plugins/AdastreaDirector YourProject/Plugins/
   ```

   Your structure should now look like:
   ```
   YourProject/
   └── Plugins/
       └── AdastreaDirector/
           ├── AdastreaDirector.uplugin
           ├── Source/
           ├── Resources/
           └── Content/
   ```

#### Step 3: Regenerate Project Files

**Windows:**
1. Right-click on `YourProject.uproject`
2. Select "Generate Visual Studio project files"
3. Wait for generation to complete

**macOS:**
1. Right-click on `YourProject.uproject`
2. Select "Generate Xcode project files"
3. Wait for generation to complete

**Linux:**
```bash
/Path/To/UnrealEngine/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh -project="/Path/To/YourProject.uproject"
```

**Alternative (All Platforms):**
```bash
# Using UnrealBuildTool directly
/Path/To/UnrealEngine/Engine/Build/BatchFiles/Build.sh -projectfiles -project="/Path/To/YourProject.uproject"
```

#### Step 4: Build the Project

**Windows (Visual Studio):**
1. Open `YourProject.sln`
2. Set configuration to "Development Editor"
3. Set platform to "Win64"
4. Build → Build Solution (or press Ctrl+Shift+B)

**macOS (Xcode):**
1. Open `YourProject.xcworkspace`
2. Set scheme to "YourProjectEditor"
3. Product → Build (or press Cmd+B)

**Linux (Command Line):**
```bash
/Path/To/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh YourProjectEditor Linux Development "/Path/To/YourProject.uproject"
```

#### Step 5: Launch and Verify

1. Launch Unreal Engine Editor
2. Open your project
3. The plugin should load automatically
4. Verify installation:
   - Go to **Edit → Plugins**
   - Search for "Adastrea Director"
   - Should appear under "Developer Tools" category
   - Status should show "Enabled"

5. Check the Output Log (Window → Developer Tools → Output Log):
   - Look for: `LogTemp: AdastreaDirector Runtime Module: StartupModule`
   - Look for: `LogTemp: AdastreaDirector Editor Module: StartupModule`

### Method 2: Engine Installation (For All Projects)

This installs the plugin for the entire Unreal Engine installation, making it available to all projects.

#### Step 1: Locate Engine Plugins Directory

**Windows:**
```
C:\Program Files\Epic Games\UE_5.X\Engine\Plugins\Marketplace\
```

**macOS:**
```
/Users/Shared/Epic Games/UE_5.X/Engine/Plugins/Marketplace/
```

**Linux:**
```
~/UnrealEngine/Engine/Plugins/Marketplace/
```

#### Step 2: Copy Plugin

```bash
cp -r Adastrea-Director/Plugins/AdastreaDirector /Path/To/Engine/Plugins/Marketplace/
```

#### Step 3: Enable in Project

1. Open your project in Unreal Engine
2. Go to Edit → Plugins
3. Search for "Adastrea Director"
4. Click checkbox to enable
5. Restart the editor when prompted

### Method 3: Packaged Installation (Future)

Once the plugin is complete and submitted to the Unreal Engine Fab marketplace, it can be installed through:

1. **Unreal Engine Fab** (Epic Games Launcher)
2. Browse to "Plugins" section
3. Search for "Adastrea Director"
4. Click "Install to Engine"
5. Enable in your project through Edit → Plugins

## Verification

### Quick Test

After installation, verify the plugin is working:

1. **Check Plugin Status:**
   - Edit → Plugins
   - Search "Adastrea Director"
   - Should be enabled and show version 1.0.0

2. **Check Console Log:**
   - Window → Developer Tools → Output Log
   - Filter for "AdastreaDirector"
   - Should see startup messages

3. **Check Module Load:**
   - Help → About Unreal Editor
   - "Loaded Modules" tab
   - Should list "AdastreaDirector" and "AdastreaDirectorEditor"

### Expected Output Log

```
LogPluginManager: Mounting plugin AdastreaDirector
LogTemp: AdastreaDirector Runtime Module: StartupModule
LogTemp: AdastreaDirector Editor Module: StartupModule
```

## Troubleshooting

### Plugin Doesn't Appear in Plugins List

**Problem:** Plugin not visible in Edit → Plugins

**Solutions:**
1. Verify folder structure is correct (see Step 2 of Method 1)
2. Check `.uplugin` file is present and valid JSON
3. Ensure plugin is in project's `Plugins/` directory or engine's `Plugins/Marketplace/`
4. Regenerate project files
5. Restart Unreal Engine

### Build Errors

**Problem:** Compilation fails with module errors

**Solutions:**

1. **Missing Dependencies:**
   ```
   Error: Missing module: Slate
   ```
   - Verify all modules in `.Build.cs` are available for your UE version
   - Check spelling of module names
   - Ensure Editor-only modules are in `AdastreaDirectorEditor.Build.cs`, not runtime module

2. **Wrong Engine Version:**
   ```
   Error: Plugin 'AdastreaDirector' requires engine version >= 5.0.0
   ```
   - Check your UE version meets minimum requirements
   - Update `EngineVersion` in `.uplugin` if needed

3. **Path Issues:**
   ```
   Error: Cannot find AdastreaDirectorModule.h
   ```
   - Regenerate project files
   - Clean and rebuild
   - Check file paths are correct

**Clean Build Steps:**
1. Close Unreal Engine
2. Delete `Binaries/` and `Intermediate/` folders from plugin directory
3. Delete `Binaries/` and `Intermediate/` folders from project directory
4. Regenerate project files
5. Rebuild solution

### Runtime Errors

**Problem:** Plugin loads but crashes or shows errors

**Solutions:**

1. **Check Log Files:**
   - `Saved/Logs/` directory in your project
   - Look for crash dumps and call stacks

2. **Verify Module Loading:**
   ```
   LogModuleManager: Warning: Module 'AdastreaDirector' failed to load
   ```
   - Check module initialization code
   - Verify IMPLEMENT_MODULE macro is correct
   - Ensure module names match in `.uplugin` and source files

3. **Debug Mode:**
   - Build in "Debug Editor" configuration
   - Attach debugger
   - Set breakpoints in StartupModule()
   - Step through initialization

### Platform-Specific Issues

#### Windows

**Visual Studio Version:**
- Use Visual Studio 2019 or 2022
- Ensure "Game Development with C++" workload is installed

**Long Paths:**
- Enable long path support in Windows if paths exceed 260 characters
- Use shorter project/plugin folder names

#### macOS

**Xcode Command Line Tools:**
```bash
xcode-select --install
```

**Code Signing:**
- Temporarily disable code signing for development:
  - Open project in Xcode
  - Select target → Signing & Capabilities
  - Uncheck "Automatically manage signing"

#### Linux

**Dependencies:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential clang mono-complete

# Fedora/RHEL
sudo dnf install clang mono-complete
```

**Permissions:**
```bash
# Ensure files have correct permissions
chmod -R 755 Plugins/AdastreaDirector/
```

## Updating the Plugin

### From Git Repository

1. Navigate to the repository:
   ```bash
   cd path/to/Adastrea-Director
   ```

2. Pull latest changes:
   ```bash
   git pull origin main
   ```

3. Copy updated plugin to your project:
   ```bash
   cp -r Plugins/AdastreaDirector /path/to/YourProject/Plugins/
   ```

4. Regenerate project files and rebuild

### From Marketplace (Future)

1. Open Epic Games Launcher
2. Go to Unreal Engine → Library → Plugins
3. Find "Adastrea Director"
4. Click "Update" if available
5. Restart Unreal Engine

## Uninstalling

### Disable Plugin

1. Edit → Plugins
2. Search "Adastrea Director"
3. Uncheck "Enabled"
4. Restart editor when prompted

### Remove Plugin

1. Close Unreal Engine Editor
2. Navigate to project's `Plugins` folder
3. Delete `AdastreaDirector` folder
4. Regenerate project files
5. Rebuild project

**Note:** If installed to engine directory, delete from engine's `Plugins/Marketplace/` instead.

## Next Steps

After successful installation:

1. **Week 2-4 Updates:**
   - Watch for Python bridge integration
   - Prepare Python backend setup
   - Configure API keys

2. **Documentation:**
   - Read [README.md](README.md) for feature overview
   - Review [PLUGIN_DEVELOPMENT_FEASIBILITY.md](../../PLUGIN_DEVELOPMENT_FEASIBILITY.md) for architecture details

3. **Community:**
   - Report issues: [GitHub Issues](https://github.com/Mittenzx/Adastrea-Director/issues)
   - Check roadmap: [ROADMAP.md](../../ROADMAP.md)

## Support

For installation help:
- Check [Troubleshooting](#troubleshooting) section above
- Review [GitHub Issues](https://github.com/Mittenzx/Adastrea-Director/issues)
- See main [README.md](../../README.md) for contact information

---

**Last Updated:** November 14, 2025  
**Plugin Version:** 1.0.0 (Phase 1, Week 1)  
**Status:** Foundation Complete - Python Bridge Coming Soon
