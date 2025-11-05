# Designer Onboarding: Faction System and Station Editor Best Practices

## Welcome!

This guide will help you quickly get up to speed with the Faction System and Station Editor in Adastrea. Whether you're a new team member or refreshing your knowledge, this document covers everything you need to know to start creating compelling content.

---

## Table of Contents
1. [Quick Start (30 Minutes)](#quick-start-30-minutes)
2. [Core Concepts](#core-concepts)
3. [Designer Workflows](#designer-workflows)
4. [Best Practices](#best-practices)
5. [Common Pitfalls and Solutions](#common-pitfalls-and-solutions)
6. [Advanced Techniques](#advanced-techniques)
7. [Team Collaboration](#team-collaboration)
8. [Resources and Support](#resources-and-support)

---

## Quick Start (30 Minutes)

**Before you begin:** Ensure your development environment is properly set up. Run `python SetupCheck.py` or `./SetupCheck.sh` from the project root to verify all prerequisites are met. See [README.md](../README.md#robust-setup-validation) for details.

### Your First Faction (15 minutes)

**Goal:** Create a complete faction from scratch.

1. **Open the project** in Unreal Engine
2. **Navigate** to `Content/Factions/` folder
3. **Right-click** → Miscellaneous → Data Asset
4. **Select** `FactionDataAsset`
5. **Name it** `DA_Faction_YourFactionName`

**Fill in these essentials:**

```
Faction Info:
  ✏️ Faction Name: "Your Faction Name"
  ✏️ Faction ID: YourFactionName (no spaces!)
  ✏️ Description: Write 2-3 sentences about the faction
  🎨 Primary Color: Choose a distinct color
  🎨 Secondary Color: Choose a complementary color

Faction Relations:
  ⭐ Initial Reputation: 0 (neutral)
  ⚔️ Hostile By Default: false

Faction Attributes:
  🔬 Technology Level: 5 (average)
  🛡️ Military Strength: 5 (average)
  💰 Economic Power: 5 (average)
```

6. **Click Save**
7. **Test it:** Place a Space Station in a test level, assign your faction

**✅ Success Criteria:** You can see your faction name and colors on the station.

### Your First Station Module (15 minutes)

**Goal:** Create a custom station module.

1. **Navigate** to `Content/Blueprints/Modules/` folder
2. **Right-click** → Blueprint Class
3. **Parent Class:** Search for `SpaceStationModule`
4. **Name it** `BP_Module_YourModuleName`
5. **Open** the Blueprint

**Configure in Class Defaults:**

```
Module Properties:
  ✏️ Module Type: "YourModuleType" (e.g., "Custom Trading Hub")
  ⚡ Module Power: 100.0 (power consumption or generation)
  📦 Module Group: Select appropriate category (e.g., Public)
```

6. **Add a visual mesh** (in the Viewport):
   - Add a Static Mesh Component
   - Choose a placeholder mesh
   - Scale and position as needed

7. **Compile and Save**

8. **Test it:** 
   - Place a Space Station in a test level
   - In the station's details, click "+' to add a module
   - Select your module Blueprint
   - It should appear attached to the station

**✅ Success Criteria:** You can see your module attached to a station.

---

## Core Concepts

### What is a Faction?

**Definition:** A faction is a group or organization in the game world with its own identity, goals, and relationships.

**Think of it like:**
- A country in Civilization
- A corporation in EVE Online
- A house in Game of Thrones

**What makes a good faction:**
- Clear identity (who are they?)
- Distinct visual style (colors, symbols)
- Unique strengths and weaknesses
- Interesting relationships with other factions
- Room for storytelling

### Faction Data Assets vs. Runtime State

**Important Distinction:**

```
📦 Faction Data Asset (Static Template)
  - Created by designers in the editor
  - Defines faction's "blueprint"
  - Doesn't change during gameplay
  - Examples: Base colors, starting attributes, default traits

🎮 Runtime State (Dynamic)
  - Created by code during gameplay
  - Tracks changing data
  - Examples: Current player reputation, temporary traits, war/peace status
```

**What this means for you:**
- Design the "ideal" faction in the data asset
- Code will handle changes during gameplay
- You can't change runtime state from the data asset
- Use data assets as templates, not save files

### Station Editor Architecture

**The Three-Layer Cake:**

```
Layer 1: C++ Foundation (You don't touch this)
  - Core logic for adding/removing modules
  - Faction assignment
  - Validation and safety checks

Layer 2: Blueprint Logic (You work here)
  - UI layout and design
  - Event handling (button clicks, etc.)
  - Visual feedback
  - Custom workflows

Layer 3: Content (You create this)
  - Module Blueprints
  - Station Blueprints
  - UI widgets
  - Visual assets (meshes, materials)
```

**Your job:** Create Layer 3 and customize Layer 2.

---

## Designer Workflows

### Workflow 1: Creating a Faction from Concept

**Starting Point:** You have a faction idea.

**Step-by-Step:**

1. **Brainstorm Identity**
   - What's their purpose? (traders, military, scientists, pirates)
   - What's their personality? (aggressive, peaceful, secretive)
   - What makes them unique?

2. **Define Attributes**
   - Tech Level: How advanced are they?
   - Military: How strong is their fleet?
   - Economy: How rich are they?
   - Balance: Not all 10s, not all 1s

3. **Choose Colors**
   - Primary: Main faction color (flags, UI, ships)
   - Secondary: Accent color (details, highlights)
   - Test against space background (avoid too dark)
   - Check against other factions (must be distinct)

4. **Write Description**
   - 2-3 sentences
   - Who, what, why
   - Leave some mystery
   - No spoilers for late-game content

5. **Add Traits (Optional but Recommended)**
   - 2-4 traits per faction
   - Mix of positive and negative
   - Reflect faction identity
   - Keep modifiers balanced (0.10 to 0.30 typical)

6. **Define Relationships**
   - Who are their friends?
   - Who are their enemies?
   - Who do they trade with?
   - Create a relationship web (3-5 other factions)

7. **Test and Iterate**
   - Assign to a station
   - Check colors in-game
   - Test tech restrictions
   - Get feedback from team

### Workflow 2: Designing a Station Module

**Starting Point:** You need a new module type.

**Step-by-Step:**

1. **Define Purpose**
   - What does this module do?
   - What category does it fit? (Docking, Power, etc.)
   - What tech level is required?

2. **Set Properties**
   - Module Type: Descriptive name
   - Module Power: Does it consume or generate? How much?
   - Module Group: Pick the right category

3. **Create Visual**
   - Find or create a mesh
   - Scale appropriately (consider station size)
   - Add materials (consider faction colors later)
   - Test attachment to station

4. **Add Faction Restrictions (If Needed)**
   - Should only high-tech factions build this?
   - Code checks Module Group for tech requirements
   - Defence/Public = Tech 7+
   - Processing = Tech 5+
   - Others = Always available

5. **Add Gameplay Hook (Advanced)**
   - Blueprint logic for module's function
   - Example: Power module generates electricity
   - Example: Docking module allows ship landing

6. **Test**
   - Can all appropriate factions build it?
   - Does it attach correctly?
   - Does it look good from all angles?
   - Does it fit the station aesthetic?

### Workflow 3: Balancing Factions

**Goal:** Ensure all factions are viable and interesting.

**Guidelines:**

**Attribute Balance:**
```
Total Attributes: Should average 15-20 across all factions
  - Low: 12-15 (struggling faction)
  - Average: 15-18 (standard faction)
  - High: 19-24 (powerful faction)

Individual Attributes:
  - Avoid 10s unless it's the faction's defining trait
  - Avoid 1s unless it's a critical weakness
  - Most should be 3-7 range
```

**Relationship Balance:**
```
Per Faction:
  - 1-2 allies (close friends)
  - 1-2 enemies (active conflicts)
  - 2-4 neutral (trading partners, wary neighbors)

Avoid:
  - Everyone allied with everyone
  - Everyone at war with everyone
  - Isolated factions with zero relationships
```

**Trait Balance:**
```
Per Faction:
  - 2-4 traits total
  - 1-2 major strengths (0.20 - 0.40 modifiers)
  - 1-2 minor weaknesses (-0.10 to -0.20 modifiers)
  - Or all moderate (0.10 - 0.20 modifiers)

Avoid:
  - Stacking too many positives
  - No weaknesses at all
  - Traits that contradict faction identity
```

### Workflow 4: Creating Faction UI

**Goal:** Make a widget that shows faction info and allows selection.

**Quick Blueprint Setup:**

1. **Create Widget Blueprint**
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_FactionSelector`
   - Parent: Can stay as UserWidget (or extend StationEditorWidget for advanced)

2. **Design Layout**
   - Add a Combo Box (String) for faction dropdown
   - Add Text Blocks for faction name, description
   - Add Images or Color Blocks for primary/secondary colors
   - Add Progress Bars for attributes

3. **Populate Dropdown** (Event Graph)
   ```
   Event Construct
     ↓
   Get All Faction Data Assets (your custom function or hard-code)
     ↓
   ForEach Loop
     → Add Option to Combo Box
   ```

4. **On Selection Changed**
   ```
   [Combo Box] Selection Changed Event
     ↓
   Get Selected Faction Data Asset
     ↓
   Update all UI elements (text, colors, bars)
   ```

5. **Test**
   - Add to viewport in test level
   - Verify all factions appear
   - Check that selection updates UI

**See Also:** `BlueprintWorkflowTemplates.md` for detailed examples

---

## Best Practices

### Naming Conventions

**Factions:**
```
✅ Good:
  - Asset: DA_Faction_NovaVanguard
  - Faction ID: NovaVanguard
  - Display Name: "Nova Vanguard"

❌ Bad:
  - Asset: Faction1
  - Faction ID: nova vanguard (spaces!)
  - Display Name: "nOVA vANGUARD" (bad caps)
```

**Modules:**
```
✅ Good:
  - Blueprint: BP_Module_DockingBay
  - Module Type: "Docking Bay"
  - Clear, descriptive

❌ Bad:
  - Blueprint: Module1
  - Module Type: "Thingy"
  - Vague, unclear
```

**Traits:**
```
✅ Good:
  - Trait ID: EliteTraining
  - Trait Name: "Elite Training"
  - Description: "+25% combat effectiveness"

❌ Bad:
  - Trait ID: trait1
  - Trait Name: "Good"
  - Description: "Makes things better"
```

### Organization

**Folder Structure:**
```
Content/
  Factions/
    DA_Faction_NovaVanguard
    DA_Faction_SolarisUnion
    DA_Faction_CelestialSyndicate
    [etc.]
  
  Blueprints/
    Stations/
      BP_SpaceStation_Military
      BP_SpaceStation_Trading
    Modules/
      Docking/
        BP_Module_SmallDockingBay
        BP_Module_LargeDockingBay
      Power/
        BP_Module_SolarPanel
        BP_Module_Reactor
      [etc.]
  
  UI/
    Faction/
      WBP_FactionSelector
      WBP_FactionInfo
    Station/
      WBP_StationEditor
      WBP_ModuleList
```

### Version Control

**Always:**
- Save your work frequently
- Commit logical chunks (one faction, one module)
- Write clear commit messages
- Pull before you push
- Test before committing

**Never:**
- Commit broken assets
- Commit half-finished work to main branch
- Force push without asking team
- Delete others' work without discussion

### Documentation

**Document your factions:**
```
In the Description field, include:
  - Faction's role in the world
  - Why they exist
  - How they interact with others

Separately (in a design doc or wiki):
  - Story arcs involving this faction
  - Mission ideas
  - Future expansion plans
  - Visual references
```

**Document your modules:**
```
In comments:
  - What the module does
  - Any special requirements
  - Known issues or limitations
  - Future features planned
```

---

## Common Pitfalls and Solutions

### Pitfall 1: Duplicate Faction IDs

**Problem:** Two factions have the same Faction ID.

**Symptoms:**
- Diplomacy relationships don't work
- Wrong faction displays
- Confusion in code

**Solution:**
- Use a naming spreadsheet to track all Faction IDs
- Check existing factions before creating new ones
- Use descriptive, unique names

**Prevention:**
- Establish naming convention early
- Code review before merging
- Automated test to detect duplicates (future)

### Pitfall 2: Colors Too Similar

**Problem:** Two factions have nearly identical colors.

**Symptoms:**
- Players can't distinguish factions
- Visual confusion
- Poor user experience

**Solution:**
- Test colors side-by-side in-game
- Use color theory (complementary, triadic)
- Ask teammates for feedback
- Use colorblind-friendly palettes

**Prevention:**
- Create color palette guide at start
- Reserve color ranges for faction types (blue = peaceful, red = aggressive)

### Pitfall 3: Unbalanced Attributes

**Problem:** One faction is overpowered or useless.

**Symptoms:**
- Players always choose the same faction
- Some factions feel boring
- Gameplay isn't challenging

**Solution:**
- Use the balance guidelines (see above)
- Playtest with different factions
- Gather player feedback
- Iterate based on data

**Prevention:**
- Balance from the start
- Regular playtesting
- Document design intent

### Pitfall 4: Broken Relationships

**Problem:** Faction A references Faction B, but Faction B doesn't exist.

**Symptoms:**
- Errors in output log
- Missing diplomacy data
- Crashes (in worst case)

**Solution:**
- Create all factions before adding relationships
- Double-check Target Faction IDs
- Use existing faction list as reference

**Prevention:**
- Create factions in order (all basic info first, then relationships)
- Keep a master list of all faction IDs

### Pitfall 5: Module Won't Attach

**Problem:** Module Blueprint doesn't attach to station.

**Symptoms:**
- Module spawns but floats away
- Module doesn't show in station's module list
- Error messages

**Solution:**
- Verify module extends SpaceStationModule class
- Check that station's AddModule function is called
- Ensure module isn't being destroyed immediately

**Prevention:**
- Use templates/starter Blueprints
- Test attachment immediately after creation

### Pitfall 6: UI Doesn't Update

**Problem:** Faction selection doesn't update UI elements.

**Symptoms:**
- Old faction name stays visible
- Colors don't change
- Attribute bars don't update

**Solution:**
- Check event bindings (is Selection Changed event connected?)
- Verify faction reference is valid (not null)
- Add debug prints to trace execution

**Prevention:**
- Test UI immediately after creating
- Use property bindings for automatic updates
- Follow workflow templates exactly

---

## Advanced Techniques

### Technique 1: Conditional Traits

**Goal:** Add traits based on game state.

**Example:**
```
Faction is at war:
  → Add temporary trait "War Economy"
  → +20% military, -10% trade
  → Remove trait when war ends
```

**Implementation:** Requires runtime trait system (see IterationGuide.md)

### Technique 2: Dynamic Relationships

**Goal:** Change faction relationships during gameplay.

**Example:**
```
Player completes peace mission:
  → Change relationship from -50 (hostile) to 0 (neutral)
  → Update both factions' relationship arrays
  → Trigger cutscene or notification
```

**Implementation:** Requires diplomacy manager (see IterationGuide.md)

### Technique 3: Faction-Specific Modules

**Goal:** Some modules only available to certain factions.

**Example:**
```
"Cloaking Device" module:
  → Only buildable by factions with trait "StealthTechnology"
  → Check in CanAddModuleForFaction function
```

**Implementation:**
```cpp
bool UStationEditorWidget::CanAddModuleForFaction(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
    // ... existing tech level check ...
    
    // Check faction traits
    if (ModuleType == "CloakingDevice")
    {
        if (!CurrentStation->HasFactionTrait("StealthTechnology"))
            return false;
    }
    
    return true;
}
```

### Technique 4: Trait Tooltips in UI

**Goal:** Show what traits do when hovering.

**Blueprint:**
```
[Trait List Item Widget]
  → On Hovered
    → Create Tooltip Widget
      → Set Tooltip Text to Trait Description
      → Show Modifier Value (+25% or -10%)
    → Display Tooltip
```

### Technique 5: Reputation Visualization

**Goal:** Show player's standing with all factions.

**Blueprint:**
```
[Reputation Screen Widget]
  ForEach Faction
    → Get Player Reputation (from component)
    → Create List Item
      → Show Faction Name
      → Show Color-coded Bar (red = hostile, green = friendly)
      → Show Numerical Value
    → Add to Scroll Box
```

---

## Team Collaboration

### Communication

**Daily:**
- Check team chat for faction/module updates
- Announce when creating new factions
- Share screenshots of new modules

**Weekly:**
- Attend design meeting
- Present new content
- Discuss balance issues
- Collect feedback

**Before Committing:**
- Test your changes
- Check for conflicts
- Write clear commit message
- Tag relevant team members

### Code Review

**When requesting review:**
- Explain what you changed
- Link to design doc if applicable
- Show before/after screenshots
- List any known issues

**When reviewing:**
- Check naming conventions
- Verify balance
- Test in editor
- Provide constructive feedback

### Design Documents

**Maintain:**
- Faction Master List (all faction IDs and summaries)
- Module Catalog (all module types and properties)
- Balance Spreadsheet (attribute totals)
- Relationship Web (visual map of alliances/wars)

**Share:**
- Google Docs for collaborative editing
- Version control for final versions
- Screenshots in team chat
- Videos of new features

---

## Resources and Support

### Documentation

**Essential Reading:**
- `README.md` - Project overview
- `BlueprintFactionAssetGuide.md` - Detailed faction creation
- `BlueprintWorkflowTemplates.md` - Blueprint examples
- `FactionSetupGuide.md` - Faction templates
- `SpaceshipDataAssetGuide.md` - Ship creation walkthrough
- `SpaceshipInteriorGuide.md` - Interior implementation guide

**Advanced Reading:**
- `IterationGuide.md` - Future expansion plans
- `PlaytestingChecklist.md` - Quality assurance
- `ShipControlGuide.md` - Ship control system

### Example Assets

**In the Project:**
- `Content/Factions/Examples/` - Example faction data assets
- `Content/Blueprints/Modules/Examples/` - Example modules
- `Content/UI/Examples/` - Example UI widgets

**Study these to learn patterns and best practices.**

### Getting Help

**When Stuck:**

1. **Check Documentation** (you're reading it!)
2. **Look at Examples** (in Content/Examples/)
3. **Search Output Log** (for error messages)
4. **Ask a Teammate** (in team chat)
5. **Ask a Programmer** (for C++ issues)

**When Asking for Help:**
- Describe what you're trying to do
- Explain what's happening instead
- Share relevant screenshots/logs
- Mention what you've already tried

### Training Resources

**Unreal Engine Basics:**
- UE5 Documentation: https://docs.unrealengine.com/
- UE5 Learning Portal: https://www.unrealengine.com/en-US/onlinelearning
- YouTube: Unreal Sensei, Virtus Learning Hub

**Game Design:**
- Extra Credits (YouTube)
- Game Maker's Toolkit (YouTube)
- GDC talks on faction systems

**Our Systems:**
- This documentation!
- Team wiki (if you have one)
- Recorded walkthroughs (ask senior designer)

---

## Onboarding Checklist

Use this checklist for new team members:

**Week 1: Learning**
- [ ] Read this document (Designer Onboarding)
- [ ] Read README.md and FactionSetupGuide.md
- [ ] Browse example assets in Content/
- [ ] Watch team walkthrough recording (if available)
- [ ] Set up version control access

**Week 2: Creating**
- [ ] Create your first faction (with mentor review)
- [ ] Create your first module (with mentor review)
- [ ] Test both in-game
- [ ] Get feedback from team

**Week 3: Contributing**
- [ ] Create a faction for the game (final version)
- [ ] Create 2-3 modules for the game
- [ ] Balance against existing factions
- [ ] Submit for code review
- [ ] Merge to main branch

**Week 4: Mastery**
- [ ] Help another new team member
- [ ] Propose a new faction feature
- [ ] Create advanced UI widget
- [ ] Contribute to documentation

**Ongoing:**
- [ ] Attend weekly design meetings
- [ ] Participate in playtesting
- [ ] Suggest balance changes
- [ ] Document your work

---

## Quick Reference Card

**Creating a Faction:**
1. Right-click → Data Asset → FactionDataAsset
2. Set Name, ID, Colors
3. Set Attributes (Tech, Military, Economy)
4. Save and test

**Creating a Module:**
1. Right-click → Blueprint → SpaceStationModule
2. Set Type, Power, Group
3. Add mesh
4. Save and test

**Assigning Faction to Station:**
1. Select station in level
2. Details → Station → Faction
3. Pick faction from dropdown

**Common Checks:**
- Faction ID unique? ✓
- Colors distinct? ✓
- Attributes balanced? ✓
- Relationships make sense? ✓
- Module attaches? ✓

**Need Help?**
1. Check docs
2. Ask team
3. Review examples

---

## Closing Thoughts

**Remember:**
- Start simple, iterate
- Test frequently
- Ask questions early
- Document your work
- Have fun creating!

**You're building a galaxy. Make it awesome.** 🌟

---

**Questions? Suggestions for this guide?**  
Contact the lead designer or submit a documentation improvement request.

**Version:** 1.0  
**Last Updated:** [Your Date]  
**Maintained By:** [Your Team]
