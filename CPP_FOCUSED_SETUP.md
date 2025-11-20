# C++ Focused Setup Guide - Adastrea

**For developers who prefer working in C++ with minimal Blueprint usage**

## Overview

This guide explains how to work with Adastrea using primarily C++ code, only creating the absolutely essential Blueprints required by Unreal Engine.

## Understanding "Blueprints" in Unreal

There are three types of "Blueprints" in Unreal:

1. **Blueprint Classes** - Visual scripting alternatives to C++ classes (YOU CAN SKIP MOST OF THESE)
2. **Data Assets** - Configuration data (REQUIRED - these are just data, not code)
3. **Input Assets** - Enhanced Input definitions (REQUIRED - part of Unreal's input system)

## Input System: C++ Approach

### What Already Exists

**C++ Classes** (you have these):
- `InputConfigDataAsset.h/.cpp` - Centralized input configuration system
- All spaceship, combat, and game systems with C++ input handling

**Input Assets** (already in `Content/Input/`):
- `IA_Move.uasset` - Movement input action (data only, not a graph)
- `IA_Look.uasset` - Camera input action
- `IA_Boost.uasset` - Boost input action
- `IA_Fire_Primary.uasset` - Fire input action
- `IA_PauseMenu.uasset` - Pause input action
- `IMC_Spaceship.uasset` - Input Mapping Context (maps keys to actions)

**These are NOT Blueprint "graphs"** - they're lightweight data assets that define input bindings. You cannot delete them without breaking Enhanced Input.

### What You Need to Create (Minimal)

#### 1. DA_InputConfig Data Asset (ONE Blueprint Data Asset)

**Why**: Even in C++, you need one Data Asset instance to hold references to your Input Actions.

**How to Create**:
1. Content Browser → Right-click → Blueprint Class
2. Search for `InputConfigDataAsset`
3. Name it: `DA_InputConfig_Default`
4. Open it and assign the existing Input Actions:
   - `MoveAction` → `IA_Move`
   - `LookAction` → `IA_Look`
   - `BoostAction` → `IA_Boost`
   - `FirePrimaryAction` → `IA_Fire_Primary`
   - `MenuAction` → `IA_PauseMenu`
   - `SpaceshipMappingContext` → `IMC_Spaceship`

**That's it for input configuration!**

#### 2. Additional Input Actions (If Needed)

If you need more controls (secondary weapons, autopilot, etc.), create additional Input Actions:

**In Editor**:
1. Content Browser → Right-click → Input → Input Action
2. Name: `IA_[ActionName]` (e.g., `IA_Fire_Secondary`)
3. Configure Value Type (Boolean, Vector2D, etc.)

Then add them to your `IMC_Spaceship` mapping context and reference in `DA_InputConfig`.

## C++ Input Binding Example

Once your Data Asset is created, use it in C++:

```cpp
// In your Spaceship or Controller class
void AMySpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Cast to Enhanced Input Component
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput) return;
    
    // Load input config Data Asset
    UInputConfigDataAsset* InputConfig = LoadObject<UInputConfigDataAsset>(
        nullptr,
        TEXT("/Game/DataAssets/Input/DA_InputConfig_Default.DA_InputConfig_Default")
    );
    
    if (!InputConfig) return;
    
    // Bind movement
    EnhancedInput->BindAction(
        InputConfig->MoveAction,
        ETriggerEvent::Triggered,
        this,
        &AMySpaceship::Move
    );
    
    // Bind look
    EnhancedInput->BindAction(
        InputConfig->LookAction,
        ETriggerEvent::Triggered,
        this,
        &AMySpaceship::Look
    );
    
    // Bind fire
    EnhancedInput->BindAction(
        InputConfig->FirePrimaryAction,
        ETriggerEvent::Started,
        this,
        &AMySpaceship::FirePrimary
    );
    
    // Bind boost
    EnhancedInput->BindAction(
        InputConfig->BoostAction,
        ETriggerEvent::Started,
        this,
        &AMySpaceship::BoostStart
    );
    
    EnhancedInput->BindAction(
        InputConfig->BoostAction,
        ETriggerEvent::Completed,
        this,
        &AMySpaceship::BoostEnd
    );
}

// Input handler functions
void AMySpaceship::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    
    AddMovementInput(GetActorRightVector(), MovementVector.X);
    AddMovementInput(GetActorForwardVector(), MovementVector.Y);
}

void AMySpaceship::Look(const FInputActionValue& Value)
{
    FVector2D LookVector = Value.Get<FVector2D>();
    
    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);
}

void AMySpaceship::FirePrimary(const FInputActionValue& Value)
{
    // Fire weapon logic
    if (WeaponComponent)
    {
        WeaponComponent->Fire();
    }
}

void AMySpaceship::BoostStart(const FInputActionValue& Value)
{
    MaxSpeed *= BoostMultiplier;
}

void AMySpaceship::BoostEnd(const FInputActionValue& Value)
{
    MaxSpeed = NormalMaxSpeed;
}
```

## Essential Blueprints (Absolutely Required)

Even with a C++-focused workflow, Unreal Engine requires a few Blueprint instances:

### 1. Game Mode Blueprint (BP_SpaceGameMode)
**Why**: Unreal requires a GameMode instance to specify which classes to use.

**Minimal Setup**:
- Parent Class: `AdastreaGameMode` (your C++ class)
- Set Default Pawn Class (your spaceship C++ class or its Blueprint child)
- Set Player Controller Class (your controller C++ class)
- **No visual scripting needed!** Just set the class references.

### 2. Player Controller Blueprint (BP_SpaceshipController) - OPTIONAL
**Why**: Only if you want to add Mapping Context in BeginPlay (can also do in C++).

**Alternative**: Do it in C++:
```cpp
void AAdastreaPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        UInputConfigDataAsset* InputConfig = LoadInputConfig(); // Load your DA_InputConfig
        
        if (InputConfig && InputConfig->GetSpaceshipMappingContext())
        {
            Subsystem->AddMappingContext(
                InputConfig->GetSpaceshipMappingContext(),
                InputConfig->SpaceshipContextPriority
            );
        }
    }
}
```

### 3. Player Ship Blueprint (BP_PlayerShip) - OPTIONAL
**Why**: Only if you want to configure mesh/visuals in editor without recompiling.

**Alternative**: Create everything in C++:
```cpp
AMySpaceship::AMySpaceship()
{
    // Create static mesh component
    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    RootComponent = ShipMesh;
    
    // Create camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 800.0f;
    
    // Create camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraBoom);
    
    // Load mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Meshes/SM_Ship"));
    if (MeshAsset.Succeeded())
    {
        ShipMesh->SetStaticMesh(MeshAsset.Object);
    }
}
```

### 4. Data Asset Instances (Required)
**Why**: Configuration data for ships, factions, etc. These are just data containers, not code.

**Essential Data Assets**:
- `DA_InputConfig_Default` - Your input configuration (discussed above)
- `DA_Ship_[YourShip]` - Ship stats (based on `USpaceshipDataAsset`)
- Any faction, personnel, or other game data you want to configure

**These are NOT code** - they're just data files that configure your C++ systems.

## What You Can Skip

### You DO NOT Need:

❌ **Widget Blueprints (WBP_*)** - Create UI in C++ with `UUserWidget` subclasses
❌ **Blueprint Actor Classes** - Use pure C++ actors
❌ **Blueprint Function Libraries** - Use C++ static functions
❌ **Blueprint Interfaces** - Use C++ interfaces (IInterface)
❌ **Blueprint AI Logic** - Use C++ AI classes (`UFactionLogic`, `UPersonnelLogic`)

### Exception: Visual Assets
✅ **Static Meshes, Materials, Textures** - These are always assets (not Blueprints)
✅ **Particle Effects, Sounds** - Also assets
✅ **Input Actions/Mapping Contexts** - Required by Enhanced Input plugin

## Summary: Minimal Setup for C++ Workflow

**Step 1**: Create `DA_InputConfig_Default` Data Asset
- Assign existing Input Actions and Mapping Context
- Takes 2 minutes

**Step 2**: Reference it in your C++ code
- Load the Data Asset in `SetupPlayerInputComponent`
- Bind input handlers

**Step 3**: Create Game Mode Blueprint (optional)
- Just set class references, no scripting
- Alternative: Set in Project Settings → Maps & Modes

**Step 4**: Create any Data Assets for game content
- Ship stats, faction data, etc.
- These are just configuration, not code

**Done!** Everything else can be pure C++.

## Can I Delete Input Assets?

**NO** - Don't delete:
- ❌ Input Actions (IA_*) - Required by Enhanced Input system
- ❌ Input Mapping Contexts (IMC_*) - Required by Enhanced Input system
- ❌ DA_InputConfig - Your centralized configuration

**YES** - You can delete:
- ✅ Unnecessary Blueprint Classes (BP_* that you're not using)
- ✅ Widget Blueprints if you're doing UI in C++
- ✅ Blueprint Function Libraries you're not using

## Workflow Summary

```
Input in C++ Workflow:
1. Input Assets (IA_*, IMC_*) define WHAT inputs exist ← lightweight data
2. DA_InputConfig references them in one place ← configuration
3. C++ code loads DA_InputConfig and binds handlers ← your code
4. C++ functions handle input logic ← your code
```

**Bottom Line**: You're working in C++, but you need a few data assets for configuration. This is normal for Unreal Engine and doesn't mean you're "doing Blueprints."

## Advanced C++ Systems - Future Implementations

The following sections describe advanced C++ systems you can implement to expand Adastrea's capabilities. All systems follow the data-driven design pattern with Blueprint exposure.

---

## Faction Logic System (C++ Implementation)

### UFactionLogic - Strategic AI for Factions

**Purpose**: Implements strategic decision-making for factions at a high level.

**Header File** (`Source/Adastrea/Public/AI/FactionLogic.h`):

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AI/NPCLogicBase.h"
#include "Factions/FactionDataAsset.h"
#include "FactionLogic.generated.h"

/**
 * Faction Logic - Strategic AI for faction-level decisions
 * 
 * Handles:
 * - Diplomatic actions (declare war, make peace, form alliances)
 * - Economic decisions (trade routes, market manipulation)
 * - Military strategy (fleet movements, territory control)
 * - Research priorities (technology advancement)
 * 
 * Usage:
 * 1. Create Blueprint based on this class
 * 2. Override BlueprintNativeEvent functions for custom behavior
 * 3. Attach to faction-controlled actors or game state
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UFactionLogic : public UNPCLogicBase
{
    GENERATED_BODY()

public:
    UFactionLogic();

    // ====================
    // Configuration
    // ====================

    /** Faction data this logic controls */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction Logic")
    UFactionDataAsset* FactionData;

    /** How aggressive the faction is (0.0 = peaceful, 1.0 = aggressive) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction Logic", meta=(ClampMin=0.0, ClampMax=1.0))
    float AggressionLevel;

    /** How much the faction prioritizes economic growth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction Logic", meta=(ClampMin=0.0, ClampMax=1.0))
    float EconomicFocus;

    /** How much the faction prioritizes military strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction Logic", meta=(ClampMin=0.0, ClampMax=1.0))
    float MilitaryFocus;

    /** How much the faction prioritizes diplomacy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction Logic", meta=(ClampMin=0.0, ClampMax=1.0))
    float DiplomaticFocus;

    // ====================
    // Decision Functions (BlueprintNativeEvent)
    // ====================

    /**
     * Decide on diplomatic action toward another faction
     * @param TargetFaction The faction to interact with
     * @param CurrentRelationship Current relationship value (-100 to 100)
     * @return Decision: 0=No Action, 1=Improve Relations, 2=Worsen Relations, 3=Declare War, 4=Propose Alliance
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction Logic|Diplomacy")
    int32 DecideDiplomaticAction(UFactionDataAsset* TargetFaction, int32 CurrentRelationship);

    /**
     * Decide on economic action
     * @param MarketConditions Current market state data
     * @return Decision: 0=Hold, 1=Invest, 2=Sell, 3=Embargo, 4=Trade Agreement
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction Logic|Economy")
    int32 DecideEconomicAction(const FString& MarketConditions);

    /**
     * Decide on military action
     * @param ThreatLevel Perceived threat level (0.0 to 1.0)
     * @param AvailableForces Number of available military units
     * @return Decision: 0=Defend, 1=Patrol, 2=Attack, 3=Retreat, 4=Fortify
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction Logic|Military")
    int32 DecideMilitaryAction(float ThreatLevel, int32 AvailableForces);

    /**
     * Evaluate if faction should go to war
     * @param TargetFaction Potential war target
     * @param RelationshipValue Current relationship (-100 to 100)
     * @return True if faction should declare war
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Faction Logic|Diplomacy")
    bool ShouldDeclareWar(UFactionDataAsset* TargetFaction, int32 RelationshipValue);

    /**
     * Calculate trade price modifier based on faction relationships
     * @param OtherFaction Faction trading with
     * @param BasePrice Base price of goods
     * @return Modified price
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction Logic|Trading")
    float CalculateTradePriceModifier(UFactionDataAsset* OtherFaction, float BasePrice) const;

    // ====================
    // State Management
    // ====================

    /**
     * Update faction strategy based on game state
     * Called periodically by game systems
     */
    UFUNCTION(BlueprintCallable, Category="Faction Logic")
    void UpdateStrategy();

protected:
    // Default C++ implementations
    virtual int32 DecideDiplomaticAction_Implementation(UFactionDataAsset* TargetFaction, int32 CurrentRelationship);
    virtual int32 DecideEconomicAction_Implementation(const FString& MarketConditions);
    virtual int32 DecideMilitaryAction_Implementation(float ThreatLevel, int32 AvailableForces);
    virtual bool ShouldDeclareWar_Implementation(UFactionDataAsset* TargetFaction, int32 RelationshipValue);

private:
    /** Time since last strategy update */
    float TimeSinceLastUpdate;

    /** Cached strategic priorities */
    TMap<FString, float> StrategicPriorities;
};
```

**Implementation File** (`Source/Adastrea/AI/FactionLogic.cpp`):

```cpp
#include "AI/FactionLogic.h"

UFactionLogic::UFactionLogic()
{
    AggressionLevel = 0.3f;
    EconomicFocus = 0.5f;
    MilitaryFocus = 0.3f;
    DiplomaticFocus = 0.5f;
    TimeSinceLastUpdate = 0.0f;
}

int32 UFactionLogic::DecideDiplomaticAction_Implementation(UFactionDataAsset* TargetFaction, int32 CurrentRelationship)
{
    if (!TargetFaction || !FactionData)
    {
        return 0; // No action
    }

    // Peaceful factions focus on diplomacy
    if (AggressionLevel < 0.3f)
    {
        if (CurrentRelationship < 50)
        {
            return 1; // Improve relations
        }
        else if (CurrentRelationship > 70)
        {
            return 4; // Propose alliance
        }
    }
    // Aggressive factions may declare war
    else if (AggressionLevel > 0.7f)
    {
        if (CurrentRelationship < -50)
        {
            return 3; // Declare war
        }
        else if (CurrentRelationship < 0)
        {
            return 2; // Worsen relations
        }
    }

    return 0; // No action
}

int32 UFactionLogic::DecideEconomicAction_Implementation(const FString& MarketConditions)
{
    // Economic-focused factions prioritize trade
    if (EconomicFocus > 0.6f)
    {
        return 4; // Trade agreement
    }
    
    // Default: hold position
    return 0;
}

int32 UFactionLogic::DecideMilitaryAction_Implementation(float ThreatLevel, int32 AvailableForces)
{
    // High threat: defend or retreat
    if (ThreatLevel > 0.7f)
    {
        return AvailableForces > 10 ? 0 : 3; // Defend or Retreat
    }
    
    // Medium threat: patrol
    if (ThreatLevel > 0.3f)
    {
        return 1; // Patrol
    }
    
    // Low threat and aggressive: attack
    if (AggressionLevel > 0.6f && MilitaryFocus > 0.5f)
    {
        return 2; // Attack
    }
    
    return 0; // Defend
}

bool UFactionLogic::ShouldDeclareWar_Implementation(UFactionDataAsset* TargetFaction, int32 RelationshipValue)
{
    // Only aggressive factions with military focus declare war
    if (AggressionLevel < 0.5f || MilitaryFocus < 0.4f)
    {
        return false;
    }
    
    // Relationship must be very negative
    return RelationshipValue < -70;
}

float UFactionLogic::CalculateTradePriceModifier(UFactionDataAsset* OtherFaction, float BasePrice) const
{
    // Implement based on PlayerReputationComponent pattern
    // Friendly factions: discount
    // Hostile factions: premium
    // This is a placeholder - integrate with reputation system
    return BasePrice;
}

void UFactionLogic::UpdateStrategy()
{
    // Update strategic priorities based on game state
    // This would integrate with UAdastreaGameState
    TimeSinceLastUpdate = 0.0f;
}
```

---

## Faction Runtime State System

### UFactionRuntimeState - Dynamic Faction State Tracking

**Purpose**: Tracks dynamic faction state that changes during gameplay (not static config).

**Header File** (`Source/Adastrea/Public/Factions/FactionRuntimeState.h`):

```cpp
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Factions/FactionDataAsset.h"
#include "FactionRuntimeState.generated.h"

/**
 * Faction Runtime State
 * Tracks dynamic state of a faction during gameplay
 * 
 * Separated from FactionDataAsset to keep configuration immutable
 * and runtime state mutable.
 * 
 * Tracks:
 * - Current resources and wealth
 * - Territory control
 * - Active wars and alliances
 * - Military strength
 * - Economic power
 * - Research progress
 */
UCLASS(BlueprintType)
class ADASTREA_API UFactionRuntimeState : public UObject
{
    GENERATED_BODY()

public:
    UFactionRuntimeState();

    // ====================
    // Configuration
    // ====================

    /** Reference to static faction configuration */
    UPROPERTY(BlueprintReadOnly, Category="Faction State")
    UFactionDataAsset* FactionData;

    // ====================
    // Economic State
    // ====================

    /** Current faction wealth */
    UPROPERTY(BlueprintReadWrite, Category="Faction State|Economy")
    int64 CurrentWealth;

    /** Income per game cycle */
    UPROPERTY(BlueprintReadOnly, Category="Faction State|Economy")
    int32 IncomeRate;

    /** Number of trade routes controlled */
    UPROPERTY(BlueprintReadOnly, Category="Faction State|Economy")
    int32 TradeRoutes;

    // ====================
    // Military State
    // ====================

    /** Total military power rating */
    UPROPERTY(BlueprintReadOnly, Category="Faction State|Military")
    float MilitaryPower;

    /** Number of ships in faction fleet */
    UPROPERTY(BlueprintReadWrite, Category="Faction State|Military")
    int32 FleetSize;

    /** Number of stations controlled */
    UPROPERTY(BlueprintReadWrite, Category="Faction State|Military")
    int32 StationsControlled;

    // ====================
    // Diplomatic State
    // ====================

    /** Factions currently at war with */
    UPROPERTY(BlueprintReadWrite, Category="Faction State|Diplomacy")
    TArray<UFactionDataAsset*> FactionsAtWar;

    /** Allied factions */
    UPROPERTY(BlueprintReadWrite, Category="Faction State|Diplomacy")
    TArray<UFactionDataAsset*> AlliedFactions;

    /** Relationship values with other factions */
    UPROPERTY(BlueprintReadWrite, Category="Faction State|Diplomacy")
    TMap<UFactionDataAsset*, int32> FactionRelationships;

    // ====================
    // Territory State
    // ====================

    /** Systems controlled by this faction */
    UPROPERTY(BlueprintReadWrite, Category="Faction State|Territory")
    TArray<FName> ControlledSystems;

    /** Influence score in galaxy */
    UPROPERTY(BlueprintReadOnly, Category="Faction State|Territory")
    float GalacticInfluence;

    // ====================
    // Blueprint Functions
    // ====================

    /**
     * Initialize runtime state from faction data asset
     * @param InFactionData Static faction configuration
     */
    UFUNCTION(BlueprintCallable, Category="Faction State")
    void Initialize(UFactionDataAsset* InFactionData);

    /**
     * Update faction state (called periodically by game)
     * @param DeltaTime Time since last update
     */
    UFUNCTION(BlueprintCallable, Category="Faction State")
    void UpdateState(float DeltaTime);

    /**
     * Check if faction is at war with another
     * @param OtherFaction Faction to check
     * @return True if at war
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction State")
    bool IsAtWarWith(UFactionDataAsset* OtherFaction) const;

    /**
     * Check if faction is allied with another
     * @param OtherFaction Faction to check
     * @return True if allied
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction State")
    bool IsAlliedWith(UFactionDataAsset* OtherFaction) const;

    /**
     * Get relationship value with another faction
     * @param OtherFaction Faction to query
     * @return Relationship value (-100 to 100)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction State")
    int32 GetRelationshipWith(UFactionDataAsset* OtherFaction) const;

    /**
     * Modify relationship with another faction
     * @param OtherFaction Target faction
     * @param Delta Amount to change (positive or negative)
     */
    UFUNCTION(BlueprintCallable, Category="Faction State")
    void ModifyRelationship(UFactionDataAsset* OtherFaction, int32 Delta);

    /**
     * Declare war on another faction
     * @param TargetFaction Faction to declare war on
     */
    UFUNCTION(BlueprintCallable, Category="Faction State")
    void DeclareWar(UFactionDataAsset* TargetFaction);

    /**
     * Make peace with a faction at war
     * @param TargetFaction Faction to make peace with
     */
    UFUNCTION(BlueprintCallable, Category="Faction State")
    void MakePeace(UFactionDataAsset* TargetFaction);

    /**
     * Form alliance with another faction
     * @param TargetFaction Faction to ally with
     */
    UFUNCTION(BlueprintCallable, Category="Faction State")
    void FormAlliance(UFactionDataAsset* TargetFaction);

    /**
     * Calculate total faction power (economic + military)
     * @return Total power rating
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction State")
    float CalculateTotalPower() const;

protected:
    /** Time accumulator for periodic updates */
    float TimeAccumulator;
};
```

---

## Material Crafting System (C++ Implementation)

### UMaterialCraftingComponent - Crafting System for Materials

**Purpose**: Enables crafting of refined materials from raw resources.

**Header File** (`Source/Adastrea/Public/Materials/MaterialCraftingComponent.h`):

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialDataAsset.h"
#include "MaterialCraftingComponent.generated.h"

/**
 * Recipe for crafting materials
 */
USTRUCT(BlueprintType)
struct FCraftingRecipe
{
    GENERATED_BODY()

    /** Recipe name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText RecipeName;

    /** Input materials and quantities */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<UMaterialDataAsset*, int32> InputMaterials;

    /** Output material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialDataAsset* OutputMaterial;

    /** Output quantity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OutputQuantity;

    /** Time to craft (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CraftingTime;

    /** Required crafting skill level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredSkillLevel;
};

/**
 * Material Crafting Component
 * Handles crafting of materials from resources
 * 
 * Usage:
 * - Attach to player ship, space station, or crafting facility
 * - Load crafting recipes
 * - Call CraftMaterial to begin crafting
 * - Track progress and collect results
 */
UCLASS(BlueprintType, ClassGroup=(Adastrea), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UMaterialCraftingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMaterialCraftingComponent();

    // ====================
    // Configuration
    // ====================

    /** Available crafting recipes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    TArray<FCraftingRecipe> AvailableRecipes;

    /** Crafting speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting", meta=(ClampMin=0.1, ClampMax=10.0))
    float CraftingSpeedMultiplier;

    /** Maximum concurrent crafting operations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    int32 MaxConcurrentCrafts;

    // ====================
    // Crafting Functions
    // ====================

    /**
     * Start crafting a material
     * @param Recipe Recipe to craft
     * @return True if crafting started successfully
     */
    UFUNCTION(BlueprintCallable, Category="Crafting")
    bool StartCrafting(const FCraftingRecipe& Recipe);

    /**
     * Cancel current crafting operation
     * @param CraftIndex Index of craft to cancel
     */
    UFUNCTION(BlueprintCallable, Category="Crafting")
    void CancelCrafting(int32 CraftIndex);

    /**
     * Get crafting progress
     * @param CraftIndex Index of craft
     * @return Progress (0.0 to 1.0)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
    float GetCraftingProgress(int32 CraftIndex) const;

    /**
     * Check if can craft recipe
     * @param Recipe Recipe to check
     * @return True if all requirements met
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
    bool CanCraftRecipe(const FCraftingRecipe& Recipe) const;

    /**
     * Get all available recipes
     * @return Array of recipes
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
    TArray<FCraftingRecipe> GetAvailableRecipes() const { return AvailableRecipes; }

    // ====================
    // Component Overrides
    // ====================

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;

private:
    /** Active crafting operations */
    struct FActiveCraft
    {
        FCraftingRecipe Recipe;
        float TimeElapsed;
        bool bComplete;
    };

    TArray<FActiveCraft> ActiveCrafts;

    /** Check inventory for materials */
    bool HasRequiredMaterials(const FCraftingRecipe& Recipe) const;

    /** Consume input materials */
    void ConsumeMaterials(const FCraftingRecipe& Recipe);

    /** Grant output materials */
    void GrantOutputMaterials(const FCraftingRecipe& Recipe);
};
```

---

## Advanced Trading Market Simulator

### UMarketSimulatorSubsystem - Dynamic Market Simulation

**Purpose**: Simulates supply/demand economics across all markets.

**Header File** (`Source/Adastrea/Public/Trading/MarketSimulatorSubsystem.h`):

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Trading/TradeItemDataAsset.h"
#include "Trading/MarketDataAsset.h"
#include "MarketSimulatorSubsystem.generated.h"

/**
 * Market state for a specific trade item in a market
 */
USTRUCT(BlueprintType)
struct FMarketItemState
{
    GENERATED_BODY()

    /** Trade item */
    UPROPERTY(BlueprintReadOnly)
    UTradeItemDataAsset* Item;

    /** Current supply */
    UPROPERTY(BlueprintReadOnly)
    int32 Supply;

    /** Current demand */
    UPROPERTY(BlueprintReadOnly)
    int32 Demand;

    /** Current price (modified by supply/demand) */
    UPROPERTY(BlueprintReadOnly)
    float CurrentPrice;

    /** Base price */
    UPROPERTY(BlueprintReadOnly)
    float BasePrice;

    /** Price trend (positive = rising, negative = falling) */
    UPROPERTY(BlueprintReadOnly)
    float PriceTrend;
};

/**
 * Market Simulator Subsystem
 * Simulates economy across all markets
 * 
 * Features:
 * - Dynamic supply/demand simulation
 * - Price fluctuations based on trading activity
 * - Market events (shortages, surpluses, blockades)
 * - AI trader simulation
 * - Cross-market arbitrage
 * 
 * Runs as GameInstance subsystem for persistence
 */
UCLASS()
class ADASTREA_API UMarketSimulatorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ====================
    // Configuration
    // ====================

    /** How often to update markets (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Market Simulator")
    float UpdateInterval;

    /** Price volatility (0.0 = stable, 1.0 = very volatile) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Market Simulator", meta=(ClampMin=0.0, ClampMax=1.0))
    float PriceVolatility;

    /** Enable AI traders */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Market Simulator")
    bool bEnableAITraders;

    /** Number of AI traders per market */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Market Simulator")
    int32 AITradersPerMarket;

    // ====================
    // Market Functions
    // ====================

    /**
     * Register a market for simulation
     * @param Market Market to register
     */
    UFUNCTION(BlueprintCallable, Category="Market Simulator")
    void RegisterMarket(UMarketDataAsset* Market);

    /**
     * Unregister a market
     * @param Market Market to unregister
     */
    UFUNCTION(BlueprintCallable, Category="Market Simulator")
    void UnregisterMarket(UMarketDataAsset* Market);

    /**
     * Get current price for item in market
     * @param Market Market to query
     * @param Item Trade item
     * @return Current price
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Market Simulator")
    float GetCurrentPrice(UMarketDataAsset* Market, UTradeItemDataAsset* Item) const;

    /**
     * Get market state for item
     * @param Market Market to query
     * @param Item Trade item
     * @return Market state
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Market Simulator")
    FMarketItemState GetMarketState(UMarketDataAsset* Market, UTradeItemDataAsset* Item) const;

    /**
     * Simulate player buying items
     * @param Market Market to buy from
     * @param Item Trade item
     * @param Quantity Quantity to buy
     */
    UFUNCTION(BlueprintCallable, Category="Market Simulator")
    void SimulateBuy(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity);

    /**
     * Simulate player selling items
     * @param Market Market to sell to
     * @param Item Trade item
     * @param Quantity Quantity to sell
     */
    UFUNCTION(BlueprintCallable, Category="Market Simulator")
    void SimulateSell(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity);

    /**
     * Trigger market event
     * @param Market Affected market
     * @param EventType Type of event (shortage, surplus, blockade, etc.)
     */
    UFUNCTION(BlueprintCallable, Category="Market Simulator")
    void TriggerMarketEvent(UMarketDataAsset* Market, const FString& EventType);

    // ====================
    // Subsystem Overrides
    // ====================

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    /** Periodic market update */
    void UpdateMarkets();

    /** Simulate AI trader activity */
    void SimulateAITraders();

    /** Calculate price based on supply/demand */
    float CalculatePrice(int32 Supply, int32 Demand, float BasePrice) const;

private:
    /** Registered markets */
    UPROPERTY()
    TArray<UMarketDataAsset*> RegisteredMarkets;

    /** Market states */
    TMap<UMarketDataAsset*, TMap<UTradeItemDataAsset*, FMarketItemState>> MarketStates;

    /** Timer handle for updates */
    FTimerHandle UpdateTimerHandle;
};
```

---

## Multiplayer Networking Foundation

### Replication and Network Architecture

**Purpose**: Foundation for future multiplayer support using Unreal's replication system.

**Header File** (`Source/Adastrea/Public/Player/AdastreaGameState.h` - Enhanced):

```cpp
// Add to existing AdastreaGameState.h

