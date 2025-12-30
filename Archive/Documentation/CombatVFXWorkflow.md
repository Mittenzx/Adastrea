# Combat VFX Designer Workflow

Step-by-step workflow for creating and implementing combat visual effects in Adastrea.

---

## Quick Reference

| Task | Time | Difficulty |
|------|------|------------|
| Create Weapon Fire Effect | 30-60 min | Medium |
| Create Impact Effect | 20-40 min | Easy |
| Setup Shield Feedback | 15-30 min | Easy |
| Create Explosion Effect | 45-90 min | Medium |
| Setup Heat Visuals | 20-30 min | Easy |

---

## Workflow 1: Creating a Weapon Fire Effect

### Goal
Create a complete weapon fire effect for a new weapon type.

### Prerequisites
- Unreal Engine 5.6 project open
- Basic knowledge of Niagara
- Understanding of weapon type characteristics

### Steps

#### 1. Plan Your Effect (5 minutes)

**Questions to Answer:**
- What weapon type? (Energy, Projectile, Missile, Torpedo, Beam, Plasma)
- What's the visual theme? (Color, intensity, style)
- What's the gameplay feel? (Fast/slow, powerful/weak)

**Design Specifications:**
```yaml
Weapon: Heavy Plasma Cannon
Type: Plasma
Visual Theme: Green ball lightning with electrical arcing
Color: RGB(0, 255, 100)
Speed: Medium (3000 cm/s)
Particle Count Budget: 800 particles
Effect Duration: Projectile travel time + 1 second
```

#### 2. Create Niagara System (20-30 minutes)

**2a. Create Base System:**
1. Content Browser → Right-click → FX → Niagara System
2. Template: Select "Projectile" or start from "Empty"
3. Name: `NS_Plasma_HeavyCannon`
4. Location: `Content/VFX/Combat/Weapons/Plasma/`

**2b. Configure Main Emitter:**
```
Emitter Properties:
├── Spawn Module
│   ├── Spawn Rate: 200 particles/sec
│   └── Spawn Burst: 50 particles on spawn
│
├── Initialize Particle
│   ├── Lifetime: 1.5 seconds
│   ├── Color: Green (0, 255, 100)
│   ├── Sprite Size: Random 10-30
│   └── Initial Velocity: Forward + Random spread
│
├── Particle Update
│   ├── Drag: 0.5 (slight deceleration)
│   └── Color over Life: Green → Dark Green → Transparent
│
└── Render
    ├── Sprite Renderer
    └── Material: M_Plasma_Additive
```

**2c. Add Lightning Arcs (for Plasma):**
```
Secondary Emitter: "Lightning Arcs"
├── Spawn Rate: 10 arcs/sec
├── Lifetime: 0.2 seconds
├── Use Beam Renderer
├── Arc from center to random points
└── Color: Bright cyan-white
```

**2d. Add Glow Core:**
```
Tertiary Emitter: "Core Glow"
├── Single particle
├── Lifetime: Match main emitter
├── Large sprite (100-200 cm)
├── Very bright
└── Additive blend
```

#### 3. Create Data Asset (10 minutes)

**3a. Create CombatVFXDataAsset:**
1. Content Browser → Right-click → Blueprint → CombatVFXDataAsset
2. Name: `DA_VFX_PlasmaHeavyCannon`
3. Location: `Content/DataAssets/Combat/VFX/`

**3b. Configure Properties:**
```yaml
Basic Info:
  VFX Name: "Heavy Plasma Cannon Effects"
  Description: "Green ball lightning with electrical arcing"
  Weapon Type: Plasma

Fire Effect:
  Muzzle Flash Effect: NS_Plasma_MuzzleFlash
  Projectile Trail Effect: NS_Plasma_HeavyCannon
  Fire Sound: S_PlasmaFire_Heavy
  Beam Type: Projectile
  Beam Width: 15.0
  Beam Color: (0, 1, 0.4, 1)
  Glow Intensity: 2.5
  
Performance:
  Max Particles: 800
  Effect Priority: 6
  Max Render Distance: 15000
```

