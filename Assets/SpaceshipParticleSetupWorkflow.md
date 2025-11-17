# Spaceship Particle System - Setup Workflow

Step-by-step guide for setting up particle effects on your spaceship in Unreal Editor.

---

## Prerequisites

✅ Adastrea project opened in Unreal Engine 5.6  
✅ Latest code compiled (includes `USpaceshipParticleComponent`)  
✅ Ship Blueprint created (based on `ASpaceship`)  

---

## Phase 1: Create Particle Templates (Content Browser)

### 1.1 Main Engine Particles (GPU)

**Location**: `Content/Particles/Ships/MainEngine/`

1. Right-click → **Particle System** → Name: `PS_MainEngine`
2. Set to **GPU Sprites** for performance
3. Configure:
   - **Spawn Rate**: 200-500 particles/sec
   - **Lifetime**: 1.0-2.0 seconds
   - **Size**: Start 50, End 200 (engine plume expansion)
   - **Color**: Blue/White gradient → Orange fade
   - **Velocity**: -1000 Z (backwards from ship)
   - **Add Parameter**: `SpawnRate` (float) - Used for throttle control

### 1.2 Afterburner Particles (GPU)

**Location**: `Content/Particles/Ships/Afterburner/`

1. Right-click → **Particle System** → Name: `PS_Afterburner`
2. Set to **GPU Sprites**
3. Configure:
   - **Spawn Rate**: 300-800 particles/sec
   - **Lifetime**: 0.5-1.0 seconds (shorter burst)
   - **Size**: Start 80, End 300 (larger explosion)
   - **Color**: Bright white/blue → Orange/Yellow
   - **Velocity**: -1500 Z (faster than main engine)

### 1.3 RCS Thruster Particles (CPU)

**Location**: `Content/Particles/Ships/RCS/`

1. Right-click → **Particle System** → Name: `PS_RCSThruster`
2. Keep as **CPU Particles** (precise positioning)
3. Configure:
   - **Spawn Rate**: 50-100 particles/sec
   - **Lifetime**: 0.2-0.5 seconds (quick puff)
   - **Size**: Start 20, End 50 (small burst)
   - **Color**: White → Transparent
   - **Burst**: Initial burst of 20 particles
   - **Add Parameter**: `Intensity` (float) - Used for effect strength

### 1.4 Jump Drive Particles

**Location**: `Content/Particles/Ships/JumpDrive/`

Create 4 particle systems:

#### PS_JumpCharge
- **Type**: GPU Sprites
- **Effect**: Energy buildup, swirling particles around ship
- **Spawn Rate**: 100-300 increasing over time
- **Color**: Blue glow increasing in intensity

#### PS_JumpActivation
- **Type**: GPU Sprites
- **Effect**: Bright flash, explosion-like
- **Burst**: 500 particles instantly
- **Color**: Bright white flash
- **Lifetime**: 0.2 seconds

#### PS_JumpTunnel
- **Type**: GPU Sprites
- **Effect**: Wormhole/hyperspace streaks
- **Spawn Rate**: 400-800 particles/sec
- **Color**: Blue/purple streaks
- **Velocity**: Towards ship (tunnel effect)

#### PS_JumpExit
- **Type**: GPU Sprites
- **Effect**: Energy dissipation, expanding ring
- **Burst**: 300 particles
- **Color**: Blue → Transparent
- **Size**: Expanding ring pattern

### 1.5 Damage State Particles

**Location**: `Content/Particles/Ships/Damage/`

#### PS_EngineDamaged
- **Type**: CPU Particles
- **Effect**: Sparks, light smoke, sputtering
- **Spawn Rate**: 50-150 particles/sec (irregular)
- **Color**: Yellow/orange sparks
- **Lifetime**: 0.5-1.0 seconds

#### PS_EngineCritical
- **Type**: CPU Particles
- **Effect**: Heavy smoke, fire
- **Spawn Rate**: 200-500 particles/sec
- **Color**: Black smoke + orange fire
- **Lifetime**: 2.0-4.0 seconds (lingering smoke)

---

## Phase 2: Create Engine Glow Material

### 2.1 Base Material

**Location**: `Content/Materials/Ships/`

1. Right-click → **Material** → Name: `M_EngineGlow`
2. Material setup:
   ```
   Scalar Parameter: EmissiveIntensity (Default: 1.0)
   │
   ├→ Multiply with Base Color (blue/white)
   └→ Connect to Emissive Color
   
   Material Domain: Surface
   Blend Mode: Opaque or Translucent
   Shading Model: Unlit or Default Lit
   ```
3. Save material

### 2.2 Material Instance

1. Right-click `M_EngineGlow` → **Create Material Instance**
2. Name: `MI_EngineGlow_[ShipName]`
3. Set `EmissiveIntensity` default (will be controlled dynamically)
4. Apply to ship's engine mesh

