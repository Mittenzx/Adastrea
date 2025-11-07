# Mittenzx Mk1 Spaceship Setup Guide

This guide explains how to use the Mittenzx Mk1 spaceship in your Adastrea project. The Mittenzx Mk1 is a versatile multi-role spacecraft with balanced capabilities for exploration, light combat, and trade missions.

---

## Overview

**Mittenzx Mk1** is a C++ Data Asset class that inherits from `USpaceshipDataAsset`. It comes pre-configured with balanced stats suitable for various gameplay scenarios.

### Key Features
- **Multi-Role Design**: Balanced stats for exploration, combat, and trade
- **Auto-Spawn Capable**: Can be configured to spawn on game start
- **Interior Support**: Automatically includes interior functionality
- **Player Possessable**: Ready for player control out of the box

### Ship Characteristics
- **Hull Strength**: 1000 (more durable than basic scouts)
- **Cargo Capacity**: 75 tons (good for trade and missions)
- **Weapon Slots**: 2 (capable of light combat)
- **Crew**: 2-4 (efficient operation)
- **Manufacturer**: Mittenzx Industries
- **Rarity**: Uncommon

---

## Quick Setup (5 Minutes)

### For Designers (No C++ Knowledge Required)

If the C++ code has already been compiled, follow these steps:

#### Step 1: Create the Data Asset
1. Open Unreal Editor
2. Navigate to `Content Browser` → `Content/DataAssets/Ships/`
3. **Right-click** → **Miscellaneous** → **Data Asset**
4. Search for and select **MittenzxMk1DataAsset**
5. Name it: `DA_Ship_MittenzxMk1`
6. **Save** the asset

✅ The Data Asset is already configured with all stats!

#### Step 2: Create a Spaceship Blueprint
1. Navigate to `Content Browser` → `Content/Blueprints/Ships/`
2. **Right-click** → **Blueprint Class**
3. Search for and select **Spaceship** as parent class
4. Name it: `BP_Ship_MittenzxMk1`
5. **Open** the Blueprint
6. In the Details panel, set the mesh and visual appearance (optional)
7. **Save** and **Compile**

#### Step 3: Configure Game Mode to Spawn Mittenzx Mk1
1. Open your level in Unreal Editor
2. Go to **Window** → **World Settings**
3. Under **Game Mode Override**, select or create `AAdastreaGameMode` Blueprint
4. In the Game Mode settings:
   - Set **DefaultSpaceshipClass** to `BP_Ship_MittenzxMk1`
   - Enable **bAutoSpawnPlayerShip** (checkbox)
   - Choose **bSpawnAtCenter** (true for center, false for random position)
5. Ensure your level has a **SpaceSectorMap** actor placed
6. **Save** the level

#### Step 4: Test in Play Mode
1. Click **Play** button in editor
2. The Mittenzx Mk1 should spawn automatically
3. You should be able to control the ship immediately

✅ You're flying the Mittenzx Mk1!

---

## Advanced Setup

### For Programmers

The Mittenzx Mk1 is implemented as a C++ class for performance and consistency.

#### File Locations
- **Header**: `Source/Adastrea/Public/Ships/MittenzxMk1DataAsset.h`
- **Implementation**: `Source/Adastrea/Ships/MittenzxMk1DataAsset.cpp`
- **Template**: `Assets/SpaceshipTemplates/MultiRole_MittenzxMk1.yaml`

#### Building the Code
1. Ensure Unreal Engine 5.6 is installed
2. Right-click `Adastrea.uproject` → **Generate Visual Studio project files**
3. Open `Adastrea.sln` in Visual Studio
4. Build in **Development Editor** configuration
5. Launch Unreal Editor

#### Customizing Stats
To create a variant:
1. Create a new C++ class inheriting from `UMittenzxMk1DataAsset`
2. Override the constructor
3. Modify desired stats
4. Rebuild the project

Example:
```cpp
UMyCustomMittenzx::UMyCustomMittenzx()
{
    // Call parent constructor first
    UMittenzxMk1DataAsset::UMittenzxMk1DataAsset();
    
    // Then customize
    ShipName = FText::FromString(TEXT("Mittenzx Mk1 Custom"));
    HullStrength = 1200.0f;  // Increased durability
    MaxSpeed = 800.0f;       // Increased speed
}
```

---

## Interior System

The Mittenzx Mk1 automatically includes interior functionality inherited from `ASpaceship`.

### How Interior Works
1. The ship spawns with an `ASpaceshipInterior` instance
2. The interior is hidden by default
3. Call `EnterInterior(PlayerController)` to transition inside
4. Interior includes entry/exit points

