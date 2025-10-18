# Material Data Asset System - Designer Guide

## Overview
The Material Data Asset system provides a comprehensive framework for defining, managing, and utilizing materials throughout the game. Materials are the building blocks of the economy, crafting systems, station management, and gameplay progression.

---

## Table of Contents
1. [Core Concepts](#core-concepts)
2. [Getting Started](#getting-started)
3. [Material Properties](#material-properties)
4. [Creating New Materials](#creating-new-materials)
5. [Integration with Station Modules](#integration-with-station-modules)
6. [Example Workflows](#example-workflows)
7. [Best Practices](#best-practices)
8. [Playtesting Guidelines](#playtesting-guidelines)

---

## Core Concepts

### What is a Material?
A Material Data Asset represents any physical item, resource, or commodity in the game. This includes:
- **Raw Resources**: Iron ore, titanium ore, crystalline quartz
- **Refined Materials**: Metal ingots, alloys, processed compounds
- **Components**: Actuators, power cells, cooling systems
- **Electronics**: Circuit boards, processors, sensor arrays
- **Construction Parts**: Hull plates, support beams, pressure seals
- **Ship Parts**: Reactor cores, thruster assemblies, shield generators
- **Food**: Protein rations, fresh vegetables, nutrient paste
- **Research Items**: Data, samples, artifacts
- **Organic Materials**: Bio samples, medical supplies, genetic material
- **Synthetic Materials**: Nanomaterials, synthetic fuels, polymers

### Key Features
- **Blueprint-friendly**: All properties accessible via Blueprint
- **Query Functions**: Check tags, categories, storage requirements
- **Module Integration**: Materials specify which modules can process them
- **Storage Management**: Automatic volume calculations based on mass and type
- **Economy Ready**: Value and rarity systems for trading

---

## Getting Started

### Prerequisites
- Unreal Engine project with AdAstrea source code compiled
- Access to Content Browser
- Familiarity with Data Assets (similar to Faction system)

### Quick Start
1. Open your Unreal Engine project
2. Navigate to `Content/Materials/` folder (create if it doesn't exist)
3. Right-click in Content Browser
4. Select **Miscellaneous → Data Asset**
5. Choose **MaterialDataAsset** from the list
6. Name your asset: `DA_Material_[MaterialName]`
7. Open and configure the asset

---

## Material Properties

### Material Info Category

#### Material Name (Text)
- **Purpose**: Display name shown to players
- **Guidelines**: 
  - Keep it clear and concise (2-3 words max)
  - Use proper capitalization
  - Make it descriptive
- **Examples**: "Iron Ore", "Quantum Processor", "Reactor Core"

#### Description (Multi-line Text)
- **Purpose**: Brief explanation of the material and its uses
- **Guidelines**:
  - 1-3 sentences
  - Explain what it is and why it matters
  - Include flavor text if desired
- **Example**: 
  ```
  Advanced fusion reactor core. Primary power source for capital 
  ships and large stations. Handle with extreme caution.
  ```

#### Category (Enum)
- **Purpose**: Classify material type for filtering and gameplay
- **Options**:
  - `Mineral`: Raw ores and unprocessed resources
  - `RefinedMineral`: Processed metals and refined resources
  - `Component`: Manufactured mechanical parts
  - `Electronics`: Tech components and circuits
  - `ConstructionPart`: Station/base building materials
  - `ShipPart`: Spacecraft-specific components
  - `Food`: Consumable nutrition
  - `Research`: Scientific data and samples
  - `Organic`: Biological materials
  - `Synthetic`: Manufactured substances
  - `Other`: Everything else

### Material Storage Category

#### Storage Type (Enum)
- **Purpose**: Defines storage requirements and handling
- **Options**:
  - `Solid`: Standard solid materials (most common)
  - `Liquid`: Fluids requiring containment
  - `Gas`: Gaseous materials (requires pressurized storage)
  - `Refrigerated`: Temperature-sensitive items
  - `Hazardous`: Dangerous materials requiring special handling
  - `Other`: Special cases
- **Impact**: Affects storage volume calculations and module requirements

### Material Stats Category

#### Rarity (Enum)
- **Purpose**: Indicates how rare and valuable the material is
- **Options**:
  - `Common`: Widely available, low value
  - `Uncommon`: Less common, moderate value
  - `Rare`: Hard to find, high value
  - `VeryRare`: Extremely scarce, very high value
  - `Legendary`: Unique or nearly impossible to obtain
- **Guidelines**: Balance rarity with value and gameplay impact

#### Value (Float)
- **Purpose**: Base price per unit in credits
- **Guidelines**:
  - Common: 1-50 credits
  - Uncommon: 50-200 credits
  - Rare: 200-1000 credits
  - VeryRare: 1000-5000 credits
  - Legendary: 5000+ credits
- **Notes**: 
  - Consider balance with acquisition difficulty
  - Account for material utility in crafting
  - Can be modified by market conditions

#### Mass (Float)
- **Purpose**: Weight per unit in kilograms
- **Guidelines**:
  - Small electronics: 0.1-1 kg
  - Components: 1-10 kg
  - Construction parts: 10-100 kg
  - Large ship parts: 100-500 kg
- **Impact**: Affects cargo capacity and storage volume

### Material Tags Category

#### Tags (Array of Names)
- **Purpose**: General gameplay identifiers for querying and filtering
- **Common Tags**:
  - `Metal`, `RawMaterial`, `Mining`
  - `Refined`, `Processed`
  - `Electronics`, `Tech`, `Advanced`
  - `Construction`, `Structural`
  - `Energy`, `Power`, `Fuel`
  - `Food`, `Consumable`
  - `Research`, `Science`, `Data`
  - `Organic`, `Biological`
  - `Synthetic`, `Manufactured`
  - `Hazardous`, `Dangerous`
  - `Alien`, `Artifact`, `Unique`
- **Usage**: Use in Blueprint to filter materials by characteristics

#### Module Tags (Array of Names)
- **Purpose**: Specifies which station modules can process/use this material
- **Common Module Tags**:
  - `CargoStorage`: General storage module
  - `Refinery`: Ore processing
  - `ManufacturingPlant`: Component manufacturing
  - `ElectronicsLab`: Electronics assembly
  - `ShipyardFacility`: Ship construction
  - `ConstructionBay`: Station building
  - `ResearchLab`: Scientific research
  - `MedicalBay`: Medical treatment
  - `Hydroponics`: Food production
  - `FuelStorage`: Fuel containment
  - `HazardStorage`: Hazardous materials
  - `RefrigeratedStorage`: Cold storage
  - `DataStorage`: Information storage
  - `SecureStorage`: Valuable items
  - `TechWorkshop`: Advanced tech work
  - `EngineeringBay`: Engineering work
  - `PowerPlant`: Power generation
  - `MiningProcessor`: Ore extraction
  - `AssemblyLine`: Mass production
- **Usage**: Connect materials to appropriate gameplay systems

---

## Creating New Materials

### Step-by-Step Process

#### 1. Plan Your Material
Before creating the asset, decide:
- What is it? (name and description)
- What category does it belong to?
- How is it obtained? (mining, crafting, trading)
- What is it used for? (crafting, trading, quests)
- How rare should it be?
- What's a fair price?
- How heavy is it?
- What modules process it?

#### 2. Create the Data Asset
1. In Content Browser, navigate to `Content/Materials/`
2. Right-click → Miscellaneous → Data Asset
3. Select **MaterialDataAsset**
4. Name it: `DA_Material_[MaterialName]`
   - Example: `DA_Material_TitaniumIngot`

#### 3. Configure Basic Info
1. Open your new asset
2. Set **Material Name**: The display name
3. Write **Description**: 1-3 sentences explaining it
4. Choose **Category**: Select the most appropriate type
5. Select **Storage Type**: How it must be stored

#### 4. Set Stats
1. Choose **Rarity**: Balance with availability
2. Set **Value**: Price per unit (see guidelines)
3. Set **Mass**: Weight per unit in kg

#### 5. Add Tags
1. **Tags**: Add descriptive gameplay tags
   - Start general, then add specific
   - Example: `Metal`, `Refined`, `Construction`
2. **Module Tags**: Add all modules that can use this material
   - Be generous - more modules = more gameplay options
   - Example: `CargoStorage`, `ConstructionBay`, `ManufacturingPlant`

#### 6. Save and Test
1. Save the asset (Ctrl+S)
2. Test in-game or in editor
3. Verify it appears in inventories/storage
4. Check that modules recognize it correctly

### Example: Creating "Steel Cable"

```
Material Name: Steel Cable
Description: Reinforced steel cable for load-bearing applications and 
             power transmission. Essential in station construction.
Category: ConstructionPart
Storage Type: Solid
Rarity: Common
Value: 40.0
Mass: 15.0
Tags: [Metal, Construction, Cable, Structural]
ModuleTags: [ConstructionBay, CargoStorage, ManufacturingPlant]
```

---

## Integration with Station Modules

### Module-Material Relationship
Materials and modules work together through Module Tags:
- Materials specify which modules can process them
- Modules filter materials by checking Module Tags
- This creates a flexible, data-driven system

### Usage in Blueprints

#### Check if Module Can Process Material
```
// In your Station Module Blueprint:
On Material Added to Storage:
  → Get Material Data Asset
  → Call CanBeProcessedByModule(Your Module Tag)
  → If True: Accept material
  → If False: Reject or redirect to appropriate module
```

#### Filter Materials by Category
```
// In your Storage UI Blueprint:
For Each Material in Inventory:
  → Get Material Data Asset
  → Call IsCategory(Desired Category)
  → If True: Display in filtered list
```

#### Check Storage Requirements
```
// In your Storage Management Blueprint:
On Material Storage Attempt:
  → Get Material Data Asset
  → Call RequiresSpecialStorage()
  → If True: Route to special storage
  → If False: Route to general cargo
  → Call GetStorageVolume() to check capacity
```

### Connecting to Existing Systems

#### Space Station Integration
Station modules can reference materials:
```cpp
// In your module's Blueprint or C++:
UPROPERTY(EditAnywhere, Category="Module")
TArray<UMaterialDataAsset*> AcceptedMaterials;

UPROPERTY(EditAnywhere, Category="Module")
TArray<UMaterialDataAsset*> ProducedMaterials;
```

#### Inventory System Integration
```cpp
// Inventory slot structure:
USTRUCT(BlueprintType)
struct FInventorySlot
{
    UPROPERTY()
    UMaterialDataAsset* Material;
    
    UPROPERTY()
    int32 Quantity;
};
```

#### Trading System Integration
Use material Value property for pricing:
```
Base Trade Price = Material.Value
Modified Price = Material.Value * (Faction Trade Modifier) * (Supply/Demand)
```

---

## Example Workflows

### Workflow 1: Mining to Construction

1. **Create Raw Ore Material**
   - Name: "Iron Ore"
   - Category: Mineral
   - Module Tags: `MiningProcessor`, `Refinery`, `CargoStorage`

2. **Create Refined Material**
   - Name: "Iron Ingot"
   - Category: RefinedMineral
   - Module Tags: `ConstructionBay`, `ManufacturingPlant`, `CargoStorage`

3. **Create Construction Part**
   - Name: "Support Beam"
   - Category: ConstructionPart
   - Module Tags: `ConstructionBay`, `CargoStorage`

4. **Set Up Processing Chain**
   - Mining Module → Outputs Iron Ore
   - Refinery Module → Inputs Iron Ore, Outputs Iron Ingot
   - Construction Module → Inputs Iron Ingot, Outputs Support Beam

### Workflow 2: Food Production

1. **Create Seed Material** (if needed)
   - Module Tags: `Hydroponics`, `CargoStorage`

2. **Create Fresh Food**
   - Category: Food
   - Storage Type: Refrigerated
   - Module Tags: `Hydroponics`, `RefrigeratedStorage`, `FoodStorage`

3. **Create Processed Food**
   - Category: Food
   - Storage Type: Solid (longer shelf life)
   - Module Tags: `ManufacturingPlant`, `CargoStorage`, `FoodStorage`

### Workflow 3: Research Progression

1. **Create Research Sample**
   - Category: Research
   - Rarity: Rare or VeryRare
   - Module Tags: `ResearchLab`, `SecureStorage`

2. **Create Research Data**
   - Category: Research
   - Mass: Very low (digital)
   - Module Tags: `ResearchLab`, `DataStorage`

3. **Create Prototype Technology**
   - Category: Electronics or ShipPart
   - Rarity: VeryRare
   - Module Tags: `ResearchLab`, `ManufacturingPlant`

---

## Best Practices

### Naming Conventions
- **Asset Names**: `DA_Material_[MaterialName]` (no spaces)
- **Display Names**: Proper capitalization, clear and concise
- **Tags**: PascalCase, descriptive
- **Module Tags**: Match your module naming system

### Balancing Guidelines

#### Value Balance
- Consider acquisition difficulty
- Factor in utility (what can be made from it?)
- Account for demand in gameplay
- Compare with similar materials

#### Mass Balance
- Realistic proportions matter for immersion
- Heavy materials limit cargo capacity
- Balance risk vs. reward for transport

#### Rarity Balance
- Don't overuse Legendary or VeryRare
- Common materials should be truly common
- Create a progression: Common → Rare → Legendary

### Organization Tips
- **Folder Structure**: 
  ```
  Content/Materials/
    ├── Minerals/
    ├── Components/
    ├── Electronics/
    ├── Food/
    ├── Research/
    └── ShipParts/
  ```
- **Naming**: Consistent prefixes help sorting
- **Documentation**: Comment complex materials
- **Templates**: Use YAML templates as starting points

### Tag Management
- **Be Consistent**: Use the same tag names across materials
- **Don't Over-tag**: 3-6 tags per material is usually enough
- **Module Tags**: Include `CargoStorage` for most materials
- **Create a Tag Dictionary**: Document all tags used

---

## Playtesting Guidelines

### What to Test

#### 1. Material Accessibility
- [ ] Can players obtain this material?
- [ ] Is the acquisition method clear?
- [ ] Does rarity match actual availability?

#### 2. Value Balance
- [ ] Is the price fair for the effort to obtain?
- [ ] Can players make reasonable profit trading it?
- [ ] Does value scale with rarity?

#### 3. Storage Integration
- [ ] Do storage modules accept the material?
- [ ] Are special storage requirements enforced?
- [ ] Is volume calculation reasonable?

#### 4. Module Integration
- [ ] Do intended modules recognize the material?
- [ ] Do module tags work correctly?
- [ ] Can the material be processed as intended?

#### 5. Gameplay Flow
- [ ] Does the material fit into crafting chains?
- [ ] Is there demand for this material?
- [ ] Does it create interesting gameplay decisions?

### Common Issues and Fixes

#### Material Not Appearing in Module
**Problem**: Module doesn't recognize material
**Fix**: Check Module Tags - add the correct module tag to material

#### Incorrect Storage Volume
**Problem**: Material takes too much/little space
**Fix**: Adjust Mass property or check Storage Type

#### Unrealistic Pricing
**Problem**: Material value doesn't match gameplay
**Fix**: Compare with similar materials, adjust Value property

#### Missing from Inventory
**Problem**: Material doesn't show in UI
**Fix**: Check Category is set correctly, verify UI filters

---

## Quick Reference

### Blueprint Function Reference

```cpp
// Query functions available on UMaterialDataAsset:

bool HasTag(FName Tag)
// Check if material has a specific tag

bool CanBeProcessedByModule(FName ModuleTag)
// Check if material works with a module type

TArray<FName> GetTags()
// Get all tags

TArray<FName> GetModuleTags()
// Get all module tags

float GetStorageVolume()
// Calculate required storage space

bool IsCategory(EMaterialCategory CheckCategory)
// Check if material is of a specific category

bool RequiresSpecialStorage()
// Check if special storage is needed
```

### Material Categories Enum
```cpp
EMaterialCategory::Mineral
EMaterialCategory::RefinedMineral
EMaterialCategory::Component
EMaterialCategory::Electronics
EMaterialCategory::ConstructionPart
EMaterialCategory::ShipPart
EMaterialCategory::Food
EMaterialCategory::Research
EMaterialCategory::Organic
EMaterialCategory::Synthetic
EMaterialCategory::Other
```

### Storage Types Enum
```cpp
EStorageType::Solid
EStorageType::Liquid
EStorageType::Gas
EStorageType::Refrigerated
EStorageType::Hazardous
EStorageType::Other
```

### Rarity Enum
```cpp
EMaterialRarity::Common
EMaterialRarity::Uncommon
EMaterialRarity::Rare
EMaterialRarity::VeryRare
EMaterialRarity::Legendary
```

---

## Additional Resources

### Related Documentation
- **Faction System Guide**: `Assets/BlueprintFactionAssetGuide.md`
- **Station Editor**: `Content/Blueprints/STATION_EDITOR_README.md`
- **Material Templates**: `Assets/MaterialTemplates.yaml`

### Example Assets
See `MaterialTemplates.yaml` for complete examples of:
- Raw minerals (Iron Ore, Titanium Ore)
- Refined materials (Iron Ingot, Steel Alloy)
- Components (Actuator, Power Cell)
- Electronics (Circuit Board, Quantum Processor)
- Construction parts (Hull Plate, Support Beam)
- Ship parts (Reactor Core, Thruster Assembly)
- Food (Protein Rations, Hydroponic Vegetables)
- Research items (Research Data, Alien Artifact)
- Organic materials (Bio Sample, Medical Supplies)
- Synthetic materials (Nanomaterial, Synthetic Fuel)

### Support
For questions or issues:
1. Check this documentation
2. Review example materials in `MaterialTemplates.yaml`
3. Examine existing faction system for similar patterns
4. Test in-editor with debug logging enabled

---

## Revision History
- **v1.0**: Initial material system documentation
- Material Data Asset system implementation
- Integration with station modules and storage
- Comprehensive templates and examples provided
