# Blueprint Class Tree Diagram - Adastrea

**Visual Reference**: Complete class hierarchy showing all blueprints and their relationships.

---

## Complete Blueprint Tree

```
Unreal Engine 5 Core Classes
│
├─ AActor
│  │
│  ├─ AGameModeBase
│  │  └─ AdastreaGameMode (C++)
│  │     ├─ BP_SpaceGameMode ✅
│  │     └─ BP_TestGameMode ⏳
│  │
│  ├─ APlayerController
│  │  └─ AdastreaPlayerController (C++)
│  │     └─ BP_SpaceshipController ⏳
│  │
│  ├─ AAIController
│  │  └─ BP_AIController ✅
│  │
│  ├─ AGameStateBase
│  │  └─ AdastreaGameState (C++)
│  │     └─ BP_GameState ⏳
│  │
│  ├─ APawn
│  │  └─ ASpaceship (C++) [IDamageable, ITargetable, IFactionMember]
│  │     ├─ BP_PlayerShip ✅
│  │     ├─ BP_PlayerShip_Fighter ⏳
│  │     ├─ BP_PlayerShip_Trader ⏳
│  │     ├─ BP_Ship_Corvette ✅
│  │     ├─ BP_Ship_Cruiser ✅
│  │     ├─ BP_Ship_Destroyer ✅
│  │     ├─ BP_Ship_Freighter ✅
│  │     ├─ BP_Ship_Fighter ✅
│  │     ├─ BP_Battleship ✅
│  │     ├─ BP_Ship_NPC_Fighter ⏳
│  │     ├─ BP_Ship_NPC_Trader ⏳
│  │     └─ BP_Ship_NPC_Interceptor ⏳
│  │
│  ├─ ASpaceStation (C++) [IDamageable, ITargetable, IFactionMember]
│  │  ├─ BP_SpaceStation ✅
│  │  ├─ BP_SpaceStation_Trading ⏳ ⭐ MVP
│  │  ├─ BP_SpaceStation_Military ⏳
│  │  └─ BP_SpaceStation_Research ⏳
│  │
│  ├─ ASpaceStationModule (C++) [IDamageable, ITargetable]
│  │  │
│  │  ├─ ADockingBayModule (C++) [Power: -50]
│  │  │  └─ BP_Module_DockingBay ⏳ ⭐ MVP
│  │  │
│  │  ├─ ADockingPortModule (C++) [Power: -10]
│  │  │  └─ BP_Module_DockingPort ⏳ ⭐ MVP
│  │  │
│  │  ├─ AReactorModule (C++) [Power: +100]
│  │  │  └─ BP_Module_Reactor ⏳
│  │  │
│  │  ├─ ASolarArrayModule (C++) [Power: +30]
│  │  │  └─ BP_Module_SolarArray ⏳
│  │  │
│  │  ├─ ACargoBayModule (C++) [Power: -5]
│  │  │  └─ BP_Module_CargoBay ✅
│  │  │
│  │  ├─ AFuelDepotModule (C++) [Power: -5]
│  │  │  └─ BP_Module_FuelDepot ⏳
│  │  │
│  │  ├─ ATurretModule (C++) [Power: -30]
│  │  │  └─ BP_Module_Turret ⏳
│  │  │
│  │  ├─ AShieldGeneratorModule (C++) [Power: -40]
│  │  │  └─ BP_Module_ShieldGenerator ⏳
│  │  │
│  │  ├─ AHabitationModule (C++) [Power: -20]
│  │  │  └─ BP_Module_Habitation ⏳
│  │  │
│  │  ├─ ABarracksModule (C++) [Power: -25]
│  │  │  └─ BP_Module_Barracks ⏳
│  │  │
│  │  ├─ AFabricationModule (C++) [Power: -35]
│  │  │  └─ BP_Module_Fabrication ⏳
│  │  │
│  │  ├─ AProcessingModule (C++) [Power: -40]
│  │  │  └─ BP_Module_Processing ⏳
│  │  │
│  │  ├─ AMarketplaceModule (C++) [Power: -15]
│  │  │  └─ BP_Module_Marketplace ⏳ ⭐ MVP
│  │  │
│  │  ├─ AScienceLabModule (C++) [Power: -25]
│  │  │  └─ BP_Module_ScienceLab ⏳
│  │  │
│  │  ├─ ACorridorModule (C++) [Power: -2]
│  │  │  └─ BP_Module_Corridor ⏳
│  │  │
│  │  └─ ASpaceStationModule (Generic)
│  │     ├─ BP_Module_CommandCenter ⏳
│  │     ├─ BP_Module_LifeSupport ⏳
│  │     └─ BP_Module_Recreation ⏳
│  │
│  ├─ ASpaceshipInterior (C++)
│  │  └─ BP_ShipInterior ⏳
│  │
│  ├─ AShipControlConsole (C++)
│  │  └─ BP_ShipControlConsole ⏳
│  │
│  ├─ Weapon Blueprints (Simple Actors)
│  │  ├─ BP_Weapon_Laser ✅
│  │  ├─ BP_Weapon_Missile ✅
│  │  ├─ BP_Weapon_Railgun ✅
│  │  └─ BP_Weapon_Plasma ✅
│  │
│  └─ Projectile Blueprints (Simple Actors)
│     ├─ BP_Projectile_Laser ⏳
│     ├─ BP_Projectile_Cannon ⏳
│     ├─ BP_Projectile_Missile ⏳
│     └─ BP_Projectile_Torpedo ⏳
│
├─ UGameInstance
│  └─ AdastreaGameInstance (C++)
│     └─ BP_GameInstance ✅
│
└─ UObject
   │
   ├─ UDataAsset
   │  │
   │  ├─ USpaceshipDataAsset (C++)
   │  │  ├─ DA_Ship_PlayerScout ⏳ ⭐ MVP
   │  │  ├─ DA_Ship_PlayerFighter ⏳
   │  │  ├─ DA_Ship_PlayerTrader ⏳ ⭐ MVP
   │  │  ├─ DA_Ship_NPCFighter ⏳
   │  │  ├─ DA_Ship_NPCTrader ⏳
   │  │  └─ DA_Ship_* (10+ more) ⏳
   │  │
   │  ├─ UWeaponDataAsset (C++)
   │  │  ├─ DA_Weapon_Laser ⏳
   │  │  ├─ DA_Weapon_Cannon ⏳
   │  │  ├─ DA_Weapon_Missile ⏳
   │  │  ├─ DA_Weapon_Torpedo ⏳
   │  │  ├─ DA_Weapon_Railgun ⏳
   │  │  └─ DA_Weapon_* (10+ more) ⏳
   │  │
   │  ├─ UFactionDataAsset (C++)
   │  │  ├─ DA_Faction_Player ⏳
   │  │  ├─ DA_Faction_Alliance ⏳
   │  │  ├─ DA_Faction_Merchants ⏳ ⭐ MVP
   │  │  ├─ DA_Faction_Pirates ⏳
   │  │  └─ DA_Faction_* (10+ more) ⏳
   │  │
   │  ├─ UTradeItemDataAsset (C++) ⭐ MVP CRITICAL
   │  │  ├─ DA_TradeItem_Food ⏳ ⭐
   │  │  ├─ DA_TradeItem_Water ⏳ ⭐
   │  │  ├─ DA_TradeItem_Minerals ⏳ ⭐
   │  │  ├─ DA_TradeItem_Metal ⏳ ⭐
   │  │  ├─ DA_TradeItem_Technology ⏳ ⭐
   │  │  ├─ DA_TradeItem_Fuel ⏳ ⭐
   │  │  └─ DA_TradeItem_* (20+ total) ⏳ ⭐
   │  │
   │  ├─ UMarketDataAsset (C++) ⭐ MVP CRITICAL
   │  │  ├─ DA_Market_Station1 ⏳ ⭐
   │  │  ├─ DA_Market_Station2 ⏳ ⭐
   │  │  ├─ DA_Market_Station3 ⏳ ⭐
   │  │  └─ DA_Market_* (10+ total) ⏳
   │  │
   │  ├─ UPersonnelDataAsset (C++)
   │  │  ├─ DA_Personnel_Captain ⏳
   │  │  ├─ DA_Personnel_Pilot ⏳
   │  │  ├─ DA_Personnel_Engineer ⏳
   │  │  └─ DA_Personnel_* (15+ total) ⏳
   │  │
   │  ├─ UQuestDataAsset (C++)
   │  │  ├─ DA_Quest_Tutorial ⏳
   │  │  └─ DA_Quest_* (10+ total) ⏳
   │  │
   │  ├─ UWayDataAsset (C++)
   │  │  └─ DA_Way_* (10+ total) ⏳
   │  │
   │  ├─ UHomeworldDataAsset (C++)
   │  │  └─ DA_Homeworld_* (5+ total) ⏳
   │  │
   │  ├─ UMaterialDataAsset (C++)
   │  │  └─ DA_Material_* (10+ total) ⏳
   │  │
   │  └─ UInputConfigDataAsset (C++)
   │     └─ DA_InputConfig ⏳
   │
   ├─ UNPCLogicBase (C++) - AI Base Class
   │  ├─ UFactionLogic (C++)
   │  │  └─ BP_FactionLogic_Standard ⏳
   │  │
   │  └─ UPersonnelLogic (C++)
   │     └─ BP_PersonnelLogic_Standard ⏳
   │
   ├─ UActorComponent
   │  │
   │  ├─ UFloatingPawnMovement (UE5) - Movement
   │  ├─ USpringArmComponent (UE5) - Camera
   │  ├─ UCameraComponent (UE5) - View
   │  ├─ UStaticMeshComponent (UE5) - Visual
   │  │
   │  ├─ UNavigationComponent (C++) - Autopilot
   │  ├─ UWeaponComponent (C++) - Weapons
   │  ├─ UTargetingComponent (C++) - Targeting
   │  ├─ UCombatHealthComponent (C++) - Health/Shields
   │  ├─ UPointDefenseComponent (C++) - Point Defense
   │  ├─ UBoardingComponent (C++) - Boarding
   │  ├─ UCargoComponent (C++) - Cargo ⭐ MVP
   │  ├─ UAITraderComponent (C++) - AI Trading
   │  ├─ UPlayerTraderComponent (C++) - Player Trading
   │  ├─ UPlayerReputationComponent (C++) - Reputation ⭐ MVP
   │  ├─ UPlayerProgressionComponent (C++) - Progression
   │  ├─ UPlayerUnlockComponent (C++) - Unlocks
   │  ├─ USpaceshipControlsComponent (C++) - Flight Controls
   │  ├─ UShipCustomizationComponent (C++) - Customization
   │  ├─ UShipUpgradeComponent (C++) - Upgrades
   │  ├─ UShipModuleComponent (C++) - Ship Modules
   │  ├─ USpaceshipParticleComponent (C++) - VFX
   │  ├─ USimpleAIMovementComponent (C++) - AI Movement
   │  ├─ UInventoryComponent (C++) - Inventory
   │  └─ UVerse (C++) - Guild Reputation
   │
   └─ UUserWidget
      │
      ├─ UAdastreaHUDWidget (C++)
      │  ├─ WBP_SpaceshipHUD ⏳ ⭐ MVP
      │  └─ WBP_CombatHUD ⏳
      │
      ├─ UTradingInterfaceWidget (C++) ⭐ MVP CRITICAL
      │  └─ WBP_TradingInterface ⏳ ⭐ MVP
      │
      ├─ UStationEditorWidget (C++)
      │  └─ WBP_StationEditor ⏳
      │
      ├─ UStationManagementWidget (C++)
      │  └─ WBP_StationManagement ⏳
      │
      ├─ UInventoryWidget (C++)
      │  └─ WBP_Inventory ⏳
      │
      ├─ UShipStatusWidget (C++)
      │  └─ WBP_ShipStatus ⏳
      │
      ├─ UShipCustomizationWidget (C++)
      │  └─ WBP_ShipCustomization ⏳
      │
      ├─ USectorMapWidget (C++)
      │  └─ WBP_SectorMap ⏳
      │
      ├─ UUniverseMapWidget (C++)
      │  └─ WBP_UniverseMap ⏳
      │
      └─ UUserWidget (Generic)
         ├─ WBP_MainMenu ⏳
         ├─ WBP_PauseMenu ⏳
         ├─ WBP_SettingsMenu ⏳
         ├─ WBP_QuestLog ⏳
         ├─ WBP_CrewManagement ⏳
         ├─ WBP_DockingInterface ⏳
         ├─ WBP_TargetingInfo ⏳
         └─ WBP_* (20+ more) ⏳
```

