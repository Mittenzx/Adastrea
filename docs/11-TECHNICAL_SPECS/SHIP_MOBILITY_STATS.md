# Ship Mobility & Stat Framework

Design spec for the common ship-stat set that differentiates ship classes.
Extends the existing `USpaceshipDataAsset` "Mobility Stats" schema
(`Source/Adastrea/Public/Ships/SpaceshipDataAsset.h`), which already defines:

```
MaxSpeed, Acceleration, Maneuverability(1-10), RotationRateMultiplier,
JumpRange, FuelCapacity, FuelConsumptionRate, SensorRange, + GetMobilityRating()
```

## Goal

Giving ships **common, comparable stats** so the player can read a ship at a
glance and the different classes feel meaningfully different to fly. Mobility is
the primary differentiator (how a ship *handles*), separate from cargo/combat
(what it *carries*/*does*).

## The Common Stat Set

Every ship has these stats, split into three families.

### 1. MOBILITY (how it moves — the feel)
| Stat | Unit | What it does | Typical range |
|------|------|--------------|---------------|
| TurnRate | deg/s | Max yaw/pitch rotation speed | 20 (freighter) – 90 (fighter) |
| RotationRateMultiplier | 0–1 | Scales base turn rate for class weight | 0.1 (cap) – 1.0 (scout) |
| MaxSpeed | u/s | Cruise top speed | 350 (hauler) – 700 (racer) |
| Acceleration | u/s² | How fast it reaches max speed | 40 – 120 |
| Maneuverability | 1–10 | Abstract agility rating (drives turn feel) | 3 (cargo) – 9 (fighter) |
| Drift/TurnStability | 0–1 | How "locked-in" the turn is vs. skid | 0.5 – 1.0 |

### 2. RANGE / ENDURANCE (how far it goes)
| Stat | Unit | What it does |
|------|------|--------------|
| JumpRange | ly | Max jump distance |
| FuelCapacity | units | Total fuel tank |
| FuelConsumptionRate | units/jump | Fuel cost per jump |
| CruiseEfficiency | 0–1 | Thrust fuel burn modifier (lower = more efficient) |

### 3. ROLE / SURVIVABILITY (what it carries, how it survives)
| Stat | Unit | What it does |
|------|------|--------------|
| CargoCapacity | m³ | Stowage (the truck's reason to exist) |
| HullIntegrity | hp | Base hit points |
| ShieldMax / ShieldRechargeRate | hp / hp·s⁻¹ | Defense |
| SensorRange | units | Detection |

## Mobility as the DIFFERENTIATOR (design principle)

**The key rule:** *nobody is good at everything.* Every class trades one mobility
axis for another. Two concrete patterns from your brief:

- **Heavy cargo hauler:** HIGH MaxSpeed, LOW TurnRate / LOW Acceleration /
  LOW Maneuverability. It flies in a straight line very fast but turns like a
  barge. Moving bulk is its job; dodging isn't.
- **Short-range quick freighter (the "fast courier"):** HIGH MaxSpeed +
  HIGH Acceleration + MEDIUM TurnRate, but SMALL FuelCapacity / SHORT JumpRange /
  LOW CargoCapacity. It's quick and responsive but can't go far on a tank and
  carries little — the "swift, fragile post-runner."

Conversely:
- **Fighter/scout:** HIGH TurnRate + HIGH Acceleration + HIGH Maneuverability,
  LOW–MED MaxSpeed, LOW CargoCapacity. Turns on a dime, accelerates hard, but
  plods in a straight line and carries nothing.
- **Capital/battleship:** HIGH Hull/Shields, LOW TurnRate, LOW Acceleration,
  HIGH range. A fortress that handles like a fortress.

## Proposed Ship-Class Archetypes (target stat blocks)

> Units: speed u/s, accel u/s², turn deg/s, range ly, cargo m³. TurnRate below is
> the *effective* rate (base TurnRate × RotationRateMultiplier).

| Class | TurnRate | MaxSpeed | Accel | Maneuver | JumpRange | FuelCap | Cargo | Hull | Shield | Profile |
|-------|---------|----------|-------|----------|-----------|---------|-------|------|--------|---------|
| **Fighter** (current default) | 90 | 420 | 100 | 9 | 6 | 400 | 10 | 200 | 150 | agile dogfighter, short legs |
| **Scout** (DA_Ship_PlayerScout) | 45 | 500 | 50 | 8 | 10 | 1000 | 50 | 300 | 100 | quick surveyor, balanced |
| **Freighter** (heavy cargo) | 25 | 700 | 40 | 3 | 18 | 2400 | 2000 | 800 | 400 | fast-in-a-line hauler, wallows in turns |
| **Courier** (short-range quick cargo) | 55 | 800 | 110 | 6 | 4 | 300 | 150 | 250 | 150 | swift+responsive, tiny tank, small hold |
| **Corvette / skirmisher** | 60 | 520 | 85 | 7 | 12 | 900 | 120 | 450 | 350 | mobile gunboat, mid everything |
| **Cruiser** | 35 | 480 | 60 | 5 | 20 | 3000 | 900 | 1200 | 900 | multirole, sluggish |
| **Battleship** | 15 | 400 | 30 | 2 | 25 | 5000 | 500 | 4000 | 3000 | fortress; the broken TurnRate=1 BP is this archetype gone wrong |

**Reading a class from the table** — the "mobility triangle":
- High speed + low turn = hauler (goes, doesn't turn).
- High turn + high accel + low speed = fighter (fights, doesn't haul/far).
- High speed + high accel + small range = courier (quick but short-legged).

## Common-Stats Data Shape (machine-readable)

Proposed `Content/Data/ShipClasses.json` (extends the trade/crafting JSON pattern):

```json
{
  "$schema": "https://local/Schemas/ship_class_schema.json",
  "classes": [
    {
      "Class": "Fighter",
      "Mobility": { "TurnRate": 90, "RotationRateMultiplier": 1.0, "MaxSpeed": 420, "Acceleration": 100, "Maneuverability": 9, "TurnStability": 0.9 },
      "Endurance": { "JumpRange": 6, "FuelCapacity": 400, "FuelConsumptionRate": 60, "CruiseEfficiency": 1.0 },
      "Role": { "CargoCapacity": 10, "HullIntegrity": 200, "ShieldMax": 150, "ShieldRechargeRate": 20, "SensorRange": 4000 }
    }
  ]
}
```

## Next Steps (not done here — data-owner choice)

1. Decide whether this doc feeds the **crafting agent** (ship stats could tie
   into its ship-part crafting chain) or stays a **main-side** data asset.
2. Create `Content/Data/ShipClasses.json` + per-class `USpaceshipDataAsset` BPs.
3. If a HUD readout is wanted, expose TurnRate/Accel/MaxSpeed on the telemetry panel.
4. Fix the known anomaly: **BP_Battleship's TurnRate=1** (matches this doc's
   Battleship archetype at 15 — the value is just broken in the asset, not the pattern).