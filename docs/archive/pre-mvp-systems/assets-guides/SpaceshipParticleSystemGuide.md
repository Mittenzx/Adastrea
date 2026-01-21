# Spaceship Particle System Guide

Complete guide for implementing and customizing particle effects for spaceship propulsion in Adastrea.

---

## Table of Contents

1. [Overview](#overview)
2. [Component Architecture](#component-architecture)
3. [Main Engine Effects](#main-engine-effects)
4. [RCS Thruster Effects](#rcs-thruster-effects)
5. [Jump Drive Effects](#jump-drive-effects)
6. [Engine Damage States](#engine-damage-states)
7. [Blueprint Integration](#blueprint-integration)
8. [Performance Optimization](#performance-optimization)
9. [Testing & Validation](#testing--validation)

---

## Overview

The `USpaceshipParticleComponent` provides a complete particle effect system for spaceships, featuring:

- **Main engine effects** with throttle-responsive intensity
- **RCS thrusters** for 6-axis movement feedback (forward, backward, left, right, up, down)
- **Jump drive effects** with multi-stage sequence (charge → activation → tunnel → exit)
- **Engine damage states** (normal, damaged, critical)
- **Performance optimization** with particle count budgeting (<10k particles per ship)

### Key Features

✅ **Throttle-responsive**: Main engine intensity scales with ship speed  
✅ **Blueprint-friendly**: All functions exposed to Blueprints  
✅ **Modular design**: Easy to customize per ship template  
✅ **Performance-conscious**: GPU particles for main engines, CPU for RCS  
✅ **Damage integration**: Visual feedback for engine health  

---

## Component Architecture

### Class Hierarchy

```
UActorComponent
└── USpaceshipParticleComponent
```

### Integration

The component is automatically created in `ASpaceship::ASpaceship()` constructor:

```cpp
// In Spaceship.cpp constructor
ParticleComponent = CreateDefaultSubobject<USpaceshipParticleComponent>(TEXT("ParticleComponent"));
```

Access in Blueprints:
```
Get Spaceship → Get Particle Component → [Call Functions]
```

---

## Main Engine Effects

### Particle Systems

The main engine uses **GPU particles** for performance with high particle counts.

#### Templates Required

1. **Main Engine Template** (`MainEngineTemplate`)
   - GPU particle system
   - Parameters: `SpawnRate` (float)
   - Recommended particles: 500-2000

2. **Afterburner Template** (`AfterburnerTemplate`)
   - GPU particle system
   - Activated during boost mode
   - Recommended particles: 300-1000

3. **Engine Glow Material** (`EngineMaterialInstance`)
   - Material Instance Dynamic
   - Parameters: `EmissiveIntensity` (scalar)
   - Controls engine emissive glow

### Throttle Response

The component automatically updates particle intensity based on ship throttle:

```cpp
// Called from Spaceship movement input
ParticleComponent->UpdateThrottle(ThrottleValue); // 0.0 to 1.0
```

**Intensity Scaling:**
- `MinParticleRateMultiplier`: 0.2 (idle intensity)
- `MaxParticleRateMultiplier`: 3.0 (full throttle)
- Linear interpolation between min and max

### Boost Mode

Activate afterburner for boost visual:

```cpp
ParticleComponent->SetBoostActive(true);  // Enable boost
ParticleComponent->SetBoostActive(false); // Disable boost
```

**Blueprint Example:**
```
On Boost Input Pressed
└── Get Particle Component
    └── Set Boost Active (true)
```

### Material Glow Control

The engine glow material is automatically updated based on:
- Current throttle (0-1)
- Boost state (1.5x multiplier)
- Damage state (flickering or dimmed)

**Material Setup:**
1. Create material with `EmissiveIntensity` scalar parameter
2. Assign to ship engine mesh
3. Create Material Instance Dynamic in Blueprint
4. Assign to `EngineMaterialInstance` property

---

## RCS Thruster Effects

### Overview

RCS (Reaction Control System) thrusters provide **6-axis movement feedback** using **CPU particles** for precise positioning.

### Thruster Axes

| Axis | Location | Activation |
|------|----------|------------|
| Forward | Front of ship (+X) | Backward movement input |
| Backward | Rear of ship (-X) | Forward movement input |
| Left | Left side (-Y) | Right strafe input |
| Right | Right side (+Y) | Left strafe input |
| Up | Top (+Z) | Down movement input |
| Down | Bottom (-Z) | Up movement input |

### Automatic Activation

RCS thrusters are automatically activated by ship movement inputs:

```cpp
// In Spaceship.cpp - MoveForward
if (Value > 0.0f) {
    ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Backward, Value);
} else {
    ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Forward, FMath::Abs(Value));
}
```

### Manual Control

Blueprint can manually activate RCS thrusters:

```cpp
// Activate specific thruster
ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Forward, 0.8f);
```

**Parameters:**
- `Axis`: Which thruster to activate (enum)
- `Intensity`: Effect strength (0.0 to 1.0)

### Duration

RCS effects auto-deactivate after `RCSThrusterDuration` seconds (default: 0.5s).

### Template Setup

1. Create CPU particle system with short burst
2. Add `Intensity` parameter (float)
3. Assign to `RCSThrusterTemplate` property
4. Recommended particles: 20-100 per thruster

---

## Jump Drive Effects

### Overview

The jump drive system provides a **4-stage visual sequence**:

1. **Charge** - Energy buildup effect (3 seconds default)
2. **Activation** - Bright flash when jump initiates (0.5 seconds)
3. **Tunnel** - Wormhole/hyperspace travel effect (2 seconds)
4. **Exit** - Flash and energy dissipation at destination (0.5 seconds)

### Triggering Jump Sequence

```cpp
// Start jump sequence with default duration
ParticleComponent->TriggerJumpSequence();

// Start jump with custom charge time
ParticleComponent->TriggerJumpSequence(5.0f); // 5 second charge
```

**Blueprint:**
```
On Jump Button Pressed
└── Get Particle Component
    └── Trigger Jump Sequence (Charge Duration = 3.0)
```

### Jump Sequence Stages

#### Stage 0: Charge (3s default)
- `JumpChargeComponent` active
- Energy buildup visual
- Duration: `JumpChargeDuration` property

#### Stage 1: Activation (0.5s)
- `JumpActivationComponent` active
- Bright flash effect
- Fixed 0.5 second duration

#### Stage 2: Tunnel (2s)
- `JumpTunnelComponent` active
- Wormhole/hyperspace visual
- Fixed 2 second duration

#### Stage 3: Exit (0.5s)
- `JumpExitComponent` active
- Exit flash and dissipation
- Fixed 0.5 second duration

### Completion Event

Blueprint-implementable event fires when sequence completes:

```cpp
UFUNCTION(BlueprintImplementableEvent)
void OnJumpSequenceComplete();
```

**Blueprint Implementation:**
```
Event On Jump Sequence Complete
└── [Your jump completion logic]
    └── Teleport Ship
    └── Load New System
    └── etc.
```

### Template Requirements

Four separate particle systems needed:

1. **JumpChargeTemplate**: Gradual energy buildup
2. **JumpActivationTemplate**: Bright flash effect
3. **JumpTunnelTemplate**: Tunnel/wormhole visual
4. **JumpExitTemplate**: Exit flash

Recommended total particles: 1500-3000 (split across stages)

---

## Engine Damage States

### Overview

Three visual states for engine health:

1. **Normal** - Full operational, no damage effects
2. **Damaged** - Sputtering, irregular thrust, flickering glow
3. **Critical** - Heavy smoke, fire, emergency shutdown visuals

### Setting Damage State

```cpp
// Set damage state
ParticleComponent->SetDamageState(EEngineDamageState::Damaged);
ParticleComponent->SetDamageState(EEngineDamageState::Critical);
ParticleComponent->SetDamageState(EEngineDamageState::Normal);
```

**Blueprint:**
```
When Health < 50%
└── Get Particle Component
    └── Set Damage State (Damaged)

When Health < 20%
└── Get Particle Component
    └── Set Damage State (Critical)
```

### Damage Visual Effects

#### Normal State
- No damage particles
- Full engine glow
- Clean thrust plume

#### Damaged State
- Sputtering particles from `DamagedEngineTemplate`
- Flickering engine glow (±30% at 10 Hz)
- Irregular thrust behavior
- Recommended particles: 200-500

#### Critical State
- Heavy smoke/fire from `CriticalDamageTemplate`
- Very dim engine glow (30% normal)
- Severe visual distress
- Recommended particles: 500-1000

### Template Setup

1. **DamagedEngineTemplate**: Light smoke, sparks, sputtering
2. **CriticalDamageTemplate**: Heavy smoke, fire, emergency visuals

---

## Blueprint Integration

### Setting Up Ship Blueprint

1. **Add Particle Templates**

In your ship Blueprint class defaults:

```
Particle Component
├── Main Engine
│   ├── Main Engine Template = [Your GPU Particle]
│   ├── Afterburner Template = [Your Boost Particle]
│   └── Engine Material Instance = [Your Material Instance]
├── RCS Thrusters
│   └── RCS Thruster Template = [Your CPU Particle]
├── Jump Drive
│   ├── Jump Charge Template = [Charge Particle]
│   ├── Jump Activation Template = [Flash Particle]
│   ├── Jump Tunnel Template = [Tunnel Particle]
│   └── Jump Exit Template = [Exit Particle]
└── Damage States
    ├── Damaged Engine Template = [Damaged Particle]
    └── Critical Damage Template = [Critical Particle]
```

2. **Wire Up Movement**

Movement input automatically triggers RCS thrusters (already implemented in C++).

3. **Add Jump Control**

```
Event Graph:
  On Jump Pressed
  └── Get Particle Component
      └── Trigger Jump Sequence
      
  On Jump Sequence Complete
  └── [Your jump logic]
      └── Teleport Ship
      └── Play Sound
      └── Camera Shake
```

4. **Damage Integration**

```
Event Graph:
  On Health Changed
  └── Branch (Health < 50%)
      ├── True: Set Damage State (Damaged)
      └── False: Set Damage State (Normal)
  
  Branch (Health < 20%)
  └── True: Set Damage State (Critical)
```

### Common Blueprint Functions

```cpp
// Get current throttle
float Throttle = ParticleComponent->CurrentThrottle;

// Check if boost active
bool IsBoosting = ParticleComponent->bBoostActive;

// Check if jump in progress
bool IsJumping = ParticleComponent->bJumpSequenceActive;

// Get particle count (performance monitoring)
int32 Count = ParticleComponent->GetCurrentParticleCount();
```

---

## Performance Optimization

### Particle Budget

**Target: <10,000 particles per ship**

Recommended distribution:
- Main engine: 1500 particles
- Afterburner: 800 particles
- RCS thrusters (6): 600 particles (100 each)
- Jump effects: 2000 particles (500 per stage)
- Damage effects: 1000 particles
- **Total: ~5,900 particles** (good margin)

### GPU vs CPU Particles

**Use GPU particles for:**
- Main engine (high particle count)
- Afterburner (high particle count)
- Jump tunnel (high particle count)

**Use CPU particles for:**
- RCS thrusters (precise positioning)
- Damage effects (collision/physics needed)
- Jump flash effects (one-shot)

### Optimization Strategies

1. **LOD System**
   - Reduce particle count at distance
   - Disable RCS effects beyond 5000 units
   - Simplify effects for distant ships

2. **Particle Pooling**
   - Reuse particle components
   - Pre-spawn at initialization
   - Don't create/destroy at runtime

3. **Update Frequency**
   - Main engine: Every tick
   - RCS: On-demand only
   - Damage effects: 0.1s update rate

4. **Quality Settings**
   - Low: Main engine only, no RCS
   - Medium: Main + RCS, no jump
   - High: All effects enabled
   - Epic: Enhanced particle density

### Monitoring Performance

```cpp
// Blueprint: Display particle count
int32 Count = GetParticleComponent()->GetCurrentParticleCount();
Print String: "Particles: " + Count
```

**Performance Targets:**
- Single ship: <10,000 particles
- 10 ships: <100,000 total particles
- Frame time budget: <2ms for all particle updates

---

## Testing & Validation

### Test Checklist

- [ ] **Main Engine**
  - [ ] Engine activates on BeginPlay
  - [ ] Throttle changes update intensity smoothly
  - [ ] Boost activates afterburner
  - [ ] Engine glow scales with throttle
  
- [ ] **RCS Thrusters**
  - [ ] Forward movement activates backward thruster
  - [ ] Backward movement activates forward thruster
  - [ ] Left strafe activates right thruster
  - [ ] Right strafe activates left thruster
  - [ ] Up movement activates down thruster
  - [ ] Down movement activates up thruster
  - [ ] RCS auto-deactivates after duration
  
- [ ] **Jump Drive**
  - [ ] Charge stage displays correctly
  - [ ] Activation flash triggers
  - [ ] Tunnel effect plays during jump
  - [ ] Exit flash completes sequence
  - [ ] OnJumpSequenceComplete event fires
  
- [ ] **Damage States**
  - [ ] Normal state: no damage effects
  - [ ] Damaged state: sputtering particles
  - [ ] Critical state: heavy smoke/fire
  - [ ] Engine glow dims with damage
  
- [ ] **Performance**
  - [ ] Particle count <10k per ship
  - [ ] No frame drops with 10 ships
  - [ ] Memory usage stable over time
  - [ ] No particle leaks (components cleaned up)

### Test Maps

Create test maps with:
1. Single ship (detailed testing)
2. 10 ships (performance testing)
3. 50+ ships (stress testing)

### Debugging Commands

```cpp
// Console commands for testing
stat particles        // Show particle stats
stat fps             // Frame rate
stat unit            // CPU/GPU timing
profileGPU           // GPU profiling
```

### Validation Script

```python
# Test with 10 ships
for i in range(10):
    spawn_ship_at_location(i * 1000, 0, 0)
    
# Monitor for 60 seconds
wait(60)

# Check metrics
assert get_average_fps() > 60
assert get_particle_count() < 100000
assert no_memory_leaks()
```

---

## Ship Template Integration

### All 6 Ship Templates Supported

1. **Mittenzx Mk1** - Balanced particle setup
2. **Pathfinder Scout** - Enhanced sensor glow, minimal RCS
3. **Warhammer Gunship** - Aggressive engine effects, heavy smoke when damaged
4. **Merchant Trader** - Larger, slower engine response
5. **Starliner Luxury** - Elegant, refined particle effects
6. **Excavator Mining** - Industrial, heavy-duty engine visuals
7. **Sovereign Command** - Multiple engine clusters, grand scale effects

### Template-Specific Customization

Each ship template can customize:
- Engine location and count
- RCS thruster positions
- Particle scale/intensity
- Damage effect intensity
- Jump effect appearance

**Example: Warhammer Gunship**
```
Main Engine:
  - Higher intensity multiplier (4.0)
  - More aggressive afterburner
  - Enhanced boost visuals

RCS:
  - Military-style thruster bursts
  - Sharper activation/deactivation

Damage:
  - More dramatic smoke effects
  - Battle damage appearance
```

---

## Advanced Topics

### Custom Jump Sequences

Create custom jump visuals by:
1. Overriding `OnJumpSequenceComplete` in Blueprint
2. Customizing stage durations
3. Adding camera effects
4. Playing audio cues

### Multi-Engine Ships

For ships with multiple engines:
1. Create multiple `SpaceshipParticleComponent` instances
2. Position each at engine hardpoint
3. Synchronize throttle updates
4. Share particle budget across engines

### Dynamic Particle LOD

Implement distance-based LOD:
```cpp
float Distance = GetDistanceToPlayer();
if (Distance > 5000.0f) {
    ParticleComponent->RCSThrusterDuration = 0.0f; // Disable RCS
    ParticleComponent->MinParticleRateMultiplier = 0.1f; // Reduce main engine
}
```

### Material Effects Integration

Combine particles with material effects:
- Engine heat distortion
- Warp field shimmering
- Exhaust glow on ship hull
- Energy field interactions

---

## Troubleshooting

### Common Issues

**Problem**: Particles not visible
- Check particle templates are assigned
- Verify particles aren't culled by bounds
- Check particle scale/lifetime

**Problem**: Performance issues
- Use `stat particles` to check count
- Verify GPU particles for main engines
- Check particle LOD settings
- Reduce spawn rate multipliers

**Problem**: RCS not activating
- Verify movement input is being received
- Check thruster templates assigned
- Ensure duration > 0
- Verify component initialized

**Problem**: Jump sequence doesn't complete
- Check all 4 jump templates assigned
- Verify `bJumpSequenceActive` flag
- Debug with print statements in UpdateJumpSequence()
- Ensure OnJumpSequenceComplete is implemented

**Problem**: Damage effects not showing
- Verify damage templates assigned
- Check SetDamageState is being called
- Ensure health integration is correct
- Verify particle spawn parameters

---

## Best Practices

1. **Always assign all particle templates** - Missing templates cause null reference issues
2. **Test with multiple ships** - Performance issues only appear at scale
3. **Use particle budgets** - Stay under 10k per ship
4. **Implement LOD** - Disable effects at distance
5. **Pool particles** - Create once, reuse
6. **Monitor performance** - Use `stat particles` regularly
7. **Document customizations** - Note template-specific changes
8. **Blueprint-friendly** - Expose all controls to designers
9. **Validate damage integration** - Test all damage states
10. **Test jump sequence** - Verify all 4 stages work

---

## Next Steps

1. **Create Particle Templates**: Design GPU/CPU particle systems in Unreal Editor
2. **Set Up Materials**: Create engine glow materials with parameters
3. **Configure Ship Blueprints**: Assign templates to all 6 ship types
4. **Integrate Health System**: Connect damage states to ship health
5. **Add Jump Controls**: Implement jump input and teleportation
6. **Performance Test**: Verify <10k particles per ship
7. **Polish**: Fine-tune timings, intensities, and visuals

---

## Additional Resources

- [Unreal Engine Cascade Documentation](https://docs.unrealengine.com/en-US/RenderingAndGraphics/ParticleSystems/Cascade/)
- [GPU vs CPU Particles Guide](https://docs.unrealengine.com/en-US/RenderingAndGraphics/ParticleSystems/Optimization/)
- [Material Instance Dynamic](https://docs.unrealengine.com/en-US/RenderingAndGraphics/Materials/MaterialInstances/)
- [Adastrea Spaceship System Guide](SpaceshipDataAssetGuide.md)
- [Adastrea Combat System Guide](CombatSystemGuide.md)

---

**Last Updated**: 2025-11-17  
**Version**: 1.0  
**Author**: Adastrea Development Team
