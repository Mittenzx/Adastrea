# Spaceship Templates - Quick Reference Guide

This document provides a quick reference for all available spaceship templates. Use these as starting points for creating your own ship variants.

---

## Template Overview

| Template | Role | Size | Crew | Combat | Mobility | Utility | Rarity |
|----------|------|------|------|--------|----------|---------|--------|
| Mittenzx Mk1 | Multi-Role | Corvette | 2-4 | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐ | Uncommon |
| Pathfinder Scout | Reconnaissance | Corvette | 2-4 | ⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | Uncommon |
| Warhammer Gunship | Combat | Frigate | 15-25 | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐ | Rare |
| Merchant Trader | Cargo Transport | Cruiser | 8-15 | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ | Common |
| Starliner Luxury | Passenger Transport | Cruiser | 40-200 | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | Legendary |
| Excavator Mining | Resource Extraction | Battleship | 25-50 | ⭐⭐ | ⭐ | ⭐⭐⭐⭐ | Uncommon |
| Sovereign Command | Fleet Flagship | Capital | 200-500 | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐⭐ | Legendary |

---

## 1. Mittenzx Mk1

**File:** `MultiRole_MittenzxMk1.yaml`

### Role & Purpose
Versatile multi-role spacecraft with balanced capabilities for various mission types.

### Key Strengths
- ✅ Balanced stats across all categories
- ✅ Good cargo capacity (75 tons)
- ✅ Dual weapon slots for versatility
- ✅ Excellent customization options (10 modular points)
- ✅ Includes small hangar bay

### Key Weaknesses
- ❌ Jack-of-all-trades, master of none
- ❌ Moderate speed (slower than dedicated scouts)
- ❌ Average sensor range
- ❌ Not specialized for any single role

### Stat Highlights
```yaml
HullStrength: 1000
MaxSpeed: 750
Maneuverability: 7
JumpRange: 22.0
SensorRange: 12000
WeaponSlots: 2
CargoCapacity: 75
ModularPoints: 10
```

### Best Used For
- Independent pilot operations
- Multi-mission contracts
- Small trade runs
- Light exploration
- Self-defense scenarios
- Learning game mechanics

### Customization Ideas
- **Combat Focus**: Add +2 weapon slots, increase armor to 50
- **Trade Variant**: Increase cargo to 150, reduce weapons to 1
- **Explorer**: Increase sensors to 18,000, add +3 to science rating
- **Fast Courier**: Increase speed to 850, reduce cargo to 50

---

## 2. Pathfinder Scout

**File:** `Scout_Pathfinder.yaml`

### Role & Purpose
Fast reconnaissance vessel for exploration and intelligence gathering.

### Key Strengths
- ✅ Exceptional sensor range (25,000 units)
- ✅ High speed and maneuverability
- ✅ Excellent jump range (25 LY)
- ✅ Good stealth capabilities
- ✅ Efficient fuel consumption

### Key Weaknesses
- ❌ Minimal combat capability
- ❌ Light armor and shields
- ❌ Small cargo capacity
- ❌ Limited crew capacity

### Stat Highlights
```yaml
HullStrength: 800
MaxSpeed: 900
Maneuverability: 9
JumpRange: 25.0
SensorRange: 25000
StealthRating: 8
WeaponSlots: 1
CargoCapacity: 50
```

### Best Used For
- Deep space exploration
- Fleet reconnaissance
- Covert operations
- Survey missions
- Early warning systems

### Customization Ideas
- **Mk II**: Increase sensors to 30,000, add +2 stealth
- **Combat Variant**: Add +2 weapon slots, reduce cargo to 30
- **Long Range**: Increase jump range to 30 LY, add +500 fuel
- **Sensor Platform**: Max sensors at 35,000, reduce speed to 800

---

## 2. Warhammer Gunship

**File:** `Gunship_Warhammer.yaml`

### Role & Purpose
Heavy assault vessel designed for combat superiority.

### Key Strengths
- ✅ Massive firepower (8 weapon slots)
- ✅ Heavy armor and shields
- ✅ Excellent point defense
- ✅ Strong boarding capability
- ✅ Good repair systems

### Key Weaknesses
- ❌ Slow speed and acceleration
- ❌ Poor maneuverability
- ❌ Limited cargo space
- ❌ Poor stealth signature
- ❌ High fuel consumption

### Stat Highlights
```yaml
HullStrength: 5000
ArmorRating: 250
ShieldStrength: 3000
WeaponSlots: 8
WeaponPowerCapacity: 8000
PointDefenseRating: 9.0
MaxSpeed: 400
BoardingCapability: 8
```

### Best Used For
- Fleet combat operations
- Station assaults
- Convoy escort (combat)
- Anti-capital ship roles
- Mercenary operations

### Customization Ideas
- **Interceptor**: Add +200 speed, remove 2 weapon slots
- **Assault**: Increase boarding to 10, add +500 hull
- **Defender**: Increase point defense to 10, add shields
- **Heavy**: Add +2 weapon slots, reduce speed to 350

---

## 3. Merchant Trader

**File:** `Trading_Merchant.yaml`

