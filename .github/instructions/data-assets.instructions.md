# Data Asset System Custom Instructions

<!-- applyTo: Source/Adastrea/**/*DataAsset.cpp, Source/Adastrea/**/*DataAsset.h -->

## Data Asset Philosophy

All game content in Adastrea uses Blueprint Data Assets. This is the PRIMARY design pattern for the entire project. Data Assets provide:
- Designer-friendly content creation
- No C++ compilation required for content changes
- Easy to create variants and test iterations
- Version control friendly (text-based in Unreal)

## Data Asset Base Pattern

### Header File Structure

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "YourDataAsset.generated.h"

/**
 * Brief description of what this Data Asset represents
 * 
 * Usage:
 * 1. Create a Blueprint based on this class
 * 2. Configure properties in the Unreal Editor
 * 3. Reference the Data Asset from other Blueprints or C++ code
 */
UCLASS(BlueprintType)
class ADASTREA_API UYourDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Basic Info - Always include display name and description
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
    FText Description;

    // Stats and Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats", meta=(ClampMin=0, ClampMax=100))
    float SomeValue;

    // Blueprint-callable functions to calculate derived values
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Your System")
    float CalculateDerivedValue() const;
};
```

## UPROPERTY Best Practices for Data Assets

### Basic Properties

```cpp
// Text (localizable, use for user-facing strings)
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
FText DisplayName;

// String (use for internal identifiers)
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
FString InternalID;

// Multiline text for descriptions
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
FText Description;
```

### Numeric Properties with Constraints

```cpp
// Integer with range
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats", 
    meta=(ClampMin=1, ClampMax=100))
int32 Level;

// Float with UI slider
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats",
    meta=(UIMin=0.0, UIMax=1.0, ClampMin=0.0, ClampMax=1.0))
float Percentage;

// Float with tooltip
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats",
    meta=(ToolTip="Higher values increase effectiveness"))
float Effectiveness;
```

### Arrays and Collections

```cpp
// Array of scalars
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Configuration")
TArray<float> Values;

// Array of structs
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Configuration")
TArray<FYourStruct> Items;

// Array of references to other Data Assets
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Relations")
TArray<UOtherDataAsset*> RelatedAssets;
```

### References to Other Assets

```cpp
// Reference to another Data Asset
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Relations")
UOtherDataAsset* RelatedAsset;

// Reference to a Blueprint class
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Configuration")
TSubclassOf<AActor> SpawnClass;

// Reference to a static mesh
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visuals")
UStaticMesh* Mesh;
```

## Validation and Defaults

### Constructor Pattern

```cpp
UYourDataAsset::UYourDataAsset()
{
    // Set sensible defaults for all properties
    DisplayName = FText::FromString("New Item");
    SomeValue = 50.0f;  // Mid-range default
    Level = 1;          // Minimum level
}
```

### Validation Functions

```cpp
// Optional: Add validation in Data Asset
#if WITH_EDITOR
virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // Validate properties when changed in editor
    if (SomeValue < 0.0f)
    {
        SomeValue = 0.0f;
    }
}
#endif
```

## Blueprint-Callable Functions

### Pure Functions (No Side Effects)

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Your System")
float CalculateEffectiveness() const
{
    return BaseValue * Multiplier;
}

UFUNCTION(BlueprintCallable, BlueprintPure, Category="Your System")
int32 GetTotalCount() const
{
    return Items.Num();
}
```

### Complex Calculations

```cpp
UFUNCTION(BlueprintCallable, Category="Your System")
float CalculateComplexValue(float Input) const
{
    // Document complex calculations
    float Result = BaseValue;
    
    // Apply modifiers
    for (const FModifier& Mod : Modifiers)
    {
        Result *= Mod.Multiplier;
    }
    
    return Result;
}
```

## Common Data Asset Patterns

### Trait System

```cpp
USTRUCT(BlueprintType)
struct FTrait
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText TraitName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Modifier;
};

UCLASS()
class UEntityDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traits")
    TArray<FTrait> Traits;
};
```

### Relationship System

```cpp
USTRUCT(BlueprintType)
struct FRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEntityDataAsset* TargetEntity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=-100, ClampMax=100))
    int32 RelationshipValue;  // -100 to +100 scale
};
```

### Rating/Categorization

```cpp
// Group related properties in categories
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Core Stats")
float Health;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Stats")
float Damage;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mobility Stats")
float Speed;

// Provide rating functions
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ratings")
float GetCombatRating() const;
```

## Integration with YAML

Data Assets can be generated from YAML templates:
- Keep property names consistent with YAML field names
- Use FText for localizable strings
- Provide sensible defaults for optional YAML fields
- Validate loaded data in constructor or PostLoad

## Testing Data Assets

When creating or modifying Data Assets:
1. Create example Blueprint instances in Content/
2. Test all properties are editable in editor
3. Verify Blueprint-callable functions work
4. Test YAML import if applicable
5. Check backward compatibility with existing assets

## Common Pitfalls

- **Don't** make properties `BlueprintReadWrite` unless runtime modification is needed
- **Don't** forget to specify `Category` for organization
- **Don't** use bare pointers without null checks in functions
- **Don't** expose internal implementation details
- **Always** provide default values
- **Always** use constraints (ClampMin/ClampMax) for numeric ranges
- **Always** document complex properties with tooltips

## Performance Considerations

- Data Assets are loaded on-demand (good for memory)
- Cache frequently-accessed calculations
- Keep Data Asset classes lightweight
- Avoid deep inheritance hierarchies
- Use forward declarations in headers

---

Data Assets are the foundation of Adastrea's designer-friendly architecture. Every new game system should be built around Data Assets first, with Blueprint integration as the primary interface.
