# Example Data Asset Configurations

This file provides ready-to-use configurations for creating Data Assets in Unreal Engine.

## How to Use

1. Create a new Data Asset of the specified type
2. Copy the values from this file into the Data Asset properties
3. Save and use in your Blueprints

---

## Example 1: Player Scout Ship

**Asset Type**: SpaceshipDataAsset  
**Name**: DA_Ship_PlayerScout  
**Location**: Content/DataAssets/Ships/

### Basic Info
```
ShipName: Scout Ship Alpha
ShipClass: Scout
ShipID: player_scout_01
Description: A fast, maneuverable scout vessel perfect for exploration and reconnaissance missions.
```

### Core Stats
```
HullStrength: 1000.0
CargoCapacity: 50.0
CrewRequired: 1
MaxCrew: 2
ModularPoints: 5
```

### Combat Stats
```
ArmorRating: 100.0
ShieldStrength: 500.0
ShieldRechargeRate: 25.0
WeaponSlots: 2
WeaponPowerCapacity: 200.0
PointDefenseRating: 30.0
```

### Mobility Stats
```
MaxSpeed: 800.0
Acceleration: 150.0
Maneuverability: 8
JumpRange: 50.0
FuelCapacity: 500.0
FuelConsumptionRate: 10.0
```

### Utility Stats
```
SensorRange: 5000.0
SensorResolution: 7
StealthRating: 6
RepairSystemRating: 3
ScienceRating: 5
MedicalRating: 2
```

### Operational Stats
```
PowerCapacity: 1000.0
LifeSupportRating: 5
MaintenanceLevel: 3
HangarCapacity: 0
DroneCapacity: 2
AISystemRating: 4
```

### Advanced Stats
```
DiplomacyRating: 3
BoardingCapability: 2
CustomizableSlots: 4
EWarRating: 4
MiningRating: 2
```

### Lore
```
Manufacturer: Stellar Dynamics Corp
YearBuilt: 2450
RarityTier: Common
LoreNotes: The workhorse of exploration fleets, known for reliability and speed.
PrimaryColor: RGB(0.2, 0.5, 0.8) - Blue
SecondaryColor: RGB(0.9, 0.9, 0.9) - Silver
```

---

## Example 2: Player Alliance Faction

**Asset Type**: FactionDataAsset  
**Name**: DA_Faction_PlayerAlliance  
**Location**: Content/DataAssets/Factions/

### Basic Info
```
FactionName: United Terran Alliance
FactionID: player_alliance
Description: A democratic coalition of human colonies dedicated to exploration, trade, and mutual defense.
```

### Colors
```
PrimaryColor: RGB(0.1, 0.3, 0.7) - Deep Blue
SecondaryColor: RGB(0.9, 0.9, 0.9) - White
```

### Attributes
```
TechnologyLevel: 7
MilitaryStrength: 6
EconomicPower: 8
InitialReputation: 100
bHostileByDefault: false
```

### Traits (Add these as array elements)

**Trait 1**:
```
TraitName: Trade Network
TraitID: trade_bonus
TraitDescription: Extensive trade routes provide economic advantages
ModifierValue: 1.2
```

**Trait 2**:
```
TraitName: Diplomatic Corps
TraitID: diplomacy_bonus
TraitDescription: Skilled diplomats improve relationships
ModifierValue: 1.15
```

### Faction Relationships

**With Pirates**:
```
TargetFactionID: pirates_syndicate
RelationshipValue: -60
bIsAllied: false
bAtWar: false
TradeModifier: 0.5
```

**With Merchants**:
```
TargetFactionID: merchant_guild
RelationshipValue: 75
bIsAllied: true
bAtWar: false
TradeModifier: 1.3
```

---

## Example 3: Player Character Personnel

**Asset Type**: PersonnelDataAsset  
**Name**: DA_Personnel_PlayerCaptain  
**Location**: Content/DataAssets/Personnel/

### Basic Info
```
PersonnelName: Commander Sarah Chen
PersonnelID: player_captain_01
Role: Captain
Description: Experienced explorer and leader with a passion for discovery.
```

### Status
```
Health: 100.0
Morale: 85.0
Fatigue: 20.0
Loyalty: 95.0
Reputation: 50.0
```

### Skills (Add as array elements)

**Skill 1**:
```
SkillName: Leadership
SkillLevel: 8
ExperiencePoints: 5000
Specialty: true
```

**Skill 2**:
```
SkillName: Piloting
SkillLevel: 7
ExperiencePoints: 3500
Specialty: true
```

**Skill 3**:
```
SkillName: Navigation
SkillLevel: 6
ExperiencePoints: 2000
Specialty: false
```

### Employment
```
Salary: 5000.0
ContractDuration: 365.0
ContractRemaining: 365.0
```

### Traits

**Trait 1**:
```
TraitName: Natural Leader
TraitID: leader_bonus
TraitDescription: Crew morale increased by 15%
ModifierValue: 1.15
```

---

## Example 4: Trading Item - Iron Ore

**Asset Type**: TradeItemDataAsset  
**Name**: DA_TradeItem_IronOre  
**Location**: Content/DataAssets/Trading/Items/

### Basic Info
```
ItemName: Iron Ore
ItemID: ore_iron
Description: Raw iron ore mined from asteroids, essential for ship construction.
Category: Raw Material
```

### Economic Values
```
BasePrice: 50.0
PriceVolatility: 0.3
SupplyLevel: 8
DemandLevel: 7
```

