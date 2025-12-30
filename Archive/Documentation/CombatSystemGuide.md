# Combat System Guide

## Overview

The Adastrea Combat System provides a comprehensive framework for space combat including weapons, targeting, damage calculation, shields, point defense, and boarding mechanics. All systems are designed to be designer-friendly and fully Blueprint-accessible.

## Table of Contents

- [System Architecture](#system-architecture)
- [Weapons](#weapons)
- [Targeting](#targeting)
- [Damage and Shields](#damage-and-shields)
- [Point Defense](#point-defense)
- [Boarding Mechanics](#boarding-mechanics)
- [Integration](#integration)
- [Blueprint Usage](#blueprint-usage)
- [Best Practices](#best-practices)

## System Architecture

The combat system consists of six primary components:

1. **WeaponDataAsset** - Defines weapon properties and statistics
2. **WeaponComponent** - Manages weapon mounting, firing, and ammunition
3. **TargetingComponent** - Handles target acquisition, lock-on, and lead calculation
4. **CombatHealthComponent** - Manages hull integrity, shields, armor, and damage
5. **PointDefenseComponent** - Automated defense against missiles and small craft
6. **BoardingComponent** - Crew-based combat and ship capture mechanics

### Component Relationships

```
Spaceship Actor
├── WeaponComponent(s) ──┐
├── TargetingComponent ───┼── Combat System
├── CombatHealthComponent ┤
├── PointDefenseComponent ┤
└── BoardingComponent ────┘
```

## Weapons

### Weapon Types

The system supports six weapon types:

- **Energy Weapons** - Laser, particle beams (no ammunition)
- **Projectile Weapons** - Railguns, autocannons (uses ammunition)
- **Missile Launchers** - Guided missiles (homing capability)
- **Torpedo Launchers** - Heavy anti-ship weapons
- **Beam Weapons** - Continuous energy beams
- **Plasma Weapons** - Superheated plasma bolts

### Damage Types

Five damage types with different penetration characteristics:

- **Kinetic** - Physical impact damage
- **Energy** - Laser and particle damage
- **Explosive** - Area of effect damage
- **Thermal** - Heat-based damage
- **EMP** - Electromagnetic pulse (disables systems)

### Creating a Weapon Data Asset

1. In Content Browser, right-click → Blueprint → WeaponDataAsset
2. Configure basic properties:
   - Weapon Name and Description
   - Weapon Type and Damage Type
   - Mount Size (Small/Medium/Large/Capital)

3. Set damage stats:
   - Base Damage
   - Armor Penetration (0-100)
   - Shield Penetration (0-100)
   - Critical Chance and Multiplier

4. Configure firing characteristics:
   - Rate of Fire (shots per second)
   - Max Range and Optimal Range
   - Projectile Speed
   - Tracking Speed (for turrets)
   - Accuracy (0-100)

5. Set resource requirements:
   - Power Per Shot
   - Ammo Capacity (0 for energy weapons)
   - Reload Time
   - Heat Generation and Cooling Rate

6. Add special properties (if applicable):
   - Point Defense Capable (yes/no)
   - AOE Radius
   - EMP Strength
   - Homing Rating (for missiles)

### Example Weapon Configurations

**Pulse Laser (Small Energy Weapon)**
```
Base Damage: 100
Rate of Fire: 5.0
Max Range: 3000m
Power Per Shot: 25
Armor Penetration: 40
Shield Penetration: 70
```

**Railgun (Medium Projectile)**
```
Base Damage: 500
Rate of Fire: 0.5
Max Range: 8000m
Ammo Capacity: 100
Armor Penetration: 90
Shield Penetration: 30
```

**Point Defense Turret (Small)**
```
Base Damage: 50
Rate of Fire: 10.0
Max Range: 2000m
Point Defense Capable: Yes
Tracking Speed: 180°/s
```

## Targeting

### Target Selection

The TargetingComponent provides multiple target selection methods:

- **Manual Selection** - Select specific target
- **Cycle Targets** - Next/Previous in detected list
- **Nearest Target** - Closest enemy
- **Highest Threat** - Most dangerous target

### Target Lock-On

Once a target is selected:
- System tracks target position and velocity
- Calculates lead position for moving targets
- Monitors target range and arc
- Auto-retargets if current target is lost

### Lead Calculation

For projectile weapons, the targeting system calculates where to aim:

```cpp
// Automatic lead calculation based on projectile speed
FVector LeadPosition = TargetingComponent->CalculateLeadPosition(ProjectileSpeed);
```

### Configuration Options

- **Max Targeting Range** - Detection radius
- **Max Targeting Angle** - Forward arc for targeting
- **Scan Interval** - How often to search for targets
- **Auto Retarget** - Automatically select new target when current is lost
- **Prioritize By Threat** - Sort targets by danger level

## Damage and Shields

### Hull Integrity

Ships have hull strength (health) that depletes when taking damage:
- Armor reduces incoming damage
- Critical damage threshold at 25% hull
- Ship destroyed at 0% hull

### Shield System

Two shield modes available:

#### Omnidirectional Shields
- Single shield pool protecting entire ship
- Simpler configuration
- Equal protection from all angles

#### Directional Shields
- Four facings: Forward, Aft, Port, Starboard
- Configurable strength per facing (percentage distribution)
- Damage only affects specific facing
- Independent recharge per facing

### Shield Mechanics

- **Recharge Rate** - Shield points restored per second
- **Recharge Delay** - Time after damage before recharging starts
- **Shield Overload** - Shields go offline when fully depleted
- **Penetration** - Some weapons bypass shields partially

### Damage Calculation

Damage flow:
1. Check if shields are active
2. Apply shield penetration (some damage bypasses shields)
3. Shield absorbs damage based on penetration rating
4. Remaining damage applies to hull
5. Armor reduces hull damage based on penetration

Formula:
```
Effective Shield Damage = Damage × (1 - Shield Penetration / 100)
Effective Armor Damage = Damage × (1 / (1 + Effective Armor / 100))
```

### Damage Events

Components fire Blueprint events for:
- Hull damaged
- Shield damaged
- Shields depleted
- Shields recharged
- Critical damage
- Ship destroyed

## Point Defense

### Overview

Point Defense automatically engages incoming missiles, torpedoes, and small fighters to protect the ship.

### Configuration

1. Add PointDefenseComponent to ship
2. Assign point defense capable weapons
3. Configure detection range
4. Set threat tags (Missile, Torpedo, Fighter, etc.)
5. Enable auto-engage

### Threat Prioritization

System prioritizes targets by:
- Distance (closer = higher priority)
- Type (missiles prioritized over fighters)
- Time to impact
- Current engagement status

### Weapon Assignment

- Automatically assigns available PD weapons to threats
- Respects max simultaneous target limit
- Redistributes weapons as threats are neutralized
- Fires automatically at assigned targets

### Interception Probability

Success chance factors:
- Distance to threat
- Number of weapons engaging
- Weapon accuracy and tracking speed
- Threat velocity and evasion

## Boarding Mechanics

### Overview

Ships can board enemy vessels to capture them through crew combat. Boarding involves approaching, breaching, fighting, and securing the ship.

### Boarding Phases

1. **Approaching** - Boarding craft moves to target (10s)
2. **Breaching** - Cutting through hull (configurable, default 30s)
3. **Combat** - Crew battles (resolved in intervals)
4. **Securing** - Taking control after combat (20s)
5. **Complete** - Ship captured

### Boarding Party Stats

- **Crew Count** - Number of boarding crew
- **Skill Level** - Combat effectiveness (0-100)
- **Equipment Quality** - Weapons and armor (0-100)
- **Morale** - Fighting spirit (0-100)

### Combat Resolution

Every combat interval (default 5s):
- Calculate attacker strength (crew × skill × equipment × morale)
- Calculate defender strength (same, with home advantage bonus)
- Resolve casualties for both sides
- Update morale based on casualties
- Check for victory or defeat

### Victory Conditions

**Attackers Win:**
- Defender crew eliminated or routed (morale < 10%)
- Ship is captured

**Defenders Win:**
- Attacker crew eliminated or routed
- Boarding action fails

### Ship Capture

When captured:
- Ownership transfers to attacker
- Original crew becomes prisoners
- Ship can be recaptured by friendly forces

## Integration

### With Spaceship Data Asset

Combat components integrate with SpaceshipDataAsset:

```cpp
// In ship Blueprint BeginPlay
CombatHealthComponent->MaxHullStrength = SpaceshipData->HullStrength;
CombatHealthComponent->MaxShieldStrength = SpaceshipData->ShieldStrength;
CombatHealthComponent->ShieldRechargeRate = SpaceshipData->ShieldRechargeRate;
CombatHealthComponent->ArmorRating = SpaceshipData->ArmorRating;
BoardingComponent->MaxBoardingCrew = SpaceshipData->BoardingCapability * 5;
```

### With AI Systems

Combat components work with AI:

```cpp
// In AI Logic
if (TargetingComponent->GetDetectedTargetCount() > 0)
{
    AActor* Target = TargetingComponent->SelectHighestThreatTarget();
    
    for (UWeaponComponent* Weapon : Weapons)
    {
        if (Weapon->CanFire() && Weapon->IsTargetInRange(Target))
        {
            Weapon->Fire(Target);
        }
    }
}
```

### With Player Input

Example player fire control:

```cpp
// In player input handler
void APlayerShip::FireWeapons()
{
    AActor* Target = TargetingComponent->CurrentTarget;
    
    for (UWeaponComponent* Weapon : Weapons)
    {
        Weapon->Fire(Target);
    }
}

void APlayerShip::CycleTarget()
{
    TargetingComponent->SelectNextTarget();
}
```

## Blueprint Usage

### Adding Combat to a Ship

1. Create ship Blueprint (or open existing)
2. Add components:
   - Add Component → Combat → WeaponComponent (add multiple)
   - Add Component → Combat → TargetingComponent
   - Add Component → Combat → CombatHealthComponent
   - Add Component → Combat → PointDefenseComponent (optional)
   - Add Component → Combat → BoardingComponent (optional)

3. Configure components in Details panel

4. Assign WeaponDataAssets to weapon components

5. For point defense:
   - Enable "Point Defense Capable" on some weapons
   - Add those weapons to PointDefenseComponent's weapon array

### Blueprint Event Examples

**Respond to Damage:**
```
Event OnHullDamaged
├── Print String ("Hull: {HullPercent}%")
├── Play Sound (DamageSound)
└── Spawn Particle Effect (DamageVFX)
```

**UI Update on Target Change:**
```
Event OnTargetLocked
├── Update UI Widget (Show Target Info)
├── Play Sound (LockSound)
└── Enable Fire Reticle
```

**Boarding Progress Display:**
```
Event Tick
└── If Boarding Active
    ├── Get Boarding Progress
    └── Update Progress Bar
```

### Custom Combat Logic

Override BlueprintNativeEvents for custom behavior:

**Custom Threat Calculation:**
```
Function CalculateThreatLevel (Target)
├── Get Base Threat
├── Check if Target Targeting Us (+30)
├── Check Target Weapon Power (+20)
└── Return Total Threat
```

**Custom Weapon Fire Effects:**
```
Function OnWeaponFired (Target)
├── Spawn Muzzle Flash
├── Play Fire Sound
├── Spawn Projectile (if visible)
└── Camera Shake
```

## Best Practices

### Performance

1. **Limit Scan Frequency**
   - Use longer scan intervals for AI ships (0.5-1.0s)
   - Shorter for player ship (0.1-0.25s) for responsiveness

2. **Weapon Component Count**
   - Reasonable weapon limits per ship (2-8 typical)
   - Larger ships can have more without issues

3. **Point Defense Targets**
   - Limit max simultaneous targets (4-6 typical)
   - Higher for capital ships with many PD weapons

### Balance

1. **Weapon Balance**
   - Energy weapons: High shield damage, low armor penetration
   - Projectile weapons: High armor damage, low shield penetration
   - Missiles: High damage, vulnerable to point defense

2. **Shield vs Hull**
   - Shields should be 30-50% of hull strength typically
   - Fast recharge (10-50 pts/sec) makes shields effective
   - Long recharge delay (5-10s) creates tactical decisions

3. **Boarding**
   - Boarding should be high-risk, high-reward
   - Requires ship to be disabled/weakened first
   - Defenders have natural advantage (1.2x strength)

### Design Guidelines

1. **Weapon Diversity**
   - Give players weapon choices (energy vs projectile)
   - Different ranges (close, medium, long)
   - Different roles (anti-ship, point defense, boarding support)

2. **Targeting Feedback**
   - Clear UI for current target
   - Lead indicators for projectile weapons
   - Threat level visualization

3. **Damage Feedback**
   - Visual effects for hull and shield damage
   - Audio cues for critical damage
   - UI updates for health/shield levels

4. **Boarding Engagement**
   - Show boarding progress clearly
   - Display casualties and morale
   - Provide abort option early in boarding

### Debugging

Enable logging for combat systems:

```
LogAdastrea: Log - Shows normal combat operations
LogAdastrea: Warning - Shows critical events (shield depleted, etc.)
LogAdastrea: Error - Shows failures (ship destroyed, etc.)
```

Check console output for:
- Weapon fire events
- Target lock/loss
- Damage application
- Point defense engagements
- Boarding phase transitions

## Advanced Topics

### Custom Damage Types

Extend EDamageType in WeaponDataAsset.h for new damage types. Update damage calculation logic in CombatHealthComponent to handle new types.

### Shield Modulation

Implement dynamic shield tuning by adjusting penetration resistance:
- High against energy, low against kinetic
- Can be changed mid-combat in Blueprint

### Electronic Warfare

Extend TargetingComponent to support:
- Jamming (reduce lock accuracy)
- Decoys (false targets)
- Sensor dampening (reduce detection range)

### Multi-Ship Battles

For fleet combat:
- Assign FireControls to different target priorities
- Coordinate point defense between ships
- Share targeting data across fleet

### Subsystem Targeting

Extend damage system to support:
- Critical hit locations (engines, weapons, bridge)
- System disabling (weapons offline, mobility reduced)
- Repair mechanics

## Troubleshooting

**Weapons Not Firing:**
- Check WeaponData is assigned
- Verify CanFire() returns true
- Check power/ammo availability
- Verify rate of fire timing

**Target Lock Fails:**
- Check target in MaxTargetingRange
- Verify target in MaxTargetingAngle
- Confirm target has valid tags (if tag filtering enabled)

**Shields Not Recharging:**
- Wait for recharge delay to pass
- Check not taking damage
- Verify MaxShieldStrength > 0
- Check not overloaded

**Point Defense Not Engaging:**
- Verify weapons have bPointDefenseCapable = true
- Check weapons added to PD weapon array
- Confirm targets have correct tags
- Check threat in DetectionRange

**Boarding Fails Immediately:**
- Check target has BoardingComponent
- Verify target bCanBeBoarded = true
- Ensure target not already captured
- Confirm sufficient boarding crew

## Related Documentation

- `SpaceshipDataAssetGuide.md` - Ship configuration
- `AISystemIntegration.md` - AI combat behaviors
- `PersonnelSystemGuide.md` - Crew management for boarding
- `CombatWeaponTemplates.yaml` - Weapon configuration examples

---

**Version:** 1.0  
**Last Updated:** 2025-11-11  
**Maintained by:** Adastrea Development Team
