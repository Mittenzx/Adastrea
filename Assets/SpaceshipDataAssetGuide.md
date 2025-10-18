# Spaceship Data Asset Creation - Complete Designer Guide

## Overview
This guide walks you through creating comprehensive Spaceship Data Assets in Unreal Engine. The spaceship system allows designers to define complete ship specifications including combat stats, mobility, utility systems, operational requirements, and lore - all without writing C++ code.

---

## Part 1: Creating Your First Spaceship Asset

### Step 1: Create a New Spaceship Data Asset

1. **Navigate to your Content Browser**
   - Recommended location: `Content/Spaceships/`
   - Create the folder if it doesn't exist

2. **Create the Data Asset**
   - Right-click in the Content Browser
   - Select **Miscellaneous → Data Asset**
   - In the dialog, search for and select **SpaceshipDataAsset**
   - Name your asset using the convention: `DA_Ship_[ShipName]`
   - Example: `DA_Ship_PathfinderScout`

3. **Open the Asset**
   - Double-click your newly created asset to open the editor

---

## Part 2: Configuring Basic Information

In the **Basic Info** category:

### Ship Name (Text)
- The display name that players will see
- Example: "Pathfinder Scout"
- Keep it memorable and appropriate to the ship's role

### Description (Multi-line Text)
- Write 2-4 sentences describing the ship's purpose and characteristics
- Include its strengths and intended role
- Example:
  ```
  A lightweight reconnaissance vessel designed for deep space exploration 
  and intelligence gathering. The Pathfinder trades armor and firepower 
  for speed, sensors, and fuel efficiency.
  ```

### Ship Class (Text)
- The ship's type/classification
- Examples: "Scout", "Gunship", "Trading Vessel", "Luxury Liner", "Mining Barge", "Command Cruiser"
- Used for filtering and categorization

### Ship ID (Name)
- A unique identifier for code reference
- Use PascalCase without spaces
- Example: "PathfinderScout"
- Must be unique across all ships

---

## Part 3: Core Stats

These define the ship's fundamental capabilities:

### Hull Strength (0 - 100,000)
- The ship's total health/hit points
- **Fighter**: 500-1,500
- **Corvette**: 1,500-3,000
- **Frigate**: 3,000-7,000
- **Cruiser**: 7,000-15,000
- **Battleship**: 15,000-30,000
- **Capital Ship**: 30,000+

### Cargo Capacity (0 - 50,000 tons)
- How much cargo the ship can carry
- **Scout**: 50-100
- **Combat**: 100-300
- **Trading**: 2,000-10,000
- **Mining**: 5,000-15,000

### Crew Required (1 - 10,000)
- Minimum crew needed to operate the ship
- **Small Fighter**: 1-2
- **Scout/Corvette**: 3-10
- **Frigate**: 10-50
- **Cruiser**: 50-200
- **Capital Ship**: 200-1,000+

### Max Crew (1 - 10,000)
- Maximum crew capacity including passengers
- Should be higher than Crew Required
- Affects operational costs and capabilities

### Modular Points (0 - 100)
- How many customization slots the ship has
- Higher values allow more equipment variety
- Typical range: 3-20

---

## Part 4: Combat Stats

Define the ship's combat effectiveness:

### Armor Rating (0 - 1,000)
- Physical damage reduction
- **Light**: 10-50
- **Medium**: 50-150
- **Heavy**: 150-300
- **Capital**: 300-500+

### Shield Strength (0 - 100,000)
- Energy shield capacity (regenerates over time)
- Usually 50-70% of hull strength for balanced ships
- Combat ships may have higher shield ratios

### Shield Recharge Rate (0 - 1,000 per second)
- How fast shields regenerate
- Typically 1-3% of max shield strength per second
- Military ships have faster recharge rates

### Weapon Slots (0 - 50)
- Number of weapon hardpoints
- **Scout/Civilian**: 0-2
- **Light Combat**: 2-4
- **Gunship**: 4-8
- **Capital Ship**: 10-20+

### Weapon Power Capacity (0 - 10,000)
- Total power available for weapons
- Multiply weapon slots by 500-1000 for baseline
- Gunships have higher power capacity

