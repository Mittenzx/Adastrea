# Troubleshooting Cascading Compilation Errors

**Quick guide to diagnose and fix cascading build errors in Adastrea**

## Understanding Cascading Errors

When you fix one error and get more errors in the next build, it's usually because:

1. ✅ **Your fixes are correct** (revealing hidden issues)
2. ❌ Build cache is inconsistent (needs clean rebuild)
3. ❌ Include dependencies are incomplete (missing headers)
4. ❌ Build order has changed (modules building in wrong order)

## Diagnostic Procedure

### Step 1: Clean Rebuild ⚡ (MOST IMPORTANT)

**90% of cascading errors are fixed by this:**

```bash
# Windows PowerShell (run from project root)
Remove-Item -Recurse -Force Intermediate, Binaries, Saved, .vs
.\GenerateProjectFiles.bat
ue5.cmd -project=Adastrea.uproject -build
```

**What this does:**
- Clears all cached compilation artifacts
- Regenerates Visual Studio project files
- Forces complete rebuild from scratch
- Eliminates build state inconsistencies

**Expected result:** You'll see the **TRUE current state** of compilation errors (not cascading artifacts from cache).

### Step 2: Check Build Output 📊

After clean rebuild, analyze the errors:

#### Pattern A: "Undefined reference to..."
**Meaning**: Missing function implementation

**Example:**
```
error LNK2019: unresolved external symbol "public: void __cdecl UMyClass::MyFunction(void)"
```

**Diagnosis:**
```bash
# Find the function declaration
grep -r "MyFunction" Source/Adastrea --include="*.h"

# Check if implementation exists
grep -r "MyFunction" Source/Adastrea --include="*.cpp"
```

**Fix:** Implement the function or remove the declaration.

#### Pattern B: "No such file or directory"
**Meaning**: Missing include or incorrect path

**Example:**
```
fatal error: 'Ships/ShipModuleComponent.h' file not found
```

**Diagnosis:**
```bash
# Find where file actually is
find Source -name "ShipModuleComponent.h"

# Check include paths in Build.cs
grep -A 5 "PublicIncludePaths" Source/Adastrea/Adastrea.Build.cs
```

**Fix:** Correct the include path or add to PublicIncludePaths.

#### Pattern C: "Incomplete type"
**Meaning**: Forward declaration used where full definition needed

**Example:**
```
error: variable has incomplete type 'class UMyClass'
note: forward declaration of 'UMyClass'
```

**Fix:**
```cpp
// IN .CPP FILE (not header!)
#include "MyClass.h"  // Add full include
```

#### Pattern D: "Multiple definition of..."
**Meaning**: Implementation in header (should be in .cpp)

**Example:**
```
error: multiple definition of 'UMyClass::MyFunction()'
```

**Fix:**
```cpp
// WRONG - Implementation in header
// MyClass.h
void MyFunction() { /* implementation */ }  // ❌

// CORRECT - Implementation in .cpp
// MyClass.h
void MyFunction();  // ✅ Declaration only

// MyClass.cpp
void UMyClass::MyFunction() { /* implementation */ }  // ✅
```

### Step 3: Module Dependency Check 🔗

Check for circular dependencies:

```bash
# Windows
python .github/scripts/check_module_dependencies.py

# Manual check
# 1. Look at Adastrea.Build.cs
# 2. Look at StationEditor/StationEditor.Build.cs
# 3. Verify no circular references
```

**Rule of Thumb:**
```
✅ GOOD: StationEditor depends on Adastrea
❌ BAD:  Adastrea depends on StationEditor (circular!)
```

### Step 4: Interface Pattern Verification 🔍

Verify all interfaces follow the correct pattern:

```bash
# Should return NO files (interfaces should be header-only)
find Source/Adastrea -name "I*.cpp"

# Check interface usage
grep -r "IShipModule::" Source/Adastrea --include="*.cpp" | grep -v "Execute_" | grep -v "_Implementation"
```

**If you find interface .cpp files:**
1. Move implementations inline to header
2. Delete the .cpp file
3. Update consuming code to use Execute_ pattern

### Step 5: Generated Header Check 📄

Verify all UCLASS/USTRUCT headers have .generated.h:

```bash
# Find UCLASS/USTRUCT without generated.h
for file in $(find Source/Adastrea -name "*.h" -exec grep -l "UCLASS\|USTRUCT" {} \;); do
    if ! tail -5 "$file" | grep -q "\.generated\.h"; then
        echo "Missing .generated.h: $file"
    fi
done
```

**Fix:**
```cpp
// Add as LAST include in header
#include "YourClass.generated.h"
```

## Common Specific Issues

### Issue: IShipModule Execute_ Errors

**Symptom:**
```
error: no matching function for call to 'Execute_GetModuleCategory'
```

**Diagnosis:**
```bash
grep -A 10 "Execute_GetModuleCategory" Source/Adastrea/Ships/ShipCustomizationComponent.cpp
```

