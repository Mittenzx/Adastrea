# Material Data Asset System - Quick Start Guide

## 5-Minute Setup

### What You'll Learn
- How to create your first material in under 5 minutes
- How to connect materials to station modules
- Where to find examples and templates

---

## Create Your First Material

### Step 1: Create the Data Asset (30 seconds)
1. Open Unreal Engine and your project
2. In Content Browser, navigate to `Content/Materials/` (create folder if needed)
3. Right-click → **Miscellaneous** → **Data Asset**
4. Select **MaterialDataAsset** from the list
5. Name it: `DA_Material_IronOre` (or your material name)

### Step 2: Configure Basic Properties (2 minutes)
Open your new asset and fill in:

```
Material Name: Iron Ore
Description: Common metallic ore found throughout the galaxy.
Category: Mineral
Storage Type: Solid
Rarity: Common
Value: 10.0
Mass: 5.0
```

### Step 3: Add Tags (1 minute)
Click the **+** button next to Tags array and add:
- `Metal`
- `RawMaterial`
- `Mining`

### Step 4: Add Module Tags (1 minute)
Click the **+** button next to Module Tags array and add:
- `Refinery`
- `CargoStorage`
- `MiningProcessor`

### Step 5: Save and Test (30 seconds)
- Press `Ctrl+S` to save
- Your material is now ready to use!

---

## Using Materials in Blueprints

### Check if Material Has a Tag
```
Get Material Data Asset
↓
HasTag (Tag: "Metal")
↓
Branch (True/False)
```

### Check if Module Can Process Material
```
Get Material Data Asset
↓
CanBeProcessedByModule (ModuleTag: "Refinery")
↓
Branch (True/False)
```

### Get Storage Volume Needed
```
Get Material Data Asset
↓
GetStorageVolume
↓
Return Float (cubic meters)
```

---

## Common Material Patterns

### Raw Resource (Mining)
```yaml
Category: Mineral
StorageType: Solid
Rarity: Common/Uncommon
Tags: [RawMaterial, Mining, Metal/Crystal/Gas]
ModuleTags: [MiningProcessor, Refinery, CargoStorage]
```

### Refined Material
```yaml
Category: RefinedMineral
StorageType: Solid
Rarity: Common/Uncommon
Tags: [Refined, Construction, Manufacturing]
ModuleTags: [ConstructionBay, ManufacturingPlant, CargoStorage]
```

### Component/Part
```yaml
Category: Component
StorageType: Solid
Rarity: Uncommon/Rare
Tags: [Mechanical, Electronics, Precision]
ModuleTags: [ManufacturingPlant, AssemblyLine, CargoStorage]
```

### Food Item
```yaml
Category: Food
StorageType: Solid/Refrigerated
Rarity: Common
Tags: [Food, Consumable, Organic/Synthetic]
ModuleTags: [FoodStorage, RefrigeratedStorage, Hydroponics]
```

### Ship Part
```yaml
Category: ShipPart
StorageType: Solid/Hazardous
Rarity: Rare/VeryRare
Tags: [Ship, Advanced, Power/Propulsion/Defense]
ModuleTags: [ShipyardFacility, EngineeringBay, CargoStorage]
```

---

## Value Pricing Guide

Quick reference for setting material values:

| Rarity | Value Range | Examples |
|--------|-------------|----------|
| Common | 1-50 | Iron ore (10), Protein rations (5), Polymer (30) |
| Uncommon | 50-200 | Titanium ore (150), Actuator (120), Sensor array (220) |
| Rare | 200-1000 | Research data (500), Shield generator (1800) |
| VeryRare | 1000-5000 | Reactor core (5000), Quantum processor (2500) |
| Legendary | 5000+ | Alien artifact (10000), Unique items |

---

## Most Common Module Tags

Include these in your materials as needed:

**Storage:**
- `CargoStorage` - General storage (include for most materials!)
- `FuelStorage` - Fuel containment
- `FoodStorage` - Food storage
- `RefrigeratedStorage` - Cold storage
- `HazardStorage` - Dangerous materials
- `SecureStorage` - Valuables
- `DataStorage` - Digital information

