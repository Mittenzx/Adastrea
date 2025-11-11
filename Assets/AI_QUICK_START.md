# AI Framework Quick Start Guide
## Get Started in 30 Minutes!

This is the fastest path to creating your first working AI. Follow these steps and you'll have a functional faction or personnel AI running in your game within 30 minutes.

**AI System Overview:**
- **Faction AI** - Political and military strategy (this guide)
- **Personnel AI** - Individual character behavior (this guide)
- **Way System AI** - Economic activities and trade operations (see [Way System Guide](WaySystemGuide.md))

---

## Choose Your Path

**Option A: Create a Faction AI (Recommended First)**
- Takes: 20-30 minutes
- Good for: Understanding macro-level AI
- Simpler: Fewer properties to configure

**Option B: Create a Personnel AI**
- Takes: 25-35 minutes
- Good for: Understanding micro-level AI
- More detail: Individual character personalities

---

## Path A: Create Your First Faction AI

### Step 1: Create Faction Data Asset (5 minutes)

1. **Content Browser** → Right-Click → **Blueprint Class**
2. Search: **FactionDataAsset**
3. Name: `DA_MyFirstFaction`
4. Open it and set:

```
Faction Info:
  Faction Name: "My First Faction"
  Description: "A peaceful faction for testing"
  Faction ID: "MyFirstFaction"
  Primary Color: Blue (0.2, 0.5, 0.9, 1.0)
  Secondary Color: Light Blue (0.7, 0.8, 1.0, 1.0)

Faction Attributes:
  Technology Level: 5
  Military Strength: 3
  Economic Power: 6
  Initial Reputation: 25
  Hostile By Default: false (unchecked)
```

5. **Save**

### Step 2: Create Faction Logic Blueprint (5 minutes)

1. **Content Browser** → Right-Click → **Blueprint Class**
2. Search: **FactionLogic**
3. Name: `BP_MyFirstFactionLogic`
4. Open it
5. **Class Defaults** (or root component):

```
Faction Data: Select "DA_MyFirstFaction"

Early Game Priorities:
  Exploration Priority: 8
  Trade Priority: 6
  Diplomacy Priority: 7
  Research Priority: 5
  Expansion Priority: 4
  Defense Priority: 3

AI Settings:
  Current Behavior Mode: Peaceful
  Is Early Game: true (checked)
  Update Interval: 5.0
  Is Active: true (checked)
```

6. **Compile** and **Save**

### Step 3: Create Test Level (5 minutes)

1. Create new level: **File** → **New Level** → **Empty Level**
2. Save as: `TestAI`
3. Add a **Directional Light** (so you can see)
4. Add a **Player Start**

### Step 4: Create AI Manager Actor (10 minutes)

1. **Content Browser** → **Blueprint Class** → **Actor**
2. Name: `BP_AITestManager`
3. Open it
4. **Add Component** → Search for your faction logic
   - Actually, we need to create component differently
5. Instead, in **Event Graph**:

**Event BeginPlay:**
```
1. Spawn Actor of Class: BP_MyFirstFactionLogic
2. Set variable: MyFactionAI = return value
3. Call function on MyFactionAI: Initialize AI
```

**Event Tick:**
```
1. Get MyFactionAI
2. Call: Update AI
3. Connect Delta Seconds to DeltaTime input
```

6. **Compile** and **Save**

### Step 5: Test It! (5 minutes)

1. Drag `BP_AITestManager` into your test level
2. **Play** the game
3. Watch the **Output Log** for messages:
   - "AI Initialized: BP_MyFirstFactionLogic"
   - Strategic priority messages
   - Decision messages

**Success!** Your first faction AI is running!

---

## Path B: Create Your First Personnel AI

### Step 1: Create Personnel Data Asset (5 minutes)

1. **Content Browser** → Right-Click → **Blueprint Class**
2. Search: **PersonnelDataAsset**
3. Name: `DA_TestCrew`
4. Open it and set:

```
Personnel Identity:
  Personnel Name: "Alex Smith"
  Personnel ID: "TestCrew001"
  Biography: "Test crew member"
  Age: 30
  Gender: "Human"
  Species: "Human"

Role and Assignment:
  Primary Role: Crew Member
  Current Assignment: "Test Ship"
  Department: "General"

Skills:
  Overall Skill Level: 5
  Total Experience: 1000

Status and Condition:
  Health: 85.0
  Morale: 70.0
  Fatigue: 30.0
  Loyalty: 75.0
  Reputation: 0
```

5. **Save**

### Step 2: Create Personnel Logic Blueprint (5 minutes)

1. **Content Browser** → Right-Click → **Blueprint Class**
2. Search: **PersonnelLogic**
3. Name: `BP_TestCrewAI`
4. Open it
5. **Class Defaults**:

```
Personnel Data: Select "DA_TestCrew"

AI Personality:
  Disposition: Pragmatic
  Current Behavior Mode: Peaceful
  Is Active: true (checked)
  Update Interval: 1.0

Task State:
  Current Task: Personal
  On Duty: false
  Hours Until Next Shift: 8.0
```

6. **Compile** and **Save**

### Step 3: Create Test Setup (Same as Faction)

