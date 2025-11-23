# Complete List of YAML Files to Convert to Data Assets

This document provides a comprehensive inventory of all YAML template files in the Adastrea project that need to be converted into Unreal Engine Data Assets.

## Summary Statistics

- **Total YAML Files**: 161
- **Categories**: 18 main categories
- **ProceduralContent Files**: 48 (subdir with generated content)
- **Manual Template Files**: 113

## Conversion Status

### Currently Supported by YAMLtoDataAsset.py
- ✅ Spaceships (SpaceshipDataAsset)
- ✅ Personnel (PersonnelDataAsset)
- ⚠️ Trading Items (Partial support mentioned)
- ⚠️ Markets (Partial support mentioned)
- ⚠️ Contracts (Partial support mentioned)
- ⚠️ Faction AI (Partial support mentioned)
- ⚠️ Materials (Partial support mentioned)

### Not Yet Implemented
- ❌ Station Modules
- ❌ Sectors
- ❌ Combat Systems
- ❌ Combat VFX
- ❌ Exploration Systems
- ❌ Inventory Items
- ❌ Audio Systems
- ❌ Quests
- ❌ Ship Customization
- ❌ Way Networks
- ❌ Tutorials
- ❌ Spaceship Particles
- ❌ Galaxy Systems

---

## 1. Spaceship Templates (24 files)
**Target Data Asset**: `USpaceshipDataAsset`  
**Output Path**: `/Game/Spaceships/DataAssets/`  
**Status**: ✅ Fully Supported

### Files:
1. `Assets/SpaceshipTemplates/Carrier_VanguardEscort.yaml`
2. `Assets/SpaceshipTemplates/Command_Sovereign.yaml`
3. `Assets/SpaceshipTemplates/Corvette_RaptorAssault.yaml`
4. `Assets/SpaceshipTemplates/Cruiser_LifelineMedical.yaml`
5. `Assets/SpaceshipTemplates/ExcavatorMining.yaml`
6. `Assets/SpaceshipTemplates/Fighter_ViperInterceptor.yaml`
7. `Assets/SpaceshipTemplates/Frigate_ShadowbladeStealth.yaml`
8. `Assets/SpaceshipTemplates/Gunship_Warhammer.yaml`
9. `Assets/SpaceshipTemplates/Luxury_Starliner.yaml`
10. `Assets/SpaceshipTemplates/MerchantTrader.yaml`
11. `Assets/SpaceshipTemplates/Mining_Excavator.yaml`
12. `Assets/SpaceshipTemplates/MittenzxMk1.yaml`
13. `Assets/SpaceshipTemplates/MultiRole_MittenzxMk1.yaml`
14. `Assets/SpaceshipTemplates/PathfinderScout.yaml`
15. `Assets/SpaceshipTemplates/Patrol_SentinelLaw.yaml`
16. `Assets/SpaceshipTemplates/Science_Odyssey.yaml`
17. `Assets/SpaceshipTemplates/Scout_Pathfinder.yaml`
18. `Assets/SpaceshipTemplates/SovereignCommand.yaml`
19. `Assets/SpaceshipTemplates/StarlinerLuxury.yaml`
20. `Assets/SpaceshipTemplates/Trading_Merchant.yaml`
21. `Assets/SpaceshipTemplates/Transport_BehemothFreighter.yaml`
22. `Assets/SpaceshipTemplates/Transport_GenesisColony.yaml`
23. `Assets/SpaceshipTemplates/Utility_PhoenixSalvage.yaml`
24. `Assets/SpaceshipTemplates/WarhammerGunship.yaml`

---

## 2. Personnel Templates (7 files)
**Target Data Asset**: `UPersonnelDataAsset`  
**Output Path**: `/Game/Personnel/DataAssets/`  
**Status**: ✅ Fully Supported

### Files:
1. `Assets/PersonnelTemplates/Captain.yaml`
2. `Assets/PersonnelTemplates/CrewMember_Generic.yaml`
3. `Assets/PersonnelTemplates/Engineer.yaml`
4. `Assets/PersonnelTemplates/Gunner.yaml`
5. `Assets/PersonnelTemplates/MedicalOfficer.yaml`
6. `Assets/PersonnelTemplates/Pilot.yaml`
7. `Assets/PersonnelTemplates/StationManager.yaml`

---

## 3. Trading System Templates (27 files)
**Target Data Asset**: `UTradeItemDataAsset`, `UMarketDataAsset`, `UContractDataAsset`  
**Output Path**: `/Game/Trading/DataAssets/`  
**Status**: ⚠️ Partially Supported