#### 4. Test in Editor (10-15 minutes)

**4a. Open Test Map:**
1. Open `Content/Maps/TestMap_CombatVFX`
2. If doesn't exist, create new map

**4b. Add Test Setup:**
```
1. Add BP_TestShip to scene
2. Position camera for good view
3. Add target dummy ship
4. Add directional light
5. Add post-process volume for glow
```

**4c. Test Fire Effect:**
```
1. Play in Editor (Alt+P)
2. Press fire button (configured input)
3. Observe:
   - Muzzle flash appears
   - Projectile travels to target
   - Trail effect visible
   - Lightning arcs present
   - Sound plays
```

**4d. Adjust Parameters:**
```
Common Adjustments:
- Particle count: Too many/few?
- Speed: Too fast/slow?
- Color: Right shade?
- Size: Appropriate scale?
- Sound: Right volume?
```

#### 5. Polish and Optimize (10-15 minutes)

**5a. Add Camera Shake:**
1. Create Camera Shake Blueprint
2. Configure shake magnitude
3. Assign to Data Asset
4. Test feel

**5b. Check Performance:**
```
Console Commands:
- stat fps        (target 60+ FPS)
- stat particles  (check count)
- stat niagara    (check systems)
```

**5c. Optimize if Needed:**
```
If Performance Issues:
1. Reduce spawn rate
2. Reduce max particles
3. Enable LOD
4. Adjust bounds
5. Increase distance culling
```

#### 6. Document and Save (5 minutes)

**6a. Create Usage Note:**
```yaml
DA_VFX_PlasmaHeavyCannon_README:
  Description: "Heavy plasma cannon visual effects"
  Weapon Type: Plasma
  Performance: 800 particles max, 60 FPS maintained
  Special Features: Lightning arcing, bright core glow
  Best Used For: Heavy fighter weapons, capital ship point defense
  Color Theme: Green plasma with cyan arcs
  Notes: "Looks best at medium range (1000-5000m)"
```

**6b. Save All Assets:**
```
Save:
- Niagara systems
- Data Asset
- Any new materials
- Test map (if created)
```

---

## Workflow 2: Creating Impact Effects

### Goal
Create shield and hull impact effects.

### Steps

#### 1. Create Shield Impact Ripple (20 minutes)

**1a. Create Niagara System:**
1. New Niagara System: `NS_Shield_ImpactRipple`
2. Template: "Mesh Renderer"
3. Location: `Content/VFX/Combat/Impacts/`

**1b. Configure Emitter:**
```
Main Emitter:
├── Spawn Burst: 1 particle
├── Lifetime: 1.0 seconds
├── Mesh: Hexagon_LowPoly
├── Scale: 10 → 200 cm over lifetime
├── Rotation: Random initial
└── Material: M_Shield_Ripple (translucent)

Material Setup:
├── Hexagonal pattern texture
├── Fresnel for edge glow
├── Opacity: 1 → 0 over lifetime
└── Color: User parameter (damage-based)
```

**1c. Add Energy Dissipation:**
```
Secondary Emitter:
├── Spawn Rate: 100 particles/sec
├── Lifetime: 0.5 seconds
├── Initial Velocity: Radial outward
├── Drag: High (particles slow quickly)
├── Color: Match ripple color
└── Size: Small sprites (5-10 cm)
```

#### 2. Create Hull Impact Effect (15 minutes)

**2a. Create Spark Effect:**
1. New Niagara System: `NS_Hull_Sparks`
2. Template: "GPU Particles"

```
Spark Emitter:
├── Spawn Burst: 50-100 particles
├── Lifetime: 0.3-0.6 seconds (random)
├── Initial Velocity: Bounce direction + random
├── Gravity: Apply realistic fall
├── Color: Yellow-white → Orange → Dark
└── Size: 2-5 cm
```

**2b. Create Debris Effect:**
1. New Niagara System: `NS_Hull_Debris`

```
Debris Emitter:
├── Spawn Burst: 20-30 particles
├── Lifetime: 1.0-2.0 seconds
├── Mesh: Small metal chunks
├── Physics: Tumbling rotation
├── Velocity: Outward with some inherit
└── Color: Gray metal
```

