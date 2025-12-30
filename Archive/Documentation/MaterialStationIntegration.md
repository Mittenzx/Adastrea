# Material System Integration with Station Modules

## Overview
This guide explains how to integrate the Material Data Asset system with the existing Space Station and Module systems, enabling materials to flow through your station's storage, processing, and crafting systems.

---

## Architecture Overview

### System Components
```
Materials (UMaterialDataAsset)
    ↓ stored in
Storage Modules (ASpaceStationModule)
    ↓ attached to
Space Stations (ASpaceStation)
    ↓ owned by
Factions (UFactionDataAsset)
```

### Data Flow
```
1. Material created as Data Asset
2. Material added to Station Storage Module
3. Module checks compatibility via Module Tags
4. Storage calculates volume via GetStorageVolume()
5. Processing modules transform materials
6. Output materials stored in appropriate modules
```

---

## Integration Patterns

### Pattern 1: Storage Module with Material Capacity

#### C++ Class Extension
```cpp
// StorageModule.h
#include "Materials/MaterialDataAsset.h"

USTRUCT(BlueprintType)
struct FStoredMaterial
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialDataAsset* Material;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;
};

UCLASS()
class AStorageModule : public ASpaceStationModule
{
    GENERATED_BODY()
    
public:
    // Total storage capacity in cubic meters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storage")
    float StorageCapacity;
    
    // Currently stored materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storage")
    TArray<FStoredMaterial> StoredMaterials;
    
    // Module tag for filtering compatible materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storage")
    FName ModuleStorageTag;
    
    // Add material to storage
    UFUNCTION(BlueprintCallable, Category="Storage")
    bool AddMaterial(UMaterialDataAsset* Material, int32 Quantity);
    
    // Remove material from storage
    UFUNCTION(BlueprintCallable, Category="Storage")
    bool RemoveMaterial(UMaterialDataAsset* Material, int32 Quantity);
    
    // Get current storage usage
    UFUNCTION(BlueprintCallable, Category="Storage")
    float GetCurrentStorageUsed() const;
    
    // Check if material can be stored
    UFUNCTION(BlueprintCallable, Category="Storage")
    bool CanStoreMaterial(UMaterialDataAsset* Material, int32 Quantity) const;
};
```

#### Implementation Example
```cpp
// StorageModule.cpp
bool AStorageModule::CanStoreMaterial(UMaterialDataAsset* Material, int32 Quantity) const
{
    if (!Material) return false;
    
    // Check if module can handle this material
    if (!Material->CanBeProcessedByModule(ModuleStorageTag))
        return false;
    
    // Calculate volume needed
    float VolumeNeeded = Material->GetStorageVolume() * Quantity;
    float CurrentUsed = GetCurrentStorageUsed();
    
    // Check capacity
    return (CurrentUsed + VolumeNeeded) <= StorageCapacity;
}

bool AStorageModule::AddMaterial(UMaterialDataAsset* Material, int32 Quantity)
{
    if (!CanStoreMaterial(Material, Quantity))
        return false;
    
    // Find existing entry or add new one
    for (FStoredMaterial& Stored : StoredMaterials)
    {
        if (Stored.Material == Material)
        {
            Stored.Quantity += Quantity;
            return true;
        }
    }
    
    // Add new entry
    FStoredMaterial NewEntry;
    NewEntry.Material = Material;
    NewEntry.Quantity = Quantity;
    StoredMaterials.Add(NewEntry);
    
    return true;
}

float AStorageModule::GetCurrentStorageUsed() const
{
    float TotalUsed = 0.0f;
    
    for (const FStoredMaterial& Stored : StoredMaterials)
    {
        if (Stored.Material)
        {
            TotalUsed += Stored.Material->GetStorageVolume() * Stored.Quantity;
        }
    }
    
    return TotalUsed;
}
```

---

### Pattern 2: Processing Module (Refinery Example)

#### Blueprint-Friendly Processing
```cpp
// RefineryModule.h
USTRUCT(BlueprintType)
struct FRefineryRecipe
{
    GENERATED_BODY()
    
    // Input material
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialDataAsset* InputMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InputQuantity;
    
    // Output material
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialDataAsset* OutputMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OutputQuantity;
    
    // Processing time in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProcessingTime;
};

UCLASS()
class ARefineryModule : public ASpaceStationModule
{
    GENERATED_BODY()
    
public:
    // Available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Refinery")
    TArray<FRefineryRecipe> Recipes;
    
    // Check if recipe is valid
    UFUNCTION(BlueprintCallable, Category="Refinery")
    bool CanProcessRecipe(FRefineryRecipe Recipe) const;
    
    // Start processing
    UFUNCTION(BlueprintCallable, Category="Refinery")
    bool StartProcessing(FRefineryRecipe Recipe);
};
```

---

### Pattern 3: Faction-Restricted Materials