**Legend:**
- ✅ = Exists in repository
- ⏳ = Planned/needed
- ⭐ MVP = Critical for Trade Simulator MVP
- `(C++)` = C++ class (not Blueprint)
- `[Interface]` = Implements interface

---

## Interface Implementation Map

```
Interfaces:

IDamageable
├─ Implemented by:
│  ├─ ASpaceship (C++)
│  ├─ ASpaceStation (C++)
│  └─ ASpaceStationModule (C++)
└─ Functions:
   ├─ ApplyDamage()
   ├─ GetHealthPercentage()
   ├─ IsDestroyed()
   └─ CanTakeDamage()

ITargetable
├─ Implemented by:
│  ├─ ASpaceship (C++)
│  ├─ ASpaceStation (C++)
│  └─ ASpaceStationModule (C++)
└─ Functions:
   ├─ CanBeTargeted()
   ├─ GetTargetPriority()
   ├─ GetTargetDisplayName()
   ├─ GetAimPoint()
   └─ IsHostileToActor()

IFactionMember
├─ Implemented by:
│  ├─ ASpaceship (C++)
│  └─ ASpaceStation (C++)
└─ Functions:
   ├─ GetFaction()
   ├─ IsAlliedWith()
   ├─ IsHostileTo()
   ├─ GetRelationshipWith()
   └─ GetTradePriceModifier() ⭐ MVP

IShipModule (Future)
└─ Implemented by:
   └─ Ship module actors (when implemented)
```