// ====================
// Network Replication
// ====================

/**
 * Setup replication for game state variables
 * Override in subclass to add more replicated properties
 */
virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/**
 * Called on clients when replicated property changes
 * @param PropertyName Name of property that changed
 */
UFUNCTION()
virtual void OnRep_GameStateChanged();

/** Server time for synchronization */
UPROPERTY(Replicated, BlueprintReadOnly, Category="Network")
float ServerTime;

/** Number of players in session */
UPROPERTY(Replicated, BlueprintReadOnly, Category="Network")
int32 PlayerCount;

/** Is server authoritative for game state */
UPROPERTY(Replicated, BlueprintReadOnly, Category="Network")
bool bIsServerAuthoritative;
```

**Example Implementation**:

```cpp
#include "Net/UnrealNetwork.h"

void AAdastreaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate server time to all clients
    DOREPLIFETIME(AAdastreaGameState, ServerTime);
    
    // Replicate player count
    DOREPLIFETIME(AAdastreaGameState, PlayerCount);
    
    // Replicate server authority flag
    DOREPLIFETIME(AAdastreaGameState, bIsServerAuthoritative);
    
    // Add more replicated properties as needed
}

void AAdastreaGameState::OnRep_GameStateChanged()
{
    // Handle replicated property changes on clients
    // Update UI, trigger events, etc.
}
```

---

## C++ Performance Optimization Patterns

### Object Pooling for Projectiles

**Purpose**: Reuse projectile objects instead of spawning/destroying.

```cpp
// Header: Source/Adastrea/Public/Combat/ProjectilePoolSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectilePoolSubsystem.generated.h"