#### Connecting Factions and Materials
```cpp
// In your station or module Blueprint:

// Check if faction can access material
bool CanFactionUseMaterial(UFactionDataAsset* Faction, UMaterialDataAsset* Material)
{
    // Example: Check tech level requirement
    if (Material->HasTag(FName("Advanced")))
    {
        return Faction->TechnologyLevel >= 7;
    }
    
    // Example: Faction-specific materials
    if (Material->HasTag(FName("MilitaryGrade")))
    {
        return Faction->HasTrait(FName("MilitaryIndustrial"));
    }
    
    return true;
}
```

---

## Blueprint Implementation Examples

### Example 1: Storage UI with Material Filtering

```
Event: On UI Opened
↓
Get Station Reference
↓
Get All Storage Modules
↓
For Each Storage Module:
    ↓
    Get Stored Materials Array
    ↓
    For Each Stored Material:
        ↓
        Get Material Data Asset
        ↓
        Branch: IsCategory(Desired Category)?
        ↓ TRUE
        Add to Filtered List
        ↓
        Create UI Widget (Material Name, Quantity, Icon)
```

### Example 2: Auto-Routing Materials to Correct Storage

```
Event: Material Delivered to Station
↓
Get Material Data Asset
↓
Call RequiresSpecialStorage()
↓
Branch:
    ↓ TRUE - Special Storage Needed
    Get Material Storage Type
    ↓
    Switch on Storage Type:
        • Refrigerated → Find RefrigeratedStorage Module
        • Hazardous → Find HazardStorage Module
        • Gas → Find GasStorage Module
    ↓ FALSE - Standard Storage
    Find CargoStorage Module
↓
Call Module's AddMaterial(Material, Quantity)
```

### Example 3: Processing Chain

```
Event: Process Material
↓
Input: Raw Material (Iron Ore)
↓
Find Module: Has ModuleTag "Refinery"
↓
Check Module: CanProcessMaterial(Iron Ore)?
↓ TRUE
Remove from Input Storage
↓
Start Timer (Processing Time)
↓
On Timer Complete:
    ↓
    Create Output Material (Iron Ingot)
    ↓
    Find Output Storage: CanStoreMaterial(Iron Ingot)?
    ↓ TRUE
    Add to Output Storage
```

---

## Storage Module Types

### General Cargo Storage
```
Module Tag: CargoStorage
Capacity: High (1000+ cubic meters)
Accepts: Most solid materials
Special Requirements: None
```

### Refrigerated Storage
```
Module Tag: RefrigeratedStorage
Capacity: Medium (500 cubic meters)
Accepts: StorageType == Refrigerated
Special Requirements: Power consumption
Filter: Material->RequiresSpecialStorage() && StorageType == Refrigerated
```

### Hazardous Storage
```
Module Tag: HazardStorage
Capacity: Low (200 cubic meters)
Accepts: StorageType == Hazardous
Special Requirements: Shielding, safety protocols
Filter: Material->RequiresSpecialStorage() && StorageType == Hazardous
```

### Fuel Storage
```
Module Tag: FuelStorage
Capacity: Very High (2000+ cubic meters)
Accepts: HasTag("Fuel")
Special Requirements: Pressure systems
```

### Data Storage
```
Module Tag: DataStorage
Capacity: Very Low by mass (digital)
Accepts: Category == Research && HasTag("Data")
Special Requirements: Power, cooling
```

---

## Integration Checklist

### For Material Creators
- [ ] Set appropriate Category
- [ ] Set correct Storage Type
- [ ] Add CargoStorage to Module Tags (if applicable)
- [ ] Add specific module tags for processing
- [ ] Set realistic Mass for volume calculations
- [ ] Add descriptive Tags for filtering

### For Module Creators
- [ ] Define Module Tag (e.g., "Refinery")
- [ ] Add storage capacity property
- [ ] Implement CanStoreMaterial() check
- [ ] Filter materials by Module Tag
- [ ] Calculate storage usage with GetStorageVolume()
- [ ] Handle special storage requirements

### For Station Designers
- [ ] Include varied storage modules
- [ ] Balance storage capacities
- [ ] Connect processing chains
- [ ] Test material flow
- [ ] Verify special storage works
- [ ] Check faction restrictions

---

## Common Integration Patterns

### 1. Mining → Refining → Manufacturing
```
Mining Module (Produces Raw Ore)
    ↓ ModuleTag: MiningProcessor
    Output: Iron Ore (Category: Mineral)
    ↓ Store in Cargo
    
Refinery Module (Processes Ore)
    ↓ ModuleTag: Refinery
    Input: Iron Ore
    Output: Iron Ingot (Category: RefinedMineral)
    ↓ Store in Cargo
    
Manufacturing Module (Creates Parts)
    ↓ ModuleTag: ManufacturingPlant
    Input: Iron Ingot
    Output: Support Beam (Category: ConstructionPart)
    ↓ Store in Cargo or use immediately
```