---

## Phase 3: Configure Ship Blueprint

### 3.1 Open Ship Blueprint

1. Open your ship Blueprint (e.g., `BP_Ship_MittenzxMk1`)
2. Ensure it inherits from `ASpaceship` C++ class

### 3.2 Assign Particle Templates

In **Class Defaults** panel:

```
Components
└── Particle Component
    ├── Main Engine
    │   ├── Main Engine Template = PS_MainEngine
    │   ├── Afterburner Template = PS_Afterburner
    │   ├── Min Particle Rate Multiplier = 0.2
    │   └── Max Particle Rate Multiplier = 3.0
    │
    ├── RCS Thrusters
    │   ├── RCS Thruster Template = PS_RCSThruster
    │   └── RCS Thruster Duration = 0.5
    │
    ├── Jump Drive
    │   ├── Jump Charge Template = PS_JumpCharge
    │   ├── Jump Activation Template = PS_JumpActivation
    │   ├── Jump Tunnel Template = PS_JumpTunnel
    │   ├── Jump Exit Template = PS_JumpExit
    │   └── Jump Charge Duration = 3.0
    │
    ├── Damage States
    │   ├── Damaged Engine Template = PS_EngineDamaged
    │   └── Critical Damage Template = PS_EngineCritical
    │
    └── Performance
        └── Max Particles Per Ship = 10000
```

### 3.3 Create Material Instance Dynamic

In **Construction Script**:

```
Construction Script
└── Get Particle Component
    └── Create Dynamic Material Instance
        ├── Parent = MI_EngineGlow_[ShipName]
        └── Set to: Particle Component → Engine Material Instance
```

### 3.4 (Optional) Add Jump Control

In **Event Graph**:

```
Event Graph

Input Action: Jump (New)
├── Branch: Can Jump?
│   └── True
│       ├── Get Particle Component
│       │   └── Trigger Jump Sequence
│       │       └── Charge Duration = 3.0
│       └── Set Can Jump = False

Event: On Jump Sequence Complete (Override from Component)
└── Delay (10 seconds - Jump Cooldown)
    └── Set Can Jump = True
```

### 3.5 (Optional) Add Boost Control

In **Event Graph**:

```
Input Action: Boost
├── Pressed
│   └── Get Particle Component
│       └── Set Boost Active (True)
│
└── Released
    └── Get Particle Component
        └── Set Boost Active (False)
```

### 3.6 (Optional) Integrate Health/Damage

If you have a health component:

```
Event: On Health Changed
├── Branch: Health < 50%
│   ├── True
│   │   └── Get Particle Component
│   │       └── Set Damage State (Damaged)
│   └── False
│       └── Get Particle Component
│           └── Set Damage State (Normal)
│
└── Branch: Health < 20%
    └── True
        └── Get Particle Component
            └── Set Damage State (Critical)
```

---

## Phase 4: Testing in Editor

### 4.1 Basic Functionality Test

1. **Play in Editor** (PIE)
2. Possess your ship
3. Verify:
   - ✅ Main engine particles visible
   - ✅ Engine glow material emitting
   - ✅ Forward movement shows bright particles
   - ✅ Strafe left/right shows RCS puffs on opposite sides
   - ✅ Vertical movement shows RCS puffs

### 4.2 Throttle Response Test

1. Move forward slowly → Low particle intensity
2. Move forward quickly → High particle intensity
3. Verify smooth interpolation between intensities

### 4.3 RCS Thruster Test

Test all 6 axes:
- ✅ Forward input → Backward RCS fires
- ✅ Backward input → Forward RCS fires
- ✅ Strafe right → Left RCS fires
- ✅ Strafe left → Right RCS fires
- ✅ Move up → Down RCS fires
- ✅ Move down → Up RCS fires

### 4.4 Boost Test (If Implemented)

1. Hold boost button
2. Verify afterburner particles activate
3. Release boost
4. Verify afterburner particles deactivate

### 4.5 Jump Sequence Test (If Implemented)

1. Press jump button
2. Observe 4 stages:
   - Charge particles build up (3 seconds)
   - Bright flash (0.5 seconds)
   - Tunnel effect (2 seconds)
   - Exit flash (0.5 seconds)
3. Verify `OnJumpSequenceComplete` event fires

### 4.6 Damage State Test

In editor, manually set damage states:

1. **Console**: `Get Particle Component → Set Damage State (Damaged)`
   - Verify sputtering particles
   - Verify flickering glow

2. **Console**: `Get Particle Component → Set Damage State (Critical)`
   - Verify heavy smoke/fire
   - Verify very dim glow

3. **Console**: `Get Particle Component → Set Damage State (Normal)`
   - Verify damage effects stop
   - Verify normal glow restored

### 4.7 Performance Test

