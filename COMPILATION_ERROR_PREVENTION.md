# Compilation Error Prevention Checklist

**Quick reference guide to prevent cascading compilation errors in Adastrea**

## Before Committing Code

### ✅ Header File Checklist

- [ ] Has `#pragma once` at the top
- [ ] Has `#include "YourClass.generated.h"` as LAST include
- [ ] Uses forward declarations instead of includes when possible
- [ ] All UCLASS/USTRUCT/UENUM have `GENERATED_BODY()`
- [ ] No circular include dependencies

**Example:**
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
// ... other necessary includes
#include "YourClass.generated.h"  // MUST BE LAST

// Forward declarations
class UOtherClass;
class AAnotherActor;

UCLASS()
class ADASTREA_API UYourClass : public UDataAsset
{
    GENERATED_BODY()
    // ...
};
```

### ✅ Interface Implementation Checklist

- [ ] Interface has NO .cpp file (header-only)
- [ ] Default implementations are inline in header
- [ ] Consuming code uses `Execute_FunctionName()` pattern
- [ ] Interface implementations call `Execute_` on Cast<UObject>

**Example:**
```cpp
// IN INTERFACE HEADER (IShipModule.h)
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
EShipModuleCategory GetModuleCategory() const;
virtual EShipModuleCategory GetModuleCategory_Implementation() const
{
    return EShipModuleCategory::Utility;
}

// IN CONSUMING CODE (.cpp)
if (Module->Implements<UShipModule>())
{
    EShipModuleCategory Category = IShipModule::Execute_GetModuleCategory(Module);
}

// IN IMPLEMENTATION CLASS (.cpp)
EShipModuleCategory UShipModuleComponent::GetModuleCategory_Implementation() const
{
    if (ModuleData)
    {
        return ModuleData->Category;
    }
    return IShipModule::GetModuleCategory_Implementation(); // Call parent default
}
```

### ✅ Property Declaration Checklist

- [ ] All properties have `UPROPERTY()` macro
- [ ] EditAnywhere vs VisibleAnywhere correctly used
- [ ] BlueprintReadWrite vs BlueprintReadOnly appropriate
- [ ] Category specified for organization
- [ ] Tooltips added for complex properties
- [ ] Constraints added with `meta=()` tags

**Example:**
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats", 
    meta=(ClampMin="0.0", ClampMax="100.0", ToolTip="Ship's maximum speed"))
float MaxSpeed;
```

### ✅ Data Validation Checklist (UE5)

- [ ] Uses `FDataValidationContext&` parameter
- [ ] Returns `EDataValidationResult`
- [ ] Wrapped in `#if WITH_EDITOR`
- [ ] Calls `Super::IsDataValid(Context)` first
- [ ] Uses `Context.AddError()` and `Context.AddWarning()`

**Example:**
```cpp
#if WITH_EDITOR
EDataValidationResult UYourDataAsset::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

    if (SomeProperty.IsEmpty())
    {
        Context.AddError(FText::FromString("SomeProperty is required"));
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}
#endif
```

### ✅ Function Declaration Checklist

- [ ] BlueprintCallable added if Blueprints should call it
- [ ] BlueprintPure added for getters with no side effects
- [ ] BlueprintNativeEvent for Blueprint-overridable functions
- [ ] Category specified
- [ ] Clear Doxygen comments with @param and @return

**Example:**
```cpp
/**
 * Calculate ship's effective speed
 * @param SpeedModifier Additional speed modifier to apply
 * @return Final calculated speed
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Stats")
float CalculateEffectiveSpeed(float SpeedModifier = 1.0f) const;
```

### ✅ Memory Management Checklist

- [ ] All UObject* pointers have `UPROPERTY()`
- [ ] All pointers initialized to `nullptr` in constructor
- [ ] Null checks before dereferencing
- [ ] Use `TWeakObjectPtr<>` for non-owning references
- [ ] Use `TSharedPtr<>` for non-UObject shared ownership

**Example:**
```cpp
UPROPERTY()
UMyComponent* ComponentPtr = nullptr;  // GC tracked

UPROPERTY()
TWeakObjectPtr<AActor> WeakActorRef;  // Non-owning reference
```

## Common Compilation Error Patterns

### ❌ ERROR: "Undefined reference to vtable"
**Cause**: Missing `GENERATED_BODY()` or wrong placement of .generated.h

**Fix:**
```cpp
// WRONG
UCLASS()
class UMyClass : public UObject
{
    // Missing GENERATED_BODY()
};

// CORRECT
UCLASS()
class UMyClass : public UObject
{
    GENERATED_BODY()
    
public:
    // Class content
};
```

### ❌ ERROR: "No matching function for call to Execute_"
**Cause**: Using interface function incorrectly

**Fix:**
```cpp
// WRONG
IShipModule::GetModuleCategory(Module);

// CORRECT
IShipModule::Execute_GetModuleCategory(Module);
```

