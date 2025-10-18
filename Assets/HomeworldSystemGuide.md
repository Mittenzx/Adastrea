# Homeworld-Based Faction Relationship System Guide

## Overview
This guide explains the Homeworld Data Asset system, which defines players' starting locations and initial faction relationships. Each homeworld provides unique diplomatic standings with various factions, creating diverse starting experiences for players.

---

## Table of Contents
1. [What is a Homeworld?](#what-is-a-homeworld)
2. [Creating a Homeworld Data Asset](#creating-a-homeworld-data-asset)
3. [Configuring Faction Relationships](#configuring-faction-relationships)
4. [Blueprint Integration](#blueprint-integration)
5. [Best Practices](#best-practices)
6. [Examples](#examples)

---

## What is a Homeworld?

A **Homeworld** is a Data Asset that defines:
- The player's starting location in the galaxy
- Initial reputation values with all major factions
- Background story and context for the player's origin

### Key Benefits
- **Diverse Starting Experiences**: Different homeworlds offer unique diplomatic situations
- **Strategic Choices**: Players can choose homeworlds that align with their playstyle
- **Narrative Depth**: Each homeworld provides context for why certain factions are friendly or hostile
- **Designer Flexibility**: Easy to create and modify homeworld configurations without code changes

### How It Works
1. Designer creates a Homeworld Data Asset
2. Designer configures starting reputations with various factions
3. Player selects a homeworld at character creation
4. Game initializes player's FactionReputationMap based on homeworld settings
5. Faction relationships evolve from these starting values during gameplay

---

## Creating a Homeworld Data Asset

### Step 1: Create the Data Asset

1. **Navigate to your Content Browser**
   - Recommended location: `Content/Homeworlds/`
   - Create the folder if it doesn't exist

2. **Create the Data Asset**
   - Right-click in the Content Browser
   - Select **Miscellaneous → Data Asset**
   - In the dialog, search for and select **HomeworldDataAsset**
   - Name your asset using the convention: `DA_Homeworld_[Name]`
   - Example: `DA_Homeworld_TerraColony`

3. **Open the Asset**
   - Double-click your newly created asset to open it

### Step 2: Configure Basic Homeworld Information

In the **Homeworld Info** category:

1. **Homeworld Name** (Text)
   - Enter the display name of your homeworld
   - Example: "Terra Colony"
   - This is what players will see in the character creation screen

2. **Description** (Multi-line Text)
   - Write 2-4 sentences describing the homeworld
   - Include:
     - Physical characteristics
     - Political situation
     - Why it matters to the story
   - Example:
     ```
     A thriving human colony on the edge of known space. Terra Colony was founded by 
     refugees from the Core Worlds seeking independence and prosperity. The colony 
     maintains cautious diplomatic relations with most factions, balancing trade 
     opportunities with security concerns.
     ```

3. **Homeworld ID** (Name)
   - Create a unique identifier (no spaces or special characters)
   - Use camel case: `TerraColony` or snake case: `terra_colony`
   - This is used for programmatic access
   - **IMPORTANT:** Must be unique across all homeworlds

---

## Configuring Faction Relationships

### Step 3: Set Up Faction Relations Array

The **Faction Relations** array defines starting reputation with each faction.

#### Adding a Faction Relationship

1. **Click the "+" button** next to "Faction Relations"
   - This creates a new array entry

2. **For each entry, configure:**

   **Faction** (Object Reference)
   - Click the dropdown
   - Select a Faction Data Asset (e.g., `DA_Faction_SolarisUnion`)
   - This is the faction the relationship applies to

   **Starting Reputation** (Integer: -100 to 100)
   - Set the initial reputation value
   - Scale:
     - **-100 to -50**: Hostile (faction may attack on sight)
     - **-49 to -25**: Very Unfriendly (closed borders, no trade)
     - **-24 to -1**: Unfriendly (limited interaction)
     - **0 to 24**: Neutral (standard diplomacy)
     - **25 to 49**: Friendly (trade bonuses)
     - **50 to 74**: Very Friendly (alliance opportunities)
     - **75 to 100**: Allied (maximum cooperation)

3. **Repeat for all relevant factions**
   - Not every faction needs an entry
   - Factions without entries default to neutral (0 reputation)
   - Typically configure 5-10 key factions per homeworld

#### Example Configuration

```
Faction Relations:
  [0]
    Faction: DA_Faction_SolarisUnion
    Starting Reputation: 50 (Very Friendly - close allies)
  
  [1]
    Faction: DA_Faction_IroncladConsortium
    Starting Reputation: 10 (Friendly - established trade relations)
  
  [2]
    Faction: DA_Faction_CrimsonArmada
    Starting Reputation: -60 (Hostile - recent border conflicts)
  
  [3]
    Faction: DA_Faction_VoidCult
    Starting Reputation: -30 (Unfriendly - ideological differences)
  
  [4]
    Faction: DA_Faction_FreeTradersGuild
    Starting Reputation: 40 (Friendly - strong economic ties)
```

### Design Guidelines for Faction Relations

**Balanced Approach:**
- Mix of friendly, neutral, and hostile factions
- Most homeworlds should have 2-3 strong allies
- 1-2 hostile factions for conflict
- Several neutral factions for diplomatic gameplay

**Thematic Consistency:**
- Relationships should make narrative sense
- Consider geography (nearby factions = stronger relations)
- Consider ideology (similar values = better relations)
- Consider history (past conflicts = poor relations)

**Gameplay Considerations:**
- Ensure players have access to at least 2-3 friendly trading partners
- At least one hostile faction provides early game challenge
- Neutral factions offer diplomatic quest opportunities

---

## Blueprint Integration

### Initializing Player Faction Reputation

The homeworld system integrates with Blueprint to initialize player faction relationships at character creation.

#### Blueprint Logic Template: Character Creation

**Event: On Player Character Created**

```blueprint
Event: OnPlayerCharacterCreated
  ↓
Get Selected Homeworld (from character creation UI)
  ↓
Branch: Is Homeworld Valid?
  ├─ False → Use Default Homeworld
  └─ True → Continue
       ↓
     Get Faction Relations (from Homeworld Data Asset)
       ↓
     Create FactionReputationMap (TMap<UFactionDataAsset*, int32>)
       ↓
     ForEachLoop: Faction Relation Entry
       ├─ Loop Body:
       │    ↓
       │  Get Faction (from current entry)
       │    ↓
       │  Get Starting Reputation (from current entry)
       │    ↓
       │  Add to Map (Faction as Key, Reputation as Value)
       │    ↓
       │  Continue Loop
       │
       └─ Completed:
            ↓
          Store FactionReputationMap on Player Character
            ↓
          Broadcast OnReputationInitialized Event
            ↓
          Log Success Message
```

#### Detailed Node Setup

**Step 1: Get Selected Homeworld**
```blueprint
Function: GetSelectedHomeworld
Returns: HomeworldDataAsset (Object Reference)

Implementation:
  - Get reference from character creation widget
  - Or get from saved player data
  - Validate it's not null
```

**Step 2: Extract Faction Relations**
```blueprint
Node: Get Faction Relations
  Input: Homeworld Data Asset reference
  Output: Array<FFactionRelationEntry>
  
This is a Blueprint-callable function on UHomeworldDataAsset
```

**Step 3: Initialize Map**
```blueprint
Node: Make Map
  Key Type: UFactionDataAsset (Object Reference)
  Value Type: Integer
  
This creates an empty TMap to store faction reputations
```

**Step 4: Populate Map**
```blueprint
ForEachLoop:
  Array: FactionRelations (from homeworld)
  Loop Body:
    1. Break FFactionRelationEntry struct
       - Get Faction (UFactionDataAsset*)
       - Get StartingReputation (int32)
    
    2. Add to Map
       - Map: FactionReputationMap
       - Key: Faction
       - Value: StartingReputation
       
    3. Optional: Log for debugging
       - Format String: "Initialized {0} reputation: {1}"
       - Param 0: Faction->FactionName
       - Param 1: StartingReputation
```

**Step 5: Store on Player**
```blueprint
Node: Set FactionReputationMap
  Target: Player Character (or Player State)
  Map: The populated FactionReputationMap
  
Store this map as a member variable on your player character
or player state for persistent access during gameplay
```

### Blueprint Logic Template: Querying Reputation

**Function: Get Reputation With Faction**

```blueprint
Function: GetReputationWithFaction
Parameters:
  - Faction (UFactionDataAsset*)
Returns:
  - Reputation (int32)

Graph:
  Get Player Character
    ↓
  Get FactionReputationMap (from player)
    ↓
  Find in Map
    - Map: FactionReputationMap
    - Key: Faction
    ↓
  Branch: Found?
    ├─ True → Return Value
    └─ False → Return 0 (neutral default)
```

### Blueprint Logic Template: Modifying Reputation

**Function: Change Faction Reputation**

```blueprint
Function: ChangeFactionReputation
Parameters:
  - Faction (UFactionDataAsset*)
  - Amount (int32) - can be positive or negative
Returns:
  - New Reputation (int32)

Graph:
  Get Current Reputation (use GetReputationWithFaction)
    ↓
  Add Amount to Current Reputation
    ↓
  Clamp Result (-100 to 100)
    ↓
  Set/Update in FactionReputationMap
    ↓
  Broadcast OnReputationChanged Event
    - Faction
    - Old Reputation
    - New Reputation
    ↓
  Return New Reputation
```

### Variable Setup in Player Character/State

**Required Variables:**
```
Player Character/Player State Blueprint:

Variables:
  - FactionReputationMap
    Type: Map (Key: Object Reference (FactionDataAsset), Value: Integer)
    Default: Empty Map
    Category: "Faction System"
    Tooltip: "Stores current reputation values with all factions"
  
  - CurrentHomeworld
    Type: Object Reference (HomeworldDataAsset)
    Category: "Faction System"
    Tooltip: "The homeworld this character originated from"
```

---

## Best Practices

### Designing Homeworlds

1. **Create Distinct Identities**
   - Each homeworld should offer a unique diplomatic starting point
   - Vary the mix of allies and enemies
   - Consider different playstyles (trade, combat, diplomacy)

2. **Maintain Balance**
   - Don't create homeworlds that are strictly better than others
   - Advantages in one area should have trade-offs
   - Example: Strong military allies might mean fewer trade partners

3. **Tell a Story**
   - Use descriptions to explain why relationships exist
   - Create logical connections between homeworld and faction relations
   - Give players narrative context for their starting situation

4. **Plan for Progression**
   - Starting relationships should leave room for change
   - Avoid extremes (-100 or +100) unless narratively justified
   - Consider how players might shift alliances during gameplay

### Technical Considerations

1. **Asset Naming**
   - Use consistent naming: `DA_Homeworld_[Name]`
   - Keep names descriptive and unique
   - Example: `DA_Homeworld_CoreworldRefugee`, `DA_Homeworld_FrontierSettler`

2. **Reference Management**
   - Always verify faction references are valid
   - Don't reference the same faction twice in one homeworld
   - Update homeworlds if factions are renamed or removed

3. **Testing**
   - Test each homeworld in-game
   - Verify all reputation values are applied correctly
   - Check that faction interactions work as expected
   - Validate the character creation UI displays homeworld information

4. **Performance**
   - Faction Relations arrays should typically have 5-15 entries
   - Very large arrays (>20 factions) may impact load times
   - Only include factions that matter for gameplay

### Documentation

1. **Comment Your Work**
   - Use the Description field meaningfully
   - Document design intent in commit messages
   - Keep a design document of homeworld purposes

2. **Designer Communication**
   - Inform team when adding new homeworlds
   - Discuss faction relationship changes in design meetings
   - Coordinate with quest designers for narrative consistency

---

## Examples

### Example 1: Core World Loyalist

```
Homeworld Name: "New Terra Prime"
Description: "A prosperous core world colony loyal to the Solaris Union. 
Citizens enjoy high technology and strong military protection, but face 
scrutiny from independence-minded frontier factions."

Homeworld ID: NewTerraPrime

Faction Relations:
  - Solaris Union: +75 (Strong ally)
  - Ironclad Consortium: +40 (Trade partner)
  - Free Traders Guild: +20 (Neutral-friendly)
  - Nova Vanguard: +60 (Military cooperation)
  - Frontier Alliance: -40 (Ideological conflict)
  - Crimson Armada: -70 (Active enemy)
  - Void Cult: -50 (Religious persecution)
```

**Design Intent:** Strong military start, good trade access, but enemies on the frontier. Suited for combat-focused players.

### Example 2: Independent Trader

```
Homeworld Name: "Port Haven"
Description: "A neutral trading station that declared independence decades ago. 
Port Haven maintains careful balance between all major powers, prioritizing 
profit over politics. An ideal start for aspiring merchants."

Homeworld ID: PortHaven

Faction Relations:
  - Free Traders Guild: +65 (Primary ally)
  - Ironclad Consortium: +50 (Strong business ties)
  - Solaris Union: +15 (Trade license holder)
  - Nova Vanguard: +5 (Neutral with slight favor)
  - Frontier Alliance: +25 (Regular customers)
  - Crimson Armada: -20 (Occasional raids)
  - Void Cult: 0 (No interaction)
```

**Design Intent:** Balanced diplomacy, strong trade focus, minimal enemies. Suited for economic/trading gameplay.

### Example 3: Frontier Survivor

```
Homeworld Name: "Last Light Station"
Description: "A hardscrabble frontier outpost on the edge of civilized space. 
Residents are tough, independent, and deeply suspicious of core world authority. 
Pirates are a constant threat, but the station's resilience is legendary."

Homeworld ID: LastLightStation

Faction Relations:
  - Frontier Alliance: +80 (Fellow independents)
  - Free Traders Guild: +30 (Essential suppliers)
  - Solaris Union: -35 (Core world oppression)
  - Nova Vanguard: -25 (Military overreach)
  - Crimson Armada: -80 (Constant attacks)
  - Void Cult: -60 (Recent raid survivors)
  - Ironclad Consortium: +10 (Occasional trade)
```

**Design Intent:** Difficult start with many enemies, strong bonds with other frontier settlers. Suited for challenge-seeking players.

### Example 4: Scientific Outpost

```
Homeworld Name: "Archimedes Research Station"
Description: "A cutting-edge research facility operated by the Academic Collective. 
The station maintains strict neutrality to facilitate scientific cooperation 
across factional boundaries, though this stance has earned both respect and scorn."

Homeworld ID: ArchimedesStation

Faction Relations:
  - Academic Collective: +85 (Parent organization)
  - Solaris Union: +45 (Research grants)
  - Ironclad Consortium: +35 (Equipment suppliers)
  - Nova Vanguard: +15 (Defensive contract)
  - Free Traders Guild: +25 (Logistics)
  - Frontier Alliance: +20 (Data sharing)
  - Crimson Armada: -15 (Target of theft)
  - Void Cult: -40 (Ideological opposition to science)
```

**Design Intent:** Diplomatic start with broad but shallow relationships. Suited for exploration and research gameplay.

---

## Integration with Other Systems

### Character Creation UI

The homeworld selection should be integrated into your character creation screen:

1. **Display Options**
   - Show homeworld name and description
   - Optionally display key faction relationships
   - Use icons or colors to indicate ally/enemy factions

2. **Selection Flow**
   - Allow player to browse available homeworlds
   - Preview faction relationships before confirming
   - Store selected homeworld for initialization

3. **Visual Feedback**
   - Highlight friendly factions in green
   - Highlight hostile factions in red
   - Show neutral factions in gray

### Save System

When saving player progress:
- Store current FactionReputationMap
- Store reference to original homeworld (for reference/achievements)
- Save reputation changes separately for analytics

### Quest System

Homeworld selection can influence:
- Available starting quests
- Faction-specific mission availability
- Dialogue options with NPCs
- Trade prices and access

---

## Troubleshooting

### Common Issues

**Issue:** Faction relationships not applying
- **Solution:** Verify homeworld is properly selected before initialization
- **Solution:** Check FactionReputationMap is being stored on player character
- **Solution:** Ensure faction references in homeworld asset are valid

**Issue:** Some factions show 0 reputation unexpectedly
- **Solution:** Not all factions need entries; 0 is the default for undefined relationships
- **Solution:** If a faction should have non-zero reputation, add it to Faction Relations array

**Issue:** Reputation values seem incorrect
- **Solution:** Check for typos in StartingReputation values
- **Solution:** Verify clamp range (-100 to 100) isn't being violated
- **Solution:** Ensure no duplicate faction entries in the same homeworld

**Issue:** Changes to homeworld not reflecting in game
- **Solution:** Save the homeworld asset after editing
- **Solution:** Restart the game/editor to reload assets
- **Solution:** Clear any cached player data from previous tests

---

## Advanced Topics

### Dynamic Homeworld Generation

For advanced implementations, homeworlds can be procedurally generated:
- Randomly assign reputation values within ranges
- Ensure balanced distributions of allies/enemies
- Generate descriptions from templates

### Homeworld Events

Create special events based on homeworld:
- Periodic messages from homeworld NPCs
- Faction relationship changes affecting homeworld
- Special quests that return player to homeworld

### Multiple Homeworld Support

If supporting multiple homeworlds per player:
- Track which homeworld provided initial relationships
- Allow reputation to diverge from homeworld defaults
- Create homeworld reputation system (player's standing with homeworld itself)

---

## Summary Checklist

When creating a new homeworld, ensure:
- [ ] Homeworld Name is descriptive and unique
- [ ] Description provides narrative context (2-4 sentences)
- [ ] Homeworld ID is unique and follows naming convention
- [ ] Faction Relations array has 5-15 entries
- [ ] Reputation values are balanced (mix of allies, neutrals, enemies)
- [ ] All faction references are valid and point to existing assets
- [ ] Relationships make narrative and logical sense
- [ ] Asset is saved with proper naming: `DA_Homeworld_[Name]`
- [ ] Homeworld is tested in-game
- [ ] Team is informed of new homeworld availability

---

## Additional Resources

- **Faction System Guide**: [Assets/FactionSetupGuide.md](Assets/FactionSetupGuide.md)
- **Blueprint Workflows**: [Assets/BlueprintWorkflowTemplates.md](Assets/BlueprintWorkflowTemplates.md)
- **Designer Onboarding**: [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md)

For questions or support, contact the game design team.