### 2. Food Production Chain
```
Hydroponics Module
    ↓ ModuleTag: Hydroponics
    Output: Fresh Vegetables (StorageType: Refrigerated)
    ↓ Must go to RefrigeratedStorage
    
Food Storage Module
    ↓ ModuleTag: RefrigeratedStorage
    Stores: Fresh food items
    ↓ Available for consumption
```

### 3. Research Pipeline
```
Exploration/Discovery
    ↓ Acquire research samples
    Bio Sample (Category: Organic, StorageType: Refrigerated)
    ↓ Store in RefrigeratedStorage
    
Research Lab Module
    ↓ ModuleTag: ResearchLab
    Input: Bio Sample
    Output: Research Data (Category: Research, Mass: 0.1kg)
    ↓ Store in DataStorage
    
Technology Unlock
    ↓ Consume Research Data
    Unlock: New blueprints, technologies
```

---

## Performance Considerations

### Optimization Tips
1. **Cache Storage Queries**: Don't call GetStorageVolume() repeatedly
2. **Index by Module Tag**: Create lookup tables for fast module finding
3. **Batch Operations**: Process multiple materials at once
4. **Update Only When Changed**: Don't recalculate storage every frame

### Example Caching
```cpp
// Cache storage data
UPROPERTY()
TMap<FName, float> CachedStorageUsage;

void UpdateStorageCache()
{
    CachedStorageUsage.Empty();
    for (auto& Module : StorageModules)
    {
        CachedStorageUsage.Add(
            Module->ModuleStorageTag,
            Module->GetCurrentStorageUsed()
        );
    }
}
```

---

## Testing Your Integration

### Test Cases
1. **Storage Capacity**: Fill storage to capacity, verify rejection of overflow
2. **Special Storage**: Add hazardous material, verify routing to HazardStorage
3. **Processing**: Run full material chain, verify outputs
4. **UI Filtering**: Filter by category, verify correct display
5. **Volume Calculation**: Store gas material, verify large volume usage
6. **Module Tags**: Add material, verify only compatible modules accept it

### Debug Commands
```cpp
// Blueprint debug functions
UFUNCTION(BlueprintCallable, Category="Debug")
void PrintStorageStats()
{
    for (auto Module : GetStorageModules())
    {
        UE_LOG(LogTemp, Warning, TEXT("Module: %s"), *Module->GetName());
        UE_LOG(LogTemp, Warning, TEXT("Capacity: %.2f / %.2f"),
            Module->GetCurrentStorageUsed(),
            Module->StorageCapacity);
    }
}

UFUNCTION(BlueprintCallable, Category="Debug")
void PrintMaterialInfo(UMaterialDataAsset* Material)
{
    UE_LOG(LogTemp, Warning, TEXT("Material: %s"), *Material->MaterialName.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Volume: %.4f m³"), Material->GetStorageVolume());
    UE_LOG(LogTemp, Warning, TEXT("Special Storage: %s"),
        Material->RequiresSpecialStorage() ? TEXT("Yes") : TEXT("No"));
}
```

---

## Troubleshooting

### Material Won't Store
1. Check Module Tag compatibility
2. Verify storage capacity available
3. Check special storage requirements
4. Ensure material reference is valid

### Processing Not Working
1. Verify input material availability
2. Check module has correct Module Tag
3. Validate recipe configuration
4. Ensure output storage has capacity

### UI Not Showing Materials
1. Check category filters
2. Verify material references
3. Ensure storage modules are found
4. Check Blueprint variable types

---

## Advanced Topics

### Dynamic Recipe Generation
Create recipes at runtime based on available materials and faction tech level.

### Material Quality System
Extend materials with quality ratings affecting value and output quantities.

### Trade System Integration
Use material values and rarity for dynamic market pricing.

### Crafting Trees
Build complex dependency graphs showing material relationships.

---

## Reference

### Key Functions
```cpp
// UMaterialDataAsset
bool HasTag(FName Tag)
bool CanBeProcessedByModule(FName ModuleTag)
float GetStorageVolume()
bool IsCategory(EMaterialCategory CheckCategory)
bool RequiresSpecialStorage()

// Your Storage Module
bool CanStoreMaterial(UMaterialDataAsset* Material, int32 Quantity)
bool AddMaterial(UMaterialDataAsset* Material, int32 Quantity)
float GetCurrentStorageUsed()
```

### Common Module Tags
- `CargoStorage`, `FuelStorage`, `FoodStorage`
- `RefrigeratedStorage`, `HazardStorage`
- `Refinery`, `ManufacturingPlant`, `ElectronicsLab`
- `ShipyardFacility`, `ConstructionBay`, `ResearchLab`
- `Hydroponics`, `MedicalBay`, `PowerPlant`

---

**Next Steps**: Start with a simple storage module and gradually add processing capabilities!
