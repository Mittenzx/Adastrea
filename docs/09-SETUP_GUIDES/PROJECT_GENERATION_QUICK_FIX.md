# Project Generation Quick Fix

**Immediate solutions for "Generate Visual Studio project files" failures | Last Updated: August 9, 2026**

---

## ⚡ 30-Second Fix (Try This First)

```bash
# 1. Close ALL: Visual Studio, Unreal Editor, VS Code
# 2. Delete these folders/files:
rm -rf Intermediate Binaries .vs *.sln *.vcxproj
# Windows: delete via Explorer or: rmdir /s Intermediate Binaries .vs & del *.sln *.vcxproj

# 3. Right-click Adastrea.uproject → "Generate Visual Studio project files"
# 4. Open Adastrea.sln → Build → Build Solution (Development Editor, Win64)
```

**Works 90% of the time.**

---

## 🔧 If Still Failing — Diagnostic Steps

### 1. Check UE Version Match
```json
// Adastrea.uproject
"EngineAssociation": "5.6"
```
Must match installed UE version exactly (5.6 or 5.6.1).

### 2. Verify Engine Path
```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" Adastrea.uproject

# Linux/Mac
~/UE_5.6/Engine/Build/BatchFiles/GenerateProjectFiles.sh Adastrea.uproject
```

### 3. Check .uproject Modules
```json
"Modules": [
  {"Name": "Adastrea", "Type": "Runtime", "LoadingPhase": "Default"},
  {"Name": "PlayerMods", "Type": "Runtime", "LoadingPhase": "Default"},
  {"Name": "StationEditor", "Type": "Runtime", "LoadingPhase": "Default"}
]
```
Each must have matching `Source/<Module>/<Module>.Build.cs`

### 4. Verify Build.cs Files Exist
```
Source/Adastrea/Adastrea.Build.cs          ✅
Source/PlayerMods/PlayerMods.Build.cs      ✅
Source/StationEditor/StationEditor.Build.cs ✅
Source/AdastreaEditor/AdastreaEditor.Build.cs ✅
```

### 5. Check Plugin Dependencies
```json
// Adastrea.uproject plugins
{"Name": "AdastreaDirector", "Enabled": true}
```
Plugin must exist at `Plugins/AdastreaDirector/AdastreaDirector.uplugin`

---

## 🐛 Specific Error Messages

| Error | Cause | Fix |
|-------|-------|-----|
| `Project file not found` | Wrong directory | Run from folder containing `.uproject` |
| `Engine version not found` | UE 5.6 not installed | Install via Epic Launcher |
| `Module 'X' not found` | Missing Build.cs | Check Source/ structure |
| `Circular dependency` | Adastrea ↔ StationEditor | Already fixed in Build.cs |
| `Plugin 'AdastreaDirector' not found` | Plugin not built | Build plugin or check path |
| `SetEnv task failed` (Windows) | Path too long | Use `build_with_ue_tools.bat` instead |
| `UnrealBuildTool exception` | Corrupt Intermediate | Delete Intermediate/ and retry |

---

## 🛠️ Advanced: Manual Generation

### Windows (Command Line)
```batch
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" ^
  -project="C:\Users\akuma\Adastrea\Adastrea.uproject" ^
  -game ^
  -engine ^
  -progress
```

### Linux/Mac (Command Line)
```bash
~/UE_5.6/Engine/Build/BatchFiles/GenerateProjectFiles.sh \
  -project="/home/user/Adastrea/Adastrea.uproject" \
  -game -engine -progress
```

### From UnrealBuildTool Directly
```bash
# Windows
UnrealBuildTool.exe -projectfiles -project="Adastrea.uproject" -game -engine -progress

# Linux/Mac
UnrealBuildTool -projectfiles -project="Adastrea.uproject" -game -engine -progress
```

---

## ✅ Verification Checklist

After generation, verify:
- [ ] `Adastrea.sln` exists
- [ ] Solution opens in VS 2022
- [ ] Projects: Adastrea, PlayerMods, StationEditor, AdastreaEditor, UE5
- [ ] Configuration: **Development Editor** | **Win64**
- [ ] Build → Build Solution → **0 errors, 0 warnings**
- [ ] Can open project in Unreal Editor

---

## 🆘 Still Stuck?

1. **Check Output Log** in VS: View → Output → Build
2. **Run Health Check**: `python repository_health_check.py`
3. **GitHub Issues**: Search existing or create new with:
   - Full error message
   - OS, UE version, VS version
   - Steps to reproduce
4. **Discord**: #adastrea-dev for real-time help

---

*Related: [VISUAL_STUDIO_PROJECT_GENERATION.md](VISUAL_STUDIO_PROJECT_GENERATION.md) | [BUILD_WITH_UE_TOOLS.md](BUILD_WITH_UE_TOOLS.md) | [WINDOWS_BUILD_SCRIPTS.md](WINDOWS_BUILD_SCRIPTS.md)*

*Back to [Setup Guides](../09-SETUP_GUIDES.md) | [INDEX.md](../INDEX.md)*