# System Classification for Trade Simulator MVP

**Purpose**: Define which existing systems are MVP-critical, simplified, or deferred  
**Last Updated**: 2026-01-13  
**Based on**: Critical Review Analysis and MVP Scope Definition  
**Current Phase**: Phase 1 - Hardcoded Prototype

---

## 🎯 MVP Classification Rules

**MVP-Critical**: Required for core trading gameplay loop (use now)  
**MVP-Simplified**: Use limited subset of features only (partial use)  
**Deferred**: Do not touch until post-MVP validation (ignore for now)

**Key Principle**: One polished system beats ten half-finished systems.

---

## ✅ MVP-CRITICAL Systems (Use Full Functionality)

### 1. Spaceship System
**Status**: ✅ Use (limited features)  
**MVP Scope**:
- Basic flight controls (movement, rotation)
- Cargo capacity property
- Ship upgrade capability (cargo only)
- Simple ship statistics display

**Deferred Features**:
- ❌ Complex combat stats
- ❌ Crew quarters
- ❌ Module system
- ❌ Multiple ship interiors
- ❌ Advanced ship customization

**Files**: `Source/Adastrea/Ships/Spaceship.h/.cpp`

---

### 2. Trading System
**Status**: ✅ Use (core system)  
**MVP Scope**:
- Buy/sell cargo interface
- Price display
- Inventory management
- Profit calculation
- Basic supply/demand

**Keep All Features**: This is the core MVP system

**Files**: `Source/Adastrea/Trading/*.h/.cpp`

---

### 3. Space Station System (Docking)
**Status**: ✅ Use (simplified)  
**MVP Scope**:
- Station docking triggers
- Trading UI access
- Basic station identification
- Market price display

**Deferred Features**:
- ❌ Station modules
- ❌ Station construction
- ❌ Station combat
- ❌ Station services beyond trading
- ❌ Multiple docking bays

**Files**: `Source/Adastrea/Stations/SpaceStation.h/.cpp`

---

### 4. HUD/UI System
**Status**: ✅ Use (minimal)  
**MVP Scope**:
- Basic HUD (speed, cargo capacity, credits)
- Trading interface widget
- Station interaction prompt
- Profit/loss display

**Deferred Features**:
- ❌ Complex combat UI
- ❌ Crew management UI
- ❌ Quest tracker
- ❌ Relationship displays
- ❌ Advanced ship status screens

**Files**: `Source/Adastrea/UI/*.h/.cpp`

---

### 5. Save System
**Status**: ✅ Use (basic)  
**MVP Scope**:
- Save player credits
- Save cargo inventory
- Save current ship stats
- Save station prices (optional)

**Deferred Features**:
- ❌ Multiple save slots
- ❌ Cloud saves
- ❌ Save game metadata
- ❌ Autosave checkpoints

**Files**: `Source/Adastrea/Core/SaveGame*.h/.cpp`

---

## ⚠️ MVP-SIMPLIFIED Systems (Limited Use)

### 6. Faction System
**Status**: ⚠️ Simplified  
**MVP Scope**:
- Station ownership (which faction owns station)
- Price modifiers (faction affects prices)
- Basic reputation (affects prices only)

**Deferred Features**:
- ❌ Faction diplomacy
- ❌ Faction quests
- ❌ Faction warfare
- ❌ Faction territories
- ❌ Complex relationship dynamics

**Files**: `Source/Adastrea/Factions/*.h/.cpp`

---

### 7. Navigation System
**Status**: ⚠️ Simplified  
**MVP Scope**:
- Station waypoint markers
- Distance to station display
- Basic compass/direction indicator

**Deferred Features**:
- ❌ Complex navigation networks
- ❌ Jump gates
- ❌ Advanced pathfinding
- ❌ Auto-pilot

**Files**: `Source/Adastrea/Navigation/*.h/.cpp`

---

### 8. Inventory System
**Status**: ⚠️ Simplified  
**MVP Scope**:
- Cargo holds array
- Item quantity tracking
- Cargo capacity limits

**Deferred Features**:
- ❌ Equipment slots
- ❌ Weapon inventory
- ❌ Module inventory
- ❌ Special items
- ❌ Item categories beyond trade goods

**Files**: `Source/Adastrea/Inventory/*.h/.cpp`

---

## 🚫 DEFERRED Systems (Do Not Use)

### 9. Combat System
**Status**: 🚫 Deferred  
**Reason**: Not part of trade simulator MVP  
**Files**: `Source/Adastrea/Combat/*.h/.cpp`

---

