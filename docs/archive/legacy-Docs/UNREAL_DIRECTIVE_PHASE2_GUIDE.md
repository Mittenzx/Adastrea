# Unreal Directive Implementation - Phase 2 Implementation Guide

**Phase**: 2 - Performance Optimizations (High Priority)  
**Status**: 📋 Planning / Ready to Implement  
**Estimated Time**: 18-24 hours  
**Prerequisites**: Phase 1 Complete ✅

---

## Overview

Phase 2 focuses on implementing key performance optimizations to improve frame rates, reduce garbage collection pressure, and enhance scalability. These optimizations will enable the game to maintain 60 FPS with 100+ ships in active combat scenarios.

---

## Task 1: Object Pooling System (8-10 hours)

### Objective
Implement object pooling for frequently spawned/destroyed objects to reduce GC pressure and allocation overhead.

### Target Systems

#### 1.1 Visual Effects Pooling

**Current State**:
- `UCombatVFXComponent` spawns Niagara components for weapon effects
- Each spawn creates a new component (GC overhead)
- High-frequency spawning during combat (10-50+ effects per second)

**Files to Modify**:
- `Source/Adastrea/Combat/CombatVFXComponent.h`
- `Source/Adastrea/Combat/CombatVFXComponent.cpp`

**Implementation Steps**:

1. **Create Niagara Component Pool**
```cpp
// In CombatVFXComponent.h
protected:
    /** Pool of reusable Niagara components */
    UPROPERTY()
    TArray<UNiagaraComponent*> NiagaraComponentPool;
    
    /** Active Niagara components currently in use */
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveNiagaraComponents;
    
    /** Pool size for each effect type */
    UPROPERTY(EditAnywhere, Category="Performance")
    int32 ComponentPoolSize = 20;
    
    /** Get or create a pooled component */
    UNiagaraComponent* GetPooledNiagaraComponent();
    
    /** Return component to pool */
    void ReturnNiagaraComponentToPool(UNiagaraComponent* Component);
    
    /** Initialize component pool */
    void InitializeComponentPool();
```

2. **Implement Pool Management**
```cpp
// In CombatVFXComponent.cpp
void UCombatVFXComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponentPool();
    // ... rest of BeginPlay
}

void UCombatVFXComponent::InitializeComponentPool()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Pre-allocate pool
    NiagaraComponentPool.Reserve(ComponentPoolSize);
    ActiveNiagaraComponents.Reserve(ComponentPoolSize);
    
    for (int32 i = 0; i < ComponentPoolSize; i++)
    {
        UNiagaraComponent* Component = NewObject<UNiagaraComponent>(GetOwner());
        if (Component)
        {
            Component->RegisterComponent();
            Component->SetAutoActivate(false);
            Component->SetVisibility(false);
            NiagaraComponentPool.Add(Component);
        }
    }
}

UNiagaraComponent* UCombatVFXComponent::GetPooledNiagaraComponent()
{
    // Try to get from pool
    if (NiagaraComponentPool.Num() > 0)
    {
        UNiagaraComponent* Component = NiagaraComponentPool.Pop();
        ActiveNiagaraComponents.Add(Component);
        return Component;
    }
    
    // Pool exhausted - create new (logged for monitoring)
    UE_LOG(LogAdastreaCombat, Warning, 
        TEXT("Niagara component pool exhausted, creating new component"));
    
    UNiagaraComponent* Component = NewObject<UNiagaraComponent>(GetOwner());
    if (Component)
    {
        Component->RegisterComponent();
        ActiveNiagaraComponents.Add(Component);
    }
    return Component;
}

void UCombatVFXComponent::ReturnNiagaraComponentToPool(UNiagaraComponent* Component)
{
    if (!Component)
    {
        return;
    }
    
    // Deactivate and hide
    Component->Deactivate();
    Component->SetVisibility(false);
    Component->ResetSystem();
    
    // Remove from active list
    ActiveNiagaraComponents.Remove(Component);
    
    // Return to pool if space available
    if (NiagaraComponentPool.Num() < ComponentPoolSize)
    {
        NiagaraComponentPool.Add(Component);
    }
    else
    {
        // Pool full - destroy excess
        Component->DestroyComponent();
    }
}
```

