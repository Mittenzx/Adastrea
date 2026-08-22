# Combat System Quick Reference

## Component Overview

| Component | Purpose | Key Functions |
|-----------|---------|---------------|
| **WeaponComponent** | Weapon firing & management | `Fire()`, `StartReload()`, `CanFire()` |
| **TargetingComponent** | Target acquisition & tracking | `SelectTarget()`, `SelectNextTarget()`, `CalculateLeadPosition()` |
| **CombatHealthComponent** | Health, shields, damage | `ApplyTypedDamage()`, `RestoreHull()`, `RestoreShields()` |
| **PointDefenseComponent** | Auto missile defense | `Activate()`, `EngageThreat()`, `GetThreatCount()` |
| **BoardingComponent** | Ship capture mechanics | `InitiateBoardingAction()`, `SurrenderShip()`, `GetBoardingProgress()` |

## Weapon Types

| Type | Best Against | Characteristics |
|------|--------------|-----------------|
| **Energy** | Shields | Instant hit, no ammo, high heat |
| **Projectile** | Armor | Travel time, limited ammo, reload |
| **Missile** | Large ships | Homing, vulnerable to PD, AOE |
| **Torpedo** | Capitals | Massive damage, slow, easy to intercept |
| **Beam** | Shields | Continuous, high power, EMP |
| **Plasma** | Balanced | Splash damage, thermal effects |

## Damage Types

| Type | Armor Pen | Shield Pen | Notes |
|------|-----------|------------|-------|
| **Kinetic** | High (70-95) | Low (20-50) | Physical impact |
| **Energy** | Low (20-50) | High (70-90) | Lasers, beams |
| **Explosive** | Med (50-70) | Med (50-70) | AOE damage |
| **Thermal** | Med (60-75) | Med (60-75) | Heat damage |
| **EMP** | Very Low (10-20) | Very High (90+) | System disable |

## Mount Sizes

| Size | Typical Ships | DPS Range | Examples |
|------|---------------|-----------|----------|
| **Small** | Fighters, Corvettes | 50-150 | Pulse laser, autocannon |
| **Medium** | Frigates, Cruisers | 150-400 | Beam, railgun |
| **Large** | Battleships | 300-800 | Heavy railgun, plasma |
| **Capital** | Capital ships | 500-1500+ | Spinal lance, mass driver |

## Shield Systems

### Omnidirectional
- Single shield pool
- Simpler configuration
- Equal protection all sides

### Directional
- Four facings (Forward/Aft/Port/Starboard)
- Configure % per facing
- Damage only affects specific facing
- Tactical positioning matters

## Damage Flow

```
Incoming Damage
    ↓
Check Active Shields?
    ↓
[YES] → Apply Shield Penetration
    ↓
Shield Absorbs Damage
    ↓
Remaining Damage → Hull
    ↓
[NO] → Apply Armor Penetration
    ↓
Reduced Damage to Hull
```

## Targeting Modes

| Mode | Use Case | Blueprint Function |
|------|----------|-------------------|
| **Manual** | Player selection | `SelectTarget(Actor)` |
| **Next/Previous** | Cycle through | `SelectNextTarget()` / `SelectPreviousTarget()` |
| **Nearest** | Auto closest | `SelectNearestTarget()` |
| **Highest Threat** | Auto dangerous | `SelectHighestThreatTarget()` |

## Point Defense Priority

1. **Missiles** (if Prioritize Missiles enabled)
2. **Distance** (closer threats first)
3. **Time to Impact** (imminent threats)
4. **Current Engagement** (unengaged threats first)

## Boarding Phases

| Phase | Duration | Description |
|-------|----------|-------------|
| **Approaching** | 10s | Moving to target |
| **Breaching** | 30s (configurable) | Cutting through hull |
| **Combat** | Variable | Crew fighting (5s intervals) |
| **Securing** | 20s | Taking control |
| **Complete** | - | Ship captured |

## Combat Formulas

### Weapon DPS
```
DPS = Base Damage × Rate of Fire
```

### Shield Damage
```
Shield Damage = Damage × (1 - Shield Penetration / 100)
```