UCLASS()
class ADASTREA_API UProjectilePoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /**
     * Get or spawn a projectile from pool
     * @param ProjectileClass Class of projectile to get
     * @param Location Spawn location
     * @param Rotation Spawn rotation
     * @return Pooled projectile actor
     */
    UFUNCTION(BlueprintCallable, Category="Object Pool")
    AActor* GetPooledProjectile(TSubclassOf<AActor> ProjectileClass, 
                                const FVector& Location, 
                                const FRotator& Rotation);

    /**
     * Return projectile to pool
     * @param Projectile Projectile to return
     */
    UFUNCTION(BlueprintCallable, Category="Object Pool")
    void ReturnToPool(AActor* Projectile);

protected:
    /** Pool of inactive projectiles */
    UPROPERTY()
    TMap<TSubclassOf<AActor>, TArray<AActor*>> ProjectilePools;

    /** Maximum pool size per projectile type */
    UPROPERTY(EditAnywhere, Category="Object Pool")
    int32 MaxPoolSize = 100;
};
```

### LOD Management for Distant Objects

```cpp
// In any actor that needs LOD management
UFUNCTION(BlueprintCallable, Category="Performance")
void UpdateLODLevel()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        float Distance = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
        
        // Set LOD based on distance
        if (Distance < 1000.0f)
        {
            // High detail - LOD 0
            SetLODLevel(0);
        }
        else if (Distance < 5000.0f)
        {
            // Medium detail - LOD 1
            SetLODLevel(1);
        }
        else if (Distance < 10000.0f)
        {
            // Low detail - LOD 2
            SetLODLevel(2);
        }
        else
        {
            // Very low detail or culled - LOD 3
            SetLODLevel(3);
        }
    }
}
```

---

## C++ Subsystem Creation Pattern

### Custom Game Instance Subsystem Example

```cpp
// Header: Source/Adastrea/Public/YourSystem/YourSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "YourSubsystem.generated.h"