### Trade Items (23 files):
1. `Assets/TradingTemplates/TradeItem_AICores.yaml`
2. `Assets/TradingTemplates/TradeItem_AdvancedSensors.yaml`
3. `Assets/TradingTemplates/TradeItem_Artwork.yaml`
4. `Assets/TradingTemplates/TradeItem_ConstructionMaterials.yaml`
5. `Assets/TradingTemplates/TradeItem_Contraband.yaml`
6. `Assets/TradingTemplates/TradeItem_CopperOre.yaml`
7. `Assets/TradingTemplates/TradeItem_Electronics.yaml`
8. `Assets/TradingTemplates/TradeItem_FoodRations.yaml`
9. `Assets/TradingTemplates/TradeItem_Helium-3.yaml`
10. `Assets/TradingTemplates/TradeItem_IronOre.yaml`
11. `Assets/TradingTemplates/TradeItem_Jewelry.yaml`
12. `Assets/TradingTemplates/TradeItem_LuxuryBeverages.yaml`
13. `Assets/TradingTemplates/TradeItem_MedicalSupplies.yaml`
14. `Assets/TradingTemplates/TradeItem_Platinum.yaml`
15. `Assets/TradingTemplates/TradeItem_PowerCells.yaml`
16. `Assets/TradingTemplates/TradeItem_ProhibitedWeapons.yaml`
17. `Assets/TradingTemplates/TradeItem_QuantumProcessor.yaml`
18. `Assets/TradingTemplates/TradeItem_RareEarthElements.yaml`
19. `Assets/TradingTemplates/TradeItem_ShipComponents.yaml`
20. `Assets/TradingTemplates/TradeItem_SteelAlloy.yaml`
21. `Assets/TradingTemplates/TradeItem_SyntheticProtein.yaml`
22. `Assets/TradingTemplates/TradeItem_TitaniumOre.yaml`
23. `Assets/TradingTemplates/TradeItem_WaterIce.yaml`

### Markets (2 files):
1. `Assets/TradingTemplates/Market_BlackMarket.yaml`
2. `Assets/TradingTemplates/Market_FreePort.yaml`

### Contracts (2 files):
1. `Assets/TradingTemplates/Contract_BasicDelivery.yaml`
2. `Assets/TradingTemplates/Contract_Smuggling.yaml`

---

