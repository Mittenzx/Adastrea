# Unreal Directive Implementation - Phase 3 Implementation Guide

**Phase**: 3 - Quality Improvements (Medium Priority)  
**Status**: 📋 Planning / Ready to Implement  
**Estimated Time**: 20-27 hours  
**Prerequisites**: Phase 1 Complete ✅, Phase 2 Recommended

---

## Overview

Phase 3 focuses on code quality, maintainability, and developer experience improvements. These changes make the codebase easier to work with, more extensible, and better protected against common errors.

---

## Task 1: Interface Patterns (6-8 hours)

### Objective
Create and implement standard interfaces for common behaviors to improve code organization and enable polymorphism.

### 1.1 IDamageable Interface

**Purpose**: Standardize damage handling across all damageable entities

**Implementation**:

```cpp
// Source/Adastrea/Public/Interfaces/IDamageable.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IDamageable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDamageable : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that can receive damage
 * 
 * Implement this interface on:
 * - Spaceships
 * - Space stations
 * - Station modules
 * - Destructible objects
 * 
 * Benefits:
 * - Unified damage handling
 * - Easy to add new damageable types
 * - Blueprint-friendly
 */
class ADASTREA_API IDamageable
{
    GENERATED_BODY()

public:
    /**
     * Apply damage to this actor
     * @param Damage Amount of damage to apply
     * @param DamageType Type of damage (Energy, Kinetic, etc.)
     * @param Instigator Actor that caused the damage
     * @param DamageCauser Direct cause of damage (projectile, explosion, etc.)
     * @return Actual damage applied (after armor/shields)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    float ApplyDamage(float Damage, EDamageType DamageType, AActor* Instigator, AActor* DamageCauser);

    /**
     * Check if this actor can currently take damage
     * @return True if damageable
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    bool CanTakeDamage() const;

    /**
     * Get current health percentage (0-1)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    float GetHealthPercentage() const;

    /**
     * Check if this actor is destroyed/dead
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    bool IsDestroyed() const;
};
```

**Usage Example**:

```cpp
// In WeaponComponent.cpp
void UWeaponComponent::ApplyDamageToTarget(AActor* Target)
{
    if (!Target || !WeaponData)
    {
        return;
    }

    // Check if target implements IDamageable
    if (Target->Implements<UDamageable>())
    {
        IDamageable* DamageableTarget = Cast<IDamageable>(Target);
        if (DamageableTarget)
        {
            float DamageDealt = DamageableTarget->Execute_ApplyDamage(
                Target,
                WeaponData->Damage,
                WeaponData->DamageType,
                GetOwner(),
                GetOwner()
            );

            // Log or process damage dealt
            OnDamageDealt(Target, DamageDealt);
        }
    }
}
```

### 1.2 ITargetable Interface

**Purpose**: Standardize targeting system interactions

```cpp
// Source/Adastrea/Public/Interfaces/ITargetable.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ITargetable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UTargetable : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that can be targeted by weapons/sensors
 */
class ADASTREA_API ITargetable
{
    GENERATED_BODY()

public:
    /**
     * Check if this actor can be targeted
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    bool CanBeTargeted() const;

    /**
     * Get targeting priority (higher = higher priority)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    int32 GetTargetPriority() const;

    /**
     * Get target display name
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    FText GetTargetDisplayName() const;

    /**
     * Get target icon/type for UI
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    UTexture2D* GetTargetIcon() const;

    /**
     * Get optimal aim point (center of mass, weak point, etc.)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Targeting")
    FVector GetAimPoint() const;
};
```

### 1.3 IFactionMember Interface

**Purpose**: Standardize faction affiliation queries

```cpp
// Source/Adastrea/Public/Interfaces/IFactionMember.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IFactionMember.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UFactionMember : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that belong to a faction
 */
class ADASTREA_API IFactionMember
{
    GENERATED_BODY()

public:
    /**
     * Get this actor's faction
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    UFactionDataAsset* GetFaction() const;

    /**
     * Check if this actor is allied with another faction member
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    bool IsAlliedWith(const TScriptInterface<IFactionMember>& Other) const;

    /**
     * Check if this actor is hostile to another faction member
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    bool IsHostileTo(const TScriptInterface<IFactionMember>& Other) const;

    /**
     * Get faction relationship value with another faction member
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction")
    int32 GetRelationshipWith(const TScriptInterface<IFactionMember>& Other) const;
};
```