/**
 * Your Custom Subsystem
 * Game Instance subsystems persist across level transitions
 * Perfect for global managers and state
 */
UCLASS()
class ADASTREA_API UYourSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ====================
    // Subsystem Lifecycle
    // ====================

    /** Called when subsystem is created */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Called when subsystem is destroyed */
    virtual void Deinitialize() override;

    /** Should this subsystem be created? */
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // ====================
    // Your Subsystem Functions
    // ====================

    UFUNCTION(BlueprintCallable, Category="Your Subsystem")
    void YourFunction();

protected:
    /** Your subsystem state */
    UPROPERTY()
    TArray<UObject*> YourState;
};

// Implementation
void UYourSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // Initialize your subsystem
    UE_LOG(LogTemp, Log, TEXT("YourSubsystem initialized"));
}

void UYourSubsystem::Deinitialize()
{
    // Cleanup
    YourState.Empty();
    Super::Deinitialize();
}
```

---

## Advanced C++ Component Patterns

### Actor Component Communication via Interfaces

**Purpose**: Enable loose coupling between components using C++ interfaces.

```cpp
// Header: Source/Adastrea/Public/Interfaces/IDamageable.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IDamageable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
    GENERATED_BODY()
};

/**
 * IDamageable Interface
 * Implement on any actor that can take damage
 */