**2c. Create Damage Decal:**
1. Create Material: `M_Decal_HullDamage`
2. Type: Deferred Decal
3. Texture: Scorch/impact mark
4. Blend Mode: Translucent

#### 3. Create Explosion Effect (30-45 minutes)

**3a. Main Explosion:**
1. New Niagara System: `NS_Explosion_Medium`

```
Main Burst:
├── Spawn Burst: 1000 particles
├── Lifetime: 2.0 seconds
├── Initial Velocity: Radial, high speed
├── Drag: Medium
├── Size: 20 → 100 cm over lifetime
└── Color: White → Orange → Red → Black
```

**3b. Shockwave:**
```
Shockwave Emitter:
├── Spawn: 1 particle
├── Lifetime: 0.5 seconds
├── Mesh: Ring
├── Scale: 0 → Explosion Radius
├── Material: Distortion + Additive
└── Opacity: 1 → 0
```

**3c. Flash:**
```
Flash Emitter:
├── Spawn: 1 particle
├── Lifetime: 0.1 seconds
├── Sprite: Large, bright white
├── Scale: Big → 0
└── Very bright (5-10x intensity)
```

#### 4. Create Impact Data Asset (10 minutes)

**4a. Create Asset:**
1. Right-click → Blueprint → ImpactEffectDataAsset
2. Name: `DA_Impact_Standard`

**4b. Configure:**
```yaml
Shield Impact:
  Ripple Effect: NS_Shield_ImpactRipple
  Dissipation Effect: NS_Shield_Dissipation
  Impact Sound: S_ShieldImpact
  Ripple Speed: 1000.0
  Ripple Duration: 1.0
  Max Ripple Radius: 200.0

Hull Impact:
  Spark Effect: NS_Hull_Sparks
  Debris Effect: NS_Hull_Debris
  Damage Decal: M_Decal_HullDamage
  Impact Sound: S_HullImpact
  Decal Size: 50.0
  Decal Lifetime: 60.0

Medium Explosion:
  Explosion Effect: NS_Explosion_Medium
  Shockwave Effect: NS_Explosion_Shockwave
  Explosion Sound: S_Explosion_Medium
  Explosion Radius: 500.0
  Flash Intensity: 5.0
```

#### 5. Test Impact Effects (10 minutes)

**5a. Test Shield Impact:**
```
1. Spawn test ship with shields
2. Fire weapon at ship
3. Verify ripple expands
4. Check color intensity matches damage
5. Verify dissipation particles
```

**5b. Test Hull Impact:**
```
1. Spawn test ship without shields
2. Fire weapon at ship
3. Verify sparks fly
4. Check debris spawns
5. Verify decal applied
```

**5c. Test Explosion:**
```
1. Place explosive barrel or spawn missile
2. Trigger explosion
3. Check radius correct
4. Verify shockwave expands
5. Check camera shake
```

---

## Workflow 3: Setting Up Shield Visual Feedback

### Goal
Configure shield visual states (full/recharging/down).

### Steps

#### 1. Create Shield Material (15 minutes)

**1a. Create Base Material:**
1. Content Browser → Create Material
2. Name: `M_Shield_Base`
3. Location: `Content/Materials/Combat/`

**1b. Material Graph:**
```
Material Setup:
├── Material Domain: Surface
├── Blend Mode: Translucent
├── Shading Model: Unlit
│
├── Parameters:
│   ├── ShieldColor (Vector)
│   ├── Intensity (Scalar)
│   ├── FresnelPower (Scalar, default 3.0)
│   └── OpacityStrength (Scalar, default 0.8)
│
└── Logic:
    ├── Fresnel (for edge glow)
    ├── Multiply by Intensity
    ├── Add ShieldColor
    └── Output to Emissive + Opacity
```

#### 2. Create Shield Niagara Effect (10 minutes)

**2a. Create System:**
1. New Niagara: `NS_Shield_Glow`
2. Template: "Sprite Particles"