3. **Update Effect Spawning**
```cpp
// Modify PlayWeaponFireEffect to use pooling
UNiagaraComponent* UCombatVFXComponent::PlayWeaponFireEffect(
    EWeaponType WeaponType,
    FVector MuzzleLocation,
    FRotator MuzzleRotation,
    FVector TargetLocation)
{
    // ... existing checks ...
    
    // Get pooled component instead of spawning new
    UNiagaraComponent* SpawnedEffect = GetPooledNiagaraComponent();
    if (!SpawnedEffect)
    {
        return nullptr;
    }
    
    // Configure and activate
    SpawnedEffect->SetWorldLocation(MuzzleLocation);
    SpawnedEffect->SetWorldRotation(MuzzleRotation);
    SpawnedEffect->SetAsset(/* appropriate system */);
    SpawnedEffect->Activate();
    SpawnedEffect->SetVisibility(true);
    
    return SpawnedEffect;
}
```

4. **Update Cleanup Logic**
```cpp
void UCombatVFXComponent::CleanupFinishedEffects()
{
    // Iterate backwards for safe removal
    for (int32 i = ActiveNiagaraComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Component = ActiveNiagaraComponents[i];
        if (Component && !Component->IsActive())
        {
            ReturnNiagaraComponentToPool(Component);
        }
    }
}
```

#### 1.2 Audio Component Pooling

**Current State**:
- `UAdastreaAudioComponent` creates audio components on-demand
- Already has some pooling infrastructure (`ActiveAudioComponents` array)

**Files to Modify**:
- `Source/Adastrea/Public/Audio/AdastreaAudioComponent.h`
- `Source/Adastrea/Audio/AdastreaAudioComponent.cpp`

**Implementation Steps**:

1. Review existing `GetAvailableAudioComponent()` implementation
2. Verify pool size is adequate (current limit)
3. Add metrics to monitor pool usage
4. Add configuration for pool size

### Expected Performance Improvements

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| GC Pressure (Combat) | High | Low | -50 to -70% |
| Component Spawn Time | ~2ms | ~0.2ms | -90% |
| Frame Drops | Occasional | Rare | Significant |
| Memory Allocations | High churn | Stable | -60% |

---

## Task 2: Tick Optimization (6-8 hours)

### Objective
Reduce Tick overhead by converting appropriate systems to timer-based updates and staggering AI updates.

### 2.1 Timer Conversion Candidates

**Identify Components for Timer Conversion**:
```bash
# Find all components with Tick
grep -r "void.*TickComponent" Source/Adastrea --include="*.cpp" -l
```

**Priority Candidates**:
1. **AI Components**: Update once per second instead of every frame
2. **Low-frequency Systems**: Trading, diplomacy, reputation
3. **UI Update Components**: HUD elements that don't need 60Hz updates

**Example Conversion**: `UPersonnelLogic`

**Before** (Every Frame):
```cpp
void UPersonnelLogic::TickComponent(float DeltaTime, ...)
{
    Super::TickComponent(DeltaTime, ...);
    
    // Decision making logic
    EvaluateSituation();
    MakeDecisions();
}
```

**After** (Timer-based):
```cpp
void UPersonnelLogic::BeginPlay()
{
    Super::BeginPlay();
    
    // Update decisions once per second
    GetWorld()->GetTimerManager().SetTimer(
        DecisionTimerHandle,
        this,
        &UPersonnelLogic::UpdateDecisions,
        1.0f,  // Update interval
        true   // Loop
    );
    
    // Disable tick
    SetComponentTickEnabled(false);
}

void UPersonnelLogic::UpdateDecisions()
{
    EvaluateSituation();
    MakeDecisions();
}
```

### 2.2 AI Update Staggering

**Problem**: All AI updates run on same frame = frame spike

**Solution**: Stagger updates across multiple frames

```cpp
// In UFactionLogic or AI manager
void UAdastreaGameInstance::BeginPlay()
{
    Super::BeginPlay();
    
    // Stagger AI updates
    TArray<UNPCLogicBase*> AllAIs = GetAllAIs();
    
    float StaggerInterval = 1.0f / AllAIs.Num();
    float CurrentDelay = 0.0f;
    
    for (UNPCLogicBase* AI : AllAIs)
    {
        if (AI)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                AI,
                &UNPCLogicBase::UpdateAI,
                1.0f,          // Repeat every second
                true,          // Loop
                CurrentDelay   // Initial delay for staggering
            );
            
            CurrentDelay += StaggerInterval;
        }
    }
}
```

