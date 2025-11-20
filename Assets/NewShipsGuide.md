# New Ship Templates - 10 Additional Vessels

This document describes the 10 new ships added to expand the Adastrea fleet roster. These ships fill gameplay niches and provide diverse options for different playstyles.

---

## Quick Reference Table

| Ship Name | Role | Size | Crew | Combat | Mobility | Utility | Rarity |
|-----------|------|------|------|--------|----------|---------|--------|
| Viper Interceptor | Dogfighter | Fighter | 1-2 | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐ | Uncommon |
| Shadowblade Stealth | Covert Ops | Frigate | 12-18 | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | Rare |
| Lifeline Medical | Hospital Ship | Cruiser | 35-120 | ⭐ | ⭐⭐ | ⭐⭐⭐⭐⭐ | Rare |
| Odyssey Research | Science Ship | Cruiser | 20-45 | ⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | Uncommon |
| Behemoth Freighter | Bulk Cargo | Transport | 18-30 | ⭐ | ⭐ | ⭐⭐ | Common |
| Phoenix Salvage | Salvage Ops | Utility | 10-20 | ⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐ | Uncommon |
| Vanguard Escort | Small Carrier | Carrier | 40-85 | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | Rare |
| Raptor Assault | Boarding Ship | Corvette | 15-40 | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐ | Uncommon |
| Sentinel Patrol | Law Enforcement | Patrol | 8-16 | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | Common |
| Genesis Colony | Colonization | Capital | 80-3000 | ⭐⭐ | ⭐ | ⭐⭐⭐⭐⭐ | Legendary |

---

## 1. Viper Interceptor

**File:** `Fighter_ViperInterceptor.yaml`

### Role & Purpose
The ultimate dogfighter - trades protection for unmatched speed and agility.

### Key Strengths
- ✅ Highest speed (1200 m/s) and maneuverability (10/10)
- ✅ Excellent acceleration for hit-and-run tactics
- ✅ Four weapon slots for flexible loadouts
- ✅ Strong point defense (8/10)
- ✅ Fast shield recharge

### Key Weaknesses
- ❌ Very low hull strength and armor
- ❌ Minimal cargo capacity
- ❌ Single-pilot operation (no redundancy)
- ❌ Short jump range
- ❌ No utility systems

### Stat Highlights
```yaml
HullStrength: 600
MaxSpeed: 1200
Maneuverability: 10
WeaponSlots: 4
StealthRating: 6
CrewRequired: 1
```

### Best Used For
- Fighter superiority missions
- Intercepting fast targets
- Escort duty
- Quick strike raids
- Racing and competitions

---

## 2. Shadowblade Stealth Frigate

**File:** `Frigate_ShadowbladeStealth.yaml`

### Role & Purpose
Advanced stealth vessel for covert operations and intelligence gathering.

### Key Strengths
- ✅ Maximum stealth rating (10/10)
- ✅ Top-tier electronic warfare (10/10)
- ✅ Excellent sensors (20,000 range)
- ✅ Advanced AI systems
- ✅ Good boarding capability

### Key Weaknesses
- ❌ High maintenance requirements
- ❌ Expensive to operate
- ❌ Limited weapon slots
- ❌ Slow compared to dedicated combat ships

### Stat Highlights
```yaml
StealthRating: 10
EWarRating: 10
SensorRange: 20000
HullStrength: 2800
CrewRequired: 12
```

### Best Used For
- Espionage missions
- Covert infiltration
- Electronic warfare
- Special operations
- Intelligence gathering

---

## 3. Lifeline Medical Cruiser

**File:** `Cruiser_LifelineMedical.yaml`

### Role & Purpose
Mobile hospital providing critical medical care in crisis zones.

### Key Strengths
- ✅ Maximum medical rating (10/10)
- ✅ Maximum life support (10/10)
- ✅ High science rating (9/10)
- ✅ Excellent diplomacy (9/10)
- ✅ Protected non-combatant status

### Key Weaknesses
- ❌ Minimal combat capability
- ❌ Slow and unmaneuverable
- ❌ Large crew requirement
- ❌ Non-threatening means vulnerable

### Stat Highlights
```yaml
MedicalRating: 10
LifeSupportRating: 10
ScienceRating: 9
DiplomacyRating: 9
MaxCrew: 120
```

### Best Used For
- Battlefield medical support
- Disaster relief
- Humanitarian missions
- Research hospitals
- Diplomatic missions

