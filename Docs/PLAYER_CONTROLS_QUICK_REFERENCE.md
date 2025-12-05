# Player Controls Quick Reference

**One-page quick reference for understanding player control systems in Adastrea**

For detailed documentation, see:
- [PLAYER_CONTROLS_WORKFLOW.md](PLAYER_CONTROLS_WORKFLOW.md) - Detailed workflows
- [PLAYER_CONTROLS_DIAGRAMS.md](PLAYER_CONTROLS_DIAGRAMS.md) - Visual diagrams

---

## System Overview

```
┌────────────────────────────────────────────────────────────┐
│                    PLAYER CONTROL SYSTEMS                  │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  ┌──────────────────────┐    ┌──────────────────────┐    │
│  │  SPACESHIP SYSTEM    │    │  CHARACTER SYSTEM    │    │
│  │  (✅ Implemented)     │←──→│  (⚠️ Planned)        │    │
│  └──────────────────────┘    └──────────────────────┘    │
│           │                            │                  │
│           │ Press E at dock            │ Press E at ship  │
│           └────────────────────────────┘                  │
└────────────────────────────────────────────────────────────┘
```

---

## Spaceship System (Implemented)

### Key Classes

| Class | Role | File |
|-------|------|------|
| `AAdastreaPlayerController` | Main player controller | AdastreaPlayerController.h |
| `ASpaceship` | Spaceship pawn with physics | Spaceship.h |
| `USpaceshipControlsComponent` | Enhanced Input handler | SpaceshipControlsComponent.h |

### Initialization Order (Game Start)

```
1. UEngine::Init()
2. AAdastreaGameMode::AAdastreaGameMode()
3. AAdastreaPlayerController::AAdastreaPlayerController()
4. AAdastreaGameMode::BeginPlay()
5. AAdastreaGameMode::SpawnPlayerSpaceship()
6. ASpaceship::ASpaceship()
   - Creates MovementComponent
   - Creates ParticleComponent
   - Initializes flight parameters
7. ASpaceship::BeginPlay()
8. USpaceshipControlsComponent::BeginPlay()
   - Creates input actions
   - Creates mapping context
   - Binds input handlers
   - Enables controls
9. APlayerController::Possess(Spaceship)
10. Ready for input
```

### Input Actions

| Input | Action | Key Binding | Handler |
|-------|--------|-------------|---------|
| Move | IA_SpaceshipMove | W/A/S/D | HandleMove → OnMoveInput |
| Look | IA_SpaceshipLook | Mouse XY | HandleLook → OnLookInput |
| Fire | IA_SpaceshipFire | LMB | HandleFirePressed → OnFirePressed |
| Speed | IA_SpaceshipSpeed | Mouse Wheel | HandleSpeed → SetCurrentSpeed |

### Runtime Flow (Example: Press W)

```
Hardware (W key pressed)
    ↓
Enhanced Input System
    ↓
SpaceshipMappingContext matches W → MoveAction (Y: 1.0)
    ↓
USpaceshipControlsComponent::HandleMove()
    ↓
OnMoveInput_Implementation(MoveValue)
    ↓
APawn::AddMovementInput(UpVector * MoveValue.Y)
    ↓
ASpaceship::Tick() - Flight physics
    ↓
UFloatingPawnMovement::TickComponent() - Apply velocity
    ↓
Actor position updated
```

---

## Character System (Planned)

### Key Classes (To Be Implemented)

| Class | Role |
|-------|------|
| `AAdastreaCharacter` | Third-person character pawn |
| `UCharacterControlsComponent` | Enhanced Input handler |
| `UCharacterMovementComponent` | Built-in UE5 movement (already exists) |

### Components

```
AAdastreaCharacter
├── UCapsuleComponent (collision)
├── USkeletalMeshComponent (character mesh)
├── UCharacterMovementComponent (physics)
├── USpringArmComponent (camera boom)
├── UCameraComponent (third-person view)
├── UCharacterControlsComponent (input)
└── UInventoryComponent (items)
```

### Input Actions (Planned)

| Input | Action | Key Binding | Purpose |
|-------|--------|-------------|---------|
| Move | IA_CharacterMove | W/A/S/D | Walk/Run |
| Look | IA_CharacterLook | Mouse XY | Camera rotation |
| Jump | IA_Jump | Space | Jump |
| Sprint | IA_Sprint | Left Shift | Sprint (hold) |
| Interact | IA_Interact | E | Interact with objects |
| Crouch | IA_Crouch | C | Crouch toggle |

---

## System Transitions

### Spaceship → Character

```
1. Player presses E at docking port/exit
2. Check if docked or safe location
3. Save spaceship in SavedExternalPawn
4. Spawn AAdastreaCharacter at exit location
5. Disable spaceship controls
6. PlayerController->Possess(Character)
7. Enable character controls
8. Player now walks around
```

### Character → Spaceship

```
1. Player approaches spaceship/console
2. Press E to interact
3. Line trace detects spaceship
4. Check SavedExternalPawn is valid
5. Disable character controls
6. PlayerController->Possess(SavedExternalPawn)
7. Enable spaceship controls
8. Hide/destroy character actor
9. Player now controls spaceship
```

---

## Enhanced Input Mapping Contexts

### Context Priority

```
Higher priority processed first:

┌────────────────────────────────────┐
│ CharacterMappingContext (Priority 1) │ ← Active when character
├────────────────────────────────────┤
│ SpaceshipMappingContext (Priority 0) │ ← Active when spaceship
└────────────────────────────────────┘

Only ONE active at a time to avoid conflicts
```

### Enabling/Disabling

