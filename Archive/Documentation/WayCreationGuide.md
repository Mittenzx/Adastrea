# Way Creation Guide

Complete step-by-step guide for creating new Ways (specialized guilds).

## Quick Start (5 Minutes)

### 1. Create Way Data Asset

**In Unreal Editor:**
1. Content Browser → Right-click in `Content/DataAssets/Ways/`
2. Blueprint Class → Parent Class: `WayDataAsset`
3. Name: `DA_Way_YourWayName` (e.g., `DA_Way_DwarvenMiners`)
4. Double-click to open

### 2. Configure Core Identity

**Identity Section:**
- **Way Name**: "Dwarven Mining Way"
- **Description**: "Hardy miners who extract ore from the most dangerous asteroids"
- **Way ID**: `DwarvenMiningWay` (unique identifier, no spaces)
- **Is School**: `false` (true = knowledge/skill focus, false = commerce/trade focus)
- **Primary Color**: Choose main UI color
- **Secondary Color**: Choose accent color

### 3. Set Organizational Attributes

**Attributes Section:**
- **Technology Level**: 1-10 (how advanced are they?)
- **Military Strength**: 1-10 (combat capability)
- **Economic Power**: 1-10 (wealth and resources)
- **Influence Power**: 1-10 (political/social influence)
- **Home Base ID**: Name of their headquarters station
- **Controlled Sectors**: Array of sector names they operate in

### 4. Define Specialization

**Specialization Section:**
- **Primary Industry**: `Mining` (choose from dropdown)
- **Secondary Industries**: Add any additional focuses (optional)
- **Quality Reputation**: `Quality` (their reputation level)
- **Specialization Description**: "We mine the hardest ores from the most dangerous asteroids. Our product purity is unmatched."
- **Member Count**: 250 (approximate number of members)

### 5. Configure Production/Consumption

**Trade Section - Produced Resources:**

Add resources this Way produces:
1. Click `+` on Produced Resources
2. **Resource Name**: "Iron Ore"
3. **Resource ID**: `IronOre`
4. **Quantity**: 1000 (per cycle/day)
5. **Quality Level**: `Quality`
6. **Is Produced**: `true`
7. **Resource Notes**: "High-purity ore extracted from asteroid belt"

Repeat for each resource type.

**Trade Section - Consumed Resources:**

Add resources this Way needs:
1. Click `+` on Consumed Resources
2. **Resource Name**: "Mining Equipment"
3. **Resource ID**: `MiningEquipment`
4. **Quantity**: 50 (per cycle/day)
5. **Quality Level**: `Standard`
6. **Is Produced**: `false`
7. **Resource Notes**: "Replacement tools and equipment"

### 6. Establish Supply Chains

**Trade Section - Supply Chains:**

Example: Dwarven Mining Way supplies ore to Pure Crafters

1. Click `+` on Supply Chains
2. **Supplier Way**: Select `DA_Way_DwarvenMiners` (this Way)
3. **Consumer Way**: Select `DA_Way_PureCrafters`
4. **Resource ID**: `IronOre`
5. **Supply Quantity**: 800 (per cycle)
6. **Price Per Unit**: 10 credits
7. **Is Exclusive**: `false`
8. **Contract Duration**: 0 (indefinite)

### 7. Define Core Precepts

**Philosophy Section:**

Add Precepts this Way values:
1. Click `+` on Core Precepts
2. **Precept**: `Mastery` (from dropdown)
3. **Importance Value**: 85 (0-100, how much they value it)
4. **Description**: "We master the art of mining through dedication and experience"

Example Precepts for Dwarven Mining Way:
- Mastery: 85 (perfect mining technique)
- Strength: 70 (physical endurance)
- Prosperity: 60 (seeking wealth)
- Survival: 75 (working in dangerous conditions)

### 8. Set Business Relationships

**Relationships Section:**

Add relationships with other Ways:
1. Click `+` on Way Relationships
2. **Target Way ID**: `PureCraftersWay`
3. **Relationship Value**: 60 (positive, cooperative)
4. **Has Partnership**: `true`
5. **In Competition**: `false`
6. **Trade Modifier**: 1.2 (20% better prices)
7. **Shares Resources**: `true`
8. **Relationship Notes**: "Long-standing supply agreement for ore delivery"

### 9. Configure Council Participation

**Governance Section:**
- **Participates In Councils**: `true`
- **Base Voting Weight**: 15 (0-100, based on influence)
- **Represented Sectors**: Add sector names where they have a council seat

### 10. Save and Test

1. Save the Data Asset (Ctrl+S)
2. Compile (if needed)
3. Test in-game or Blueprint

## Complete Examples

### Example 1: Dwarven Mining Way

```yaml
Identity:
  Way Name: "Dwarven Mining Way"
  Way ID: DwarvenMiningWay
  Is School: false
  Description: "Hardy miners extracting ore from dangerous asteroids"

Attributes:
  Technology Level: 6
  Military Strength: 7
  Economic Power: 7
  Influence Power: 6
  Home Base ID: "MiningStation_Alpha"
  Controlled Sectors: ["AsteroidBelt_Outer", "FrontierMining"]

Specialization:
  Primary Industry: Mining
  Quality Reputation: Quality
  Member Count: 250

Produced Resources:
  - Resource Name: "Iron Ore"
    Resource ID: IronOre
    Quantity: 1000
    Quality: Quality
  
  - Resource Name: "Rare Metals"
    Resource ID: RareMetals
    Quantity: 200
    Quality: Premium

Consumed Resources:
  - Resource Name: "Mining Equipment"
    Resource ID: MiningEquipment
    Quantity: 50
    Quality: Standard

Core Precepts:
  - Mastery: 85
  - Strength: 70
  - Prosperity: 60
  - Survival: 75

Governance:
  Voting Weight: 15
  Represented Sectors: ["AsteroidBelt_Outer", "FrontierMining"]
```

