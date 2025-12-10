# BP_ImportAI - Autonomous AI Ship

> **AI-controlled ship that flies autonomously to random coordinates**

## Overview

BP_ImportAI is an AI-controlled spaceship based on BP_Import that autonomously flies around your level to random coordinates. Perfect for:
- Testing space environments
- Adding ambient traffic
- Creating NPC ships for encounters
- Demonstrating ship movement

Just drag it into your level, press Play, and watch it fly! No scripting or input required.

## Quick Links

- 🚀 **Quick Start**: [BP_IMPORTAI_QUICKSTART.md](BP_IMPORTAI_QUICKSTART.md) - Get started in 3 steps
- 📖 **Full Guide**: [BP_IMPORTAI_IMPLEMENTATION_GUIDE.md](BP_IMPORTAI_IMPLEMENTATION_GUIDE.md) - Complete documentation
- ✅ **Testing**: [BP_IMPORTAI_TESTING_CHECKLIST.md](BP_IMPORTAI_TESTING_CHECKLIST.md) - Comprehensive test checklist

## Features

✅ **Fully Autonomous** - No player input needed  
✅ **Random Targets** - Generates random coordinates to fly to  
✅ **Configurable** - Adjust speed, distance, turn rate  
✅ **Debug Mode** - Visual debugging with lines and spheres  
✅ **Multiple Ships** - Add as many as you want  
✅ **Performance Friendly** - Efficient C++ implementation

## What's Included

### C++ Component
**SimpleAIMovementComponent** - Handles all AI movement logic
- Location: `Source/Adastrea/Ships/SimpleAIMovementComponent.h/cpp`
- Automatically generates and navigates to random targets
- Configurable movement parameters
- Optional debug visualization

### Blueprint
**BP_ImportAI** - The AI ship actor
- Location: `Content/Blueprints/Ships/BP_ImportAI.uasset`
- Based on BP_Import
- Configured with AI controller
- Ready to use out of the box

### Automation Script
**CreateAIShip.py** - Automates Blueprint setup
- Location: `CreateAIShip.py` (project root)
- Creates BP_ImportAI from BP_Import
- Configures AI settings
- Run once during setup

### Documentation
- **BP_IMPORTAI_QUICKSTART.md** - 3-step quick start
- **BP_IMPORTAI_IMPLEMENTATION_GUIDE.md** - Complete guide
- **BP_IMPORTAI_TESTING_CHECKLIST.md** - Testing procedures
- **BP_IMPORTAI_README.md** - This file

## How It Works

```
1. Ship spawns in level
   ↓
2. AI controller possesses ship
   ↓
3. SimpleAIMovementComponent generates random target
   ↓
4. Ship flies toward target
   ↓
5. Ship arrives at target
   ↓
6. Component generates new target
   ↓
7. Repeat forever
```

## Configuration

### Movement Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| **Min Distance** | 10000 | Minimum distance for random targets |
| **Max Distance** | 50000 | Maximum distance for random targets |
| **Move Speed** | 2000 | Ship movement speed |
| **Arrival Threshold** | 500 | Distance to consider "arrived" |
| **Turn Rate** | 90 | Rotation speed (degrees/second) |

### Advanced Options

| Parameter | Default | Description |
|-----------|---------|-------------|
| **Constrain To Horizontal Plane** | True | Keep ship level (yaw only) |
| **Show Debug** | False | Enable debug visualization |

## Setup (Quick Version)

### 1. Compile C++ Code
```
Open Adastrea.sln → Build (Development Editor)
```

### 2. Run Setup Script
```python
# In Unreal Editor Python console:
import CreateAIShip
CreateAIShip.create_ai_ship()
```

### 3. Configure Blueprint
```
1. Open BP_ImportAI
2. Add Component → SimpleAIMovementComponent
3. Set Show Debug = TRUE (for testing)
4. Compile and Save
```

### 4. Test
```
1. Drag BP_ImportAI into level
2. Press Alt+P
3. Watch it fly! 🚀
```

See [BP_IMPORTAI_QUICKSTART.md](BP_IMPORTAI_QUICKSTART.md) for detailed instructions.

