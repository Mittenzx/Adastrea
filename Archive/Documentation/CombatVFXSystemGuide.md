# Combat Visual Effects System Guide

Complete guide for implementing and customizing combat visual effects in Adastrea.

---

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Data Assets](#data-assets)
4. [Components](#components)
5. [Weapon Fire Effects](#weapon-fire-effects)
6. [Impact Effects](#impact-effects)
7. [Shield Visual Feedback](#shield-visual-feedback)
8. [Weapon Heat Management](#weapon-heat-management)
9. [Blueprint Integration](#blueprint-integration)
10. [Performance Optimization](#performance-optimization)
11. [Creating Effects in Niagara](#creating-effects-in-niagara)
12. [Testing & Debugging](#testing--debugging)

---

## Overview

The Combat VFX System provides comprehensive visual effects for space combat, featuring:

- **6 Weapon Fire Types**: Energy, Projectile, Missile, Torpedo, Beam, Plasma
- **Impact Effects**: Shield ripples, hull sparks, explosions (3 sizes)
- **Shield Feedback**: Color-coded states (full/recharging/down)
- **Heat Management**: Gradient visualization and cooling effects
- **Performance Budget**: <5ms per effect with distance culling

### Key Features

✅ **Data-Driven Design**: All effects configured via Data Assets  
✅ **Blueprint-Friendly**: Full Blueprint exposure for customization  
✅ **Modular Architecture**: Independent components for flexibility  
✅ **Performance-Conscious**: Particle budgets and LOD support  
✅ **Integration-Ready**: Works seamlessly with existing combat systems  

---

## System Architecture

### Class Hierarchy

```
UPrimaryDataAsset
├── UCombatVFXDataAsset        (Weapon fire effects)
└── UImpactEffectDataAsset     (Impact & explosion effects)

UActorComponent
├── UCombatVFXComponent        (Main VFX manager)
└── UShieldVFXComponent        (Shield visual feedback)
```

### Integration with Combat System

```
Spaceship Actor
├── WeaponComponent(s) ──────┐
├── CombatHealthComponent ───┼── Combat VFX System
├── CombatVFXComponent ──────┤
└── ShieldVFXComponent ──────┘
```

---

## Data Assets

### CombatVFXDataAsset

Defines weapon fire effects for a specific weapon type.

**Location**: `Content/DataAssets/Combat/VFX/`

**Creating a Weapon VFX Asset:**

1. Right-click in Content Browser → Blueprint → CombatVFXDataAsset
2. Name it: `DA_VFX_[WeaponType]` (e.g., `DA_VFX_EnergyLaser`)
3. Configure properties:

```yaml
Basic Info:
  VFX Name: "Energy Laser Effects"
  Description: "Blue laser beam with glow and heat distortion"
  Weapon Type: Energy

Fire Effect:
  Muzzle Flash Effect: [Select Niagara System]
  Beam Effect: [Select Niagara System]
  Beam Type: Instant
  Beam Width: 5.0
  Beam Color: (0, 0.5, 1, 1)  # Blue
  Glow Intensity: 2.0
  Heat Distortion Strength: 0.3
  Fire Sound: [Select Sound]

Heat Visuals:
  Heat Vent Effect: [Select Niagara System]
  Heat Gradient Material: [Select Material]
  Cool Color: (0.1, 0.3, 0.5, 1)    # Blue
  Warning Color: (1, 0.5, 0, 1)     # Orange
  Overheat Color: (1, 0, 0, 1)      # Red

Performance:
  Max Particles: 500
  Effect Priority: 5
  Max Render Distance: 10000
```

### ImpactEffectDataAsset

Defines impact and explosion effects.

**Location**: `Content/DataAssets/Combat/VFX/`

**Creating an Impact Effect Asset:**

1. Right-click in Content Browser → Blueprint → ImpactEffectDataAsset
2. Name it: `DA_Impact_Default`
3. Configure properties:

```yaml
Shield Impact:
  Ripple Effect: [Hexagonal ripple Niagara system]
  Dissipation Effect: [Energy particles Niagara system]
  Ripple Speed: 1000.0
  Ripple Duration: 1.0
  Ripple Color: (0, 0.5, 1, 1)
  Max Ripple Radius: 200.0

Hull Impact:
  Spark Effect: [Spark Niagara system]
  Debris Effect: [Debris Niagara system]
  Damage Decal: [Damage decal material]
  Spark Intensity: 1.0
  Debris Intensity: 1.0
  Decal Size: 50.0
  Decal Lifetime: 60.0

Explosions:
  Small Explosion:
    Explosion Radius: 200.0
    Flash Intensity: 2.0
  Medium Explosion:
    Explosion Radius: 500.0
    Flash Intensity: 5.0
  Large Explosion:
    Explosion Radius: 1000.0
    Flash Intensity: 10.0
```

---

## Components

### CombatVFXComponent

Main component for managing all combat visual effects.

**Adding to Ship:**

1. Open ship Blueprint
2. Add Component → CombatVFXComponent
3. Configure properties:

```yaml
Combat VFX:
  Combat VFX Data: [Select CombatVFXDataAsset]
  Impact Effect Data: [Select ImpactEffectDataAsset]
  Weapon Components: [Auto-detected or manually assign]

Performance:
  Max Active Effects: 50
  Enable Distance Culling: true
  Max Effect Distance: 10000.0
```

**Blueprint Functions:**

```cpp
// Play weapon fire effect
PlayWeaponFireEffect(WeaponType, MuzzleLocation, MuzzleRotation, TargetLocation)

// Play impact effects
PlayShieldImpact(ImpactLocation, ImpactNormal, DamageAmount)
PlayHullImpact(ImpactLocation, ImpactNormal, SurfaceComponent)

// Play explosion
PlayExplosion(ExplosionLocation, ExplosionSize, ExplosionRadius)

// Update weapon heat
UpdateWeaponHeatVisuals(WeaponComponent, HeatPercentage)
```

### ShieldVFXComponent

Component for shield visual feedback.

**Adding to Ship:**

1. Open ship Blueprint
2. Add Component → ShieldVFXComponent
3. Configure properties:

```yaml
Shield Visuals:
  Shield Base Material: [Select shield material]
  Shield Effect Template: [Select Niagara system]
  Combat Health Component: [Auto-detected]

Shield Colors:
  Full Shield Color: (0, 1, 0, 0.8)      # Green
  Recharging Color: (0, 0.5, 1, 0.8)    # Blue
  Shield Down Color: (1, 0, 0, 0.8)     # Red
  Recharge Pulse Speed: 2.0
```

**Blueprint Functions:**

```cpp
// Set shield state manually
SetFacingState(Facing, NewState)

// Get current state
GetFacingState(Facing)

// Play shield impact
PlayShieldImpact(ImpactLocation, ImpactNormal, Facing)
```

---

## Weapon Fire Effects

### Energy Weapons

**Visual Design**: Laser beam with glow and heat distortion

**Niagara Parameters:**
- `BeamStart` (Vector): Start location
- `BeamEnd` (Vector): End location
- `BeamWidth` (Float): Width in cm (default: 5.0)
- `BeamColor` (Color): Beam color
- `GlowIntensity` (Float): Glow multiplier (default: 2.0)
- `HeatDistortion` (Float): Distortion strength (0-1)

**Implementation:**
```cpp
// In CombatVFXComponent
PlayBeamEffect(MuzzleLocation, TargetLocation, 5.0f, BlueColor, 0.1f);
```

### Projectile Weapons

**Visual Design**: Tracer rounds with muzzle flash

**Niagara Parameters:**
- `Velocity` (Vector): Projectile velocity
- `TrailColor` (Color): Trail color
- `TrailWidth` (Float): Trail width (default: 5.0)

**Implementation:**
```cpp
// Calculate velocity
FVector Velocity = MuzzleDirection * 5000.0f;

// Spawn trail
PlayProjectileTrail(MuzzleLocation, MuzzleRotation, Velocity, EWeaponType::Projectile);
```

### Missile Weapons

**Visual Design**: Smoke trail and engine glow

**Niagara Parameters:**
- `Velocity` (Vector): Missile velocity
- `TrailWidth` (Float): Smoke trail width (default: 20.0)
- `EngineGlow` (Float): Engine intensity (default: 2.0)

**Features:**
- Persistent smoke trail
- Glowing engine exhaust
- Homing behavior visualization

### Torpedo Weapons

**Visual Design**: Plasma trail effect

**Niagara Parameters:**
- `Velocity` (Vector): Torpedo velocity
- `TrailWidth` (Float): Plasma trail width (default: 30.0)
- `PlasmaIntensity` (Float): Plasma glow (default: 3.0)

**Features:**
- Thick plasma trail
- High intensity glow
- Slower projectile speed

### Beam Weapons

**Visual Design**: Sustained beam with start/end points

**Niagara Parameters:**
- `BeamStart` (Vector): Start location
- `BeamEnd` (Vector): End location
- `BeamWidth` (Float): Width in cm (default: 10.0)
- `Duration` (Float): Beam duration in seconds

**Features:**
- Sustained effect (0.5-2.0 seconds)
- Visible start and end points
- Continuous damage visualization

### Plasma Weapons

**Visual Design**: Ball lightning with arcing

**Niagara Parameters:**
- `Velocity` (Vector): Plasma bolt velocity
- `TrailWidth` (Float): Effect width (default: 15.0)
- `ArcingIntensity` (Float): Lightning arc strength (default: 1.5)

**Features:**
- Electrical arcing effects
- Ball lightning appearance
- Medium projectile speed

---

## Impact Effects

### Shield Impacts

**Visual Design**: Hexagonal ripple with energy dissipation

**Effect Stages:**
1. **Initial Impact**: Bright flash at impact point
2. **Ripple Expansion**: Hexagonal pattern expands outward
3. **Energy Dissipation**: Particles scatter from impact
4. **Fade Out**: Effect dissipates over time

**Niagara Parameters:**
- `ImpactLocation` (Vector): World location
- `ImpactNormal` (Vector): Surface normal
- `RippleColor` (Color): Color based on damage
- `RippleSpeed` (Float): Expansion speed (default: 1000 cm/s)
- `MaxRadius` (Float): Maximum ripple size (default: 200 cm)

**Color Coding:**
- **Low Damage** (<33%): Blue
- **Medium Damage** (33-66%): Cyan
- **High Damage** (>66%): White/Yellow

### Hull Impacts

**Visual Design**: Sparks, debris, damage decals

**Effect Components:**
1. **Sparks**: Bright metal sparks from impact
2. **Debris**: Small hull fragments
3. **Damage Decal**: Permanent or temporary damage mark
4. **Impact Flash**: Brief bright flash

**Niagara Parameters:**
- `Intensity` (Float): Spark/debris intensity
- Decal applied via UDecalComponent

**Decal Configuration:**
- Size: 50 cm default
- Lifetime: 60 seconds (0 = permanent)
- Fade: Gradual fade at end of lifetime

### Explosion Effects

**Three Size Categories:**

#### Small Explosion
- **Radius**: 200 cm
- **Use Case**: Small projectiles, fighters
- **Flash Intensity**: 2.0
- **Camera Shake**: Minor

#### Medium Explosion
- **Radius**: 500 cm
- **Use Case**: Standard missiles, torpedoes
- **Flash Intensity**: 5.0
- **Camera Shake**: Moderate

#### Large Explosion
- **Radius**: 1000 cm
- **Use Case**: Capital ships, stations
- **Flash Intensity**: 10.0
- **Camera Shake**: Heavy
- **Additional**: Fireball effect

**Effect Components:**
1. **Main Explosion**: Primary blast effect
2. **Shockwave**: Expanding ring effect
3. **Fireball** (Large only): Sustained fire ball
4. **Camera Shake**: Distance-based intensity
5. **Sound**: Explosion audio

---

## Shield Visual Feedback

### Shield States

#### Full Shield (Green)
- **Color**: RGB(0, 255, 0) with 80% opacity
- **Behavior**: Steady glow
- **Trigger**: Shield strength at 100%

#### Recharging (Blue Pulse)
- **Color**: RGB(0, 128, 255) with 80% opacity
- **Behavior**: Pulsing animation (2 Hz default)
- **Intensity Range**: 30% to 150%
- **Trigger**: Shield below 100% and recharging

#### Shield Down (Red Warning)
- **Color**: RGB(255, 0, 0) with 80% opacity
- **Behavior**: Steady glow
- **Trigger**: Shield strength at 0%

#### Disabled
- **Behavior**: No visual effect
- **Trigger**: Shield system offline

### Directional Shield Support

For ships with 4-facing shields:
- **Forward**: Shows independent state
- **Aft**: Shows independent state
- **Port**: Shows independent state
- **Starboard**: Shows independent state

Each facing updates independently based on its own shield strength and recharge state.

---

## Weapon Heat Management

### Heat Gradient Visualization

**Color Progression:**
1. **Cool** (0-50% heat): Blue → Orange
2. **Warning** (50-100% heat): Orange → Red

**Implementation:**
```cpp
FLinearColor HeatColor = CombatVFXData->GetHeatColor(HeatPercentage);
```

**Material Parameters:**
- `HeatColor` (Vector): Current heat color
- `HeatPercentage` (Scalar): Current heat level (0-1)
- `ShimmerIntensity` (Scalar): Heat shimmer effect

### Cooling Vent Effects

**Trigger**: Weapon heat > 50% and cooling

**Visual Design**: Steam/heat shimmer from vents

**Niagara Parameters:**
- `Intensity` (Float): Effect intensity (based on heat)
- Location: Weapon cooling vent position

### Overheat Warning

**Trigger**: Weapon reaches overheat threshold (typically 100%)

**Visual Design**: Orange→Red gradient with pulsing

**Material Parameters:**
- `OverheatWarning` (Scalar): 0 or 1 (off/on)
- Overrides normal heat gradient with urgent warning

---

## Blueprint Integration

### Setting Up Combat VFX on a Ship

**Step 1: Add Components**
```
1. Open ship Blueprint (BP_Ship_YourShip)
2. Add Component → CombatVFXComponent
3. Add Component → ShieldVFXComponent (if ship has shields)
4. Compile and Save
```

**Step 2: Configure Data Assets**
```
1. In CombatVFXComponent details:
   - Assign Combat VFX Data (DA_VFX_[WeaponType])
   - Assign Impact Effect Data (DA_Impact_Default)
   
2. In ShieldVFXComponent details:
   - Assign Shield Base Material
   - Assign Shield Effect Template
   - Configure shield colors
```

**Step 3: Hook Up Events**
```
1. In Event Graph, get WeaponComponent
2. On weapon fired:
   - Get CombatVFXComponent
   - Call PlayWeaponFireEffect
   
3. On damage received:
   - Check if shields absorbed damage
   - Call PlayShieldImpact OR PlayHullImpact
```

### Blueprint Event Examples

**Example 1: Weapon Fired**
```
Event WeaponFired (from WeaponComponent)
├── Get Combat VFX Component
├── Get Weapon World Position → Muzzle Location
├── Get Aim Direction → Muzzle Rotation
├── Get Target → Target Location
└── Play Weapon Fire Effect
    ├── Weapon Type
    ├── Muzzle Location
    ├── Muzzle Rotation
    └── Target Location
```

**Example 2: Shield Impact**
```
Event On Damage Received (from CombatHealthComponent)
├── Branch: Has Shields?
│   ├── True:
│   │   ├── Get Combat VFX Component
│   │   ├── Get Impact Location
│   │   ├── Get Impact Normal
│   │   ├── Get Damage Amount
│   │   └── Play Shield Impact
│   └── False:
│       ├── Get Combat VFX Component
│       ├── Get Impact Location
│       ├── Get Impact Normal
│       ├── Get Hit Component
│       └── Play Hull Impact
```

**Example 3: Shield State Updates**
```
Event Tick
├── Get Shield VFX Component
├── Update Shield Visuals (automatic)
└── On Shield State Changed
    ├── Check New State
    ├── Play Sound Effect
    └── Update UI
```

---

## Performance Optimization

### Particle Budget Management

**Target Performance**: <5ms per effect

**Budget Allocation:**
- Muzzle Flash: 50-100 particles
- Beam Effects: 200-500 particles
- Projectile Trails: 100-300 particles
- Shield Impacts: 300-500 particles
- Hull Impacts: 200-400 particles
- Explosions (Small): 500-1000 particles
- Explosions (Medium): 1000-2000 particles
- Explosions (Large): 2000-5000 particles

**Configuration:**
```yaml
CombatVFXComponent:
  Max Active Effects: 50
  Enable Distance Culling: true
  Max Effect Distance: 10000.0  # meters
```

### Distance-Based Culling

**Implementation:**
```cpp
bool IsWithinRenderDistance(FVector Location)
{
    float Distance = FVector::Dist(PlayerLocation, Location);
    return Distance <= MaxEffectDistance;
}
```

**LOD Levels:**
- **Close** (<2000m): Full quality, all effects
- **Medium** (2000-5000m): Reduced particle count
- **Far** (5000-10000m): Essential effects only
- **Very Far** (>10000m): Effects culled

### Effect Priority System

**Priority Levels (0-10):**
- **10**: Critical (explosions, ship destruction)
- **8-9**: High (weapon fire, impacts)
- **5-7**: Normal (trails, heat effects)
- **2-4**: Low (ambient effects)
- **0-1**: Very Low (decorative effects)

**Usage:**
```cpp
// High priority effects always spawn
if (EffectPriority >= 8 || CanSpawnEffect(EffectPriority))
{
    SpawnEffect();
}
```

### Effect Pooling

For frequently used effects:
1. Pre-spawn effect instances
2. Activate/deactivate as needed
3. Reset parameters on reuse
4. Reduces spawn/destroy overhead

### Performance Metrics

**Getting Performance Data:**
```cpp
int32 ActiveEffects;
int32 TotalParticles;
float FrameTime;

CombatVFXComponent->GetPerformanceMetrics(
    ActiveEffects, 
    TotalParticles, 
    FrameTime
);
```

**Optimization Tips:**
- Use GPU particles for high-count effects (beams, trails)
- Use CPU particles for complex behavior (missiles, debris)
- Enable LOD in Niagara systems
- Set appropriate bounds for culling
- Use fixed bounds when possible
- Reduce update rate for distant effects

---

## Creating Effects in Niagara

### Energy Weapon Laser Beam

**Step 1: Create Niagara System**
1. Content Browser → Right-click → Niagara System
2. Base System: "Beam Emitter"
3. Name: `NS_EnergyLaser_Beam`

**Step 2: Configure Emitter**
```
Emitter Settings:
  - Spawn Rate: 1000
  - Lifetime: 0.1 seconds
  - Beam Start: User Parameter (Vector)
  - Beam End: User Parameter (Vector)
  
Beam Properties:
  - Width: User Parameter (Float, default 5.0)
  - Color: User Parameter (Color)
  - Glow Intensity: User Parameter (Float, default 2.0)
  
Material:
  - Additive blend mode
  - Emissive material
  - Heat distortion via refraction
```

**Step 3: Add User Parameters**
- `BeamStart` (Vector)
- `BeamEnd` (Vector)
- `BeamWidth` (Float, default 5.0)
- `BeamColor` (LinearColor, default White)
- `GlowIntensity` (Float, default 2.0)
- `HeatDistortion` (Float, default 0.3)

### Projectile Tracer Trail

**Step 1: Create Niagara System**
1. Base System: "Ribbon Emitter"
2. Name: `NS_Projectile_Tracer`

**Step 2: Configure Emitter**
```
Emitter Settings:
  - Spawn Rate: 500
  - Lifetime: 0.5 seconds
  - Velocity: User Parameter (Vector)
  
Ribbon Properties:
  - Width: User Parameter (Float, default 5.0)
  - Color: User Parameter (Color)
  - Fade In/Out: 0.1 / 0.2 seconds
  
Physics:
  - Drag: 0.1 (slight deceleration)
```

### Shield Impact Ripple

**Step 1: Create Niagara System**
1. Base System: "Mesh Renderer"
2. Name: `NS_Shield_ImpactRipple`

**Step 2: Configure Emitter**
```
Emitter Settings:
  - Spawn Burst: 1 particle
  - Lifetime: 1.0 seconds
  - Mesh: Hexagon mesh (low poly)
  
Scale:
  - Initial: 10 cm
  - Scale by Curve: 0 → Max Radius over lifetime
  
Material:
  - Translucent with ripple pattern
  - Color: User Parameter (Color)
  - Opacity: Fade from 1 → 0 over lifetime
```

**Step 3: Add Energy Dissipation**
```
Secondary Emitter:
  - Spawn Rate: 100
  - Lifetime: 0.5 seconds
  - Initial Velocity: Radial outward
  - Drag: High (quick stop)
  - Color: Match ripple color
```

### Explosion Effect (Medium)

**Step 1: Create Niagara System**
1. Base System: "Explosion Emitter"
2. Name: `NS_Explosion_Medium`

**Step 2: Main Explosion**
```
Emitter Settings:
  - Spawn Burst: 1000 particles
  - Lifetime: 2.0 seconds
  - Initial Velocity: Radial (high speed)
  
Properties:
  - Size: 20-100 cm over lifetime
  - Color: White → Orange → Red → Black
  - Opacity: Fade out
  - Drag: Medium (realistic deceleration)
```

**Step 3: Shockwave**
```
Shockwave Emitter:
  - Spawn Burst: 1 particle
  - Mesh: Ring mesh
  - Scale: 0 → Explosion Radius over 0.5 seconds
  - Opacity: 1 → 0
  - Material: Additive with distortion
```

**Step 4: Flash**
```
Flash Emitter:
  - Spawn Burst: 1 particle
  - Sprite: Bright white
  - Scale: Large → 0 quickly
  - Lifetime: 0.1 seconds
```

---

## Testing & Debugging

### Test Map Setup

1. Create `TestMap_CombatVFX`
2. Add test spaceship with CombatVFXComponent
3. Add target dummy ships
4. Add lighting for visibility
5. Add post-process volume for effects enhancement

### Testing Checklist

**Weapon Fire Effects:**
- [ ] Energy weapons: Beam visible, color correct
- [ ] Projectile weapons: Tracer visible, muzzle flash present
- [ ] Missile weapons: Smoke trail and engine glow visible
- [ ] Torpedo weapons: Plasma trail visible
- [ ] Beam weapons: Sustained beam duration correct
- [ ] Plasma weapons: Arcing visible

**Impact Effects:**
- [ ] Shield impact: Ripple expands correctly
- [ ] Shield impact: Color matches damage intensity
- [ ] Hull impact: Sparks and debris present
- [ ] Hull impact: Decal applied correctly
- [ ] Small explosion: Correct radius
- [ ] Medium explosion: Shockwave visible
- [ ] Large explosion: Fireball present

**Shield Feedback:**
- [ ] Full shield: Green glow steady
- [ ] Recharging: Blue pulse animates
- [ ] Shield down: Red glow visible
- [ ] Per-facing states update independently

**Heat Management:**
- [ ] Heat gradient: Color changes with heat level
- [ ] Cooling vents: Steam effect plays when cooling
- [ ] Overheat warning: Red warning displays at threshold

### Performance Testing

**FPS Counter:**
```
Command: stat fps
Target: 60 FPS minimum
```

**Particle Stats:**
```
Command: stat particles
Check:
  - Total particle count
  - Active emitters
  - Render time
```

**Effect Profiling:**
```
Command: stat unit
Check:
  - Game thread time
  - Render thread time
  - GPU time
```

### Debugging Commands

```
// Show all Niagara systems
stat niagara

// Show particle counts
stat particles

// Show bounds for culling debug
show bounds

// Toggle effect visibility
show particles

// Profile specific system
profilegpu
```

### Common Issues & Solutions

**Issue: Effects not appearing**
- Check Niagara system assigned in Data Asset
- Verify component is active
- Check distance culling settings
- Verify effect priority and budget

**Issue: Poor performance**
- Reduce Max Active Effects
- Enable distance culling
- Lower particle counts in Niagara
- Use LOD in Niagara systems
- Check effect bounds

**Issue: Shield colors wrong**
- Verify color settings in ShieldVFXComponent
- Check CombatHealthComponent integration
- Verify shield state detection logic

**Issue: Heat effects not updating**
- Check weapon heat component reference
- Verify UpdateWeaponHeatVisuals called each frame
- Check heat gradient material assigned

---

## Best Practices

### Design Guidelines

1. **Visual Clarity**: Effects should be immediately recognizable
2. **Performance First**: Always profile effects
3. **Color Coding**: Use consistent colors for damage types
4. **Scale Appropriately**: Match effect size to weapon/ship scale
5. **Sound Integration**: Always pair visuals with audio

### Naming Conventions

```
Data Assets:
  DA_VFX_[WeaponType]       (e.g., DA_VFX_EnergyLaser)
  DA_Impact_[Variant]        (e.g., DA_Impact_Default)

Niagara Systems:
  NS_[Category]_[Effect]     (e.g., NS_Energy_LaserBeam)
  
Materials:
  M_[Type]_[Purpose]         (e.g., M_Shield_Ripple)
  MI_[Type]_[Variant]        (e.g., MI_Shield_Blue)
```

### Version Control

- Separate asset files from code
- Use asset dependencies carefully
- Test after pulling changes
- Document breaking changes

---

## Examples

### Complete Ship Setup

```yaml
BP_Ship_Fighter:
  Components:
    - CombatVFXComponent:
        Combat VFX Data: DA_VFX_EnergyLaser
        Impact Effect Data: DA_Impact_Default
        Max Active Effects: 30
    
    - ShieldVFXComponent:
        Shield Base Material: M_Shield_Base
        Shield Effect Template: NS_Shield_Glow
        Full Shield Color: (0, 1, 0, 0.8)
        Recharging Color: (0, 0.5, 1, 0.8)
        Shield Down Color: (1, 0, 0, 0.8)
    
    - WeaponComponent:
        Weapon Data: DA_Weapon_PulseLaser
        Mount Location: (100, 0, 0)
```

### Custom Blueprint Effect

```
Custom Weapon Fire Effect:
  1. Get CombatVFXComponent
  2. Get weapon properties
  3. Call OnWeaponFired event
  4. In event, spawn custom effects
  5. Call parent implementation
```

---

## Additional Resources

- **Niagara Documentation**: Unreal Engine 5 Niagara documentation
- **Combat System Guide**: `Assets/CombatSystemGuide.md`
- **Workflow Guide**: `Assets/CombatVFXWorkflow.md`
- **YAML Templates**: `Assets/CombatVFXTemplates/`
- **Example Project**: Content/Examples/CombatVFX/

---

**Document Version**: 1.0  
**Last Updated**: November 18, 2025  
**Author**: Adastrea Development Team

For questions or issues, refer to the main Combat System Guide or create an issue in the project repository.
