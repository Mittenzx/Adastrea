# Unreal Directive Best Practices for Adastrea

This document extends the core copilot instructions with advanced best practices derived from [UnrealDirective.com](https://unrealdirective.com/) and Epic Games' official guidelines.

## Table of Contents

- [Modular Game Features](#modular-game-features)
- [Performance and Optimization](#performance-and-optimization)
- [Advanced UPROPERTY and UFUNCTION Usage](#advanced-uproperty-and-ufunction-usage)
- [Primary Data Assets](#primary-data-assets)
- [Garbage Collection Management](#garbage-collection-management)
- [Blueprint Organization Excellence](#blueprint-organization-excellence)
- [Asset Naming Standards](#asset-naming-standards)
- [C++ and Blueprint Balance](#c-and-blueprint-balance)
- [Memory Management](#memory-management)
- [Code Organization and Reusability](#code-organization-and-reusability)

---

## Modular Game Features

### Overview

Modular Game Features (introduced in UE 4.27, expanded in UE5) enable packaging discrete features as dynamically loadable plugins. This reduces tech debt and improves maintainability.

### Key Principles

1. **Self-Contained Features**: Each feature should be an independent plugin in `/Plugins/GameFeatures/*`
2. **No Direct References**: Base game should not directly reference features
3. **Session-Wide Activation**: Features are activated for entire multiplayer sessions
4. **Easy Transfer**: Features can be moved between projects

### Adastrea Application

```cpp
// ✅ GOOD: Feature as plugin with Data Assets
// Plugins/GameFeatures/NewShipClass/
//   - Data Assets define ship types
//   - Components handle unique behavior
//   - Can be loaded/unloaded dynamically

// ❌ AVOID: Hardcoded features in core module
// Source/Adastrea/Ships/HardcodedShipType.cpp
```

### Best Practices

- Use Data Assets to configure modular features
- Ensure clean activation/deactivation
- Profile memory impact of dynamic loading
- Test features in isolation and integrated

**References:**
- [Modular Game Features Guide](https://unrealdirective.com/articles/modular-game-features-what-you-need-to-know)
- Epic's Modular Gameplay documentation

---

## Performance and Optimization

### Profiling First

- **Always profile before optimizing** - Use UE's built-in profiling tools
- **Test in packaged builds** - Editor performance != shipping performance
- **Measure, don't assume** - Data beats intuition

### Object Pooling

For frequently spawned/destroyed actors (projectiles, effects, ships):

```cpp
// ✅ GOOD: Object pooling for projectiles
UCLASS()
class UProjectilePoolComponent : public UActorComponent
{
    UPROPERTY()
    TArray<AProjectile*> PooledProjectiles;
    
    AProjectile* AcquireProjectile();
    void ReturnProjectile(AProjectile* Projectile);
};

// ❌ AVOID: Constant spawning/destroying
void FireWeapon()
{
    AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>();
    // Later: Projectile->Destroy(); // Causes GC pressure
}
```

### Tick Optimization

```cpp
// ✅ GOOD: Stagger updates, use timers
UFUNCTION()
void InitializeAI()
{
    // Update at different intervals
    GetWorld()->GetTimerManager().SetTimer(
        UpdateTimerHandle, 
        this, 
        &UMyAI::UpdateLogic, 
        1.0f,  // Every second
        true   // Loop
    );
}

// ❌ AVOID: Everything on Tick
void Tick(float DeltaTime) override
{
    // Heavy logic every frame - BAD!
}
```

### LOD (Level of Detail)

- Use LOD for distant ships and stations
- Reduce update frequency for distant objects
- Disable particle effects beyond view range
- Stream content based on player location

### Caching Expensive Calculations

```cpp
// ✅ GOOD: Cache expensive calculations
UPROPERTY()
float CachedCombatRating;

bool bCombatRatingDirty;

float GetCombatRating()
{
    if (bCombatRatingDirty)
    {
        CachedCombatRating = CalculateCombatRating();
        bCombatRatingDirty = false;
    }
    return CachedCombatRating;
}

// ❌ AVOID: Recalculating every time
float GetCombatRating() const
{
    return CalculateCombatRating(); // Expensive every call
}
```

---

## Advanced UPROPERTY and UFUNCTION Usage

### UPROPERTY for Garbage Collection

**Critical**: All `UObject*` pointers MUST use `UPROPERTY()` to prevent memory corruption, even if not exposed to editor or Blueprints.

```cpp
// ✅ GOOD: All UObject pointers marked
UPROPERTY()
UFactionDataAsset* InternalFactionData;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
USpaceshipDataAsset* ShipConfig;

// ❌ BAD: Raw pointer without UPROPERTY - MEMORY CORRUPTION RISK
UFactionDataAsset* InternalFactionData;  // ❌ DANGEROUS!
```

### UPROPERTY Specifier Combinations

```cpp
// Read-only display in editor, Blueprint read access
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
float CurrentHealth;

// Editable in editor, Blueprint read/write, with constraints
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", 
    meta=(ClampMin="0.0", ClampMax="1000.0", UIMin="0.0", UIMax="1000.0"))
float MaxSpeed;

// Instance editable only (not default), with tooltip
UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Runtime", 
    meta=(ToolTip="Set per-instance in level"))
bool bIsActive;

// Multiline text for descriptions
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Info",
    meta=(MultiLine=true))
FText Description;

// Transient - not saved, not visible
UPROPERTY(Transient)
float TemporaryCalculation;
```

### Advanced UFUNCTION Specifiers

```cpp
// Pure function - no side effects, shows as pure node in BP
UFUNCTION(BlueprintPure, Category="Stats")
float GetHealthPercent() const;

// Callable with custom display name
UFUNCTION(BlueprintCallable, Category="Combat", 
    meta=(DisplayName="Apply Damage", CompactNodeTitle="DMG"))
void DealDamage(float Amount);

// Native event - C++ default, Blueprint override
UFUNCTION(BlueprintNativeEvent, Category="AI")
void OnTargetDetected(AActor* Target);
virtual void OnTargetDetected_Implementation(AActor* Target);

// Implementable event - Blueprint only
UFUNCTION(BlueprintImplementableEvent, Category="Events")
void OnShieldDepleted();

// RPC for networking
UFUNCTION(Server, Reliable, WithValidation)
void ServerFireWeapon(FVector Location);

// Advanced display options
UFUNCTION(BlueprintCallable, Category="Advanced",
    meta=(AdvancedDisplay="3", DefaultToSelf="Target"))
void ComplexFunction(int32 A, int32 B, int32 C, int32 D, AActor* Target);
```

### Encapsulation Best Practices

```cpp
// ✅ GOOD: Private/Protected with public accessors
class UMyComponent : public UActorComponent
{
protected:
    UPROPERTY(EditAnywhere, Category="Config")
    float MaxValue;
    
public:
    // Blueprint-friendly accessor
    UFUNCTION(BlueprintPure, Category="Stats")
    float GetMaxValue() const { return MaxValue; }
    
    UFUNCTION(BlueprintCallable, Category="Stats")
    void SetMaxValue(float NewValue) 
    { 
        MaxValue = FMath::Clamp(NewValue, 0.0f, 100.0f);
    }
};

// ❌ AVOID: Direct public access
class UMyComponent : public UActorComponent
{
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
    float MaxValue;  // No validation, no encapsulation
};
```

---

## Primary Data Assets

### When to Use Primary Data Assets

Use **Primary Data Assets** when you need:
- Unique identification for each asset
- Asset Manager support for loading/unloading
- Dynamic asset management (DLC, modular content)
- Large-scale projects with many content types

### Comparison: Data Asset vs Primary Data Asset

| Feature | Data Asset | Primary Data Asset |
|---------|-----------|-------------------|
| Static data storage | ✅ Yes | ✅ Yes |
| Unique Asset ID | ❌ No | ✅ Yes |
| Asset Manager support | ❌ No | ✅ Yes |
| Manual loading/unloading | ❌ Limited | ✅ Yes |
| Designer friendly | ✅ Very | ✅ Very |
| Best for | Game config, items | Robust asset management |

### Implementation

```cpp
// Primary Data Asset with unique ID
UCLASS()
class UShipPrimaryDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
    
public:
    // Override to provide unique ID
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("Ship", GetFName());
    }
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship")
    FText ShipName;
    
    // ... other properties
};
```

### Asset Manager Usage

```cpp
// Load Primary Data Asset by ID
UAssetManager& AssetManager = UAssetManager::Get();
FPrimaryAssetId AssetId = FPrimaryAssetId("Ship", FName("Pathfinder"));

TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAsset(
    AssetId,
    TArray<FName>(),
    FStreamableDelegate::CreateUObject(this, &UMyClass::OnAssetLoaded)
);

// Unload when no longer needed
AssetManager.UnloadPrimaryAsset(AssetId);
```

### When to Use Regular Data Assets (Current Adastrea Pattern)

The current Adastrea approach using regular `UDataAsset` is appropriate for:
- ✅ Designer-friendly content creation
- ✅ Static configuration data
- ✅ Content that's referenced in world/blueprints
- ✅ Medium-scale projects (current stage)

Consider migrating to Primary Data Assets if:
- Project scales to 1000+ unique content pieces
- Need dynamic content loading for performance
- Planning DLC or modular content system
- Need robust asset tracking and management

---

## Garbage Collection Management

### Understanding UE Garbage Collection

Unreal's GC periodically frees unreferenced `UObject`-derived objects. Poor management causes:
- Frame rate spikes and stuttering
- Memory bloat
- Performance degradation

### GC Optimization Settings

```ini
; In DefaultEngine.ini or Project Settings
[/Script/Engine.GarbageCollectionSettings]

; Increase GC interval (default 60s, can go higher)
gc.TimeBetweenPurgingPendingKillObjects=300

; Enable incremental GC (breaks cleanup into smaller chunks)
gc.IncrementalBeginDestroyEnabled=1

; Enable multithreaded GC
gc.MultithreadedDestructionEnabled=1

; Cluster settings for related objects
gc.CreateGCClusters=1
gc.ActorClusteringEnabled=1
gc.BlueprintClusteringEnabled=1
```

### Code-Level GC Best Practices

```cpp
// ✅ GOOD: Explicit dereferencing
void UnloadFeature()
{
    if (FeatureObject)
    {
        FeatureObject->ConditionalBeginDestroy();
        FeatureObject = nullptr;  // Clear reference
    }
}

// ✅ GOOD: Manage object lifecycles
UFUNCTION()
void CleanupProjectiles()
{
    for (AProjectile* Projectile : ActiveProjectiles)
    {
        if (Projectile && Projectile->IsValidLowLevel())
        {
            Projectile->Destroy();
        }
    }
    ActiveProjectiles.Empty();  // Clear array
}

// ❌ AVOID: Leaving dangling references
void UnloadFeature()
{
    // FeatureObject not cleared - prevents GC
}
```

### Avoiding GC Spikes

1. **Minimize object creation/destruction** - Use pooling
2. **Stagger destruction** - Don't destroy 1000 objects at once
3. **Profile GC** - Use `stat gc` console command
4. **Avoid Tick for temporary objects** - Use timers or events
5. **Level transitions** - Manage object cleanup carefully

```cpp
// ✅ GOOD: Staggered destruction
void DestroyObjectsGradually()
{
    const int32 BatchSize = 10;
    for (int32 i = 0; i < BatchSize && ObjectsToDestroy.Num() > 0; ++i)
    {
        ObjectsToDestroy[0]->Destroy();
        ObjectsToDestroy.RemoveAt(0);
    }
    
    if (ObjectsToDestroy.Num() > 0)
    {
        // Schedule next batch
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            this, &UMyClass::DestroyObjectsGradually
        );
    }
}
```

### Debugging GC Issues

```cpp
// Console commands for GC debugging (packaged builds only)
// stat gc - Show GC statistics
// obj gc - Force garbage collection
// obj list class=Actor - List all actors

// Logging GC events
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaGC, Log, All);

void MyClass::BeginDestroy()
{
    Super::BeginDestroy();
    UE_LOG(LogAdastreaGC, Log, TEXT("Destroying: %s"), *GetName());
}
```

---

## Blueprint Organization Excellence

### Graph Organization

1. **Keep functions small** - Maximum 20-30 nodes
2. **Use comment boxes** - Group related nodes with descriptive comments
3. **Color coding** - Consistent colors for comment boxes (red = input, blue = logic, green = output)
4. **Reroute nodes** - Keep wires clean and organized
5. **Alignment** - Use alignment tools (Ctrl+Shift+W)

### Function and Macro Usage

```
✅ GOOD Blueprint Structure:
- Main logic calls functions/macros
- Each function has single responsibility
- Complex logic extracted to separate functions
- Reusable logic as macros or function libraries

❌ BAD Blueprint Structure:
- Everything in EventGraph
- 100+ node functions
- Duplicate logic in multiple places
- Spaghetti wire connections
```

### Variable Organization

```cpp
// Blueprint variable categories (organize in editor)
// ✅ GOOD: Organized categories
Category: "Movement"
  - MaxSpeed
  - Acceleration
  - TurnRate

Category: "Combat"
  - CurrentHealth
  - MaxHealth
  - Armor

Category: "Debug"
  - bDebugMode
  - DebugDrawDistance

// Add tooltips in Details panel for complex variables
```

### Blueprint Naming

```
✅ GOOD:
Function: CalculateDamage
Function: GetHealthPercent
Variable: bIsInCombat
Variable: TargetActor
Macro: ApplyStatusEffect

❌ AVOID:
Function: Func1
Function: DoStuff
Variable: temp
Variable: var1
Macro: M
```

### Flow Control

```
✅ GOOD:
- Left-to-right execution flow
- Top-to-bottom for branches
- Clear input/output separation
- Minimal wire crossings

❌ AVOID:
- Backwards execution flow
- Criss-crossed wires
- Unclear entry/exit points
```

---

## Asset Naming Standards

### Expanded Prefix System

Following UnrealDirective and Epic's recommendations:

| Asset Type | Prefix | Example | Notes |
|------------|--------|---------|-------|
| Blueprint | BP_ | BP_Ship_Fighter | Actor Blueprints |
| Widget Blueprint | WBP_ | WBP_HUD_Main | UI widgets |
| Animation Blueprint | ABP_ | ABP_Character | Character animation |
| Data Asset | DA_ | DA_Ship_Pathfinder | Primary game data |
| Material | M_ | M_Metal_Chrome | Base materials |
| Material Instance | MI_ | MI_Metal_Chrome_Blue | Material instances |
| Material Function | MF_ | MF_Noise_Generator | Reusable material functions |
| Texture | T_ | T_Metal_D | + suffix for type |
| Static Mesh | SM_ | SM_Crate_Large | 3D models |
| Skeletal Mesh | SK_ | SK_Character_Hero | Rigged models |
| Skeleton | SKEL_ | SKEL_Character | Animation skeleton |
| Animation | A_ | A_Walk_Forward | Animation sequences |
| Animation Montage | AM_ | AM_Attack_Combo | Animation montages |
| Particle System | P_ | P_Explosion | Cascade particles |
| Niagara System | NS_ | NS_ShipTrail | Niagara effects |
| Sound Cue | SC_ | SC_Explosion | Sound cues |
| Sound Wave | SW_ | SW_Footstep_01 | Raw audio |
| Sound Mix | SM_ | SM_Combat | Audio mixes |
| Physics Material | PM_ | PM_Metal | Physics materials |
| Level | L_ or MAP_ | L_SpaceStation_01 | Game levels |
| Data Table | DT_ | DT_Items | Data tables |
| Curve | C_ | C_DamageFalloff | Curve assets |

### Texture Suffixes

- `_D` or `_BC` - Diffuse/Base Color/Albedo
- `_N` - Normal Map
- `_R` - Roughness
- `_M` - Metallic
- `_S` - Specular
- `_AO` - Ambient Occlusion
- `_H` or `_Disp` - Height/Displacement
- `_E` - Emissive
- `_Mask` - Mask texture
- `_A` - Alpha/Opacity

### Naming Structure

```
[Prefix]_[AssetName]_[Descriptor]_[Variant]_[Suffix]

Examples:
✅ T_Metal_Chrome_Diffuse_01_D
✅ M_Ship_Hull_Master
✅ MI_Ship_Hull_Fighter_Blue
✅ SM_Crate_Wooden_Large
✅ BP_Enemy_Fighter_Variant_A
✅ DA_Ship_Pathfinder
✅ WBP_Menu_Settings

❌ texture1
❌ MyMaterial
❌ ship
❌ BP
```

### Folder Structure

```
Content/
├── Blueprints/
│   ├── Characters/
│   ├── Ships/
│   ├── Stations/
│   └── UI/
├── DataAssets/
│   ├── Ships/
│   ├── Factions/
│   └── Personnel/
├── Materials/
│   ├── Master/          # Master materials
│   ├── Instances/       # Material instances
│   └── Functions/       # Material functions
├── Meshes/
│   ├── Static/
│   └── Skeletal/
├── Textures/
│   ├── Ships/
│   └── UI/
└── Audio/
    ├── Music/
    └── SFX/
```

**Rules:**
- PascalCase for all folders
- No spaces in folder names
- Group by asset type first, then by system/feature
- Keep folder depth reasonable (max 4-5 levels)

---

## C++ and Blueprint Balance

### When to Use C++

Use C++ for:
- ✅ Performance-critical code (AI, physics, calculations)
- ✅ Complex algorithms and data structures
- ✅ Low-level engine interaction
- ✅ Reusable utility functions
- ✅ Network replication logic
- ✅ Core game systems

### When to Use Blueprint

Use Blueprint for:
- ✅ Level-specific logic
- ✅ Prototyping and iteration
- ✅ Designer-driven content
- ✅ UI and visual effects
- ✅ Simple event handling
- ✅ Gameplay tweaking

### The Ideal Balance (Adastrea Pattern)

```cpp
// ✅ EXCELLENT: C++ foundation with Blueprint extension
UCLASS(BlueprintType)
class USpaceshipDataAsset : public UDataAsset
{
    GENERATED_BODY()
    
public:
    // Data configured by designers in editor
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
    float MaxHealth;
    
    // Complex calculation in C++ for performance
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat")
    float CalculateCombatRating() const;
    
    // Extensible behavior via BlueprintNativeEvent
    UFUNCTION(BlueprintNativeEvent, Category="Combat")
    void OnDamageReceived(float Damage);
    virtual void OnDamageReceived_Implementation(float Damage);
};
```

### Exposing C++ to Blueprint

```cpp
// ✅ GOOD: Well-designed Blueprint API
class ASpaceship : public APawn
{
    // Pure functions for queries (no side effects)
    UFUNCTION(BlueprintPure, Category="Ship Status")
    float GetHealthPercent() const;
    
    UFUNCTION(BlueprintPure, Category="Ship Status")
    bool IsAlive() const;
    
    // Callable functions for actions
    UFUNCTION(BlueprintCallable, Category="Ship Control")
    void SetThrottle(float Percent);
    
    UFUNCTION(BlueprintCallable, Category="Combat")
    void FireWeapon(int32 WeaponIndex);
    
    // Events for Blueprint to implement
    UFUNCTION(BlueprintImplementableEvent, Category="Events")
    void OnHealthChanged(float NewHealth, float OldHealth);
    
    // Native events for hybrid implementation
    UFUNCTION(BlueprintNativeEvent, Category="AI")
    void OnTargetAcquired(AActor* Target);
};
```

### Design Philosophy

```
C++ Layer (Performance Foundation):
    ↓
BlueprintNativeEvent Layer (Extensibility):
    ↓
Blueprint Layer (Designer Content):
    ↓
Data Asset Layer (Configuration):
```

---

## Memory Management

### Smart Pointers vs Raw Pointers

```cpp
// ✅ GOOD: UPROPERTY for UObject pointers
UPROPERTY()
UFactionDataAsset* FactionData;

UPROPERTY()
TArray<ASpaceship*> Ships;

// ✅ GOOD: Smart pointers for non-UObject C++ objects
TSharedPtr<FComplexData> SharedData;
TWeakPtr<FComplexData> WeakReference;
TUniquePtr<FComplexData> UniqueData;

// ❌ AVOID: Raw pointers to UObjects without UPROPERTY
UFactionDataAsset* FactionData;  // ❌ GC will not track this!

// ✅ GOOD: Raw pointers for temporary references (but check validity)
void ProcessActor(AActor* Actor)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }
    // Use Actor...
}
```

### Null Checks

```cpp
// ✅ GOOD: Always check before use
void UseDataAsset()
{
    if (!ShipDataAsset)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("ShipDataAsset is null"));
        return;
    }
    
    float Speed = ShipDataAsset->GetMaxSpeed();
}

// ✅ GOOD: IsValid for actor references
void UseActor()
{
    if (!IsValid(TargetActor))
    {
        UE_LOG(LogAdastrea, Warning, TEXT("TargetActor is invalid"));
        return;
    }
    
    TargetActor->TakeDamage(10.0f);
}

// ❌ BAD: No null check
void UseDataAsset()
{
    float Speed = ShipDataAsset->GetMaxSpeed();  // CRASH if null!
}
```

### Avoiding Memory Leaks

```cpp
// ✅ GOOD: Proper cleanup in destructor/BeginDestroy
void UMyComponent::BeginDestroy()
{
    // Clean up any references
    if (TimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    }
    
    // Clear arrays
    CachedActors.Empty();
    
    // Call parent
    Super::BeginDestroy();
}

// ✅ GOOD: RAII pattern with TUniquePtr
class UMySystem : public UObject
{
    TUniquePtr<FExpensiveResource> Resource;
    
    void Initialize()
    {
        Resource = MakeUnique<FExpensiveResource>();
    }
    
    // Automatically cleaned up when object destroyed
};
```

---

## Code Organization and Reusability

### Composition Over Inheritance

```cpp
// ✅ GOOD: Composition with components
UCLASS()
class ASpaceship : public APawn
{
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USpaceshipControlsComponent* ControlsComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCombatHealthComponent* HealthComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UWeaponComponent* WeaponComponent;
};

// ❌ AVOID: Deep inheritance hierarchies
class ASpaceship : public APawn { };
class AFighterShip : public ASpaceship { };
class AHeavyFighterShip : public AFighterShip { };
class AEliteFighterShip : public AHeavyFighterShip { };  // Too deep!
```

### Interface Pattern

```cpp
// ✅ GOOD: Interfaces for shared behavior
UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
    GENERATED_BODY()
};

class IDamageable
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    void TakeDamage(float Amount, AActor* Instigator);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    bool IsAlive() const;
};

// Multiple classes can implement
class ASpaceship : public APawn, public IDamageable { };
class ASpaceStation : public AActor, public IDamageable { };
```

### Utility Functions

```cpp
// ✅ GOOD: Static utility functions in function libraries
UCLASS()
class UAdastreaFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    /** Calculate distance between two actors */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities")
    static float GetDistanceBetweenActors(AActor* A, AActor* B);
    
    /** Check if actor is in view cone */
    UFUNCTION(BlueprintPure, Category="Adastrea|Utilities")
    static bool IsInViewCone(AActor* Observer, AActor* Target, float ConeAngle);
    
    /** Get all actors of class within radius */
    UFUNCTION(BlueprintCallable, Category="Adastrea|Utilities")
    static TArray<AActor*> GetActorsInRadius(
        UObject* WorldContextObject,
        FVector Center,
        float Radius,
        TSubclassOf<AActor> ActorClass
    );
};
```

### Module Organization

```
✅ GOOD Module Structure:
Source/Adastrea/
├── Public/          # Public headers
│   ├── Ships/
│   ├── Factions/
│   └── AI/
├── Private/         # Implementation files
│   ├── Ships/
│   ├── Factions/
│   └── AI/
└── Adastrea.Build.cs

❌ AVOID:
- Public and private mixed
- Circular dependencies between modules
- Exposing internal implementation
```

---

## Summary: Unreal Directive Principles Applied to Adastrea

### Core Principles

1. **Data-Driven Design** ✅ Already excellent in Adastrea
2. **Modular Architecture** ✅ Good, can improve with game features plugin pattern
3. **Performance First** ⚠️ Apply profiling and optimization techniques
4. **Designer-Friendly** ✅ Already excellent with Blueprint exposure
5. **Memory Management** ⚠️ Ensure all UPROPERTY usage is correct
6. **Clean Code** ⚠️ Review naming conventions and organization

### Quick Wins for Adastrea

1. Add UPROPERTY to all UObject* pointers (even private ones)
2. Implement object pooling for projectiles/effects
3. Review and optimize Tick usage
4. Add GC optimization settings to DefaultEngine.ini
5. Ensure consistent asset naming throughout Content/
6. Add utility function libraries for common operations
7. Consider Primary Data Assets for future scalability

### Long-Term Improvements

1. Evaluate modular game features for DLC/expansion support
2. Implement comprehensive profiling strategy
3. Add automated asset validation (naming, references)
4. Create Blueprint style guide document
5. Set up performance budgets per system

---

## References and Further Reading

### Official Epic Games Resources

- [Epic C++ Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine)
- [Blueprint Best Practices](https://dev.epicgames.com/documentation/en-us/unreal-engine/blueprint-best-practices-in-unreal-engine)
- [Asset Naming Conventions](https://dev.epicgames.com/documentation/en-us/unreal-engine/recommended-asset-naming-conventions-in-unreal-engine-projects)
- [Garbage Collection](https://dev.epicgames.com/documentation/en-us/unreal-engine/garbage-collection-settings-in-the-unreal-engine-project-settings)

### Unreal Directive Resources

- [Unreal Directive Home](https://unrealdirective.com/)
- [Modular Game Features](https://unrealdirective.com/articles/modular-game-features-what-you-need-to-know)
- [Asset Naming Conventions](https://unrealdirective.com/resources/asset-naming-conventions)
- [Coding Standards](https://unrealdirective.com/community/resources/coding-standard)
- [Working with Data](https://unrealdirective.com/community/resources/working-with-data-in-unreal-engine)

### Community Resources

- [Allar's UE5 Style Guide](https://github.com/Allar/ue5-style-guide)
- [Unreal Garden](https://unreal-garden.com/)
- [Epic Developer Community Forums](https://forums.unrealengine.com/)

---

**Document Version**: 1.0  
**Last Updated**: 2025-12-08  
**Based on**: UnrealDirective.com best practices and Epic Games official guidelines  
**Maintained by**: Adastrea Development Team
