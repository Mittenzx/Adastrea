# Code Style Guide

> **Complete Style Guide**: See [CODE_STYLE.md](../../CODE_STYLE.md) in the repository root for comprehensive coding standards.

## Quick Reference

This page provides a quick overview of Adastrea's coding standards. For complete details, examples, and edge cases, refer to the full guide.

## C++ Style Guidelines

### Naming Conventions

**Classes**:
- `UCLASS()` from UObject: `UClassName` (e.g., `USpaceshipDataAsset`)
- `ACLASS()` from AActor: `AClassName` (e.g., `ASpaceship`)
- Structs: `FStructName` (e.g., `FFactionTrait`)
- Enums: `EEnumName` (e.g., `EStationModuleGroup`)
- Interfaces: `IInterfaceName` (e.g., `IInteractable`)

**Variables**:
- Member variables: `PascalCase` (e.g., `MaxHealth`)
- Booleans: Prefix with `b` (e.g., `bIsAlive`)
- Local variables: `camelCase` or `PascalCase` (consistent within file)
- Constants: `kConstantName` or `ALL_CAPS`
- Always initialize pointers to `nullptr`

**Functions**:
- PascalCase with verb-noun (e.g., `GetHealth()`, `CalculateDamage()`)
- Use `const` for functions that don't modify state
- BlueprintNativeEvent: `_Implementation` suffix in .cpp

### File Structure

**Header (.h)**:
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "YourClass.generated.h"

// Forward declarations
class UOtherClass;

/**
 * Brief description
 * Detailed explanation
 */
UCLASS(BlueprintType)
class ADASTREA_API UYourClass : public UDataAsset
{
    GENERATED_BODY()

public:
    // Public interface
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic")
    FText DisplayName;

    UFUNCTION(BlueprintCallable, Category="Your Category")
    void DoSomething();

protected:
    // Protected members

private:
    // Private members
};
```

### UPROPERTY Best Practices

```cpp
// Designer-editable runtime values
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
float MaxHealth;

// Designer-set configuration
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
FText DisplayName;

// Read-only display
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Info")
int32 CurrentValue;

// With constraints
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats",
    meta=(ClampMin=0, ClampMax=100))
float Percentage;

// Multiline text
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Info",
    meta=(MultiLine=true))
FText Description;
```

### UFUNCTION Best Practices

```cpp
// Blueprint callable
UFUNCTION(BlueprintCallable, Category="Actions")
void PerformAction();

// Pure getter (no side effects)
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
float GetHealth() const;

// Blueprint overridable
UFUNCTION(BlueprintNativeEvent, Category="AI")
void DecideAction();

// Blueprint-only implementation
UFUNCTION(BlueprintImplementableEvent, Category="Events")
void OnDamaged();
```

## Blueprint Standards

### Naming Convention
- Blueprints: `BP_SystemName_Purpose`
- Widgets: `WBP_WidgetName`
- Data Assets: `DA_Type_Name`

**Examples**:
- `BP_Ship_Fighter`
- `WBP_HUD_Main`
- `DA_Faction_SolarisUnion`

### Organization
- Keep functions small (max 20-30 nodes)
- Use comment boxes for grouping
- Add tooltips to variables
- Organize with categories
- Follow left-to-right, top-to-bottom flow

## Asset Naming

```
BP_   - Blueprint
DA_   - Data Asset
M_    - Material
MI_   - Material Instance
T_    - Texture (_D diffuse, _N normal, _R roughness)
SM_   - Static Mesh
SK_   - Skeletal Mesh
A_    - Animation
S_    - Sound
P_    - Particle
WBP_  - Widget Blueprint
```

## Security Practices

### Code Security
- Never hardcode secrets
- Validate all inputs
- Sanitize file paths
- Use secure defaults
- Always initialize pointers to `nullptr`

### Data Asset Security
- Validate numeric values with min/max
- Sanitize string inputs
- Avoid exposing internal IDs
- Use bounded loops

## Documentation

### Code Comments
```cpp
/**
 * Calculate the combat effectiveness rating
 * 
 * @param Modifier Optional modifier to apply (default 1.0)
 * @return Combat rating from 0.0 to 100.0
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ratings")
float GetCombatRating(float Modifier = 1.0f) const;
```

### Documentation Style
- Use Doxygen-style comments (`/** */`)
- Explain WHY, not WHAT
- Add inline comments for complex logic
- Use TODO, FIXME, NOTE tags

## Common Patterns

### Data Asset Pattern
```cpp
UCLASS(BlueprintType)
class UMyDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic")
    FText DisplayName;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Functions")
    float CalculateValue() const;
};
```

### BlueprintNativeEvent Pattern
```cpp
// Header
UFUNCTION(BlueprintNativeEvent, Category="AI")
void DecideAction(AActor* Target);

// Implementation
void UMyClass::DecideAction_Implementation(AActor* Target)
{
    // Default C++ implementation
    // Can be overridden in Blueprint
}
```

### Trait System Pattern
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
```

## Performance Guidelines

- Profile before optimizing
- Cache expensive calculations
- Use object pooling for frequent spawns
- Stagger updates across frames
- Use LOD for distant objects
- Prefer const references over copying

## Testing Standards

Before submitting code:
- [ ] Compiles without errors or warnings
- [ ] Works in Unreal Editor
- [ ] Blueprint integration tested
- [ ] No crashes or critical bugs
- [ ] Documentation updated
- [ ] Example Data Assets still work

## Version Control

### Commit Messages
```
feat: Add new spaceship rating system
fix: Resolve faction relationship crash
docs: Update trading system guide
refactor: Simplify personnel skill logic
test: Add unit tests for combat system
chore: Update dependencies
```

### Git Best Practices
- Write clear, focused commits
- Keep commits atomic
- Don't commit build artifacts
- Use `.gitignore` properly
- Test before committing

## Complete Documentation

For detailed guidelines, examples, and edge cases:

📖 **[CODE_STYLE.md](../../CODE_STYLE.md)** - Complete code style guide

Also see:
- **[.github/copilot-instructions.md](../../.github/copilot-instructions.md)** - GitHub Copilot standards
- **[CONTRIBUTING.md](../../CONTRIBUTING.md)** - Contribution guidelines

## Related Documentation

- [Architecture Overview](Architecture.md)
- [Contributing Guidelines](Contributing.md)
- [Blueprint Development](../Blueprint-Development/Blueprint-Beginners-Guide.md)

---

**Consistent code style makes the project easier to read, maintain, and contribute to.**

*Last Updated: November 23, 2025*