Follow Faction **Step 3** and **Step 4**, but:
- Spawn: `BP_TestCrewAI` instead
- Call: Initialize AI and Update AI same way

### Step 4: Test It!

1. Drag AI Manager into level
2. **Play**
3. Watch for personnel messages:
   - Task changes
   - Schedule updates
   - Decisions

**Success!** Your first personnel AI is running!

---

## Next Steps After Success

### If You Created a Faction:

1. ✅ **Add a relationship** to another faction
   - Open `DA_MyFirstFaction`
   - Add to "Faction Relationships" array
   - Set another faction ID and relationship value

2. ✅ **Add a trait**
   - Open `DA_MyFirstFaction`
   - Add to "Traits" array
   - Give it a name and modifier value

3. ✅ **Override a behavior**
   - Open `BP_MyFirstFactionLogic`
   - Event Graph → Right-Click → Add Event
   - Choose "Event On Territory Discovered"
   - Add a Print String node with "Territory found!"

4. ✅ **Create a second faction**
   - Copy `DA_MyFirstFaction`
   - Change values to create different personality
   - Make them friendly or hostile to each other

### If You Created Personnel:

1. ✅ **Add a skill**
   - Open `DA_TestCrew`
   - Add to "Skills" array
   - Name it, set level 1-10

2. ✅ **Try different disposition**
   - Open `BP_TestCrewAI`
   - Change Disposition to "Curious"
   - Notice behavior changes

3. ✅ **Add a relationship**
   - Open `DA_TestCrew`
   - Add to "Relationships" array
   - Create friend or rival

4. ✅ **Create a second crew member**
   - Copy `DA_TestCrew`
   - Different role, disposition, skills
   - Test interactions

---

## Common First-Time Issues

### "AI Initialized" message doesn't appear
- ✅ Check: Is AI Manager in the level?
- ✅ Check: Did BeginPlay call Initialize AI?
- ✅ Check: Is the logic Blueprint set to spawn?

### AI not making decisions
- ✅ Check: Is "Is Active" checked?
- ✅ Check: Is Update AI being called in Tick?
- ✅ Check: Is Data Asset assigned?

### Can't find FactionLogic/PersonnelLogic in Blueprint
- ✅ Make sure C++ classes are compiled
- ✅ Restart Unreal Editor
- ✅ Check Source/Adastrea/AI folder exists

### No debug messages
- ✅ Check Output Log (Window → Developer Tools → Output Log)
- ✅ Messages are green/yellow/cyan/red colored
- ✅ Make sure GEngine messages are enabled

---

## Your 30-Minute Checklist

### Faction AI (20-30 min):
- ☐ Create Data Asset (5 min)
- ☐ Create Logic Blueprint (5 min)
- ☐ Create Test Level (5 min)
- ☐ Create AI Manager (10 min)
- ☐ Test and verify (5 min)

### Personnel AI (25-35 min):
- ☐ Create Data Asset (5 min)
- ☐ Create Logic Blueprint (5 min)
- ☐ Create Test Level (5 min)
- ☐ Create AI Manager (10 min)
- ☐ Test and verify (5 min)

---

## What You Just Learned

✅ How to create Data Assets
✅ How to create Logic Blueprints
✅ How to assign data to logic
✅ How to initialize and update AI
✅ How to test AI in a level

---

## Next Learning Steps

**After your first AI works:**

1. Read full guides:
   - [FactionAISetupGuide.md](FactionAISetupGuide.md) for factions
   - [PersonnelAISetupGuide.md](PersonnelAISetupGuide.md) for personnel

2. Study YAML templates:
   - [FactionAITemplates/](FactionAITemplates/) for faction examples
   - [PersonnelAITemplates/](PersonnelAITemplates/) for personnel examples

3. Learn customization:
   - [AIDesignerWorkflow.md](AIDesignerWorkflow.md) for overall workflow
   - [AISystemIntegration.md](AISystemIntegration.md) for game integration

4. Build your game:
   - Create diverse factions (5-10)
   - Create memorable crew (10-20)
   - Integrate with ships and stations
   - Polish and balance

---

## Celebration Time! 🎉

**You just created your first AI!**

You now have:
- ✅ A working AI that makes decisions
- ✅ A test environment to experiment
- ✅ Understanding of the basic workflow
- ✅ Foundation to build more complex AI

**Keep going!** The framework makes it easy to create many more AIs. Each one gets faster and easier.

---

## Remember

🎯 **Start simple, add complexity gradually**

- First AI: Use defaults, minimal configuration
- Second AI: Add traits, relationships
- Third AI: Try custom behaviors
- Fourth+: Build your unique AI ecosystem

**You're ready to create amazing AI for your game!**

---

## Help and Resources

**Stuck?**
- Check troubleshooting in setup guides
- Compare to YAML templates
- Simplify until it works
- Ask in community

**Want to learn more?**
- Read comprehensive guides
- Study template configurations
- Experiment with overrides
- Build and iterate

**Ready for advanced topics?**
- Custom BlueprintNativeEvent overrides
- Complex decision trees
- Multi-AI interaction systems
- Performance optimization

---

Good luck, and have fun creating AI! 🚀