### 10. Exploration System
**Status**: 🚫 Deferred  
**Reason**: Not part of trade simulator MVP  
**Files**: `Source/Adastrea/Exploration/*.h/.cpp`

---

### 11. Quest System
**Status**: 🚫 Deferred  
**Reason**: Trading IS the game, no quests needed  
**Files**: `Source/Adastrea/Quest/*.h/.cpp`

---

### 12. Personnel System
**Status**: 🚫 Deferred  
**Reason**: Solo pilot only for MVP  
**Files**: `Source/Adastrea/Characters/*.h/.cpp`

---

### 13. AI System
**Status**: 🚫 Deferred (mostly)  
**MVP Exception**: Basic economy AI for price updates  
**Deferred**: NPC ships, faction AI, combat AI  
**Files**: `Source/Adastrea/AI/*.h/.cpp`

---

### 14. Way System
**Status**: 🚫 Deferred  
**Reason**: Not needed for simple station-to-station flight  
**Files**: `Source/Adastrea/Ways/*.h/.cpp`

---

### 15. Sector System
**Status**: 🚫 Deferred  
**Reason**: Single sector sufficient for MVP  
**Files**: `Source/Adastrea/Sectors/*.h/.cpp`

---

### 16. Map System
**Status**: 🚫 Deferred  
**Reason**: Simple waypoints sufficient  
**Files**: `Source/Adastrea/Maps/*.h/.cpp`

---

### 17. Player Progression
**Status**: 🚫 Deferred (mostly)  
**MVP Exception**: Credits tracking, ship upgrade unlocks  
**Deferred**: XP, levels, skills, achievements  
**Files**: `Source/Adastrea/Player/Progression*.h/.cpp`

---

### 18. Tutorial System
**Status**: 🚫 Deferred (initially)  
**When to Add**: Week 9-12 (polish phase)  
**Files**: `Source/Adastrea/Tutorial/*.h/.cpp`

---

### 19. Audio System
**Status**: 🚫 Deferred (initially)  
**When to Add**: Week 7-9 (basic sound effects)  
**Files**: `Source/Adastrea/Audio/*.h/.cpp`

---

### 20. Material System
**Status**: 🚫 Deferred  
**Reason**: Use simple materials for MVP  
**Files**: `Source/Adastrea/Materials/*.h/.cpp`

---

### 21. Homeworld System
**Status**: 🚫 Deferred  
**Reason**: Not relevant to trading MVP  
**Files**: `Source/Adastrea/Homeworlds/*.h/.cpp`

---

### 22. Performance Tools
**Status**: ⚠️ Use for metrics  
**MVP Scope**: Add FPS counter, memory tracking  
**Purpose**: Measure performance during development  
**Files**: Performance monitoring code

---

## 📊 Summary Statistics

| Classification | Count | Percentage |
|---------------|-------|------------|
| **MVP-Critical** | 5 systems | 23% |
| **MVP-Simplified** | 3 systems | 14% |
| **Deferred** | 14 systems | 63% |
| **Total** | 22 systems | 100% |

---

## 🎯 Development Guidelines

### When Adding Features

Ask three questions:
1. **Is this required for buy/sell/upgrade loop?**
   - If NO → Defer
2. **Can players have fun without this?**
   - If YES → Defer
3. **Would removing this break the core loop?**
   - If NO → Defer

### Code Organization

**Do NOT delete deferred systems**
- Leave C++ code as-is
- Don't create Blueprints for deferred systems
- Don't create Data Assets for deferred systems
- Don't write documentation for deferred systems

**Focus energy on MVP-Critical systems**
- Create all necessary Blueprints
- Create all necessary Data Assets
- Test thoroughly in gameplay
- Polish and iterate

---

## 🚨 Scope Creep Prevention

If someone asks to add:
- ❌ Combat mechanics → "After MVP validation"
- ❌ Crew management → "After MVP validation"
- ❌ Exploration → "After MVP validation"
- ❌ Complex quests → "Trading IS the game"
- ❌ Multiplayer → "After MVP validation"

**Standard Response**: "Great idea for v2.0! Let's validate the trading loop is fun first."

---

## 📅 Post-MVP Expansion

**If MVP succeeds (75% fun, 50% buy intent):**
1. Add one system at a time
2. Validate each addition with playtesters
3. Prioritize based on player feedback
4. Expand gradually, not all at once

**If MVP fails:**
- Pivot to different core loop
- OR simplify trading further
- OR abandon project

---

**Key Principle**: One polished system beats ten half-finished systems.

---

**Last Updated**: 2025-12-24  
**Next Review**: After Week 4 playtest results