---

## 4. Odyssey Research Vessel

**File:** `Science_Odyssey.yaml`

### Role & Purpose
Premier scientific exploration ship for long-duration research missions.

### Key Strengths
- ✅ Maximum science rating (10/10)
- ✅ Exceptional sensors (35,000 range, 10/10 resolution)
- ✅ Longest jump range (30 LY)
- ✅ Large drone capacity (25)
- ✅ Extended fuel capacity

### Key Weaknesses
- ❌ Minimal combat capability
- ❌ Limited weapons (1 slot)
- ❌ Poor stealth signature
- ❌ Moderate speed

### Stat Highlights
```yaml
ScienceRating: 10
SensorRange: 35000
JumpRange: 30.0
DroneCapacity: 25
MedicalRating: 7
```

### Best Used For
- Deep space exploration
- Planetary surveys
- Scientific research
- Xenobiological studies
- Astronomical observations

---

## 5. Behemoth Heavy Freighter

**File:** `Transport_BehemothFreighter.yaml`

### Role & Purpose
Massive bulk cargo hauler for large-scale commercial shipping.

### Key Strengths
- ✅ Enormous cargo capacity (20,000 tons)
- ✅ Very high hull strength
- ✅ Large fuel capacity
- ✅ Good repair systems
- ✅ Common and affordable

### Key Weaknesses
- ❌ Extremely slow (350 m/s)
- ❌ Terrible maneuverability (2/10)
- ❌ Minimal sensors
- ❌ Large, easy target
- ❌ Poor stealth

### Stat Highlights
```yaml
CargoCapacity: 20000
HullStrength: 8000
FuelCapacity: 12000
MaxSpeed: 350
Maneuverability: 2
```

### Best Used For
- Bulk cargo transport
- Mining operations
- Station supply runs
- Long-haul freight
- Resource gathering

---

## 6. Phoenix Salvage Ship

**File:** `Utility_PhoenixSalvage.yaml`

### Role & Purpose
Specialized vessel for recovering wreckage and valuable materials.

### Key Strengths
- ✅ Maximum repair rating (10/10)
- ✅ Large cargo for salvaged goods
- ✅ Good sensors for wreck detection
- ✅ Mining capability (6/10)
- ✅ Strong power generation

### Key Weaknesses
- ❌ Moderate combat ability
- ❌ Not particularly fast
- ❌ Dangerous work environment
- ❌ Requires skilled crew

### Stat Highlights
```yaml
RepairSystemRating: 10
CargoCapacity: 2500
MiningRating: 6
PowerCapacity: 8500
DroneCapacity: 20
```

### Best Used For
- Salvage operations
- Wreck recovery
- Battlefield cleanup
- Material reclamation
- Emergency repairs

---

## 7. Vanguard Escort Carrier

**File:** `Carrier_VanguardEscort.yaml`

### Role & Purpose
Compact carrier projecting fighter/drone power across the battlefield.

### Key Strengths
- ✅ Large hangar capacity (12)
- ✅ Massive drone capacity (40)
- ✅ Advanced AI systems
- ✅ Strong point defense
- ✅ Good sensors

### Key Weaknesses
- ❌ Moderate direct firepower
- ❌ Large crew requirement
- ❌ High maintenance
- ❌ Expensive to operate

### Stat Highlights
```yaml
HangarCapacity: 12
DroneCapacity: 40
AISystemRating: 9
PointDefenseRating: 9.0
CrewRequired: 40
```

### Best Used For
- Fleet support operations
- Fighter launch platform
- Drone coordination
- Tactical command
- Area control

---

## 8. Raptor Assault Corvette

**File:** `Corvette_RaptorAssault.yaml`

### Role & Purpose
Specialized boarding vessel for ship capture operations.

### Key Strengths
- ✅ Maximum boarding capability (10/10)
- ✅ Fast and maneuverable
- ✅ Multiple hangar bays for shuttles
- ✅ Strong point defense
- ✅ Good electronic warfare

### Key Weaknesses
- ❌ Moderate hull strength
- ❌ Limited long-range capability
- ❌ High-risk missions
- ❌ Requires elite crew

### Stat Highlights
```yaml
BoardingCapability: 10
MaxSpeed: 720
HangarCapacity: 4
PointDefenseRating: 8.0
MaxCrew: 40
```

### Best Used For
- Ship capture operations
- Piracy
- Special forces insertion
- Anti-piracy (capture not destroy)
- Salvage enforcement

