# Player Controls Workflow Documentation

This document provides detailed C++ flowcharts and workflow diagrams for Adastrea's player control systems.

## Table of Contents

1. [Spaceship Control System](#spaceship-control-system)
   - [Initialization Flow](#spaceship-initialization-flow)
   - [Runtime Input Flow](#spaceship-runtime-input-flow)
   - [Component Relationships](#spaceship-component-relationships)
2. [Third-Person Character System](#third-person-character-system)
   - [Planned Architecture](#character-planned-architecture)
   - [Initialization Flow](#character-initialization-flow)
   - [Runtime Input Flow](#character-runtime-input-flow)
3. [System Transitions](#system-transitions)

---

## Spaceship Control System

The spaceship control system uses Enhanced Input and consists of three primary classes:
- **AAdastreaPlayerController**: Main player controller
- **ASpaceship**: Spaceship pawn with movement
- **USpaceshipControlsComponent**: Enhanced Input component for spaceship controls

### Spaceship Initialization Flow

```
GAME START (UE5 Engine)
    ↓
┌─────────────────────────────────────────────────────────────────┐
│ ENGINE INITIALIZATION                                           │
│ • UEngine::Init()                                               │
│ • Load World                                                    │
│ • Initialize Game Framework                                     │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ GAME MODE INITIALIZATION                                        │
│ AAdastreaGameMode::AAdastreaGameMode() - Constructor            │
│ • Set bAutoSpawnPlayerShip = true                               │
│ • Set DefaultSpaceshipClass = nullptr (set in Blueprint)        │
│ • Set bSpawnAtCenter = false                                    │
│ • Set FallbackSpawnLocation = FVector::ZeroVector              │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ PLAYER CONTROLLER SPAWN                                         │
│ UWorld::SpawnPlayActor()                                        │
│ • Engine creates AAdastreaPlayerController instance             │
│ • AAdastreaPlayerController::AAdastreaPlayerController()        │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ GAME MODE BEGIN PLAY                                            │
│ AAdastreaGameMode::BeginPlay()                                  │
│ ├─ If (bShowTestSettingsOnStartup && TestSettingsWidgetClass)  │
│ │   └─→ ShowTestSettingsWidget()                               │
│ │       └─→ Wait for user input → OnTestSettingsContinue()    │
│ └─ Else: OnTestSettingsContinue()                              │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ SPACESHIP SPAWNING                                              │
│ AAdastreaGameMode::SpawnPlayerSpaceship()                       │
│ ├─ Find ASpaceSectorMap in level                               │
│ ├─ Determine spawn location:                                   │
│ │  ├─ If sector map exists:                                    │
│ │  │  ├─ If bSpawnAtCenter: GetSectorCenter()                 │
│ │  │  └─ Else: GetRandomPositionInSector()                    │
│ │  └─ Else: Use FallbackSpawnLocation                         │
│ ├─ Set FActorSpawnParameters                                   │
│ └─ GetWorld()->SpawnActor<ASpaceship>(...)                    │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ SPACESHIP CONSTRUCTION                                          │
│ ASpaceship::ASpaceship() - Constructor                          │
│ ├─ Create MovementComponent (UFloatingPawnMovement)            │
│ │  └─ Set MaxSpeed, Acceleration, Deceleration, TurningBoost  │
│ ├─ Create ParticleComponent (USpaceshipParticleComponent)      │
│ ├─ Initialize X4-style flight parameters:                      │
│ │  ├─ bFlightAssistEnabled = true                              │
│ │  ├─ RotationDampingFactor = 0.85f                           │
│ │  ├─ AutoLevelStrength = 0.5f                                │
│ │  ├─ ThrottlePercentage = 0.0f                               │
│ │  └─ ThrottleStep = 10.0f                                    │
│ └─ Initialize physics state (velocities, inputs)               │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ SPACESHIP BEGIN PLAY                                            │
│ ASpaceship::BeginPlay()                                         │
│ ├─ Initialize hull integrity from ShipDataAsset                │
│ ├─ Spawn InteriorInstance (ASpaceshipInterior)                 │
│ │  ├─ SpawnActor<ASpaceshipInterior>()                        │
│ │  ├─ AttachToActor(this, ...)                                │
│ │  └─ SetActorHiddenInGame(true)                              │
│ └─ Enable PrimaryActorTick.bCanEverTick = true                │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ SPACESHIP CONTROLS COMPONENT INITIALIZATION                     │
│ (If USpaceshipControlsComponent attached to spaceship BP)       │
│ USpaceshipControlsComponent::BeginPlay()                        │
│ ├─ CreateInputActions()                                         │
│ │  ├─ Create MoveAction (IA_SpaceshipMove, Axis2D)            │
│ │  ├─ Create LookAction (IA_SpaceshipLook, Axis2D)            │
│ │  ├─ Create FireAction (IA_SpaceshipFire, Boolean)           │
│ │  └─ Create SpeedAction (IA_SpaceshipSpeed, Axis1D)          │
│ ├─ CreateInputMappingContext()                                 │
│ │  ├─ Create SpaceshipMappingContext (IMC_SpaceshipControls)  │
│ │  ├─ Map W/S keys to MoveAction (vertical strafe)            │
│ │  ├─ Map A/D keys to MoveAction (horizontal strafe)          │
│ │  ├─ Map Mouse2D to LookAction                               │
│ │  ├─ Map LeftMouseButton to FireAction                       │
│ │  └─ Map MouseWheelAxis to SpeedAction                       │
│ ├─ Cache WeaponComponent reference                             │
│ ├─ Cast owner to APawn                                         │
│ ├─ Cast InputComponent to UEnhancedInputComponent              │
│ ├─ SetupInputBindings(EnhancedInputComponent)                  │
│ │  ├─ BindAction(MoveAction, ETriggerEvent::Triggered, ...)   │
│ │  ├─ BindAction(LookAction, ETriggerEvent::Triggered, ...)   │
│ │  ├─ BindAction(FireAction, Started/Completed, ...)          │
│ │  └─ BindAction(SpeedAction, ETriggerEvent::Triggered, ...) │
│ └─ EnableControls()                                             │
│    └─ AddMappingContext(SpaceshipMappingContext, Priority)    │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ PLAYER CONTROLLER POSSESSION                                    │
│ AAdastreaGameMode::SpawnPlayerSpaceship() (continued)           │
│ ├─ Get PlayerController (index 0)                              │
│ └─ PlayerController->Possess(SpaceshipActor)                   │
│    ├─ APawn::PossessedBy(Controller)                           │
│    └─ ASpaceship::SetupPlayerInputComponent(InputComponent)    │
│       └─ Bind legacy input axes (MoveForward, MoveRight, etc.) │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ PLAYER CONTROLLER BEGIN PLAY                                    │
│ AAdastreaPlayerController::BeginPlay()                          │
│ └─ SetupInputComponent()                                        │
│    └─ (Note: Actual bindings done via Enhanced Input/Blueprint)│
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ READY FOR INPUT                                                 │
│ • Enhanced Input subsystem active                               │
│ • SpaceshipControlsComponent listening for input                │
│ • ASpaceship Tick() running flight physics                     │
│ • Player can now control spaceship                             │
└─────────────────────────────────────────────────────────────────┘
```

### Call Order Summary (Game Start to First Input)

```
1. UEngine::Init()
2. AAdastreaGameMode::AAdastreaGameMode()
3. UWorld::SpawnPlayActor() → AAdastreaPlayerController spawned
4. AAdastreaPlayerController::AAdastreaPlayerController()
5. AAdastreaGameMode::BeginPlay()
6. AAdastreaGameMode::OnTestSettingsContinue()
7. AAdastreaGameMode::SpawnPlayerSpaceship()
8. UWorld::SpawnActor<ASpaceship>()
9. ASpaceship::ASpaceship()
   - Creates MovementComponent
   - Creates ParticleComponent
   - Initializes flight parameters
10. ASpaceship::BeginPlay()
    - Spawns InteriorInstance
    - Initializes hull from DataAsset
11. USpaceshipControlsComponent::BeginPlay()
    - Creates input actions
    - Creates mapping context
    - Binds input handlers
    - Enables controls (adds mapping context)
12. APlayerController::Possess(Spaceship)
13. ASpaceship::SetupPlayerInputComponent()
14. AAdastreaPlayerController::BeginPlay()
15. AAdastreaPlayerController::SetupInputComponent()
16. ✓ System Ready - Player input active
```

### Spaceship Runtime Input Flow

```
PLAYER INPUT (e.g., Press W key)
    ↓
┌─────────────────────────────────────────────────────────────────┐
│ ENHANCED INPUT SYSTEM                                           │
│ UEnhancedInputLocalPlayerSubsystem                              │
│ ├─ Input event captured from hardware                          │
│ ├─ Match key to registered InputMappingContext                 │
│ │  └─ SpaceshipMappingContext (Priority: 0)                   │
│ ├─ Find mapped InputAction                                     │
│ │  └─ MoveAction (W key mapped to Y: 1.0)                     │
│ ├─ Apply modifiers (SwizzleAxis, Negate, etc.)                │
│ └─ Trigger appropriate ETriggerEvent                           │
│    └─ ETriggerEvent::Triggered for axis input                 │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ INPUT COMPONENT CALLBACK                                        │
│ USpaceshipControlsComponent::HandleMove()                       │
│ ├─ Receive FInputActionValue from Enhanced Input               │
│ ├─ Extract FVector2D: Value.Get<FVector2D>()                   │
│ ├─ Apply multipliers:                                          │
│ │  └─ MoveValue = Value * MovementSpeed * CurrentSpeed        │
│ └─ Call BlueprintNativeEvent:                                  │
│    └─ OnMoveInput(MoveValue)                                   │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ MOVEMENT INPUT PROCESSING                                       │
│ USpaceshipControlsComponent::OnMoveInput_Implementation()       │
│ ├─ Cast GetOwner() to APawn                                    │
│ ├─ Get pawn vectors:                                           │
│ │  ├─ RightVector = GetActorRightVector()                     │
│ │  └─ UpVector = GetActorUpVector()                           │
│ ├─ Calculate movement direction:                               │
│ │  └─ MoveDir = (Right * MoveValue.X) + (Up * MoveValue.Y)   │
│ └─ Apply to pawn:                                              │
│    └─ OwningPawn->AddMovementInput(MoveDirection, 1.0f)       │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ PAWN MOVEMENT PROCESSING                                        │
│ APawn::AddMovementInput()                                       │
│ ├─ Accumulate input vector in ControlInputVector               │
│ └─ ControlInputVector += WorldDirection * ScaleValue           │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ TICK - FLIGHT PHYSICS UPDATE                                    │
│ ASpaceship::Tick(DeltaTime)                                     │
│ ├─ Check if possessed (GetController() != nullptr)             │
│ ├─ If bFlightAssistEnabled:                                    │
│ │  └─ ApplyFlightAssist(DeltaTime)                            │
│ │     ├─ Update RotationVelocity with damping                 │
│ │     └─ Interpolate rotation smoothly                        │
│ ├─ UpdateThrottleVelocity(DeltaTime)                          │
│ │  ├─ Calculate target velocity from throttle %               │
│ │  ├─ Apply boost/travel multipliers                          │
│ │  └─ Interpolate current velocity toward target              │
│ ├─ If auto-leveling && no rotation input:                     │
│ │  └─ ApplyAutoLeveling(DeltaTime)                            │
│ │     └─ Gradually level ship roll to ecliptic plane          │
│ └─ Movement component handles velocity application             │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ MOVEMENT COMPONENT UPDATE                                       │
│ UFloatingPawnMovement::TickComponent()                          │
│ ├─ Apply ControlInputVector to velocity                        │
│ ├─ Apply acceleration/deceleration                             │
│ ├─ Clamp velocity to MaxSpeed                                  │
│ ├─ Calculate new position:                                     │
│ │  └─ NewLocation = CurrentLocation + (Velocity * DeltaTime)  │
│ ├─ Perform collision detection                                 │
│ └─ Update actor location                                       │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ VISUAL UPDATE                                                   │
│ • Actor transform updated                                       │
│ • Renderer updates spaceship position                          │
│ • ParticleComponent updates engine effects                     │
│ • Camera follows ship (if attached)                            │
└─────────────────────────────────────────────────────────────────┘
```

### Look Input Flow (Mouse Movement)

```
MOUSE MOVEMENT
    ↓
Enhanced Input System
    ↓
SpaceshipMappingContext matches Mouse2D → LookAction
    ↓
USpaceshipControlsComponent::HandleLook(InputActionValue)
    ├─ Extract FVector2D: Value.Get<FVector2D>()
    ├─ Apply LookSensitivity multiplier
    ├─ Apply Y-axis inversion if bInvertLookY == true
    └─ Call OnLookInput(LookValue)
        ↓
USpaceshipControlsComponent::OnLookInput_Implementation(LookValue)
    ├─ Cast GetOwner() to APawn
    ├─ Apply rotation to controller:
    │  ├─ AddControllerYawInput(LookValue.X)  // Left/Right
    │  └─ AddControllerPitchInput(LookValue.Y) // Up/Down
    └─ Controller rotation affects pawn orientation
        ↓
ASpaceship::Tick() - Rotation damping applied
    └─ If bFlightAssistEnabled: ApplyFlightAssist()
        └─ Smooth rotation with RotationDampingFactor
```

### Fire Input Flow (Left Mouse Button)

```
LEFT MOUSE BUTTON PRESS
    ↓
Enhanced Input System
    ↓
SpaceshipMappingContext matches LeftMouseButton → FireAction
    ↓
ETriggerEvent::Started triggered
    ↓
USpaceshipControlsComponent::HandleFirePressed()
    └─ Call OnFirePressed()
        ↓
USpaceshipControlsComponent::OnFirePressed_Implementation()
    ├─ Get cached weapon component
    ├─ If not cached: GetWeaponComponent()
    │  └─ FindComponentByClass<UWeaponComponent>()
    └─ If weapon found: CachedWeaponComponent->Fire()
        ↓
UWeaponComponent::Fire()
    └─ Execute weapon firing logic (projectile spawn, etc.)

LEFT MOUSE BUTTON RELEASE
    ↓
ETriggerEvent::Completed triggered
    ↓
USpaceshipControlsComponent::HandleFireReleased()
    └─ Call OnFireReleased()
        └─ OnFireReleased_Implementation() (default: no-op)
```

### Speed Adjustment Flow (Mouse Wheel)

```
MOUSE WHEEL SCROLL
    ↓
Enhanced Input System
    ↓
SpaceshipMappingContext matches MouseWheelAxis → SpeedAction
    ↓
USpaceshipControlsComponent::HandleSpeed(InputActionValue)
    ├─ Extract float: Value.Get<float>()
    ├─ If ScrollValue > 0: IncreaseSpeed()
    │  └─ SetCurrentSpeed(CurrentSpeed + SpeedStep)
    └─ If ScrollValue < 0: DecreaseSpeed()
       └─ SetCurrentSpeed(CurrentSpeed - SpeedStep)
           ↓
USpaceshipControlsComponent::SetCurrentSpeed(NewSpeed)
    ├─ Clamp to [MinSpeed, MaxSpeed]
    ├─ Calculate delta = NewSpeed - OldSpeed
    ├─ Call OnSpeedChanged(NewSpeed, Delta)
    ├─ Broadcast OnSpeedChangedEvent
    └─ CurrentSpeed affects movement multiplier in HandleMove()
```

### Spaceship Component Relationships

```
┌─────────────────────────────────────────────────────────────────┐
│ AAdastreaPlayerController                                       │
│ • Manages player state                                          │
│ • Handles possession                                            │
│ • Delegates input to possessed pawn                             │
└────────────────────┬────────────────────────────────────────────┘
                     │ Possesses
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ ASpaceship (Pawn)                                               │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ Components:                                                  │ │
│ │ • UFloatingPawnMovement - Physics-based movement            │ │
│ │ • USpaceshipParticleComponent - Engine effects              │ │
│ │ • USpaceshipControlsComponent - Enhanced Input              │ │
│ │ • UWeaponComponent - Weapon firing                          │ │
│ │ • UCombatComponent - Combat logic (optional)                │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
│ Properties:                                                     │
│ • ShipDataAsset - Configuration (stats, appearance)            │
│ • InteriorInstance - Interior space for boarding               │
│ • Flight physics state (velocity, throttle, etc.)              │
│                                                                 │
│ Methods:                                                        │
│ • Tick() - Flight physics update every frame                   │
│ • SetupPlayerInputComponent() - Legacy input binding           │
│ • BeginControl() / EndControl() - Interior transition          │
└─────────────────────────────────────────────────────────────────┘
                     │
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ USpaceshipControlsComponent                                     │
│ • Creates and manages Enhanced Input Actions                    │
│ • Creates and manages Input Mapping Context                     │
│ • Handles all spaceship input callbacks                         │
│ • Provides Blueprint events for override                        │
│ • Auto-enables controls on BeginPlay                           │
│ • Communicates with weapon/movement components                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Third-Person Character System

**Status**: Not yet implemented. This section describes the planned architecture.

The third-person character system is intended for when the player exits the spaceship and walks on stations, planets, or ship interiors. It follows Unreal Engine 5 best practices for third-person character control.

### Character Planned Architecture

**Key Classes (To Be Implemented)**:
- **AAdastreaCharacter**: Character pawn for walking/running
- **UCharacterControlsComponent**: Enhanced Input component for character
- **UCharacterMovementComponent**: Built-in UE5 character movement (already exists)

### Character Initialization Flow

```
TRANSITION FROM SPACESHIP TO CHARACTER
    ↓
┌─────────────────────────────────────────────────────────────────┐
│ PLAYER EXITS SPACESHIP                                          │
│ • Player triggers "Exit Ship" action (E key)                    │
│ • Spaceship interior entered OR                                 │
│ • Spaceship docked at station/planet                            │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ CHARACTER SPAWN                                                 │
│ ASpaceship::BeginControl() OR ExitToExterior()                  │
│ ├─ Determine exit location (interior or exterior)              │
│ ├─ Set spawn parameters                                        │
│ └─ GetWorld()->SpawnActor<AAdastreaCharacter>(...)             │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ CHARACTER CONSTRUCTION                                          │
│ AAdastreaCharacter::AAdastreaCharacter()                        │
│ ├─ Create CapsuleComponent (collision)                         │
│ ├─ Create SkeletalMeshComponent (character mesh)               │
│ ├─ Create CharacterMovementComponent (UE5 built-in)            │
│ │  ├─ Set walking/running speeds                               │
│ │  ├─ Set jump height                                          │
│ │  ├─ Set gravity scale                                        │
│ │  └─ Configure movement modes (walking, falling, swimming)    │
│ ├─ Create SpringArmComponent (camera boom)                     │
│ │  ├─ Set target arm length (distance from character)          │
│ │  ├─ Enable camera lag for smooth follow                      │
│ │  └─ Configure collision testing                              │
│ ├─ Create CameraComponent (third-person view)                  │
│ │  └─ Attach to SpringArmComponent                             │
│ └─ Initialize character properties:                            │
│    ├─ BaseTurnRate, BaseLookUpRate                            │
│    ├─ bUseControllerRotationYaw = false                        │
│    └─ CharacterMovement->bOrientRotationToMovement = true     │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ CHARACTER BEGIN PLAY                                            │
│ AAdastreaCharacter::BeginPlay()                                 │
│ ├─ Initialize health/stamina from DataAsset (if any)           │
│ ├─ Setup animation blueprint                                   │
│ └─ Initialize inventory/equipment systems                       │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ CHARACTER CONTROLS COMPONENT INITIALIZATION                     │
│ UCharacterControlsComponent::BeginPlay()                        │
│ ├─ CreateInputActions()                                         │
│ │  ├─ Create MoveAction (IA_CharacterMove, Axis2D)            │
│ │  ├─ Create LookAction (IA_CharacterLook, Axis2D)            │
│ │  ├─ Create JumpAction (IA_Jump, Boolean)                    │
│ │  ├─ Create SprintAction (IA_Sprint, Boolean)                │
│ │  ├─ Create InteractAction (IA_Interact, Boolean)            │
│ │  └─ Create CrouchAction (IA_Crouch, Boolean)                │
│ ├─ CreateInputMappingContext()                                 │
│ │  ├─ Create CharacterMappingContext (IMC_Character)           │
│ │  ├─ Map W/A/S/D keys to MoveAction                          │
│ │  ├─ Map Mouse2D to LookAction                               │
│ │  ├─ Map Space to JumpAction                                 │
│ │  ├─ Map LeftShift to SprintAction                           │
│ │  ├─ Map E to InteractAction                                 │
│ │  └─ Map C to CrouchAction                                   │
│ ├─ Cache CharacterMovementComponent reference                  │
│ ├─ SetupInputBindings(EnhancedInputComponent)                  │
│ │  ├─ BindAction(MoveAction, Triggered, HandleMove)           │
│ │  ├─ BindAction(LookAction, Triggered, HandleLook)           │
│ │  ├─ BindAction(JumpAction, Started/Completed, ...)          │
│ │  ├─ BindAction(SprintAction, Started/Completed, ...)        │
│ │  ├─ BindAction(InteractAction, Started, HandleInteract)     │
│ │  └─ BindAction(CrouchAction, Started, HandleCrouch)         │
│ └─ EnableControls()                                             │
│    └─ AddMappingContext(CharacterMappingContext, Priority)    │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ PLAYER CONTROLLER POSSESSION                                    │
│ AAdastreaPlayerController::Possess(CharacterPawn)               │
│ ├─ Unpossess current pawn (spaceship)                          │
│ ├─ Save reference to spaceship in SavedExternalPawn            │
│ ├─ Possess new character pawn                                  │
│ └─ Character->SetupPlayerInputComponent() called               │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ READY FOR CHARACTER CONTROL                                     │
│ • Enhanced Input subsystem active                               │
│ • CharacterControlsComponent listening for input                │
│ • CharacterMovementComponent handling physics                   │
│ • Player can now walk, jump, interact                          │
└─────────────────────────────────────────────────────────────────┘
```

### Call Order Summary (Spaceship to Character Transition)

```
1. Player triggers exit action (e.g., press E at docking port)
2. ASpaceship::ExitToExterior() or BeginControl() reversed
3. UWorld::SpawnActor<AAdastreaCharacter>()
4. AAdastreaCharacter::AAdastreaCharacter()
   - Creates CapsuleComponent
   - Creates SkeletalMeshComponent
   - Creates CharacterMovementComponent (UE5 built-in)
   - Creates SpringArmComponent (camera boom)
   - Creates CameraComponent
   - Initializes movement properties
5. AAdastreaCharacter::BeginPlay()
   - Initializes stats from DataAsset
   - Sets up animation blueprint
6. UCharacterControlsComponent::BeginPlay()
   - Creates input actions for character
   - Creates character mapping context
   - Binds input handlers
   - Enables controls
7. AAdastreaPlayerController::Possess(Character)
   - Saves spaceship reference in SavedExternalPawn
   - Transfers control to character
8. AAdastreaCharacter::SetupPlayerInputComponent()
9. ✓ Character System Ready - Player can walk/run/jump
```

### Character Runtime Input Flow

```
PLAYER INPUT (e.g., Press W key)
    ↓
┌─────────────────────────────────────────────────────────────────┐
│ ENHANCED INPUT SYSTEM                                           │
│ UEnhancedInputLocalPlayerSubsystem                              │
│ ├─ Input captured from hardware                                │
│ ├─ Match key to CharacterMappingContext                        │
│ ├─ Find mapped InputAction (MoveAction)                        │
│ └─ Trigger ETriggerEvent::Triggered                            │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ INPUT COMPONENT CALLBACK                                        │
│ UCharacterControlsComponent::HandleMove()                       │
│ ├─ Receive FInputActionValue                                   │
│ ├─ Extract FVector2D: Value.Get<FVector2D>()                   │
│ ├─ Apply sprint multiplier if bIsSprinting                     │
│ └─ Call OnMoveInput(MoveValue)                                 │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ MOVEMENT INPUT PROCESSING                                       │
│ UCharacterControlsComponent::OnMoveInput_Implementation()       │
│ ├─ Cast GetOwner() to ACharacter                               │
│ ├─ Get camera/controller rotation for forward direction        │
│ ├─ Calculate movement direction relative to camera:            │
│ │  ├─ ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(X)│
│ │  └─ RightDir = FRotationMatrix(YawRotation).GetUnitAxis(Y)  │
│ ├─ Apply input to character:                                   │
│ │  ├─ AddMovementInput(ForwardDir, MoveValue.Y)               │
│ │  └─ AddMovementInput(RightDir, MoveValue.X)                 │
│ └─ CharacterMovementComponent processes input automatically    │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ CHARACTER MOVEMENT COMPONENT UPDATE                             │
│ UCharacterMovementComponent::TickComponent()                    │
│ ├─ Calculate desired velocity from input                       │
│ ├─ Apply acceleration/friction                                 │
│ ├─ Handle movement mode (walking, falling, jumping, etc.)      │
│ ├─ Perform collision detection and resolution                  │
│ ├─ Apply gravity if falling                                    │
│ ├─ Update capsule component position                           │
│ └─ Rotate character mesh to movement direction (if configured) │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ ANIMATION SYSTEM UPDATE                                         │
│ Animation Blueprint (UAnimInstance)                             │
│ ├─ Read movement data from CharacterMovementComponent:         │
│ │  ├─ GetVelocity().Size() → Speed                            │
│ │  ├─ IsFalling() → bIsInAir                                  │
│ │  └─ IsCrouching() → bIsCrouched                             │
│ ├─ Update blend spaces for walk/run/sprint animations          │
│ └─ Trigger state machine transitions (idle/walk/run/jump)      │
└────────────────────┬────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────────┐
│ VISUAL UPDATE                                                   │
│ • Character position and rotation updated                       │
│ • Skeletal mesh animated                                        │
│ • Camera follows character via SpringArm                        │
│ • Renderer updates scene                                        │
└─────────────────────────────────────────────────────────────────┘
```

### Look Input Flow (Mouse Movement - Character)

```
MOUSE MOVEMENT
    ↓
Enhanced Input System
    ↓
CharacterMappingContext matches Mouse2D → LookAction
    ↓
UCharacterControlsComponent::HandleLook(InputActionValue)
    ├─ Extract FVector2D: Value.Get<FVector2D>()
    ├─ Apply LookSensitivity multiplier
    └─ Call OnLookInput(LookValue)
        ↓
UCharacterControlsComponent::OnLookInput_Implementation(LookValue)
    ├─ Cast GetOwner() to ACharacter
    ├─ Add controller input for camera rotation:
    │  ├─ AddControllerYawInput(LookValue.X * BaseTurnRate * DeltaTime)
    │  └─ AddControllerPitchInput(LookValue.Y * BaseLookUpRate * DeltaTime)
    └─ Camera rotates on SpringArm
        └─ Character mesh optionally rotates to face movement direction
```

### Jump Input Flow

```
SPACE BAR PRESS
    ↓
Enhanced Input System → JumpAction → ETriggerEvent::Started
    ↓
UCharacterControlsComponent::HandleJumpPressed()
    └─ Call OnJumpPressed()
        ↓
UCharacterControlsComponent::OnJumpPressed_Implementation()
    └─ Cast GetOwner() to ACharacter
        └─ Character->Jump()
            ↓
ACharacter::Jump()
    ├─ Check CanJump() (on ground, not crouched, etc.)
    ├─ Set bPressedJump = true
    └─ CharacterMovementComponent->DoJump()
        ├─ Apply jump velocity (vertical impulse)
        ├─ Change movement mode to Falling
        └─ Trigger jump animation

SPACE BAR RELEASE
    ↓
Enhanced Input System → JumpAction → ETriggerEvent::Completed
    ↓
UCharacterControlsComponent::HandleJumpReleased()
    └─ Character->StopJumping()
        └─ Allows variable jump height based on hold duration
```

### Sprint Input Flow

```
LEFT SHIFT PRESS (Hold)
    ↓
Enhanced Input System → SprintAction → ETriggerEvent::Started
    ↓
UCharacterControlsComponent::HandleSprintPressed()
    └─ Call OnSprintPressed()
        ↓
UCharacterControlsComponent::OnSprintPressed_Implementation()
    ├─ Set bIsSprinting = true
    └─ Update movement speed:
        └─ CharacterMovement->MaxWalkSpeed *= SprintMultiplier

LEFT SHIFT RELEASE
    ↓
Enhanced Input System → SprintAction → ETriggerEvent::Completed
    ↓
UCharacterControlsComponent::HandleSprintReleased()
    └─ Call OnSprintReleased()
        └─ Set bIsSprinting = false
            └─ CharacterMovement->MaxWalkSpeed = DefaultWalkSpeed
```

### Interact Input Flow

```
E KEY PRESS
    ↓
Enhanced Input System → InteractAction → ETriggerEvent::Started
    ↓
UCharacterControlsComponent::HandleInteract()
    └─ Call OnInteract()
        ↓
UCharacterControlsComponent::OnInteract_Implementation()
    ├─ Perform line trace from camera forward
    ├─ Check for interactable actors (IInteractable interface)
    ├─ If found:
    │  └─ Call actor->Interact(Character)
    │     └─ Examples:
    │        • Door opens
    │        • Item picked up
    │        • Console activated
    │        • Enter spaceship (transition back to ship)
    └─ If interactable is ASpaceship:
       └─ Transition back to spaceship control system
```

---

## System Transitions

### Transition: Spaceship → Character (Exiting Ship)

```
1. Player presses "Exit Ship" action (E key) while near door/hatch
2. ASpaceship checks if docked or at safe location
3. If safe:
   a. Save current spaceship in SavedExternalPawn
   b. Spawn AAdastreaCharacter at exit location
   c. AAdastreaPlayerController->Possess(Character)
   d. Disable spaceship controls (USpaceshipControlsComponent::DisableControls())
   e. Enable character controls (UCharacterControlsComponent::EnableControls())
4. Player now controls character
```

### Transition: Character → Spaceship (Entering Ship)

```
1. Player approaches spaceship door/console
2. Press "Interact" (E key)
3. Line trace detects IInteractable on spaceship or console
4. Interaction triggers:
   a. Check if SavedExternalPawn is valid (the spaceship)
   b. AAdastreaPlayerController->Possess(SavedExternalPawn)
   c. Disable character controls (UCharacterControlsComponent::DisableControls())
   d. Enable spaceship controls (USpaceshipControlsComponent::EnableControls())
   e. Destroy or hide character actor
5. Player now controls spaceship
```

### Interior → Spaceship Control (Sitting at Console)

```
1. Player (as character) walks to ship control console inside interior
2. Press "Interact" (E key) at console
3. Console actor (AShipControlConsole) detects interaction
4. Call ASpaceship::BeginControl(PlayerController, CharacterPawn)
   a. Save CharacterPawn in SavedExternalPawn
   b. Hide CharacterPawn (don't destroy)
   c. PlayerController->Possess(Spaceship)
   d. Enable spaceship controls
   e. Show spaceship exterior view (switch camera)
5. Player controls spaceship from interior perspective
6. To exit console: Press "Exit" → EndControl() → return to walking character
```

### Component Priority System (Enhanced Input)

When both systems exist, priority determines which input mapping context takes precedence:

```
┌─────────────────────────────────────────────────────────────────┐
│ ENHANCED INPUT MAPPING CONTEXT PRIORITY                         │
│                                                                 │
│ Higher Priority = Processed First                               │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ CharacterMappingContext (Priority: 1)                       │ │
│ │ • Active when player controls character                     │ │
│ │ • Handles WASD, mouse, jump, sprint, interact              │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ SpaceshipMappingContext (Priority: 0)                       │ │
│ │ • Active when player controls spaceship                     │ │
│ │ • Handles WASD, mouse, fire, speed adjust                   │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
│ When transitioning:                                             │
│ • Disable old context: RemoveMappingContext(...)               │
│ • Enable new context: AddMappingContext(..., Priority)         │
│ • Only one active at a time to avoid input conflicts           │
└─────────────────────────────────────────────────────────────────┘
```

---

## Class Hierarchy Summary

```
┌─────────────────────────────────────────────────────────────────┐
│ SPACESHIP SYSTEM                                                │
│                                                                 │
│ AAdastreaPlayerController (APlayerController)                   │
│     └─ Possesses → ASpaceship (APawn)                          │
│         ├─ UFloatingPawnMovement                                │
│         ├─ USpaceshipParticleComponent                          │
│         ├─ USpaceshipControlsComponent (UActorComponent)        │
│         ├─ UWeaponComponent (optional)                          │
│         └─ UCombatComponent (optional)                          │
│                                                                 │
│ Data Assets:                                                    │
│ • USpaceshipDataAsset - Ship configuration                      │
│ • UInputConfigDataAsset - Input configuration (C++ approach)    │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ CHARACTER SYSTEM (Planned)                                      │
│                                                                 │
│ AAdastreaPlayerController (APlayerController)                   │
│     └─ Possesses → AAdastreaCharacter (ACharacter)              │
│         ├─ UCapsuleComponent                                    │
│         ├─ USkeletalMeshComponent                               │
│         ├─ UCharacterMovementComponent (UE5 built-in)           │
│         ├─ USpringArmComponent                                  │
│         ├─ UCameraComponent                                     │
│         ├─ UCharacterControlsComponent (UActorComponent)        │
│         └─ UInventoryComponent (optional)                       │
│                                                                 │
│ Data Assets:                                                    │
│ • UCharacterDataAsset - Character stats (to be created)         │
│ • UInputConfigDataAsset - Input configuration                   │
└─────────────────────────────────────────────────────────────────┘
```

---

## Key Design Patterns

### 1. Component-Based Controls
Both systems use dedicated `UActorComponent` subclasses for input handling:
- `USpaceshipControlsComponent` for spaceships
- `UCharacterControlsComponent` for characters (planned)

**Benefits**:
- Separation of concerns
- Reusable across different pawn types
- Easy to enable/disable controls
- Blueprint-friendly customization

### 2. Enhanced Input System
All input uses Unreal Engine 5's Enhanced Input:
- Input Actions (IA_*) - Abstract input events
- Input Mapping Contexts (IMC_*) - Key bindings
- Input Modifiers - Transform raw input
- Input Triggers - Define when actions fire

**Benefits**:
- Hardware-agnostic (keyboard, gamepad, etc.)
- Runtime remapping support
- Easier to maintain than legacy input
- Better debugging tools

### 3. BlueprintNativeEvent Pattern
Input handlers use `BlueprintNativeEvent`:
```cpp
UFUNCTION(BlueprintNativeEvent, Category="Controls")
void OnMoveInput(FVector2D MoveValue);
```

**Benefits**:
- Default C++ implementation for performance
- Blueprint override for custom behavior
- No recompilation needed for design iteration

### 4. Possession-Based Transitions
Player controller possesses different pawns:
- Spaceship (ASpaceship)
- Character (AAdastreaCharacter)

**Benefits**:
- Seamless transition between control modes
- Consistent input handling pattern
- Easy to save/restore possessed pawn

---

## Performance Considerations

### Spaceship System
- **Tick Optimization**: Only runs physics when possessed (checks `GetController()`)
- **Component Caching**: Weapon components cached on BeginPlay
- **Input Throttling**: Speed changes ignore near-zero scroll values
- **Flight Assist**: Conditional - only applies when enabled

### Character System (Planned)
- **Animation Optimization**: Use LOD for distant characters
- **Collision Optimization**: Simple capsule for character collision
- **Movement Prediction**: Built into UCharacterMovementComponent
- **Sprint State**: Cached boolean to avoid repeated calculations

---

## Debugging Tips

### Spaceship Controls
1. Enable verbose logging: `LogAdastreaInput` category
2. Check input mapping context is added: `showdebug enhancedinput`
3. Verify component is attached to spaceship actor
4. Check `bControlsEnabled` flag
5. Verify Enhanced Input subsystem is active

### Character Controls (When Implemented)
1. Check CharacterMovementComponent settings
2. Verify camera is attached to SpringArm
3. Check collision capsule size and placement
4. Verify animation blueprint is assigned
5. Test on flat ground first before complex terrain

### Common Issues
- **No input response**: Check mapping context priority
- **Wrong input direction**: Check axis swizzle modifiers
- **Jittery movement**: Adjust damping/interpolation values
- **Can't transition**: Verify SavedExternalPawn is valid

---

## Future Enhancements

### Planned Features
1. **Dual control mode**: Simultaneous ship + character (for multiplayer)
2. **AI characters**: NPCs walking on ships using same character system
3. **VR support**: Alternative input mapping contexts for VR
4. **Accessibility**: Customizable input remapping UI
5. **Gamepad support**: Complete gamepad bindings for both systems

---

## References

### Related Files
- `Source/Adastrea/Public/Player/AdastreaPlayerController.h`
- `Source/Adastrea/Player/AdastreaPlayerController.cpp`
- `Source/Adastrea/Ships/Spaceship.h`
- `Source/Adastrea/Ships/Spaceship.cpp`
- `Source/Adastrea/Public/Ships/SpaceshipControlsComponent.h`
- `Source/Adastrea/Ships/SpaceshipControlsComponent.cpp`
- `Source/Adastrea/AdastreaGameMode.h`
- `Source/Adastrea/AdastreaGameMode.cpp`

### Documentation
- `ENHANCED_INPUT_GUIDE.md` - Manual Enhanced Input setup
- `Assets/EnhancedInputImplementation.md` - C++ InputConfigDataAsset approach
- `ARCHITECTURE.md` - Overall system architecture
- `CODE_STYLE.md` - C++ coding standards

---

**Last Updated**: 2025-12-05  
**Version**: 1.0  
**Status**: Spaceship system implemented, Character system planned
