# AI Designer Workflow Guide
## Complete Workflow for Designers (No C++ Required!)

This guide walks you through the entire process of creating and implementing AI for your game, from concept to testing. Perfect for designers with no programming experience!

---

## Table of Contents
1. [Overview](#overview)
2. [The Big Picture](#the-big-picture)
3. [Workflow Steps](#workflow-steps)
4. [Daily Workflow](#daily-workflow)
5. [Common Tasks Quick Reference](#common-tasks-quick-reference)
6. [Tips and Best Practices](#tips-and-best-practices)

---

## Overview

### What You Can Do (Without C++)

✅ **Create Factions** - Design political/military organizations with unique traits and priorities
✅ **Create Personnel** - Design individual characters with personalities and skills
✅ **Create Ways** - Design economic guilds and trade networks (see [Way System Guide](WaySystemGuide.md))
✅ **Configure AI Behaviors** - Set priorities, relationships, and decision patterns
✅ **Customize Logic** - Override specific behaviors in Blueprint visual scripting
✅ **Test and Iterate** - See AI in action and refine behavior

### What's Already Done (In C++)

✅ Core AI systems and update loops
✅ Default behaviors for all AI types
✅ Data structures and properties
✅ Blueprint hooks and events
✅ Debug visualization

---

## The Big Picture

### Three Layers of AI

```
┌─────────────────────────────────────────┐
│  1. DATA ASSETS (Your Content)          │
│  Factions, Personnel, Stats, Traits     │
│  ↓ References                           │
│  2. LOGIC BLUEPRINTS (Your Behavior)    │
│  Decision making, Priorities, Events    │
│  ↓ Used by                              │
│  3. GAME ACTORS (Your World)            │
│  Ships, Stations, Characters            │
└─────────────────────────────────────────┘
```

### How It All Connects

1. **Data Assets** store the facts (who, what, stats)
2. **Logic Blueprints** use data to make decisions (how, when, why)
3. **Game Actors** execute the decisions (actions in world)

---

## Workflow Steps

### Phase 1: Planning (Design Document)

**Before opening Unreal**, answer these questions:

**For Factions:**
1. What's the faction's philosophy? (peaceful, aggressive, mercantile)
2. What are their priorities? (exploration, trade, conquest)
3. Who are their friends/enemies?
4. What unique traits do they have?
5. Are they early-game focused?

**For Personnel:**
1. What's their personality type? (curious, disciplined, empathetic, etc.)
2. What's their role? (captain, engineer, medic, etc.)
3. What are they good at? (skills)
4. How do they handle stress?
5. Who are they friends with?

**Example Faction Planning:**
```
Faction: Peaceful Explorers
Philosophy: Discovery through peaceful means
Priorities: Exploration (9), Trade (7), Diplomacy (8)
Allies: Research Collective, Trade Guild
Enemies: Pirate Clans
Unique: Scientific focus, pacifist
Early Game: Yes - avoid conflict, build alliances
```

### Phase 2: Create Data Assets

**Step-by-Step:**

1. **Open Content Browser** in Unreal
2. **Right-Click** → **Blueprint Class**
3. **Choose Parent:** FactionDataAsset or PersonnelDataAsset
4. **Name it** (e.g., `DA_PeacefulExplorers`)
5. **Open and configure** all fields

**Use the YAML templates** as reference:
- Copy values from `Assets/FactionAITemplates/*.yaml`
- Copy values from `Assets/PersonnelAITemplates/*.yaml`

**Pro Tip:** Create several data assets at once, then configure them all. More efficient!

### Phase 3: Create Logic Blueprints

**Step-by-Step:**

1. **Right-Click** → **Blueprint Class**
2. **Choose Parent:** FactionLogic or PersonnelLogic
3. **Name it** (e.g., `BP_PeacefulExplorerLogic`)
4. **Open Blueprint**
5. **Assign Data Asset** in Class Defaults
6. **Configure AI Settings:**
   - Disposition (for personnel)
   - Early Game Priorities (for factions)
   - Behavior Mode
   - Update Interval

**Start Simple:** Don't override anything yet! Use default behaviors first.

### Phase 4: Test Default Behavior

**Create a test level:**

1. **Create a simple test map**
2. **Add Faction/Personnel Manager** (Blueprint Actor)
3. **Add your Logic Blueprint** as component
4. **In BeginPlay:**
   ```
   Initialize AI → Update AI (with Tick)
   ```
5. **Run the game**
6. **Watch debug messages**

**What to look for:**
- ✅ AI initializes without errors
- ✅ AI updates every interval
- ✅ Decisions make sense for configuration
- ✅ Debug messages show activity

### Phase 5: Customize Behaviors (Optional)

**Only if needed!** Default behaviors work well.

**When to customize:**
- ❌ Don't like default behavior
- ✅ Want unique faction personality
- ✅ Special scenario needs special logic
- ✅ Want more control over decisions

**How to customize:**

1. **Open Logic Blueprint**
2. **Event Graph**
3. **Right-Click** → **Add Event** → **Event [Function Name]**
4. **Implement custom logic**
5. **Call Parent Function** for default fallback

**Start with ONE event**, test it, then add more.

### Phase 6: Iteration and Polish

**Refine your AI:**

1. **Play the game** with your AI
2. **Take notes** on behavior
3. **Adjust values** in Data Assets
4. **Test again**
5. **Repeat** until satisfied

**Common adjustments:**
- Priority values (too aggressive? lower military priority)
- Update intervals (AI too reactive? increase interval)
- Relationship values (wrong allies? adjust relationships)
- Trait modifiers (not unique enough? add/adjust traits)

---

## Daily Workflow

### Morning: Planning

1. **Review design doc** - What am I building today?
2. **Check existing assets** - What's already done?
3. **Plan tasks** - What needs creating/fixing?

### Midday: Creation

1. **Create Data Assets** (30-60 min)
   - Factions or Personnel
   - Use YAML templates
2. **Create Logic Blueprints** (15-30 min)
   - Assign data
   - Configure settings
3. **Quick test** (10-15 min)
   - Does it work?
   - Any errors?

### Afternoon: Testing

1. **Extended testing** (30-60 min)
   - Watch AI in action
   - Take notes
2. **Iteration** (30-60 min)
   - Adjust values
   - Fix issues
   - Re-test

### End of Day: Documentation

1. **Document decisions** (15 min)
   - What worked?
   - What didn't?
   - What's next?
2. **Save and commit**

---

## Common Tasks Quick Reference

### Create a New Faction

**Time: 20-30 minutes**

1. ✅ Plan faction concept
2. ✅ Create `DA_[FactionName]` Data Asset
3. ✅ Fill in all fields (use YAML template)
4. ✅ Create `BP_[FactionName]Logic` Blueprint
5. ✅ Assign Data Asset
6. ✅ Set Early Game Priorities
7. ✅ Test in game

### Create a New Crew Member

**Time: 15-25 minutes**

1. ✅ Plan character concept
2. ✅ Create `DA_[Name]` Data Asset
3. ✅ Fill in identity, role, skills (use YAML)
4. ✅ Create `BP_[Name]AI` Blueprint
5. ✅ Assign Data Asset
6. ✅ Set Disposition
7. ✅ Test in game

### Add a Faction Relationship

**Time: 2-5 minutes**

1. ✅ Open Faction Data Asset
2. ✅ Find "Faction Relationships" array
3. ✅ Click **+**
4. ✅ Set Target Faction ID
5. ✅ Set Relationship Value (-100 to 100)
6. ✅ Set Alliance/War flags
7. ✅ Set Trade Modifier
8. ✅ Save

### Add a Personnel Skill

**Time: 1-2 minutes**

1. ✅ Open Personnel Data Asset
2. ✅ Find "Skills" array
3. ✅ Click **+**
4. ✅ Set Skill Name
5. ✅ Set Skill Level (1-10)
6. ✅ Set Experience Points
7. ✅ Save

### Modify AI Priorities

**Time: 2-5 minutes**

**For Factions:**
1. ✅ Open Logic Blueprint
2. ✅ Class Defaults → Early Game Priorities
3. ✅ Adjust values (0-10)
4. ✅ Save and test

**For Personnel:**
1. ✅ Open Logic Blueprint
2. ✅ Class Defaults → Disposition
3. ✅ Change to desired type
4. ✅ Save and test

### Add a Custom Behavior

**Time: 15-45 minutes (varies by complexity)**

1. ✅ Open Logic Blueprint
2. ✅ Event Graph
3. ✅ Right-Click → Add Event → [Choose Event]
4. ✅ Add your custom logic nodes
5. ✅ Add "Call Parent Function" node
6. ✅ Connect nodes
7. ✅ Compile
8. ✅ Test

### Debug AI Issues

**Time: 10-30 minutes**

1. ✅ Check debug messages in game
2. ✅ Print "Get State Description" to screen
3. ✅ Verify Data Asset is assigned
4. ✅ Check "Is Active" is true
5. ✅ Verify "Initialize AI" was called
6. ✅ Check "Update AI" is being called
7. ✅ Review values in Data Asset
8. ✅ Test with simpler configuration

---

## Tips and Best Practices

### Data Asset Best Practices

✅ **DO:**
- Use descriptive names (`DA_PeacefulExplorer` not `DA_Faction1`)
- Use YAML templates as reference
- Start with moderate values, adjust later
- Document unusual values in notes
- Create folders to organize assets

❌ **DON'T:**
- Leave fields at default without reason
- Use extreme values (0 or 10) everywhere
- Copy-paste without customization
- Forget to set Faction/Personnel ID

### Blueprint Best Practices

✅ **DO:**
- Start with default behaviors
- Override ONE thing at a time
- Test after each change
- Comment your custom logic
- Call Parent Function when appropriate

❌ **DON'T:**
- Override everything (unnecessary!)
- Make complex logic immediately
- Skip testing
- Ignore errors/warnings

### Testing Best Practices

✅ **DO:**
- Test in isolated environment first
- Use debug messages/prints
- Test edge cases (low morale, high stress)
- Test interactions between AI
- Keep notes on behavior

❌ **DON'T:**
- Test everything at once
- Ignore debug output
- Skip edge cases
- Test in production level first

### Iteration Best Practices

✅ **DO:**
- Make small changes
- Test after each change
- Keep a changelog
- Ask "does this feel right?"
- Get feedback from others

❌ **DON'T:**
- Change 10 things at once
- Assume first attempt is perfect
- Skip documentation
- Work in isolation forever

### Organization Best Practices

**Folder Structure:**
```
Content/
├── AI/
│   ├── Factions/
│   │   ├── DataAssets/
│   │   │   ├── DA_PeacefulExplorers
│   │   │   └── DA_TradeCoalition
│   │   └── Logic/
│   │       ├── BP_PeacefulExplorerLogic
│   │       └── BP_TradeCoalitionLogic
│   └── Personnel/
│       ├── DataAssets/
│       │   ├── DA_Explorer_Rivera
│       │   └── DA_Security_Chen
│       └── Logic/
│           ├── BP_ExplorerAI
│           └── BP_SecurityAI
```

### Naming Conventions

**Data Assets:**
- Factions: `DA_[FactionName]` (e.g., `DA_PeacefulExplorers`)
- Personnel: `DA_[Role]_[LastName]` (e.g., `DA_Pilot_Martinez`)

**Logic Blueprints:**
- Factions: `BP_[FactionName]Logic` (e.g., `BP_PeacefulExplorerLogic`)
- Personnel: `BP_[Name]AI` or `BP_[Role]AI` (e.g., `BP_ExplorerAI`)

### Performance Tips

⚡ **For good performance:**
- Use appropriate Update Intervals
  - Factions: 5-10 seconds
  - Personnel: 1-2 seconds
- Don't create hundreds of AI at once
- Disable AI for distant/inactive entities
- Use simpler logic for background characters

### Common Pitfalls

🚫 **Avoid these mistakes:**

1. **Not assigning Data Asset** → AI has no data to work with
2. **Forgetting to Initialize** → AI never starts
3. **Not calling Update** → AI doesn't tick
4. **Mismatched IDs** → Relationships don't work
5. **Extreme priority values** → Unbalanced behavior
6. **Overriding too much** → Hard to maintain
7. **No testing** → Broken AI in game
8. **No documentation** → Can't remember what you did

---

## Your First Day Checklist

**Morning (30 min):**
- ✅ Read this guide
- ✅ Read Faction AI Setup Guide OR Personnel AI Setup Guide
- ✅ Pick which to create first (recommend: start with Faction)

**Midday (2 hours):**
- ✅ Create your first Data Asset using YAML template
- ✅ Create corresponding Logic Blueprint
- ✅ Assign Data Asset to Logic
- ✅ Configure basic settings

**Afternoon (2 hours):**
- ✅ Create simple test level
- ✅ Add your AI to test level
- ✅ Test and debug
- ✅ Iterate on values

**End of Day (30 min):**
- ✅ Document what you learned
- ✅ Note what worked/didn't work
- ✅ Plan tomorrow's tasks

**By end of day:** You should have ONE working AI (faction or personnel) that makes sensible decisions!

---

## Getting Help

**Resources:**
- [Faction AI Setup Guide](FactionAISetupGuide.md) - Detailed faction creation
- [Personnel AI Setup Guide](PersonnelAISetupGuide.md) - Detailed personnel creation
- [AI System Integration](AISystemIntegration.md) - Connecting to game systems
- [YAML Templates](FactionAITemplates/) - Pre-made configurations

**Troubleshooting:**
- Check "Troubleshooting" sections in setup guides
- Look at debug messages
- Simplify until it works, then add complexity
- Compare your setup to YAML templates

**Community:**
- Share your AI designs
- Ask for feedback
- Learn from others' creations

---

## Next Steps

**After your first AI works:**

1. ✅ Create 2-3 more (variety!)
2. ✅ Test interactions between them
3. ✅ Try customizing ONE behavior
4. ✅ Build more complex scenarios
5. ✅ Integrate into your actual game

**Long term goals:**

- Build diverse faction ecosystem (5-10 factions)
- Create memorable crew characters (10-20 personnel)
- Design unique faction interactions
- Create dynamic story through AI relationships
- Polish and balance AI behaviors

---

## Remember

🎯 **You don't need to know C++ to create great AI!**

The system is designed for designers:
- Data Assets = Your content
- Blueprints = Your control
- C++ = Already done for you

**Start simple, iterate often, have fun!**
