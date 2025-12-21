# Visual Studio Project Generation - Quick Fix Card

**Problem:** Cannot generate Visual Studio project files?  
**Error:** CS0101, "duplicate namespace", or "Bad IL format"?

---

## 🚨 IMMEDIATE FIX (90% of cases)

```batch
:: Step 1: Delete build artifacts
rmdir /s /q Intermediate
rmdir /s /q Binaries
rmdir /s /q .vs

:: Step 2: Remove duplicate plugins (if they exist)
:: Check for and delete these folders if found:
::   - Any folder ending in "_Built" (e.g., AdastreaDirector_Built)
::   - Any "HostProject" folders inside Plugins/

:: Step 3: Regenerate project files
:: Right-click Adastrea.uproject → "Generate Visual Studio project files"
```

---

## 📋 Quick Diagnostic Checklist

Run through these checks in order:

### 1. Plugin Structure Check ✓
```
Plugins/
├── AdastreaDirector/         ← Should exist (source)
├── UnrealMCP/                ← Should exist (source)
└── vc-ue-extensions/         ← Should exist (source)

❌ NO folders named:
   - AdastreaDirector_Built
   - Any_Plugin_Built
   - HostProject
```

### 2. Clean Build Artifacts ✓
```batch
del /s /q Intermediate\Build\BuildRules\*.dll
rmdir /s /q Intermediate
rmdir /s /q Binaries
```

### 3. Verify Build.cs Files ✓
Each module should have ONE .Build.cs file with a unique class name:
- ✅ `Source/Adastrea/Adastrea.Build.cs` → `public class Adastrea`
- ✅ `Plugins/AdastreaDirector/.../AdastreaDirector.Build.cs` → `public class AdastreaDirector`
- ❌ NO duplicate class names

### 4. Regenerate Project ✓
```
Right-click Adastrea.uproject → Generate Visual Studio project files
```

---

## 🔍 Still Not Working?

### Try Complete Clean:
```batch
:: Close all instances of Visual Studio and Unreal Editor first!

:: Delete everything generated
rmdir /s /q Binaries
rmdir /s /q Intermediate
rmdir /s /q Saved
rmdir /s /q .vs
del *.sln

:: Clean all plugins
for /d %%D in (Plugins\*) do (
    if exist "%%D\Binaries" rmdir /s /q "%%D\Binaries"
    if exist "%%D\Intermediate" rmdir /s /q "%%D\Intermediate"
)

:: Regenerate
:: Right-click Adastrea.uproject → Generate Visual Studio project files
```

---

## 📖 Need More Help?

See the complete troubleshooting guide:
**[VISUAL_STUDIO_PROJECT_GENERATION.md](VISUAL_STUDIO_PROJECT_GENERATION.md)**

Covers:
- Detailed explanation of all errors
- Platform-specific instructions (Windows/Mac/Linux)
- Module dependency issues
- Plugin loading problems
- Advanced debugging techniques

---

## 🎯 Error Code Reference

| Error Code | Meaning | Quick Fix |
|------------|---------|-----------|
| **CS0101** | Duplicate class/namespace | Remove `*_Built` folders + clean |
| **Bad IL Format** | Corrupted DLL files | Delete `Intermediate/Build/BuildRules/*.dll` |
| **Module not found** | Missing .Build.cs or wrong structure | Verify plugin structure |
| **Circular dependency** | Modules reference each other | Run `check_module_dependencies.py` |

---

## 💡 Prevention Tips

Add to your `.gitignore` (already done in this repo):
```gitignore
*_Built/
HostProject/
/Intermediate/Build/BuildRules/*.dll
```

**Never** put built plugins back into the source tree!

---

**Quick Start:** [README.md](README.md) | **Full Guide:** [VISUAL_STUDIO_PROJECT_GENERATION.md](VISUAL_STUDIO_PROJECT_GENERATION.md)  
**Build Errors:** [build_errors.md](build_errors.md) | **Module Guide:** [MODULE_DEPENDENCY_GUIDE.md](MODULE_DEPENDENCY_GUIDE.md)