### Point Defense Rating (0 - 100)
- Effectiveness against missiles and fighters
- **None**: 0-3
- **Basic**: 3-5
- **Good**: 5-8
- **Excellent**: 8-10

---

## Part 5: Mobility Stats

Control how the ship moves:

### Max Speed (0 - 10,000 m/s)
- Top speed in normal space
- **Heavy/Capital**: 300-500
- **Medium**: 500-700
- **Light/Scout**: 700-1,000+

### Acceleration (0 - 1,000 m/s²)
- How quickly ship reaches max speed
- Usually 10-20% of max speed as a baseline
- Affects combat responsiveness

### Maneuverability (1 - 10)
- Turn rate and agility
- **Heavy/Capital**: 2-4
- **Medium**: 4-6
- **Light/Scout**: 6-10

### Jump Range (0 - 1,000 light years)
- How far the ship can jump in one FTL transit
- **Short Range**: 5-10 LY
- **Medium Range**: 10-20 LY
- **Long Range**: 20-30+ LY
- Exploration ships should have high values

### Fuel Capacity (0 - 100,000 units)
- Total fuel storage
- Balance with jump range and consumption
- Larger ships typically have more fuel

### Fuel Consumption Rate (0 - 10,000 per jump)
- Fuel used per FTL jump
- Calculate: (Fuel Capacity / Jump Range) * 0.5 for efficient ships
- Heavy ships consume more fuel

---

## Part 6: Utility/Support Stats

Systems that provide non-combat benefits:

### Sensor Range (0 - 100,000 units)
- Detection range for objects and ships
- **Basic**: 5,000-10,000
- **Good**: 10,000-20,000
- **Excellent**: 20,000-30,000+
- Scouts should have exceptional range

### Sensor Resolution (1 - 10)
- Detail level of sensor readings
- Higher values provide more information
- Military and science ships need high resolution

### Stealth Rating (1 - 10)
- How hard the ship is to detect
- **Poor**: 1-3 (large signature)
- **Average**: 4-6
- **Good**: 7-8
- **Excellent**: 9-10 (scout/infiltration)

### Repair System Rating (0 - 10)
- Effectiveness of onboard repair systems
- **None**: 0
- **Basic**: 3-5
- **Good**: 5-7
- **Advanced**: 8-10 (industrial ships)

### Science Rating (0 - 10)
- Research and analysis capabilities
- **None**: 0-2
- **Basic**: 3-5
- **Research Vessel**: 7-10

### Medical Rating (0 - 10)
- Medical facilities quality
- **None**: 0-1
- **Basic First Aid**: 2-4
- **Medical Bay**: 5-7
- **Hospital Ship**: 8-10

---

## Part 7: Operational Stats

Day-to-day operational capabilities:

### Power Capacity (0 - 100,000)
- Total power generation
- Should support all active systems
- Calculate: Sum of weapon power + (crew * 10) + (hull strength * 0.5)

### Life Support Rating (1 - 10)
- Quality of life support systems
- Affects crew morale and endurance
- Luxury ships: 8-10
- Military/Industrial: 5-7
- Basic: 3-5

### Maintenance Level (1 - 10)
- Maintenance requirements (higher = more maintenance)
- **Low**: 2-4 (reliable, simple systems)
- **Medium**: 5-7
- **High**: 8-10 (complex, cutting-edge tech)

### Hangar Capacity (0 - 100)
- Number of small craft (fighters, shuttles)
- **None**: 0
- **Shuttle Bay**: 1-3
- **Light Carrier**: 5-10
- **Fleet Carrier**: 20+

### Drone Capacity (0 - 1,000)
- Number of automated drones/probes
- Scouts: 10-20
- Mining: 30-50
- Military: 50-100+

### AI System Rating (0 - 10)
- Sophistication of ship AI
- **Basic**: 3-5 (navigation only)
- **Advanced**: 6-8 (tactical support)
- **Cutting Edge**: 9-10 (autonomous operations)

---

## Part 8: Advanced/Special Stats

Specialized capabilities:

### Diplomacy Rating (0 - 10)
- Negotiation and diplomatic facilities
- **None**: 0-2
- **Basic**: 3-5
- **Embassy Ship**: 6-8
- **Flagship**: 9-10