### Implementation Checklist

- [ ] Create interface header files in `Source/Adastrea/Public/Interfaces/`
- [ ] Implement interfaces on existing classes:
  - [ ] `ASpaceship` implements `IDamageable`, `ITargetable`, `IFactionMember`
  - [ ] `ASpaceStation` implements `IDamageable`, `ITargetable`, `IFactionMember`
  - [ ] `ASpaceStationModule` implements `IDamageable`, `IFactionMember`
- [ ] Update weapon system to use `IDamageable`
- [ ] Update targeting system to use `ITargetable`
- [ ] Update AI to use `IFactionMember`
- [ ] Add Blueprint examples
- [ ] Document interface usage in guides

---

## Task 2: Utility Function Library (4-6 hours)

### Objective
Create reusable utility functions to reduce code duplication and improve maintainability.

### Implementation

```cpp
// Source/Adastrea/Public/AdastreaFunctionLibrary.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AdastreaFunctionLibrary.generated.h"

/**
 * Blueprint Function Library for Adastrea
 * 
 * Provides commonly used utility functions for:
 * - Distance and position calculations
 * - Faction relationships
 * - Combat calculations
 * - Data validation
 * 
 * All functions are static and BlueprintCallable.
 */
UCLASS()
class ADASTREA_API UAdastreaFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ====================
    // DISTANCE & POSITION
    // ====================

    /**
     * Get 2D distance between actors (ignoring Z axis)
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Position")
    static float GetDistance2D(AActor* ActorA, AActor* ActorB);

    /**
     * Check if actor is within range
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Position")
    static bool IsWithinRange(AActor* ActorA, AActor* ActorB, float Range);

    /**
     * Get closest actor from array
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Position")
    static AActor* GetClosestActor(AActor* Origin, const TArray<AActor*>& Actors);

    /**
     * Get actors within sphere
     */
    UFUNCTION(BlueprintCallable, Category="Adastrea|Utilities|Position")
    static TArray<AActor*> GetActorsWithinSphere(
        const UObject* WorldContextObject,
        FVector Center,
        float Radius,
        TSubclassOf<AActor> ActorClass = nullptr
    );

    // ====================
    // FACTION UTILITIES
    // ====================

    /**
     * Check if two factions are allied
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Faction")
    static bool AreFactionsAllied(UFactionDataAsset* FactionA, UFactionDataAsset* FactionB);

    /**
     * Check if two factions are hostile
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Faction")
    static bool AreFactionsHostile(UFactionDataAsset* FactionA, UFactionDataAsset* FactionB);

    /**
     * Get faction relationship value
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Faction")
    static int32 GetFactionRelationship(UFactionDataAsset* FactionA, UFactionDataAsset* FactionB);

    // ====================
    // COMBAT UTILITIES
    // ====================

    /**
     * Calculate damage after armor reduction
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Combat")
    static float CalculateDamageAfterArmor(float RawDamage, float ArmorValue, EDamageType DamageType);

    /**
     * Calculate optimal firing solution (lead target)
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Combat")
    static FVector CalculateLeadTarget(
        FVector ShooterLocation,
        FVector TargetLocation,
        FVector TargetVelocity,
        float ProjectileSpeed
    );

    /**
     * Check if target is in firing arc
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Combat")
    static bool IsInFiringArc(
        FVector ShooterLocation,
        FRotator ShooterRotation,
        FVector TargetLocation,
        float ArcAngle
    );

    // ====================
    // DATA VALIDATION
    // ====================

    /**
     * Validate Data Asset has required properties
     */
    UFUNCTION(BlueprintCallable, Category="Adastrea|Utilities|Validation")
    static bool ValidateDataAsset(UDataAsset* DataAsset, FString& OutErrorMessage);

    /**
     * Clamp value to valid range
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|Math")
    static float ClampValue(float Value, float Min, float Max);

    // ====================
    // STRING UTILITIES
    // ====================

    /**
     * Format large number with commas (1000 -> "1,000")
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|String")
    static FString FormatLargeNumber(int32 Number);

    /**
     * Format percentage (0.75 -> "75%")
     */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities|String")
    static FString FormatPercentage(float Percentage, int32 DecimalPlaces = 0);
};
```

