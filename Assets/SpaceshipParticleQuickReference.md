# Spaceship Particle System - Quick Reference

Fast lookup guide for particle system implementation.

---

## Component Setup

```cpp
// C++ - Already integrated in Spaceship.cpp
ParticleComponent = CreateDefaultSubobject<USpaceshipParticleComponent>(TEXT("ParticleComponent"));
```

**Blueprint**: Access via `Get Particle Component` from ship reference

---

## Main Engine

### Update Throttle
```cpp
ParticleComponent->UpdateThrottle(0.75f); // 0.0 to 1.0
```

### Boost Mode
```cpp
ParticleComponent->SetBoostActive(true);  // Enable
ParticleComponent->SetBoostActive(false); // Disable
```

### Properties to Set
- `MainEngineTemplate`: GPU particle system
- `AfterburnerTemplate`: GPU particle system
- `EngineMaterialInstance`: Material with `EmissiveIntensity` parameter

---

## RCS Thrusters

### Axes
| Axis | Enum Value | Location |
|------|-----------|----------|
| Forward | `ERCSThrusterAxis::Forward` | Front (+X) |
| Backward | `ERCSThrusterAxis::Backward` | Rear (-X) |
| Left | `ERCSThrusterAxis::Left` | Left (-Y) |
| Right | `ERCSThrusterAxis::Right` | Right (+Y) |
| Up | `ERCSThrusterAxis::Up` | Top (+Z) |
| Down | `ERCSThrusterAxis::Down` | Bottom (-Z) |

### Activate Manually
```cpp
ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Forward, 1.0f);
```

### Properties to Set
- `RCSThrusterTemplate`: CPU particle system
- `RCSThrusterDuration`: 0.5 seconds (default)

**Note**: RCS automatically activates from movement input (already implemented)

---

## Jump Drive

### Trigger Sequence
```cpp
ParticleComponent->TriggerJumpSequence();        // Default duration
ParticleComponent->TriggerJumpSequence(5.0f);   // Custom charge time
```

### Sequence Stages
1. **Charge** - 3 seconds (customizable)
2. **Activation** - 0.5 seconds
3. **Tunnel** - 2 seconds
4. **Exit** - 0.5 seconds

### Blueprint Event
```cpp
Event OnJumpSequenceComplete
└── [Your jump completion logic]
```

### Properties to Set
- `JumpChargeTemplate`: Charge particle system
- `JumpActivationTemplate`: Flash particle system
- `JumpTunnelTemplate`: Tunnel particle system
- `JumpExitTemplate`: Exit particle system
- `JumpChargeDuration`: 3.0 seconds (default)

---

## Damage States

### Set State
```cpp
ParticleComponent->SetDamageState(EEngineDamageState::Normal);   // No damage
ParticleComponent->SetDamageState(EEngineDamageState::Damaged);  // Sputtering
ParticleComponent->SetDamageState(EEngineDamageState::Critical); // Heavy smoke
```

### Properties to Set
- `DamagedEngineTemplate`: Sputtering particle system
- `CriticalDamageTemplate`: Smoke/fire particle system

---

## Performance

### Get Particle Count
```cpp
int32 Count = ParticleComponent->GetCurrentParticleCount();
```

### Budget
- **Target**: <10,000 particles per ship
- **Max Property**: `MaxParticlesPerShip` (default: 10000)

### Particle Distribution
| System | Recommended Count |
|--------|-------------------|
| Main Engine | 1500 |
| Afterburner | 800 |
| RCS (6 total) | 600 |
| Jump Effects | 2000 |
| Damage Effects | 1000 |
| **Total** | **5900** |

---

## Blueprint Property Assignment

**In Ship Blueprint Class Defaults:**

```
Components
└── Particle Component
    ├── Main Engine
    │   ├── Main Engine Template
    │   ├── Afterburner Template
    │   ├── Min Particle Rate Multiplier = 0.2
    │   └── Max Particle Rate Multiplier = 3.0
    ├── RCS Thrusters
    │   ├── RCS Thruster Template
    │   └── RCS Thruster Duration = 0.5
    ├── Jump Drive
    │   ├── Jump Charge Template
    │   ├── Jump Activation Template
    │   ├── Jump Tunnel Template
    │   ├── Jump Exit Template
    │   └── Jump Charge Duration = 3.0
    ├── Damage States
    │   ├── Damaged Engine Template
    │   └── Critical Damage Template
    └── Performance
        └── Max Particles Per Ship = 10000
```

---

## Common Functions

### Read-Only Properties
```cpp
float CurrentThrottle = ParticleComponent->CurrentThrottle;  // 0-1
bool IsBoosting = ParticleComponent->bBoostActive;
bool IsJumping = ParticleComponent->bJumpSequenceActive;
EEngineDamageState State = ParticleComponent->DamageState;
```

### Components Access
```cpp
UParticleSystemComponent* MainEngine = ParticleComponent->MainEngineComponent;
UParticleSystemComponent* Afterburner = ParticleComponent->AfterburnerComponent;
// See header for all component accessors
```

---

## Automatic Features

✅ **Main engine** activates on BeginPlay  
✅ **RCS thrusters** activate from movement input  
✅ **Engine glow** updates automatically  
✅ **Throttle scaling** linear interpolation  
✅ **Jump sequence** auto-progresses through stages  
✅ **RCS auto-deactivate** after duration  

---

## Testing Commands

```cpp
// Console commands
stat particles   // Show particle statistics
stat fps        // Frame rate
stat unit       // CPU/GPU timing
profileGPU      // GPU profiling
```

---

## Integration Examples

### Health-Based Damage
```cpp
Event On Health Changed
├── Branch (Health < 50%)
│   ├── True → Set Damage State (Damaged)
│   └── False → Set Damage State (Normal)
└── Branch (Health < 20%)
    └── True → Set Damage State (Critical)
```

### Jump with Cooldown
```cpp
Event On Jump Pressed
├── Branch (Can Jump?)
│   └── True
│       ├── Trigger Jump Sequence
│       └── Set Can Jump = False
│
Event On Jump Sequence Complete
└── Delay (10 seconds)
    └── Set Can Jump = True
```

### Boost Input
```cpp
Event On Boost Pressed
└── Set Boost Active (True)

Event On Boost Released
└── Set Boost Active (False)
```

---

## File Locations

**C++ Header**: `Source/Adastrea/Public/Ships/SpaceshipParticleComponent.h`  
**C++ Implementation**: `Source/Adastrea/Ships/SpaceshipParticleComponent.cpp`  
**Integration**: `Source/Adastrea/Ships/Spaceship.h/cpp`  
**Full Guide**: `Assets/SpaceshipParticleSystemGuide.md`

---

## Support Matrix

| Ship Template | Status |
|--------------|--------|
| Mittenzx Mk1 | ✅ Supported |
| Pathfinder Scout | ✅ Supported |
| Warhammer Gunship | ✅ Supported |
| Merchant Trader | ✅ Supported |
| Starliner Luxury | ✅ Supported |
| Excavator Mining | ✅ Supported |
| Sovereign Command | ✅ Supported |

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| No particles visible | Check templates assigned, verify scale/lifetime |
| Performance lag | Check particle count, use GPU particles |
| RCS not working | Verify movement input, check duration > 0 |
| Jump doesn't complete | Ensure all 4 templates assigned |
| Damage not showing | Verify SetDamageState called, templates assigned |

---

**Quick Start**: Assign particle templates → Test throttle → Test RCS → Test jump → Test damage  
**See**: [Full Guide](SpaceshipParticleSystemGuide.md) for detailed instructions

---

**Version**: 1.0  
**Updated**: 2025-11-17