## Usage Examples

### Single AI Ship (Testing)
```
1. Add BP_ImportAI to level
2. Enable Show Debug
3. Play and observe movement
```

### Multiple AI Ships (Traffic)
```
1. Add 5-10 BP_ImportAI instances
2. Position around level
3. Disable Show Debug
4. Play for ambient space traffic
```

### Custom Behavior Variant
```
1. Duplicate BP_ImportAI → BP_FastScout
2. Set Move Speed = 5000
3. Set Max Distance = 100000
4. Save as patrol ship variant
```

## Troubleshooting

### Ship Doesn't Move
- ✅ Check SimpleAIMovementComponent is added
- ✅ Check Auto Possess AI is enabled
- ✅ Check C++ code compiled

### Ship Moves Erratically
- ✅ Reduce Move Speed
- ✅ Increase Turn Rate
- ✅ Adjust Arrival Threshold

### Can't Find Component
- ✅ Compile C++ code first
- ✅ Restart Unreal Editor
- ✅ Check compilation logs

See [BP_IMPORTAI_IMPLEMENTATION_GUIDE.md](BP_IMPORTAI_IMPLEMENTATION_GUIDE.md) for detailed troubleshooting.

## Technical Details

### Architecture
- **Component-Based**: Uses ActorComponent pattern
- **AI-Driven**: Leverages Unreal's AI controller system
- **Pawn Movement**: Uses FloatingPawnMovement for physics
- **Tick-Based**: Updates every frame for smooth movement

### Performance
- **Efficient**: C++ implementation for best performance
- **Scalable**: Tested with 20+ ships
- **Debug Cost**: Debug visualization adds minor overhead
- **Production**: Disable debug for optimal performance

### Integration
Works with:
- ✅ Existing ship Blueprints (ASpaceship base class)
- ✅ Player ships (when AI-controlled)
- ✅ Custom ship variants
- ✅ Multiplayer (runs on server)

## Development

### Extending Functionality

To add custom behavior:

1. **Subclass in Blueprint**
   - Duplicate BP_ImportAI
   - Add Blueprint logic
   - Call SimpleAIMovementComponent functions

2. **Subclass in C++**
   - Inherit from SimpleAIMovementComponent
   - Override virtual functions
   - Add custom movement logic

3. **Event-Based**
   - Use Blueprint events
   - Call `GenerateNewTarget()` manually
   - Respond to `GetTargetLocation()` changes

### API Reference

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

// Check if arrived at target
UFUNCTION(BlueprintCallable, BlueprintPure)
bool HasArrivedAtTarget() const;
```

## Roadmap

Potential future enhancements:

- [ ] Obstacle avoidance using Navigation System
- [ ] Formation flying for multiple ships
- [ ] Waypoint patrol routes
- [ ] React to player proximity
- [ ] Faction-based behavior
- [ ] Combat integration
- [ ] Dynamic speed based on distance

## Contributing

Improvements welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Reporting Issues

Found a bug? Please include:
- Unreal Engine version
- Steps to reproduce
- Expected vs actual behavior
- Output Log errors
- Screenshot/video if relevant

## License

Same as Adastrea project - see [LICENSE](LICENSE)

## Credits

**Created by**: Adastrea Development Team  
**Date**: 2025-12-10  
**Version**: 1.0

## Related Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) - Project architecture
- [CODE_STYLE.md](CODE_STYLE.md) - Coding standards
- [BLUEPRINT_REQUIREMENTS_LIST.md](BLUEPRINT_REQUIREMENTS_LIST.md) - Blueprint reference

---

**Need Help?**
- 📖 Read the [Implementation Guide](BP_IMPORTAI_IMPLEMENTATION_GUIDE.md)
- 🚀 Follow the [Quick Start](BP_IMPORTAI_QUICKSTART.md)
- ✅ Check the [Testing Checklist](BP_IMPORTAI_TESTING_CHECKLIST.md)
- 🐛 Search existing issues on GitHub
- 💬 Ask in project discussions

**Happy Flying!** 🚀✨
