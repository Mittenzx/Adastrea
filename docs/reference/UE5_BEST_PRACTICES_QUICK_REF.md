# UE5 Best Practices Quick Reference

**Quick guide for common UE5 patterns in Adastrea**

---

## UPROPERTY Patterns

### All UObject Pointers MUST Have UPROPERTY
```cpp
// ❌ NEVER DO THIS
private:
    AActor* CachedTarget;  // CRASH RISK - Not GC tracked!

// ✅ ALWAYS DO THIS
private:
    UPROPERTY()
    TObjectPtr<AActor> CachedTarget;  // Safe - GC tracked
```

### Use TObjectPtr in UE5
```cpp
// ❌ OLD STYLE
UPROPERTY()
UDataAsset* MyData;

// ✅ MODERN UE5
UPROPERTY()
TObjectPtr<UDataAsset> MyData;
```

### Configuration vs Runtime Properties
```cpp
// Configuration (set in editor only - most common case)
UPROPERTY(EditAnywhere, BlueprintReadOnly)
float MaxHealth;

// Runtime state (read-only, changes during gameplay)
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
float CurrentHealth;

// Runtime modifiable (only if Blueprints truly need write access - rare)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runtime")
bool bShieldsActive;
```

> **Best Practice (PR #370)**: Component configuration properties should use `BlueprintReadOnly`. Properties set in editor should not be modifiable at runtime unless there's a specific design need.

### Optional References
```cpp
// Use TWeakObjectPtr for optional/non-owned references
UPROPERTY()
TWeakObjectPtr<AActor> OptionalTarget;

// Always check validity
if (OptionalTarget.IsValid())
{
    AActor* Target = OptionalTarget.Get();
    // Use Target...
}
```

---

## UFUNCTION Patterns

### const Correctness
```cpp
// ✅ All getters should be const
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
float GetMaxHealth() const;

UFUNCTION(BlueprintCallable, BlueprintPure, Category="Info")
FText GetDisplayName() const;
```

### Blueprint Exposure Guidelines
```cpp
// ✅ Expose only designer-facing functions
UFUNCTION(BlueprintCallable, Category="Weapon")
void Fire();  // Designers need this

// ✅ Keep internal helpers private
private:
    void CalculateTrajectory();  // Internal, no BlueprintCallable
```

---

## UCLASS Patterns

### Components Always Have ClassGroup
```cpp
// ✅ All components should specify ClassGroup
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class UWeaponComponent : public UActorComponent
{
    GENERATED_BODY()
};
```

### ClassGroup Categories
- `Combat` - Weapons, health, shields
- `Trading` - Cargo, economy
- `AI` - Logic, behavior
- `Navigation` - Movement
- `Exploration` - Scanning, discovery

---

## Performance Patterns

### Use Timers Instead of Tick
```cpp
// ❌ BAD: Tick every frame
void TickComponent(float DeltaTime, ...)
{
    SlowPeriodicCheck();  // Don't need frame-rate precision
}

// ✅ GOOD: Use timer
void BeginPlay()
{
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle, this, &UMyComponent::SlowPeriodicCheck,
        1.0f, true  // Every second, looping
    );
}
```

### Object Pooling for Frequent Spawning
```cpp
// ✅ Use pooling for projectiles, VFX, etc.
AProjectile* Projectile = ProjectilePool->AcquireProjectile();
// Use projectile...
// Automatically returned to pool after lifetime
```

### Stagger AI Updates
```cpp
// ✅ Avoid all AIs updating same frame
void InitializeAI()
{
    float RandomOffset = FMath::RandRange(0.0f, UpdateInterval);
    GetWorld()->GetTimerManager().SetTimer(
        AITimer, this, &UAI::Update,
        UpdateInterval, true, RandomOffset
    );
}
```

---

## Common Mistakes to Avoid

### ❌ Don't: Over-expose to Blueprints
```cpp
// TOO MUCH - Internal helpers don't need BP exposure
UFUNCTION(BlueprintCallable)
float CalculateInternalModifier() const;
```

### ❌ Don't: Make Everything BlueprintReadWrite
```cpp
// Config should be read-only at runtime
UPROPERTY(EditAnywhere, BlueprintReadWrite)  // Wrong
float BaseDamage;
```

### ❌ Don't: Skip UPROPERTY on Pointers
```cpp
// CRASH RISK - Not tracked by GC
private:
    UObject* SomeObject;  // Missing UPROPERTY()!
```

### ❌ Don't: Use Tick for Infrequent Updates
```cpp
// Wasteful - doesn't need every frame
void Tick(float DeltaTime)
{
    OncePerSecondCheck();
}
```

---

## Checklist for New Code

When creating new components/actors:

- [ ] All UObject* pointers have UPROPERTY()
- [ ] Used TObjectPtr<> instead of raw pointers
- [ ] Config properties are BlueprintReadOnly
- [ ] Components have ClassGroup specified
- [ ] Getter functions are const
- [ ] Only expose necessary functions to BP
- [ ] Use timers for periodic updates
- [ ] Added null checks for optional references
- [ ] Documented public API with comments

---

## Quick Links

- Full Guide: [NON_STANDARD_UE5_PRACTICES.md](../development/NON_STANDARD_UE5_PRACTICES.md)
- Unreal Directive: [unreal-directive-best-practices.md](../../.github/instructions/unreal-directive-best-practices.md)
- Code Style: [CODE_STYLE.md](../../CODE_STYLE.md)

---

**Last Updated**: 2025-12-27  
**Version**: 1.0