**Processing:**
- `Refinery` - Ore processing
- `ManufacturingPlant` - Component manufacturing
- `ElectronicsLab` - Electronics assembly
- `ConstructionBay` - Construction materials
- `ShipyardFacility` - Ship construction
- `ResearchLab` - Scientific research
- `Hydroponics` - Food production
- `MedicalBay` - Medical supplies

**Specialized:**
- `MiningProcessor` - Raw ore extraction
- `PowerPlant` - Power generation
- `EngineeringBay` - Engineering work
- `AssemblyLine` - Mass production
- `TechWorkshop` - Advanced tech

---

## Examples to Copy

### Example 1: Simple Resource
```
Name: Copper Ore
Description: Common conductive metal ore.
Category: Mineral
Storage: Solid
Rarity: Common
Value: 8.0
Mass: 5.5
Tags: [Metal, RawMaterial, Mining, Conductive]
ModuleTags: [Refinery, CargoStorage, MiningProcessor]
```

### Example 2: Advanced Component
```
Name: Fusion Coil
Description: High-energy containment coil for fusion reactors.
Category: Electronics
Storage: Solid
Rarity: Rare
Value: 800.0
Mass: 12.0
Tags: [Electronics, Energy, Advanced, Reactor]
ModuleTags: [ElectronicsLab, EngineeringBay, CargoStorage]
```

### Example 3: Hazardous Material
```
Name: Radioactive Isotope
Description: Unstable radioactive material. Handle with care.
Category: Research
Storage: Hazardous
Rarity: VeryRare
Value: 3000.0
Mass: 1.0
Tags: [Radioactive, Hazardous, Research, Nuclear]
ModuleTags: [ResearchLab, HazardStorage]
```

---

## Troubleshooting

### Material Doesn't Show in Module
**Problem**: Created material but module won't accept it  
**Solution**: Add the module's tag to your material's Module Tags array

### Incorrect Storage Volume
**Problem**: Material takes up wrong amount of space  
**Solution**: Adjust the Mass property (1kg ≈ 0.001 cubic meters for solids)

### Can't Find MaterialDataAsset in List
**Problem**: MaterialDataAsset doesn't appear when creating Data Asset  
**Solution**: Make sure project is compiled. Build the C++ project first.

---

## Next Steps

### Learn More
- **Full Guide**: [MaterialSystemGuide.md](MaterialSystemGuide.md) - Complete documentation
- **Templates**: [MaterialTemplates.yaml](MaterialTemplates.yaml) - 25+ examples
- **Faction System**: [BlueprintFactionAssetGuide.md](BlueprintFactionAssetGuide.md) - Similar pattern

### Expand Your System
1. Create a full material chain (ore → ingot → component → part)
2. Set up processing modules that consume/produce materials
3. Implement trading system using material values
4. Create inventory UI that filters by category
5. Add crafting recipes that combine materials

### Join the Community
- Check existing materials for consistency
- Document your custom module tags
- Share your material templates with team
- Test storage and processing integration

---

## Cheat Sheet

### Property Checklist
- [ ] Material Name (display name)
- [ ] Description (1-3 sentences)
- [ ] Category (11 options)
- [ ] Storage Type (6 options)
- [ ] Rarity (5 levels)
- [ ] Value (price in credits)
- [ ] Mass (weight in kg)
- [ ] Tags (3-6 gameplay tags)
- [ ] Module Tags (2-5 compatible modules)

### Blueprint Functions
| Function | Purpose |
|----------|---------|
| `HasTag` | Check for tag |
| `CanBeProcessedByModule` | Check module compatibility |
| `GetTags` | Get all tags |
| `GetModuleTags` | Get all module tags |
| `GetStorageVolume` | Calculate space needed |
| `IsCategory` | Check category type |
| `RequiresSpecialStorage` | Check special requirements |

---

**Ready to create more materials?** Use the templates in `MaterialTemplates.yaml` as starting points!
