# Free Look Camera - Visual Guide

## Component Hierarchy

```
ASpaceship (Pawn)
    │
    ├── RootComponent (Scene Component)
    │
    ├── MovementComponent (FloatingPawnMovement)
    │
    ├── ParticleComponent (SpaceshipParticleComponent)
    │
    └── CameraSpringArm (SpringArmComponent) ◄── NEW
            │
            └── Camera (CameraComponent) ◄── NEW
```

## Input Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     Player Input                              │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │   Right Mouse Button State?             │
        └─────────────────────────────────────────┘
                  │                      │
         Pressed  │                      │  Released
                  ▼                      ▼
    ┌───────────────────────┐  ┌──────────────────────┐
    │  FreeLookStarted()    │  │  FreeLookCompleted() │
    │  - Set bFreeLook=true │  │  - Set bFreeLook=false│
    │  - Store base rotation│  │  - Reset camera       │
    └───────────────────────┘  └──────────────────────┘
                  │
         Holding  │  + Mouse Movement
                  ▼
    ┌───────────────────────────────┐
    │  FreeLookCamera()             │
    │  - Get mouse delta            │
    │  - Update camera rotation     │
    │  - Clamp pitch (±89°)         │
    │  - Apply to SpringArm         │
    └───────────────────────────────┘
```

## Rotation Independence

### Normal Flight Mode (RMB NOT held)
```
┌──────────────┐
│ Mouse Input  │
└──────────────┘
       │
       ▼
┌──────────────┐      ┌──────────────┐
│ Look Action  │ ───► │ Ship Rotates │
└──────────────┘      └──────────────┘
       │                      │
       ▼                      ▼
┌──────────────┐      ┌──────────────┐
│Turn()/LookUp()│     │Camera Follows│
└──────────────┘      └──────────────┘
```

### Free Look Mode (RMB HELD)
```
┌──────────────┐
│ Mouse Input  │
└──────────────┘
       │
       ├────────────────────┬──────────────────────┐
       │                    │                      │
       ▼                    ▼                      ▼
┌──────────────┐    ┌──────────────┐      ┌──────────────┐
│ Look Action  │    │ FreeLook     │      │ Ship Stays   │
│  (IGNORED)   │    │   Active     │      │   Stationary │
└──────────────┘    └──────────────┘      └──────────────┘
                            │
                            ▼
                    ┌──────────────┐
                    │Camera Rotates│
                    │Independently │
                    └──────────────┘
```

## State Machine

```
┌─────────────────────────────────────────────────────────┐
│                    INACTIVE STATE                        │
│  bFreeLookActive = false                                │
│  Camera follows ship rotation                           │
│  Look() input rotates ship                              │
└─────────────────────────────────────────────────────────┘
                            │
                    RMB Pressed (Started Event)
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│                     ACTIVE STATE                         │
│  bFreeLookActive = true                                 │
│  Camera rotation independent                            │
│  Look() input ignored                                   │
│  FreeLookCamera() updates camera                        │
└─────────────────────────────────────────────────────────┘
                            │
                    RMB Released (Completed Event)
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│                  RETURNING STATE                         │
│  bFreeLookActive = false                                │
│  Camera smoothly returns to ship forward               │
│  SpringArm rotation lerps to (0,0,0)                    │
└─────────────────────────────────────────────────────────┘
                            │
                            ▼
                    Back to INACTIVE STATE
```

## Camera Rotation Calculation

### Normal Mode
```
CameraRotation = ShipRotation + SpringArmRelativeRotation(0,0,0)
                └─────────────────────────────────────────┘
                          Camera follows ship exactly
```

### Free Look Mode
```
CameraRotation = ShipRotation + FreeLookRotation(Pitch, Yaw, 0)
                 └─────────┘    └─────────────────────────────┘
                 Ship stays     Camera rotates independently
                 constant
                 
FreeLookRotation.Pitch = Clamped to [-89°, +89°]
FreeLookRotation.Yaw = Unlimited rotation
FreeLookRotation.Roll = Always 0 (no roll in free look)
```

## Settings Hierarchy

```
┌──────────────────────────────────────────────────────┐
│             Spaceship Class Properties               │
├──────────────────────────────────────────────────────┤
│                                                      │
│  Camera | Free Look                                 │
│  ├── bFreeLookActive (Runtime state)                │
│  └── FreeLookSensitivity (1.5)                      │
│                                                      │
│  Camera | Settings                                  │
│  ├── CameraDistance (800.0)                         │
│  └── CameraLagSpeed (10.0)                          │
│                                                      │
│  Flight Control | Advanced                          │
│  ├── MouseFlightSensitivity (1.0)                   │
│  └── TurnRate (45.0)                                │
└──────────────────────────────────────────────────────┘
                           │
                           ▼
        ┌─────────────────────────────────┐
        │  CameraSpringArm Component      │
        ├─────────────────────────────────┤
        │  ├── TargetArmLength             │
        │  ├── bEnableCameraLag            │
        │  └── CameraLagSpeed              │
        └─────────────────────────────────┘
```

## Key Advantages of This Implementation

1. **Non-Intrusive**: 
   - Doesn't modify core ship movement code
   - Only affects camera behavior when active
   - Gracefully degrades if not configured

2. **Smooth Transitions**:
   - Camera lag provides natural feel
   - Smooth return to ship orientation
   - No jarring snaps or jumps

3. **Performance Efficient**:
   - Only processes input when RMB is held
   - Standard UE components (no custom rendering)
   - Minimal CPU overhead

4. **Designer Friendly**:
   - All settings exposed to Blueprint
   - Easy to tune in editor
   - No recompilation needed for adjustments

5. **Input System Integration**:
   - Uses Enhanced Input system properly
   - Respects trigger events (Started/Completed)
   - Compatible with future input remapping

## Testing Visualization

```
Test Scenario 1: Basic Free Look
═══════════════════════════════════════════════════════════
Time    Action                  Ship State    Camera State
────────────────────────────────────────────────────────────
T0      Ship facing forward     →             →
T1      Hold RMB                →             → (locked)
T2      Move mouse right        →             ↗ (looks right)
T3      Move mouse up           →             ↗↑ (looks up-right)
T4      Release RMB             →             ↗↑ (returning...)
T5      Wait...                 →             → (back to forward)


Test Scenario 2: Flight During Free Look
═══════════════════════════════════════════════════════════
Time    Action                  Ship State    Camera State
────────────────────────────────────────────────────────────
T0      Ship facing forward     →             →
T1      Hold RMB                →             → (locked)
T2      Look left with mouse    →             ← (camera left)
T3      Turn ship right (WASD)  ↗             ← (still left)
T4      Ship rotates more       →             ← (still looking left)
T5      Release RMB             →             ← (returning...)
T6      Wait...                 →             → (follows ship)
```

---

**Note**: This visual guide is for developer reference. See `FREE_LOOK_SETUP_GUIDE.md` for actual setup instructions.
