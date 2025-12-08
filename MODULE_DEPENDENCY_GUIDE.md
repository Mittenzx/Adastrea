# Adastrea Module Dependency Architecture

## Overview

This document explains the module dependency architecture in the Adastrea project and the proper way to structure dependencies between runtime and editor modules.

## Module Structure

The Adastrea project consists of the following modules:

- **Adastrea** - Core runtime module containing game logic, systems, and gameplay code
- **StationEditor** - Editor/runtime module for station building and editing functionality
- **PlayerMods** - Player modification and customization systems

## Dependency Rules

### Core Principle: Avoid Circular Dependencies

Unreal Build Tool (UBT) does not allow circular dependencies between modules. A circular dependency occurs when:
- Module A depends on Module B
- Module B depends on Module A

This creates: `A → B → A` which is invalid and causes build errors.

### Correct Dependency Pattern

**Runtime modules should NOT depend on editor modules.** Instead:

1. **Editor modules can depend on runtime modules** ✓
   - `StationEditor → Adastrea` (correct)
   
2. **Runtime modules should NOT depend on editor modules** ✗
   - `Adastrea → StationEditor` (incorrect - causes circular dependency)

### How Adastrea Module Structure Works

```
StationEditor (editor/runtime module)
    ↓
Adastrea (core runtime module)
```

**StationEditor** depends on **Adastrea** to access:
- `AdastreaLog.h` - Logging macros
- `SpaceStation.h` - Station classes
- `FactionDataAsset.h` - Faction data
- Other core game systems

**Adastrea** should NOT depend on **StationEditor** because:
- It would create a circular dependency
- Runtime code should not depend on editor-specific functionality
- The player controller only needs to reference StationEditor types, not depend on the module

## The Circular Dependency Fix (December 2025)

### The Problem

Previously, `Adastrea.Build.cs` had:
```csharp
PublicDependencyModuleNames.AddRange(new string[] 
{ 
    // ...
    "StationEditor"  // ❌ Incorrect - creates circular dependency
});
```

This created: `Adastrea → StationEditor → Adastrea` circular dependency.

### The Solution

Removed `StationEditor` from `Adastrea.Build.cs`:
```csharp
PublicDependencyModuleNames.AddRange(new string[] 
{ 
    "Core", 
    "CoreUObject", 
    "Engine", 
    // ... other dependencies
    "Niagara"
    // Note: StationEditor dependency removed to fix circular dependency
});
```

### Why This Works

`AdastreaPlayerController` (in Adastrea module) includes StationEditor headers, but this works because:

1. **Header file uses forward declarations:**
   ```cpp
   // AdastreaPlayerController.h
   class UStationEditorWidget;  // Forward declaration
   class UStationModuleCatalog; // Forward declaration
   ```

2. **Implementation file includes the actual headers:**
   ```cpp
   // AdastreaPlayerController.cpp
   #include "StationEditor/UI/StationEditorWidget.h"
   #include "StationEditor/StationModuleCatalog.h"
   ```

3. **Module linking happens at a different level:**
   - The Target.cs files specify which modules to include in the build
   - Both `Adastrea.Target.cs` and `AdastreaEditor.Target.cs` include StationEditor
   - This ensures StationEditor is linked into the executable
   - But the *module dependency* is one-way: StationEditor → Adastrea

## Best Practices

### Forward Declarations in Headers

Always use forward declarations in header files when possible:

```cpp
// ✓ Correct - Forward declaration
class UStationEditorWidget;

UCLASS()
class ADASTREA_API AMyClass : public AActor
{
    UPROPERTY()
    UStationEditorWidget* EditorWidget;
};
```

Avoid including headers in other headers unless necessary:

```cpp
// ✗ Incorrect - Creates tight coupling
#include "StationEditor/UI/StationEditorWidget.h"

UCLASS()
class ADASTREA_API AMyClass : public AActor
{
    UPROPERTY()
    UStationEditorWidget* EditorWidget;
};
```

### Include in Implementation Files

Include the actual headers in `.cpp` files:

```cpp
// MyClass.cpp
#include "MyClass.h"
#include "StationEditor/UI/StationEditorWidget.h"  // ✓ Include in .cpp

void AMyClass::DoSomething()
{
    if (EditorWidget)
    {
        EditorWidget->Update();
    }
}
```

### Module Dependency Checklist

Before adding a module dependency, ask:

1. ✓ Is this a runtime module depending on another runtime module?
2. ✓ Is this an editor module depending on a runtime module?
3. ✗ Is this a runtime module depending on an editor module? (avoid)
4. ✗ Does this create a circular dependency? (forbidden)

## Target.cs vs Build.cs

### Target.cs (Adastrea.Target.cs, AdastreaEditor.Target.cs)

- Specifies which **modules** to include in the **executable**
- Controls what gets linked into the final build
- Can include multiple modules

```csharp
// AdastreaEditor.Target.cs
ExtraModuleNames.Add("Adastrea");
ExtraModuleNames.Add("PlayerMods");
ExtraModuleNames.Add("StationEditor");  // ✓ OK - Links module into editor executable
```

### Build.cs (Adastrea.Build.cs, StationEditor.Build.cs)

- Specifies **dependencies** between modules
- Controls compile-time dependencies
- Must avoid circular dependencies

```csharp
// Adastrea.Build.cs
PublicDependencyModuleNames.AddRange(new string[] 
{ 
    "Core",
    "Engine"
    // NOT "StationEditor" - would create circular dependency
});
```

## Common Scenarios

### Scenario 1: Adding a New Editor Module

If you create a new editor module `MyEditor`:

1. MyEditor should depend on Adastrea:
   ```csharp
   // MyEditor.Build.cs
   PublicDependencyModuleNames.Add("Adastrea");
   ```

2. Adastrea should NOT depend on MyEditor:
   ```csharp
   // Adastrea.Build.cs - NO changes needed
   ```

3. Add to Target.cs files:
   ```csharp
   // AdastreaEditor.Target.cs
   ExtraModuleNames.Add("MyEditor");
   ```

### Scenario 2: Runtime Code Needs Editor Functionality

If runtime code needs editor functionality:

1. **Use forward declarations** in header files
2. **Include headers** in implementation files
3. **Add to Target.cs** but NOT to Build.cs dependencies
4. Consider using **interfaces** for better decoupling

### Scenario 3: Shared Functionality

If multiple modules need shared functionality:

1. Put shared code in the **Adastrea** runtime module
2. Both runtime and editor modules can depend on Adastrea
3. Example: `AdastreaLog.h` is in Adastrea and used by StationEditor

## Debugging Circular Dependencies

If you encounter a circular dependency error:

1. Check Build.cs files for mutual dependencies:
   ```bash
   grep -r "PublicDependencyModuleNames" Source/*/Build.cs
   ```

2. Draw the dependency graph:
   ```
   A → B → C → A  (circular - bad)
   ```

3. Identify where to break the cycle:
   - Remove runtime → editor dependencies
   - Use forward declarations
   - Move shared code to a common base module

4. Verify the fix:
   - Build the project
   - Check for remaining circular dependency errors

## References

- [Unreal Engine Module Programming](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)
- [Unreal Build Tool Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-build-tool-in-unreal-engine)
- Adastrea ARCHITECTURE.md - Overall system architecture
- Adastrea CODE_STYLE.md - Coding standards and patterns

---

**Last Updated**: December 8, 2025  
**Applies to**: Unreal Engine 5.6, Adastrea v2.0+  
**Maintained by**: Adastrea Development Team
