# AI System Integration Guide
## Connecting Data Assets, Logic, and Game Systems

This guide shows how to integrate the AI framework into your actual game, connecting data assets and logic blueprints to ships, stations, characters, and gameplay systems.

---

## Table of Contents
1. [Overview](#overview)
2. [Integration Patterns](#integration-patterns)
3. [Faction AI Integration](#faction-ai-integration)
4. [Personnel AI Integration](#personnel-ai-integration)
5. [Connecting to Game Systems](#connecting-to-game-systems)
6. [Event Communication](#event-communication)
7. [Performance Optimization](#performance-optimization)
8. [Complete Examples](#complete-examples)

---

## Overview

### The Integration Flow

```
Game System (Ship/Station/Character)
    ↓ owns/references
Logic Blueprint (FactionLogic/PersonnelLogic)
    ↓ references
Data Asset (FactionDataAsset/PersonnelDataAsset)
    ↑ data flows back up
    ↓ decisions flow down
Game System executes actions
```

### Three Integration Approaches

1. **Component-Based** - Add Logic as a component to Actor
2. **Reference-Based** - Store Logic as a variable, create separately
3. **Manager-Based** - Centralized AI manager controls all AI

**Recommendation:** Start with Component-Based for simplicity.

---

## Integration Patterns

### Pattern 1: Component-Based (Recommended)

**Best for:** Ships, Stations, Characters that exist as Actors

**Setup:**

1. Open your Actor Blueprint (e.g., `BP_Spaceship`)
2. Add Component → **FactionLogic** or **PersonnelLogic**
3. Name it (e.g., `FactionAI` or `PersonnelAI`)
4. Configure component:
   - Assign Data Asset
   - Set AI properties
5. In BeginPlay:
   ```
   Get Component → Initialize AI → Update AI (in Tick)
   ```

**Pros:**
- ✅ Simple to set up
- ✅ AI automatically destroyed with Actor
- ✅ Easy to find and debug

**Cons:**
- ❌ One AI per Actor (can't share)
- ❌ Recreated if Actor respawns

### Pattern 2: Reference-Based

**Best for:** Persistent AI, multiple Actors using same AI

**Setup:**

1. Create Logic Blueprint Actor (not component)
2. In game Actor, add variable:
   - Type: `FactionLogic` or `PersonnelLogic`
   - Name: `MyAI`
3. In BeginPlay:
   ```
   Spawn Actor of Class (Logic Blueprint)
   → Set MyAI = spawned actor
   → Initialize AI
   ```
4. In Tick:
   ```
   Get MyAI → Update AI
   ```
5. In EndPlay:
   ```
   Get MyAI → Destroy Actor (or keep alive)
   ```

**Pros:**
- ✅ Can persist beyond Actor lifetime
- ✅ Multiple Actors can reference same AI
- ✅ More control over lifecycle

**Cons:**
- ❌ More complex setup
- ❌ Must manually manage lifecycle

### Pattern 3: Manager-Based

**Best for:** Many AI, centralized control, complex systems

**Setup:**

1. Create `BP_AIManager` Actor
2. Add arrays:
   - `FactionAIs` (array of FactionLogic)
   - `PersonnelAIs` (array of PersonnelLogic)
3. Add functions:
   - `RegisterFactionAI` - Add to array
   - `RegisterPersonnelAI` - Add to array
   - `UpdateAllAI` - Tick all AI
4. Place in level, make persistent
5. Other Actors register their AI with manager

**Pros:**
- ✅ Centralized control
- ✅ Easy to pause/control all AI
- ✅ Can optimize update order
- ✅ Good for save/load

**Cons:**
- ❌ Most complex setup
- ❌ Single point of failure
- ❌ Requires more infrastructure

---

## Faction AI Integration

### Integrating with Space Stations

**Scenario:** Station owned by a faction, AI controls station behavior

**Setup:**

1. Open `BP_SpaceStation` (or your station Blueprint)
2. Add Component → **FactionLogic**
3. Add variable: `FactionData` (FactionDataAsset reference)
4. In BeginPlay:
   ```
   Get FactionAI Component
   → Set Faction Data = [Your Faction Data Asset]
   → Initialize AI
   ```
5. In Tick:
   ```
   Get FactionAI Component → Update AI (DeltaSeconds)
   ```

**Responding to AI Decisions:**

```
Event "On Territory Discovered" (from FactionLogic):
  → Add Station Module (expansion)
  → Upgrade Station (if high value territory)
  → Notify Player

Event "Handle Diplomatic Interaction" (from FactionLogic):
  → Allow/Deny Docking based on relationship
  → Adjust Trade Prices based on relationship
  → Send Diplomatic Message
```

### Integrating with Fleet Management

**Scenario:** Faction controls a fleet of ships

**Setup:**

1. Create `BP_FleetManager` Actor
2. Add FactionLogic component
3. Add array of ship references
4. In FactionLogic Blueprint, override:
   ```
   Event "Evaluate Strategic Priority":
     → If priority is "Expansion":
       - Send fleet to new territory
     → If priority is "Defense":
       - Recall fleet to stations
     → If priority is "Trade":
       - Send fleet on trade routes
   ```

### Faction Territory Control

**Connecting Territories to Faction AI:**

```
In Territory Actor:
1. Add variable: ControllingFaction (FactionDataAsset)
2. When faction discovers territory:
   a. Get Faction's FactionLogic
   b. Call "On Territory Discovered"
   c. If faction claims it:
      - Set ControllingFaction = Faction Data
      - Apply faction traits to territory
      - Update faction territory count
```

### Faction Diplomacy Integration

**Connecting Relationship System:**

```
In Diplomatic Event (meeting, trade, conflict):
1. Get both Factions' FactionLogic components
2. Call "Handle Diplomatic Interaction" on both
3. Update relationship values in both Data Assets
4. Check if truces should be added/removed
5. Trigger game events based on new relationship
```

---

## Personnel AI Integration

### Integrating with Character Actors

**Scenario:** Crew member with AI controls behavior

**Setup:**

1. Open `BP_CrewMember` (or character Blueprint)
2. Add Component → **PersonnelLogic**
3. In BeginPlay:
   ```
   Get PersonnelAI Component
   → Set Personnel Data = [Your Personnel Data Asset]
   → Set Disposition = [Curious/Disciplined/etc]
   → Initialize AI
   ```
4. In Tick:
   ```
   Get PersonnelAI Component → Update AI (DeltaSeconds)
   ```

**Animating Based on AI:**

```
Event Tick:
1. Get Current Task from PersonnelAI
2. Switch on Task:
   - DutyStation → Play Work Animation
   - Social → Play Talk Animation
   - Personal → Play Relax Animation
   - Training → Play Study Animation
   - Exploration → Play Investigate Animation
3. Set Animation State
```

### Crew Assignment System

**Assigning Personnel to Stations:**

```
In Station/Ship Blueprint:
1. Add array: AssignedCrew (PersonnelLogic references)
2. Add function: AssignCrewMember
   → Add to array
   → Update PersonnelData.CurrentAssignment
   → Call PersonnelLogic.Initialize AI
3. Add function: UpdateCrewSchedule
   → For each crew:
     - Update HoursUntilNextShift
     - Check if needs rest
     - Reassign if needed
```

### Skill-Based Task Assignment

**Using Skills to Assign Tasks:**

```
When Task Needs Personnel:
1. Get all available personnel
2. For each personnel:
   a. Get their skills from PersonnelData
   b. Check if skills match task requirements
   c. Calculate suitability score
3. Assign personnel with highest score
4. Update their PersonnelLogic.CurrentTask
```

### Social Interaction System

**Personnel Interacting with Each Other:**

```
When Two Personnel Meet:
1. Get both PersonnelLogic components
2. Call "Handle Social Interaction" on both
3. Pass each other's Personnel ID
4. Check if friends/conflicts develop:
   a. If positive interaction:
      - Increase relationship strength
      - Check if should become friends
   b. If negative interaction:
      - Decrease relationship strength
      - Check if conflict develops
5. Update mood/morale based on interaction
```

### Daily Schedule System

**Managing Crew Schedules:**

```
In Shift Management System:
1. Every game hour:
   a. For each personnel:
      - Call "Update Daily Routine"
      - Check if shift starts/ends
      - Update On Duty status
2. Handle shift changes:
   - Notify personnel
   - Update work assignments
   - Trigger animations/locations
```

---

## Connecting to Game Systems

### Trade System

**Faction AI → Trade Decisions:**

```
In Trade System:
1. When trade opportunity appears:
   a. Get Faction's FactionLogic
   b. Call "Should Initiate Trade" with partner faction
   c. If true:
      - Calculate prices using Trade Modifier
      - Execute trade
      - Update economic strength
```

### Combat System

**Faction AI → Combat Decisions:**

```
In Combat System:
1. When encounter occurs:
   a. Get both Factions' FactionLogic
   b. Check "Is At War With"
   c. Check "Can Peacefully Interact"
   d. If peaceful:
      - Call "Handle Diplomatic Interaction"
   e. If hostile:
      - Initiate combat
      - Update relationship values
```

### Research System

**Faction Priorities → Research Focus:**

```
In Research System:
1. Every research cycle:
   a. Get Faction's Strategic Priority
   b. If priority is "Research":
      - Allocate more resources
      - Apply faction trait modifiers
   c. When research completes:
      - Update faction TechnologyLevel
      - Trigger "On Research Complete" event
```

### Morale System

**Personnel AI → Crew Morale:**

```
In Morale System:
1. Each update cycle:
   a. For each personnel:
      - Get current morale from PersonnelData
      - Check if needs rest
      - Check social interactions
   b. Apply morale effects:
      - If high morale: work bonuses
      - If low morale: work penalties
   c. If morale critical:
      - Trigger events (complaints, requests)
```

### Station Management

**AI-Driven Station Behavior:**

```
In Station Actor:
1. FactionAI determines:
   - Trade policies (prices, restrictions)
   - Docking permissions
   - Module construction priorities
   - Defense readiness

2. PersonnelAI determines:
   - Staff assignments
   - Work schedules
   - Maintenance priorities
   - Social areas usage
```

---

## Event Communication

### From AI to Game (AI Makes Decisions)

**Use Events and Delegates:**

```
In FactionLogic Blueprint:
1. Create Event Dispatcher: "OnStrategicPriorityChanged"
2. Override "Update Strategic Goals":
   a. Call parent
   b. If priority changed:
      - Call Event Dispatcher
3. In game system:
   a. Bind to Event Dispatcher
   b. React to priority changes
```

**Example Events:**
- `OnTerritoryDiscovered` → Expand map UI
- `OnDiplomaticRelationChange` → Update faction colors
- `OnTradeInitiated` → Show trade screen
- `OnTaskCompleted` → Award experience/rewards

### From Game to AI (Game Events Trigger AI)

**Call AI Functions Directly:**

```
When Player Attacks Faction Station:
1. Get Station's FactionLogic
2. Modify relationship value
3. Call "Handle Diplomatic Interaction" with player faction
4. FactionAI responds (may declare war, demand reparations, etc.)
```

**Example Triggers:**
- Player actions → Update faction relationships
- Discovery event → Call `OnTerritoryDiscovered`
- Crew member hurt → Update morale, trigger stress
- Research complete → Update faction tech level

### Bidirectional Communication

**AI ↔ Game System:**

```
Example: Trade Negotiation

Game System:
1. Player initiates trade with NPC faction
2. Call FactionAI.ShouldInitiateTrade
3. FactionAI checks relationships, priorities
4. Returns true/false

FactionAI (if accepted):
1. Calls Event Dispatcher "OnTradeAccepted"
2. Game System receives event
3. Opens trade UI with faction's Trade Modifier applied

Game System (after trade):
1. Calls FactionAI.UpdateStrategicGoals
2. Updates Economic Strength based on trade
3. FactionAI may adjust priorities based on new strength
```

---

## Performance Optimization

### Update Frequency Management

**Different update rates for different AI:**

```
In AI Manager:
1. Factions update every 5-10 seconds (slow strategic thinking)
2. Active personnel update every 1 second (responsive)
3. Background personnel update every 5 seconds (less important)
4. Inactive AI paused (far away, not relevant)
```

### Level of Detail (LOD) for AI

```
Distance-Based AI Detail:
1. Near player (< 50m):
   - Full AI updates
   - All decisions
   - Animation updates

2. Medium distance (50-500m):
   - Reduced update frequency
   - Essential decisions only
   - Simplified animations

3. Far distance (> 500m):
   - Minimal updates or paused
   - State only
   - No animations
```

### Async AI Processing

**For many AI:**

```
In AI Manager (advanced):
1. Split AI into batches
2. Update batch 1 on frame 1
3. Update batch 2 on frame 2
4. Update batch 3 on frame 3
5. Repeat

Result: Spread AI cost over multiple frames
```

### Selective AI Activation

```
Smart Activation:
1. Only activate AI that matters:
   - Player can see them
   - Player can interact with them
   - Involved in active events
2. Keep others in "dormant" state
3. Quick activate when needed
4. Deactivate when no longer relevant
```

---

## Complete Examples

### Example 1: Peaceful Exploration Ship

**Full Integration:**

```
BP_ExplorationShip (inherits from Spaceship):

Components:
- FactionLogic (PeacefulExplorersFaction)
- PersonnelLogic x 5 (Captain, Pilot, Engineer, Scientist, Medic)

BeginPlay:
1. Initialize FactionAI with DA_PeacefulExplorers
2. For each PersonnelAI:
   - Assign Personnel Data Asset
   - Set Disposition
   - Initialize AI
3. Start AI update loop

Tick:
1. Update FactionAI (every 5 seconds)
2. Update each PersonnelAI (every 1 second)
3. Animate crew based on current tasks
4. Update ship behavior based on faction priorities

Custom Events:
- OnNewSystemDiscovered:
  → Trigger FactionAI.OnTerritoryDiscovered
  → If faction claims it:
    - Establish outpost
    - Send notification
    
- OnMeetAnotherShip:
  → Get other ship's faction
  → Trigger FactionAI.HandleDiplomaticInteraction
  → If should trade:
    - Initiate trade sequence
  → If should avoid:
    - Change course
```

### Example 2: Trading Station

**Full Integration:**

```
BP_TradeStation (inherits from SpaceStation):

Components:
- FactionLogic (MerchantCoalition)
- PersonnelLogic array (station staff)

BeginPlay:
1. Initialize Station FactionAI
2. Spawn and initialize staff PersonnelAI
3. Set up trade price modifiers from faction data

Tick:
1. Update FactionAI
2. Update staff AI
3. Adjust trade prices based on:
   - Faction relationships (Trade Modifier)
   - Economic strength
   - Staff morale

Custom Events:
- OnPlayerDocking:
  → Check FactionAI.CanPeacefullyInteract with player
  → If yes:
    - Allow docking
    - Apply faction trade modifier to prices
  → If no:
    - Deny docking or apply penalties

- OnTradeTransaction:
  → Update FactionAI Economic Strength
  → Update staff morale (successful trade)
  → Check if should adjust priorities
```

### Example 3: Multi-Crew Ship

**Full Integration:**

```
BP_CrewShip (inherits from Spaceship):

Components:
- PersonnelLogic array (full crew)

Variables:
- CrewRoster (map of Role → PersonnelLogic)
- ShiftSchedule (current shift assignments)

BeginPlay:
1. For each crew role:
   - Create PersonnelAI
   - Assign to roster
   - Initialize with Data Asset
2. Set up shift schedule
3. Assign initial tasks

Tick:
1. Update all active crew AI
2. Check for shift changes
3. Handle social interactions
4. Update ship systems based on crew:
   - Engineering efficiency (engineer morale)
   - Navigation accuracy (pilot skill + morale)
   - Combat effectiveness (security officer state)

Daily Schedule System:
- Morning shift:
  → Set crew On Duty
  → Assign duty stations
- Evening shift:
  → Set crew Off Duty
  → Allow social time
- Night shift:
  → Reduced crew
  → Rest time

Social System:
- When crew members interact:
  → Call HandleSocialInteraction on both
  → Update relationships
  → Affect morale
- Friend bonuses:
  → Working with friends = morale boost
- Conflict penalties:
  → Working with rivals = morale reduction
```

---

## Next Steps

1. ✅ **Choose Integration Pattern** for your game
2. ✅ **Start with one system** (e.g., ships OR stations)
3. ✅ **Test thoroughly** before adding complexity
4. ✅ **Iterate** on integration points
5. ✅ **Optimize** once working
6. ✅ **Expand** to other systems

---

## Troubleshooting Integration

**Q: AI isn't affecting game systems**
- A: Check event bindings/delegates
- A: Verify AI functions are being called
- A: Add debug prints to trace execution

**Q: Game events not reaching AI**
- A: Ensure AI components/references are valid
- A: Check function calls have correct parameters
- A: Verify AI is initialized before game calls it

**Q: Performance issues**
- A: Reduce update frequency
- A: Use LOD system
- A: Deactivate distant/irrelevant AI
- A: Profile to find bottlenecks

**Q: AI decisions don't match game state**
- A: Ensure Data Assets are updated with game changes
- A: Call appropriate AI update functions
- A: Check bidirectional communication

---

## See Also

- [Faction AI Setup Guide](FactionAISetupGuide.md) - Creating faction AI
- [Personnel AI Setup Guide](PersonnelAISetupGuide.md) - Creating personnel AI
- [AI Designer Workflow](AIDesignerWorkflow.md) - Designer-focused workflow
- [Templates](FactionAITemplates/) - Pre-made configurations
