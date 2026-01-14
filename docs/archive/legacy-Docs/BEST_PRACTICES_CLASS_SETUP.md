# Best Practices for Class Setup in Adastrea

This document outlines the class setup standards and best practices for the Adastrea project, based on Unreal Engine conventions and C++ best practices.

## Table of Contents

- [Access Specifiers](#access-specifiers)
- [Blueprint Exposure](#blueprint-exposure)
- [Const Correctness](#const-correctness)
- [Forward Declarations](#forward-declarations)
- [Documentation Standards](#documentation-standards)
- [Naming Conventions](#naming-conventions)
- [Common Patterns](#common-patterns)

## Access Specifiers

### Golden Rule: Public Blueprint API

**All `UFUNCTION` declarations with `BlueprintCallable`, `BlueprintPure`, or `BlueprintNativeEvent` MUST be in the `public` or `protected` section.**

❌ **WRONG:**
```cpp
UCLASS()
class UMyClass : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PublicProperty;

private:
    // This is WRONG - Blueprint functions in private section!
    UFUNCTION(BlueprintCallable)
    void DoSomething();  // ❌ Not accessible from Blueprints!
};
```

✅ **CORRECT:**
```cpp
UCLASS()
class UMyClass : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PublicProperty;

    // Blueprint-callable functions in public section
    UFUNCTION(BlueprintCallable, Category="My Category")
    void DoSomething();  // ✓ Properly accessible from Blueprints

private:
    // Private implementation details
    void InternalHelper();  // ✓ C++-only helper function
};
```

### Recommended Structure

```cpp
UCLASS(BlueprintType)
class ADASTREA_API UMyDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 1. Constructor
    UMyDataAsset();

    // 2. Public UPROPERTY declarations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText DisplayName;

    // 3. Blueprint-callable functions
    UFUNCTION(BlueprintCallable, Category="My Category")
    int32 CalculateValue() const;

    // 4. BlueprintNativeEvent functions
    UFUNCTION(BlueprintNativeEvent, Category="My Category")
    void OnSomethingHappened();
    virtual void OnSomethingHappened_Implementation();

protected:
    // 5. Protected virtual overrides
    virtual void BeginPlay() override;

    // 6. Protected helper functions for derived classes
    void ProtectedHelperFunction();

private:
    // 7. Private member variables (caches, internal state)
    mutable TMap<FName, int32> CachedData;

    // 8. Private helper functions
    void RebuildCache() const;
};
```

## Blueprint Exposure

### UPROPERTY Specifiers

Choose the appropriate specifiers based on how the property should be used:

| Use Case | Specifiers | Example |
|----------|-----------|---------|
| Designer-configurable, Blueprint read/write | `EditAnywhere, BlueprintReadWrite` | `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")` |
| Designer-configurable, Blueprint read-only | `EditAnywhere, BlueprintReadOnly` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")` |
| Runtime-only, Blueprint read/write | `VisibleAnywhere, BlueprintReadWrite` | `UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")` |
| Display-only, Blueprint read-only | `VisibleAnywhere, BlueprintReadOnly` | `UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Info")` |
| Internal state, not exposed | `UPROPERTY()` | `UPROPERTY()` (for garbage collection only) |

### UFUNCTION Specifiers

| Function Type | Specifiers | When to Use |
|--------------|-----------|-------------|
| Callable action | `BlueprintCallable` | Functions that perform actions or have side effects |
| Pure getter | `BlueprintPure` | Getter functions with no side effects |
| Overridable in Blueprint | `BlueprintNativeEvent` | Functions that can be overridden in Blueprints with C++ fallback |
| Blueprint-only override | `BlueprintImplementableEvent` | Functions only implemented in Blueprints |

### Meta Tags

Use meta tags to improve the editor experience:

```cpp
// Numeric clamping
UPROPERTY(EditAnywhere, Category="Stats", meta=(ClampMin="0", ClampMax="100"))
float HealthPercentage;

// UI slider ranges
UPROPERTY(EditAnywhere, Category="Stats", meta=(UIMin="0.0", UIMax="10.0"))
float Speed;

// Multiline text
UPROPERTY(EditAnywhere, Category="Info", meta=(MultiLine=true))
FText Description;

// Tooltips
UPROPERTY(EditAnywhere, Category="Advanced", meta=(ToolTip="This setting affects performance"))
bool bEnableOptimization;
```

## Const Correctness

### Mark Non-Mutating Functions as Const

All getter functions and query functions that don't modify object state MUST be marked `const`:

✅ **CORRECT:**
```cpp
UFUNCTION(BlueprintCallable, Category="Stats")
float GetCombatRating() const;  // ✓ Doesn't modify state

UFUNCTION(BlueprintPure, Category="Query")
bool HasTrait(FName TraitID) const;  // ✓ Pure getter

UFUNCTION(BlueprintCallable, Category="Info")
TArray<FName> GetAllIDs() const;  // ✓ Returns copy, doesn't modify
```

❌ **WRONG:**
```cpp
UFUNCTION(BlueprintCallable)
float GetCombatRating();  // ❌ Missing const

UFUNCTION(BlueprintPure)
bool HasTrait(FName TraitID);  // ❌ Pure function should be const
```

### Const Parameters

Use `const&` for input parameters of complex types:

```cpp
// Simple types - pass by value
void SetHealth(float NewHealth);
void SetID(FName NewID);

// Complex types - pass by const reference
void SetDescription(const FText& NewDescription);
void AddTrait(const FMyTrait& NewTrait);
bool HasRelationshipWith(const FName& OtherID) const;
```

### Mutable for Caching

Use `mutable` for cache variables that can change in const functions:

```cpp
class UFactionDataAsset : public UPrimaryDataAsset
{
public:
    // Const function that may modify cache
    bool GetRelationship(FName OtherID, FFactionRelationship& OutRelationship) const;

private:
    // Cache can be modified even in const functions
    mutable TMap<FName, const FFactionRelationship*> RelationshipCache;
    mutable bool bCacheValid;
};
```

## Forward Declarations

### When to Use Forward Declarations

Use forward declarations in headers when you only need pointers or references:

✅ **CORRECT - Use Forward Declaration:**
```cpp
// MyClass.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyClass.generated.h"

// Forward declarations - only need pointers
class UFactionDataAsset;
class USpaceshipDataAsset;

UCLASS()
class AMyClass : public AActor
{
    GENERATED_BODY()

public:
    // Pointer members - forward declaration sufficient
    UPROPERTY(EditAnywhere)
    UFactionDataAsset* Faction;

    // Function parameters - forward declaration sufficient
    void SetFaction(UFactionDataAsset* NewFaction);
};
```

❌ **WRONG - Unnecessary Full Include:**
```cpp
// MyClass.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Factions/FactionDataAsset.h"  // ❌ Full include not needed
#include "Ships/SpaceshipDataAsset.h"   // ❌ Full include not needed
#include "MyClass.generated.h"

UCLASS()
class AMyClass : public AActor
{
    // ... same as above
};
```

### When to Use Full Includes

Use full includes in headers only when:

1. **Inheritance** - You inherit from the class
2. **Member Variables** - You have member variables by value (not pointer)
3. **Template Specialization** - You need the complete type definition

```cpp
// MyDerivedClass.h
#pragma once

#include "CoreMinimal.h"
#include "Ships/SpaceshipDataAsset.h"  // ✓ Needed for inheritance
#include "MyDerivedClass.generated.h"

UCLASS()
class UMyDerivedClass : public USpaceshipDataAsset  // Inheriting
{
    GENERATED_BODY()
    // ...
};
```

### Include Full Headers in .cpp Files

Always include full headers in implementation files:

```cpp
// MyClass.cpp
#include "MyClass.h"
#include "Factions/FactionDataAsset.h"  // ✓ Full include in .cpp
#include "Ships/SpaceshipDataAsset.h"   // ✓ Full include in .cpp

void AMyClass::SetFaction(UFactionDataAsset* NewFaction)
{
    // Now we can access FactionDataAsset members
    if (NewFaction)
    {
        FText Name = NewFaction->FactionName;
        // ...
    }
}
```

## Documentation Standards

### Class Documentation

Every class should have a comprehensive header comment:

```cpp
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
 * 
 * See Also: RelatedClass, RelatedSystem
 */
UCLASS(BlueprintType)
class ADASTREA_API UMyClass : public UObject
{
    GENERATED_BODY()
    // ...
};
```

### Function Documentation

Document all public and protected functions:

```cpp
/**
 * Calculate the overall combat effectiveness rating
 * 
 * This function combines armor, shields, weapons, and point defense
 * into a single 0-100 rating value.
 * 
 * @return Combat effectiveness rating (0-100)
 * 
 * @see GetMobilityRating, GetUtilityRating
 */
UFUNCTION(BlueprintCallable, Category="Stats")
float GetCombatRating() const;

/**
 * Check if ship is suitable for a specific role
 * 
 * @param Role The role to check (e.g., "Combat", "Trade", "Exploration")
 * @return True if ship is well-suited for this role
 */
UFUNCTION(BlueprintCallable, Category="Stats")
bool IsSuitableForRole(const FString& Role) const;
```

### Property Documentation

Use inline comments for properties:

```cpp
// Maximum hull strength (health points)
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Core Stats", meta=(ClampMin="1", ClampMax="100000"))
float HullStrength;

// Whether this faction is hostile by default
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Relations")
bool bHostileByDefault;
```

## Naming Conventions

### Classes and Types

| Type | Prefix | Example |
|------|--------|---------|
| UObject-derived class | `U` | `USpaceshipDataAsset` |
| AActor-derived class | `A` | `ASpaceStation` |
| Struct | `F` | `FFactionTrait` |
| Enum | `E` | `EPersonnelRole` |
| Interface (implementation) | `I` | `IMyInterface` |

### Variables

```cpp
// Member variables - PascalCase
float MaxHealth;
int32 CrewRequired;

// Boolean properties - 'b' prefix
bool bIsAlive;
bool bHostileByDefault;

// Constants
static const float kMaxSpeed = 1000.0f;
static constexpr int32 MAX_CREW = 100;

// Pointers - always initialize
AActor* MyActor = nullptr;
UObject* MyObject = nullptr;
```

### Functions

```cpp
// PascalCase with verb-noun combinations
void SetHealth(float NewHealth);
float GetHealth() const;
bool CalculateDamage(float Amount);
void UpdateState();
```

## Common Patterns

### Data Asset Pattern

```cpp
UCLASS(BlueprintType)
class ADASTREA_API UMyDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 1. Constructor
    UMyDataAsset();

    // 2. Designer-configurable properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FName UniqueID;

    // 3. Blueprint-callable query functions
    UFUNCTION(BlueprintCallable, Category="My Category")
    float CalculateValue() const;

    UFUNCTION(BlueprintPure, Category="My Category")
    bool HasProperty(FName PropertyID) const;
};
```

### Actor with Data Asset Pattern

```cpp
UCLASS()
class ADASTREA_API AMyActor : public AActor
{
    GENERATED_BODY()

public:
    AMyActor();

    // Reference to data asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data")
    UMyDataAsset* DataAsset;

    // Actor-specific functionality
    UFUNCTION(BlueprintCallable, Category="Actor")
    void DoSomething();

protected:
    virtual void BeginPlay() override;

private:
    // Internal state
    float CachedValue;
};
```

### BlueprintNativeEvent Pattern

```cpp
UCLASS(Blueprintable)
class ADASTREA_API UMyLogic : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Called when something happens
     * Override in Blueprint to customize behavior
     * 
     * @param Amount The amount of change
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Events")
    void OnSomethingHappened(float Amount);
    
    // C++ implementation (can be called from Blueprint override)
    virtual void OnSomethingHappened_Implementation(float Amount);
};
```

## Checklist for New Classes

When creating a new class, verify:

- [ ] Proper class prefix (U, A, F, E)
- [ ] `#pragma once` header guard
- [ ] `#include "CoreMinimal.h"`
- [ ] `.generated.h` as last include
- [ ] Comprehensive class documentation
- [ ] All BlueprintCallable functions in public section
- [ ] Const correctness on getter functions
- [ ] Forward declarations used where appropriate
- [ ] Properties have appropriate UPROPERTY specifiers
- [ ] Functions have appropriate UFUNCTION specifiers
- [ ] Categories for all exposed properties/functions
- [ ] Meta tags for numeric ranges and UI hints
- [ ] Constructor declared and implemented
- [ ] Pointers initialized to nullptr (if not UPROPERTY)

## References

- [Unreal Engine C++ Coding Standard](https://docs.unrealengine.com/5.0/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- [Adastrea CODE_STYLE.md](../CODE_STYLE.md)
- [Adastrea ARCHITECTURE.md](../ARCHITECTURE.md)

---

**Last Updated**: 2025-01-05  
**Maintained by**: Adastrea Development Team