1. Spawn 10 ships in level
2. Open console: `stat particles`
3. Verify total particle count <100,000
4. Check frame rate (target: 60 FPS @ 1080p)
5. Monitor with: `stat fps` and `stat unit`

---

## Phase 5: Optimization

### 5.1 Particle Count Check

Use Blueprint to display particle count:

```
Event Tick
└── Get Particle Component
    └── Get Current Particle Count
        └── Print String: "Particles: " + [Count]
```

Target: <10,000 per ship

### 5.2 Adjust If Over Budget

If particle count exceeds 10,000:
1. Reduce spawn rates in templates
2. Shorten particle lifetimes
3. Reduce burst counts
4. Disable non-essential effects (RCS at distance)

### 5.3 LOD System (Advanced)

Implement distance-based LOD:

```
Event Tick
└── Get Distance to Player Camera
    └── Branch: Distance > 5000?
        ├── True
        │   └── Get Particle Component
        │       ├── Set RCS Thruster Duration = 0.0 (Disable)
        │       └── Set Min Particle Rate Multiplier = 0.1 (Reduce)
        └── False
            └── Get Particle Component
                ├── Set RCS Thruster Duration = 0.5 (Enable)
                └── Set Min Particle Rate Multiplier = 0.2 (Normal)
```

---

## Phase 6: Final Validation

### Checklist

- [ ] All 7 particle templates created and assigned
- [ ] Engine glow material created and assigned
- [ ] Material Instance Dynamic created in Blueprint
- [ ] Main engine responds to throttle
- [ ] All 6 RCS thrusters activate correctly
- [ ] Boost activates afterburner (if implemented)
- [ ] Jump sequence completes all 4 stages (if implemented)
- [ ] Damage states change particle effects correctly
- [ ] Particle count <10,000 per ship
- [ ] Performance: 60 FPS with 10 ships
- [ ] No console errors or warnings

---

## Troubleshooting

### Issue: No particles visible

**Solutions:**
1. Check templates are assigned in Blueprint
2. Verify particle spawn rate > 0
3. Check particle lifetime > 0
4. Ensure particle scale is visible
5. Check particle bounds culling

### Issue: RCS not activating

**Solutions:**
1. Verify `RCSThrusterTemplate` assigned
2. Check `RCSThrusterDuration` > 0
3. Test movement input is working
4. Check console for errors

### Issue: Performance lag

**Solutions:**
1. Check particle count with `stat particles`
2. Reduce spawn rates in templates
3. Use GPU particles for main engine
4. Implement LOD system
5. Check material complexity

### Issue: Jump sequence doesn't complete

**Solutions:**
1. Verify all 4 jump templates assigned
2. Check `OnJumpSequenceComplete` is implemented
3. Add print statements to debug stages
4. Verify `bJumpSequenceActive` flag

### Issue: Damage effects not showing

**Solutions:**
1. Check damage templates assigned
2. Verify `SetDamageState` is being called
3. Test manually in editor first
4. Check particle spawn parameters

---

## Ship-Specific Customization

### Mittenzx Mk1 (Balanced)
- Standard particle setup
- Moderate intensity
- All effects enabled

### Pathfinder Scout (Fast)
- Smaller particles
- Higher velocity
- Minimal RCS (efficiency)
- Enhanced sensor glow

### Warhammer Gunship (Combat)
- Larger, aggressive particles
- Higher intensity multipliers
- Military-style RCS bursts
- Dramatic damage effects

### Merchant Trader (Cargo)
- Larger, slower particles
- Lower intensity
- Softer glow
- Industrial appearance

### Starliner Luxury (Elegant)
- Refined, smooth particles
- Elegant color palette
- Subtle glow
- Premium appearance

### Excavator Mining (Industrial)
- Heavy, industrial particles
- Darker colors
- Rugged appearance
- Work-focused aesthetics

### Sovereign Command (Capital)
- Multiple engine clusters
- Grand scale effects
- Majestic appearance
- High particle counts (optimize!)

---

## Next Steps

1. ✅ Complete Phase 1-3: Basic setup
2. ✅ Test functionality in editor (Phase 4)
3. ✅ Optimize performance (Phase 5)
4. ✅ Validate all checklist items (Phase 6)
5. → Create variants for all 6 ship templates
6. → Integrate with combat/health systems
7. → Polish visual appearance
8. → Create particle effect presets for designers

---

## Support

- **Full Guide**: `Assets/SpaceshipParticleSystemGuide.md`
- **Quick Reference**: `Assets/SpaceshipParticleQuickReference.md`
- **Code**: `Source/Adastrea/Public/Ships/SpaceshipParticleComponent.h`

---

**Estimated Setup Time**: 2-4 hours for first ship, 30 minutes per additional ship

**Version**: 1.0  
**Updated**: 2025-11-17