```
Shield Glow:
├── Spawn Rate: 50 particles/sec
├── Lifetime: 2.0 seconds
├── Location: Surface of shield mesh
├── Velocity: Very slight movement
├── Size: Medium (20-40 cm)
├── Color: User parameter (state-based)
└── Material: Additive soft particles
```

#### 3. Configure ShieldVFXComponent (10 minutes)

**3a. Add to Ship:**
1. Open ship Blueprint
2. Add Component → ShieldVFXComponent
3. Position doesn't matter (manages materials)

**3b. Configure Properties:**
```yaml
Shield Visuals:
  Shield Base Material: M_Shield_Base
  Shield Effect Template: NS_Shield_Glow
  Combat Health Component: (Auto-detected)

Shield Colors:
  Full Shield Color: (0, 1, 0, 0.8)      # Green
  Recharging Color: (0, 0.5, 1, 0.8)    # Blue  
  Shield Down Color: (1, 0, 0, 0.8)     # Red
  Recharge Pulse Speed: 2.0
  Min Pulse Intensity: 0.3
  Max Pulse Intensity: 1.5
```

#### 4. Test Shield States (15 minutes)

**4a. Test Full State:**
```
1. Start with full shields
2. Verify green glow
3. Check intensity steady
4. Verify all facings show green
```

**4b. Test Recharging:**
```
1. Damage shields to 50%
2. Wait for recharge delay
3. Verify blue pulse starts
4. Check pulse speed (2 Hz)
5. Verify intensity oscillates
```

**4c. Test Shield Down:**
```
1. Drain shields to 0%
2. Verify red glow appears
3. Check on correct facing(s)
4. Verify warning is clear
```

**4d. Test Directional (if applicable):**
```
1. Damage forward shields only
2. Verify only forward shows red/blue
3. Other facings remain green
4. Check state independence
```

---

## Workflow 4: Adding Weapon Heat Visuals

### Goal
Create heat management visual feedback for weapons.

### Steps

#### 1. Create Heat Gradient Material (15 minutes)

**1a. Create Material:**
1. New Material: `M_Weapon_HeatGradient`
2. Location: `Content/Materials/Combat/`

**1b. Material Graph:**
```
Heat Material:
├── Parameters:
│   ├── HeatColor (Vector)
│   ├── HeatPercentage (Scalar, 0-1)
│   ├── ShimmerIntensity (Scalar, 0-1)
│   └── OverheatWarning (Scalar, 0 or 1)
│
└── Logic:
    ├── Lerp between Cool/Warning/Hot colors
    ├── Add heat shimmer (using HeatPercentage)
    ├── Add pulsing when OverheatWarning active
    └── Output to Emissive
```

#### 2. Create Heat Vent Effect (10 minutes)

**2a. Create Niagara:**
1. New System: `NS_Weapon_HeatVent`
2. Template: "CPU Particles"

```
Heat Vent:
├── Spawn Rate: 20 * Intensity
├── Lifetime: 1.0-2.0 seconds
├── Location: Weapon vent location
├── Velocity: Upward + slight random
├── Size: 10-30 cm
├── Color: White steam with opacity fade
├── Material: Soft smoke/steam
└── User Parameter: Intensity (0-1)
```

#### 3. Configure in CombatVFXDataAsset (5 minutes)

**3a. Update Data Asset:**
```yaml
Heat Visuals:
  Heat Vent Effect: NS_Weapon_HeatVent
  Heat Gradient Material: M_Weapon_HeatGradient
  Cool Color: (0.1, 0.3, 0.5, 1)    # Blue
  Warning Color: (1, 0.5, 0, 1)     # Orange
  Overheat Color: (1, 0, 0, 1)      # Red
  Max Shimmer Intensity: 0.8
```

#### 4. Test Heat System (10 minutes)

**4a. Test Cool to Warning:**
```
1. Fire weapon repeatedly
2. Observe color change: Blue → Orange
3. Check gradient smooth
4. Verify at 50% heat
```

**4b. Test Warning to Overheat:**
```
1. Continue firing
2. Observe: Orange → Red
3. Check shimmer intensity increases
4. Verify overheat warning triggers
```