```cpp
// Enable spaceship controls
Subsystem->AddMappingContext(SpaceshipMappingContext, Priority);

// Disable spaceship controls
Subsystem->RemoveMappingContext(SpaceshipMappingContext);
```

---

## Component Creation Pattern

### Spaceship Component Setup

```cpp
// In Constructor
ASpaceship::ASpaceship()
{
    // Create movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = 3000.0f;
    
    // Create particle component
    ParticleComponent = CreateDefaultSubobject<USpaceshipParticleComponent>(TEXT("ParticleComponent"));
    
    // Initialize flight parameters
    bFlightAssistEnabled = true;
    ThrottlePercentage = 0.0f;
}

// In BeginPlay
void ASpaceship::BeginPlay()
{
    Super::BeginPlay();
    
    // Spawn interior
    InteriorInstance = GetWorld()->SpawnActor<ASpaceshipInterior>(...);
}
```

---

## Input Handler Pattern

### BlueprintNativeEvent Implementation

```cpp
// Header (.h)
UFUNCTION(BlueprintNativeEvent, Category="Controls")
void OnMoveInput(FVector2D MoveValue);

// Implementation (.cpp)
void USpaceshipControlsComponent::OnMoveInput_Implementation(FVector2D MoveValue)
{
    APawn* OwningPawn = Cast<APawn>(GetOwner());
    if (!OwningPawn) return;
    
    const FVector RightVector = OwningPawn->GetActorRightVector();
    const FVector UpVector = OwningPawn->GetActorUpVector();
    const FVector MoveDirection = (RightVector * MoveValue.X) + (UpVector * MoveValue.Y);
    
    OwningPawn->AddMovementInput(MoveDirection, 1.0f);
}
```

**Benefits**:
- Default C++ implementation
- Can be overridden in Blueprint
- No recompilation needed for design changes

---

## Flight Physics (Spaceship)

### Flight Assist System

```cpp
if (bFlightAssistEnabled)
{
    ApplyFlightAssist(DeltaTime);
    UpdateThrottleVelocity(DeltaTime);
    
    if (no rotation input)
    {
        ApplyAutoLeveling(DeltaTime);
    }
}
```

### Key Parameters

| Parameter | Default | Purpose |
|-----------|---------|---------|
| `bFlightAssistEnabled` | true | Enable smooth rotation damping |
| `RotationDampingFactor` | 0.85 | Rotation smoothness (0-1) |
| `AutoLevelStrength` | 0.5 | Roll leveling strength (0-1) |
| `ThrottlePercentage` | 0.0 | Current throttle (0-100%) |
| `ThrottleStep` | 10.0 | Throttle increment per step |

---

## Debugging

### Common Issues

| Issue | Check | Solution |
|-------|-------|----------|
| No input response | Mapping context added? | Verify `AddMappingContext()` called |
| Wrong input direction | Axis modifiers | Check SwizzleAxis, Negate modifiers |
| Jittery movement | Damping values | Adjust RotationDampingFactor |
| Can't transition | SavedExternalPawn | Verify pawn reference is valid |

### Logging

```cpp
// Enable verbose input logging
LogAdastreaInput category (Verbose level)

// Check Enhanced Input state
Console: showdebug enhancedinput
```

---

## File Locations

### Source Code

```
Source/Adastrea/
├── Player/
│   ├── AdastreaPlayerController.h/.cpp
│   └── ... (other player components)
├── Ships/
│   ├── Spaceship.h/.cpp
│   ├── SpaceshipControlsComponent.h/.cpp
│   └── ... (other ship components)
└── AdastreaGameMode.h/.cpp
```

### Documentation

```
Docs/
├── PLAYER_CONTROLS_WORKFLOW.md (detailed workflows)
├── PLAYER_CONTROLS_DIAGRAMS.md (visual diagrams)
└── PLAYER_CONTROLS_QUICK_REFERENCE.md (this file)
```

### Related Docs

```
Root/
├── ENHANCED_INPUT_GUIDE.md (manual input setup)
├── ARCHITECTURE.md (system architecture)
└── CODE_STYLE.md (coding standards)

Assets/
└── EnhancedInputImplementation.md (C++ approach)
```

---

## Testing Checklist

### Spaceship Controls

- [ ] Spaceship spawns on game start
- [ ] WASD keys move spaceship (strafe)
- [ ] Mouse movement rotates spaceship
- [ ] Left mouse button fires weapon
- [ ] Mouse wheel adjusts speed
- [ ] Flight assist provides smooth control
- [ ] Throttle system works correctly

### Character Controls (When Implemented)

- [ ] Character spawns when exiting ship
- [ ] WASD keys move character
- [ ] Mouse rotates camera
- [ ] Space bar makes character jump
- [ ] Shift key enables sprint
- [ ] E key triggers interactions
- [ ] C key toggles crouch

### Transitions

- [ ] Exit ship creates character
- [ ] Board ship restores spaceship
- [ ] Console activates ship control from interior
- [ ] Exit console returns to walking
- [ ] No memory leaks on transitions

---

## Performance Notes

- Spaceship Tick only runs when possessed (checks `GetController()`)
- Weapon component cached on BeginPlay
- Input events use Enhanced Input (efficient)
- CharacterMovementComponent optimized by UE5

---

**Quick Links**:
- [Full Workflow Documentation](PLAYER_CONTROLS_WORKFLOW.md)
- [Visual Diagrams](PLAYER_CONTROLS_DIAGRAMS.md)
- [Enhanced Input Guide](../ENHANCED_INPUT_GUIDE.md)
- [Architecture Overview](../ARCHITECTURE.md)

---

**Version**: 1.0  
**Last Updated**: 2025-12-05  
**Status**: Spaceship implemented, Character planned