### Implementation Example

```cpp
// Source/Adastrea/AdastreaFunctionLibrary.cpp
#include "AdastreaFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

float UAdastreaFunctionLibrary::GetDistance2D(AActor* ActorA, AActor* ActorB)
{
    if (!IsValid(ActorA) || !IsValid(ActorB))
    {
        return -1.0f;
    }

    FVector LocationA = ActorA->GetActorLocation();
    FVector LocationB = ActorB->GetActorLocation();
    LocationA.Z = 0.0f;
    LocationB.Z = 0.0f;

    return FVector::Dist(LocationA, LocationB);
}

bool UAdastreaFunctionLibrary::IsWithinRange(AActor* ActorA, AActor* ActorB, float Range)
{
    if (!IsValid(ActorA) || !IsValid(ActorB))
    {
        return false;
    }

    float Distance = FVector::Dist(ActorA->GetActorLocation(), ActorB->GetActorLocation());
    return Distance <= Range;
}

FVector UAdastreaFunctionLibrary::CalculateLeadTarget(
    FVector ShooterLocation,
    FVector TargetLocation,
    FVector TargetVelocity,
    float ProjectileSpeed)
{
    // Calculate interception point
    FVector ToTarget = TargetLocation - ShooterLocation;
    float Distance = ToTarget.Size();
    
    if (ProjectileSpeed <= 0.0f || Distance <= 0.0f)
    {
        return TargetLocation;
    }
    
    float TimeToHit = Distance / ProjectileSpeed;
    FVector LeadLocation = TargetLocation + (TargetVelocity * TimeToHit);
    
    return LeadLocation;
}
```

---

## Task 3: Data Asset Validation (4-5 hours)

### Objective
Add comprehensive validation to all Data Assets to catch configuration errors in editor.

### Implementation Pattern

```cpp
// In SpaceshipDataAsset.h
public:
#if WITH_EDITOR
    /**
     * Validate Data Asset properties
     * Called when asset is saved or validated in editor
     */
    virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
```

```cpp
// In SpaceshipDataAsset.cpp
#if WITH_EDITOR
EDataValidationResult USpaceshipDataAsset::IsDataValid(TArray<FText>& ValidationErrors)
{
    EDataValidationResult Result = EDataValidationResult::Valid;

    // Validate display name
    if (DisplayName.IsEmpty())
    {
        ValidationErrors.Add(FText::FromString(TEXT("Display Name is empty")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate stats are in valid range
    if (MaxHealth <= 0.0f)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Max Health must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    if (MaxSpeed < 0.0f)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Max Speed cannot be negative")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate references
    if (!ShipMesh)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Ship Mesh is not assigned")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate ratings are in expected range
    float CombatRating = GetCombatRating();
    if (CombatRating < 0.0f || CombatRating > 100.0f)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString(TEXT("Combat Rating out of range: {0} (expected 0-100)")),
            FText::AsNumber(CombatRating)
        ));
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}
#endif
```

### Data Assets to Validate

- [ ] `USpaceshipDataAsset`
- [ ] `UFactionDataAsset`
- [ ] `UPersonnelDataAsset`
- [ ] `UWeaponDataAsset`
- [ ] `UTradeItemDataAsset`
- [ ] `UQuestDataAsset`
- [ ] All other Data Assets

---

## Task 4: Automated Quality Checks (6-8 hours)

### Objective
Create CI/CD integration for automated code quality validation.

### 4.1 GitHub Actions Workflow

