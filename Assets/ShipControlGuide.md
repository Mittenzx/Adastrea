# Ship Interior Control System Guide

## Overview

The Ship Interior Control System allows players to control their spaceship from inside the interior by interacting with a control console. The system handles possession swapping between the player's walking pawn and the ship pawn seamlessly.

## Components

### 1. AShipControlConsole

An interactable actor placed inside the spaceship interior that serves as the pilot's control interface.

**Key Properties:**
- `OwningShip` - Reference to the spaceship being controlled
- `InteractionRange` - Distance from console where player can interact (default: 150 units)
- `bIsControlActive` - Whether the console is currently in use

**Key Methods:**
- `Interact(PlayerController)` - Toggles ship control on/off
- `CanInteract(PlayerController)` - Checks if interaction is possible

### 2. ASpaceship Control Methods

The spaceship class now supports direct player control with possession swapping.

**New Methods:**
- `BeginControl(PC, ExternalPawn)` - Start controlling the ship
  - Saves the walking pawn reference
  - Disables and hides the walking pawn
  - Possesses the ship
  - Activates ship movement
  
- `EndControl(PC)` - Stop controlling the ship
  - Returns possession to walking pawn
  - Restores walking pawn visibility and collision
  - Teleports player to exit location
  - Clears saved pawn reference

- `SetupPlayerInputComponent(InputComponent)` - Binds ship controls
  - MoveForward/Back (W/S)
  - MoveRight/Left (A/D)
  - MoveUp/Down (Space/Ctrl)
  - Turn (Mouse X)
  - LookUp (Mouse Y)

**New Properties:**
- `TurnRate` - Rotation speed in degrees per second (default: 45.0)
- `SavedExternalPawn` - Stored reference to walking pawn

### 3. ASpaceshipInterior Exit Location

The interior now tracks where the player should be placed when exiting ship control.

**New Methods:**
- `GetExitLocation()` - Returns the exit spawn point

**New Properties:**
- `ExitLocation` - World location for player exit

## Setup Instructions

### Step 1: Configure Input (Already Done)

Input mappings are configured in `Config/DefaultInput.ini`:
- Axis: MoveForward, MoveRight, MoveUp, Turn, LookUp
- Action: Interact

### Step 2: Place Control Console in Interior Blueprint

1. Open your spaceship's interior Blueprint
2. Add a `ShipControlConsole` actor to the scene
3. Position it where the pilot seat/console should be
4. In the Details panel, set:
   - `OwningShip`: Reference to the parent spaceship
   - `InteractionRange`: Adjust as needed (e.g., 150-200 units)

### Step 3: Configure Exit Location

1. In the interior Blueprint or actor instance:
2. Set the `ExitLocation` property
3. This is where the player will spawn when leaving the pilot seat
4. Typically near the console or at a seat location

### Step 4: Implement Player Interact Logic

In your player character/pawn Blueprint or C++ class:

1. Bind the "Interact" action input
2. On interact press, perform a sphere/line trace
3. Check if hit actor is a `ShipControlConsole`
4. Call `Console->Interact(GetController())`

**Example Blueprint Logic:**
```
On "Interact" Pressed:
  └─> Line Trace (or Sphere Overlap)
      └─> Hit Actor is ShipControlConsole?
          └─> Cast to ShipControlConsole
              └─> Call Interact (PlayerController)
```

**Example C++ Code:**
```cpp
void APlayerCharacter::OnInteract()
{
    // Trace for interactable objects
    FHitResult HitResult;
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 200.0f;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
    {
        AShipControlConsole* Console = Cast<AShipControlConsole>(HitResult.GetActor());
        if (Console && Console->CanInteract(GetController<APlayerController>()))
        {
            Console->Interact(GetController<APlayerController>());
        }
    }
}
```

## Usage Flow

### Entering Ship Control

1. Player walks to the control console inside the interior
2. Player presses Interact (E key by default)
3. System checks if console can be interacted with
4. Walking pawn is saved, hidden, and disabled
5. Player controller possesses the spaceship
6. Player can now control the ship with WASD/Mouse

### Exiting Ship Control

1. Player presses Interact (E key) again while controlling ship
2. Player controller re-possesses the walking pawn
3. Walking pawn is restored (visible, collision enabled)
4. Walking pawn is teleported to the exit location
5. Player can now walk around the interior again

## Customization

### Adjusting Turn Rate

Change the `TurnRate` property on the spaceship Blueprint to adjust rotation sensitivity:
- Lower values = slower turning (e.g., 30.0 for heavy ships)
- Higher values = faster turning (e.g., 60.0 for fighters)

### Custom Exit Behavior

Override or extend `ASpaceship::EndControl()` to:
- Add camera transitions
- Play sound effects
- Show/hide UI elements
- Trigger animations

### Multiple Control Stations

You can place multiple `ShipControlConsole` actors in the interior:
- All reference the same `OwningShip`
- Each can have different interaction ranges
- Only one can be active at a time (handled automatically)

## Blueprint Integration

All functionality is exposed to Blueprints:

- `ShipControlConsole->Interact()` - BlueprintCallable
- `Spaceship->BeginControl()` - BlueprintCallable
- `Spaceship->EndControl()` - BlueprintCallable
- All properties are BlueprintReadWrite or BlueprintReadOnly

## Troubleshooting

### Console doesn't respond to interact
- Check that `OwningShip` reference is set
- Verify player is within `InteractionRange`
- Ensure player's interact logic calls the console's `Interact()` method

### Player stuck controlling ship
- Call `Console->Interact()` again to toggle off
- Or manually call `Ship->EndControl(PlayerController)`

### Walking pawn disappears after exit
- Check that `SavedExternalPawn` is not null
- Verify `EndControl()` is being called properly
- Check that the walking pawn still exists in the world

### Ship doesn't move when controlled
- Verify input bindings in DefaultInput.ini
- Check that `MovementComponent` is active
- Ensure `SetupPlayerInputComponent()` is being called

## Example: Complete Setup

1. Create a Blueprint based on `ASpaceship` (e.g., `BP_PlayerShip`)
2. Add interior mesh and collision
3. Place a `ShipControlConsole` actor at the pilot station
4. Set console's `OwningShip` to self (the ship Blueprint)
5. Configure `ExitLocation` in the interior instance
6. Implement player interact to detect and call console interaction
7. Test in-game: walk to console, press E to control, press E again to exit

## Notes

- The system preserves the walking pawn's state during control
- Input is automatically routed to the possessed pawn (ship or walker)
- Movement component remains active throughout possession swaps
- Console interaction is proximity-based for realistic gameplay
- Exit location can be set per-interior or per-console for flexibility

## Future Enhancements

Possible additions to this system:
- Smooth camera transitions
- UI prompts for interact availability
- Sound effects for entering/exiting control
- Animation support for sitting/standing
- Co-pilot seats with limited controls
- Damage effects on console disabling control