### 2.3 Tick Priority Optimization

**Review and Optimize Tick Groups**:
```cpp
// High priority (needs to run early)
PrimaryComponentTick.TickGroup = TG_PrePhysics;

// Normal priority
PrimaryComponentTick.TickGroup = TG_DuringPhysics;

// Low priority (can wait)
PrimaryComponentTick.TickGroup = TG_PostPhysics;

// Very low priority
PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
```

### Expected Performance Improvements

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| Tick Count | ~1000 | ~600-700 | -30 to -40% |
| AI Frame Spikes | Common | Rare | Significant |
| CPU Usage | High | Moderate | -20 to -30% |

---

## Task 3: Calculation Caching (4-6 hours)

### Objective
Cache expensive calculations in Data Assets to avoid redundant computation.

### 3.1 Spaceship Rating Caching

**Current State**:
- `USpaceshipDataAsset` calculates ratings on every call
- Ratings involve complex weighted sums
- Called frequently (UI, AI decision making, comparisons)

**Files to Modify**:
- `Source/Adastrea/Ships/SpaceshipDataAsset.h`
- `Source/Adastrea/Ships/SpaceshipDataAsset.cpp`

**Implementation**:

```cpp
// In SpaceshipDataAsset.h
protected:
    /** Cached rating values */
    UPROPERTY(Transient)
    mutable float CachedCombatRating;
    
    UPROPERTY(Transient)
    mutable float CachedMobilityRating;
    
    UPROPERTY(Transient)
    mutable float CachedUtilityRating;
    
    UPROPERTY(Transient)
    mutable float CachedOverallRating;
    
    /** Dirty flag for cache invalidation */
    UPROPERTY(Transient)
    mutable bool bRatingsCacheDirty;
    
    /** Invalidate cached ratings */
    void InvalidateRatingsCache();
    
    /** Calculate and cache all ratings */
    void UpdateRatingsCache() const;

public:
#if WITH_EDITOR
    /** Invalidate cache when properties change */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
```

```cpp
// In SpaceshipDataAsset.cpp
USpaceshipDataAsset::USpaceshipDataAsset()
{
    // ... existing initialization ...
    bRatingsCacheDirty = true;
    CachedCombatRating = 0.0f;
    CachedMobilityRating = 0.0f;
    CachedUtilityRating = 0.0f;
    CachedOverallRating = 0.0f;
}

float USpaceshipDataAsset::GetCombatRating() const
{
    if (bRatingsCacheDirty)
    {
        UpdateRatingsCache();
    }
    return CachedCombatRating;
}

void USpaceshipDataAsset::UpdateRatingsCache() const
{
    // Calculate all ratings at once
    CachedCombatRating = CalculateCombatRatingInternal();
    CachedMobilityRating = CalculateMobilityRatingInternal();
    CachedUtilityRating = CalculateUtilityRatingInternal();
    CachedOverallRating = CalculateOverallRatingInternal();
    
    bRatingsCacheDirty = false;
}

void USpaceshipDataAsset::InvalidateRatingsCache()
{
    bRatingsCacheDirty = true;
}

#if WITH_EDITOR
void USpaceshipDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // Invalidate cache when any stat changes
    if (PropertyChangedEvent.Property)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();
        if (PropertyName != NAME_None)
        {
            InvalidateRatingsCache();
        }
    }
}
#endif
```

### 3.2 Faction Relationship Caching

**Similar pattern for**:
- `UFactionDataAsset::GetCategoryModifierTotal()`
- `UPersonnelDataAsset::GetSkillLevel()`
- Any other expensive Data Asset calculations

### Expected Performance Improvements

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| Rating Calculation Time | ~1ms | ~0.1ms | -90% |
| UI Update Performance | Laggy | Smooth | Significant |
| AI Decision Speed | Moderate | Fast | 10x faster |

---

## Testing Strategy

### Unit Tests

Create performance benchmarks:

