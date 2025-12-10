# BP_ImportAI Implementation Guide

## Overview

BP_ImportAI is an AI-controlled ship that autonomously flies to random coordinates in the level. It's based on BP_Import but uses an AI controller and the SimpleAIMovementComponent for autonomous navigation.

## Features

- ✅ Autonomous flight to random locations
- ✅ Automatic target generation when destination reached
- ✅ Configurable movement speed and distances
- ✅ Debug visualization (optional)
- ✅ No player input required - just add to level and play!

## Implementation Components

### 1. SimpleAIMovementComponent (C++)

**Location**: `Source/Adastrea/Ships/SimpleAIMovementComponent.h/cpp`

This component handles all AI movement logic:
- Generates random target locations
- Moves ship toward target using AddMovementInput
- Rotates ship to face target
- Detects arrival and generates new target
- Optional debug visualization

**Key Properties**:
- `MinDistance` (10000.0) - Minimum distance for random target
- `MaxDistance` (50000.0) - Maximum distance for random target
- `MoveSpeed` (2000.0) - Movement speed
- `ArrivalThreshold` (500.0) - Distance to consider "arrived"
- `TurnRate` (90.0) - Rotation speed in degrees/second
- `bShowDebug` (false) - Enable debug visualization

### 2. BP_ImportAI (Blueprint)

**Location**: `Content/Blueprints/Ships/BP_ImportAI.uasset`

A duplicate of BP_Import configured with:
- AI Controller class (default AIController)
- Auto Possess AI set to "Placed in World or Spawned"
- SimpleAIMovementComponent added

### 3. CreateAIShip.py (Setup Script)

**Location**: `CreateAIShip.py`

Python script to automate creation of BP_ImportAI and setup.

## Setup Instructions

### Step 1: Compile C++ Code

1. Open Visual Studio solution
2. Build the project (Development Editor configuration)
3. Wait for compilation to complete
4. Close and reopen Unreal Editor if it's running

### Step 2: Run Setup Script

**Option A: Via Unreal Editor Python Console**

1. Open Unreal Editor
2. Go to **Window** → **Developer Tools** → **Output Log**
3. Change log filter to **Python**
4. In the Python console at bottom, run:
   ```python
   exec(open('CreateAIShip.py').read())
   ```

**Option B: Via MCP (if configured)**

```
Execute CreateAIShip.py in Unreal Engine
```

### Step 3: Configure BP_ImportAI

1. Navigate to `Content/Blueprints/Ships/`
2. Double-click **BP_ImportAI** to open Blueprint Editor
3. In the **Components** panel, click **+ Add Component**
4. Search for `SimpleAIMovement`
5. Click **SimpleAIMovementComponent** to add it
6. Select the component and configure in **Details** panel:
   - **Min Distance**: 10000.0 (or adjust as needed)
   - **Max Distance**: 50000.0 (or adjust as needed)
   - **Move Speed**: 2000.0 (controls FloatingPawnMovement MaxSpeed)
   - **Arrival Threshold**: 500.0
   - **Turn Rate**: 90.0
   - **Show Debug**: ✓ Check for testing (uncheck for production)
7. Click **Compile** (top toolbar)
8. Click **Save** (top toolbar)

### Step 4: Test in Level

1. Open any level (e.g., TestLevel)
2. In **Content Browser**, navigate to `Content/Blueprints/Ships/`
3. Drag **BP_ImportAI** into the viewport
4. Position it where you want (it will start flying from there)
5. Press **Alt+P** (or click **Play** button) to test
6. The ship should:
   - Automatically possess itself with AI
   - Generate a random target location
   - Fly toward that location
   - Generate a new target when it arrives
   - Repeat indefinitely!

## Configuration

### Adjusting Movement Behavior

Open BP_ImportAI and select SimpleAIMovementComponent:

**For Faster Movement**:
- Increase `MoveSpeed` (e.g., 5000.0)
- Increase `TurnRate` (e.g., 180.0)

**For Shorter Trips**:
- Decrease `MaxDistance` (e.g., 20000.0)
- Decrease `MinDistance` (e.g., 5000.0)

**For More Precise Arrival**:
- Decrease `ArrivalThreshold` (e.g., 200.0)

**For Debugging**:
- Enable `bShowDebug` to see:
  - Green line to target
  - Yellow sphere at target location
  - Distance text above ship

## Troubleshooting

### Ship Doesn't Move

**Check:**
- Is the ship possessed by AI? (Auto Possess AI should be set)
- Is SimpleAIMovementComponent added?
- Is the C++ code compiled?
- Check Output Log for errors

**Solution:**
- Open BP_ImportAI Class Defaults
- Verify `Auto Possess AI` = "Placed in World or Spawned"
- Verify `AI Controller Class` = "AIController"

### Ship Moves Erratically

**Check:**
- Is MoveSpeed too high?
- Is TurnRate too low?

