# Faction AI Setup Guide
## Complete Blueprint Workflow for Data-Driven Faction AI

This guide provides step-by-step instructions for setting up faction-level AI in Unreal Engine using Blueprints. No C++ knowledge required!

---

## Table of Contents
1. [Overview](#overview)
2. [Creating Faction Data Assets](#creating-faction-data-assets)
3. [Creating Faction Logic Blueprints](#creating-faction-logic-blueprints)
4. [Configuring Peaceful Early-Game AI](#configuring-peaceful-early-game-ai)
5. [Implementing Custom Behaviors](#implementing-custom-behaviors)
6. [Testing Your Faction AI](#testing-your-faction-ai)
7. [Common Patterns and Examples](#common-patterns-and-examples)

---

## Overview

### What is Faction AI?

Faction AI handles **macro-level strategic and political decisions** for entire factions:
- **Territory expansion** and control
- **Political diplomacy** with other factions  
- **Military strategies** and warfare
- **High-level resource allocation**
- **Research priorities** and technology

**Note:** Day-to-day economic activities, trade, and production are handled by the [Way System](WaySystemGuide.md) AI. Faction AI focuses on political and military strategy, while Way AI handles economic operations and guild management.

### Key Components

1. **UFactionDataAsset** (C++ Data Asset) - Stores faction data (traits, relationships, stats)
2. **UFactionLogic** (C++ Logic Class) - Provides AI behavior with Blueprint extension points
3. **Blueprint Logic** (Your Custom Logic) - Override behaviors for unique faction personalities

### The Blueprint-Native Event Pattern

All AI logic uses **BlueprintNativeEvent** functions:
- **C++ provides the default behavior** (you don't need to override everything)
- **Blueprints can override** any function to customize behavior
- **Call Parent Function** to use C++ defaults when needed

---

## Creating Faction Data Assets

### Step 1: Create a Faction Data Asset

1. In Content Browser, **Right-Click** → **Blueprint Class**
2. Search for **"FactionDataAsset"**
3. Select **FactionDataAsset** as parent class
4. Name it (e.g., `DA_PeacefulExplorers`)
5. **Double-click** to open the asset

### Step 2: Configure Basic Info

Fill in these required fields:

**Faction Info:**
- `Faction Name` - Display name (e.g., "Peaceful Explorers")
- `Description` - Brief faction background
- `Faction ID` - Unique identifier (e.g., "PeacefulExplorers")
- `Primary Color` - Main faction color (RGB)
- `Secondary Color` - Accent color

**Faction Attributes:**
- `Technology Level` (1-10) - How advanced the faction is
- `Military Strength` (1-10) - Combat capability
- `Economic Power` (1-10) - Economic influence
- `Initial Reputation` (-100 to 100) - Starting relationship with player
- `Hostile By Default` (checkbox) - Whether faction is initially hostile

### Step 3: Add Faction Traits

Traits define unique characteristics and bonuses:

1. In the asset editor, find **"Traits"** array
2. Click **+** to add a new trait
3. Configure each trait:
   - `Trait ID` - Unique identifier (e.g., "ScientificMindset")
   - `Trait Name` - Display name
   - `Trait Description` - What it does
   - `Modifier Value` - Numerical bonus (1.25 = 25% bonus)

**Example Peaceful Faction Traits:**
```
Trait 1:
  Trait ID: ScientificMindset
  Trait Name: Scientific Mindset
  Description: Prioritizes research and discovery
  Modifier Value: 1.25 (25% research bonus)

Trait 2:
  Trait ID: PacifistPhilosophy
  Trait Name: Pacifist Philosophy
  Description: Avoids conflict, gains diplomatic bonuses
  Modifier Value: 1.3 (30% diplomacy bonus)
```

### Step 4: Configure Diplomacy

Define relationships with other factions:

1. Find **"Faction Relationships"** array
2. Click **+** to add relationships
3. For each relationship:
   - `Target Faction ID` - The other faction's ID
   - `Relationship Value` (-100 to 100) - How friendly (positive) or hostile (negative)
   - `Is Allied` - Formal alliance (checkbox)
   - `At War` - Active warfare (checkbox)
   - `Trade Modifier` (0-2) - Trade efficiency multiplier

**Example Peaceful Faction Relationships:**
```
Relationship 1:
  Target Faction ID: SolarisUnion
  Relationship Value: 50 (friendly)
  Is Allied: false
  At War: false
  Trade Modifier: 1.2 (20% better trade)

Relationship 2:
  Target Faction ID: ResearchCollective
  Relationship Value: 60 (very friendly)
  Is Allied: true
  At War: false
  Trade Modifier: 1.3 (30% better trade)

Relationship 3:
  Target Faction ID: PirateClans
  Relationship Value: -60 (hostile)
  Is Allied: false
  At War: false
  Trade Modifier: 0.5 (poor trade)
```

### Step 5: Use YAML Templates

Copy values from YAML templates in `Assets/FactionAITemplates/`:
- `PeacefulExplorers.yaml` - Exploration-focused peaceful faction
- `TradeCoalition.yaml` - Commerce-focused peaceful faction

---

## Creating Faction Logic Blueprints

### Step 1: Create the Blueprint

1. **Right-Click** in Content Browser → **Blueprint Class**
2. Search for **"FactionLogic"**
3. Select **FactionLogic** as parent
4. Name it (e.g., `BP_PeacefulExplorerLogic`)
5. **Double-click** to open

### Step 2: Assign Faction Data

In the Blueprint editor:

1. Select the **root component** (or Class Defaults)
2. Find **"Faction Data"** property
3. Click dropdown and select your Faction Data Asset (e.g., `DA_PeacefulExplorers`)

### Step 3: Configure Early Game Priorities

For **peaceful exploration** gameplay:

1. Find **"Early Game Priorities"** struct
2. Set priority values (0-10 scale, higher = more important):

**Recommended Peaceful/Exploration Settings:**
```
Exploration Priority: 9 (very high)
Trade Priority: 7 (high)
Diplomacy Priority: 8 (high)
Research Priority: 7 (high)
Expansion Priority: 4 (medium-low, peaceful only)
Defense Priority: 3 (low, minimal military)
```

3. Set **"Is Early Game"** to **true**
4. Set **"Current Behavior Mode"** to **"Peaceful"**

### Step 4: Configure Truces

For peaceful gameplay, add factions to the **"Truce List"**:

1. Find **"Truce List"** array
2. Add faction IDs you want temporary peace with:
   - `SolarisUnion`
   - `TradeGuild`
   - `ResearchCollective`

Truces prevent conflict even if relationships are neutral.

---

## Configuring Peaceful Early-Game AI

### Understanding Early Game Priorities

When **"Is Early Game"** is true:
- AI uses **Early Game Priorities** to make decisions
- **Peaceful** and **Exploration** are prioritized
- **Military** actions are minimized
- **Diplomatic** and **Trade** interactions are favored

### Transition Out of Early Game

The faction automatically transitions when:
- Territory Count ≥ 5, OR
- Economic Strength ≥ 70, OR
- Military Power ≥ 60

You can override this in Blueprints (see Custom Behaviors).

### Peaceful Expansion Strategy

Set priorities for **non-aggressive** growth:
1. **High Exploration** (8-10) - Discover new territories
2. **Medium Expansion** (4-6) - Claim unclaimed space only
3. **High Diplomacy** (7-9) - Build alliances before expanding
4. **Low Defense** (2-4) - Minimal military buildup

### Trade-Focused Strategy

For **economic** peaceful factions:
1. **Very High Trade** (9-10) - Primary focus
2. **High Diplomacy** (7-9) - Good relations = good business
3. **Medium Exploration** (5-7) - Find trade routes
4. **High Expansion** (6-8) - Build trade posts

---

## Implementing Custom Behaviors

### Override BlueprintNativeEvents

Right-click in Event Graph → **Add Event** → **Event [Function Name]**

### Key Events to Override

#### 1. Event Evaluate Strategic Priority

**When:** Called every AI tick to determine what the faction should focus on  
**Use:** Customize how faction chooses priorities

```
Example: Force Exploration in Early Game
1. Add Event: "Event Evaluate Strategic Priority"
2. Add Branch: If "Is Early Game" is true
   - Return "Exploration" (EFactionPriority enum)
3. Else: Call Parent Function (default behavior)
```

#### 2. Event Handle Diplomatic Interaction

**When:** Called when faction meets another faction  
**Use:** Custom diplomacy behavior

```
Example: Always Friendly to Scientists
1. Add Event: "Event Handle Diplomatic Interaction"
2. Get Other Faction Data (from ID)
3. Check if other faction has "ScientificMindset" trait
4. If yes:
   - Add them to Truce List
   - Initiate Trade
5. Else: Call Parent Function
```

#### 3. Event On Territory Discovered

**When:** Faction discovers a new territory  
**Use:** Custom expansion logic

```
Example: Only Claim High-Value Territories
1. Add Event: "Event On Territory Discovered"
2. Check "Territory Value" input
3. If Territory Value > 60 AND Is Peaceful Expansion:
   - Claim Territory (increment Territory Count)
4. Print log message
```

#### 4. Event Should Initiate Trade

**When:** Deciding whether to trade with another faction  
**Use:** Custom trade logic

```
Example: Trade with Anyone Not at War
1. Add Event: "Event Should Initiate Trade"
2. Call "Is At War With" function with Other Faction ID
3. If NOT at war:
   - Return TRUE
4. Else:
   - Return FALSE
```

#### 5. Event Update Strategic Goals

**When:** Called periodically to re-evaluate faction goals  
**Use:** Dynamic priority adjustment

```
Example: Switch to Defense if Under Threat
1. Add Event: "Event Update Strategic Goals"
2. Check if Military Power < 30 AND Territory Count > 2
3. If true:
   - Set Current Strategic Priority to "Defense"
   - Set Behavior Mode to "Defensive"
4. Else: Call Parent Function
```

### Using Query Functions

These functions help make decisions:

**Diplomacy Queries:**
- `Has Truce With` - Check if truce exists
- `Can Peacefully Interact` - Safe to engage?
- `Is Exploration Focused` - Currently exploring?
- `Is Trade Focused` - Currently trading?

**Example Usage:**
```
If "Can Peacefully Interact" with Other Faction:
  - Initiate Trade
  - Build relationship
Else:
  - Avoid contact
  - Increase defense
```

---

## Testing Your Faction AI

### Step 1: Add to Game World

Create a **Faction Manager** Blueprint:

1. Create new Blueprint → Actor
2. Add **Faction Logic** component(s)
3. In BeginPlay:
   - Call `Initialize AI` on each faction
   - Start updating with `Update AI` (pass Delta Time)

### Step 2: Debug Visualization

Enable debug messages (already in C++ code):

- Green = Positive events (alliances, discoveries)
- Yellow = State changes (priority shifts)
- Cyan = Information (interactions, decisions)
- Red = Conflicts

### Step 3: Test Scenarios

**Scenario 1: Peaceful Discovery**
- Spawn faction with high exploration priority
- Trigger "On Territory Discovered" events
- Verify faction claims peacefully

**Scenario 2: Diplomatic Contact**
- Create two peaceful factions
- Trigger "Handle Diplomatic Interaction"
- Check if trade/alliances form

**Scenario 3: Threat Response**
- Create peaceful faction
- Lower Military Power to <30
- Verify faction switches to defensive mode

### Step 4: Monitor State

Print these values to screen:
- `Get Faction State Description` - Overall state
- `Current Strategic Priority` - Current focus
- `Territory Count` - Expansion progress
- `Economic Strength` - Economic health

---

## Common Patterns and Examples

### Pattern 1: Alliance Network Builder

**Goal:** Build alliances with all non-hostile factions

```
Override "Update Strategic Goals":
1. Get all known factions
2. For each faction:
   a. If Relationship Value > 0 AND not allied:
      - Propose alliance
      - Add to Truce List
3. Call Parent Function
```

### Pattern 2: Peaceful Territory Claim

**Goal:** Only expand into unclaimed space

```
Override "On Territory Discovered":
1. Check if territory is unclaimed (no faction owns it)
2. Check if "Should Pursue Peaceful Expansion" returns true
3. If both true:
   - Claim territory
   - Send notification
```

### Pattern 3: Trade Opportunist

**Goal:** Maximize trade with everyone possible

```
Override "Should Initiate Trade":
1. If other faction is NOT at war with us:
   - Return TRUE
2. If we have alliance:
   - Return TRUE
3. Else:
   - Return FALSE
```

### Pattern 4: Dynamic Priority Adjustment

**Goal:** Shift priorities based on game state

```
Override "Evaluate Strategic Priority":
1. If Territory Count < 3:
   - Return "Exploration" (need space)
2. Else if Economic Strength < 50:
   - Return "Trade" (need money)
3. Else if Diplomatic Influence < 40:
   - Return "Diplomacy" (need friends)
4. Else:
   - Return "Research" (improve tech)
```

### Pattern 5: Conditional Early Game Exit

**Goal:** Custom transition out of early game

```
Override "Update Strategic Goals":
1. If Is Early Game AND (custom conditions):
   - Set Is Early Game to FALSE
   - Adjust priorities for mid-game
   - Change behavior mode if needed
2. Call Parent Function
```

---

## Next Steps

1. **Create your first faction** using this guide
2. **Test in-game** with debug visualization
3. **Iterate** on priorities and behaviors
4. **Read Personnel AI Guide** for individual crew AI
5. **Check Integration Guide** for connecting systems

---

## Troubleshooting

**Q: Faction isn't making any decisions**
- A: Ensure `Initialize AI` is called in BeginPlay
- A: Check `Update AI` is being called every tick with Delta Time
- A: Verify `Is Active` is set to true

**Q: Priorities seem wrong**
- A: Check "Early Game Priorities" values
- A: Verify "Is Early Game" is true
- A: Look at "Current Strategic Priority" value

**Q: Diplomacy not working**
- A: Ensure Faction Data Asset is assigned
- A: Check Faction IDs match exactly
- A: Verify relationships are configured in both factions

**Q: Faction too aggressive**
- A: Lower Defense and Expansion priorities
- A: Increase Trade and Diplomacy priorities
- A: Add more factions to Truce List
- A: Set Behavior Mode to "Peaceful"

---

## See Also

- [Personnel AI Setup Guide](PersonnelAISetupGuide.md) - Individual crew AI
- [AI Designer Workflow](AIDesignerWorkflow.md) - Overall AI system workflow
- [AI System Integration](AISystemIntegration.md) - Connecting AI to game systems
- [Faction Templates](FactionAITemplates/) - Pre-made YAML configurations