---

## Component Attachment Map

```
ASpaceship (Actor)
├─ CORE Components (Always Present):
│  ├─ UFloatingPawnMovement (Movement)
│  ├─ UStaticMeshComponent (Visual)
│  ├─ USpaceshipParticleComponent (VFX)
│  ├─ USpringArmComponent (Camera positioning)
│  └─ UCameraComponent (Player view)
│
├─ COMBAT Components (Combat ships):
│  ├─ UWeaponComponent (Weapons)
│  ├─ UTargetingComponent (Targeting)
│  ├─ UCombatHealthComponent (Health/Shields)
│  └─ UPointDefenseComponent (Point defense)
│
├─ TRADING Components (Trader ships): ⭐ MVP
│  ├─ UCargoComponent (Cargo inventory) ⭐
│  └─ UAITraderComponent or UPlayerTraderComponent (Trading logic)
│
├─ NAVIGATION Components (All ships):
│  └─ UNavigationComponent (Autopilot, pathfinding)
│
├─ PLAYER Components (Player ship only):
│  ├─ UPlayerReputationComponent (Faction reputation) ⭐ MVP
│  ├─ UPlayerProgressionComponent (Advancement)
│  ├─ UPlayerUnlockComponent (Unlocks)
│  ├─ UShipCustomizationComponent (Visual customization)
│  └─ UShipUpgradeComponent (Upgrades)
│
└─ CONTROL Components (All ships):
   └─ USpaceshipControlsComponent (Flight controls)

ASpaceStation (Actor)
├─ CORE:
│  ├─ UStaticMeshComponent (Visual)
│  └─ TArray<ASpaceStationModule*> (Modules)
│
└─ TRADING (Trading stations): ⭐ MVP
   └─ References DA_Market_* (MarketDataAsset)

ASpaceStationModule (Actor)
├─ CORE:
│  ├─ UStaticMeshComponent (Visual)
│  ├─ FString ModuleType
│  ├─ float ModulePower
│  └─ EStationModuleGroup ModuleGroup
│
└─ DOCKING (Docking modules): ⭐ MVP
   └─ TArray<USceneComponent*> DockingPoints
```