### Role & Purpose
Civilian cargo hauler for commercial trade routes.

### Key Strengths
- ✅ Massive cargo capacity (5,000 tons)
- ✅ Good jump range
- ✅ Efficient fuel consumption
- ✅ Excellent life support
- ✅ Advanced trade AI
- ✅ High customizable slots

### Key Weaknesses
- ❌ Minimal weapons
- ❌ Light armor
- ❌ Poor stealth
- ❌ Limited combat capability

### Stat Highlights
```yaml
CargoCapacity: 5000
JumpRange: 18.0
FuelCapacity: 4000
MaxSpeed: 550
DiplomacyRating: 6
CustomizableSlots: 8
WeaponSlots: 2
```

### Best Used For
- Commercial shipping
- Long-haul cargo transport
- Trade route operations
- Supply missions
- Civilian transport

### Customization Ideas
- **Bulk Hauler**: Increase cargo to 7,000, reduce speed to 500
- **Fast Courier**: Reduce cargo to 3,000, increase speed to 700
- **Armed Merchant**: Add +2 weapon slots, +500 shields
- **Luxury Cargo**: Add passenger capacity, increase life support

---

## 4. Starliner Luxury

**File:** `Luxury_Starliner.yaml`

### Role & Purpose
Premium passenger vessel for VIP transport and diplomatic missions.

### Key Strengths
- ✅ Exceptional crew/passenger capacity (200 max)
- ✅ Maximum diplomacy rating (10)
- ✅ Excellent medical facilities
- ✅ Outstanding life support (10)
- ✅ Strong defensive shields
- ✅ Advanced AI systems

### Key Weaknesses
- ❌ Limited combat capability
- ❌ Expensive to maintain
- ❌ Average stealth
- ❌ No specialized functions (mining, etc.)

### Stat Highlights
```yaml
MaxCrew: 200
DiplomacyRating: 10
MedicalRating: 9
LifeSupportRating: 10
ShieldStrength: 2000
JumpRange: 20.0
MaintenanceLevel: 6
```

### Best Used For
- Diplomatic missions
- VIP transport
- Luxury cruises
- Mobile embassies
- High-society events
- Peace negotiations

### Customization Ideas
- **Embassy Ship**: Max diplomacy, add +100 crew capacity
- **Medical Cruiser**: Increase medical to 10, add science labs
- **Executive**: Reduce crew to 50, add +20% speed
- **Conference**: Add command facilities, increase power

---

## 5. Excavator Mining

**File:** `Mining_Excavator.yaml`

### Role & Purpose
Industrial mining vessel for asteroid and resource extraction.

### Key Strengths
- ✅ Maximum mining capability (10)
- ✅ Enormous cargo capacity (8,000 tons)
- ✅ Extensive drone complement (50)
- ✅ Excellent repair systems
- ✅ High-resolution sensors for ore detection
- ✅ Multiple hangar bays

### Key Weaknesses
- ❌ Very slow speed
- ❌ Poor maneuverability
- ❌ Terrible stealth
- ❌ High maintenance requirements
- ❌ Heavy fuel consumption

### Stat Highlights
```yaml
MiningRating: 10
CargoCapacity: 8000
DroneCapacity: 50
HangarCapacity: 4
RepairSystemRating: 8
SensorResolution: 8
MaxSpeed: 350
MaintenanceLevel: 8
```

### Best Used For
- Asteroid mining
- Resource extraction
- Ore processing
- Deep space mining operations
- Industrial operations

### Customization Ideas
- **Heavy Miner**: Increase cargo to 10,000, add +20 drones
- **Survey Ship**: Increase sensors, add science rating
- **Refinery**: Add processing capability, reduce cargo
- **Salvage**: Add boarding, increase repair systems

---

## 6. Sovereign Command

**File:** `Command_Sovereign.yaml`

### Role & Purpose
Capital flagship for fleet command and control operations.

### Key Strengths
- ✅ Massive hull strength (15,000)
- ✅ Capital ship weapons and defenses
- ✅ Full carrier operations (20 fighters)
- ✅ Exceptional sensors and EWar
- ✅ Complete diplomatic facilities
- ✅ Maximum AI and tactical systems
- ✅ Hospital-level medical facilities

### Key Weaknesses
- ❌ Very slow and ponderous
- ❌ Extremely high maintenance
- ❌ Poor stealth (massive signature)
- ❌ Heavy fuel consumption
- ❌ Requires large crew (200+)

### Stat Highlights
```yaml
HullStrength: 15000
WeaponSlots: 15
WeaponPowerCapacity: 15000
HangarCapacity: 20
DroneCapacity: 100
SensorRange: 30000
EWarRating: 10
AISystemRating: 10
DiplomacyRating: 9
```

### Best Used For
- Fleet flagship
- Mobile command center
- Combined arms operations
- Diplomatic summits
- System defense
- Major military campaigns

### Customization Ideas
- **Carrier Variant**: Increase hangar to 30, reduce weapons to 10
- **Battleship**: Max weapons to 20, reduce hangar to 10
- **Command Cruiser**: Reduce size 30%, maintain command systems
- **Diplomatic**: Increase diplomacy to 10, add luxury facilities