```yaml
# .github/workflows/code-quality.yml
name: Code Quality Checks

on:
  pull_request:
    branches: [ main, develop ]
  push:
    branches: [ main, develop ]

jobs:
  uproperty-check:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.10'
      
      - name: Run UPROPERTY Check
        run: |
          python Tools/check_uproperty.py --export uproperty_results.txt
          
      - name: Upload Results
        if: failure()
        uses: actions/upload-artifact@v3
        with:
          name: uproperty-results
          path: uproperty_results.txt

  naming-convention-check:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.10'
      
      - name: Run Naming Convention Check
        run: |
          python Tools/validate_naming.py --export naming_results.txt
```

### 4.2 Pre-commit Hooks

```bash
# .git/hooks/pre-commit
#!/bin/bash

# Run UPROPERTY check on staged files
echo "Running UPROPERTY validation..."
python Tools/check_uproperty.py

if [ $? -ne 0 ]; then
    echo "UPROPERTY validation failed. Please fix issues before committing."
    exit 1
fi

echo "Code quality checks passed!"
exit 0
```

### 4.3 Asset Naming Validator

```python
# Tools/validate_naming.py
"""
Validate asset naming conventions
"""
import os
import re
from pathlib import Path

ASSET_PATTERNS = {
    'Blueprint': r'^BP_.*\.uasset$',
    'Data Asset': r'^DA_.*\.uasset$',
    'Material': r'^M_.*\.uasset$',
    'Material Instance': r'^MI_.*\.uasset$',
    'Texture': r'^T_.*\.uasset$',
    'Static Mesh': r'^SM_.*\.uasset$',
    'Widget Blueprint': r'^WBP_.*\.uasset$',
}

def validate_content_naming(content_dir):
    """Validate content naming conventions"""
    issues = []
    
    for root, dirs, files in os.walk(content_dir):
        for file in files:
            if not file.endswith('.uasset'):
                continue
            
            # Check against patterns
            matched = False
            for asset_type, pattern in ASSET_PATTERNS.items():
                if re.match(pattern, file):
                    matched = True
                    break
            
            if not matched:
                issues.append({
                    'file': os.path.join(root, file),
                    'message': 'Does not match any naming convention'
                })
    
    return issues
```

---

## Success Criteria

### Interfaces

✅ **3 Core Interfaces**: `IDamageable`, `ITargetable`, `IFactionMember` implemented  
✅ **Implementation Coverage**: 90%+ of relevant classes implement appropriate interfaces  
✅ **Blueprint Integration**: All interfaces fully exposed and usable in Blueprints  
✅ **Documentation**: Interface usage guide complete

### Utility Library

✅ **50+ Utility Functions**: Comprehensive function library created  
✅ **Blueprint Exposure**: All functions BlueprintCallable  
✅ **Code Reduction**: 20%+ reduction in duplicated code  
✅ **Performance**: No performance regression from function calls

### Data Asset Validation

✅ **100% Coverage**: All Data Assets have validation  
✅ **Editor Integration**: Validation runs automatically on save  
✅ **Clear Messages**: Validation errors are descriptive and actionable  
✅ **No False Positives**: Validation logic is accurate

### Automated Checks

✅ **CI/CD Integration**: GitHub Actions running checks  
✅ **Pre-commit Hooks**: Local validation before commit  
✅ **Asset Validation**: Naming conventions enforced  
✅ **Zero Breaking Changes**: Checks don't break existing workflow

---

## Timeline

| Task | Time | Priority |
|------|------|----------|
| Interface Design & Implementation | 6-8 hours | High |
| Utility Function Library | 4-6 hours | Medium |
| Data Asset Validation | 4-5 hours | Medium |
| Automated Quality Checks | 6-8 hours | Medium |
| Testing & Documentation | 2-3 hours | High |
| **Total** | **22-30 hours** | - |

---

**Document Status**: ✅ Ready for Implementation  
**Prerequisites**: Phase 1 Complete ✅  
**Recommended**: Complete Phase 2 first for maximum benefit

For implementation questions, refer to:
- [Unreal Directive Best Practices](.github/instructions/unreal-directive-best-practices.md)
- [Phase 1 Report](UNREAL_DIRECTIVE_PHASE1_REPORT.md)
- [Phase 2 Guide](UNREAL_DIRECTIVE_PHASE2_GUIDE.md)
