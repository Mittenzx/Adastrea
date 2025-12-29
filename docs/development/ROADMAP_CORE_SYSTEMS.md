# Core Systems Roadmap

**Part of**: [Main Project Roadmap](../../ROADMAP.md)  
**Last Updated**: December 29, 2025  
**Status**: ⏸️ **PAUSED** - Trade Simulator MVP Focus

> **⚠️ MVP PIVOT (Dec 24, 2025)**: Core systems work is paused. Only Ship and Station systems are MVP-relevant. All other systems (AI, Combat, Navigation, Quest, Exploration) are deferred until post-MVP validation.

---

## Overview

**Current Status**: 
- Code: 85% Complete (all systems implemented)
- MVP Subset: Ships (5-6 functions) + Stations (5-6 functions) only
- Non-MVP Systems: Paused until Week 13+

### MVP-Relevant Systems (Active)

**Ships System**:
- Status: MVP-critical functions identified (Phase 2.1)
- Keep: Basic flight, docking, cargo (5-6 functions)
- Deferred: Customization, modules, particles (80+ functions)

**Stations System**:
- Status: Already lean, MVP-ready
- Keep: Docking, market access (5-6 functions)
- Deferred: Module construction (most functions)

### Non-MVP Systems (Deferred)

All systems below are **paused** until Trade Simulator MVP validation complete:

---

## Table of Contents

