# Code Style Guide

This document defines the coding standards for the Adastrea project. Following these guidelines ensures consistency and maintainability.

## Table of Contents

- [General Principles](#general-principles)
- [C++ Style](#c-style)
- [Blueprint Style](#blueprint-style)
- [Asset Naming](#asset-naming)
- [Documentation](#documentation)

## General Principles

1. **Readability First**: Code should be self-documenting
2. **Consistency**: Follow existing patterns in the codebase
3. **Simplicity**: Prefer simple solutions over clever ones
4. **Modularity**: Keep systems decoupled and focused
5. **Performance**: Optimize when necessary, not prematurely

## C++ Style

### Naming Conventions

#### Classes and Structs

```cpp
// UObject-derived classes
UCLASS()
class ADASTREA_API UMyDataAsset : public UDataAsset
{
    // ...
};

// Actor-derived classes
UCLASS()
class ADASTREA_API AMyActor : public AActor
{
    // ...
};

// Structs
USTRUCT(BlueprintType)
struct FMyStruct
{
    // ...
};

// Enums
UENUM(BlueprintType)
enum class EMyEnum : uint8
{
    // ...
};

// Interfaces
UINTERFACE()
class UMyInterface : public UInterface
{
    // ...
};

class IMyInterface
{
    // ...
};
```

**Rules**:
- `U` prefix for UObject-derived classes
- `A` prefix for Actor-derived classes
- `F` prefix for structs (including USTRUCT)
- `E` prefix for enums
- `I` prefix for interface implementations
- PascalCase for all class names

#### Variables

```cpp
// Member variables
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
float MaxHealth;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
bool bIsAlive;

// Local variables
int32 HealthValue = 100;
float DamageAmount = 25.0f;
bool bShouldRespawn = true;

// Constants
static const float kMaxSpeed = 1000.0f;
static constexpr int32 MAX_CREW = 100;

// Pointers
AActor* MyActor = nullptr;
UDataAsset* MyDataAsset = nullptr;
```

**Rules**:
- PascalCase for member variables
- camelCase for local variables (or PascalCase - be consistent)
- Boolean variables prefixed with `b`
- Constants: `kConstantName` or `ALL_CAPS`
- Always initialize pointers to `nullptr`

#### Functions

```cpp
// Blueprint-callable function
UFUNCTION(BlueprintCallable, Category="Combat")
void DealDamage(float DamageAmount);

// Blueprint-pure function (getter)
UFUNCTION(BlueprintPure, Category="Stats")
float GetHealth() const;

// Blueprint-implementable event
UFUNCTION(BlueprintImplementableEvent, Category="Events")
void OnHealthChanged(float NewHealth);

// Blueprint-native event
UFUNCTION(BlueprintNativeEvent, Category="AI")
void MakeDecision();
virtual void MakeDecision_Implementation();

// Regular function
void UpdateInternalState();
```

**Rules**:
- PascalCase for function names
- Descriptive verb-noun combinations (GetHealth, SetDamage, CalculateScore)
- Use `const` for functions that don't modify state
- Mark Blueprint-callable functions appropriately
- BlueprintNativeEvent functions need `_Implementation` suffix in .cpp

### Header File Structure

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyDataAsset.generated.h"

// Forward declarations
class UOtherClass;
class AOtherActor;

/**
 * Brief one-line description of the class
 * 
 * Detailed description explaining:
 * - What this class does
 * - When to use it
 * - How it integrates with other systems
 * 
 * Usage Example:
 * - Create a Blueprint based on this class
 * - Configure properties in editor
 * - Use Blueprint functions to interact
 */
UCLASS(BlueprintType)
class ADASTREA_API UMyDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // ====================
    // Properties
    // ====================
    
    // Brief description of what this property does
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText DisplayName;

    // ====================
    // Public Functions
    // ====================
    
    /**
     * Brief description of what this function does
     * @param ParamName Description of parameter
     * @return Description of return value
     */
    UFUNCTION(BlueprintCallable, Category="My Category")
    int32 CalculateValue(float Input) const;

protected:
    // ====================
    // Protected Members
    // ====================
    
    UPROPERTY()
    int32 InternalValue;

private:
    // ====================
    // Private Members
    // ====================
    
    void InternalHelperFunction();
};
```

### Implementation File Structure

```cpp
#include "MyDataAsset.h"
#include "Other/Dependencies.h"

UMyDataAsset::UMyDataAsset()
{
    // Initialize default values
    DisplayName = FText::FromString(TEXT("Default Name"));
    InternalValue = 0;
}

int32 UMyDataAsset::CalculateValue(float Input) const
{
    // Validate input
    if (Input < 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid input: %f"), Input);
        return 0;
    }

    // Calculate result
    int32 Result = FMath::RoundToInt(Input * InternalValue);
    
    return Result;
}

void UMyDataAsset::InternalHelperFunction()
{
    // Implementation
}
```

### UPROPERTY Best Practices

**Critical Rule**: ALL `UObject*` pointers MUST have `UPROPERTY()` macro, even private ones. This is required for Unreal Engine's garbage collection to track object references. Without UPROPERTY, objects may be prematurely destroyed, causing crashes.

```cpp
// ❌ WRONG - Missing UPROPERTY (will cause GC issues)
private:
    UDataAsset* MyData;

// ✅ CORRECT - All UObject* pointers must have UPROPERTY
private:
    UPROPERTY()
    UDataAsset* MyData;

// ✅ CORRECT - Even private pointers need UPROPERTY for GC tracking
protected:
    UPROPERTY()
    AActor* CachedActor;

// Editable in editor, read-only in Blueprints
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
float MaxValue;

// Editable everywhere
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
float CurrentValue;

// Visible but not editable
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
float CalculatedValue;

// With constraints
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats",
          meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
float HealthPercent;

// With multiline text
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Info",
          meta=(MultiLine=true))
FText Description;

// Array
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Items")
TArray<UItemDataAsset*> Items;

// Map
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Relations")
TMap<FName, int32> Relationships;
```

### UFUNCTION Best Practices

```cpp
// Callable function
UFUNCTION(BlueprintCallable, Category="Combat")
void Attack(float Damage);

// Pure function (no side effects, shows as pure node in Blueprint)
UFUNCTION(BlueprintPure, Category="Stats")
float GetHealth() const;

// BlueprintNativeEvent (can be overridden in Blueprint)
UFUNCTION(BlueprintNativeEvent, Category="AI")
void OnTargetDetected(AActor* Target);
virtual void OnTargetDetected_Implementation(AActor* Target);

// BlueprintImplementableEvent (implemented only in Blueprint)
UFUNCTION(BlueprintImplementableEvent, Category="Events")
void OnDamageReceived(float Damage);
```

### Comments

```cpp
// Single-line comment for brief notes
int32 Value; // Inline comment

/**
 * Multi-line Doxygen-style comment for classes, functions, and properties
 * 
 * Detailed explanation of what this does, why it exists, and how to use it.
 * 
 * @param InputValue The value to process
 * @param bShouldValidate Whether to validate the input
 * @return The processed result, or 0 if validation fails
 */
UFUNCTION(BlueprintCallable, Category="Processing")
int32 ProcessValue(int32 InputValue, bool bShouldValidate) const;

// TODO: Implement advanced feature
// FIXME: This has a known issue with negative values
// NOTE: This relies on initialization order
```

## Blueprint Style

### Naming Conventions

- **Blueprints**: `BP_SystemName_Purpose`
  - `BP_Ship_Fighter`
  - `BP_Station_TradeHub`
  - `BP_UI_MainMenu`

- **Widgets**: `WBP_WidgetName`
  - `WBP_HUD_Main`
  - `WBP_Dialog_Confirmation`

- **Data Assets**: `DA_Type_Name`
  - `DA_Ship_Pathfinder`
  - `DA_Faction_SolarisUnion`
  - `DA_Personnel_Captain`

### Organization

```
Content/
├── Blueprints/
│   ├── Ships/
│   │   ├── BP_Ship_Fighter
│   │   ├── BP_Ship_Freighter
│   │   └── BP_Ship_Capital
│   ├── Stations/
│   │   ├── BP_Station_Military
│   │   └── BP_Station_Trade
│   └── UI/
│       ├── WBP_HUD_Main
│       └── WBP_Menu_Settings
└── DataAssets/
    ├── Ships/
    ├── Factions/
    └── Personnel/
```

### Blueprint Graphs

1. **Use Comment Boxes**:
   - Group related nodes
   - Describe what each section does
   - Use consistent colors

2. **Keep Functions Small**:
   - Max 20-30 nodes per function
   - Extract complex logic to separate functions

3. **Use Reroute Nodes**:
   - Keep wires organized
   - Avoid crossing wires

4. **Name Variables Clearly**:
   - Use categories to organize
   - Add tooltips for complex variables

5. **Consistent Layout**:
   - Left to right flow
   - Top to bottom for branches
   - Align nodes neatly

## Asset Naming

### General Rules

```
[Prefix]_[AssetType]_[Descriptor]_[Variant]_[Suffix]

Examples:
T_Rock_Diffuse_01_D      (Texture, Diffuse)
M_Metal_Shiny            (Material)
MI_Metal_Shiny_Blue      (Material Instance)
BP_Station_Agricultural  (Blueprint, Station)
WBP_TradingUI           (Widget, Trading)
DA_TradeItem_Water      (Data Asset, Trade Item)
```

**IMPORTANT**: For complete file structure and naming standards, see:
📖 **[FILE_STRUCTURE_STANDARD.md](docs/reference/FILE_STRUCTURE_STANDARD.md)** - Official source of truth

### Asset Prefixes

| Prefix | Asset Type | Location | Example |
|--------|------------|----------|---------|
| `BP_` | Blueprint Class | `Content/Blueprints/[System]/` | `BP_TradingStation` |
| `WBP_` | Widget Blueprint | `Content/UI/[System]/` | `WBP_TradingUI` |
| `DA_` | Data Asset | `Content/DataAssets/[System]/` | `DA_TradeItem_Water` |
| `M_` | Material (Master) | `Content/Materials/Master/` | `M_Metal_Base` |
| `MI_` | Material Instance | `Content/Materials/Instances/` | `MI_Metal_Blue` |
| `SM_` | Static Mesh | `Content/Meshes/[Type]/` | `SM_Ship_Fighter` |
| `SK_` | Skeletal Mesh | `Content/Meshes/[Type]/` | `SK_Character_Human` |
| `T_` | Texture | `Content/Textures/[Type]/` | `T_Ship_Hull_D` |
| `A_` | Animation | `Content/Animations/` | `A_Walk_Forward` |
| `S_` | Sound | `Content/Audio/SFX/` | `S_Explosion_Large` |
| `P_` | Particle System | `Content/VFX/Particles/` | `P_Engine_Trail` |
| `L_` | Level/Map | `Content/Maps/` | `L_TradingTest` |

### Trade Simulator MVP Naming Standards

**✅ OFFICIAL NAMES** (use these):
- `WBP_TradingUI` - Main trading interface widget
- `WBP_TradeItemRow` - Trade item row widget
- `BP_TradingStation` - Base trading station
- `BP_Station_Agricultural` - Agricultural station variant
- `BP_Station_Industrial` - Industrial station variant
- `BP_TradingShip` - Player trading ship
- `BP_TradingGameMode` - Trading game mode
- `DA_TradeItem_Water` - Water trade item
- `DA_Market_Agricultural` - Agricultural market

**❌ DEPRECATED NAMES** (do not use):
- ~~`WBP_TradingInterface`~~ → Use `WBP_TradingUI`
- ~~`WBP_Trading_Interface`~~ → Use `WBP_TradingUI`
- ~~`BP_UI_Trading`~~ → Use `WBP_TradingUI`
- ~~`BP_MVP_*`~~ → No MVP prefix needed
SM_Crate_Large           (Static Mesh)
SK_Character_Hero        (Skeletal Mesh)
A_Hero_Idle              (Animation)
DA_Ship_Fighter          (Data Asset)
BP_Enemy_Boss            (Blueprint)
```

### Common Prefixes

| Asset Type | Prefix | Example |
|------------|--------|---------|
| Blueprint | BP_ | BP_Ship_Fighter |
| Data Asset | DA_ | DA_Faction_Traders |
| Material | M_ | M_Metal_Base |
| Material Instance | MI_ | MI_Metal_Blue |
| Texture | T_ | T_Metal_Diffuse |
| Static Mesh | SM_ | SM_Crate |
| Skeletal Mesh | SK_ | SK_Character |
| Animation | A_ | A_Walk |
| Sound | S_ | S_Explosion |
| Particle | P_ | P_Smoke |
| Widget Blueprint | WBP_ | WBP_MainMenu |

### Texture Suffixes

- `_D` - Diffuse/Base Color
- `_N` - Normal Map
- `_R` - Roughness
- `_M` - Metallic
- `_AO` - Ambient Occlusion
- `_H` - Height Map
- `_E` - Emissive

## Documentation

### When to Document

- **Always**: Public APIs, classes, complex functions
- **Usually**: Protected members, important algorithms
- **Rarely**: Self-explanatory code, obvious getters/setters

### How to Document

```cpp
/**
 * Brief description (one line)
 * 
 * Detailed explanation:
 * - What it does
 * - Why it exists
 * - How to use it
 * - Important caveats or gotchas
 * 
 * @param ParamName Parameter description
 * @return Return value description
 * @see RelatedClass, RelatedFunction
 */
```

### Documentation Files

- **System Guides**: `Assets/[System]Guide.md`
- **Workflows**: `Assets/[System]Workflow.md`
- **Templates**: `Assets/[System]Templates.md`
- **API Reference**: `Assets/[System]APIReference.md`

## Code Review Checklist

Before submitting code:

- [ ] Follows naming conventions
- [ ] Has appropriate comments
- [ ] Blueprint exposure is correct
- [ ] Properties have categories
- [ ] No compiler warnings
- [ ] Code is formatted consistently
- [ ] Documentation is updated
- [ ] Examples are provided

## Tools

### Recommended IDE Extensions

**Visual Studio**:
- Visual Assist (IntelliSense enhancement)
- CodeMaid (code cleanup)
- ReSharper C++ (refactoring)

**VS Code**:
- C/C++ Extension Pack
- Unreal Engine 4 Snippets
- Blueprint syntax highlighting

### Code Formatting

Use `.editorconfig` for consistent formatting:
- Indent: 4 spaces
- Line endings: LF (Unix)
- Trim trailing whitespace
- Final newline in files

## References

- [Unreal Engine Coding Standard](https://docs.unrealengine.com/5.6/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- [UE4 Style Guide](https://github.com/Allar/ue5-style-guide)
- [Modern C++ Best Practices](https://isocpp.github.io/CppCoreGuidelines/)

---

**Remember**: Consistency is more important than perfection. When in doubt, follow existing patterns in the codebase.