---

## Data Asset Dependency Map

```
BP_PlayerShip
├─ Requires: DA_Ship_PlayerScout
│  └─ Defines: Hull, speed, cargo capacity, stats
│
├─ Uses: DA_Weapon_* (if armed)
│  └─ Configures: UWeaponComponent
│
├─ References: DA_Faction_Player
│  └─ For: Faction membership, diplomacy
│
└─ Attached Components:
   ├─ UCargoComponent ⭐ MVP
   │  └─ Capacity from: DA_Ship_PlayerScout.CargoCapacity
   │
   ├─ UPlayerReputationComponent ⭐ MVP
   │  └─ Uses: DA_Faction_* relationships
   │
   └─ UWeaponComponent
      └─ Configured by: DA_Weapon_*

BP_SpaceStation
├─ Contains: TArray<ASpaceStationModule*>
│  ├─ BP_Module_DockingBay ⭐ MVP
│  ├─ BP_Module_Reactor
│  ├─ BP_Module_CargoBay
│  └─ BP_Module_Marketplace ⭐ MVP
│
├─ References: DA_Faction_*
│  └─ Defines: Station owner faction
│
└─ Trading Stations Reference: DA_Market_* ⭐ MVP
   └─ Defines: Item prices, supply/demand

WBP_TradingInterface ⭐ MVP
├─ Reads: DA_Market_* (prices)
├─ Reads: DA_TradeItem_* (item properties)
├─ Queries: UCargoComponent (player ship)
├─ Queries: UPlayerReputationComponent (price modifiers)
└─ Updates: Market supply/demand on transactions
```