**Fix:** Ensure calling with correct parameters:
```cpp
// WRONG
IShipModule::Execute_GetModuleCategory(Module);

// CORRECT (Module must be UObject*)
IShipModule::Execute_GetModuleCategory(Cast<UObject>(Module));

// OR if Module is already UObject*
IShipModule::Execute_GetModuleCategory(Module);
```

### Issue: Data Validation Errors

**Symptom:**
```
error: no matching function for call to 'IsDataValid'
candidate: IsDataValid(FDataValidationContext&)
```

**Fix:** Migrate to UE5 pattern:
```cpp
// OLD (UE4)
EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) const;

// NEW (UE5)
#if WITH_EDITOR
EDataValidationResult IsDataValid(FDataValidationContext& Context) const;
#endif
```

### Issue: Property Not Found

**Symptom:**
```
error: 'MaxHealth' is not a member of 'USpaceshipDataAsset'
```

**Diagnosis:**
```bash
# Find where property is referenced
grep -r "MaxHealth" Source/Adastrea --include="*.cpp"

# Check actual property name in DataAsset
grep -A 5 "UPROPERTY" Source/Adastrea/Ships/SpaceshipDataAsset.h | grep -i health
```

**Fix:** Use correct property name (e.g., HullStrength instead of MaxHealth).

### Issue: Linker Errors After Interface Changes

**Symptom:**
```
error LNK2019: unresolved external symbol IShipModule::CanInstallInSlot_Implementation
```

**Cause:** Old .obj files reference deleted .cpp implementation

**Fix:**
```bash
# Nuclear option - full clean
Remove-Item -Recurse -Force Intermediate, Binaries
.\GenerateProjectFiles.bat
```

## Advanced Diagnostics

### Enable Verbose Build Output

Add to Adastrea.Build.cs:
```cpp
bPrintToolChainTimingInfo = true;
bPrintBuildSteps = true;
```

### Check Precompiled Headers

PCH issues can cause cascading errors:

```cpp
// In Adastrea.Build.cs
PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;  // Current setting (correct)

// If still issues, try:
// PCHUsage = PCHUsageMode.NoPCHs;  // Slower but more reliable
```

### Verify Module Loading Order

Check .uproject file:
```json
{
  "Modules": [
    {
      "Name": "Adastrea",
      "Type": "Runtime",
      "LoadingPhase": "Default"
    },
    {
      "Name": "StationEditor",
      "Type": "Editor",
      "LoadingPhase": "Default",
      "AdditionalDependencies": ["Adastrea"]
    }
  ]
}
```

## Preventive Measures

### Pre-Commit Checks

```bash
# 1. Build succeeds
ue5.cmd -project=Adastrea.uproject -build

# 2. No interface .cpp files
! find Source -name "I*.cpp" | grep -q .

# 3. All generated.h includes present
# (manual check or add to pre-commit hook)

# 4. No .cpp includes
! find Source -name "*.cpp" -exec grep -l "#include.*\.cpp" {} \; | grep -q .
```

### CI/CD Checks

Add to `.github/workflows/build.yml`:
```yaml
- name: Clean Build Test
  run: |
    Remove-Item -Recurse -Force Intermediate, Binaries
    .\GenerateProjectFiles.bat
    ue5.cmd -project=Adastrea.uproject -build
```

## When to Ask for Help

Ask for help if:
1. ✅ Clean rebuild still shows 50+ errors
2. ✅ Same errors appear after multiple clean rebuilds
3. ✅ Errors reference engine headers (not your code)
4. ✅ Linker errors with no clear source
5. ✅ Errors in generated code (.gen.cpp files)

Don't ask for help if:
1. ❌ Haven't done clean rebuild yet
2. ❌ Only 1-5 errors (likely quick fixes)
3. ❌ Errors are in your code (read error message carefully)

## Quick Reference Commands

```bash
# Full clean rebuild (Windows)
Remove-Item -Recurse -Force Intermediate, Binaries, Saved, .vs
.\GenerateProjectFiles.bat
ue5.cmd -project=Adastrea.uproject -build

# Find interface .cpp files (should be none)
find Source -name "I*.cpp"

# Check interface usage
grep -r "IShipModule::" Source/Adastrea --include="*.cpp"

# Find missing generated.h
for f in $(find Source/Adastrea -name "*.h"); do 
    tail -1 "$f" | grep -q "generated.h" || echo "$f"
done

# Check module dependencies
python .github/scripts/check_module_dependencies.py

# Verify property names
grep -r "MaxHealth\|MinCrew" Source/Adastrea --include="*.cpp"
```

## Success Criteria

After following this guide, you should have:
- ✅ Clean build with 0 errors
- ✅ Clear understanding of remaining warnings
- ✅ Confidence in build system health
- ✅ Checklist for future prevention

## Resources

- **PR #327 Review**: `PR_327_REVIEW_AND_RECOMMENDATIONS.md`
- **Prevention Guide**: `COMPILATION_ERROR_PREVENTION.md`
- **Quick Summary**: `PR_327_QUICK_SUMMARY.md`
- **Epic C++ Standard**: https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine

---

**Last Updated**: 2025-12-22  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team

**TL;DR**: 90% of cascading errors are fixed by clean rebuild. Do that first. ⚡
