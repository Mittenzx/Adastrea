# Visual Studio Project Generation

**Fix project generation issues | Last Updated: August 9, 2026**

---

## 🎯 Quick Fix (Most Common)

```bash
# 1. Close Visual Studio and Unreal Editor
# 2. Delete generated files:
#    - Adastrea.sln
#    - Intermediate/
#    - Binaries/
#    - .vs/
# 3. Right-click Adastrea.uproject → "Generate Visual Studio project files"
# 4. Open Adastrea.sln
# 5. Build → Build Solution (Development Editor, Win64)
```

---

## 🔧 Common Issues & Fixes

### Issue: "Project generation failed"
**Cause**: Missing UE 5.6 installation or wrong version
**Fix**: 
- Verify UE 5.6 installed via Epic Launcher
- Check `.uproject` `"EngineAssociation": "5.6"`
- Run `GenerateProjectFiles.bat` from Engine directory

### Issue: "Module 'Adastrea' not found"
**Cause**: Build.cs not discovered
**Fix**:
- Verify `Source/Adastrea/Adastrea.Build.cs` exists
- Check `.uproject` modules list includes "Adastrea"
- Delete Intermediate/Build and regenerate

### Issue: "Circular dependency detected"
**Cause**: StationEditor module depends on Adastrea, Adastrea references StationEditor
**Fix**: Already resolved - StationEditor removed from Adastrea.Build.cs PublicDependencies
```csharp
// In Adastrea.Build.cs - this was removed:
// PublicDependencyModuleNames.Add("StationEditor");
```

### Issue: "Plugin 'AdastreaDirector' not found"
**Cause**: Plugin not built
**Fix**:
- Build plugin separately: `BuildPlugin.bat AdastreaDirector`
- Or enable in `.uproject` plugins list (already enabled)

### Issue: IntelliSense errors (red squiggles) but builds fine
**Cause**: VS not parsing UE macros correctly
**Fix**:
- Install "Unreal Engine" VS extension
- Right-click solution → "Rescan Solution"
- Delete `.vs/` folder and regenerate

---

## 📋 Full Generation Process

### Method 1: Right-Click (Easiest)
1. Right-click `Adastrea.uproject`
2. Select "Generate Visual Studio project files"
3. Wait for completion
4. Open `Adastrea.sln`

### Method 2: Command Line (CI/CD)
```batch
REM Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" Adastrea.uproject

REM Linux/Mac
~/UE_5.6/Engine/Build/BatchFiles/GenerateProjectFiles.sh Adastrea.uproject
```

### Method 3: From Editor
```
File → Generate Visual Studio Project Files
```

---

## ✅ Verify Generation Success

### Check Solution Structure
```
Adastrea.sln
├── Adastrea (C++ project)
│   ├── Source/
│   │   ├── Adastrea/
│   │   ├── AdastreaEditor/
│   │   ├── PlayerMods/
│   │   └── StationEditor/
│   └── Config/
├── UE5 (Engine project)
└── Plugins/
    └── AdastreaDirector/
```

### Check Build Configuration
- Configuration: **Development Editor**
- Platform: **Win64** (or Linux/Mac)
- Build → Build Solution

### Expected Output
```
========== Build: X succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

---

## 🐛 Still Broken? Nuclear Option

```bash
# 1. Backup your Source/ and Content/ folders
# 2. Delete EVERYTHING except:
#    - Source/
#    - Content/
#    - Config/
#    - Plugins/
#    - Adastrea.uproject
#    - .git/
# 3. Delete: Intermediate/, Binaries/, .vs/, *.sln, *.vcxproj
# 4. Right-click Adastrea.uproject → Generate
# 5. Open .sln → Build
```

---

## 🔗 Related

- [PROJECT_GENERATION_QUICK_FIX.md](PROJECT_GENERATION_QUICK_FIX.md) — Immediate fixes
- [WINDOWS_BUILD_SCRIPTS.md](WINDOWS_BUILD_SCRIPTS.md) — Windows build details
- [BUILD_WITH_UE_TOOLS.md](BUILD_WITH_UE_TOOLS.md) — Build without editor

---

*Back to [Setup Guides](../09-SETUP_GUIDES.md) | [INDEX.md](../INDEX.md)*