### Physical Properties
```
CargoSpacePerUnit: 1.0
MassPerUnit: 1.0
bIsIllegal: false
bIsPerishable: false
```

### Trading Modifiers
```
MinPriceMultiplier: 0.5
MaxPriceMultiplier: 2.0
```

---

## Example 5: Trade Market - Station Hub

**Asset Type**: MarketDataAsset  
**Name**: DA_Market_CentralHub  
**Location**: Content/DataAssets/Trading/Markets/

### Basic Info
```
MarketName: Central Trading Hub
MarketID: market_central_01
Description: The main trading station for the sector.
```

### Market Properties
```
MarketSize: Large
TaxRate: 0.05
bAllowsIllegalGoods: false
RefreshRate: 24.0
```

### Available Goods (Add to array)
```
Items: 
  - DA_TradeItem_IronOre
  - DA_TradeItem_FoodSupplies
  - DA_TradeItem_FuelCells
  - DA_TradeItem_Electronics
```

### Price Modifiers
```
BuyPriceModifier: 0.9
SellPriceModifier: 1.1
```

---

## Example 6: Simple Homeworld

**Asset Type**: HomeworldDataAsset  
**Name**: DA_Homeworld_Earth  
**Location**: Content/DataAssets/Homeworlds/

### Basic Info
```
HomeworldName: Earth
HomeworldID: homeworld_earth
Description: Humanity's birthplace and the heart of the United Terran Alliance.
```

### Faction Relations (Add to array)

**With Player Alliance**:
```
Faction: DA_Faction_PlayerAlliance
StartingReputation: 100
```

**With Merchants**:
```
Faction: DA_Faction_MerchantGuild
StartingReputation: 50
```

**With Pirates**:
```
Faction: DA_Faction_Pirates
StartingReputation: -75
```

---

## Quick Creation Checklist

When creating Data Assets, always:

- [ ] Set a descriptive name
- [ ] Fill in all required fields
- [ ] Use consistent IDs (lowercase, underscores)
- [ ] Add meaningful descriptions
- [ ] Test in-game before finalizing
- [ ] Save in appropriate folder
- [ ] Document any special properties

---

## Example 9: Faction with Categorized Traits (NEW)

**Asset Type**: FactionDataAsset  
**Name**: DA_Faction_SolarisUnion  
**Location**: Content/DataAssets/Factions/

### Faction Info
```
FactionName: Solaris Union
FactionID: solaris_union
Description: A democratic alliance of solar systems focused on scientific advancement and peaceful expansion.
PrimaryColor: (R=0.2, G=0.5, B=0.9, A=1.0)  // Blue
SecondaryColor: (R=0.9, G=0.9, B=0.95, A=1.0)  // White
```

### Faction Attributes
```
InitialReputation: 0
bHostileByDefault: false
TechnologyLevel: 8
MilitaryStrength: 6
EconomicPower: 7
```

### Traits (with Categories)
```
Trait 1:
  TraitID: scientific_excellence
  TraitName: Scientific Excellence
  TraitDescription: Advanced research facilities provide technology bonuses
  Category: Scientific
  ModifierValue: 1.3

Trait 2:
  TraitID: peaceful_traders
  TraitName: Peaceful Traders
  TraitDescription: Improved trade relationships and better prices
  Category: Economic
  ModifierValue: 1.2

Trait 3:
  TraitID: diplomatic_corps
  TraitName: Diplomatic Corps
  TraitDescription: Skilled diplomats improve faction relationships
  Category: Diplomatic
  ModifierValue: 1.15

Trait 4:
  TraitID: defensive_stance
  TraitName: Defensive Stance
  TraitDescription: Strong defense but limited offensive capability
  Category: Military
  ModifierValue: 0.8
```

### Faction Relationships
```
Relationship 1:
  TargetFactionID: nova_vanguard
  RelationshipValue: 50
  bIsAllied: true
  bAtWar: false
  TradeModifier: 1.2

Relationship 2:
  TargetFactionID: obsidian_order
  RelationshipValue: -30
  bIsAllied: false
  bAtWar: false
  TradeModifier: 1.1

Relationship 3:
  TargetFactionID: pirates_consortium
  RelationshipValue: -75
  bIsAllied: false
  bAtWar: true
  TradeModifier: 0.0
```

### Usage with New Systems

**In Blueprints:**
```
1. Create Faction Runtime State component
2. Initialize From Template → Select DA_Faction_SolarisUnion
3. Access via Get Faction State → Modify Player Reputation
4. Get Traits By Category → Filter by Scientific/Economic/etc
5. Use with Diplomacy Manager for faction interactions
```

**Example Integration:**
```cpp
// Get all scientific traits
TArray<FFactionTrait> ScienceTraits = FactionAsset->GetTraitsByCategory(EFactionTraitCategory::Scientific);

// Calculate research bonus
float ResearchBonus = FactionAsset->GetCategoryModifierTotal(EFactionTraitCategory::Scientific);
// Result: 1.3 from Scientific Excellence trait
```

---

## See Also

- `Assets/SpaceshipTemplates.md` - More ship examples
- `Assets/FactionSetupGuide.md` - More faction examples
- `Assets/FactionSystemQuickReference.md` - **NEW** Quick reference for faction, reputation, and diplomacy systems
- `Assets/PersonnelTemplates.md` - More personnel examples
- `Assets/TradingSystemGuide.md` - More trading examples
- `CONTENT_CREATION_QUICKSTART.md` - How to use these assets
- `WHATS_STILL_NEEDED.md` - Implementation status and next steps
