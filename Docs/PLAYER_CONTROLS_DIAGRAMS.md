# Player Controls Visual Diagrams

This document provides visual flowcharts using Mermaid syntax for the Adastrea player control systems.

## Table of Contents
1. [Spaceship System Diagrams](#spaceship-system-diagrams)
2. [Character System Diagrams](#character-system-diagrams)
3. [Transition Diagrams](#transition-diagrams)

---

## Spaceship System Diagrams

### Game Start to Spaceship Control - Complete Flow

```mermaid
sequenceDiagram
    participant Engine as UE5 Engine
    participant GameMode as AAdastreaGameMode
    participant World as UWorld
    participant PC as AAdastreaPlayerController
    participant Ship as ASpaceship
    participant Controls as USpaceshipControlsComponent
    participant Input as Enhanced Input Subsystem
    
    Engine->>Engine: UEngine::Init()
    Engine->>GameMode: Constructor Called
    GameMode->>GameMode: Set defaults (bAutoSpawnPlayerShip=true)
    
    Engine->>World: SpawnPlayActor()
    World->>PC: Create AAdastreaPlayerController
    PC->>PC: Constructor - Initialize
    
    Engine->>GameMode: BeginPlay()
    GameMode->>GameMode: Check bShowTestSettingsOnStartup
    alt Test Settings Enabled
        GameMode->>GameMode: ShowTestSettingsWidget()
        Note over GameMode: Wait for user input
        GameMode->>GameMode: OnTestSettingsContinue()
    else Test Settings Disabled
        GameMode->>GameMode: OnTestSettingsContinue()
    end
    
    GameMode->>GameMode: SpawnPlayerSpaceship()
    GameMode->>World: Find SpaceSectorMap
    World-->>GameMode: Sector map or null
    GameMode->>GameMode: Calculate spawn location
    GameMode->>World: SpawnActor<ASpaceship>()
    
    World->>Ship: Constructor
    Ship->>Ship: Create MovementComponent
    Ship->>Ship: Create ParticleComponent
    Ship->>Ship: Initialize flight parameters
    
    World->>Ship: BeginPlay()
    Ship->>Ship: Initialize hull from DataAsset
    Ship->>Ship: Spawn InteriorInstance
    
    Ship->>Controls: BeginPlay() (if component attached)
    Controls->>Controls: CreateInputActions()
    Controls->>Controls: CreateInputMappingContext()
    Controls->>Controls: SetupInputBindings()
    Controls->>Input: AddMappingContext()
    Input-->>Controls: Context active
    Controls->>Controls: EnableControls()
    
    GameMode->>PC: Possess(Ship)
    PC->>Ship: PossessedBy(PC)
    Ship->>Ship: SetupPlayerInputComponent()
    
    PC->>PC: BeginPlay()
    PC->>PC: SetupInputComponent()
    
    Note over Ship,Input: ✓ System Ready - Player can control spaceship
```

### Spaceship Runtime Input Processing

```mermaid
flowchart TD
    Start([Player Presses W Key]) --> Input[Enhanced Input System<br/>Captures Input]
    Input --> Match[Match to SpaceshipMappingContext]
    Match --> Action[Find MoveAction<br/>W → Y:1.0]
    Action --> Modifier[Apply Modifiers<br/>SwizzleAxis, Negate, etc.]
    Modifier --> Trigger[Trigger ETriggerEvent::Triggered]
    
    Trigger --> Handler[USpaceshipControlsComponent<br/>::HandleMove]
    Handler --> Extract[Extract FVector2D value]
    Extract --> Multiply[Multiply by MovementSpeed<br/>and CurrentSpeed]
    Multiply --> Event[Call OnMoveInput BlueprintNativeEvent]
    
    Event --> Impl[OnMoveInput_Implementation]
    Impl --> GetVectors[Get RightVector and UpVector<br/>from Pawn]
    GetVectors --> CalcDir[Calculate Movement Direction<br/>MoveDir = Right*X + Up*Y]
    CalcDir --> AddInput[APawn::AddMovementInput]
    
    AddInput --> Accumulate[Accumulate in ControlInputVector]
    
    Accumulate --> Tick[ASpaceship::Tick]
    Tick --> CheckPossessed{Possessed?}
    CheckPossessed -->|No| EndTick[Skip Physics]
    CheckPossessed -->|Yes| FlightAssist{Flight Assist<br/>Enabled?}
    
    FlightAssist -->|Yes| ApplyAssist[ApplyFlightAssist]
    ApplyAssist --> UpdateThrottle[UpdateThrottleVelocity]
    FlightAssist -->|No| UpdateThrottle
    
    UpdateThrottle --> AutoLevel{Auto-Level<br/>Enabled?}
    AutoLevel -->|Yes| ApplyLevel[ApplyAutoLeveling]
    AutoLevel -->|No| MovementComp[UFloatingPawnMovement<br/>::TickComponent]
    ApplyLevel --> MovementComp
    
    MovementComp --> ApplyVel[Apply Velocity to Position]
    ApplyVel --> Collision[Collision Detection]
    Collision --> UpdatePos[Update Actor Location]
    UpdatePos --> Render[Renderer Updates Visual]
    
    Render --> End([Frame Complete])
    EndTick --> End
```

### Spaceship Input Binding Architecture

```mermaid
classDiagram
    class EnhancedInputSubsystem {
        +AddMappingContext()
        +RemoveMappingContext()
        -ProcessInput()
    }
    
    class InputMappingContext {
        +Name: IMC_SpaceshipControls
        +Priority: 0
        +MapKey(Action, Key, Modifiers)
    }
    
    class InputAction {
        +ValueType
        +Triggers
        +Modifiers
    }
    
    class SpaceshipControlsComponent {
        +MoveAction: IA_SpaceshipMove
        +LookAction: IA_SpaceshipLook
        +FireAction: IA_SpaceshipFire
        +SpeedAction: IA_SpaceshipSpeed
        +EnableControls()
        +DisableControls()
        +HandleMove()
        +HandleLook()
        +HandleFire()
        +HandleSpeed()
    }
    
    class ASpaceship {
        +MovementComponent
        +ParticleComponent
        +Tick()
        +SetupPlayerInputComponent()
    }
    
    EnhancedInputSubsystem --> InputMappingContext : Uses
    InputMappingContext --> InputAction : Contains multiple
    SpaceshipControlsComponent --> InputAction : Creates & Binds
    SpaceshipControlsComponent --> InputMappingContext : Creates
    SpaceshipControlsComponent --> EnhancedInputSubsystem : Registers with
    SpaceshipControlsComponent --|> ASpaceship : Component of
```

### Spaceship Component Hierarchy

```mermaid
graph TD
    PC[AAdastreaPlayerController] -->|Possesses| Ship[ASpaceship<br/>APawn]
    
    Ship --> Movement[UFloatingPawnMovement<br/>Physics-based movement]
    Ship --> Particles[USpaceshipParticleComponent<br/>Engine effects]
    Ship --> Controls[USpaceshipControlsComponent<br/>Enhanced Input handler]
    Ship --> Weapon[UWeaponComponent<br/>Weapon firing]
    Ship --> Combat[UCombatComponent<br/>Combat logic]
    
    Ship --> DataAsset[USpaceshipDataAsset<br/>Ship configuration]
    Ship --> Interior[ASpaceshipInterior<br/>Walkable interior]
    
    Controls --> InputActions[Input Actions<br/>Move, Look, Fire, Speed]
    Controls --> MappingContext[Input Mapping Context<br/>Key bindings]
    
    style PC fill:#e1f5ff
    style Ship fill:#ffe1e1
    style Controls fill:#ffe1ff
    style Movement fill:#e1ffe1
```

---

## Character System Diagrams

### Character Spawn and Initialization (Planned)

```mermaid
sequenceDiagram
    participant Ship as ASpaceship
    participant World as UWorld
    participant Char as AAdastreaCharacter
    participant Movement as UCharacterMovementComponent
    participant Controls as UCharacterControlsComponent
    participant Input as Enhanced Input Subsystem
    participant PC as AAdastreaPlayerController
    
    Note over Ship: Player triggers exit (E key)
    Ship->>Ship: Check if docked/safe
    Ship->>World: SpawnActor<AAdastreaCharacter>()
    
    World->>Char: Constructor
    Char->>Char: Create CapsuleComponent
    Char->>Char: Create SkeletalMeshComponent
    Char->>Movement: Create CharacterMovementComponent
    Movement->>Movement: Set walk/run speeds
    Movement->>Movement: Set jump height
    Char->>Char: Create SpringArmComponent
    Char->>Char: Create CameraComponent
    Char->>Char: Configure rotation settings
    
    World->>Char: BeginPlay()
    Char->>Char: Initialize health/stamina
    Char->>Char: Setup animation blueprint
    
    Char->>Controls: BeginPlay()
    Controls->>Controls: CreateInputActions()
    Note over Controls: Move, Look, Jump, Sprint,<br/>Interact, Crouch
    Controls->>Controls: CreateInputMappingContext()
    Note over Controls: Map WASD, Mouse, Space,<br/>Shift, E, C
    Controls->>Controls: SetupInputBindings()
    Controls->>Input: AddMappingContext()
    Controls->>Controls: EnableControls()
    
    Ship->>PC: Possess(Character)
    PC->>Ship: Save reference in SavedExternalPawn
    PC->>Char: PossessedBy(PC)
    Char->>Char: SetupPlayerInputComponent()
    
    Note over Char,Input: ✓ Character Ready - Player can walk
```

### Character Movement Input Flow (Planned)

```mermaid
flowchart TD
    Start([Player Presses W Key]) --> Input[Enhanced Input System]
    Input --> Context[CharacterMappingContext]
    Context --> Action[MoveAction triggered]
    Action --> Handler[UCharacterControlsComponent<br/>::HandleMove]
    
    Handler --> Extract[Extract FVector2D]
    Extract --> Sprint{Is Sprinting?}
    Sprint -->|Yes| ApplySprint[Apply Sprint Multiplier]
    Sprint -->|No| CallEvent[Call OnMoveInput]
    ApplySprint --> CallEvent
    
    CallEvent --> GetCamera[Get Camera/Controller Rotation]
    GetCamera --> CalcForward[Calculate Forward Direction<br/>from YawRotation]
    CalcForward --> CalcRight[Calculate Right Direction<br/>from YawRotation]
    CalcRight --> AddForward[AddMovementInput Forward]
    AddForward --> AddRight[AddMovementInput Right]
    
    AddRight --> CharMovement[UCharacterMovementComponent<br/>::TickComponent]
    CharMovement --> CalcVel[Calculate Desired Velocity]
    CalcVel --> Accel[Apply Acceleration/Friction]
    Accel --> Mode{Movement Mode?}
    
    Mode -->|Walking| Ground[Process Ground Movement]
    Mode -->|Falling| Air[Process Air Movement<br/>Apply Gravity]
    Mode -->|Swimming| Water[Process Swimming]
    
    Ground --> Collision[Collision Detection]
    Air --> Collision
    Water --> Collision
    
    Collision --> UpdatePos[Update Capsule Position]
    UpdatePos --> Rotate[Rotate to Movement Direction]
    Rotate --> Animation[Update Animation Blueprint]
    
    Animation --> Speed[Calculate Speed]
    Speed --> State[Update State Machine<br/>Idle/Walk/Run/Jump]
    State --> Blend[Update Blend Spaces]
    Blend --> Render[Render Character]
    
    Render --> End([Frame Complete])
```

### Character Look and Camera Control (Planned)

```mermaid
flowchart TD
    Start([Mouse Movement]) --> Input[Enhanced Input System]
    Input --> Context[CharacterMappingContext]
    Context --> Action[LookAction triggered]
    Action --> Handler[UCharacterControlsComponent<br/>::HandleLook]
    
    Handler --> Extract[Extract FVector2D<br/>X=Yaw, Y=Pitch]
    Extract --> Sensitivity[Apply LookSensitivity]
    Sensitivity --> Event[Call OnLookInput]
    
    Event --> Yaw[AddControllerYawInput<br/>Horizontal rotation]
    Event --> Pitch[AddControllerPitchInput<br/>Vertical rotation]
    
    Yaw --> Controller[Controller Rotation Updated]
    Pitch --> Controller
    
    Controller --> Spring[SpringArmComponent<br/>Follows Controller Rotation]
    Spring --> Camera[CameraComponent<br/>Attached to SpringArm]
    Camera --> Smooth[Camera Lag Smoothing]
    Smooth --> Orient{bOrientRotationToMovement?}
    
    Orient -->|Yes| RotateMesh[Rotate Character Mesh<br/>to Movement Direction]
    Orient -->|No| NoRotate[Mesh uses Controller Rotation]
    
    RotateMesh --> Render[Render Updated View]
    NoRotate --> Render
    Render --> End([Frame Complete])
```

### Character Jump Mechanics (Planned)

```mermaid
stateDiagram-v2
    [*] --> Grounded: Character spawned
    
    Grounded --> Jumping: Press Space<br/>(HandleJumpPressed)
    Jumping --> Falling: Jump velocity applied<br/>DoJump()
    Falling --> Grounded: Land on ground<br/>ProcessLanded()
    
    Jumping --> Falling: Release Space early<br/>(StopJumping)
    
    state Grounded {
        [*] --> Idle
        Idle --> Walking: WASD input
        Walking --> Running: Speed threshold
        Running --> Sprinting: Hold Shift
        Sprinting --> Running: Release Shift
        Running --> Walking: Below threshold
        Walking --> Idle: No input
    }
    
    state Falling {
        [*] --> InAir
        InAir --> InAir: Apply gravity<br/>each frame
        InAir --> [*]: Touch ground
    }
    
    note right of Jumping
        Jump Input Flow:
        1. Space pressed
        2. Check CanJump()
        3. Set bPressedJump=true
        4. DoJump() applies velocity
        5. Change mode to Falling
        6. Trigger jump animation
    end note
    
    note right of Falling
        Variable Jump Height:
        - Hold Space: Full jump
        - Tap Space: Short jump
        - StopJumping() reduces
          upward velocity
    end note
```

### Character Component Architecture (Planned)

```mermaid
graph TD
    PC[AAdastreaPlayerController] -->|Possesses| Char[AAdastreaCharacter<br/>ACharacter]
    
    Char --> Capsule[UCapsuleComponent<br/>Collision]
    Char --> Mesh[USkeletalMeshComponent<br/>Character mesh]
    Char --> Movement[UCharacterMovementComponent<br/>Built-in UE5 movement]
    Char --> Spring[USpringArmComponent<br/>Camera boom]
    Char --> Camera[UCameraComponent<br/>Third-person view]
    Char --> Controls[UCharacterControlsComponent<br/>Enhanced Input]
    Char --> Inventory[UInventoryComponent<br/>Item management]
    
    Char --> DataAsset[UCharacterDataAsset<br/>Character stats]
    
    Controls --> CharInputActions[Input Actions<br/>Move, Look, Jump,<br/>Sprint, Interact, Crouch]
    Controls --> CharMappingContext[Input Mapping Context<br/>Character key bindings]
    
    Mesh --> AnimBP[Animation Blueprint<br/>State machine & blends]
    
    style PC fill:#e1f5ff
    style Char fill:#ffe1e1
    style Controls fill:#ffe1ff
    style Movement fill:#e1ffe1
    style AnimBP fill:#fff4e1
```

---

## Transition Diagrams

### Spaceship to Character Transition

```mermaid
flowchart TD
    Start([Player in Spaceship]) --> Near{Near Exit/Door?}
    Near -->|No| Wait[Continue flying]
    Near -->|Yes| ShowPrompt[Show Interact Prompt<br/>Press E to Exit]
    
    Wait --> Near
    ShowPrompt --> PressE{Press E?}
    PressE -->|No| ShowPrompt
    PressE -->|Yes| CheckSafe{Docked or<br/>Safe Location?}
    
    CheckSafe -->|No| Warning[Show Warning<br/>Cannot exit in space]
    Warning --> ShowPrompt
    
    CheckSafe -->|Yes| SaveShip[Save Spaceship in<br/>SavedExternalPawn]
    SaveShip --> SpawnChar[Spawn AAdastreaCharacter<br/>at exit location]
    SpawnChar --> DisableShip[Disable Spaceship Controls<br/>DisableControls]
    DisableShip --> Possess[PlayerController->Possess<br/>Character]
    Possess --> EnableChar[Enable Character Controls<br/>EnableControls]
    EnableChar --> End([Now controlling Character])
    
    style Start fill:#e1f5ff
    style End fill:#ffe1e1
```

### Character to Spaceship Transition

```mermaid
flowchart TD
    Start([Player as Character]) --> Approach[Approach Spaceship]
    Approach --> Range{Within Interact<br/>Range?}
    Range -->|No| Approach
    Range -->|Yes| Trace[Line Trace Forward]
    Trace --> Check{Hit Interactable?}
    
    Check -->|No| Range
    Check -->|Yes| CheckType{Is Spaceship or<br/>Console?}
    
    CheckType -->|No| OtherInteract[Handle Other Interaction<br/>Door, Item, NPC, etc.]
    CheckType -->|Yes| ShowPrompt[Show Interact Prompt<br/>Press E to Board]
    
    OtherInteract --> Range
    ShowPrompt --> PressE{Press E?}
    PressE -->|No| ShowPrompt
    PressE -->|Yes| CheckSaved{SavedExternalPawn<br/>Valid?}
    
    CheckSaved -->|No| Error[Error: No saved ship<br/>Stay as character]
    CheckSaved -->|Yes| DisableChar[Disable Character Controls<br/>DisableControls]
    
    Error --> Range
    
    DisableChar --> Possess[PlayerController->Possess<br/>SavedExternalPawn]
    Possess --> EnableShip[Enable Spaceship Controls<br/>EnableControls]
    EnableShip --> HideChar[Hide or Destroy<br/>Character Actor]
    HideChar --> End([Now controlling Spaceship])
    
    style Start fill:#ffe1e1
    style End fill:#e1f5ff
```

### Interior Console Control Transition

```mermaid
flowchart TD
    Start([Character Inside Ship Interior]) --> Walk[Walk to Control Console]
    Walk --> Console[Approach AShipControlConsole]
    Console --> Range{Within Range?}
    Range -->|No| Walk
    Range -->|Yes| ShowPrompt[Show Prompt<br/>Press E to Pilot]
    
    ShowPrompt --> PressE{Press E?}
    PressE -->|No| ShowPrompt
    PressE -->|Yes| BeginControl[ASpaceship::BeginControl<br/>PlayerController, Character]
    
    BeginControl --> SaveChar[Save Character in<br/>SavedExternalPawn]
    SaveChar --> HideChar[Hide Character<br/>Don't Destroy]
    HideChar --> Possess[PlayerController->Possess<br/>Spaceship]
    Possess --> EnableShip[Enable Spaceship Controls]
    EnableShip --> SwitchView[Switch to Exterior View<br/>Show spaceship]
    SwitchView --> Flying([Flying from Interior])
    
    Flying --> ExitKey{Press Exit Key?}
    ExitKey -->|No| Flying
    ExitKey -->|Yes| EndControl[ASpaceship::EndControl<br/>PlayerController]
    
    EndControl --> RestoreChar[Restore SavedExternalPawn<br/>Show Character]
    RestoreChar --> PossessChar[PlayerController->Possess<br/>Character]
    PossessChar --> DisableShip[Disable Spaceship Controls]
    DisableShip --> EnableCharCtrl[Enable Character Controls]
    EnableCharCtrl --> Interior([Back to Walking in Interior])
    
    Interior --> Walk
    
    style Start fill:#ffe1e1
    style Flying fill:#e1f5ff
    style Interior fill:#ffe1e1
```

### Enhanced Input Context Priority Management

```mermaid
flowchart TD
    Start([Input Received]) --> Subsystem[Enhanced Input Subsystem]
    Subsystem --> GetContexts[Get All Active<br/>Mapping Contexts]
    GetContexts --> Sort[Sort by Priority<br/>Higher First]
    
    Sort --> Check1{CharacterMappingContext<br/>Priority: 1<br/>Active?}
    Check1 -->|Yes| Process1[Process Character Inputs]
    Check1 -->|No| Check2
    
    Process1 --> Consume{Input Consumed?}
    Consume -->|Yes| End([Input Handled])
    Consume -->|No| Check2
    
    Check2{SpaceshipMappingContext<br/>Priority: 0<br/>Active?}
    Check2 -->|Yes| Process2[Process Spaceship Inputs]
    Check2 -->|No| NoContext[No Context Matched]
    
    Process2 --> Consume2{Input Consumed?}
    Consume2 -->|Yes| End
    Consume2 -->|No| NoContext
    
    NoContext --> End
    
    Note1[Note: Only one context<br/>should be active at a time<br/>to avoid conflicts]
    Note1 -.-> Check1
    Note1 -.-> Check2
    
    style Check1 fill:#ffe1e1
    style Check2 fill:#e1f5ff
```

### State Machine: Player Control Modes

```mermaid
stateDiagram-v2
    [*] --> GameStart
    
    GameStart --> SpaceshipControl: GameMode spawns<br/>spaceship & possesses
    
    SpaceshipControl --> CharacterControl: Exit spaceship<br/>(E at door/dock)
    CharacterControl --> SpaceshipControl: Board spaceship<br/>(E at ship/console)
    
    SpaceshipControl --> InteriorWalking: Enter interior<br/>(transition to interior)
    InteriorWalking --> ConsolePiloting: Use control console<br/>(E at console)
    ConsolePiloting --> InteriorWalking: Exit console<br/>(Exit key)
    InteriorWalking --> SpaceshipControl: Exit interior<br/>(E at door)
    
    state SpaceshipControl {
        [*] --> Flying
        Flying --> Combat: Enemy detected
        Combat --> Flying: Combat ends
        Flying --> Docking: Approach station
        Docking --> Flying: Undock
    }
    
    state CharacterControl {
        [*] --> Walking
        Walking --> Running: Speed up
        Running --> Sprinting: Hold Shift
        Sprinting --> Running: Release Shift
        Running --> Walking: Slow down
        Walking --> Jumping: Press Space
        Jumping --> Walking: Land
        Walking --> Interacting: Press E
        Interacting --> Walking: Interaction complete
    }
    
    state InteriorWalking {
        [*] --> ExploreInterior
        ExploreInterior --> AtConsole: Walk to console
        AtConsole --> ExploreInterior: Walk away
    }
    
    state ConsolePiloting {
        [*] --> PilotingFromInterior
        note right of PilotingFromInterior
            Same as SpaceshipControl
            but character is hidden,
            saved in SavedExternalPawn
        end note
    }
    
    note left of SpaceshipControl
        Enhanced Input Context:
        SpaceshipMappingContext (Priority 0)
        Controls: WASD strafe, Mouse look,
        LMB fire, MouseWheel speed
    end note
    
    note right of CharacterControl
        Enhanced Input Context:
        CharacterMappingContext (Priority 1)
        Controls: WASD move, Mouse look,
        Space jump, Shift sprint, E interact
    end note
```

---

## Implementation Checklist

### Spaceship System (✓ Implemented)
- [x] AAdastreaPlayerController
- [x] ASpaceship with FloatingPawnMovement
- [x] USpaceshipControlsComponent
- [x] Enhanced Input Actions (Move, Look, Fire, Speed)
- [x] Input Mapping Context
- [x] Flight physics (flight assist, throttle, auto-level)
- [x] Weapon integration

### Character System (⚠️ Planned)
- [ ] AAdastreaCharacter (derived from ACharacter)
- [ ] UCharacterControlsComponent
- [ ] Enhanced Input Actions (Move, Look, Jump, Sprint, Interact, Crouch)
- [ ] Character Input Mapping Context
- [ ] Animation Blueprint setup
- [ ] Inventory system integration
- [ ] Interaction system (IInteractable interface)

### Transition System (⚠️ Partially Implemented)
- [x] SavedExternalPawn tracking in controller
- [x] ASpaceship::BeginControl / EndControl for interior console
- [ ] Character spawn on ship exit
- [ ] Character-to-ship boarding interaction
- [ ] Interior-to-exterior transition
- [ ] Seamless possession switching

---

## Testing Procedures

### Spaceship Controls Testing
1. Start game → Verify spaceship spawns
2. Press WASD → Verify strafe movement
3. Move mouse → Verify rotation
4. Press LMB → Verify weapon fires (if weapon component exists)
5. Scroll mouse wheel → Verify speed changes
6. Check flight assist → Enable/disable and test feel
7. Throttle controls → Test speed multipliers

### Character Controls Testing (When Implemented)
1. Exit spaceship → Verify character spawns at correct location
2. Press WASD → Verify forward/back/strafe movement
3. Move mouse → Verify camera rotation
4. Press Space → Verify jump
5. Hold Shift while moving → Verify sprint
6. Press E near object → Verify interaction
7. Press C → Verify crouch
8. Board spaceship → Verify return to spaceship control

### Transition Testing
1. Spaceship → Character → Verify smooth possession switch
2. Character → Spaceship → Verify saved ship restored
3. Interior console → Verify character hidden, ship control active
4. Exit console → Verify character restored, walking active
5. Multiple transitions → Verify no memory leaks or lost references

---

## Performance Profiling

### Key Metrics to Monitor

```mermaid
graph LR
    A[Frame Rate] --> B[Target: 60 FPS minimum]
    C[Input Latency] --> D[Target: <16ms]
    E[Memory Usage] --> F[Monitor SavedExternalPawn refs]
    G[Tick Overhead] --> H[Only when possessed]
    
    style B fill:#90EE90
    style D fill:#90EE90
    style F fill:#FFD700
    style H fill:#90EE90
```

**Optimization Tips**:
- Spaceship Tick only runs when possessed (check GetController())
- Component caching reduces repeated FindComponent calls
- Input mapping contexts cleanly enable/disable
- Character movement uses built-in UE5 optimizations

---

**Document Version**: 1.0  
**Last Updated**: 2025-12-05  
**Status**: Spaceship diagrams complete, Character diagrams planned