**4c. Test Cooling:**
```
1. Stop firing
2. Let weapon cool
3. Verify color returns: Red → Orange → Blue
4. Check heat vent steam appears
5. Verify steam intensity matches heat level
```

---

## Workflow 5: Complete Ship VFX Setup

### Goal
Set up all combat VFX on a new ship from scratch.

### Time Required
30-45 minutes (depending on complexity)

### Steps

#### 1. Prepare Assets (5 minutes)

**Gather Required Assets:**
- [ ] Weapon VFX Data Asset (DA_VFX_[WeaponType])
- [ ] Impact Effect Data Asset (DA_Impact_Standard)
- [ ] Shield material (if ship has shields)
- [ ] Shield Niagara effect (if ship has shields)

#### 2. Add Components (5 minutes)

**2a. Open Ship Blueprint:**
1. Content Browser → Ships → Open `BP_Ship_[YourShip]`

**2b. Add CombatVFXComponent:**
```
1. Components Panel → Add Component
2. Search: "CombatVFX"
3. Select: CombatVFXComponent
4. Name: "CombatVFX"
```

**2c. Add ShieldVFXComponent (if applicable):**
```
1. Components Panel → Add Component
2. Search: "ShieldVFX"
3. Select: ShieldVFXComponent
4. Name: "ShieldVFX"
```

#### 3. Configure CombatVFXComponent (10 minutes)

**3a. Basic Configuration:**
```yaml
Combat VFX:
  Combat VFX Data: DA_VFX_[YourWeaponType]
  Impact Effect Data: DA_Impact_Standard
  Weapon Components: (Leave empty for auto-detect)

Performance:
  Max Active Effects: 50 (fighters) / 100 (capitals)
  Enable Distance Culling: true
  Max Effect Distance: 10000.0
```

**3b. Verify Weapon Detection:**
```
1. Compile Blueprint
2. Check "Weapon Components" auto-populated
3. If not, manually add WeaponComponent references
```

#### 4. Configure ShieldVFXComponent (10 minutes)

**4a. Basic Configuration:**
```yaml
Shield Visuals:
  Shield Base Material: M_Shield_Base
  Shield Effect Template: NS_Shield_Glow
  Combat Health Component: (Leave empty for auto-detect)

Shield Colors:
  Full Shield Color: (0, 1, 0, 0.8)
  Recharging Color: (0, 0.5, 1, 0.8)
  Shield Down Color: (1, 0, 0, 0.8)
  Recharge Pulse Speed: 2.0
  Min Pulse Intensity: 0.3
  Max Pulse Intensity: 1.5
```

**4b. Verify Health Component:**
```
1. Compile Blueprint
2. Check "Combat Health Component" auto-populated
3. If not, manually assign from ship components
```

#### 5. Hook Up Events (10-15 minutes)

**5a. Weapon Fire Event:**
```
Event Graph:
1. Find or create "Weapon Fired" event
2. Add node: Get CombatVFXComponent
3. Add node: Play Weapon Fire Effect
   - Weapon Type: Get from WeaponComponent
   - Muzzle Location: WeaponComponent->GetWeaponWorldPosition
   - Muzzle Rotation: WeaponComponent->GetAimDirection
   - Target Location: Current Target (if any)
4. Connect execution
```

**5b. Damage Received Event:**
```
Event Graph:
1. Find or create "On Damage Received" event
2. Add Branch: Check if shields absorbed damage
3. True Branch:
   - Get CombatVFXComponent
   - Call PlayShieldImpact
4. False Branch:
   - Get CombatVFXComponent
   - Call PlayHullImpact
```

**5c. Explosion Event (if ship can be destroyed):**
```
Event Graph:
1. Find or create "On Destroyed" event
2. Get CombatVFXComponent
3. Call PlayExplosion
   - Explosion Location: Actor Location
   - Explosion Size: Based on ship class
   - Explosion Radius: Ship size * 2
```

#### 6. Test Complete Setup (10 minutes)

