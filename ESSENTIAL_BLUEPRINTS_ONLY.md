# Essential Blueprints Only - Minimal Setup Guide

**For C++ developers who want the absolute minimum Blueprint usage**

## TL;DR - What You Actually Need

### Absolutely Required (3 items):
1. **DA_InputConfig** - One Data Asset to configure input
2. **Game Mode Blueprint** OR Project Settings configuration
3. **Input Assets** (IA_*, IMC_*) - Already exist, don't delete them

### That's It!

Everything else can be pure C++.

---

## Detailed Breakdown

### ✅ MUST HAVE: Input System

#### DA_InputConfig (1 Data Asset)
- **Type**: Blueprint Data Asset (but just data, no code/graphs)
- **Parent**: `InputConfigDataAsset` (your C++ class)
- **Location**: `Content/DataAssets/Input/DA_InputConfig.uasset`
- **Purpose**: References Input Actions in one place
- **Time to Create**: 2 minutes
- **Can I skip?**: NO - C++ code needs to load this

**How to Create**:
```
1. Content Browser → Right-click → Blueprint Class
2. Search: InputConfigDataAsset
3. Name: DA_InputConfig
4. Open and assign:
   - MoveAction → IA_Move
   - LookAction → IA_Look
   - BoostAction → IA_Boost
   - FirePrimaryAction → IA_Fire_Primary
   - MenuAction → IA_PauseMenu
   - SpaceshipMappingContext → IMC_Spaceship
5. Save
```

#### Input Actions & Mapping Context (Already Exist)
- **Files**: `Content/Input/Actions/IA_*.uasset`, `Content/Input/IMC_Spaceship.uasset`
- **Status**: ✅ Already created
- **Purpose**: Define what inputs exist and map keys to them
- **Can I skip?**: NO - Required by Unreal's Enhanced Input plugin
- **Are these "graphs"?**: NO - They're lightweight data definitions

**Note**: These are NOT Blueprint "graphs" with visual scripting. They're simple data assets that Unreal's Enhanced Input system requires. Think of them like header files that define your input API.

---

### ⚠️ OPTIONAL: Game Framework

#### Option A: Use Project Settings (No Blueprint Needed)
**Best for pure C++ workflow**

1. Edit → Project Settings → Project → Maps & Modes
2. Set Default GameMode to your C++ class (e.g., `AdastreaGameMode`)
3. Set Default Pawn Class to your C++ class (e.g., `ASpaceship`)
4. Done! No Blueprint required.

#### Option B: BP_SpaceGameMode Blueprint
**If you prefer to see it visually or override per-map**

- **Type**: Blueprint Class
- **Parent**: `AdastreaGameMode` (your C++ class)
- **Purpose**: Just sets which C++ classes to use (no scripting needed)
- **Time to Create**: 30 seconds
- **Can I skip?**: YES - Use Project Settings instead

**How to Create**:
```
1. Content Browser → Right-click → Blueprint Class
2. Parent: AdastreaGameMode
3. Name: BP_SpaceGameMode
4. Open Class Defaults:
   - Default Pawn Class: Your ship C++ class
   - Player Controller Class: Your controller C++ class
5. Save
```

---

### 🟢 COMPLETELY OPTIONAL: Everything Else

#### Player Ship Blueprint
- **Can I skip?**: YES
- **Alternative**: Pure C++ `ASpaceship` subclass
- **Only create if**: You want to tweak mesh/visuals in editor without recompiling

#### HUD/UI Widgets
- **Can I skip?**: YES
- **Alternative**: C++ `UUserWidget` subclasses
- **Only create if**: You want visual UI editing

#### Data Assets (Ship Stats, Faction Data, etc.)
- **Can I skip?**: Technically yes, but NOT recommended
- **Why keep them?**: These are just configuration data, not code
- **Benefit**: Change ship stats without recompiling C++
- **Adastrea Philosophy**: Data-driven design with C++ logic

**Example**: Your ship logic is in C++ `ASpaceship`, but ship stats (health, speed, etc.) come from `DA_Ship_Scout` Data Asset. This lets you have multiple ship variants without duplicating C++ code.

---

## What Can I Delete?

### ✅ Safe to Delete (if not using):
- Blueprint Actor Classes you're not using (BP_*)
- Widget Blueprints (WBP_*)
- Blueprint Function Libraries
- Any "example" or "template" Blueprints