### Example 2: Way of Pure Crafters

```yaml
Identity:
  Way Name: "Way of Pure Crafters"
  Way ID: PureCraftersWay
  Is School: true
  Description: "Master artisans creating legendary spaceship components"

Attributes:
  Technology Level: 9
  Military Strength: 3
  Economic Power: 8
  Influence Power: 7
  Home Base ID: "CraftersGuild_Station"
  Controlled Sectors: ["CoreSystems_Manufacturing"]

Specialization:
  Primary Industry: Manufacturing
  Secondary Industries: [Engineering]
  Quality Reputation: Elite
  Member Count: 180

Produced Resources:
  - Resource Name: "Elite Ship Components"
    Resource ID: EliteShipComponents
    Quantity: 500
    Quality: Elite
  
  - Resource Name: "Custom Parts"
    Resource ID: CustomParts
    Quantity: 100
    Quality: Legendary

Consumed Resources:
  - Resource Name: "Iron Ore"
    Resource ID: IronOre
    Quantity: 800
    Quality: Quality
  
  - Resource Name: "Rare Metals"
    Resource ID: RareMetals
    Quantity: 150
    Quality: Premium

Supply Chains:
  - From: DwarvenMiningWay
    Resource: IronOre
    Quantity: 800
    Price: 10 per unit

Core Precepts:
  - Craftsmanship: 95
  - Mastery: 90
  - Innovation: 75
  - Tradition: 80

Governance:
  Voting Weight: 20
  Represented Sectors: ["CoreSystems_Manufacturing"]
```

### Example 3: Scout's League

```yaml
Identity:
  Way Name: "Scout's League"
  Way ID: ScoutsLeague
  Is School: false
  Description: "Brave explorers mapping the unknown reaches of space"

Attributes:
  Technology Level: 7
  Military Strength: 5
  Economic Power: 4
  Influence Power: 6
  Home Base ID: "ExplorersOutpost"
  Controlled Sectors: ["FrontierSpace"]

Specialization:
  Primary Industry: Exploration
  Secondary Industries: [Salvage, Trading]
  Quality Reputation: Standard
  Member Count: 120

Produced Resources:
  - Resource Name: "Survey Data"
    Resource ID: SurveyData
    Quantity: 300
    Quality: Quality
  
  - Resource Name: "Salvage Materials"
    Resource ID: SalvageMaterials
    Quantity: 400
    Quality: Standard

Consumed Resources:
  - Resource Name: "Fuel"
    Resource ID: Fuel
    Quantity: 200
    Quality: Standard
  
  - Resource Name: "Ship Repairs"
    Resource ID: ShipRepairs
    Quantity: 80
    Quality: Standard

Core Precepts:
  - Discovery: 90
  - Freedom: 85
  - Survival: 75
  - Ambition: 70

Governance:
  Voting Weight: 12
  Represented Sectors: ["FrontierSpace"]
```

## Best Practices

### Naming Conventions

- **Way Name**: Descriptive, memorable (e.g., "Dwarven Mining Way", "Scout's League")
- **Way ID**: CamelCase, no spaces (e.g., `DwarvenMiningWay`, `ScoutsLeague`)
- **Resource IDs**: CamelCase (e.g., `IronOre`, `EliteShipComponents`)

### Balance Guidelines

**Technology Level:**
- 1-3: Primitive, outdated tech
- 4-6: Standard, current tech
- 7-9: Advanced, cutting-edge
- 10: Revolutionary, unique

**Member Count:**
- 50-100: Very small, specialized niche
- 100-500: Small guild, focused operations
- 500-1000: Medium guild, established presence
- 1000+: Large organization, major influence

**Voting Weight:**
- 5-10: Minor presence, small voice
- 10-20: Standard member, normal influence
- 20-30: Major player, significant voice
- 30+: Dominant force (use sparingly)

**Quality Tiers:**
- Basic/Standard: Most Ways start here
- Quality/Premium: Established reputation
- Elite/Legendary: Reserved for exceptional Ways

### Supply Chain Design

1. **Identify needs**: What does your Way need to operate?
2. **Find suppliers**: Which Ways produce those resources?
3. **Set quantities**: Realistic production/consumption rates
4. **Balance economy**: Don't make Ways too self-sufficient
5. **Create dependencies**: Interesting trade networks

### Precept Selection

Choose 3-5 core Precepts that define the Way's values:

- **Crafting Ways**: Craftsmanship, Mastery, Innovation
- **Combat Ways**: Strength, Honor, Loyalty
- **Trade Ways**: Prosperity, Cunning, Efficiency
- **Exploration Ways**: Discovery, Freedom, Ambition
- **Research Ways**: Innovation, Mastery, Progress

## Troubleshooting

### Way doesn't appear in sector councils

- Check `Participates In Councils` is `true`
- Verify `Represented Sectors` includes the sector
- Ensure `Base Voting Weight` > 0

### Supply chain not working

- Verify both Ways exist and are valid
- Check `Resource ID` matches exactly (case-sensitive)
- Confirm supplier produces and consumer consumes the resource

### Resources not balanced

- Total production should roughly match consumption
- Some surplus is OK (allows for trade flexibility)
- Severe shortages create economic pressure

## Next Steps

1. Create 5-10 diverse Ways for your sector
2. Establish supply chain relationships
3. Create sector council with representatives
4. Test economic simulation
5. Balance taxes and budgets

See `SectorCouncilGuide.md` for council setup.