class ADASTREA_API IDamageable
{
    GENERATED_BODY()

public:
    /**
     * Apply damage to this actor
     * @param DamageAmount Amount of damage to apply
     * @param DamageType Type of damage (Energy, Kinetic, etc.)
     * @param DamageInstigator Actor that caused the damage
     * @return Actual damage dealt after armor/shields
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    float ApplyDamage(float DamageAmount, const FName& DamageType, AActor* DamageInstigator);

    /**
     * Check if actor is still alive
     * @return True if alive
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    bool IsAlive() const;
};

// Implementation in actor:
class ASpaceship : public APawn, public IDamageable
{
    // ...
    
    virtual float ApplyDamage_Implementation(float DamageAmount, const FName& DamageType, AActor* DamageInstigator) override
    {
        if (HealthComponent)
        {
            return HealthComponent->TakeDamage(DamageAmount, DamageType);
        }
        return 0.0f;
    }
    
    virtual bool IsAlive_Implementation() const override
    {
        return HealthComponent && HealthComponent->GetCurrentHealth() > 0.0f;
    }
};

// Usage in other code:
void UWeaponComponent::FireAtTarget(AActor* Target)
{
    if (IDamageable* DamageableTarget = Cast<IDamageable>(Target))
    {
        float DamageDealt = IDamageable::Execute_ApplyDamage(Target, WeaponDamage, DamageType, GetOwner());
        
        if (!IDamageable::Execute_IsAlive(Target))
        {
            // Target destroyed
            OnTargetDestroyed.Broadcast(Target);
        }
    }
}
```

---

## Async Task Execution (for Heavy Calculations)

### Running Heavy Calculations Off Game Thread

**Purpose**: Prevent frame drops during expensive operations.

```cpp
// Header: Source/Adastrea/Public/AsyncTasks/PathfindingTask.h
#pragma once

#include "CoreMinimal.h"
#include "Async/AsyncWork.h"

/**
 * Async pathfinding task
 * Runs pathfinding calculation on background thread
 */
class FPathfindingTask : public FNonAbandonableTask
{
    friend class FAutoDeleteAsyncTask<FPathfindingTask>;

public:
    FPathfindingTask(const FVector& InStart, const FVector& InGoal)
        : StartLocation(InStart)
        , GoalLocation(InGoal)
    {
    }