---

## MVP Trade Simulator Critical Path

```
MINIMAL REQUIRED BLUEPRINTS FOR MVP:

1. BP_PlayerShip (Player's trading vessel)
   └─ Requires: DA_Ship_PlayerScout
   └─ Must Have: UCargoComponent, UPlayerReputationComponent

2. BP_SpaceStation_Trading (x2-3 stations)
   └─ Must Have: BP_Module_DockingBay or BP_Module_DockingPort
   └─ References: DA_Market_Station1, DA_Market_Station2, DA_Market_Station3

3. WBP_TradingInterface (Trading UI)
   └─ Core functionality: Buy, Sell, Show prices, Show inventory

4. Data Assets:
   ├─ DA_Ship_PlayerScout (Player ship stats)
   ├─ DA_TradeItem_* (x10-20 trade goods)
   ├─ DA_Market_* (x2-3 markets with different prices)
   └─ DA_Faction_* (x2-3 for price modifiers)

5. TestLevel.umap (Playable map with stations)

TOTAL: ~30-40 items to create for MVP
```

---

## Future Expansion Paths

### Post-MVP Phase 1: Combat
```
Add:
├─ BP_Ship_NPC_Fighter (Enemy ships)
├─ BP_Weapon_* Blueprints (Laser, Missile, etc.)
├─ BP_Projectile_* (Visual projectiles)
├─ DA_Weapon_* (x10 weapon configs)
├─ WBP_CombatHUD (Combat UI)
└─ Combat-focused modules (Turrets, Shields)
```

### Post-MVP Phase 2: Faction Diplomacy
```
Add:
├─ DA_Faction_* (x10 factions)
├─ BP_FactionLogic_* (Faction AI behaviors)
├─ Diplomatic quest line
├─ Faction-specific missions
└─ Alliance/war mechanics
```

### Post-MVP Phase 3: Crew Management
```
Add:
├─ DA_Personnel_* (x15 crew roles)
├─ BP_PersonnelLogic_* (Crew AI)
├─ WBP_CrewManagement (Crew UI)
├─ Crew skills and progression
└─ Crew relationships and morale
```

### Post-MVP Phase 4: Quest System
```
Add:
├─ DA_Quest_* (x10+ quests)
├─ WBP_QuestLog (Quest tracker)
├─ Quest objectives and rewards
├─ Procedural quest generation
└─ Story campaign
```

### Post-MVP Phase 5: Station Building
```
Add:
├─ All 25 station module blueprints
├─ WBP_StationEditor (Construction UI)
├─ Module placement and grid system
├─ Resource cost and construction time
└─ Multiplayer station sharing
```

---

## Quick Statistics

### Current State (Jan 2026)
- **C++ Classes**: 52
- **Blueprints Exist**: ~30
- **Blueprints Needed**: ~80
- **Data Assets Needed**: ~100
- **Total to Create**: ~180 items

### MVP Requirements (Trade Simulator)
- **Critical Blueprints**: 5-8
- **Critical Data Assets**: 20-30
- **Critical UI Widgets**: 2-3
- **Estimated Creation Time**: 2-4 weeks

### Full Project (Post-MVP)
- **Total Blueprints**: 110+
- **Total Data Assets**: 200+
- **Total UI Widgets**: 30+
- **Estimated Creation Time**: 12-16 weeks

---

## Related Documentation

- **Comprehensive Guide**: `BLUEPRINT_ARCHITECTURE_GUIDE.md` (main document)
- **Blueprint List**: `docs/archive/UPDATED_BLUEPRINT_LIST_2025.md` (208 items)
- **MVP Guide**: `docs/mvp/BLUEPRINT_IMPLEMENTATION_GUIDE.md`
- **System Guides**: `Assets/*SystemGuide.md`

---

**End of Blueprint Tree Diagram**