**6a. Test Weapon Fire:**
```
1. PIE (Play In Editor)
2. Fire all weapons
3. Verify effects appear
4. Check sound plays
5. Verify no errors
```

**6b. Test Impacts:**
```
1. Get hit by enemy
2. Verify shield impact (if shields up)
3. Drain shields
4. Verify hull impact (shields down)
5. Check decals applied
```

**6c. Test Shield States:**
```
1. Full shields: Green glow
2. Take damage: Blue pulse
3. Shields down: Red warning
4. Verify transitions smooth
```

**6d. Test Heat Management:**
```
1. Fire repeatedly
2. Watch heat build: Blue → Orange → Red
3. Stop firing
4. Watch cooling: Red → Orange → Blue
5. Verify heat vents steam
```

---

## Common Issues & Solutions

### Issue: Effect Not Appearing

**Possible Causes:**
1. Niagara system not assigned
2. Component not active
3. Outside render distance
4. Effect priority too low
5. Particle budget exceeded

**Solutions:**
```
Check:
1. Data Asset has Niagara system assigned
2. Component "Auto Activate" = true
3. MaxEffectDistance high enough
4. EffectPriority >= 5
5. MaxActiveEffects not reached
```

### Issue: Poor Performance

**Symptoms:**
- FPS drops when firing
- Stuttering during combat
- High particle counts

**Solutions:**
```
Optimizations:
1. Reduce spawn rates
2. Lower MaxActiveEffects
3. Enable distance culling
4. Use LOD in Niagara
5. Check bounds settings
6. Profile with stat commands
```

### Issue: Colors Wrong

**Symptoms:**
- Shield colors not changing
- Heat gradient incorrect
- Impact colors don't match damage

**Solutions:**
```
Check:
1. Color parameters set in Data Asset
2. Material parameters named correctly
3. Component getting correct values
4. Color space (Linear vs sRGB)
```

### Issue: Timing Problems

**Symptoms:**
- Effects too short/long
- Pulses too fast/slow
- Beams don't sustain

**Solutions:**
```
Adjust:
1. Lifetime in Niagara system
2. Duration parameters in Data Asset
3. Pulse speed in ShieldVFXComponent
4. Beam duration in effect configuration
```

---

## Performance Guidelines

### Target Metrics

```yaml
Per-Effect Budgets:
  Muzzle Flash: <0.5ms, <100 particles
  Beam Effect: <0.8ms, <500 particles
  Projectile Trail: <0.6ms, <300 particles
  Shield Impact: <1.0ms, <500 particles
  Hull Impact: <0.8ms, <400 particles
  Small Explosion: <1.5ms, <1000 particles
  Medium Explosion: <2.5ms, <2000 particles
  Large Explosion: <5.0ms, <5000 particles

Total Budget: <5ms for all active effects
```

### Optimization Checklist

- [ ] Use GPU particles for high counts (beams, trails)
- [ ] Use CPU particles for complex behavior (missiles)
- [ ] Enable LOD in all Niagara systems
- [ ] Set appropriate fixed bounds
- [ ] Use distance culling
- [ ] Limit active effects (MaxActiveEffects)
- [ ] Profile regularly (stat particles, stat niagara)
- [ ] Test on target hardware

---

## Quick Reference

### Console Commands

```
// Show FPS
stat fps

// Show particle stats
stat particles

// Show Niagara stats
stat niagara

// Show unit time
stat unit

// Profile GPU
profilegpu

// Toggle particle visibility
show particles

// Show bounds for debugging
show bounds
```

### Keyboard Shortcuts (Editor)

```
PIE (Play In Editor): Alt + P
Simulate: Alt + S
Eject: F8
Pause: Pause key
Single step: .  (period)
Stop: Esc
```

---

## Additional Resources

- **Full System Guide**: `Assets/CombatVFXSystemGuide.md`
- **Combat System Guide**: `Assets/CombatSystemGuide.md`
- **YAML Templates**: `Assets/CombatVFXTemplates/`
- **Example Ships**: `Content/Examples/Ships/`

---

**Document Version**: 1.0  
**Last Updated**: November 18, 2025  
**Author**: Adastrea Development Team