## 4. Station Module Templates (20 files)
**Target Data Asset**: `UStationModuleDataAsset` (needs implementation)  
**Output Path**: `/Game/Stations/Modules/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Core Modules (2 files):
1. `Assets/StationModuleTemplates/Core_CentralHub.yaml`
2. `Assets/StationModuleTemplates/Core_StructuralCore.yaml`

### Docking Modules (2 files):
3. `Assets/StationModuleTemplates/Docking_LargeDockingBay.yaml`
4. `Assets/StationModuleTemplates/Docking_SmallDockingPort.yaml`

### Power Modules (2 files):
5. `Assets/StationModuleTemplates/Power_FissionReactor.yaml`
6. `Assets/StationModuleTemplates/Power_SolarArray.yaml`

### Storage Modules (2 files):
7. `Assets/StationModuleTemplates/Storage_CargoBay.yaml`
8. `Assets/StationModuleTemplates/Storage_FuelDepot.yaml`

### Processing Modules (3 files):
9. `Assets/StationModuleTemplates/Processing_AsteroidProcessingUnit.yaml`
10. `Assets/StationModuleTemplates/Processing_FabricationWorkshop.yaml`
11. `Assets/StationModuleTemplates/Processing_ScienceLab.yaml`

### Defence Modules (2 files):
12. `Assets/StationModuleTemplates/Defence_PointDefenseTurret.yaml`
13. `Assets/StationModuleTemplates/Defence_ShieldGenerator.yaml`

### Habitation Modules (3 files):
14. `Assets/StationModuleTemplates/Habitation_CrewBarracks.yaml`
15. `Assets/StationModuleTemplates/Habitation_LuxuryQuarters.yaml`
16. `Assets/StationModuleTemplates/Habitation_StandardHabitationBlock.yaml`

### Public Modules (1 file):
17. `Assets/StationModuleTemplates/Public_Marketplace.yaml`

### Connection Modules (1 file):
18. `Assets/StationModuleTemplates/Connection_CrossroadsConnector.yaml`

### Utility Modules (2 files):
19. `Assets/StationModuleTemplates/Utility_CommunicationsRelay.yaml`
20. `Assets/StationModuleTemplates/Utility_MedicalBay.yaml`

---

## 5. Sector Templates (7 files)
**Target Data Asset**: `USectorDataAsset` (needs implementation)  
**Output Path**: `/Game/Sectors/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/SectorTemplates/ContestedTerritorySector.yaml`
2. `Assets/SectorTemplates/CoreSystemsSector.yaml`
3. `Assets/SectorTemplates/FrontierMiningSector.yaml`
4. `Assets/SectorTemplates/IndependentColoniesSector.yaml`
5. `Assets/SectorTemplates/MilitaryBorderSector.yaml`
6. `Assets/SectorTemplates/ResearchSector.yaml`
7. `Assets/SectorTemplates/TradeHubSector.yaml`

---

## 6. Way Network Templates (5 files)
**Target Data Asset**: `UWayDataAsset` or `UFactionDataAsset` (needs clarification)  
**Output Path**: `/Game/Factions/Ways/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/WayNetworkTemplates/CraftsmenCompact.yaml`
2. `Assets/WayNetworkTemplates/FrontierAlliance.yaml`
3. `Assets/WayNetworkTemplates/HonorCircle.yaml`
4. `Assets/WayNetworkTemplates/MerchantCoalition.yaml`
5. `Assets/WayNetworkTemplates/ScholarsSyndicate.yaml`

---

## 7. Combat System Templates (1 file)
**Target Data Asset**: `UWeaponDataAsset` (needs implementation)  
**Output Path**: `/Game/Combat/Weapons/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/CombatTemplates/WeaponTemplates.yaml`

---

## 8. Combat VFX Templates (4 files)
**Target Data Asset**: `UCombatVFXDataAsset` (needs implementation)  
**Output Path**: `/Game/VFX/Combat/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/CombatVFXTemplates/ImpactEffects_Standard.yaml`
2. `Assets/CombatVFXTemplates/WeaponVFX_EnergyLaser.yaml`
3. `Assets/CombatVFXTemplates/WeaponVFX_MissileLauncher.yaml`
4. `Assets/CombatVFXTemplates/WeaponVFX_PlasmaCannon.yaml`

---

## 9. Exploration System Templates (4 files)
**Target Data Asset**: Multiple types needed  
**Output Path**: `/Game/Exploration/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/ExplorationTemplates/AnomalyTemplates.yaml`
2. `Assets/ExplorationTemplates/DiscoveryTemplates.yaml`
3. `Assets/ExplorationTemplates/ScannableObjectTemplates.yaml`
4. `Assets/ExplorationTemplates/ScannerTemplates.yaml`

---

## 10. Inventory Templates (3 files)
**Target Data Asset**: `UInventoryItemDataAsset` (needs implementation)  
**Output Path**: `/Game/Inventory/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/InventoryTemplates/ConsumableItems.yaml`
2. `Assets/InventoryTemplates/ResourceItems.yaml`
3. `Assets/InventoryTemplates/WeaponItems.yaml`

---

## 11. Personnel AI Templates (3 files)
**Target Data Asset**: `UPersonnelLogic` or `UPersonnelAIDataAsset` (needs clarification)  
**Output Path**: `/Game/AI/Personnel/DataAssets/`  
**Status**: ⚠️ Partially Supported (mentioned but unclear)

### Files:
1. `Assets/PersonnelAITemplates/CuriousExplorer.yaml`
2. `Assets/PersonnelAITemplates/DisciplinedSecurity.yaml`
3. `Assets/PersonnelAITemplates/EmpatheticMedic.yaml`

---

## 12. Ship Customization Templates (3 files)
**Target Data Asset**: `UShipModuleDataAsset` (needs implementation)  
**Output Path**: `/Game/Ships/Customization/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/ShipCustomizationTemplates/DefensiveModules.yaml`
2. `Assets/ShipCustomizationTemplates/EngineModules.yaml`
3. `Assets/ShipCustomizationTemplates/WeaponModules.yaml`

---

## 13. Audio Templates (2 files)
**Target Data Asset**: `UAudioDataAsset` (needs implementation)  
**Output Path**: `/Game/Audio/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/AudioTemplates/MusicTrackTemplates.yaml`
2. `Assets/AudioTemplates/SoundEffectTemplates.yaml`

---

## 14. Tutorial Templates (1 file)
**Target Data Asset**: `UTutorialDataAsset` (needs implementation)  
**Output Path**: `/Game/Tutorials/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/TutorialTemplates/BasicTutorialSequences.yaml`

---

## 15. Spaceship Particle Templates (1 file)
**Target Data Asset**: `USpaceshipParticleDataAsset` (needs implementation)  
**Output Path**: `/Game/VFX/Ships/DataAssets/`  
**Status**: ❌ Not Yet Implemented

### Files:
1. `Assets/SpaceshipParticleTemplates/ParticleSystemTemplate.yaml`

---

## 16. Material Templates (1 file)
**Target Data Asset**: `UMaterialDataAsset`  
**Output Path**: `/Game/Materials/DataAssets/`  
**Status**: ⚠️ Partially Supported (mentioned in YAMLtoDataAsset.py)

### Files:
1. `Assets/MaterialTemplates.yaml`

---

## 17. Procedurally Generated Content (48 files)
**Note**: These are procedurally generated examples, may not need manual conversion

### Galaxy Systems (5 files):
1. `Assets/ProceduralContent/Galaxy/System_EpsilonSecundus.yaml`
2. `Assets/ProceduralContent/Galaxy/System_Lux19.yaml`
3. `Assets/ProceduralContent/Galaxy/System_NovaMajor.yaml`
4. `Assets/ProceduralContent/Galaxy/System_Theta546.yaml`
5. `Assets/ProceduralContent/Galaxy/System_ZetaRadiant.yaml`

### Personnel (20 files):
1. `Assets/ProceduralContent/Personnel/Captain_Captain_TylerWood.yaml`
2. `Assets/ProceduralContent/Personnel/CargoSpecialist_CargoSpecialist_DonnaS.Ward.yaml`
3. `Assets/ProceduralContent/Personnel/CargoSpecialist_CargoSpecialist_JasonGonzalez.yaml`
4. `Assets/ProceduralContent/Personnel/Communications_Communications_DanielBailey.yaml`
5. `Assets/ProceduralContent/Personnel/Communications_Communications_RobertLewis.yaml`
6. `Assets/ProceduralContent/Personnel/Engineer_Engineer_PaulTurner.yaml`
7. `Assets/ProceduralContent/Personnel/Engineer_Engineer_RonaldPrice.yaml`
8. `Assets/ProceduralContent/Personnel/Gunner_Gunner_BettyDiaz.yaml`
9. `Assets/ProceduralContent/Personnel/Gunner_Gunner_SusanWright.yaml`
10. `Assets/ProceduralContent/Personnel/Navigator_Navigator_KarenEvans.yaml`
11. `Assets/ProceduralContent/Personnel/Navigator_Navigator_SusanH.Moore.yaml`
12. `Assets/ProceduralContent/Personnel/Scientist_Scientist_AaronStewart.yaml`
13. `Assets/ProceduralContent/Personnel/Scientist_Scientist_JoshuaM.Reyes.yaml`
14. `Assets/ProceduralContent/Personnel/SecurityOfficer_SecurityOfficer_LindaLopez.yaml`
15. `Assets/ProceduralContent/Personnel/SecurityOfficer_SecurityOfficer_PatrickC.Morgan.yaml`
16. `Assets/ProceduralContent/Personnel/SecurityOfficer_SecurityOfficer_ScottBell.yaml`
17. `Assets/ProceduralContent/Personnel/SensorOperator_SensorOperator_JosephBaker.yaml`
18. `Assets/ProceduralContent/Personnel/SensorOperator_SensorOperator_KennethEvans.yaml`
19. `Assets/ProceduralContent/Personnel/SensorOperator_SensorOperator_WilliamJackson.yaml`
20. `Assets/ProceduralContent/Personnel/Trader_Trader_DianeLee.yaml`

### Quests (15 files):
1. `Assets/ProceduralContent/Quests/Quest_Combat_8121.yaml`
2. `Assets/ProceduralContent/Quests/Quest_Delivery_4248.yaml`
3. `Assets/ProceduralContent/Quests/Quest_Delivery_5318.yaml`
4. `Assets/ProceduralContent/Quests/Quest_Escort_1379.yaml`
5. `Assets/ProceduralContent/Quests/Quest_Escort_4126.yaml`
6. `Assets/ProceduralContent/Quests/Quest_Escort_9552.yaml`
7. `Assets/ProceduralContent/Quests/Quest_Exploration_8039.yaml`
8. `Assets/ProceduralContent/Quests/Quest_Investigation_4346.yaml`
9. `Assets/ProceduralContent/Quests/Quest_Investigation_4605.yaml`
10. `Assets/ProceduralContent/Quests/Quest_Investigation_6100.yaml`
11. `Assets/ProceduralContent/Quests/Quest_Mining_6342.yaml`
12. `Assets/ProceduralContent/Quests/Quest_Procurement_6010.yaml`
13. `Assets/ProceduralContent/Quests/Quest_Procurement_6382.yaml`
14. `Assets/ProceduralContent/Quests/Quest_Rescue_9619.yaml`
15. `Assets/ProceduralContent/Quests/Quest_Rescue_9670.yaml`

### Ships (8 files):
1. `Assets/ProceduralContent/Ships/Fighter_EagleVoyager.yaml`
2. `Assets/ProceduralContent/Ships/Fighter_Eternal.yaml`
3. `Assets/ProceduralContent/Ships/Fighter_Hunter.yaml`
4. `Assets/ProceduralContent/Ships/Fighter_PhoenixPathfinder.yaml`
5. `Assets/ProceduralContent/Ships/Fighter_SilentExplorer.yaml`
6. `Assets/ProceduralContent/Ships/Fighter_Swift.yaml`
7. `Assets/ProceduralContent/Ships/Fighter_Tiger.yaml`
8. `Assets/ProceduralContent/Ships/Fighter_Valiant.yaml`

---

## 18. Population Templates (2 files)
**Note**: These are configuration files for scene population, not Data Assets

### Files:
1. `PopulationTemplates/example_scene_config.yaml`
2. `PopulationTemplates/station_interior_template.yaml`

---

## Conversion Priority Recommendations

### High Priority (Core Gameplay)
1. ✅ **Spaceships** - Already supported, needs batch conversion
2. ✅ **Personnel** - Already supported, needs batch conversion
3. ⚠️ **Trading Items** - Partially supported, needs completion
4. ❌ **Station Modules** - Critical for station building gameplay
5. ❌ **Sectors** - Critical for world building

### Medium Priority (Enhanced Gameplay)
6. ❌ **Combat System** - Weapon templates
7. ❌ **Combat VFX** - Visual effects for combat
8. ❌ **Ship Customization** - Module systems
9. ❌ **Inventory Items** - Player inventory management
10. ❌ **Way Networks** - Faction system integration

### Low Priority (Polish & Content)
11. ❌ **Exploration Systems** - Anomalies, discoveries, scanners
12. ❌ **Audio Templates** - Sound and music
13. ❌ **Tutorial Templates** - Tutorial sequences
14. ❌ **Spaceship Particles** - Ship VFX
15. ⚠️ **Personnel AI** - AI behavior templates
16. ⚠️ **Materials** - Material system

### Optional (Generated Content)
17. **ProceduralContent** - These are examples, may regenerate as needed

---

## Next Steps

### For Developers

1. **Complete YAMLtoDataAsset.py Implementation**
   - Finish Trading system import (Items, Markets, Contracts)
   - Add Station Module import support
   - Add Sector import support
   - Add Combat system import support
   - Add other system imports as prioritized

2. **Create Missing Data Asset Classes**
   - `UStationModuleDataAsset`
   - `USectorDataAsset`
   - `UWeaponDataAsset`
   - `UCombatVFXDataAsset`
   - `UInventoryItemDataAsset`
   - `UShipModuleDataAsset`
   - `UAudioDataAsset`
   - `UTutorialDataAsset`
   - Other specialized data assets

3. **Batch Conversion**
   - Use `YAMLtoDataAsset.batch_import_spaceships()` for all ships
   - Use `YAMLtoDataAsset.batch_import_personnel()` for all personnel
   - Create similar batch functions for each category

### For Designers

1. **Review Existing YAML Templates**
   - Ensure all YAML files are up-to-date
   - Fix any formatting issues
   - Add missing properties

2. **Test Imported Assets**
   - Verify imported Data Assets work correctly
   - Check property values transferred correctly
   - Report any conversion issues

3. **Create New Content**
   - Use YAML templates as starting point
   - Follow existing naming conventions
   - Document any new properties needed

---

## Usage Instructions

### Importing Spaceships
```python
import YAMLtoDataAsset
YAMLtoDataAsset.batch_import_spaceships()
```

### Importing Personnel
```python
import YAMLtoDataAsset
YAMLtoDataAsset.batch_import_personnel()
```

### Importing Single File
```python
import YAMLtoDataAsset
YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Scout_Pathfinder.yaml")
```

---

## Document Maintenance

**Last Updated**: 2025-11-23  
**Total Files Inventoried**: 161  
**Version**: 1.0

### Update Instructions
When adding new YAML templates:
1. Add to appropriate category section
2. Update summary statistics
3. Update conversion status if importer is updated
4. Update document version and date