**Solution:**
- Reduce `MoveSpeed` to 1000-2000
- Increase `TurnRate` to 90-180

### Ship Goes Too Far Away

**Check:**
- Is MaxDistance too high?

**Solution:**
- Reduce `MaxDistance` to 20000-30000
- Adjust camera/viewport settings to track ship

### Ship Stops Moving

**Check:**
- Did it reach a target and fail to generate new one?
- Check Output Log for errors

**Solution:**
- Enable `bShowDebug` to see target location
- Check that ArrivalThreshold is reasonable

## Adding Multiple AI Ships

You can add as many BP_ImportAI instances as you want:

1. Drag BP_ImportAI into level multiple times
2. Position them at different locations
3. Optionally, configure each with different movement parameters
4. All will fly independently!

**Performance Note**: Each ship runs its own AI logic. For 10+ ships, consider optimization.

## Advanced Customization

### Creating Variants

To create AI ships with different behaviors:

1. Duplicate BP_ImportAI
2. Rename (e.g., BP_FastScout_AI)
3. Adjust SimpleAIMovementComponent properties
4. Save and use

Example variants:
- **Fast Scout**: MoveSpeed=5000, MaxDistance=100000
- **Patrol Ship**: MoveSpeed=1000, MaxDistance=10000
- **Wanderer**: MoveSpeed=500, MaxDistance=50000

### Adding Custom Logic

You can extend BP_ImportAI in Blueprint:

1. Open BP_ImportAI Event Graph
2. Add custom events (e.g., detect player nearby)
3. Call SimpleAIMovementComponent functions:
   - `GenerateNewTarget()` - Force new target
   - `GetTargetLocation()` - Get current target
   - `GetDistanceToTarget()` - Get distance to target
   - `HasArrivedAtTarget()` - Check if arrived

## Technical Details

### How It Works

1. **BeginPlay**: Component generates first random target
2. **Tick**: Every frame:
   - Check if arrived (distance < ArrivalThreshold)
   - If arrived → Generate new target
   - If not arrived:
     - Calculate direction to target
     - Call AddMovementInput on Pawn
     - Rotate toward target

3. **AI Controller**: Pawn uses AI controller instead of player controller, allowing AddMovementInput to work autonomously

### Component Integration

SimpleAIMovementComponent is designed to work with any Pawn that has:
- FloatingPawnMovement component (or similar)
- Ability to be possessed by AIController

It should work with:
- ✅ ASpaceship (base class)
- ✅ BP_Import
- ✅ BP_PlayerShip (when AI-controlled)
- ✅ Any custom ship Blueprints

## Code Reference

### C++ Component Interface

```cpp
// Generate new random target
UFUNCTION(BlueprintCallable)
void GenerateNewTarget();

// Get current target location
UFUNCTION(BlueprintCallable, BlueprintPure)
FVector GetTargetLocation() const;

// Get distance to target
UFUNCTION(BlueprintCallable, BlueprintPure)
float GetDistanceToTarget() const;

// Check if arrived
UFUNCTION(BlueprintCallable, BlueprintPure)
bool HasArrivedAtTarget() const;
```

### Python Script Functions

```python
# Main function
create_ai_ship()
  - Creates BP_ImportAI from BP_Import
  - Configures AI controller
  - Sets Auto Possess AI
```

## Best Practices

1. **Testing**: Always enable `bShowDebug` when first testing
2. **Performance**: Disable `bShowDebug` in production
3. **Movement**: Start with conservative speeds and distances
4. **Iteration**: Adjust parameters incrementally
5. **Variants**: Create variants for different AI behaviors

## FAQ

**Q: Can I use this with other ship Blueprints?**
A: Yes! Add SimpleAIMovementComponent to any ship Blueprint and set Auto Possess AI.

**Q: How do I make the ship follow a specific path?**
A: This component is for random movement. For paths, use Unreal's Navigation System or custom Blueprint logic.

**Q: Can the AI avoid obstacles?**
A: Not currently. This is a simple AI. Add collision detection or navigation for obstacle avoidance.

**Q: Does it work in multiplayer?**
A: Should work, but not tested. AI runs on server in multiplayer.

**Q: Can I control how often it picks new targets?**
A: Yes, adjust `ArrivalThreshold`. Larger = changes target less frequently.

## Related Files

- `Source/Adastrea/Ships/SimpleAIMovementComponent.h` - Component header
- `Source/Adastrea/Ships/SimpleAIMovementComponent.cpp` - Component implementation
- `CreateAIShip.py` - Setup automation script
- `Content/Blueprints/Ships/BP_ImportAI.uasset` - AI ship Blueprint

## Support

If you encounter issues:
1. Check Output Log for error messages
2. Enable `bShowDebug` for visual debugging
3. Verify all setup steps completed
4. Check that C++ code compiled successfully

---

**Version**: 1.0  
**Last Updated**: 2025-12-10  
**Author**: Adastrea Development Team
