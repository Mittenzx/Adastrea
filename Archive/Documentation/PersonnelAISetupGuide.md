# Personnel AI Setup Guide
## Complete Blueprint Workflow for Data-Driven Personnel AI

This guide provides step-by-step instructions for setting up individual personnel/crew AI in Unreal Engine using Blueprints. No C++ knowledge required!

---

## Table of Contents
1. [Overview](#overview)
2. [Creating Personnel Data Assets](#creating-personnel-data-assets)
3. [Creating Personnel Logic Blueprints](#creating-personnel-logic-blueprints)
4. [Configuring Personality-Driven Behaviors](#configuring-personality-driven-behaviors)
5. [Implementing Custom Behaviors](#implementing-custom-behaviors)
6. [Social Interactions and Relationships](#social-interactions-and-relationships)
7. [Testing Your Personnel AI](#testing-your-personnel-ai)
8. [Common Patterns and Examples](#common-patterns-and-examples)

---

## Overview

### What is Personnel AI?

Personnel AI handles **micro-level individual behaviors** for crew members:
- **Daily task prioritization** (duty, rest, social, training)
- **Personality-driven decision making**
- **Social interactions** with other crew
- **Skill development** and career progression
- **Morale and wellbeing** management

### Key Components

1. **UPersonnelDataAsset** (C++ Data Asset) - Stores personnel data (skills, traits, relationships, stats)
2. **UPersonnelLogic** (C++ Logic Class) - Provides AI behavior with Blueprint extension points
3. **Blueprint Logic** (Your Custom Logic) - Override behaviors for unique personalities

### The 8 Personality Dispositions

Each personnel has a **disposition** that affects how they make decisions:

| Disposition | Behavior Pattern | Best For |
|-------------|------------------|----------|
| **Cautious** | Risk-averse, methodical, chooses safe options | Security, Safety Officers |
| **Bold** | Risk-taking, decisive, chooses daring options | Pilots, Combat Roles |
| **Analytical** | Logical, calculated, balanced decisions | Engineers, Scientists |
| **Empathetic** | People-focused, supportive, helps others | Medical, Counselors |
| **Pragmatic** | Results-oriented, efficient, practical | Managers, Quartermasters |
| **Curious** | Inquisitive, exploratory, seeks knowledge | Scientists, Explorers |
| **Disciplined** | By-the-book, orderly, follows protocol | Security, Military |
| **Creative** | Innovative, unconventional, outside-the-box | Researchers, Artists |

---

## Creating Personnel Data Assets

### Step 1: Create a Personnel Data Asset

1. In Content Browser, **Right-Click** → **Blueprint Class**
2. Search for **"PersonnelDataAsset"**
3. Select **PersonnelDataAsset** as parent class
4. Name it (e.g., `DA_Explorer_Rivera`)
5. **Double-click** to open the asset

### Step 2: Configure Basic Identity

**Personnel Identity:**
- `Personnel Name` - Full name (e.g., "Dr. Alex Rivera")
- `Personnel ID` - Unique identifier (e.g., "Explorer_001")
- `Biography` - Background story (use multi-line text)
- `Age` - Character age (18-200)
- `Gender`, `Species`, `Nationality` - Character details
- `Tags` - Search/filter tags (e.g., "Scientist", "Explorer")

### Step 3: Assign Role and Assignment

**Role Configuration:**
- `Primary Role` - Select from 31 roles (Captain, Pilot, Engineer, etc.)
- `Current Assignment` - Ship/station name
- `Department` - Which department (Science, Engineering, Medical, etc.)
- `Manager ID` - Their supervisor's Personnel ID
- `Direct Reports` - Array of Personnel IDs they supervise

**Available Roles:**
- Command: Captain, XO, Station Manager
- Flight: Pilot, Navigator
- Technical: Engineer, AI Operator, Researcher
- Combat: Gunnery Chief, Gunner, Security Officer
- Medical: Medical Officer, Science Officer
- Support: Quartermaster, Chef, Bartender, Janitor
- Specialized: Diplomat, Trader, Salvage, Mining, Smuggler
- General: Crew Member, Station Staff, Entertainment, Civilian, Ambassador

### Step 4: Configure Skills

Add skills that define the character's capabilities:

1. Find **"Skills"** array
2. Click **+** to add skills
3. For each skill:
   - `Skill Name` - What they're good at
   - `Skill Level` (1-10) - How good they are
   - `Experience Points` - Progress toward next level

**Example Science Officer Skills:**
```
Skill 1:
  Skill Name: Xenobiology
  Skill Level: 9
  Experience Points: 4500

Skill 2:
  Skill Name: Sensor Operations
  Skill Level: 8
  Experience Points: 3200

Skill 3:
  Skill Name: Data Analysis
  Skill Level: 7
  Experience Points: 2800
```

Also set:
- `Overall Skill Level` (1-10) - General competence
- `Total Experience` - Career experience points
- `Specialties` - Array of specialty areas

### Step 5: Set Status and Condition

Configure current state:

**Status Values (0-100):**
- `Health` - Physical wellbeing (70+ is good)
- `Morale` - Happiness/satisfaction (60+ is good)
- `Fatigue` - Tiredness (30- is rested, 70+ needs rest)
- `Loyalty` - Dedication to crew (70+ is loyal)
- `Reputation` - Respect from others (-100 to 100, 0 is neutral)

**Example Healthy Explorer:**
```
Health: 85.0 (good health)
Morale: 75.0 (happy, loves the job)
Fatigue: 35.0 (a bit tired but enthusiastic)
Loyalty: 80.0 (very loyal)
Reputation: 45 (well-regarded)
```

### Step 6: Add Personality Traits

Traits provide unique characteristics:

1. Find **"Traits"** array
2. Click **+** to add traits
3. Configure each:
   - `Trait ID` - Unique identifier
   - `Trait Name` - Display name
   - `Trait Description` - What it does
   - `Modifier Value` - Numerical effect

**Example Curious Explorer Traits:**
```
Trait 1:
  Trait ID: Insatiably_Curious
  Trait Name: Insatiably Curious
  Description: Bonus to exploration and research
  Modifier Value: 1.3 (30% bonus)

Trait 2:
  Trait ID: Quick_Learner
  Trait Name: Quick Learner
  Description: Faster skill progression
  Modifier Value: 1.25 (25% faster learning)
```

### Step 7: Define Relationships

Add relationships with other crew:

1. Find **"Relationships"** array
2. Click **+** to add relationships
3. For each:
   - `Target Personnel ID` - Other person's ID
   - `Relationship Type` - Friend, Rival, Mentor, Professional, Family
   - `Relationship Strength` (-100 to 100)
   - `Notes` - Additional context

**Example Relationships:**
```
Relationship 1:
  Target Personnel ID: Engineer_003
  Relationship Type: Friend
  Relationship Strength: 60
  Notes: Collaborate on equipment modifications

Relationship 2:
  Target Personnel ID: Captain_001
  Relationship Type: Professional
  Relationship Strength: 45
  Notes: Needs reminders about safety protocols
```

### Step 8: Use YAML Templates

Copy values from `Assets/PersonnelAITemplates/`:
- `CuriousExplorer.yaml` - Inquisitive scientist
- `DisciplinedSecurity.yaml` - By-the-book security officer
- `EmpatheticMedic.yaml` - Caring medical officer

---

## Creating Personnel Logic Blueprints

### Step 1: Create the Blueprint

1. **Right-Click** in Content Browser → **Blueprint Class**
2. Search for **"PersonnelLogic"**
3. Select **PersonnelLogic** as parent
4. Name it (e.g., `BP_ExplorerAI`)
5. **Double-click** to open

### Step 2: Assign Personnel Data

1. Select **root component** (or Class Defaults)
2. Find **"Personnel Data"** property
3. Select your Personnel Data Asset (e.g., `DA_Explorer_Rivera`)

### Step 3: Configure AI Settings

Set core AI properties:

**Personality:**
- `Disposition` - Choose from 8 types (Curious, Disciplined, Empathetic, etc.)
- `Current Behavior Mode` - Usually "Peaceful" or "Exploration"
- `Is Active` - Set to **true**
- `Update Interval` - 1.0 seconds (personnel update frequently)

**Task State:**
- `Current Task` - Initial task (usually "Personal" or "DutyStation")
- `On Duty` - Whether currently on shift
- `Hours Until Next Shift` - Time until duty starts/ends

**Example Curious Explorer Settings:**
```
Disposition: Curious
Current Behavior Mode: Exploration
Is Active: true
Update Interval: 1.0
Current Task: Exploration
On Duty: true
Hours Until Next Shift: 6.0
```

---

## Configuring Personality-Driven Behaviors

### Understanding Dispositions

Each disposition makes **different** decisions in the same situation:

**Example: "What should I do during off-duty time?"**

| Disposition | Decision |
|-------------|----------|
| Curious | Explore new areas, pursue training |
| Empathetic | Check on crew, social interactions |
| Disciplined | Maintenance tasks, equipment checks |
| Pragmatic | Rest efficiently, prepare for next shift |
| Bold | Recreation, high-energy activities |
| Cautious | Quiet personal time, avoid risks |
| Analytical | Study, analyze data |
| Creative | Personal projects, hobbies |

### Task Priority by Disposition

Dispositions affect which tasks are prioritized:

**Curious Disposition:**
- **Highest:** Exploration (if morale > 60)
- **High:** Training
- **Medium:** Social (share discoveries)
- **Low:** Maintenance

**Disciplined Disposition:**
- **Highest:** Duty Station (when on duty)
- **High:** Maintenance
- **Medium:** Training (combat/security)
- **Low:** Social

**Empathetic Disposition:**
- **Highest:** Social (check on others)
- **High:** Duty Station (caring for patients)
- **Medium:** Personal (but helps others first)
- **Low:** Exploration

### Decision Making Patterns

The AI makes personality-driven decisions automatically:

**Safety vs. Discovery Choice:**
- **Cautious** → Choose safe option
- **Bold** → Choose risky option
- **Curious** → Choose option with new information
- **Analytical** → Balanced, calculate risk/reward

**People vs. Task Choice:**
- **Empathetic** → Prioritize people
- **Pragmatic** → Prioritize task completion
- **Disciplined** → Follow protocol

**Quick vs. Thorough:**
- **Pragmatic** → Quick and efficient
- **Analytical** → Thorough analysis
- **Bold** → Quick decision, act fast

---

## Implementing Custom Behaviors

### Override BlueprintNativeEvents

Right-click in Event Graph → **Add Event** → **Event [Function Name]**

### Key Events to Override

#### 1. Event Evaluate Task Priority

**When:** Called to decide what task to do next  
**Use:** Customize task selection logic

```
Example: Force Exploration When Discovery Available
1. Add Event: "Event Evaluate Task Priority"
2. Check if "Discovery Opportunity" variable is true
3. If true AND Disposition is Curious:
   - Return "Exploration"
4. Else: Call Parent Function
```

#### 2. Event Handle Social Interaction

**When:** Interacting with another crew member  
**Use:** Custom social behavior

```
Example: Empathetic Check-In
1. Add Event: "Event Handle Social Interaction"
2. If Disposition is Empathetic:
   a. Get other person's Morale from their data
   b. If their Morale < 50:
      - Offer support
      - Boost their morale slightly
3. Call Parent Function
```

#### 3. Event Make Personality Driven Decision

**When:** Need to choose between options  
**Use:** Override default decision logic

```
Example: Bold Always Chooses Action
1. Add Event: "Event Make Personality Driven Decision"
2. If Disposition is Bold:
   - Return last option index (most action-oriented)
3. Else: Call Parent Function (use default logic)
```

#### 4. Event On Task Completed

**When:** Finishes a task  
**Use:** Handle completion, grant rewards

```
Example: Gain Experience on Training
1. Add Event: "Event On Task Completed"
2. If Completed Task is "Training":
   a. If Success is true:
      - Add experience points
      - Increase relevant skill
      - Boost morale
3. Call Parent Function
```

#### 5. Event Update Daily Routine

**When:** Called regularly to manage schedule  
**Use:** Custom schedule logic

```
Example: Night Owl Schedule
1. Add Event: "Event Update Daily Routine"
2. Check game time of day
3. If this person has "Night Owl" trait:
   - Shift duty hours to night
   - Better performance at night
4. Else: Call Parent Function
```

#### 6. Event Handle Stress Change

**When:** Stress level changes  
**Use:** Custom stress response

```
Example: Bold Handles Stress Better
1. Add Event: "Event Handle Stress Change"
2. If Disposition is Bold:
   - Multiply stress by 0.8 (20% reduction)
3. If Disposition is Cautious:
   - Multiply stress by 1.2 (20% increase)
4. Update morale based on modified stress
```

### Using Query Functions

Helper functions for decision making:

**Social Queries:**
- `Is Friends With` - Check friendship
- `Has Conflict With` - Check for rivalry
- `Get Relationship Strength` - Get relationship value

**Status Queries:**
- `Needs Rest` - Fatigue too high?
- `Is Fit For Duty` - Healthy enough for work?
- `Is Loyal` - Will they follow orders?

**Task Queries:**
- `Is Suited For Task` - Disposition matches task?
- `Get Personnel State Description` - Debug info

**Example Usage:**
```
If "Needs Rest":
  - Set Current Task to "Personal"
  - Set Is Resting to true
  - Reduce Fatigue over time
Else if "On Duty":
  - Set Current Task to "Duty Station"
```

---

## Social Interactions and Relationships

### Building Friendships

Personnel can become friends through interactions:

1. Frequent positive social interactions
2. Similar interests/specialties
3. Compatible dispositions
4. Working together successfully

**Blueprint Example:**
```
When Social Interaction Succeeds:
1. Check current relationship strength
2. If positive (>0):
   a. Increment relationship by 5
   b. If relationship reaches 50:
      - Call "Make Friend" function
      - Update Friends List
```

### Managing Conflicts

Conflicts can arise from:

1. Failed collaborative tasks
2. Incompatible dispositions
3. Competing for resources/recognition
4. Stress and low morale

**Blueprint Example:**
```
When Task Fails with Partner:
1. Check both personnel morale
2. If either has low morale:
   a. Decrement relationship by 10
   b. If relationship drops below -50:
      - Call "Add Conflict" function
      - Update Conflict List
```

### Disposition Compatibility

Some dispositions work well together:

**Good Combinations:**
- Curious + Analytical (research partners)
- Empathetic + Cautious (care and safety)
- Bold + Disciplined (action with backup)
- Pragmatic + Creative (execution + ideas)

**Challenging Combinations:**
- Bold + Cautious (risk disagreement)
- Disciplined + Creative (procedure vs. innovation)
- Pragmatic + Empathetic (efficiency vs. care)

### Social Network Effects

Friends and conflicts affect behavior:

**With Friends:**
- Higher morale when together
- Better task collaboration
- Share discoveries (Curious)
- Support during stress (Empathetic)

**With Conflicts:**
- Lower morale when together
- Reduced collaboration effectiveness
- Avoid social interactions
- May request transfers

---

## Testing Your Personnel AI

### Step 1: Add to Game World

Create a **Personnel Manager** Blueprint or add to Character:

1. Create component or reference to Personnel Logic
2. In BeginPlay:
   - Call `Initialize AI`
   - Start update loop with `Update AI` (Delta Time)

### Step 2: Debug Visualization

Enable debug messages to see AI thinking:

```
Print to Screen:
- Personnel Name
- Current Task
- Disposition
- Morale, Health, Fatigue
- Current Behavior Mode
```

Use `Get Personnel State Description` for quick summary.

### Step 3: Test Scenarios

**Scenario 1: Daily Routine**
- Watch personnel switch between tasks
- Verify duty shifts work correctly
- Check rest behavior when fatigued

**Scenario 2: Social Interaction**
- Trigger interactions between personnel
- Watch friendship/conflict development
- Test disposition-based responses

**Scenario 3: Decision Making**
- Present choices to personnel
- Verify disposition affects choice
- Check consistency with personality

**Scenario 4: Stress Response**
- Apply stress to personnel
- Watch morale changes
- Test different dispositions' resilience

### Step 4: Monitor Long-Term

Track these over time:
- Skill progression
- Relationship changes
- Morale stability
- Task performance

---

## Common Patterns and Examples

### Pattern 1: Skill-Based Task Assignment

**Goal:** Personnel naturally work on tasks they're good at

```
Override "Evaluate Task Priority":
1. Get all skills from Personnel Data
2. Find highest skill level
3. Map skill to task type:
   - "Combat Training" → Maintenance/Training
   - "Medicine" → Duty Station (if Medical Officer)
   - "Research" → Exploration/Training
4. Return appropriate task
```

### Pattern 2: Mentor Relationship Growth

**Goal:** Experienced personnel mentor juniors

```
Override "Handle Social Interaction":
1. Check if this person has high skill in something
2. Check if other person is junior in that skill
3. If yes and disposition is Empathetic or Disciplined:
   a. Establish Mentor relationship
   b. Boost other person's skill slowly
   c. Increase relationship strength
```

### Pattern 3: Burnout Prevention

**Goal:** Prevent overwork by monitoring fatigue

```
Override "Update Daily Routine":
1. Check Fatigue level
2. If Fatigue > 80 for 3+ shifts:
   a. Force rest period
   b. Reduce morale if continue working
   c. Notify player/manager
3. If disposition is Pragmatic:
   - Better at self-management (higher threshold)
```

### Pattern 4: Curiosity-Driven Exploration

**Goal:** Curious personnel volunteer for discoveries

```
Override "Evaluate Task Priority":
1. If Disposition is Curious:
   a. Check if unexplored areas exist
   b. Check if morale > 50 (motivated)
   c. If both true:
      - Return "Exploration" (even if tired)
2. Else: Call Parent Function
```

### Pattern 5: Team Morale Boost

**Goal:** Empathetic personnel improve team morale

```
Override "On Tick AI":
1. If Disposition is Empathetic AND not on duty:
   a. Get nearby personnel
   b. For each with low morale:
      - Initiate social interaction
      - Boost their morale slightly
      - Gain morale for helping
2. Call Parent Function
```

---

## Next Steps

1. **Create your first personnel** using this guide
2. **Test interactions** between different dispositions
3. **Build a diverse crew** with varied personalities
4. **Read AI System Integration** for connecting to gameplay
5. **Experiment** with custom behaviors

---

## Troubleshooting

**Q: Personnel isn't doing anything**
- A: Check `Initialize AI` was called
- A: Verify `Update AI` is called with Delta Time
- A: Ensure `Is Active` is true

**Q: Tasks seem random**
- A: Check Disposition is set correctly
- A: Verify Personnel Data is assigned
- A: Look at morale/fatigue affecting priorities

**Q: Social interactions not working**
- A: Ensure Personnel IDs match exactly
- A: Check Relationships are configured
- A: Verify both personnel have logic components

**Q: Personality doesn't match behavior**
- A: Check Disposition value
- A: Verify you're using correct Disposition enum
- A: Test with `Make Personality Driven Decision`

**Q: Skills not progressing**
- A: Check `On Task Completed` is being called
- A: Verify Success parameter is true
- A: Ensure Task type matches skill category

---

## See Also

- [Faction AI Setup Guide](FactionAISetupGuide.md) - Faction-level AI
- [AI Designer Workflow](AIDesignerWorkflow.md) - Overall workflow
- [AI System Integration](AISystemIntegration.md) - Connecting to game
- [Personnel Templates](PersonnelAITemplates/) - Pre-made YAML configurations