    // Required by FNonAbandonableTask
    void DoWork()
    {
        // Heavy pathfinding calculation here
        PathResult = CalculatePath(StartLocation, GoalLocation);
    }

    FORCEINLINE TStatId GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FPathfindingTask, STATGROUP_ThreadPoolAsyncTasks);
    }

    // Get result (call from game thread)
    TArray<FVector> GetPath() const { return PathResult; }

private:
    FVector StartLocation;
    FVector GoalLocation;
    TArray<FVector> PathResult;

    TArray<FVector> CalculatePath(const FVector& Start, const FVector& Goal)
    {
        // A* or other pathfinding algorithm
        TArray<FVector> Path;
        // ... complex calculation ...
        return Path;
    }
};

// Usage in component:
void UNavigationComponent::FindPathAsync(const FVector& Start, const FVector& Goal)
{
    // Start async task
    AsyncTask = new FAutoDeleteAsyncTask<FPathfindingTask>(Start, Goal);
    AsyncTask->StartBackgroundTask();
}

void UNavigationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Check if async task is done
    if (AsyncTask && AsyncTask->IsDone())
    {
        TArray<FVector> Path = AsyncTask->GetTask().GetPath();
        OnPathFound.Broadcast(Path);
        AsyncTask = nullptr; // Auto-deleted
    }
}
```

---

## Smart Pointer Usage for Memory Safety

### Using TSharedPtr, TWeakPtr, TUniquePtr

**Purpose**: Prevent memory leaks and dangling pointers.

```cpp
// For non-UObject classes that need shared ownership:
class FCustomDataProcessor
{
public:
    void ProcessData() { /* ... */ }
};

