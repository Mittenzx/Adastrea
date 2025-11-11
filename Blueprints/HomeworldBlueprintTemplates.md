# Homeworld System - Blueprint Logic Templates

## Overview
This document provides detailed Blueprint implementation templates for the Homeworld-based faction relationship system. These templates show how to initialize and manage player political faction reputations based on their chosen homeworld.

**Note:** Homeworlds define relationships with political **factions**. For economic guild (Way) relationships, see the [Way System Guide](../Assets/WaySystemGuide.md).

---

## Table of Contents
1. [Player Character Setup](#player-character-setup)
2. [Character Creation Initialization](#character-creation-initialization)
3. [Reputation Query Functions](#reputation-query-functions)
4. [Reputation Modification](#reputation-modification)
5. [UI Integration](#ui-integration)
6. [Save/Load System](#saveload-system)

---

## Player Character Setup

### Required Variables in Player Character or Player State

Create these variables in your Player Character or Player State Blueprint:

```
Variable Name: FactionReputationMap
Type: Map
  Key Type: Object Reference → FactionDataAsset
  Value Type: Integer
Category: "Faction System"
Default Value: Empty Map
Tooltip: "Stores current reputation with all factions. Initialized from homeworld."
Replication: Replicated (if multiplayer)

Variable Name: CurrentHomeworld
Type: Object Reference → HomeworldDataAsset
Category: "Faction System"
Tooltip: "The homeworld this player character originated from."
Replication: Replicated (if multiplayer)

Variable Name: bReputationInitialized
Type: Boolean
Category: "Faction System"
Default Value: false
Tooltip: "True if faction reputation has been initialized from homeworld."
```

---

## Character Creation Initialization

### Template 1: Basic Initialization (Character Creation)

**Context:** Called when player completes character creation and selects a homeworld.

**Function Name:** InitializeFactionReputationFromHomeworld
**Function Type:** Custom Function
**Inputs:**
  - SelectedHomeworld (HomeworldDataAsset, Object Reference)
**Outputs:**
  - Success (Boolean)

**Blueprint Graph:**

```
┌─────────────────────────────────────┐
│ Function: Initialize Faction        │
│ Reputation From Homeworld           │
└─────────────────────────────────────┘

Input: SelectedHomeworld
   ↓
┌──────────────────────────────┐
│ Is Valid (SelectedHomeworld) │
└──────────────────────────────┘
   ↓
Branch
 ├─ False ──→ ┌──────────────────────┐
 │            │ Print String         │
 │            │ "Invalid Homeworld"  │
 │            └──────────────────────┘
 │               ↓
 │            Return False
 │
 └─ True ──→ ┌────────────────────────────────┐
              │ Set CurrentHomeworld           │
              │ (Store on Player Character)    │
              └────────────────────────────────┘
                 ↓
              ┌────────────────────────────────────┐
              │ Get Faction Relations              │
              │ Target: SelectedHomeworld          │
              │ Return: Array<FactionRelationEntry>│
              └────────────────────────────────────┘
                 ↓
              ┌────────────────────────┐
              │ Clear FactionReputationMap│
              │ (Start fresh)          │
              └────────────────────────┘
                 ↓
              ┌────────────────────────────────┐
              │ ForEachLoop                    │
              │ Array: Faction Relations       │
              └────────────────────────────────┘
                 ↓
              Loop Body:
                 ↓
              ┌────────────────────────────────────┐
              │ Break FFactionRelationEntry        │
              │ → Faction (UFactionDataAsset*)     │
              │ → StartingReputation (int32)       │
              └────────────────────────────────────┘
                 ↓
              ┌─────────────────────────┐
              │ Is Valid (Faction)      │
              └─────────────────────────┘
                 ↓
              Branch
               ├─ False ──→ Continue Loop
               │
               └─ True ──→ ┌────────────────────────────┐
                           │ Add to Map                 │
                           │ Map: FactionReputationMap  │
                           │ Key: Faction               │
                           │ Value: StartingReputation  │
                           └────────────────────────────┘
                              ↓
                           ┌─────────────────────────────┐
                           │ Print String (optional)     │
                           │ "Init {Faction.Name}: {Rep}"│
                           └─────────────────────────────┘
                              ↓
                           Continue Loop
              ↓
           Completed:
              ↓
           ┌──────────────────────────────────┐
           │ Set bReputationInitialized = true│
           └──────────────────────────────────┘
              ↓
           ┌────────────────────────────────────┐
           │ Broadcast Event:                   │
           │ OnFactionReputationInitialized     │
           └────────────────────────────────────┘
              ↓
           Return True
```

**Implementation Notes:**
- Call this function from your character creation screen after player selects homeworld
- The function validates the homeworld before processing
- Invalid faction references are skipped automatically
- Optional debug prints help verify initialization

### Template 2: Initialization with Event Dispatcher

**Event Dispatcher:** OnFactionReputationInitialized

Create this event dispatcher in your Player Character:

```
Event Dispatcher Name: OnFactionReputationInitialized
Inputs:
  - Homeworld (HomeworldDataAsset)
  - TotalFactionsInitialized (Integer)
```

**Usage in Initialization Function:**

```
After Loop Completed:
   ↓
┌────────────────────────────────────┐
│ Get Map Length                     │
│ Target: FactionReputationMap       │
│ Return: Integer (count)            │
└────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Call OnFactionReputationInitialized    │
│ Param 1: CurrentHomeworld              │
│ Param 2: Map Length                    │
└────────────────────────────────────────┘
```

**Binding to Event:**
Other systems (UI, quests, etc.) can bind to this event to react when reputation is initialized.

---

## Reputation Query Functions

### Template 3: Get Reputation with Faction

**Function Name:** GetFactionReputation
**Function Type:** Pure Function (no execution pins)
**Inputs:**
  - Faction (FactionDataAsset, Object Reference)
**Outputs:**
  - Reputation (Integer)
  - Found (Boolean)

**Blueprint Graph:**

```
┌────────────────────────────────┐
│ Function: Get Faction Reputation│
└────────────────────────────────┘

Input: Faction
   ↓
┌──────────────────────────┐
│ Is Valid (Faction)       │
└──────────────────────────┘
   ↓
Branch
 ├─ False ──→ Return: Reputation = 0, Found = false
 │
 └─ True ──→ ┌──────────────────────────────┐
              │ Find in Map                  │
              │ Map: FactionReputationMap    │
              │ Key: Faction                 │
              │ Return: Value, Found         │
              └──────────────────────────────┘
                 ↓
              Branch on Found
               ├─ True ──→ Return: Reputation = Value, Found = true
               │
               └─ False ──→ Return: Reputation = 0, Found = false
```

**Usage Example:**

```
Get Faction Reputation
  Input: DA_Faction_SolarisUnion
  Output: Reputation = 50, Found = true

Branch on Found:
  True → Display reputation in UI
  False → Show "No relationship established"
```

### Template 4: Get Reputation Status Text

**Function Name:** GetReputationStatusText
**Function Type:** Pure Function
**Inputs:**
  - Reputation (Integer)
**Outputs:**
  - StatusText (Text)
  - StatusColor (Linear Color)

**Blueprint Graph:**

```
┌──────────────────────────────────────┐
│ Function: Get Reputation Status Text │
└──────────────────────────────────────┘

Input: Reputation
   ↓
┌────────────────────────┐
│ Switch on Int          │
│ (Multiple conditions)  │
└────────────────────────┘
   ↓
   ├─ Reputation >= 75 ──→ Status = "Allied", Color = Bright Green
   ├─ Reputation >= 50 ──→ Status = "Very Friendly", Color = Green
   ├─ Reputation >= 25 ──→ Status = "Friendly", Color = Light Green
   ├─ Reputation >= 1 ──→ Status = "Neutral", Color = Gray
   ├─ Reputation >= -24 ──→ Status = "Unfriendly", Color = Orange
   ├─ Reputation >= -49 ──→ Status = "Very Unfriendly", Color = Red
   └─ Reputation <= -50 ──→ Status = "Hostile", Color = Dark Red
```

**Alternative Implementation using Select Node:**

```
Input: Reputation
   ↓
Create array of breakpoints: [75, 50, 25, 1, -24, -49, -100]
   ↓
Find which range Reputation falls into
   ↓
Select from Status options using index
```

### Template 5: Check Reputation Threshold

**Function Name:** IsReputationAtLeast
**Function Type:** Pure Function
**Inputs:**
  - Faction (FactionDataAsset)
  - Threshold (Integer)
**Outputs:**
  - Result (Boolean)

**Blueprint Graph:**

```
┌────────────────────────────────┐
│ Function: Is Reputation At Least│
└────────────────────────────────┘

Input: Faction, Threshold
   ↓
┌──────────────────────────────────┐
│ Get Faction Reputation           │
│ Input: Faction                   │
│ Output: Reputation, Found        │
└──────────────────────────────────┘
   ↓
┌────────────────────────────┐
│ Greater Than or Equal      │
│ A: Reputation              │
│ B: Threshold               │
│ Return: Boolean            │
└────────────────────────────┘
   ↓
Return Result
```

**Usage Example:**

```
Check if player can trade:
  IsReputationAtLeast(Faction, 0) → Returns true if neutral or better

Check if faction offers quests:
  IsReputationAtLeast(Faction, 25) → Returns true if friendly or better

Check if can dock at stations:
  IsReputationAtLeast(Faction, -24) → Returns true if not very hostile
```

---

## Reputation Modification

### Template 6: Modify Faction Reputation

**Function Name:** ChangeFactionReputation
**Function Type:** Custom Function
**Inputs:**
  - Faction (FactionDataAsset)
  - Amount (Integer) - positive or negative
  - Reason (Text) - optional, for logging
**Outputs:**
  - NewReputation (Integer)
  - Success (Boolean)

**Blueprint Graph:**

```
┌─────────────────────────────────┐
│ Function: Change Faction Reputation│
└─────────────────────────────────┘

Input: Faction, Amount, Reason
   ↓
┌──────────────────────────┐
│ Is Valid (Faction)       │
└──────────────────────────┘
   ↓
Branch
 ├─ False ──→ Return: Success = false, NewReputation = 0
 │
 └─ True ──→ ┌──────────────────────────────────┐
              │ Get Faction Reputation           │
              │ Input: Faction                   │
              │ Output: OldReputation, Found     │
              └──────────────────────────────────┘
                 ↓
              ┌──────────────────────────────────┐
              │ Add                              │
              │ A: OldReputation                 │
              │ B: Amount                        │
              │ Return: NewReputationRaw         │
              └──────────────────────────────────┘
                 ↓
              ┌──────────────────────────────────┐
              │ Clamp                            │
              │ Value: NewReputationRaw          │
              │ Min: -100                        │
              │ Max: 100                         │
              │ Return: NewReputation            │
              └──────────────────────────────────┘
                 ↓
              ┌────────────────────────────────────┐
              │ Add or Update Map Entry            │
              │ Map: FactionReputationMap          │
              │ Key: Faction                       │
              │ Value: NewReputation               │
              └────────────────────────────────────┘
                 ↓
              ┌────────────────────────────────────────┐
              │ Broadcast Event:                       │
              │ OnFactionReputationChanged             │
              │ Params: Faction, OldRep, NewRep, Reason│
              └────────────────────────────────────────┘
                 ↓
              ┌────────────────────────────────────────┐
              │ Optional: Log Change                   │
              │ "Reputation with {Faction}: {Old}→{New}"│
              └────────────────────────────────────────┘
                 ↓
              Return: Success = true, NewReputation
```

**Event Dispatcher:** OnFactionReputationChanged

```
Event Name: OnFactionReputationChanged
Inputs:
  - Faction (FactionDataAsset)
  - OldReputation (Integer)
  - NewReputation (Integer)
  - Reason (Text)
```

### Template 7: Reputation Change Triggers

**Common Scenarios for Reputation Changes:**

**Completing a Quest:**
```
On Quest Completed:
   ↓
Get Quest Faction
   ↓
Change Faction Reputation
  Faction: Quest Faction
  Amount: +15
  Reason: "Completed quest: {QuestName}"
```

**Attacking Faction Ships:**
```
On Faction Ship Destroyed:
   ↓
Get Ship Faction
   ↓
Change Faction Reputation
  Faction: Ship Faction
  Amount: -30
  Reason: "Destroyed faction ship"
```

**Trading:**
```
On Trade Transaction Complete:
   ↓
Get Station Faction
   ↓
Get Trade Value (in credits)
   ↓
Calculate Reputation Gain (Value / 1000, clamped to 1-5)
   ↓
Change Faction Reputation
  Faction: Station Faction
  Amount: Reputation Gain
  Reason: "Trade transaction"
```

---

## UI Integration

### Template 8: Character Creation Homeworld Selection

**Widget: WBP_CharacterCreation_HomeworldSelection**

**Variables:**
```
AvailableHomeworlds: Array<HomeworldDataAsset>
SelectedHomeworld: HomeworldDataAsset
CurrentHomeworldIndex: Integer
```

**Event Graph: On Widget Constructed**

```
Event: Construct
   ↓
┌──────────────────────────────────┐
│ Load Available Homeworlds        │
│ (Get from data table or folder)  │
└──────────────────────────────────┘
   ↓
┌──────────────────────────────────┐
│ Set AvailableHomeworlds Array    │
└──────────────────────────────────┘
   ↓
┌──────────────────────────────────┐
│ Set CurrentHomeworldIndex = 0    │
└──────────────────────────────────┘
   ↓
┌──────────────────────────────────┐
│ Call DisplayCurrentHomeworld     │
└──────────────────────────────────┘
```

**Function: DisplayCurrentHomeworld**

```
Function: DisplayCurrentHomeworld
   ↓
┌────────────────────────────────────────┐
│ Get from Array                         │
│ Array: AvailableHomeworlds             │
│ Index: CurrentHomeworldIndex           │
│ Return: CurrentHomeworld               │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Set Text: HomeworldNameTextBlock       │
│ Value: CurrentHomeworld.HomeworldName  │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Set Text: DescriptionTextBlock         │
│ Value: CurrentHomeworld.Description    │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Call DisplayFactionRelations           │
│ Input: CurrentHomeworld                │
└────────────────────────────────────────┘
```

**Function: DisplayFactionRelations**

```
Function: DisplayFactionRelations
Input: Homeworld
   ↓
┌────────────────────────────────────────┐
│ Clear Children                         │
│ Target: FactionRelationsPanel          │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Get Faction Relations                  │
│ Target: Homeworld                      │
│ Return: Array<FactionRelationEntry>    │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ ForEachLoop                            │
│ Array: Faction Relations               │
└────────────────────────────────────────┘
   ↓
Loop Body:
   ↓
┌────────────────────────────────────────┐
│ Break FFactionRelationEntry            │
│ → Faction                              │
│ → StartingReputation                   │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Create Widget                          │
│ Class: WBP_FactionRelationDisplay      │
│ Return: RelationWidget                 │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Set Faction Name                       │
│ Target: RelationWidget                 │
│ Value: Faction.FactionName             │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Set Reputation Value                   │
│ Target: RelationWidget                 │
│ Value: StartingReputation              │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Get Reputation Status                  │
│ Input: StartingReputation              │
│ Return: StatusText, StatusColor        │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Set Status Display                     │
│ Target: RelationWidget                 │
│ Text: StatusText                       │
│ Color: StatusColor                     │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Add Child to Panel                     │
│ Target: FactionRelationsPanel          │
│ Content: RelationWidget                │
└────────────────────────────────────────┘
   ↓
Continue Loop
```

**Button Handlers:**

```
Button: NextHomeworld
OnClicked:
   ↓
Increment CurrentHomeworldIndex
   ↓
Wrap around if exceeds array length
   ↓
Call DisplayCurrentHomeworld

Button: PreviousHomeworld
OnClicked:
   ↓
Decrement CurrentHomeworldIndex
   ↓
Wrap around if below 0
   ↓
Call DisplayCurrentHomeworld

Button: ConfirmHomeworld
OnClicked:
   ↓
Get Selected Homeworld
   ↓
Store in Player Character
   ↓
Call InitializeFactionReputationFromHomeworld
   ↓
Close Character Creation Widget
   ↓
Start Game
```

### Template 9: In-Game Faction Reputation Display

**Widget: WBP_FactionReputationPanel**

**Function: Update Reputation Display**

```
Function: UpdateReputationDisplay
Input: Faction (optional, if null shows all)
   ↓
Branch: Is Faction Valid?
 ├─ True ──→ Display Single Faction
 │             ↓
 │          Get Faction Reputation
 │             ↓
 │          Update UI for that faction
 │
 └─ False ──→ Display All Factions
                 ↓
              Get Player Character
                 ↓
              Get FactionReputationMap
                 ↓
              ForEachLoop (Map Keys and Values)
                 ↓
              Create UI entry for each faction
                 ↓
              Sort by reputation (highest first)
```

---

## Save/Load System

### Template 10: Saving Faction Reputation

**Function: SaveFactionReputation**

```
Function: SaveFactionReputation
Input: SaveGameObject
   ↓
┌────────────────────────────────────────┐
│ Get Player Character                   │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Get FactionReputationMap               │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Convert Map to Arrays                  │
│ (Map Keys → Faction Array)             │
│ (Map Values → Reputation Array)        │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Store Arrays in SaveGameObject         │
│ - SavedFactions (Array<FactionDataAsset>)│
│ - SavedReputations (Array<Integer>)    │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Store CurrentHomeworld reference       │
│ SaveGameObject.PlayerHomeworld         │
└────────────────────────────────────────┘
```

### Template 11: Loading Faction Reputation

**Function: LoadFactionReputation**

```
Function: LoadFactionReputation
Input: SaveGameObject
   ↓
┌────────────────────────────────────────┐
│ Get Saved Arrays                       │
│ - SavedFactions                        │
│ - SavedReputations                     │
└────────────────────────────────────────┘
   ↓
┌────────────────────────────────────────┐
│ Validate arrays have same length       │
└────────────────────────────────────────┘
   ↓
Branch: Valid?
 ├─ False ──→ Log Error, Use Default
 │
 └─ True ──→ Clear FactionReputationMap
                 ↓
              ForLoop (0 to Array Length)
                 ↓
              Get Faction at index
                 ↓
              Get Reputation at index
                 ↓
              Add to FactionReputationMap
                 ↓
              Continue Loop
                 ↓
              Load CurrentHomeworld reference
                 ↓
              Set bReputationInitialized = true
```

---

## Complete Example: Player Character Blueprint

### Full Implementation Checklist

1. **Add Variables**
   - FactionReputationMap (Map<FactionDataAsset, Integer>)
   - CurrentHomeworld (HomeworldDataAsset)
   - bReputationInitialized (Boolean)

2. **Add Functions**
   - InitializeFactionReputationFromHomeworld
   - GetFactionReputation
   - ChangeFactionReputation
   - GetReputationStatusText
   - IsReputationAtLeast

3. **Add Event Dispatchers**
   - OnFactionReputationInitialized
   - OnFactionReputationChanged

4. **Add Event Handlers**
   - BeginPlay: Check if reputation needs initialization
   - OnPossessed: Set up player-specific reputation data

### Example BeginPlay Implementation

```
Event: BeginPlay
   ↓
Branch: Is bReputationInitialized?
 ├─ True ──→ Continue (already initialized)
 │
 └─ False ──→ ┌────────────────────────────────┐
               │ Try Load from Save Game        │
               └────────────────────────────────┘
                  ↓
               Branch: Save Data Found?
                ├─ True ──→ Load Faction Reputation
                │
                └─ False ──→ ┌─────────────────────────────┐
                             │ Use Default/Starting Homeworld│
                             └─────────────────────────────┘
                                ↓
                             Initialize From Homeworld
```

---

## Testing and Debugging

### Debug Commands

**Console Command: ShowFactionReputation**

```
Custom Function: Debug_ShowFactionReputation
   ↓
Get All Map Keys (FactionReputationMap)
   ↓
ForEachLoop:
   ↓
Get Faction Name
   ↓
Get Reputation Value
   ↓
Print to Screen: "{FactionName}: {Reputation}"
```

**Console Command: SetFactionReputation**

```
Custom Function: Debug_SetFactionReputation
Inputs: FactionID (Name), NewReputation (Integer)
   ↓
Find Faction by ID
   ↓
Change Faction Reputation
   ↓
Print Result
```

### Testing Checklist

- [ ] Create test homeworld with varied reputation values
- [ ] Initialize player character with test homeworld
- [ ] Verify FactionReputationMap populated correctly
- [ ] Test GetFactionReputation for existing and non-existing factions
- [ ] Test ChangeFactionReputation with positive and negative values
- [ ] Test reputation clamping at -100 and +100
- [ ] Test save/load preserves reputation data
- [ ] Test UI displays reputation correctly
- [ ] Test event dispatchers fire at appropriate times
- [ ] Test multiplayer replication (if applicable)

---

## Performance Considerations

### Best Practices

1. **Use Maps, Not Arrays**: TMap provides O(1) lookup vs O(n) for arrays
2. **Cache Faction References**: Don't look up factions repeatedly in tight loops
3. **Batch Reputation Changes**: If changing multiple factions, batch the events
4. **Limit UI Updates**: Only update faction UI when visible or changed
5. **Async Loading**: Load faction assets asynchronously if possible

### Optimization Example

**Bad: Repeated Lookups**
```
For each quest reward:
  Get Faction Reputation (lookup each time)
  Calculate new reputation
  Set Reputation
```

**Good: Batched Processing**
```
Get all faction reputations once
For each quest reward:
  Calculate new reputation (cached data)
Batch update all changed reputations
Broadcast single event with all changes
```

---

## Summary

These Blueprint templates provide a complete foundation for implementing the homeworld-based faction relationship system. Key points:

1. Initialize reputation from homeworld at character creation
2. Store reputation in TMap for efficient lookups
3. Provide query functions for gameplay systems
4. Use event dispatchers for UI and system updates
5. Implement save/load for persistence
6. Test thoroughly with various homeworlds and scenarios

For additional support, refer to:
- Homeworld System Guide: [Assets/HomeworldSystemGuide.md](Assets/HomeworldSystemGuide.md)
- Faction Setup Guide: [Assets/FactionSetupGuide.md](Assets/FactionSetupGuide.md)
- General Blueprint Workflows: [Assets/BlueprintWorkflowTemplates.md](Assets/BlueprintWorkflowTemplates.md)