### Boarding Capability (0 - 10)
- Effectiveness in boarding actions
- **None**: 0-2
- **Basic Security**: 3-5
- **Marines**: 6-8
- **Elite Commandos**: 9-10

### Customizable Slots (0 - 20)
- Additional module slots for player customization
- Varies by ship role and size
- Typically 3-12 slots

### EWar Rating (0 - 10)
- Electronic warfare capabilities
- **None**: 0-2
- **Basic**: 3-5
- **Military**: 6-8
- **Specialized**: 9-10

### Mining Rating (0 - 10)
- Mining and resource extraction capability
- **None**: 0
- **Basic**: 1-3 (can mine if needed)
- **Mining Vessel**: 7-10

---

## Part 9: Lore and Flavor

Add character and story to your ship:

### Manufacturer (Text)
- Company or organization that built the ship
- Examples: "Vanguard Aerospace", "Ironforge Military Systems", "Stellar Commerce Industries"
- Use consistent naming for related ships

### Year Built (Integer)
- In-universe year of manufacture
- Used for tech level and availability
- Current game year should be established in your lore

### Rarity Tier (Text)
- How common the ship is
- **Common**: Mass-produced, readily available
- **Uncommon**: Specialized production
- **Rare**: Limited production, advanced tech
- **Legendary**: One-of-a-kind, prototype, or historic

### Lore Notes (Multi-line Text)
- Background story and interesting details
- 2-4 sentences
- Include notable features or history

### Primary Color (Linear Color)
- Main hull color
- Use the color picker to set RGBA values
- Consider faction colors or ship role

### Secondary Color (Linear Color)
- Accent/trim color
- Should complement primary color
- Used for detail elements

---

## Part 10: Using YAML Templates

Pre-made templates are available in `Assets/SpaceshipTemplates/`:

### Available Templates:
1. **Scout_Pathfinder.yaml** - Fast reconnaissance vessel
2. **Gunship_Warhammer.yaml** - Heavy combat ship
3. **Trading_Merchant.yaml** - Large cargo hauler
4. **Luxury_Starliner.yaml** - Premium passenger vessel
5. **Mining_Excavator.yaml** - Industrial mining barge
6. **Command_Sovereign.yaml** - Capital flagship

### How to Use Templates:
1. Open the YAML file in a text editor
2. Copy the values you want
3. Paste into your Data Asset in Unreal Engine
4. Adjust values to create variants
5. Save and test your ship

---

## Part 11: Blueprint Integration

### Accessing Spaceship Data in Blueprints:

1. **Add a Spaceship Data Asset Reference**
   - Create a variable of type `Spaceship Data Asset (Object Reference)`
   - Make it `Instance Editable` to set per-instance

2. **Get Stats**
   - Drag from your reference and use Get functions
   - Example: `Get Max Speed`, `Get Hull Strength`
   - All stats are Blueprint Readable

3. **Use Helper Functions**
   - `Get Combat Rating` - Returns overall combat effectiveness (0-100)
   - `Get Mobility Rating` - Returns overall mobility (0-100)
   - `Get Utility Rating` - Returns overall utility (0-100)
   - `Is Suitable For Role` - Check if ship fits a role ("Combat", "Trade", "Exploration", etc.)
   - `Get Size Category` - Returns size classification
   - `Get Operational Cost` - Returns daily maintenance cost

### Example Blueprint Usage:

```
Event BeginPlay
├─ Get Spaceship Data Asset
├─ Get Combat Rating
└─ Branch (Combat Rating >= 60)
   ├─ True: Assign to Combat Fleet
   └─ False: Assign to Patrol Duty
```

---

## Part 12: Balancing Guidelines

### Design Philosophy:
- **Specialization over Generalization**: Ships should excel in their role
- **Trade-offs**: High combat = low cargo, high speed = low armor
- **Cost vs Benefit**: Powerful ships should have higher operational costs

### Balance Checklist:
- [ ] Combat ships sacrifice cargo and efficiency
- [ ] Trading ships have minimal combat capability
- [ ] Scouts have excellent sensors and range but weak combat
- [ ] Capital ships are powerful but expensive to operate
- [ ] Stats align with ship's intended role
- [ ] Rarity tier matches overall power level