// Shared pointer (reference counted)
TSharedPtr<FCustomDataProcessor> Processor = MakeShared<FCustomDataProcessor>();

// Weak pointer (doesn't keep object alive)
TWeakPtr<FCustomDataProcessor> WeakProcessor = Processor;

// Later, check if still valid:
if (TSharedPtr<FCustomDataProcessor> SharedProcessor = WeakProcessor.Pin())
{
    SharedProcessor->ProcessData();
}

// Unique pointer (single ownership)
TUniquePtr<FCustomDataProcessor> UniqueProcessor = MakeUnique<FCustomDataProcessor>();

// For UObject*, always use TWeakObjectPtr or TSoftObjectPtr:
UPROPERTY()
TWeakObjectPtr<ASpaceship> TargetShip;

// Check validity before use:
if (TargetShip.IsValid())
{
    TargetShip->TakeDamage(10.0f);
}
```

---

## Delegates and Events for Decoupled Communication

### Multicast Delegates for Broadcasting Events

```cpp
// In header:
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldDepleted, ASpaceship*, Ship, float, LastDamage);

class ADASTREA_API UCombatHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /** Fired when shields are depleted */
    UPROPERTY(BlueprintAssignable, Category="Combat|Events")
    FOnShieldDepleted OnShieldDepleted;

    void TakeDamage(float Damage)
    {
        ShieldStrength -= Damage;
        
        if (ShieldStrength <= 0.0f)
        {
            // Broadcast to all listeners
            OnShieldDepleted.Broadcast(Cast<ASpaceship>(GetOwner()), Damage);
        }
    }
};

// Binding in Blueprint or C++:
void ASpaceship::BeginPlay()
{
    Super::BeginPlay();
    
    if (HealthComponent)
    {
        // Bind C++ function to delegate
        HealthComponent->OnShieldDepleted.AddDynamic(this, &ASpaceship::HandleShieldDepleted);
    }
}

void ASpaceship::HandleShieldDepleted(ASpaceship* Ship, float LastDamage)
{
    UE_LOG(LogTemp, Warning, TEXT("Shields depleted! Last damage: %f"), LastDamage);
    // Play warning sound, activate visual effect, etc.
}
```

---

## Struct Optimization for Data Assets

### Using Bit Packing for Boolean Flags

```cpp
// Instead of many bool properties (each takes 1 byte):
USTRUCT(BlueprintType)
struct FShipCapabilities_Inefficient
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere) bool bCanLandOnPlanets;
    UPROPERTY(EditAnywhere) bool bCanDockWithStations;
    UPROPERTY(EditAnywhere) bool bCanWarpJump;
    UPROPERTY(EditAnywhere) bool bCanCarryCargo;
    UPROPERTY(EditAnywhere) bool bHasCloakingDevice;
    UPROPERTY(EditAnywhere) bool bHasShields;
    UPROPERTY(EditAnywhere) bool bHasWeapons;
    UPROPERTY(EditAnywhere) bool bHasScanner;
    // 8 bytes total
};

// Better: Use bitfield (takes 1 byte total):
USTRUCT(BlueprintType)
struct FShipCapabilities
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask))
    uint8 Capabilities;
    
    // Define bit flags
    static constexpr uint8 CanLandOnPlanets = 1 << 0;  // 0b00000001
    static constexpr uint8 CanDockWithStations = 1 << 1; // 0b00000010
    static constexpr uint8 CanWarpJump = 1 << 2;       // 0b00000100
    static constexpr uint8 CanCarryCargo = 1 << 3;     // 0b00001000
    static constexpr uint8 HasCloakingDevice = 1 << 4; // 0b00010000
    static constexpr uint8 HasShields = 1 << 5;        // 0b00100000
    static constexpr uint8 HasWeapons = 1 << 6;        // 0b01000000
    static constexpr uint8 HasScanner = 1 << 7;        // 0b10000000
    
    // Helper functions
    bool HasCapability(uint8 Flag) const { return (Capabilities & Flag) != 0; }
    void AddCapability(uint8 Flag) { Capabilities |= Flag; }
    void RemoveCapability(uint8 Flag) { Capabilities &= ~Flag; }
};

// Usage:
FShipCapabilities Caps;
Caps.AddCapability(FShipCapabilities::CanWarpJump);
Caps.AddCapability(FShipCapabilities::HasWeapons);

if (Caps.HasCapability(FShipCapabilities::CanWarpJump))
{
    // Ship can warp
}
```

---

## Custom Logging Categories

### Organized Logging for Debugging

```cpp
// In a header (e.g., AdastreaLog.h):
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaShip, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaFaction, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaTrading, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaQuest, Log, All);

// In a .cpp file:
DEFINE_LOG_CATEGORY(LogAdastreaShip);
DEFINE_LOG_CATEGORY(LogAdastreaFaction);
DEFINE_LOG_CATEGORY(LogAdastreaTrading);
DEFINE_LOG_CATEGORY(LogAdastreaAI);
DEFINE_LOG_CATEGORY(LogAdastreaQuest);

// Usage throughout codebase:
UE_LOG(LogAdastreaShip, Warning, TEXT("Ship %s health critical: %f"), *GetName(), CurrentHealth);
UE_LOG(LogAdastreaFaction, Display, TEXT("Faction %s declared war on %s"), 
    *FactionA->GetFactionName().ToString(), *FactionB->GetFactionName().ToString());
UE_LOG(LogAdastreaTrading, Error, TEXT("Failed to complete trade: insufficient funds"));

