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

## Additional Resources

- **C++ Input Handling**: See `InputConfigDataAsset.h` for reference
- **Enhanced Input Docs**: [Unreal Engine Enhanced Input](https://docs.unrealengine.com/5.6/en-US/enhanced-input-in-unreal-engine/)
- **Data-Driven C++**: All Adastrea systems use Data Assets for configuration while keeping logic in C++

---

**Questions?** The input system in Adastrea is already C++-focused. The Input Actions are just lightweight configuration data, not Blueprint "graphs."
