# Non-Standard UE5 Practices in Adastrea

**Date**: 2025-12-27  
**Status**: Analysis Complete - Implementation Pending  
**Priority**: High - Affects scalability and future development

## Executive Summary

This document identifies non-standard Unreal Engine 5 practices in the Adastrea codebase that should be addressed to improve:
- **Scalability**: Better performance with large numbers of objects
- **Maintainability**: Clearer API boundaries and reduced complexity
- **UE5 Compatibility**: Modern UE5 patterns and future-proofing
- **Memory Safety**: Proper garbage collection tracking
- **Team Productivity**: Clearer code organization and patterns

**Source**: Analysis based on Epic Games coding standards, Unreal Directive best practices, and UE5.6 recommendations.

---

## Critical Issues (Fix Immediately)

### 1. Missing UPROPERTY on UObject Pointers

**Severity**: 🔴 **CRITICAL**  
**Impact**: Potential garbage collection crashes, memory leaks

**Problem**:
Some UObject* pointers lack UPROPERTY() macros, making them invisible to garbage collection.

**Examples Found**:
```cpp
// ❌ BAD: Private pointer without UPROPERTY
private:
    AActor* CachedTarget;  // NOT tracked by GC!
    UWeaponDataAsset* WeaponConfig;  // Could be collected!
```

**Correct Pattern**:
```cpp
// ✅ GOOD: All UObject pointers have UPROPERTY
private:
    UPROPERTY()
    AActor* CachedTarget;  // GC tracked
    
    UPROPERTY()
    UWeaponDataAsset* WeaponConfig;  // GC tracked
```

**Why It Matters**:
- UObject* without UPROPERTY() are NOT tracked by garbage collector
- Objects can be destroyed while still referenced
- Causes crashes when accessing deleted objects
- **RULE**: ALL UObject* pointers MUST have UPROPERTY(), even private ones

**Files Affected**: Multiple (need systematic audit)

**Action Required**:
1. Run automated check: `Tools/check_uproperty.py`
2. Add UPROPERTY() to all UObject* pointers
3. Add to CI validation to prevent future occurrences

---

### 2. Raw UObject Pointers Instead of TObjectPtr

**Severity**: 🟡 **HIGH**  
**Impact**: UE5.0+ compatibility, modern best practices

**Problem**:
Codebase uses raw `UObject*` pointers instead of modern `TObjectPtr<UObject>`.

**Current Pattern**:
```cpp
// ❌ OLD STYLE: Raw pointers
UPROPERTY(EditAnywhere, BlueprintReadOnly)
UWeaponDataAsset* WeaponData;

UPROPERTY()
AActor* CurrentTarget;
```

**Modern UE5 Pattern**:
```cpp
// ✅ MODERN: TObjectPtr (UE5.0+)
UPROPERTY(EditAnywhere, BlueprintReadOnly)
TObjectPtr<UWeaponDataAsset> WeaponData;

UPROPERTY()
TObjectPtr<AActor> CurrentTarget;
```

**Benefits**:
- Better compile-time type safety
- Improved debugging (easier to track references)
- Future-proof for UE5+ changes
- Automatic null checks in debug builds
- Better IDE integration

**Migration Strategy**:
1. Start with new code (use TObjectPtr going forward)
2. Phase 1: Migrate Data Assets
3. Phase 2: Migrate Components
4. Phase 3: Migrate Actors
5. Keep backward compatibility during transition