---

## 9. Sentinel Patrol Ship

**File:** `Patrol_SentinelLaw.yaml`

### Role & Purpose
Versatile law enforcement vessel for peacekeeping and customs.

### Key Strengths
- ✅ Excellent sensors (24,000 range)
- ✅ High diplomacy rating (8/10)
- ✅ Fast and agile
- ✅ Good electronic warfare
- ✅ Balanced capabilities

### Key Weaknesses
- ❌ Moderate hull strength
- ❌ Limited heavy weapons
- ❌ Not specialized in combat
- ❌ Requires diplomatic approach

### Stat Highlights
```yaml
SensorRange: 24000
DiplomacyRating: 8
MaxSpeed: 780
BoardingCapability: 5
EWarRating: 7
```

### Best Used For
- Law enforcement patrol
- Customs inspection
- Peacekeeping operations
- Search and rescue
- Traffic control

---

## 10. Genesis Colony Ship

**File:** `Transport_GenesisColony.yaml`

### Role & Purpose
Massive colonization vessel carrying thousands to establish new worlds.

### Key Strengths
- ✅ Enormous crew capacity (3000)
- ✅ Massive cargo for colony supplies
- ✅ Maximum life support (10/10)
- ✅ Excellent medical facilities
- ✅ Self-sustaining ecosystem

### Key Weaknesses
- ❌ Extremely slow (300 m/s)
- ❌ Terrible maneuverability
- ❌ Huge, easy target
- ❌ High fuel consumption
- ❌ Very expensive

### Stat Highlights
```yaml
MaxCrew: 3000
CargoCapacity: 15000
HullStrength: 12000
LifeSupportRating: 10
MedicalRating: 9
```

### Best Used For
- Colony establishment
- Mass evacuation
- Generation ships
- Mobile cities
- Civilization expansion

---

## Design Philosophy

These 10 ships were designed to fill specific gameplay niches:

1. **Viper** - Pure combat performance
2. **Shadowblade** - Stealth and espionage
3. **Lifeline** - Medical and humanitarian
4. **Odyssey** - Science and exploration
5. **Behemoth** - Bulk cargo transport
6. **Phoenix** - Salvage and recovery
7. **Vanguard** - Carrier warfare
8. **Raptor** - Boarding specialist
9. **Sentinel** - Law enforcement
10. **Genesis** - Colonization

Each ship has clear strengths and weaknesses, encouraging different playstyles and strategies.

---

## Integration with Existing Fleet

These new ships complement the existing roster:

| Niche | Existing Ship | New Ship | Difference |
|-------|---------------|----------|------------|
| Fast Combat | - | Viper | Dedicated interceptor |
| Stealth | Pathfinder (Scout) | Shadowblade | Military-grade stealth |
| Medical | Starliner (9 rating) | Lifeline (10 rating) | Dedicated hospital |
| Science | - | Odyssey | Pure research vessel |
| Bulk Cargo | Merchant (5000) | Behemoth (20000) | 4x capacity |
| Utility | - | Phoenix | Salvage specialist |
| Carrier | Sovereign (minor) | Vanguard | Dedicated carrier |
| Boarding | Warhammer (8 rating) | Raptor (10 rating) | Boarding focus |
| Patrol | - | Sentinel | Law enforcement |
| Colony | - | Genesis | Massive scale transport |

---

## File Locations

All new ship templates are located in:
```
Assets/SpaceshipTemplates/
├── Fighter_ViperInterceptor.yaml
├── Frigate_ShadowbladeStealth.yaml
├── Cruiser_LifelineMedical.yaml
├── Science_Odyssey.yaml
├── Transport_BehemothFreighter.yaml
├── Utility_PhoenixSalvage.yaml
├── Carrier_VanguardEscort.yaml
├── Corvette_RaptorAssault.yaml
├── Patrol_SentinelLaw.yaml
└── Transport_GenesisColony.yaml
```

---

## Next Steps

1. **Import to Unreal**: Use `YAMLtoDataAsset.py` to import these ships as Data Assets
2. **Test Balance**: Playtest each ship to ensure balanced gameplay
3. **Create Variants**: Use these as bases for faction-specific variants
4. **Add Visuals**: Create or assign 3D models and textures
5. **Write Lore**: Expand backstories for each manufacturer

See `SpaceshipDataAssetGuide.md` for detailed import instructions.