### ❌ Do NOT Delete:
- Input Actions (IA_*)
- Input Mapping Contexts (IMC_*)
- DA_InputConfig (once you create it)
- Any Data Assets you're actually using for configuration

---

## Example: Pure C++ Ship

```cpp
// MySpaceship.h
#pragma once

#include "CoreMinimal.h"
#include "Ships/Spaceship.h"
#include "InputActionValue.h"
#include "MySpaceship.generated.h"

UCLASS()
class ADASTREA_API AMySpaceship : public ASpaceship
{
    GENERATED_BODY()
    
public:
    AMySpaceship();
    
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
protected:
    // Input handlers
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void FirePrimary(const FInputActionValue& Value);
    void BoostStart(const FInputActionValue& Value);
    void BoostEnd(const FInputActionValue& Value);
    
    // Components (created in C++)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* ShipMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USpringArmComponent* CameraBoom;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCameraComponent* Camera;
};

// MySpaceship.cpp
#include "MySpaceship.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/InputConfigDataAsset.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AMySpaceship::AMySpaceship()
{
    // Create mesh
    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    RootComponent = ShipMesh;
    
    // Create camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 800.0f;
    CameraBoom->bUsePawnControlRotation = true;
    
    // Create camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraBoom);
    
    // Configure collision
    ShipMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ShipMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AMySpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Cast to Enhanced Input
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput) return;
    
    // Load input config Data Asset
    UInputConfigDataAsset* InputConfig = LoadObject<UInputConfigDataAsset>(
        nullptr,
        TEXT("/Game/DataAssets/Input/DA_InputConfig.DA_InputConfig")
    );
    
    if (!InputConfig) return;
    
    // Bind all inputs
    EnhancedInput->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &AMySpaceship::Move);
    EnhancedInput->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &AMySpaceship::Look);
    EnhancedInput->BindAction(InputConfig->FirePrimaryAction, ETriggerEvent::Started, this, &AMySpaceship::FirePrimary);
    EnhancedInput->BindAction(InputConfig->BoostAction, ETriggerEvent::Started, this, &AMySpaceship::BoostStart);
    EnhancedInput->BindAction(InputConfig->BoostAction, ETriggerEvent::Completed, this, &AMySpaceship::BoostEnd);
}

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
    // Your fire logic here
    UE_LOG(LogTemp, Log, TEXT("Firing primary weapon!"));
}

void AMySpaceship::BoostStart(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Log, TEXT("Boost activated!"));
}

void AMySpaceship::BoostEnd(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Log, TEXT("Boost deactivated!"));
}
```

**No Blueprints needed!** (Except DA_InputConfig for configuration)

---

## Quick Start Checklist

- [ ] 1. Create `DA_InputConfig` Data Asset (2 minutes)
- [ ] 2. Assign existing Input Actions to it
- [ ] 3. Load it in your C++ `SetupPlayerInputComponent`
- [ ] 4. (Optional) Set Default Game Mode in Project Settings
- [ ] 5. Write all gameplay logic in C++

**Done!** You're working in pure C++ with minimal editor setup.

---

## FAQ

**Q: The guides say to create IMC_Spaceship and IA_Move. Are these "graphs"?**  
A: NO. These are lightweight data assets (like JSON files) that define input bindings. They're NOT visual scripting graphs. They already exist in the repository.

**Q: Can I delete all the IA_* and IMC_* files?**  
A: NO. These are required by Unreal's Enhanced Input plugin. Even pure C++ projects need these.

**Q: Do I have to use Blueprint visual scripting?**  
A: NO. The only "Blueprint" you need is `DA_InputConfig`, which is just a data container with no scripting.

**Q: What about Data Assets for ships, factions, etc.?**  
A: These are recommended (not required). They're configuration data, not code. You can hardcode everything in C++ if you prefer, but Data Assets make iteration faster.

**Q: Is this against Adastrea's design philosophy?**  
A: Not at all! Adastrea uses **Data-Driven C++** design. C++ for logic, Data Assets for configuration. You're doing it right!

---

## Summary

**Minimum Setup**:
1. Create `DA_InputConfig` (1 Data Asset)
2. Don't delete existing Input Assets (IA_*, IMC_*)
3. Everything else in C++

**You are NOT doing "Blueprint visual scripting"** - you're using Unreal's asset system for configuration data.

This is the recommended approach for C++ developers in Adastrea! 🚀