### Customizing the Interior (Blueprint)
1. Open `BP_Ship_MittenzxMk1`
2. Override `BeginPlay` event
3. Spawn custom interior actors
4. Set `InteriorInstance` reference

---

## Player Possession

The Mittenzx Mk1 is automatically possessable through the `AAdastreaGameMode`.

### Automatic Possession (Default)
- Set `bAutoSpawnPlayerShip = true` in Game Mode
- Player controller automatically possesses the ship on spawn

### Manual Possession (Blueprint)
```cpp
// Get player controller
APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

// Get the ship reference
ASpaceship* Ship = ...; // Your ship reference

// Possess the ship
PC->Possess(Ship);
```

### Using Control Console
For interior ship control:
1. Place a `ShipControlConsole` in the interior
2. Walk up to the console
3. Call `BeginControl(PC, WalkingPawn)` to pilot from interior
4. Call `EndControl(PC)` to exit control and return to walking

---

## Ship Stats Reference

### Combat Capabilities
- **Armor**: 35 (decent protection)
- **Shields**: 500 (good defense)
- **Weapons**: 2 slots (light combat)
- **Point Defense**: 4/10 (moderate)

### Mobility
- **Max Speed**: 750 m/s (fast)
- **Acceleration**: 110 m/s² (good)
- **Maneuverability**: 7/10 (agile)
- **Jump Range**: 22 light-years (good)

### Utility
- **Sensors**: 12,000 units range (good)
- **Stealth**: 5/10 (moderate)
- **Repair**: 5/10 (self-sufficient)
- **Science**: 5/10 (balanced)

### Operations
- **Power**: 4500 units (good)
- **Life Support**: 7/10 (excellent)
- **Drones**: 12 (good for exploration)
- **Hangar**: 1 small craft

---

## Comparison with MyFirstSpaceship

| Feature | MyFirstSpaceship | Mittenzx Mk1 |
|---------|------------------|--------------|
| Hull Strength | 750 | 1000 |
| Cargo Capacity | 50 | 75 |
| Weapon Slots | 1 | 2 |
| Max Speed | 850 | 750 |
| Role | Scout | Multi-Role |
| Rarity | Common | Uncommon |

**Mittenzx Mk1 Advantages:**
- +33% more durable (hull)
- +50% more cargo space
- Double the weapons
- Better balanced for multiple roles
- Includes hangar bay

**MyFirstSpaceship Advantages:**
- +13% faster speed
- Better sensor range
- Higher stealth rating
- Specialized for scouting

---

## Troubleshooting

### Ship Doesn't Spawn
**Cause**: Game Mode not configured correctly
**Solution**: 
1. Check World Settings → Game Mode Override is set
2. Verify `DefaultSpaceshipClass` is set to your BP_Ship_MittenzxMk1
3. Ensure `bAutoSpawnPlayerShip` is enabled
4. Confirm a SpaceSectorMap actor exists in the level

### Can't Control the Ship
**Cause**: Possession not occurring
**Solution**:
1. Check player controller exists in level
2. Verify input mappings in Project Settings
3. Ensure ship Blueprint has input enabled
4. Check console for error messages

### Interior Doesn't Work
**Cause**: Interior instance not created
**Solution**:
1. Verify `InteriorInstance` is set in Blueprint
2. Check BeginPlay spawns interior correctly
3. Ensure interior has valid entry/exit points

### Compilation Errors
**Cause**: Project needs to be rebuilt
**Solution**:
1. Close Unreal Editor
2. Delete `Intermediate` and `Binaries` folders
3. Right-click `.uproject` → Generate VS project files
4. Build in Development Editor configuration
5. Reopen editor

---

## Best Practices

### For Designers
- Use the YAML template as reference for creating variants
- Test spawn behavior in different levels
- Customize visual appearance in Blueprint
- Document any custom modifications

### For Programmers
- Inherit from UMittenzxMk1DataAsset for variants
- Keep constructor initialization consistent
- Use BlueprintCallable functions for gameplay integration
- Follow Adastrea coding standards

---

## Related Documentation

- **Spaceship System**: `Assets/SpaceshipDataAssetGuide.md`
- **Designer Workflow**: `Assets/SpaceshipDesignerWorkflow.md`
- **Templates**: `Assets/SpaceshipTemplates.md`
- **Ship Control**: `Assets/ShipControlGuide.md`

---

## Support

For questions or issues:
1. Check documentation in `Assets/` folder
2. Review existing spaceship implementations
3. See `CONTRIBUTING.md` for contribution guidelines
4. Open an issue on GitHub for bugs

---

**Last Updated**: 2025-11-05  
**Version**: 1.0  
**Author**: Adastrea Development Team