// Filter logs in editor console:
// Log LogAdastreaShip All    <- Show all ship logs
// Log LogAdastreaShip Off    <- Hide ship logs
// Log LogAdastreaAI Verbose  <- Show verbose AI logs
```

---

## Blueprint Function Library for Utilities

### Static Helper Functions Accessible from Blueprints

```cpp
// Header: Source/Adastrea/Public/AdastreaFunctionLibrary.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AdastreaFunctionLibrary.generated.h"

/**
 * Adastrea Blueprint Function Library
 * Static utility functions accessible from Blueprints and C++
 */
UCLASS()
class ADASTREA_API UAdastreaFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Calculate travel time between two points
     * @param StartLocation Starting position
     * @param EndLocation Destination position
     * @param ShipSpeed Ship's maximum speed
     * @return Time in seconds
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Adastrea|Travel")
    static float CalculateTravelTime(const FVector& StartLocation, const FVector& EndLocation, float ShipSpeed);

    /**
     * Get faction relationship tier name
     * @param RelationshipValue Value from -100 to 100
     * @return Tier name (e.g., "Hostile", "Neutral", "Friendly")
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Adastrea|Faction")
    static FText GetRelationshipTierName(int32 RelationshipValue);

    /**
     * Format credits amount with commas
     * @param Amount Credit amount
     * @return Formatted string (e.g., "1,234,567 Cr")
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Adastrea|Formatting")
    static FText FormatCredits(int64 Amount);

    /**
     * Get random point within sphere
     * @param Origin Center of sphere
     * @param Radius Sphere radius
     * @return Random point
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Adastrea|Math")
    static FVector GetRandomPointInSphere(const FVector& Origin, float Radius);

    /**
     * Check if location is in space (not near planet)
     * @param Location Location to check
     * @param WorldContextObject World context
     * @return True if in open space
     */
    UFUNCTION(BlueprintCallable, Category="Adastrea|Space", meta=(WorldContext="WorldContextObject"))
    static bool IsLocationInOpenSpace(UObject* WorldContextObject, const FVector& Location);
};

// Implementation:
float UAdastreaFunctionLibrary::CalculateTravelTime(const FVector& StartLocation, const FVector& EndLocation, float ShipSpeed)
{
    float Distance = FVector::Dist(StartLocation, EndLocation);
    return ShipSpeed > 0.0f ? Distance / ShipSpeed : 0.0f;
}

FText UAdastreaFunctionLibrary::GetRelationshipTierName(int32 RelationshipValue)
{
    if (RelationshipValue >= 75) return FText::FromString("Allied");
    if (RelationshipValue >= 50) return FText::FromString("Friendly");
    if (RelationshipValue >= 25) return FText::FromString("Cordial");
    if (RelationshipValue >= -25) return FText::FromString("Neutral");
    if (RelationshipValue >= -50) return FText::FromString("Unfriendly");
    if (RelationshipValue >= -75) return FText::FromString("Hostile");
    return FText::FromString("Enemy");
}

FText UAdastreaFunctionLibrary::FormatCredits(int64 Amount)
{
    FNumberFormattingOptions Options;
    Options.UseGrouping = true;
    Options.GroupingSeparator = FText::FromString(",");
    
    FText FormattedNumber = FText::AsNumber(Amount, &Options);
    return FText::Format(FText::FromString("{0} Cr"), FormattedNumber);
}

FVector UAdastreaFunctionLibrary::GetRandomPointInSphere(const FVector& Origin, float Radius)
{
    return Origin + FMath::VRand() * FMath::RandRange(0.0f, Radius);
}
```

---

## Data Validation for Data Assets

### Ensure Data Assets Have Valid Configuration

```cpp
// In your Data Asset class:
#if WITH_EDITOR
virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override
{
    EDataValidationResult Result = Super::IsDataValid(ValidationErrors);
    
    // Validate faction name
    if (FactionName.IsEmpty())
    {
        ValidationErrors.Add(FText::FromString("Faction name cannot be empty"));
        Result = EDataValidationResult::Invalid;
    }
    
    // Validate reputation range
    if (InitialReputation < -100 || InitialReputation > 100)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString("Initial reputation {0} is out of range (-100 to 100)"),
            FText::AsNumber(InitialReputation)
        ));
        Result = EDataValidationResult::Invalid;
    }
    
    // Validate required references
    if (!HomeworldLocation)
    {
        ValidationErrors.Add(FText::FromString("Faction must have a homeworld location"));
        Result = EDataValidationResult::Invalid;
    }
    
    return Result;
}
#endif

// Unreal Editor will show validation errors when saving the asset
```

---

## Additional Resources

- **C++ Input Handling**: See `InputConfigDataAsset.h` for reference
- **Enhanced Input Docs**: [Unreal Engine Enhanced Input](https://docs.unrealengine.com/5.6/en-US/enhanced-input-in-unreal-engine/)
- **Data-Driven C++**: All Adastrea systems use Data Assets for configuration while keeping logic in C++
- **Networking**: [Unreal Engine Networking](https://docs.unrealengine.com/5.6/en-US/networking-and-multiplayer-in-unreal-engine/)
- **Subsystems**: [Unreal Engine Subsystems](https://docs.unrealengine.com/5.6/en-US/programming-subsystems-in-unreal-engine/)
- **Performance**: [Unreal Engine Performance Guidelines](https://docs.unrealengine.com/5.6/en-US/performance-guidelines-for-unreal-engine/)

---

## Summary: Extending Adastrea with C++

This guide has shown you:

1. **Minimal Setup** - Essential Blueprints for C++ workflow
2. **Input System** - C++ implementation with Enhanced Input
3. **FactionLogic** - Strategic AI for factions
4. **FactionRuntimeState** - Dynamic faction state tracking
5. **MaterialCrafting** - Crafting system implementation
6. **MarketSimulator** - Advanced trading simulation
7. **Networking Foundation** - Multiplayer preparation
8. **Performance Patterns** - Object pooling, LOD management
9. **Subsystem Creation** - Custom game subsystems

**All examples follow Adastrea's core principles:**
- ✅ Data-Driven Design (Data Assets for config)
- ✅ Blueprint Exposure (BlueprintCallable, BlueprintNativeEvent)
- ✅ C++ Performance (Core logic in C++)
- ✅ Designer-Friendly (Blueprint override capability)
- ✅ Modular Architecture (Independent, reusable systems)

**Questions?** The input system in Adastrea is already C++-focused. The Input Actions are just lightweight configuration data, not Blueprint "graphs." Use these patterns to extend Adastrea with additional C++ systems while maintaining designer accessibility.