### Quick Balance Formula:
1. Choose primary role (Combat, Trade, Exploration, etc.)
2. Max out 2-3 stats for that role (8-10 rating)
3. Keep complementary stats moderate (5-7)
4. Minimize opposing role stats (1-3)
5. Calculate operational cost to match power level

---

## Part 13: Testing Your Ship

### In-Editor Testing:
1. Create a test Blueprint extending the Spaceship class
2. Assign your Data Asset
3. Add debug display to show stats
4. Place in test level and verify all values appear correctly

### Gameplay Testing:
1. Test movement (speed, acceleration, turning)
2. Test combat (weapons, shields, armor)
3. Test utility systems (sensors, repair)
4. Verify operational costs are balanced
5. Check that role suitability functions return expected results

### Performance Testing:
- Ensure stats don't cause gameplay issues
- Very high speeds may need level design adjustments
- Very large cargo values may need economy balancing
- Test with multiple ships to ensure variety

---

## Part 14: Common Patterns and Tips

### Creating Ship Variants:
- Duplicate existing asset
- Modify 2-3 key stats
- Change name and ID
- Adjust lore notes
- Example: "Scout Mk II" with better sensors but same hull

### Role-Based Presets:

**Scout Template:**
- Low hull, high speed, excellent sensors, long jump range

**Combat Template:**
- High hull, strong weapons, heavy armor, low cargo

**Trader Template:**
- Massive cargo, moderate speed, minimal weapons, good jump range

**Mining Template:**
- High mining rating, huge cargo, low speed, good repair systems

**Luxury Template:**
- High crew capacity, excellent life support, good diplomacy, moderate everything else

### Common Mistakes to Avoid:
- ❌ Making every stat high (no trade-offs)
- ❌ Setting operational costs too low
- ❌ Forgetting to set Ship ID (causes blueprint errors)
- ❌ Making extreme values (100,000 speed, etc.) without testing
- ❌ Not matching lore to stats (luxury ship with maintenance 10?)

---

## Part 15: Advanced Customization

### Creating Custom Stat Systems:
The Data Asset can be extended in C++ to add custom stats or calculations. See the source code in `Source/Adastrea/Ships/SpaceshipDataAsset.h` for examples.

### Integration with Other Systems:
- **Faction System**: Ships can reference faction data assets for faction-specific ships
- **Module System**: Use Customizable Slots to tie into modular equipment
- **Economy System**: Operational Cost provides a hook for economy integration
- **Quest System**: Ship ID can be used for quest requirements ("Find a ship with Mining Rating >= 7")

### Workflow Integration:
1. Design on paper first (role, key stats, trade-offs)
2. Use YAML template as starting point
3. Create Data Asset in Unreal
4. Test in isolation
5. Balance against other ships
6. Integrate into gameplay
7. Iterate based on playtesting

---

## Part 16: Quick Reference

### Essential Stats by Role:

| Role | Primary Stats | Secondary Stats | Minimize |
|------|--------------|----------------|----------|
| Scout | Sensors, Speed, Jump Range | Stealth, Fuel | Cargo, Weapons |
| Combat | Weapons, Armor, Shields | Hull, Point Defense | Cargo, Diplomacy |
| Trader | Cargo, Fuel, Jump Range | Power, Life Support | Weapons, Speed |
| Mining | Mining, Cargo, Drones | Repair, Power | Speed, Combat |
| Luxury | Life Support, Diplomacy, Medical | Crew Capacity, Shields | Weapons, Mining |
| Command | Weapons, Hangar, Diplomacy | Hull, AI, EWar | Mining, Cargo |

---

## Conclusion

You now have everything you need to create comprehensive, balanced spaceship data assets for your game! Start with the provided templates, customize them for your needs, and create the diverse fleet of vessels your game requires.

For more examples and templates, see:
- `Assets/SpaceshipTemplates/` - YAML templates
- `Assets/SpaceshipDesignerWorkflow.md` - Workflow guide
- `README.md` - System overview

Happy ship designing!