**References**:
- [TObjectPtr Documentation](https://docs.unrealengine.com/5.0/en-US/API/Runtime/CoreUObject/UObject/TObjectPtr/)
- Unreal Directive: "Use TObjectPtr for all UObject pointers in UE5+"

---

## High Priority Issues (Address Soon)

### 3. Over-Exposure to Blueprints

**Severity**: 🟡 **HIGH**  
**Impact**: API complexity, performance, maintainability

**Problem**:
Excessive Blueprint exposure creates unclear API boundaries.

**Current Stats**:
- 1,041 BlueprintCallable functions
- 537 EditAnywhere + BlueprintReadWrite properties
- ~90% of API exposed to Blueprints

**Recommendation**:
- Target 10-20% Blueprint exposure
- Expose only designer-facing functionality
- Keep internal implementation private

**Examples**:

```cpp
// ❌ BAD: Internal calculation exposed unnecessarily
UFUNCTION(BlueprintCallable, Category="Internal")
float CalculateInternalStatModifier() const;  // Designers don't need this

// ✅ GOOD: Only expose designer-relevant functions
UFUNCTION(BlueprintCallable, Category="Weapon")
float GetTotalDamage() const;  // Designers need this

private:
    // Keep internal
    float CalculateInternalStatModifier() const;
```

**Guidelines for Blueprint Exposure**:

1. **BlueprintCallable**: Only for actions designers will trigger
2. **BlueprintReadOnly**: For status/config values designers need to see
3. **BlueprintReadWrite**: ONLY when runtime modification is required
4. **Private**: Everything else (internal state, helpers, calculations)

**Action Items**:
1. Audit all BlueprintCallable functions
2. Create "Internal" or private categories for helpers
3. Document "Blueprint API" vs "Internal API"
4. Update `.github/instructions/` guidelines

---

### 4. EditAnywhere + BlueprintReadWrite Over-Usage

**Severity**: 🟡 **HIGH**  
**Impact**: Unintended runtime modifications, state bugs

**Problem**:
537 properties allow BOTH editor configuration AND runtime Blueprint modification.

**Issue**:
Most configuration properties should be set once in editor, not modified at runtime.

**Examples**:

```cpp
// ❌ BAD: Config property modifiable at runtime
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
float BaseDamage;  // Could be changed mid-combat by mistake

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
float MaxSpeed;  // Should not change at runtime

// ✅ GOOD: Config is read-only, use functions for valid modifications
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
float BaseDamage;  // Set in editor, read in Blueprint

UFUNCTION(BlueprintCallable, Category="Weapon")
void ApplyDamageModifier(float Multiplier);  // Controlled modification
```

**Pattern to Follow**:

```cpp
// Configuration (designer sets in editor, never changes)
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
float MaxHullStrength;

// Runtime State (changes during gameplay, read by BP)
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Status")
float CurrentHullStrength;

// Controlled Modification (only when intentional runtime changes needed)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay")
bool bShieldsEnabled;  // Toggle at runtime is valid
```

**Action Items**:
1. Audit all EditAnywhere + BlueprintReadWrite properties
2. Change to BlueprintReadOnly unless runtime modification is intentional
3. Create setter functions for controlled modifications
4. Document when BlueprintReadWrite is appropriate

---

### 5. Insufficient TWeakObjectPtr Usage

**Severity**: 🟡 **MEDIUM**  
**Impact**: Memory management, optional references

**Problem**:
Only 2 uses of TWeakObjectPtr found. Many optional/temporary references use raw pointers.

**When to Use TWeakObjectPtr**:
- Optional references (may be null)
- References you don't own (could be destroyed)
- Cached references (may become stale)
- Temporary tracking (actor may despawn)

**Examples**:

```cpp
// ❌ BAD: Raw pointer to actor we don't own
UPROPERTY()
AActor* LastTarget;  // Could be destroyed, dangling pointer!

// ✅ GOOD: Weak pointer for optional reference
UPROPERTY()
TWeakObjectPtr<AActor> LastTarget;  // Safe, returns null if destroyed

// Usage
if (LastTarget.IsValid())
{
    AActor* Target = LastTarget.Get();
    // Use Target...
}
```

**Pattern Guide**:

```cpp
// Use raw TObjectPtr for owned/required references
UPROPERTY()
TObjectPtr<UStaticMeshComponent> MeshComponent;  // We own this

// Use TWeakObjectPtr for optional/non-owned references  
UPROPERTY()
TWeakObjectPtr<AActor> CurrentTarget;  // May be destroyed

// Use TSoftObjectPtr for lazy-loaded assets
UPROPERTY(EditAnywhere)
TSoftObjectPtr<UTexture2D> IconTexture;  // Loaded on demand
```

**Action Items**:
1. Identify all optional AActor* references
2. Convert to TWeakObjectPtr<AActor>
3. Add .IsValid() checks before usage
4. Document pattern in style guide

---

### 6. Tick-Heavy Components

**Severity**: 🟡 **MEDIUM**  
**Impact**: Performance, scalability

**Problem**:
Multiple components use Tick for periodic updates that don't need frame-rate precision.

**Current Pattern**:
```cpp
// ❌ BAD: Tick every frame for occasional checks
void UCombatHealthComponent::TickComponent(float DeltaTime, ...)
{
    // Check for damage every frame - wasteful!
    CheckForNearbyEnemies();
    UpdateShieldRecharge(DeltaTime);
}
```

**Better Pattern**:
```cpp
// ✅ GOOD: Use timers for periodic updates
void UCombatHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Update once per second instead of 60 times per second
    GetWorld()->GetTimerManager().SetTimer(
        UpdateTimerHandle,
        this,
        &UCombatHealthComponent::PeriodicUpdate,
        1.0f,  // Every second
        true   // Loop
    );
}

void UCombatHealthComponent::PeriodicUpdate()
{
    CheckForNearbyEnemies();
    UpdateShieldRecharge(1.0f);  // Fixed delta time
}
```

**AI Update Staggering**:
```cpp
// ✅ GOOD: Stagger AI updates to spread CPU load
void UNPCLogicBase::InitializeAI()
{
    // Random offset to stagger updates
    float RandomOffset = FMath::RandRange(0.0f, UpdateInterval);
    
    GetWorld()->GetTimerManager().SetTimer(
        AIUpdateTimer,
        this,
        &UNPCLogicBase::PerformAIUpdate,
        UpdateInterval,
        true,
        RandomOffset  // Initial delay
    );
}
```

**Guidelines**:
- Frame-rate updates: Use Tick (player input, camera, critical gameplay)
- Per-second updates: Use 1.0f timers (AI, resource checks)
- Slow updates: Use 2-5 second timers (economy, distant actors)
- Stagger AI updates to avoid CPU spikes

**Action Items**:
1. Profile Tick usage with Unreal Insights
2. Identify components with low-frequency updates
3. Convert to timer-based updates
4. Implement AI update staggering

---

## Medium Priority Issues (Address in Refactor)

### 7. Missing const Correctness

**Severity**: 🟢 **MEDIUM**  
**Impact**: Code clarity, optimization opportunities

**Problem**:
Many getter functions that don't modify state lack const qualifier.

**Examples**:

```cpp
// ❌ INCONSISTENT: Some getters const, some not
UFUNCTION(BlueprintCallable, BlueprintPure)
float GetMaxHealth();  // Should be const

UFUNCTION(BlueprintCallable, BlueprintPure)
FText GetDisplayName();  // Should be const

// ✅ GOOD: All non-modifying functions are const
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
float GetMaxHealth() const;

UFUNCTION(BlueprintCallable, BlueprintPure, Category="Info")
FText GetDisplayName() const;
```

**Rule**:
- ALL functions marked BlueprintPure MUST be const
- ALL getter functions SHOULD be const
- Functions that don't modify state SHOULD be const

**Benefits**:
- Compiler can optimize better
- Clear intent (this function doesn't change state)
- Can be called on const references
- Better code documentation

**Action Items**:
1. Audit all BlueprintPure functions
2. Add const to non-modifying functions
3. Update `.github/instructions/` to mandate const

---

### 8. Inconsistent Component ClassGroup

**Severity**: 🟢 **MEDIUM**  
**Impact**: Editor organization, usability

**Problem**:
Some components specify ClassGroup, others don't.

**Examples**:

```cpp
// ❌ INCONSISTENT: Some have ClassGroup
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class UWeaponComponent : public UActorComponent { };

// Others don't
UCLASS(meta=(BlueprintSpawnableComponent))
class UCargoComponent : public UActorComponent { };
```

**Standard Pattern**:
```cpp
// ✅ GOOD: All components have ClassGroup
UCLASS(ClassGroup=(Trading), meta=(BlueprintSpawnableComponent))
class UCargoComponent : public UActorComponent { };

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class UWeaponComponent : public UActorComponent { };

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class UNPCLogicComponent : public UActorComponent { };
```

**ClassGroup Categories**:
- `Combat` - Weapons, health, shields
- `Trading` - Cargo, economy, markets
- `AI` - Logic, behavior, pathfinding
- `Navigation` - Movement, waypoints
- `UI` - HUD, widgets, displays
- `Exploration` - Scanning, discovery
- `Custom` - Misc/utility

**Action Items**:
1. Add ClassGroup to all component UCLASS declarations
2. Standardize category names
3. Update component creation templates

---

### 9. UPROPERTY Without Specifiers

**Severity**: 🟢 **LOW**  
**Impact**: Code clarity, functionality

**Problem**:
Found several UPROPERTY() declarations with no specifiers.

**Examples**:
```cpp
// ❌ UNCLEAR: What is this for?
UPROPERTY()
TArray<AActor*> CachedActors;

UPROPERTY()
float InternalState;
```

**Better Pattern**:
```cpp
// ✅ CLEAR: Purpose is documented via specifiers
UPROPERTY(Transient)  // Runtime cache, don't serialize
TArray<TObjectPtr<AActor>> CachedActors;

UPROPERTY()  // Just for GC tracking, truly internal
float InternalState;
```

**When Bare UPROPERTY() is OK**:
- Pure GC tracking (UObject* pointer with no other purpose)
- Truly internal state (not serialized, not visible, not accessible)

**When to Add Specifiers**:
- Transient - Runtime state, don't save
- VisibleAnywhere - Show in editor for debugging
- BlueprintReadOnly - Blueprint can read
- Category - Organize in editor

**Action Items**:
1. Review bare UPROPERTY() declarations
2. Add appropriate specifiers
3. Document intent

---

### 10. Object Pooling Not Fully Utilized

**Severity**: 🟢 **MEDIUM**  
**Impact**: Performance, GC pressure

**Problem**:
ProjectilePoolComponent exists but may not be consistently used.

**Current State**:
```cpp
// Projectile.cpp
void AProjectile::DestroyProjectile()
{
    // Has pooling support
    ResetToPoolState();
}
```

**Need to Verify**:
1. Are all weapon components using the pool?
2. Are projectiles properly returned to pool?
3. Is pool size configured appropriately?
4. Are there other candidates for pooling? (VFX, particles)

**Pooling Pattern**:
```cpp
// ✅ GOOD: Consistent pooling usage
void UWeaponComponent::Fire()
{
    if (ProjectilePool)
    {
        AProjectile* Projectile = ProjectilePool->AcquireProjectile();
        // Configure and fire
        // Pool automatically recycles after lifetime
    }
}
```

**Other Pooling Candidates**:
- Particle effects (explosions, muzzle flashes)
- Temporary UI elements (damage numbers)
- Audio sources (for frequent sounds)
- Decals (impacts, scorch marks)

**Action Items**:
1. Audit all projectile spawning code
2. Verify pool usage consistency
3. Profile GC with/without pooling
4. Identify other pooling candidates

---

## Low Priority / Nice to Have

### 11. Enum Class Usage

**Current**: Mix of UENUM() regular enums and enum class  
**Recommendation**: Prefer enum class for type safety

```cpp
// ✅ BETTER: Type-safe enum class
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Energy,
    Projectile,
    Missile
};
```

### 12. Forward Declarations

**Current**: Good usage in most places  
**Recommendation**: Continue pattern, expand where applicable

### 13. Documentation Coverage

**Current**: Good coverage on public APIs  
**Recommendation**: Add more implementation comments for complex algorithms

---

## Implementation Priority

### Phase 1: Critical (Week 1-2)
1. Add UPROPERTY() to all UObject* pointers
2. Run automated validation (check_uproperty.py)
3. Add CI validation for future code

### Phase 2: High Priority (Week 3-4)
1. Audit and reduce Blueprint exposure
2. Change EditAnywhere+BlueprintReadWrite to BlueprintReadOnly where appropriate
3. Add TWeakObjectPtr for optional references

### Phase 3: Medium Priority (Week 5-8)
1. Begin TObjectPtr migration (new code first)
2. Convert Tick to timers where appropriate
3. Add const to getter functions
4. Standardize ClassGroup usage

### Phase 4: Refactor (Ongoing)
1. Continue TObjectPtr migration
2. Improve documentation
3. Address remaining issues
4. Update coding guidelines

---

## Validation Tools

### Automated Checks

1. **check_uproperty.py**: Validates all UObject* have UPROPERTY
2. **check_null_safety.py**: Checks for null pointer access
3. **validate_naming.py**: Enforces naming conventions

### Manual Reviews

1. Blueprint API review (quarterly)
2. Performance profiling (before major releases)
3. Code review checklist updates

### CI Integration

Add to GitHub Actions:
```yaml
- name: Validate UProperty Usage
  run: python Tools/check_uproperty.py

- name: Check Null Safety
  run: python Tools/check_null_safety.py
```

---

## References

### Epic Games Official
- [Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine)
- [Gameplay Architecture](https://docs.unrealengine.com/5.6/en-US/gameplay-architecture-in-unreal-engine/)
- [TObjectPtr Documentation](https://docs.unrealengine.com/5.0/en-US/API/Runtime/CoreUObject/UObject/TObjectPtr/)

### Unreal Directive
- [Best Practices](https://unrealdirective.com/)
- [Memory Management](https://unrealdirective.com/articles/memory-management)
- [Performance Optimization](https://unrealdirective.com/articles/performance)

### Project Specific
- `.github/instructions/unreal-directive-best-practices.md`
- `.github/instructions/data-assets.instructions.md`
- `CODE_STYLE.md`

---

## Conclusion

Addressing these non-standard practices will:
- ✅ Improve garbage collection safety
- ✅ Modernize codebase for UE5+
- ✅ Reduce API complexity
- ✅ Improve performance and scalability
- ✅ Better team productivity
- ✅ Easier onboarding for new developers

**Next Step**: Review this document with team, prioritize changes, begin Phase 1 implementation.

---

**Maintained by**: Adastrea Development Team  
**Last Updated**: 2025-12-27  
**Version**: 1.0