---

## Ship Size Comparison

### Size Categories (Hull Strength)
- **Fighter**: < 1,500 HP
- **Corvette**: 1,500 - 3,000 HP
- **Frigate**: 3,000 - 7,000 HP
- **Cruiser**: 7,000 - 15,000 HP
- **Battleship**: 15,000 - 30,000 HP
- **Capital Ship**: 30,000+ HP

### Template Classifications
```
Fighter     Corvette    Frigate     Cruiser     Battleship  Capital
   |           |           |           |            |          |
           Pathfinder  Warhammer  Merchant                Sovereign
                                  Starliner
                                              Excavator
```

---

## Combat Effectiveness Comparison

```
                Combat Rating (0-100)
Warhammer       ████████████████████ 90+
Sovereign       ███████████████████  85+
Pathfinder      █████                25
Excavator       ██████               30
Merchant        ████                 20
Starliner       █████                25
```

---

## Mobility Comparison

```
                Mobility Rating (0-100)
Pathfinder      ████████████████████ 95+
Starliner       ████████████         60
Merchant        ███████████          55
Warhammer       ████████             40
Sovereign       ███████              35
Excavator       ████                 20
```

---

## Utility Comparison

```
                Utility Rating (0-100)
Sovereign       ████████████████████ 95+
Pathfinder      ██████████████████   90
Excavator       ███████████████      75
Starliner       ██████████████       70
Merchant        ████████             40
Warhammer       ████                 20
```

---

## Role Suitability Matrix

| Template | Combat | Trade | Exploration | Mining | Passenger | Command |
|----------|--------|-------|-------------|--------|-----------|---------|
| Pathfinder | ❌ | ❌ | ✅✅✅ | ❌ | ❌ | ❌ |
| Warhammer | ✅✅✅ | ❌ | ❌ | ❌ | ❌ | ⚠️ |
| Merchant | ❌ | ✅✅✅ | ⚠️ | ⚠️ | ⚠️ | ❌ |
| Starliner | ❌ | ❌ | ⚠️ | ❌ | ✅✅✅ | ✅✅ |
| Excavator | ❌ | ⚠️ | ❌ | ✅✅✅ | ❌ | ❌ |
| Sovereign | ✅✅✅ | ❌ | ⚠️ | ❌ | ⚠️ | ✅✅✅ |

Legend: ✅✅✅ = Excellent | ✅✅ = Good | ⚠️ = Can do but not optimal | ❌ = Not suitable

---

## Operational Cost Comparison

Daily operational costs (estimated):

```
Sovereign:   $5,500/day    ████████████████████
Excavator:   $2,200/day    █████████
Warhammer:   $1,900/day    ████████
Starliner:   $1,800/day    ███████
Merchant:    $  800/day    ███
Pathfinder:  $  180/day    █
```

---

## Quick Selection Guide

### "I need a ship for..."

**Exploring unknown systems**
→ Pathfinder Scout (sensors, range, speed)

**Front-line combat**
→ Warhammer Gunship (firepower, armor)

**Hauling cargo for profit**
→ Merchant Trader (capacity, efficiency)

**Transporting VIPs safely**
→ Starliner Luxury (comfort, diplomacy, defense)

**Mining asteroids for resources**
→ Excavator Mining (mining tools, cargo, drones)

**Leading a fleet in battle**
→ Sovereign Command (weapons, hangar, sensors, command)

**Covert operations**
→ Pathfinder Scout (stealth, speed) or modify with enhanced stealth

**Emergency medical response**
→ Starliner Luxury (medical 9) or create medical variant

**Long-range patrol**
→ Pathfinder Scout (range, sensors, endurance)

**Station assault**
→ Warhammer Gunship (firepower, boarding) or Sovereign (overwhelming force)

---

## Creating Variants

### Quick Variant Formula:
1. Start with the template closest to your goal
2. Identify 2-3 stats to change
3. Maintain the ship's core identity
4. Adjust rarity if significantly better
5. Update lore notes to reflect changes

### Example Variants:

**From Pathfinder:**
- **Pathfinder Mk II**: +20% sensors, +2 stealth → Rare
- **Pathfinder Combat**: +2 weapons, -20 cargo → Uncommon
- **Pathfinder Survey**: +science 8, -speed 100 → Uncommon

**From Warhammer:**
- **Warhammer Heavy**: +2 weapons, -50 speed → Very Rare
- **Warhammer Escort**: +2 maneuver, -20% hull → Rare
- **Warhammer Command**: +diplomacy 5, +crew 50 → Rare

**From Merchant:**
- **Merchant Express**: +200 speed, -2000 cargo → Uncommon
- **Merchant Bulk**: +3000 cargo, -100 speed → Common
- **Merchant Armed**: +4 weapons, +1000 shields → Uncommon

---

## Conclusion

These templates provide a solid foundation for creating diverse ship types. Mix and match stats, create variants, and balance against gameplay needs to build the perfect fleet for your game.

For detailed creation instructions, see `SpaceshipDataAssetGuide.md`.
For workflow guidance, see `SpaceshipDesignerWorkflow.md`.