### Armor Reduction
```
Effective Armor = max(0, Armor Rating - Armor Penetration)
Damage Multiplier = 1 / (1 + Effective Armor / 100)
Hull Damage = Remaining Damage × Damage Multiplier
```

### Critical Hit
```
if (Random < Critical Chance):
    Damage = Base Damage × Critical Multiplier
```

### Boarding Strength
```
Combat Strength = Crew × (Skill/100) × (Equipment/100) × (Morale/100)
Defender Strength = Combat Strength × 1.2  // Home advantage
```

## Blueprint Quick Start

### Add Combat to Ship
```
1. Add WeaponComponent (multiple)
2. Add TargetingComponent
3. Add CombatHealthComponent
4. (Optional) Add PointDefenseComponent
5. (Optional) Add BoardingComponent
6. Assign WeaponDataAssets
7. Configure component properties
```

### Basic Weapon Control
```blueprint
Event Input Fire
    ├── Get Current Target (from TargetingComponent)
    ├── For Each Weapon in Weapons Array
    │   ├── Can Fire?
    │   └── Fire (Target)
    └── Play Fire Sound
```

### Basic Targeting Control
```blueprint
Event Input Cycle Target
    ├── Select Next Target
    └── Update UI

Event Input Lock Target
    ├── Get Actor Under Crosshair
    └── Select Target (Actor)
```

### Damage Response
```blueprint
Event OnHullDamaged (Damage, Causer)
    ├── Play Damage VFX
    ├── Play Damage Sound
    ├── If Critical Damage
    │   └── Trigger Warning Klaxon
    └── Update UI
```

## Common Issues & Solutions

### Weapon Won't Fire
- ✓ Check `WeaponData` assigned
- ✓ Verify `CanFire()` returns true
- ✓ Check ammunition/power
- ✓ Verify not overheated
- ✓ Check rate of fire cooldown

### No Target Lock
- ✓ Target in `MaxTargetingRange`?
- ✓ Target in `MaxTargetingAngle`?
- ✓ Target has required tags?
- ✓ Target is valid actor?

### Shields Not Recharging
- ✓ Wait for `ShieldRechargeDelay`
- ✓ Not taking damage?
- ✓ `MaxShieldStrength` > 0?
- ✓ Not overloaded?

### Point Defense Inactive
- ✓ Weapons have `bPointDefenseCapable`?
- ✓ Weapons in PD array?
- ✓ Threats have correct tags?
- ✓ PD component active?

### Boarding Fails
- ✓ Target has `BoardingComponent`?
- ✓ Target `bCanBeBoarded` = true?
- ✓ Target not captured?
- ✓ Sufficient crew count?

## Performance Tips

- Set scan intervals: Player 0.1-0.25s, AI 0.5-1.0s
- Limit weapons per ship: 2-8 typical
- Limit PD targets: 4-6 simultaneous
- Use TickGroup settings appropriately
- Disable components when not in combat

## Balance Guidelines

### Weapon Balance
- Energy: Fast, infinite ammo, high heat
- Projectile: Accurate, limited ammo, reload
- Missile: High damage, interceptable, limited

### Ship Balance
- Fighter: 2-4 small weapons
- Corvette: 4-6 small weapons
- Frigate: 2-4 medium weapons
- Cruiser: 4-6 medium weapons
- Battleship: 2-4 large weapons
- Capital: 1-2 capital + many smaller

### Shield vs Hull
- Fast ships: High shields, low hull
- Heavy ships: Low shields, high hull
- Balanced: 40-50% shields of hull

## Keyboard Shortcuts (Example)

| Key | Function |
|-----|----------|
| **Space** | Fire all weapons |
| **T** | Cycle next target |
| **Shift+T** | Cycle previous target |
| **R** | Reload all weapons |
| **F** | Select nearest target |
| **G** | Select highest threat |
| **Tab** | Activate/deactivate point defense |

## Related Files

- `CombatSystemGuide.md` - Full documentation
- `WeaponTemplates.yaml` - Weapon configurations
- `SpaceshipDataAssetGuide.md` - Ship setup
- `AISystemIntegration.md` - AI combat

---

**Quick Help:** For detailed information, see `CombatSystemGuide.md`
