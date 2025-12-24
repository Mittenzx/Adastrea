# Station & Player Systems Roadmap

**Part of**: [Main Project Roadmap](ROADMAP.md)  
**Last Updated**: December 20, 2025  
**Status**: Active Development

---

## Overview

This roadmap covers Space Stations (construction system), Player Systems (progression, inventory, customization), and UI/HUD systems. These systems provide the player-facing gameplay experience.

**Overall Status**: 80% Complete (Code), 5% Complete (Content)

---

## Table of Contents

- [Space Station System](#space-station-system)
- [Player Progression](#player-progression)
- [Inventory System](#inventory-system)
- [Ship Customization](#ship-customization)
- [HUD System](#hud-system)
- [UI Systems](#ui-systems)
- [Tutorial System](#tutorial-system)

---

## Space Station System

**Location**: `Source/Adastrea/Stations/`  
**Status**: ✅ 90% Complete (Code), ❌ 5% Complete (Content)  
**Documentation**: [Station Editor Guide](Content/Blueprints/STATION_EDITOR_README.md), [Station Editor Complete Guide](STATION_EDITOR_COMPLETE_GUIDE.md)

### Current State

#### ✅ Completed (C++)
- **ASpaceStation**: Main station actor
- **ASpaceStationModule**: Modular building blocks
- **UStationEditorWidget**: Station building UI (C++)
- 10 module group types:
  - Core, Habitation, Industrial, Trade
  - Defence, Research, Entertainment, Agriculture
  - Medical, Power
- Module snapping and connection system
- Power and resource management
- Faction ownership integration
- Blueprint exposure for all features

#### ❌ Missing (Content)
- Station module Blueprints (~10+ needed)
- Station prefab Blueprints (~5+ needed)
- Station editor UI widget (Blueprint implementation)
- Module Data Assets
- Station templates
- Test stations in maps

### Roadmap

#### Phase 1: Basic Modules (Week 1-2) - Priority: CRITICAL

**Essential Modules** (5 Blueprints)
- [ ] `BP_Module_Docking`
  - Parent: `ASpaceStationModule`
  - Group: Core
  - Size: Large (3x3)
  - Function: Ship docking bays (2 slots)
  - Components: DockingComponent
  - Power: -50
- [ ] `BP_Module_Power`
  - Group: Core
  - Size: Medium (2x2)
  - Function: Power generation
  - Power: +200 (net +150 after self-consumption)
- [ ] `BP_Module_Storage`
  - Group: Trade
  - Size: Medium (2x2)
  - Function: Cargo storage
  - Capacity: 1000 tons
  - Power: -20
- [ ] `BP_Module_Defence`
  - Group: Defence
  - Size: Small (1x1)
  - Function: Weapon turret mount
  - Components: WeaponComponent
  - Power: -30
- [ ] `BP_Module_Trade`
  - Group: Trade
  - Size: Medium (2x2)
  - Function: Trading terminal
  - Components: TradingComponent
  - Power: -40

**Basic Station** (1 Blueprint)
- [ ] `BP_SpaceStation_Basic`
  - Parent: `ASpaceStation`
  - Pre-configured modules:
    - 1x Docking Bay
    - 1x Power Generator
    - 1x Storage
    - 1x Trade Module
  - Faction: Neutral/Player
  - Test in TestLevel.umap

**Station Editor UI** (1 Widget)
- [ ] `WBP_StationEditor`
  - Parent: `UStationEditorWidget` (C++)
  - Layout: Side panel + 3D preview
  - Features:
    - Module palette (categorized)
    - Drag-and-drop placement
    - Rotation controls
    - Delete/undo
    - Power/resource display
    - Save/load station designs
  - Reference: `STATION_EDITOR_COMPLETE_GUIDE.md`

**SUCCESS MILESTONE**: Can build basic station with docking, power, storage, and trade

---

#### Phase 2: Extended Modules (Week 3-4) - Priority: HIGH

**More Modules** (5 Blueprints)
- [ ] `BP_Module_Habitation`
  - Group: Habitation
  - Size: Large (3x3)
  - Function: Crew quarters (50 crew)
  - Power: -60
- [ ] `BP_Module_Command`
  - Group: Core
  - Size: Large (3x3)
  - Function: Station command center
  - Power: -50
  - Required: 1 per station
- [ ] `BP_Module_Industrial`
  - Group: Industrial
  - Size: Large (3x3)
  - Function: Manufacturing
  - Power: -100
  - Produces: Technology, Electronics
- [ ] `BP_Module_Research`
  - Group: Research
  - Size: Medium (2x2)
  - Function: Research and development
  - Power: -80
  - Bonus: Scan range +20%
- [ ] `BP_Module_Medical`
  - Group: Medical
  - Size: Medium (2x2)
  - Function: Medical services
  - Power: -40
  - Heals crew injuries

**Specialized Stations** (3 Blueprints)
- [ ] `BP_SpaceStation_Military`
  - Pre-configured: Heavy defence focus
  - 4x Defence modules
  - 2x Docking bays
  - 2x Power generators
  - Faction: Military
- [ ] `BP_SpaceStation_TradeHub`
  - Pre-configured: Trade focus
  - 3x Trade modules
  - 3x Storage modules
  - 4x Docking bays
  - Faction: Traders Union
- [ ] `BP_SpaceStation_Research`
  - Pre-configured: Research focus
  - 3x Research modules
  - 1x Medical module
  - Advanced scanner array

**Module Connections**
- [ ] Implement module connection visuals
  - Corridors/tubes between modules
  - Power conduits
  - Cargo transfer lines
- [ ] Add connection requirements
  - Core modules must connect to Command
  - Defence modules must have power
  - Trade modules need storage access

**SUCCESS MILESTONE**: Can build diverse specialized stations

---

#### Phase 3: Advanced Modules (Week 5-6) - Priority: MEDIUM

**Additional Modules** (3 more)
- [ ] `BP_Module_Entertainment`
  - Group: Entertainment
  - Morale boost for crew
- [ ] `BP_Module_Agriculture`
  - Group: Agriculture
  - Produces food
- [ ] `BP_Module_Mining`
  - Group: Industrial
  - Ore processing facility

**Station Features**
- [ ] Station upgrade system
  - Module upgrades (Level 1-3)
  - Improved efficiency
  - Enhanced capabilities
- [ ] Station economy
  - Station resource production
  - Maintenance costs
  - Crew wages
- [ ] Station events
  - Equipment failures
  - Crew incidents
  - Pirate attacks
  - Trade opportunities

**Station Management**
- [ ] Create `WBP_StationManagement` widget
  - Parent: `UStationManagementWidget` (C++)
  - Overview of all modules
  - Resource tracking
  - Crew assignment
  - Financial summary
  - Upgrade interface

---

#### Phase 4: Station Polish (Week 7-8) - Priority: LOW

- [ ] Station visuals polish
- [ ] Module variety (paint schemes)
- [ ] Station interiors (if time allows)
- [ ] Station quests
- [ ] Station ownership mechanics

---

## Player Progression

**Location**: `Source/Adastrea/Player/`  
**Status**: ✅ 85% Complete (Code), ❌ 10% Complete (Content)  
**Documentation**: [Player Progression Guide](Assets/PlayerProgressionGuide.md)

### Current State

#### ✅ Completed (C++)
- **UPlayerProgressionComponent** - Leveling system
- **UPlayerReputationComponent** - Faction reputation
- **UShipUpgradeComponent** - Ship upgrade system
- Experience and leveling
- Skill trees framework
- Unlock system
- Achievement framework
- Statistics tracking

#### ❌ Missing (Content)
- Skill tree definitions
- Unlock configurations
- Achievement definitions
- Progression balance
- Progression UI

### Roadmap

#### Phase 1: Basic Progression (Week 2-3) - Priority: HIGH

**Leveling System**
- [ ] Configure XP rewards
  - Combat: 10-100 XP per kill
  - Quests: 50-500 XP per quest
  - Exploration: 20-200 XP per discovery
  - Trading: 1 XP per 100 credits profit
- [ ] Define level progression
  - Level 1-10: 1000 XP per level
  - Level 11-20: 2000 XP per level
  - Level 21-30: 5000 XP per level
  - Level 31-50: 10000 XP per level
- [ ] Set level rewards
  - Skill points
  - Unlock access
  - Stat bonuses

**Reputation System**
- [ ] Configure reputation gains/losses
  - Per faction (see Economy Roadmap)
  - Per guild/Way
  - Reputation thresholds
- [ ] Define reputation rewards
  - Rank 1 (Neutral): Basic access
  - Rank 2 (Friendly): Discount prices
  - Rank 3 (Honored): Special missions
  - Rank 4 (Revered): Elite equipment
  - Rank 5 (Exalted): Faction ships

**Progression UI** (1 Widget)
- [ ] `WBP_PlayerProgression`
  - Character stats display
  - Level and XP bar
  - Skill trees interface
  - Reputation overview
  - Achievements list

**SUCCESS MILESTONE**: Player gains levels and reputation

---

#### Phase 2: Skill Trees (Week 4-5) - Priority: MEDIUM

**Skill Categories** (5 trees)
- [ ] **Combat Skills** (10 skills)
  - Weapon damage +5% per rank (5 ranks)
  - Shield strength +10% per rank (5 ranks)
  - Critical hit chance +2% per rank (3 ranks)
  - Reload speed +5% per rank (3 ranks)
  - Evasion +5% per rank (3 ranks)
- [ ] **Trading Skills** (8 skills)
  - Trade prices +5% per rank (5 ranks)
  - Cargo capacity +10% per rank (5 ranks)
  - Market intel access (3 ranks)
  - Negotiation bonus +5% per rank (3 ranks)
- [ ] **Exploration Skills** (8 skills)
  - Scan range +10% per rank (5 ranks)
  - Jump range +10% per rank (5 ranks)
  - Anomaly detection +10% per rank (3 ranks)
  - Mining yield +10% per rank (3 ranks)
- [ ] **Leadership Skills** (8 skills)
  - Crew efficiency +5% per rank (5 ranks)
  - Max crew +2 per rank (5 ranks)
  - Crew morale +10% per rank (3 ranks)
  - Command authority +1 per rank (3 ranks)
- [ ] **Technical Skills** (8 skills)
  - Repair speed +10% per rank (5 ranks)
  - Upgrade cost -5% per rank (5 ranks)
  - System efficiency +5% per rank (3 ranks)
  - Hacking bonus +10% per rank (3 ranks)

**Skill Tree UI**
- [ ] Visual skill tree layout
- [ ] Prerequisite connections
- [ ] Skill descriptions and costs
- [ ] Respec functionality

---

#### Phase 3: Unlocks & Achievements (Week 6) - Priority: LOW

**Unlockables** (Configure)
- [ ] Ship unlocks (level-gated)
- [ ] Equipment unlocks (reputation-gated)
- [ ] Module unlocks (achievement-gated)
- [ ] Location unlocks (progression-gated)

**Achievements** (20 achievements)
- [ ] Story achievements (5)
- [ ] Combat achievements (5)
- [ ] Trading achievements (5)
- [ ] Exploration achievements (5)

---

## Inventory System

**Location**: `Source/Adastrea/UI/`  
**Status**: ✅ 85% Complete (Code), ❌ 0% Complete (Content)  
**Documentation**: [Inventory System Guide](Assets/InventorySystemGuide.md)

### Current State

#### ✅ Completed (C++)
- **UInventoryComponent** - Inventory management
- **UInventoryWidget** - Inventory UI (C++)
- Item categories (8 types)
- Weight and volume limits
- Item stacking
- Item transfer system
- Equipment slots

#### ❌ Missing (Content)
- Inventory UI widget (Blueprint)
- Item Data Assets (~50+ needed)
- Item icons
- Equipment definitions

### Roadmap

#### Phase 1: Basic Inventory (Week 2) - Priority: HIGH

**Inventory UI** (1 Widget)
- [ ] `WBP_Inventory`
  - Parent: `UInventoryWidget` (C++)
  - Layout: Grid-based inventory
  - Features:
    - Item icons and tooltips
    - Sort by: Type, Name, Value, Weight
    - Filter by category
    - Weight/volume bars
    - Quick transfer buttons
    - Equipment slots display
  - Reference: `Assets/InventorySystemGuide.md`

**Basic Items** (10 Data Assets)
- [ ] `DA_Item_HealthPack` (consumable)
- [ ] `DA_Item_RepairKit` (consumable)
- [ ] `DA_Item_Fuel` (resource)
- [ ] `DA_Item_Ammo_Energy` (resource)
- [ ] `DA_Item_Ammo_Kinetic` (resource)
- [ ] `DA_Item_Scanner_Basic` (equipment)
- [ ] `DA_Item_Shield_Booster` (equipment)
- [ ] `DA_Item_Cargo_Expander` (upgrade)
- [ ] `DA_Item_QuestItem_DataChip` (quest)
- [ ] `DA_Item_Salvage_Scrap` (trade good)

**SUCCESS MILESTONE**: Can view inventory, use items, transfer items

---

#### Phase 2: Extended Inventory (Week 3-4) - Priority: MEDIUM

**More Items** (40+ Data Assets)
- [ ] Consumables (10)
- [ ] Equipment (10)
- [ ] Upgrades (10)
- [ ] Trade goods (10)
- [ ] Quest items (10)

**Item Features**
- [ ] Item rarity system (Common, Uncommon, Rare, Epic, Legendary)
- [ ] Item quality system (Damaged, Normal, Enhanced, Masterwork)
- [ ] Item modifications
- [ ] Crafting system (if time allows)

---

## Ship Customization

**Location**: `Source/Adastrea/UI/`  
**Status**: ✅ 85% Complete (Code), ❌ 0% Complete (Content)  
**Documentation**: [Ship Customization Guide](Assets/ShipCustomizationGuide.md)

### Current State

#### ✅ Completed (C++)
- **UShipCustomizationWidget** - Customization UI (C++)
- **UShipUpgradeComponent** - Upgrade system
- Loadout management
- Weapon slots
- Module slots
- Visual customization framework

#### ❌ Missing (Content)
- Customization UI widget (Blueprint)
- Upgrade Data Assets
- Paint schemes
- Decals and emblems

### Roadmap

#### Phase 1: Basic Customization (Week 3) - Priority: HIGH

**Customization UI** (1 Widget)
- [ ] `WBP_ShipCustomization`
  - Parent: `UShipCustomizationWidget` (C++)
  - Tabs: Loadout, Upgrades, Visuals, Stats
  - Features:
    - Weapon slot management
    - Module slot management
    - Paint scheme selection
    - Decal placement
    - Stat comparison
    - Cost display
  - Reference: `Assets/ShipCustomizationGuide.md`

**Weapon Loadouts**
- [ ] Configure weapon slots per ship
  - Scout: 2 light weapons
  - Fighter: 4 medium weapons
  - Trader: 1 light weapon (defensive)
  - Capital: 8+ heavy weapons
- [ ] Loadout presets (3 per ship)

**Module Slots**
- [ ] Configure module slots per ship
  - Shield module
  - Engine module
  - Cargo module
  - Scanner module
  - Special module

**SUCCESS MILESTONE**: Can customize ship loadout and stats

---

#### Phase 2: Advanced Customization (Week 4-5) - Priority: MEDIUM

**Ship Upgrades** (20 upgrades)
- [ ] Hull upgrades (5 tiers)
- [ ] Shield upgrades (5 tiers)
- [ ] Engine upgrades (5 tiers)
- [ ] Weapon upgrades (5 tiers)
- [ ] Special upgrades (rare)

**Visual Customization**
- [ ] Paint schemes (10+ colors)
- [ ] Decals (20+ options)
- [ ] Ship skins (premium)
- [ ] Engine trails (5 colors)

---

## HUD System

**Location**: `Source/Adastrea/UI/`  
**Status**: ✅ 95% Complete (Code), ❌ 0% Complete (Content)  
**Documentation**: [HUD System Guide](Assets/HUDSystemGuide.md)

### Current State

#### ✅ Completed (C++)
- **UAdastreaHUDWidget** - Main HUD (C++)
- Ship status display
- Targeting system UI
- Minimap/radar
- Quest tracking
- Warning systems
- Damage indicators

#### ❌ Missing (Content)
- HUD UI widget (Blueprint)
- HUD customization options
- HUD themes

### Roadmap

#### Phase 1: Essential HUD (Week 1) - Priority: CRITICAL

**Main HUD** (1 Widget)
- [ ] `WBP_HUD_Main`
  - Parent: `UAdastreaHUDWidget` (C++)
  - Layout: Full-screen overlay
  - Components:
    - **Top-left**: Ship status
      - Hull bar (red)
      - Shield bar (blue) x4 (front/back/left/right)
      - Speed indicator
      - Power level
    - **Top-right**: Navigation
      - Autopilot status
      - Waypoint distance
      - Current sector
    - **Center**: Targeting reticle
      - Target name
      - Target health/shields
      - Target distance
      - Lead indicator
    - **Bottom-left**: Radar/minimap
      - Local space (5km radius)
      - Ships (color-coded by faction)
      - Stations
      - Objectives
    - **Bottom-right**: Quick access
      - Active quests (3 slots)
      - Warnings/alerts
      - Communication log
  - Reference: `Assets/HUDSystemGuide.md`

**SUCCESS MILESTONE**: Can see all essential ship and game state info

---

#### Phase 2: HUD Polish (Week 2-3) - Priority: MEDIUM

**Additional HUD Elements**
- [ ] Damage direction indicators
- [ ] Missile warning system
- [ ] Scan progress bar
- [ ] Cargo capacity indicator
- [ ] Fuel gauge
- [ ] Crew status summary

**HUD Customization**
- [ ] HUD layout presets (3 options)
- [ ] HUD opacity settings
- [ ] HUD color themes (5 themes)
- [ ] HUD element toggles

---

## UI Systems

**Location**: `Source/Adastrea/UI/`  
**Status**: ✅ 80% Complete (Code), ❌ 0% Complete (Content)

### Roadmap

#### Phase 1: Core UI (Week 1-2) - Priority: CRITICAL

**Menus** (3 Widgets)
- [ ] `WBP_MainMenu` (see Content Creation Roadmap)
- [ ] `WBP_PauseMenu` (see Content Creation Roadmap)
- [ ] `WBP_SettingsMenu`
  - Graphics settings
  - Audio settings
  - Controls settings
  - Gameplay settings

**Core Gameplay UI** (5 Widgets)
- [ ] `WBP_HUD_Main` (see above)
- [ ] `WBP_Inventory` (see above)
- [ ] `WBP_TradingInterface` (see Content Creation Roadmap)
- [ ] `WBP_QuestLog` (see Content Creation Roadmap)
- [ ] `WBP_Scanner` (see Core Systems Roadmap)

---

#### Phase 2: Management UI (Week 3-4) - Priority: HIGH

**Management Interfaces** (5 Widgets)
- [ ] `WBP_StationManagement` (see above)
- [ ] `WBP_ShipCustomization` (see above)
- [ ] `WBP_PlayerProgression` (see above)
- [ ] `WBP_StationEditor` (see above)
- [ ] `WBP_ShipStatus` (implemented, needs Blueprint)

---

#### Phase 3: Advanced UI (Week 5-6) - Priority: MEDIUM

**Additional Interfaces** (10+ Widgets)
- [ ] Crew management
- [ ] Faction status
- [ ] Map interfaces (2)
- [ ] Dialogue system
- [ ] Comms interface
- [ ] Tutorial overlays
- [ ] Notification system
- [ ] Statistics screens

---

## Tutorial System

**Location**: `Source/Adastrea/Tutorial/`  
**Status**: ✅ 90% Complete (Code), ❌ 5% Complete (Content)  
**Documentation**: [Tutorial System Guide](Assets/TutorialSystemGuide.md)

### Current State

#### ✅ Completed (C++)
- **UTutorialManagerSubsystem** - Tutorial management
- **UTutorialStepDataAsset** - Tutorial step definitions
- 6 tutorial step types
- Progress tracking
- Conditional steps
- Tutorial UI framework

#### ❌ Missing (Content)
- Tutorial Step Data Assets (~20+ needed)
- Tutorial UI widget
- Tutorial quest integration

### Roadmap

#### Phase 1: Essential Tutorials (Week 2-3) - Priority: HIGH

**Tutorial UI** (1 Widget)
- [ ] `WBP_TutorialOverlay`
  - Semi-transparent overlay
  - Highlight target UI elements
  - Step instructions
  - Skip button
  - Progress indicator

**Basic Tutorial Steps** (15 steps)
- [ ] Movement tutorial (3 steps)
  - WASD/Gamepad movement
  - Mouse look
  - Boost control
- [ ] Combat tutorial (4 steps)
  - Target enemy
  - Fire weapons
  - Manage shields
  - Evade attacks
- [ ] Docking tutorial (3 steps)
  - Approach station
  - Activate docking
  - Confirm dock
- [ ] Trading tutorial (3 steps)
  - Open trade interface
  - Buy goods
  - Sell goods
- [ ] Navigation tutorial (2 steps)
  - Set waypoint
  - Activate autopilot

**Tutorial Quest Integration**
- [ ] Link tutorial steps to tutorial quests
- [ ] Reward completion
- [ ] Track progress

**SUCCESS MILESTONE**: New players can learn basic controls

---

#### Phase 2: Advanced Tutorials (Week 4) - Priority: MEDIUM

**Advanced Tutorial Steps** (10+ steps)
- [ ] Combat tactics (3 steps)
- [ ] Station management (3 steps)
- [ ] Trading routes (2 steps)
- [ ] Exploration (2 steps)
- [ ] Guild/Way system (2 steps)

---

## Integration Status

### System Dependencies

```
Station System
  ├─→ Requires: Module Blueprints, Station Editor UI
  └─→ Enables: Station building, Station management

Player Progression
  ├─→ Requires: XP configuration, Skill trees
  └─→ Enables: Character advancement, Unlocks

Inventory System
  ├─→ Requires: Item Data Assets, Inventory UI
  └─→ Enables: Item management, Trading

Ship Customization
  ├─→ Requires: Upgrade definitions, Customization UI
  └─→ Enables: Ship personalization, Loadouts

HUD System
  ├─→ Requires: HUD widget implementation
  └─→ Enables: Player awareness, Game state display

Tutorial System
  ├─→ Requires: Tutorial steps, Tutorial UI
  └─→ Enables: Player onboarding, Learning curve
```

---

## Success Metrics

### Q1 2026 Goals
- [ ] Can build basic stations with 5+ module types
- [ ] HUD displays all essential information
- [ ] Can manage inventory and use items
- [ ] Can customize ship loadout
- [ ] Can gain levels and reputation
- [ ] Tutorial teaches basic controls

### Q2 2026 Goals
- [ ] Station system fully featured with 10+ modules
- [ ] Progression system balanced and engaging
- [ ] All UI systems polished and intuitive
- [ ] Advanced tutorials available
- [ ] Ship customization comprehensive

---

## Related Documentation

- [Main Roadmap](ROADMAP.md)
- [Content Creation Roadmap](ROADMAP_CONTENT_CREATION.md)
- [Core Systems Roadmap](ROADMAP_CORE_SYSTEMS.md)
- [Station Editor Guide](STATION_EDITOR_COMPLETE_GUIDE.md)
- [HUD System Guide](Assets/HUDSystemGuide.md)
- [Inventory System Guide](Assets/InventorySystemGuide.md)
- [Player Progression Guide](Assets/PlayerProgressionGuide.md)
- [Tutorial System Guide](Assets/TutorialSystemGuide.md)

---

**Last Updated**: December 20, 2025  
**Next Review**: January 2026