```cpp
// Test: Object Pooling Performance
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNiagaraPoolingBenchmark, 
    "Adastrea.Performance.NiagaraPooling", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter);

bool FNiagaraPoolingBenchmark::RunTest(const FString& Parameters)
{
    // Benchmark spawn time
    const int32 SpawnCount = 100;
    
    // Without pooling
    double StartTime = FPlatformTime::Seconds();
    for (int32 i = 0; i < SpawnCount; i++)
    {
        UNiagaraComponent* Comp = NewObject<UNiagaraComponent>();
        // ... spawn and destroy
    }
    double WithoutPoolingTime = FPlatformTime::Seconds() - StartTime;
    
    // With pooling
    StartTime = FPlatformTime::Seconds();
    for (int32 i = 0; i < SpawnCount; i++)
    {
        UNiagaraComponent* Comp = GetPooledComponent();
        ReturnToPool(Comp);
    }
    double WithPoolingTime = FPlatformTime::Seconds() - StartTime;
    
    float Improvement = ((WithoutPoolingTime - WithPoolingTime) / WithoutPoolingTime) * 100.0f;
    TestTrue(TEXT("Pooling should be faster"), WithPoolingTime < WithoutPoolingTime);
    
    return true;
}
```

### Integration Tests

**Combat Stress Test**:
- Spawn 100+ ships
- Full combat engagement
- Monitor:
  - Frame rate (target: 60 FPS)
  - GC frequency (target: <20 per hour)
  - Memory usage (target: <10% growth per hour)

**AI Performance Test**:
- 200+ NPCs with AI
- Staggered updates
- Monitor tick count and frame times

### Profiling

**Before Phase 2**:
```
stat fps
stat game
stat gc
stat memory
```

**After Phase 2**:
- Compare all metrics
- Document improvements
- Identify remaining bottlenecks

---

## Success Criteria

### Quantitative Metrics

✅ **GC Pressure Reduction**: 50-70% reduction in GC allocations during combat  
✅ **Tick Count Reduction**: 30-40% fewer Tick calls per frame  
✅ **Frame Rate**: Maintain 60 FPS with 100+ ships  
✅ **Cached Calculations**: 10x speedup for rating calculations

### Qualitative Metrics

✅ **Smooth Gameplay**: No noticeable hitches during intense combat  
✅ **Scalability**: Performance scales well with more actors  
✅ **Maintainability**: Pooling code is reusable and well-documented

---

## Risk Mitigation

### Pool Size Tuning

**Risk**: Pool too small = frequent allocations; too large = memory waste

**Mitigation**:
- Make pool sizes configurable
- Add monitoring/logging
- Tune based on profiling

### Timer Timing Issues

**Risk**: Timer-based updates may miss critical events

**Mitigation**:
- Keep critical systems on Tick (player controls, physics)
- Only convert non-critical systems
- Add event-driven triggers for important state changes

### Cache Invalidation Bugs

**Risk**: Stale cached data leading to incorrect calculations

**Mitigation**:
- Invalidate cache on ALL property changes
- Add validation in development builds
- Unit tests for cache correctness

---

## Documentation Requirements

1. **Performance Guide**: Document pooling patterns and usage
2. **API Documentation**: Update function comments
3. **Benchmark Results**: Document before/after metrics
4. **Configuration Guide**: Explain pool size tuning

---

## Next Steps After Phase 2

1. **Profile Results**: Measure actual improvements
2. **Adjust Settings**: Tune pool sizes and timer intervals
3. **Expand Pooling**: Apply to other systems if beneficial
4. **Phase 3 Planning**: Proceed with quality improvements

---

## Estimated Timeline

| Task | Time | Priority |
|------|------|----------|
| VFX Pooling | 4-5 hours | High |
| Audio Pooling Review | 1-2 hours | Medium |
| Tick Optimization | 6-8 hours | High |
| Calculation Caching | 4-6 hours | High |
| Testing & Tuning | 3-4 hours | Critical |
| **Total** | **18-25 hours** | - |

---

**Document Status**: ✅ Ready for Implementation  
**Prerequisites**: Phase 1 Complete ✅  
**Start Date**: TBD (after team review)

For questions or implementation assistance, refer to:
- [Unreal Directive Best Practices](.github/instructions/unreal-directive-best-practices.md)
- [Phase 1 Report](UNREAL_DIRECTIVE_PHASE1_REPORT.md)
- [Implementation Plan](../UNREAL_DIRECTIVE_IMPLEMENTATION_PLAN.md)
