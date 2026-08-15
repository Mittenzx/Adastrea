# Build with UE Tools Only

**Compile Adastrea without full Unreal Editor (~500 MB vs ~50 GB) | Last Updated: August 9, 2026**

---

## 🎯 Why Use This?

- **CI/CD pipelines** — No editor needed
- **Automated testing** — Fast compilation verification
- **Windows: Bypasses MSBuild SetEnv 49KB path limit** — Known UE 5.6 issue
- **Code validation** — Catch compile errors before opening Editor

> **Note**: You still need full UE 5.6 to *run* and *edit* the project. This is for compilation only.

---

## 📦 Setup (One Time)

### Linux / Mac
```bash
# Download UE Build Tools (~500 MB)
./setup_ue_build_tools.sh

# Verify
ls -la ~/UE_BuildTools/  # Should contain Engine/Build/BatchFiles/
```

### Windows
```batch
REM Download UE Build Tools (~500 MB)
setup_ue_build_tools.bat

REM Verify
dir %USERPROFILE%\UE_BuildTools\
```

---

## 🔨 Build Commands

### Linux / Mac
```bash
# Development build (fast iteration)
./build_with_ue_tools.sh Development Linux

# Shipping build (demo package)
./build_with_ue_tools.sh Shipping Linux

# Clean build
./build_with_ue_tools.sh Development Linux --clean
```

### Windows
```batch
REM Development build
build_with_ue_tools.bat Development Win64

REM Shipping build
build_with_ue_tools.bat Shipping Win64

REM Clean build
build_with_ue_tools.bat Development Win64 --clean
```

---

## ⚙️ Script Details

### `setup_ue_build_tools.sh/.bat`
Downloads Epic's BuildTools from GitHub releases:
- UnrealBuildTool.exe
- DotNET utilities
- Platform SDKs (minimal)
- No editor, no content, no samples

### `build_with_ue_tools.sh/.bat`
Runs UnrealBuildTool with project settings:
```bash
# Equivalent to:
UnrealBuildTool Adastrea Development Linux -Project="Adastrea.uproject" -TargetType=Editor -Progress -NoEngineChanges
```

---

## 🚨 Windows: SetEnv Workaround

**Problem**: UE 5.6 large projects exceed MSBuild's 49KB environment variable limit for include paths.

**Error**: `SetEnv task failed` or `The command line is too long`

**Solution**: These scripts use response files (`.rsp`) to bypass the limit.

```batch
REM build_with_ue_tools.bat internally uses:
MSBuild Adastrea.sln /p:Configuration=Development /p:Platform=Win64 /p:UseResponseFiles=true
```

---

## 🔧 Configuration

### Build Settings (Adastrea.Build.cs)
```csharp
public class Adastrea : ModuleRules
{
    public Adastrea(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bWarningsAsErrors = true;  // Strict - fix all warnings
        
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "InputCore",
            "EnhancedInput", "UMG", "Slate", "SlateCore", "Niagara"
        });
        
        PrivateDependencyModuleNames.AddRange(new[] {
            "AIModule", "NavigationSystem"
        });
    }
}
```

### Target Platforms (.uproject)
```json
"TargetPlatforms": ["Windows", "Mac", "Linux"]
```

---

## 📊 Expected Build Times

| Configuration | Platform | Time (Clean) | Time (Incremental) |
|---------------|----------|--------------|-------------------|
| Development | Linux | 3-5 min | 30-60 sec |
| Development | Windows | 5-8 min | 1-2 min |
| Shipping | Linux | 8-12 min | N/A |
| Shipping | Windows | 12-18 min | N/A |

---

## ✅ Verification

### Check Build Success
```bash
# Look for:
# [OK] Build completed successfully
# [OK] 0 errors, 0 warnings (bWarningsAsErrors=true)

# Output location:
ls -la Binaries/Linux/UnrealEditor-Adastrea  # Linux
dir Binaries\Win64\UnrealEditor-Adastrea.exe  # Windows
```

### Run Tests After Build
```bash
# Python tests (no editor needed)
python test_trading_system.py
python test_station_systems.py
python test_damage_calculations.py

# If these pass, C++ math logic is correct
```

---

## 🐛 Troubleshooting

| Issue | Solution |
|-------|----------|
| `UnrealBuildTool not found` | Run `setup_ue_build_tools.sh` first |
| `SDK not found` (Windows) | Install Visual Studio 2022 with "Game development with C++" workload |
| `Module not found` | Check `Adastrea.Build.cs` dependencies match `.uproject` modules |
| `Circular dependency` | StationEditor removed from Adastrea module - verify not re-added |
| `Permission denied` (Linux) | `chmod +x build_with_ue_tools.sh setup_ue_build_tools.sh` |

---

## 🔗 Related

- [PROJECT_GENERATION_QUICK_FIX.md](PROJECT_GENERATION_QUICK_FIX.md) — VS project issues
- [VISUAL_STUDIO_PROJECT_GENERATION.md](VISUAL_STUDIO_PROJECT_GENERATION.md) — Full VS setup
- [WINDOWS_BUILD_SCRIPTS.md](WINDOWS_BUILD_SCRIPTS.md) — Windows-specific details

---

*Back to [INDEX.md](../INDEX.md) | [Setup Guides](../09-SETUP_GUIDES.md)*