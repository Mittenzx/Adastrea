# Spaceship Interior Implementation Guide

## Overview

This comprehensive guide explains how to implement walkable spaceship interiors in Adastrea. You'll learn about the two main approaches to creating interiors, how to set them up, and how to integrate them with the spaceship control system.

**What You'll Learn:**
- Two approaches to interior implementation: Separate Level vs. Embedded Interior
- Pros and cons of each approach
- Step-by-step setup instructions
- Integration with ship control systems
- Best practices for design and optimization
- Common issues and solutions

**Prerequisites:**
- Basic understanding of Unreal Engine
- Familiarity with Blueprints
- Understanding of the Spaceship Data Asset system (see `SpaceshipDataAssetGuide.md`)

---

## Table of Contents

1. [Understanding Interior Approaches](#understanding-interior-approaches)
2. [Approach 1: Separate Level Interior](#approach-1-separate-level-interior)
3. [Approach 2: Embedded Interior](#approach-2-embedded-interior)
4. [Control Console Setup](#control-console-setup)
5. [Player Interaction System](#player-interaction-system)
6. [Best Practices](#best-practices)
7. [Optimization Guidelines](#optimization-guidelines)
8. [Common Issues and Solutions](#common-issues-and-solutions)
9. [Advanced Topics](#advanced-topics)
10. [Complete Example Workflow](#complete-example-workflow)

---

## Understanding Interior Approaches

There are two main ways to implement spaceship interiors in Adastrea, each with its own advantages and use cases.

### Approach Comparison

| Feature | Separate Level | Embedded Interior |
|---------|---------------|------------------|
| **Best For** | Large ships, capital ships, stations | Small ships, fighters, corvettes |
| **Memory Efficiency** | High (loaded on demand) | Low (always in memory) |
| **Transition Time** | Loading screen required | Instant transition |
| **Complexity** | Higher (level streaming) | Lower (single actor) |
| **Exterior Visibility** | None (in separate level) | Can see exterior through windows |
| **Networking** | Simpler (separate zones) | More complex (single actor) |
| **Use Cases** | Player ship, motherships, space stations | NPC ships, boarding targets, escape pods |

### Key Considerations

**Choose Separate Level if:**
- Ship interior is large (cruiser, battleship, capital ship)
- Multiple rooms with complex layouts
- You need loading screens for immersion
- Interior will have many interactive elements
- Performance is critical (can unload when not in use)

**Choose Embedded Interior if:**
- Ship interior is small (fighter, corvette, small freighter)
- Simple single-room layout
- You want instant boarding/transitions
- You need to see exterior from inside
- Ship will be frequently boarded

---

## Approach 1: Separate Level Interior

Separate level interiors are best for large ships where the interior is loaded as its own persistent level that can be streamed in and out as needed.

### Architecture

```
PlayerShip (Main Level Actor)
    ↓
Transition Trigger (Airlock, Boarding Ramp)
    ↓
Level Streaming (Load Interior Level)
    ↓
Interior Level (Complete walkable space)
    ↓
Exit Points (Return to exterior)
```

### Step 1: Create the Interior Level

1. **Create a New Level**
   - Content Browser → Right-click → Level → Empty Level
   - Name: `MAP_Interior_[ShipName]`
   - Example: `MAP_Interior_Pathfinder`, `MAP_Interior_Battlecruiser`

2. **Set Up Basic Lighting**
   - Add a **Sky Light** (Intensity: 0.5, Mobility: Stationary)
   - Add **Point Lights** for interior lighting
   - Add **Spot Lights** for accent lighting
   - Keep lighting atmospheric (ship interior ambience)

3. **Configure World Settings**
   - Open World Settings (Window → World Settings)
   - Set **Default Gravity Z**: -980.0 (normal gravity inside ship)
   - Set **Kill Z**: -10000.0 (prevent falling through floor)
   - **Game Mode Override**: Your game mode or BP_GameMode_Interior

4. **Build the Interior Geometry**

   **Option A: Using BSP (Quick Prototyping)**
   ```
   - Create walls with BSP Box brushes
   - Create floor with BSP Box brush
   - Create ceiling with BSP Box brush
   - Add doorways by subtractive brushes
   - Convert to Static Mesh when finalized
   ```

   **Option B: Using Static Meshes (Production)**
   ```
   - Import or create modular interior pieces
   - Snap together corridor sections
   - Add room modules (bridge, engineering, crew quarters)
   - Use modular approach for flexibility
   ```

5. **Add Collision**
   - Ensure all meshes have collision enabled
   - Use simple collision for walls/floors
   - Use complex collision only where necessary
   - Test by walking around with a character

### Step 2: Create Entry/Exit Points

1. **Add Player Start**
   - Place a **Player Start** actor at the main entry point
   - Set rotation to face into the ship
   - This is where players spawn when entering

2. **Add Exit Triggers**
   - Create a **Box Trigger** (TriggerBox or BoxCollision)
   - Position at airlocks, boarding ramps, or exits
   - Name: `ExitTrigger_Airlock`, `ExitTrigger_Rear`, etc.

3. **Create Exit Blueprint Logic**

   **In Level Blueprint or Custom Blueprint:**
   ```
   Event BeginOverlap (ExitTrigger)
       ↓
   Cast to Player Character
       ↓
   [Success]
       ↓
   Show "Press E to Exit Ship" UI prompt
       ↓
   On Player Press E
       ↓
   Unload Interior Level
       ↓
   Load/Return to Exterior Level
       ↓
   Spawn Player at Exterior Exit Location
   ```

### Step 3: Integrate with Spaceship

1. **Add Interior Reference to Spaceship**

   Open your spaceship Blueprint or C++ class:

   **In Blueprint:**
   - Add variable: `InteriorLevelName` (Type: Name)
   - Set value: `MAP_Interior_Pathfinder`

   **In C++ (ASpaceship.h):**
   ```cpp
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
   FName InteriorLevelName;
   ```

2. **Create Entry Trigger on Ship Exterior**

   - Add a **Box Collision** component to ship exterior
   - Position at airlock door or boarding ramp
   - Name: `EntryTrigger_Airlock`
   - Set collision to overlap Pawn only

3. **Implement Entry Logic**

   **In Spaceship Blueprint:**
   ```
   On Component Begin Overlap (EntryTrigger)
       ↓
   Cast to Player Character
       ↓
   [Success]
       ↓
   Show "Press E to Enter Ship" UI prompt
       ↓
   On Player Press E
       ↓
   Save Player External State
       ↓
   Load Level Instance (InteriorLevelName)
       ↓
   Wait for Level Loaded
       ↓
   Teleport Player to Interior Player Start
       ↓
   Set Game Mode to Interior Mode
   ```

### Step 4: Level Streaming Setup

**Option A: Using Blueprint (Recommended for Designers)**

1. Create a new Blueprint (Actor or Actor Component)
   - Name: `BP_InteriorLevelManager`

2. Add Level Streaming Logic:
   ```
   Function: LoadInterior(LevelName)
       ↓
   Load Stream Level (LevelName)
       ↓
   Make Visible When Loaded: True
       ↓
   Block on Load: True (for loading screen)
       ↓
   Return: Level Instance
   
   Function: UnloadInterior(LevelInstance)
       ↓
   Unload Stream Level (LevelInstance)
       ↓
   Wait for Unload Complete
   ```

**Option B: Using C++ (Advanced Users)**

```cpp
// In your GameMode or Manager class
void AInteriorManager::LoadInterior(FName LevelName)
{
    FLatentActionInfo LatentInfo;
    UGameplayStatics::LoadStreamLevel(this, LevelName, true, true, LatentInfo);
}

void AInteriorManager::UnloadInterior(FName LevelName)
{
    FLatentActionInfo LatentInfo;
    UGameplayStatics::UnloadStreamLevel(this, LevelName, LatentInfo, false);
}
```

### Step 5: Testing Separate Level Interior

1. **Test Entry**
   - Play from main level with spaceship
   - Approach airlock trigger
   - Press interact key
   - Verify level loads and player spawns inside

2. **Test Exit**
   - Walk to exit trigger
   - Press interact key
   - Verify return to exterior

3. **Test Control Console** (if applicable)
   - Find ship control console
   - Press interact to take control
   - Verify ship movement works
   - Exit control, verify player returns to walking

---

## Approach 2: Embedded Interior

Embedded interiors are built directly into the spaceship actor, making them ideal for small ships with simple layouts.

### Architecture

```
Spaceship Actor
    ↓
├─ Exterior Mesh (Ship Hull)
├─ Interior Mesh (Walkable Space)
├─ SpaceshipInterior Component
├─ Entry Triggers
└─ Control Console
```

### Step 1: Create Interior Geometry

1. **Open Your Spaceship Blueprint**
   - Navigate to Content/Blueprints/Ships/
   - Open your ship Blueprint: `BP_Ship_[Name]`

2. **Add Interior Static Mesh**
   - In Components panel, click **Add Component**
   - Select **Static Mesh**
   - Name: `InteriorMesh`
   - Assign your interior mesh asset
   - Set **Collision Preset**: Custom or BlockAll

3. **Position Interior Relative to Exterior**
   - Set transform to align interior with ship hull
   - Interior should be inside the exterior mesh
   - Ensure floors are level for walking

4. **Verify Collision**
   - Enable **Collision Visibility** in viewport (Alt+C)
   - Check that floor/walls have collision
   - Test by placing a character inside

### Step 2: Add SpaceshipInterior Actor Component

The `ASpaceshipInterior` class handles entry/exit locations for the interior.

1. **Add Interior Component**

   **In Blueprint:**
   - Components panel → Add Component
   - Search for "SpaceshipInterior"
   - Add `SpaceshipInterior` actor

   **In C++ (if extending):**
   ```cpp
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
   ASpaceshipInterior* InteriorInstance;
   ```

2. **Configure Interior Settings**

   In the Details panel for SpaceshipInterior:

   - **Entry Location**: Set to where player spawns when boarding
     - X, Y, Z relative to ship center
     - Example: `(500, 0, 100)` - 5 meters forward, at floor level

   - **Entry Rotation**: Direction player faces when entering
     - Example: `(0, 90, 0)` - facing starboard

   - **Exit Location**: Where player appears when leaving control
     - Should be near control console
     - Example: `(300, -50, 100)` - behind pilot seat

### Step 3: Create Boarding Triggers

1. **Add Entry Collision**

   - Components panel → Add Component → Box Collision
   - Name: `EntryTrigger_Airlock`
   - Position at ship exterior entry point (door, ramp)
   - Size: Large enough for player to walk into
   - **Collision Preset**: Overlap All Dynamic

2. **Implement Boarding Logic**

   **In Spaceship Blueprint Event Graph:**

   ```
   Event: On Entry Trigger Begin Overlap
       ↓
   Cast to Player Character
       ↓
   [Success]
       ↓
   Get Player Controller
       ↓
   Store: Current Player Pawn
       ↓
   Detach Player from Current Pawn
       ↓
   Get Interior Entry Location (from SpaceshipInterior)
       ↓
   Spawn Player Character at Entry Location
       ↓
   Attach Player to Ship (for relative movement)
       ↓
   Hide Exterior Player Pawn
       ↓
   Set: Player Is Inside Ship = True
   ```

3. **Add Exit Collision**

   - Add another Box Collision at interior exit
   - Name: `ExitTrigger_Interior`
   - Place near airlock inside the ship

4. **Implement Exit Logic**

   ```
   Event: On Exit Trigger Begin Overlap
       ↓
   Cast to Player Character
       ↓
   Check: Player Is Inside Ship == True
       ↓
   Get Stored Exterior Pawn
       ↓
   Restore Exterior Pawn Position/Visibility
       ↓
   Detach Player from Ship
       ↓
   Possess Exterior Pawn
       ↓
   Destroy Interior Player Character
       ↓
   Set: Player Is Inside Ship = False
   ```

### Step 4: Handle Ship Movement While Inside

When the player is walking inside a moving ship, they need to move with it.

**Attach Player to Ship (Blueprint):**

```
After Spawning Player Inside:
    ↓
Attach Actor to Actor
    ↓
  Target: Player Character
  Parent: This Ship (Self)
  Socket Name: None
  Location Rule: Keep World
  Rotation Rule: Keep World
  Scale Rule: Keep World
```

**This ensures:**
- Player moves with ship
- Player rotation stays independent
- Gravity still works normally

### Step 5: Add Windows (Optional)

For small ships, players might want to see outside from the interior.

1. **Create Window Meshes**
   - Use emissive materials for monitors/windows
   - Use scene capture components for real window views

2. **Scene Capture for Window View**
   - Add **Scene Capture Component 2D**
   - Position outside ship looking at space
   - Create Render Target texture
   - Apply render target to window material

3. **Optimize Performance**
   - Lower scene capture resolution (256x256 or 512x512)
   - Update only when needed (not every frame)
   - Consider using static skybox instead

### Step 6: Testing Embedded Interior

1. **Test Boarding**
   - Play in editor
   - Approach ship airlock
   - Walk into entry trigger
   - Verify spawn inside ship

2. **Test Movement**
   - While inside, use ship controls to move
   - Walk around interior
   - Verify player moves with ship

3. **Test Exit**
   - Walk to exit trigger
   - Verify return to exterior
   - Check pawn restoration works

---

## Control Console Setup

Both interior approaches use the Ship Control Console to allow piloting from inside.

### Overview

The `AShipControlConsole` actor allows players to take control of the ship while standing inside the interior.

### Step 1: Place Control Console

1. **Add Console to Interior**

   **For Separate Level:**
   - Open interior level
   - Place Actors panel → Search "ShipControlConsole"
   - Drag `ShipControlConsole` into level
   - Position at pilot station (bridge, cockpit)

   **For Embedded Interior:**
   - Open spaceship Blueprint
   - Components panel → Add Component
   - Search "ShipControlConsole"
   - Position inside interior mesh at pilot seat

2. **Position Console**
   - Place where pilot would sit/stand
   - Face forward (toward bow of ship)
   - Height should be accessible (50-100 units above floor)

### Step 2: Configure Console

Select the ShipControlConsole and configure in Details panel:

1. **Owning Ship** (Critical - Must Set!)
   - For Separate Level: Use a reference or level script
   - For Embedded Interior: Set to `Self` (the spaceship)
   - This links the console to the ship it controls

2. **Interaction Range**
   - Default: 150.0 units
   - Adjust based on console size/accessibility
   - Larger values = can interact from farther away

3. **Visual Feedback** (Optional)
   - Add a mesh component for the console physical appearance
   - Add a prompt widget above console ("Press E to Pilot")

### Step 3: Console Interaction Logic

The console needs to detect when the player is near and handle interaction.

**Basic Implementation (Already in C++):**
The `ShipControlConsole` class handles:
- `CanInteract(PlayerController)` - Checks if player is in range
- `Interact(PlayerController)` - Toggles ship control on/off
- `BeginControl()` - Saves walking pawn, possesses ship
- `EndControl()` - Restores walking pawn, exits ship control

**Blueprint Extension (Optional):**

You can extend the console in Blueprint to add:
- Visual feedback (glowing buttons)
- Sound effects (startup/shutdown sounds)
- UI displays (HUD changes)
- Animation (player sitting down)

### Step 4: Player Interaction Detection

The player character needs to detect nearby consoles and interact with them.

**In Player Character Blueprint:**

```
Event: On Interact Key Pressed (E key)
    ↓
Sphere Trace by Channel
    ↓
  Start: Player Location
  End: Player Location + Forward * 200
  Radius: 50
  Channel: Visibility
    ↓
For Each Hit Actor
    ↓
Cast to ShipControlConsole
    ↓
[Success]
    ↓
Can Interact? (Call CanInteract function)
    ↓
[True]
    ↓
Call Interact (Pass Player Controller)
```

**Input Binding (Already configured in DefaultInput.ini):**
- Action: `Interact`
- Key: `E` (default)

### Step 5: Ship Control Mechanics

When the player activates the console:

**BeginControl Sequence:**
1. Player's walking pawn is saved
2. Walking pawn is hidden and disabled
3. Player controller possesses the spaceship pawn
4. Ship movement controls are enabled
5. Camera switches to ship view

**During Control:**
- WASD: Move ship (forward/back, strafe)
- Mouse: Rotate ship (yaw, pitch)
- Space/Ctrl: Move up/down
- E: Exit control

**EndControl Sequence:**
1. Player controller unpossesses ship
2. Walking pawn is restored at exit location
3. Walking pawn becomes visible and enabled
4. Player regains walking controls
5. Ship returns to idle state

### Step 6: Testing Control Console

1. **Test Interaction Range**
   - Stand near console
   - Move away slowly
   - Note exact range where interaction works
   - Adjust InteractionRange if needed

2. **Test Control Activation**
   - Stand near console
   - Press E (interact key)
   - Verify possession switches to ship
   - Verify camera changes to ship view

3. **Test Ship Movement**
   - While controlling: Press W/A/S/D
   - Verify ship moves correctly
   - Test mouse look (ship rotation)
   - Test vertical movement (Space/Ctrl)

4. **Test Control Exit**
   - While controlling: Press E again
   - Verify return to walking pawn
   - Check player spawns at exit location
   - Verify walking controls work again

---

## Player Interaction System

A robust interaction system makes entering ships and using consoles intuitive.

### UI Prompts

**Display Interaction Hints:**

When player is near interactable objects, show UI prompts:

```
Player Near Ship Entry
    ↓
Show Widget: "Press E to Board Ship"

Player Near Control Console
    ↓
Show Widget: "Press E to Pilot Ship"

Player Controlling Ship
    ↓
Show Widget: "Press E to Exit Control"
```

**Widget Blueprint Example:**
- Create: `WBP_InteractionPrompt`
- Add Text: "Press [E] to ..."
- Add Fade Animation
- Position above interactable object

### Interaction Flow

**Complete interaction flow from exterior to piloting:**

```
1. Player walks to ship exterior
    ↓
2. Entry trigger detected
    ↓
3. Show "Board Ship" prompt
    ↓
4. Player presses E
    ↓
5. Load interior (or spawn inside)
    ↓
6. Player walks to control console
    ↓
7. Console proximity detected
    ↓
8. Show "Pilot Ship" prompt
    ↓
9. Player presses E
    ↓
10. Switch to ship control
    ↓
11. Player flies ship
    ↓
12. Player presses E again
    ↓
13. Return to walking in interior
    ↓
14. Player walks to exit
    ↓
15. Exit trigger detected
    ↓
16. Show "Exit Ship" prompt
    ↓
17. Player presses E
    ↓
18. Return to exterior
```

### Interaction Manager (Optional)

For complex games, create a centralized interaction manager:

**BP_InteractionManager:**

```
Responsibilities:
- Track all interactable objects
- Display appropriate prompts
- Handle interaction priorities
- Manage interaction state
- Provide interaction feedback
```

---

## Best Practices

### Design Guidelines

1. **Interior Scale**
   - Keep interiors proportional to exterior
   - 1 UE unit = 1 cm (standard scale)
   - Doorways: 200-250 units tall (2-2.5 meters)
   - Corridors: 300-400 units wide
   - Rooms: Varies by purpose

2. **Player Flow**
   - Clear path from entry to control console
   - Avoid dead ends unless intentional
   - Mark important areas (console, exits)
   - Use lighting to guide player

3. **Interaction Clarity**
   - Always show prompts for interactions
   - Use consistent button mappings (E for all)
   - Provide audio/visual feedback
   - Test with new players

4. **Performance Considerations**
   - Use separate level for large interiors
   - Optimize mesh complexity (< 10k tris per room)
   - Use lighting efficiently (baked where possible)
   - Limit dynamic objects

### Collision Setup

1. **Walkable Surfaces**
   - Use simple box collision for floors
   - Ensure collision is set to "Block All" or "Block Pawn"
   - Test walking on all surfaces

2. **Walls and Obstacles**
   - Use simple collision primitives
   - Avoid complex collision (use simplified meshes)
   - Block Pawn channel minimum

3. **Triggers**
   - Use "Overlap" collision for entry/exit
   - Use "Overlap Pawn" for player-only triggers
   - Size triggers slightly larger than needed

### Lighting

1. **Ambient Lighting**
   - Sky Light: 0.3-0.5 intensity for ship interior
   - Add slight blue tint for sci-fi feel
   - Use HDRI if available

2. **Accent Lighting**
   - Point lights at control stations (high intensity, small radius)
   - Spot lights for corridors (directional, focused)
   - Emissive materials for panels and screens

3. **Performance**
   - Use stationary lights for static geometry
   - Limit dynamic lights (< 10 per room)
   - Bake lighting for production

---

## Optimization Guidelines

### Memory Management

1. **Separate Level Interiors**
   - Load only when needed
   - Unload when player exits
   - Stream asynchronously if possible
   - Show loading screen during load

2. **Embedded Interiors**
   - Keep geometry simple (< 20k tris total)
   - Use instanced meshes for repeated elements
   - LOD system for distant details
   - Cull interior when exterior camera active

### Rendering Optimization

1. **Occlusion Culling**
   - Use occlusion volumes for rooms
   - Hide interior when outside (set visibility)
   - Hide exterior when inside (if separate level)

2. **Mesh Complexity**
   - Use modular pieces (reusable)
   - Simple collision hulls
   - LOD levels for detailed meshes
   - Instancing for repeated elements

3. **Texture Memory**
   - Use texture atlases for interior materials
   - Share materials across ships
   - Compress textures appropriately
   - Use lower resolution for background details

### Gameplay Performance

1. **Tick Optimization**
   - Disable tick on interior when not in use
   - Use timers instead of tick where possible
   - Update UI only when values change

2. **Collision Checks**
   - Use simple collision shapes
   - Limit number of active triggers
   - Disable triggers when not needed

---

## Common Issues and Solutions

### Issue: Player Falls Through Floor

**Symptoms:**
- Player spawns inside ship and immediately falls
- Walking results in falling through floor

**Solutions:**
1. Check collision is enabled on floor mesh
2. Set collision preset to "BlockAll" or "BlockAllDynamic"
3. Ensure player capsule collision is correct
4. Verify floor mesh has collision geometry

**Debugging:**
- Enable collision visualization (Alt+C in viewport)
- Check Output Log for collision warnings
- Use "Show Collision" in play mode

### Issue: Can't Interact with Console

**Symptoms:**
- Pressing E near console does nothing
- No UI prompt appears

**Solutions:**
1. Verify OwningShip reference is set on console
2. Check InteractionRange is large enough
3. Ensure player's interaction trace is working
4. Verify input binding for Interact action (E key)

**Debugging:**
- Add debug prints to interaction trace
- Check console location is accessible
- Verify console is not hidden/disabled

### Issue: Ship Doesn't Move When Controlled

**Symptoms:**
- Taking control works, but ship is stuck
- Input has no effect on ship movement

**Solutions:**
1. Check MovementComponent is active
2. Verify input bindings in DefaultInput.ini
3. Ensure SetupPlayerInputComponent is called
4. Check TurnRate is not zero

**Debugging:**
- Print input values in MoveForward/MoveRight functions
- Check MovementComponent->IsActive()
- Verify possession is successful

### Issue: Player Stuck After Exiting Control

**Symptoms:**
- Exiting control leaves player unable to move
- Camera is broken
- Player pawn is missing

**Solutions:**
1. Verify SavedExternalPawn is not null
2. Check exit location is valid
3. Ensure pawn is restored properly
4. Verify collision is enabled on restored pawn

**Debugging:**
- Add debug log in EndControl function
- Check if SavedExternalPawn exists
- Verify teleport to exit location succeeds

### Issue: Interior Not Loading (Separate Level)

**Symptoms:**
- Loading screen appears but interior never loads
- Errors in Output Log about missing level

**Solutions:**
1. Verify level name is correct (case-sensitive)
2. Check level is saved in Content/Maps/
3. Ensure level is not marked for exclusion
4. Check level streaming settings

**Debugging:**
- Check Output Log for "Failed to load level" errors
- Verify level exists in Content Browser
- Try loading level manually in editor

### Issue: Ship Moves Without Player (Embedded Interior)

**Symptoms:**
- Ship moves but player stays in place
- Player is left behind when ship rotates

**Solutions:**
1. Attach player to ship actor
2. Use "Snap to Target" attachment
3. Keep world rotation rule
4. Update attachment each frame if needed

**Debugging:**
- Check attachment in editor (show component hierarchy)
- Verify attachment rules are correct
- Test with simple movement first

---

## Advanced Topics

### Dynamic Interior Generation

For procedural or modular interiors:

1. **Modular Components**
   - Create reusable room templates
   - Snap together using sockets
   - Generate layouts at runtime
   - Use seed for consistency

2. **Procedural Placement**
   - Place objects algorithmically
   - Use splines for corridors
   - Spawn furniture/props
   - Ensure collision works

### Multiplayer Considerations

For networked games:

1. **Replication**
   - Replicate interior state
   - Replicate console control state
   - Handle multiple players in interior
   - Sync boarding/exiting

2. **Authority**
   - Server controls ship movement
   - Server validates boarding
   - Clients predict interior movement
   - Handle possession changes

### Damage and Destruction

For advanced gameplay:

1. **Interior Damage**
   - Deformation of walls
   - Destruction of objects
   - Fire/smoke effects
   - Breach/depressurization

2. **Emergency Systems**
   - Backup lights
   - Warning sounds
   - Emergency exits
   - Life support indicators

### Crew NPCs

For living interiors:

1. **NPC Crew**
   - Place crew at stations
   - Patrol/idle behaviors
   - Interaction with player
   - Context-aware dialogue

2. **NPC Integration**
   - Use Personnel System (see PersonnelSystemGuide.md)
   - Assign crew roles
   - Crew AI behaviors
   - Crew customization

---

## Complete Example Workflow

### Example: Small Fighter Interior (Embedded)

This example creates a simple fighter cockpit interior.

**Step 1: Prepare Assets**
1. Interior mesh: Cockpit with pilot seat
2. Exterior mesh: Fighter hull
3. Console mesh: Control panel
4. Interaction widgets

**Step 2: Set Up Spaceship Blueprint**

Create or open `BP_Ship_Fighter`:

1. Add Static Mesh Component: `ExteriorMesh`
   - Assign fighter hull mesh
   - Set collision to "BlockAll"

2. Add Static Mesh Component: `InteriorMesh`
   - Assign cockpit interior mesh
   - Position inside exterior hull
   - Set collision to "BlockAll"

3. Add SpaceshipInterior Component: `InteriorInstance`
   - Entry Location: (200, 0, 50) - front of ship
   - Entry Rotation: (0, 0, 0) - facing forward
   - Exit Location: (150, -30, 50) - behind seat

4. Add Box Collision: `EntryTrigger`
   - Position at rear of ship (hatch location)
   - Size: (200, 200, 300)
   - Collision: "Overlap Pawn"

5. Add Ship Control Console: `ControlConsole`
   - Position at pilot seat: (180, 0, 80)
   - Owning Ship: Self
   - Interaction Range: 150

**Step 3: Implement Boarding Logic**

In Event Graph:

```
Event: Begin Play
    ↓
Set: Player Is Inside = False

Event: Entry Trigger Begin Overlap
    ↓
Cast to Player Character
    ↓
[Success] Branch: NOT Player Is Inside
    ↓
Get Player Controller
    ↓
Store: Current Player Pawn → Saved Exterior Pawn
    ↓
Spawn Actor (Player Character Class)
    ↓
  Location: InteriorInstance → Get Entry Location
  Rotation: InteriorInstance → Get Entry Rotation
    ↓
Store: New Player Pawn → Interior Player Pawn
    ↓
Attach Actor to Actor
    ↓
  Target: Interior Player Pawn
  Parent: Self
  Snap to Target: True
    ↓
Player Controller → Possess: Interior Player Pawn
    ↓
Set Actor Hidden (Saved Exterior Pawn): True
    ↓
Set: Player Is Inside = True
    ↓
Show Widget: WBP_InteriorHUD

Event: Entry Trigger End Overlap (when exiting interior)
    ↓
Cast to Player Character
    ↓
[Success] Branch: Player Is Inside
    ↓
Get Player Controller
    ↓
Player Controller → Possess: Saved Exterior Pawn
    ↓
Set Actor Hidden (Saved Exterior Pawn): False
    ↓
Destroy Actor: Interior Player Pawn
    ↓
Set: Player Is Inside = False
    ↓
Remove Widget: WBP_InteriorHUD
```

**Step 4: Test**

1. Place BP_Ship_Fighter in test level
2. Play in editor
3. Walk to entry trigger (rear of ship)
4. Verify spawn inside cockpit
5. Walk to control console
6. Press E to pilot ship
7. Verify ship controls work
8. Press E to exit control
9. Walk to exit trigger
10. Verify return to exterior

**Step 5: Polish**

1. Add entry/exit sound effects
2. Add cockpit HUD overlay
3. Add window views (scene captures)
4. Add ambient sounds (engine hum)
5. Test with different movement scenarios

---

## Quick Reference

### Checklist: Separate Level Interior

- [ ] Create interior level (MAP_Interior_[ShipName])
- [ ] Configure world settings (gravity, kill Z)
- [ ] Build interior geometry with collision
- [ ] Add player start at entry point
- [ ] Add exit triggers at airlocks
- [ ] Configure level streaming logic
- [ ] Add entry trigger to ship exterior
- [ ] Test loading/unloading transitions
- [ ] Add ship control console
- [ ] Test full boarding → piloting → exiting flow

### Checklist: Embedded Interior

- [ ] Add interior mesh to spaceship Blueprint
- [ ] Position interior inside exterior hull
- [ ] Verify collision on interior surfaces
- [ ] Add SpaceshipInterior component
- [ ] Configure entry/exit locations
- [ ] Add entry trigger on exterior
- [ ] Implement boarding logic (spawn player inside)
- [ ] Implement exit logic (restore exterior pawn)
- [ ] Attach player to ship for movement
- [ ] Add ship control console
- [ ] Test boarding/exiting
- [ ] Test piloting from interior

### Key Classes Reference

**C++ Classes:**
- `ASpaceship` - Base spaceship class
- `ASpaceshipInterior` - Interior configuration actor
- `AShipControlConsole` - Interactive control console
- `UFloatingPawnMovement` - Ship movement component

**Blueprint Base Classes:**
- `BP_Spaceship_Base` - Recommended base for all ships
- `BP_PlayerCharacter` - Player walking character
- `WBP_InteractionPrompt` - UI interaction widgets

### Common Functions

**SpaceshipInterior:**
- `GetEntryLocation()` - Returns entry spawn point
- `GetEntryRotation()` - Returns entry facing direction
- `GetExitLocation()` - Returns exit spawn point

**ShipControlConsole:**
- `Interact(PlayerController)` - Toggle ship control
- `CanInteract(PlayerController)` - Check if interaction possible
- `BeginControl(PC, ExternalPawn)` - Start ship control
- `EndControl(PC)` - Stop ship control

**Spaceship:**
- `EnterInterior(PlayerController)` - Transition to interior
- `BeginControl(PC, ExternalPawn)` - Start piloting
- `EndControl(PC)` - Stop piloting
- `SetupPlayerInputComponent(InputComponent)` - Bind controls

---

## Related Documentation

**Essential Reading:**
- `SpaceshipDataAssetGuide.md` - Ship configuration and stats
- `ShipControlGuide.md` - Ship control system details
- `SpaceshipDesignerWorkflow.md` - Ship creation workflow

**Supplementary Guides:**
- `CONTENT_ORGANIZATION.md` - Asset naming and organization
- `EmptyMapSetup.md` - Level setup and configuration
- `BLUEPRINT_CREATION_GUIDE.md` - Blueprint best practices

**System Integration:**
- `PersonnelSystemGuide.md` - Adding crew to interiors
- `StationSetupGuide.md` - Similar principles for stations
- `TradingSystemGuide.md` - Trading from ship interiors

---

## Conclusion

You now have a complete understanding of how to implement spaceship interiors in Adastrea! 

**Key Takeaways:**
- Use **Separate Levels** for large, complex interiors
- Use **Embedded Interiors** for small, simple interiors
- Always provide clear interaction feedback
- Test thoroughly at each step
- Optimize for performance

**Next Steps:**
1. Choose your approach (separate level vs. embedded)
2. Follow the step-by-step workflow for your choice
3. Implement control console integration
4. Test all interaction flows
5. Polish with UI, sound, and visual effects

For questions or issues, refer to the troubleshooting section or check related documentation.

**Happy ship building!** 🚀

---

**Last Updated**: 2025-11-05  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