- [AI System](#ai-system)
- [Combat System](#combat-system)
- [Navigation System](#navigation-system)
- [Quest System](#quest-system)
- [Exploration System](#exploration-system)
- [Procedural Generation](#procedural-generation)
- [Integration Status](#integration-status)

---

## AI System

**Location**: `Source/Adastrea/AI/`  
**Status**: ⏸️ **DEFERRED** - Post-MVP Only  
**Documentation**: [AI Quick Start](Assets/AI_QUICK_START.md)

> **MVP Note**: No AI in Trade Simulator MVP. Economy simulation only (EconomyManager handles basic supply/demand). Full AI deferred to post-MVP.

### Deferred Components

- ❌ NPCLogicBase (not needed for MVP)
- ❌ FactionLogic (basic faction pricing only in MVP)
- ❌ PersonnelLogic (no crew in MVP)
- ❌ AI trader behaviors (deferred)
- ❌ Social relationships (deferred)

**Resume**: Post-MVP validation (Week 13+)

### Roadmap

#### Phase 1: Content Creation (Q1 2026) - Priority: HIGH
- [ ] Create `DA_FactionAI_*` Data Assets for each faction (10 assets)
  - Use templates: `Assets/FactionAITemplates/*.yaml`
  - Configure decision weights, aggression, trade preferences
- [ ] Create `DA_PersonnelAI_*` Data Assets for personality types (8 assets)
  - Use templates: `Assets/PersonnelAITemplates/*.yaml`
  - Configure reaction patterns, relationship modifiers
- [ ] Create `BP_NPCShip_*` Blueprints with AI logic (5+ variants)
  - Fighter AI (aggressive)
  - Trader AI (cautious)
  - Explorer AI (curious)
  - Pirate AI (opportunistic)
  - Escort AI (protective)

#### Phase 2: Advanced Behaviors (Q2 2026) - Priority: MEDIUM
- [ ] Implement complex decision trees in Blueprints
- [ ] Add faction-specific AI behaviors
- [ ] Create AI state machines for combat tactics
- [ ] Implement formation flying AI
- [ ] Add AI fleet coordination

#### Phase 3: Polish & Balance (Q3 2026) - Priority: LOW
- [ ] Balance AI difficulty levels
- [ ] Add AI personality variety
- [ ] Optimize AI performance for large battles
- [ ] Add debug visualization for AI decisions
- [ ] Create AI testing scenarios

### Dependencies
- **Requires**: Faction System content, Personnel content, Ship Blueprints
- **Blocks**: Realistic NPC behaviors, Dynamic faction interactions

### Next Steps (Immediate)
1. Create example AI Data Assets (1-2 per type)
2. Create test BP_NPCShip with basic AI
3. Test AI in TestLevel.umap
4. Document AI configuration process

---

## Combat System

**Location**: `Source/Adastrea/Combat/`  
**Status**: ✅ 100% Complete (Code), ❌ 5% Complete (Content)  
**Documentation**: [Combat System Guide](Assets/CombatSystemGuide.md), [Combat Quick Reference](Assets/CombatQuickReference.md)

### Current State

#### ✅ Completed (C++)
- 6 weapon types (Energy, Projectile, Missile, Torpedo, Beam, Plasma)
- 5 damage types (Kinetic, Energy, Explosive, Thermal, EMP)
- Targeting system with lock-on and threat prioritization
- Directional shields (4 facings) with recharge
- Hull integrity and armor penetration
- Point defense with automated interception
- 5-phase boarding mechanics
- WeaponComponent, TargetingComponent, DamageComponent

#### ❌ Missing (Content)
- Weapon Data Assets (~12 needed)
- Weapon projectile Blueprints (~6 needed)
- Combat visual effects (muzzle flashes, impacts, shields)
- Combat sound effects
- Balanced weapon stats
- Test combat scenarios

### Roadmap

#### Phase 1: Weapon Content (Q1 2026) - Priority: CRITICAL
- [ ] Create Weapon Data Assets (12 assets)
  - Use templates: `Assets/CombatSystemGuide.md` → Weapon Templates
  - Energy weapons: Laser, Plasma, Beam (3 variants each)
  - Projectile weapons: Railgun, Autocannon, Coilgun
  - Missiles: Standard, Torpedo, Anti-ship
- [ ] Create projectile Blueprints (6 assets)
  - `BP_Projectile_Laser`
  - `BP_Projectile_Plasma`
  - `BP_Projectile_Railgun`
  - `BP_Projectile_Missile`
  - `BP_Projectile_Torpedo`
  - `BP_Projectile_Beam` (instant-hit effect)
- [ ] Add basic combat VFX placeholders
  - Weapon fire effects
  - Impact effects
  - Shield hit effects

#### Phase 2: Combat Integration (Q1 2026) - Priority: HIGH
- [ ] Add WeaponComponent to ship Blueprints
- [ ] Configure weapon hardpoints on ships
- [ ] Set up TargetingComponent on player and NPC ships
- [ ] Create test combat scenario map
- [ ] Balance weapon damage and ranges

#### Phase 3: Advanced Combat (Q2 2026) - Priority: MEDIUM
- [ ] Create diverse weapon loadouts
- [ ] Implement tactical combat AI
- [ ] Add combat sound effects
- [ ] Polish combat VFX
- [ ] Create combat tutorial quest
- [ ] Balance point defense effectiveness

#### Phase 4: Combat Depth (Q3 2026) - Priority: LOW
- [ ] Add weapon customization options
- [ ] Implement ammunition types
- [ ] Add weapon overheating visuals
- [ ] Create capital ship combat
- [ ] Add boarding action gameplay

### Dependencies
- **Requires**: Ship Blueprints, Audio assets, VFX assets
- **Blocks**: Combat quests, PvP gameplay, Combat AI testing

### Next Steps (Immediate)
1. Create 3 basic weapon Data Assets (Laser, Railgun, Missile)
2. Create 3 projectile Blueprints
3. Add WeaponComponent to BP_PlayerShip
4. Test basic combat in TestLevel.umap

---

## Navigation System

**Location**: `Source/Adastrea/Navigation/`  
**Status**: ✅ 100% Complete (Code), ⚠️ 20% Complete (Content)  
**Documentation**: [Navigation System Guide](Assets/NavigationSystemGuide.md)

### Current State

#### ✅ Completed (C++)
- NavigationComponent with autopilot
- 3D pathfinding with obstacle avoidance
- Multi-waypoint navigation
- AI following for escort/convoy
- Configurable navigation parameters
- Blueprint-exposed functions

#### ⚠️ Partially Complete (Content)
- Basic autopilot testing done
- Some test waypoints placed
- Limited pathfinding testing

#### ❌ Missing (Content)
- Navigation UI widgets
- Waypoint placement tools
- Nav markers and beacons
- Jump gate mechanics
- FTL travel content

### Roadmap

#### Phase 1: Basic Navigation Content (Q1 2026) - Priority: HIGH
- [ ] Create navigation UI widget (`WBP_Navigation`)
  - Autopilot toggle
  - Waypoint list
  - Distance/ETA display
  - Route visualization
- [ ] Add navigation markers to test map
  - Waypoint markers
  - Station markers
  - Hazard markers
- [ ] Create `DA_NavigationConfig` Data Asset
  - Configure default speeds
  - Set autopilot behavior
  - Define avoidance parameters

#### Phase 2: Advanced Navigation (Q2 2026) - Priority: MEDIUM
- [ ] Implement jump gate system
  - Create `BP_JumpGate` actor
  - Link jump gates between sectors
  - Add jump animation
- [ ] Create FTL travel system
  - FTL charge-up mechanic
  - Jump route planning
  - Jump interdiction zones
- [ ] Add navigation hazards
  - Asteroid fields
  - Radiation zones
  - Gravity wells

#### Phase 3: Navigation Polish (Q3 2026) - Priority: LOW
- [ ] Advanced route planning UI
- [ ] Trade route optimization
- [ ] Auto-trade route navigation
- [ ] Fleet formation navigation
- [ ] Nav computer upgrades

### Dependencies
- **Requires**: HUD system, Sector maps, Ship Blueprints
- **Blocks**: Long-distance travel, Trade automation, Fleet mechanics

### Next Steps (Immediate)
1. Create basic WBP_Navigation widget
2. Add autopilot toggle to HUD
3. Place test waypoints in sector map
4. Test multi-waypoint navigation

---

## Quest System

**Location**: `Source/Adastrea/Quest/`  
**Status**: ✅ 100% Complete (Code), ❌ 5% Complete (Content)  
**Documentation**: [Quest System Guide](Assets/QuestSystemGuide.md)

### Current State

#### ✅ Completed (C++)
- 10 quest types (Delivery, Combat, Exploration, Rescue, etc.)
- 9 objective types with automatic tracking
- Quest chains with prerequisites
- Difficulty-scaled rewards
- Procedural generation support
- QuestDataAsset, QuestManagerSubsystem

#### ❌ Missing (Content)
- Quest Data Assets (need ~50+)
- Quest UI widget
- Quest dialogue
- Tutorial quests
- Story campaign quests
- Procedural quest templates

### Roadmap

#### Phase 1: Basic Quests (Q1 2026) - Priority: CRITICAL
- [ ] Create tutorial quest chain (5 quests)
  - `DA_Quest_Tutorial_FirstFlight`
  - `DA_Quest_Tutorial_Docking`
  - `DA_Quest_Tutorial_Trading`
  - `DA_Quest_Tutorial_Combat`
  - `DA_Quest_Tutorial_Navigation`
- [ ] Create quest UI (`WBP_QuestLog`)
  - Active quest list
  - Quest details panel
  - Objective tracking
  - Rewards display
- [ ] Add quest markers to HUD
  - Objective waypoints
  - Distance indicators
  - Quest giver icons

#### Phase 2: Core Quests (Q2 2026) - Priority: HIGH
- [ ] Create 20 generic side quests
  - 5 Delivery quests
  - 5 Combat quests
  - 5 Exploration quests
  - 5 Mixed-objective quests
- [ ] Create faction-specific quests (5 per faction = 50 total)
  - Reputation requirements
  - Faction-themed objectives
  - Unique rewards
- [ ] Implement quest dialogue system
  - Quest giver conversations
  - Accept/decline options
  - Quest completion dialogue

#### Phase 3: Story Campaigns (Q3 2026) - Priority: MEDIUM
- [ ] Main story campaign (15-20 quests)
- [ ] Faction storylines (5-8 quests per faction)
- [ ] Character-driven quest chains
- [ ] Dynamic event quests

#### Phase 4: Procedural Content (Q4 2026) - Priority: LOW
- [ ] Procedural quest generation templates
- [ ] Radiant quest system
- [ ] Dynamic contract generation
- [ ] Reputation-gated quests

### Dependencies
- **Requires**: UI system, Faction content, Ship content, NPC actors
- **Blocks**: Story progression, Tutorial system, Player engagement

### Next Steps (Immediate)
1. Create tutorial quest Data Assets (5 quests)
2. Create basic WBP_QuestLog widget
3. Add quest tracking to HUD
4. Test quest flow end-to-end

---

## Exploration System

**Location**: `Source/Adastrea/Exploration/`  
**Status**: ✅ 70% Complete (Phases 1-2 Done)  
**Documentation**: [Exploration System Guide](Assets/ExplorationSystemGuide.md)

### Current State

#### ✅ Completed (C++ - Phases 1-2)
- **Phase 1: Scanner & Sensors**
  - ScannerComponent with passive/active modes
  - ScannableObjectComponent
  - 8 signal types
  - Sensor range and resolution
  - Stealth detection
- **Phase 2: Discovery & Anomalies**
  - AnomalyDataAsset (10 types, 5 rarity tiers)
  - DiscoveryDataAsset (10 location types)
  - DiscoveryComponent
  - DiscoveryManagerSubsystem

#### ❌ Missing (Content & Phase 3-4)
- Scanner Data Assets
- Anomaly Data Assets
- Discovery location Data Assets
- Mining mechanics (Phase 3)
- Cartography system (Phase 4)

### Roadmap

#### Phase 1: Exploration Content (Q1 2026) - Priority: HIGH
- [ ] Create Scanner Data Assets (7 scanners)
  - Use templates: `Assets/ExplorationSystemGuide.md` → Scanner Templates
  - Basic Scanner (starter)
  - Improved Scanner (mid-tier)
  - Advanced Scanner (high-tier)
  - Military Scanner
  - Survey Scanner
  - Mining Scanner
  - Research Scanner
- [ ] Create Anomaly Data Assets (10 anomalies)
  - Wormholes
  - Nebulae
  - Black holes
  - Quantum fluctuations
  - etc.
- [ ] Create Discovery Data Assets (10 locations)
  - Ancient ruins
  - Derelict ships
  - Research stations
  - etc.

#### Phase 2: Scanner Integration (Q1 2026) - Priority: HIGH
- [ ] Add ScannerComponent to ship Blueprints
- [ ] Create scanner UI widget (`WBP_Scanner`)
  - Active scan interface
  - Detected signals list
  - Signal strength indicators
- [ ] Place scannable objects in test maps
- [ ] Test scanning mechanics

#### Phase 3: Resource Gathering (Q2 2026) - Priority: MEDIUM
- [ ] Implement mining mechanics (C++)
  - Mining laser component
  - Resource extraction
  - Cargo collection
- [ ] Create mineable asteroids
- [ ] Add mining UI
- [ ] Create mining quests

#### Phase 4: Cartography (Q2 2026) - Priority: MEDIUM
- [ ] Implement star map system (C++)
- [ ] Create map UI (`WBP_StarMap`)
- [ ] Add discovered region tracking
- [ ] Implement navigation markers
- [ ] Create exploration quests

### Dependencies
- **Requires**: Ship Blueprints, UI system, Sector maps
- **Blocks**: Exploration gameplay loop, Mining economy, Cartography features

### Next Steps (Immediate)
1. Create 3 scanner Data Assets (Basic, Improved, Advanced)
2. Create 3 anomaly Data Assets (test variety)
3. Add ScannerComponent to BP_PlayerShip
4. Create basic WBP_Scanner UI
5. Test scanning in TestLevel.umap

---

## Procedural Generation

**Location**: `Source/Adastrea/Procedural/`  
**Status**: ⚠️ 30% Complete  
**Documentation**: [Procedural Generation Guide](PROCEDURAL_GENERATION_GUIDE.md)

### Current State

#### ✅ Completed
- Python procedural generation scripts
- YAML-based content templates
- Sector generation framework (C++)
- ProceduralGenerators.py tools

#### ❌ Missing
- Runtime procedural generation (C++)
- Galaxy generation system
- Planet generation
- NPC ship spawning
- Dynamic economy generation

### Roadmap

#### Phase 1: Sector Generation (Q2 2026) - Priority: MEDIUM
- [ ] Implement runtime sector generation (C++)
  - Procedural star placement
  - Planet orbit generation
  - Asteroid field placement
  - Space station spawning
- [ ] Create sector generation Data Assets
- [ ] Test procedural sector creation

#### Phase 2: Content Generation (Q2 2026) - Priority: MEDIUM
- [ ] NPC ship spawning system
- [ ] Dynamic trade route generation
- [ ] Random encounter generation
- [ ] Procedural quest generation

#### Phase 3: Large-Scale Generation (Q3 2026) - Priority: LOW
- [ ] Galaxy-scale generation
- [ ] Faction territory generation
- [ ] Trade network generation
- [ ] Political boundary generation

### Dependencies
- **Requires**: Core systems complete, Sector system
- **Blocks**: Large-scale content creation, Replayability

### Next Steps (Immediate)
1. Prioritize manual content creation over procedural
2. Use Python scripts for batch asset creation
3. Defer runtime procedural generation to Phase 5

---

## Integration Status

### System Interdependencies

```
AI System
  ├─→ Requires: Faction content, Personnel content, Ship Blueprints
  └─→ Enables: Realistic NPC behavior, Dynamic faction interactions

Combat System
  ├─→ Requires: Ship Blueprints, Weapon Data Assets, VFX
  └─→ Enables: Combat gameplay, Quest objectives, PvP

Navigation System
  ├─→ Requires: Sector maps, UI system
  └─→ Enables: Travel gameplay, Trade routes, Exploration

Quest System
  ├─→ Requires: All systems, UI, NPCs
  └─→ Enables: Story progression, Player goals, Tutorial

Exploration System
  ├─→ Requires: Ships, UI, Sector maps
  └─→ Enables: Discovery gameplay, Mining, Cartography
```

### Critical Path (Q1 2026)

To enable basic gameplay loop, prioritize in this order:

1. **Ships & UI** (Week 1-2)
   - Create 2-3 ship Blueprints
   - Create basic HUD and menus

2. **Combat** (Week 3)
   - Create 3 weapon Data Assets
   - Create projectile Blueprints
   - Test basic combat

3. **Quests** (Week 4)
   - Create 5 tutorial quests
   - Create quest UI
   - Test quest flow

4. **Navigation & Exploration** (Week 5-6)
   - Create navigation UI
   - Create scanner UI
   - Add waypoints to maps

5. **AI** (Week 7-8)
   - Create AI Data Assets
   - Create NPC ship Blueprints
   - Test NPC behaviors

---

## Success Metrics

### Q1 2026 Goals
- [ ] Can play tutorial quest chain start-to-finish
- [ ] Can engage in basic combat with NPCs
- [ ] Can navigate between waypoints with autopilot
- [ ] Can scan and discover anomalies
- [ ] NPCs exhibit believable behaviors

### Q2 2026 Goals
- [ ] 20+ quests available
- [ ] 10+ weapon types functional
- [ ] Navigation system fully featured
- [ ] Mining gameplay functional
- [ ] AI behaviors polished

---

## Related Documentation

- [Main Roadmap](ROADMAP.md)
- [Content Creation Roadmap](ROADMAP_CONTENT_CREATION.md)
- [Technical Infrastructure Roadmap](ROADMAP_TECHNICAL.md)
- [AI System Guide](Assets/AI_QUICK_START.md)
- [Combat System Guide](Assets/CombatSystemGuide.md)
- [Navigation System Guide](Assets/NavigationSystemGuide.md)
- [Quest System Guide](Assets/QuestSystemGuide.md)
- [Exploration System Guide](Assets/ExplorationSystemGuide.md)

---

**Last Updated**: December 20, 2025  
**Next Review**: January 2026