### ❌ ERROR: "Cannot convert from 'TArray<FText>' to 'FDataValidationContext'"
**Cause**: Using UE4 validation pattern instead of UE5

**Fix:**
```cpp
// WRONG (UE4)
EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) const;

// CORRECT (UE5)
#if WITH_EDITOR
EDataValidationResult IsDataValid(FDataValidationContext& Context) const;
#endif
```

### ❌ ERROR: "Incomplete type" or "Forward declaration"
**Cause**: Missing include in .cpp file

**Fix:**
```cpp
// IN HEADER (.h) - Forward declaration
class UOtherClass;

UCLASS()
class UMyClass : public UObject
{
    UPROPERTY()
    UOtherClass* OtherPtr;  // Just pointer, forward declaration OK
};

// IN IMPLEMENTATION (.cpp) - Full include
#include "OtherClass.h"  // Need full definition to use the class

void UMyClass::UseOtherClass()
{
    if (OtherPtr)
    {
        OtherPtr->SomeMethod();  // Needs full definition
    }
}
```

### ❌ ERROR: "Circular dependency detected"
**Cause**: A.h includes B.h, B.h includes A.h

**Fix:**
```cpp
// IN A.h
#pragma once
#include "CoreMinimal.h"
class UClassB;  // Forward declaration, no include

UCLASS()
class UClassA : public UObject
{
    UPROPERTY()
    UClassB* BRef;  // Pointer only
};

// IN A.cpp
#include "A.h"
#include "B.h"  // Include here, not in header
```

### ❌ ERROR: "Hidden by local variable" / Variable shadowing
**Cause**: Parameter name same as member variable

**Fix:**
```cpp
// WRONG
class UMyClass
{
    float Value;
    
    void SetValue(float Value)  // Shadows member variable
    {
        Value = Value;  // Ambiguous!
    }
};

// CORRECT
class UMyClass
{
    float Value;
    
    void SetValue(float InValue)  // Use 'In' prefix
    {
        Value = InValue;
    }
};
```

## Clean Build Procedure

When encountering cascading errors:

```bash
# 1. Clean all intermediate files
rm -rf Intermediate/ Binaries/ Saved/

# 2. Regenerate project files (Windows)
GenerateProjectFiles.bat

# 3. Full rebuild
ue5.cmd -project=Adastrea.uproject -build

# 4. If still errors, clean VS cache (Windows)
# Delete .vs folder in project root
```

## Module Dependency Guidelines

### ✅ Valid Dependencies
```
Adastrea → Engine, CoreUObject, UMG, Slate
StationEditor → Adastrea
PlayerMods → Adastrea
```

### ❌ Invalid Dependencies
```
Adastrea → StationEditor  (CIRCULAR!)
Adastrea → PlayerMods     (CIRCULAR!)
```

**Rule**: Core modules (Adastrea) should NOT depend on specialized modules (StationEditor, PlayerMods)

## Testing Before Commit

1. **Build Test**: `ue5.cmd -project=Adastrea.uproject -build`
2. **Editor Test**: Open in editor, check for errors/warnings
3. **PIE Test**: Play in Editor to verify runtime behavior
4. **Code Review**: Self-review checklist above

## Pre-Commit Checklist

- [ ] Code compiles without errors
- [ ] Code compiles without warnings (or warnings documented)
- [ ] All interfaces follow header-only pattern
- [ ] All .generated.h includes are last in headers
- [ ] All UObject* have UPROPERTY()
- [ ] All Blueprint-exposed functions have correct specifiers
- [ ] No circular dependencies introduced
- [ ] Forward declarations used where possible
- [ ] Null checks before pointer dereferences
- [ ] Property constraints (ClampMin/Max) added
- [ ] Categories specified for organization
- [ ] Comments added for complex logic

## Quick Reference Commands

```bash
# Find all interface .cpp files (should be NONE)
find Source -name "I*.cpp"

# Find headers missing .generated.h
find Source -name "*.h" -exec tail -1 {} \; | grep -v "generated.h"

# Find deprecated macros
grep -r "GENERATED_UCLASS_BODY\|GENERATED_USTRUCT_BODY" Source

# Find incorrect interface calls
grep -r "IShipModule::" Source --include="*.cpp" | grep -v "Execute_" | grep -v "_Implementation"

# Find .cpp includes (anti-pattern)
find Source -name "*.cpp" -exec grep -l "#include.*\.cpp" {} \;
```

## Resources

- **Project Standards**: `.github/copilot-instructions.md`
- **Unreal Best Practices**: `.github/instructions/unreal-directive-best-practices.md`
- **Architecture**: `ARCHITECTURE.md`
- **Code Style**: `CODE_STYLE.md`
- **Epic C++ Standard**: https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine

---

**Last Updated**: 2025-12-22  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
