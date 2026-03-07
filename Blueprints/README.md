# Blueprint Documentation Overview

> **Find the right Blueprint guide for your needs**

**Last Updated**: 2026-01-03

---

## 📚 Quick Navigation

### For Blueprint Authors

**Creating a new guide?**
1. Start here: [Blueprint Guide Template](../reference/BLUEPRINT_GUIDE_TEMPLATE.md) - Copy this template
2. Read this: [Blueprint Guide Standards](../reference/BLUEPRINT_GUIDE_STANDARDS.md) - How to write guides
3. See example: [Trading UI Simplified Guide](TradingSystemBlueprintGuide_SIMPLIFIED.md) - Reference implementation

### For Blueprint Users

**Learning to use Blueprints?**
1. Start here: [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md) - Complete beginner course
2. Then try: [Trading UI Guide](TradingSystemBlueprintGuide_SIMPLIFIED.md) - First real Blueprint
3. Advanced: See the guides list below

---

## 📋 Available Blueprint Guides

### 🚀 MVP Essential Guides (Start Here)

These guides are critical for the Trade Simulator MVP:

| Guide | Complexity | Time | Status |
|-------|------------|------|--------|
| [Trading Ship (Player)](BP_TradingShip_GUIDE.md) | Intermediate | 40-50 min | ✅ New |
| [Trading Game Mode](BP_TradingGameMode_GUIDE.md) | Intermediate | 35-45 min | ✅ New |
| [Market Display Widget](WBP_MarketDisplay_GUIDE.md) | Beginner | 25-35 min | ✅ New |
| [Trading UI Widget](TradingSystemBlueprintGuide_SIMPLIFIED.md) | Intermediate | 45-60 min | ✅ Updated |
| [Trading Station Actor](TradingStationActorGuide.md) | Beginner | 30-40 min | ✅ Updated |
| [Ship Cargo System](ShipCargoSystemGuide.md) | Beginner | 25-35 min | ✅ Updated |
| [Docking UI](DockingUIGuide.md) | Beginner | 20-30 min | ✅ Updated |

### 🎮 System-Specific Guides

Organized by game system:

#### Core MVP Blueprints
- [Trading Ship (Player)](BP_TradingShip_GUIDE.md) - Player's flyable trading ship ✅ **NEW**
- [Trading Game Mode](BP_TradingGameMode_GUIDE.md) - Economy simulation and game rules ✅ **NEW**
- [Market Display Widget](WBP_MarketDisplay_GUIDE.md) - Price comparison display ✅ **NEW**

#### Trading System
- [Trading UI Widget](TradingSystemBlueprintGuide_SIMPLIFIED.md) - Main trading interface ✅
- [Trading Station Actor](TradingStationActorGuide.md) - Station with trading terminal ✅
- [Trading System (Original)](DEPRECATED_TradingSystemBlueprintGuide.md) - 🗑️ Deprecated, use simplified version
- [Trading Blueprint Examples](../Assets/TradingBlueprintExamples.md) - ⚠️ Advanced patterns

#### Ship Systems
- [Ship Cargo System](ShipCargoSystemGuide.md) - Cargo management for trading ✅
- [Ship Status Screen](ShipStatusScreenGuide.md) - HUD display ✅
- [Ship Controls](../docs/setup/SPACESHIP_HUD_BLUEPRINT_GUIDE.md) - Flight controls UI

#### Station Systems
- [Space Station Editor](BlueprintWorkflowTemplates.md) - Station building UI (from Assets folder)
- [Docking UI](DockingUIGuide.md) - Station interaction prompts ✅

#### Way Network System
- [Way Networks](WayNetworkBlueprintExamples.md) - Faction relationship system

#### Homeworld System
- [Homeworld Templates](HomeworldBlueprintTemplates.md) - Planet and system creation

---

## 🔧 Guide Status Key

| Symbol | Meaning |
|--------|---------|
| ✅ | **Updated** - Follows new template standard |
| 🔄 | **Being Updated** - In progress |
| ⚠️ | **Legacy** - Old format, still functional but inconsistent |
| ⏳ | **Coming Soon** - Planned but not yet written |
| 🗑️ | **Deprecated** - Old system, no longer recommended |

---

## 📖 Documentation Structure

### New Standard (2026+)

All new Blueprint guides follow this structure:

```
1. Overview - What you'll build
2. Prerequisites - What you need first
3. Dependencies - Other Blueprints required
4. Setup - Variables and Functions to create UPFRONT
5. Step-by-Step - Numbered implementation
6. Testing - Verification steps
7. Troubleshooting - Common issues
```

**Key Improvement**: Variables and Functions are listed BEFORE implementation steps, so you know what to create upfront.

### Legacy Guides

Older guides may have:
- Inconsistent structure
- Missing prerequisites
- Variables discovered mid-implementation
- Varying levels of detail

**Migration Plan**: We're updating high-priority guides to the new standard. See [Blueprint Guide Standards](../reference/BLUEPRINT_GUIDE_STANDARDS.md) for details.

---

## 🎯 Finding the Right Guide

### I'm a Complete Beginner

**Start with**:
1. [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
2. Then pick any guide marked "Beginner" complexity

### I Know Basic Blueprints

**Try**:
- Any "Intermediate" complexity guide
- [Trading UI Widget](TradingSystemBlueprintGuide_SIMPLIFIED.md) is a good starting point

### I'm an Advanced User

**Options**:
- Pick any "Advanced" guide
- Original complex guides (marked ⚠️ Legacy)
- Contribute new guides using the template!

### I Want to Build [Specific Feature]

**Use the search**:
- Trading? → Trading System guides
- Ship controls? → Ship Systems guides
- Stations? → Station Systems guides
- UI? → Look for "Widget" guides

---

## 🛠️ Creating Your Own Guide

### Process

1. **Copy the template**: [`BLUEPRINT_GUIDE_TEMPLATE.md`](../reference/BLUEPRINT_GUIDE_TEMPLATE.md)
2. **Read the standards**: [`BLUEPRINT_GUIDE_STANDARDS.md`](../reference/BLUEPRINT_GUIDE_STANDARDS.md)
3. **Write your guide** following the structure
4. **Test the instructions** with a fresh Blueprint
5. **Submit as PR** or add to appropriate folder

### Where to Put Your Guide

| Guide Type | Location |
|------------|----------|
| General Blueprint guide | `Blueprints/` (this folder) |
| MVP-specific guide | `docs/mvp/` |
| Setup/installation guide | `docs/setup/` |
| System documentation | `docs/systems/` |
| Reference/template | `docs/reference/` |

### Naming Convention

**Use one of these formats**:
- `[SystemName]BlueprintGuide.md` - e.g., `TradingSystemBlueprintGuide.md`
- `[BlueprintName]_GUIDE.md` - e.g., `WBP_TradingUI_GUIDE.md`
- `[Feature]_BLUEPRINT_GUIDE.md` - e.g., `SPACESHIP_HUD_BLUEPRINT_GUIDE.md`

**For simplified versions**:
- Append `_SIMPLIFIED` - e.g., `TradingSystemBlueprintGuide_SIMPLIFIED.md`

---

## 📊 Migration Status

### Phase 1: Template Creation ✅ Complete

- [x] Create master template
- [x] Define standards document
- [x] Create example simplified guide

### Phase 2: High-Priority Updates 🔄 In Progress

**Target**: MVP-critical guides

- [x] Trading UI Widget
- [ ] Trading Station Actor
- [ ] Ship Cargo System
- [ ] Docking UI
- [ ] Market Display Widget

### Phase 3: System Guides ⏳ Planned

- [ ] Ship Systems guides
- [ ] Station Systems guides
- [ ] Combat guides
- [ ] Faction guides

### Phase 4: Advanced Guides ⏳ Planned

- [ ] AI Trader system
- [ ] Procedural generation
- [ ] Advanced UI patterns

---

## 🔍 Guide Quality Checklist

Before marking a guide as "Updated", verify:

- [ ] Follows template structure
- [ ] Lists prerequisites upfront
- [ ] Lists ALL variables before implementation
- [ ] Lists ALL functions before implementation
- [ ] Step-by-step numbered instructions
- [ ] Expected results stated for each step
- [ ] Testing section included
- [ ] Troubleshooting section included
- [ ] Tested by someone who didn't write it

---

## 💡 Tips for Using Guides

### Best Practices

1. **Read the whole guide first** - Don't start implementing before understanding the full scope
2. **Check prerequisites** - Verify you have everything before starting
3. **Create variables/functions upfront** - Don't wait until you need them
4. **Test after each major step** - Catch issues early
5. **Use troubleshooting section** - Don't struggle alone

### Common Mistakes to Avoid

- ❌ **Skipping prerequisites** - Leads to errors mid-implementation
- ❌ **Not creating variables upfront** - Have to backtrack
- ❌ **Rushing through steps** - Miss important details
- ❌ **Not testing incrementally** - Hard to debug later
- ❌ **Ignoring expected results** - Don't know if you're on track

---

## 📞 Getting Help

### If a Guide is Unclear

1. Check the [Troubleshooting section](#troubleshooting) in the guide
2. Review [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
3. Check [Blueprint Image Guide](../reference/BLUEPRINT_IMAGE_GUIDE.md) for node explanations
4. Open a GitHub issue with tag `documentation`

### If Something Doesn't Work

1. Double-check prerequisites are met
2. Review each step carefully
3. Check the Troubleshooting section
4. Add debug `Print String` nodes to see what's happening
5. Ask for help in Discord/GitHub with:
   - Which guide you're following
   - Which step failed
   - What error messages you see
   - Screenshots if possible

---

## 🎓 Learning Path

**Recommended progression for complete beginners**:

1. **Week 1: Basics**
   - Read [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
   - Create a simple button widget
   - Create a simple actor Blueprint

2. **Week 2: Simple Systems**
   - Follow [Trading UI Widget](TradingSystemBlueprintGuide_SIMPLIFIED.md)
   - Understand event-driven programming
   - Learn function calls to C++

3. **Week 3: Integration**
   - Connect trading UI to stations
   - Create your own item row widget
   - Customize the trading experience

4. **Week 4+: Advanced**
   - Explore other system guides
   - Create your own Blueprints
   - Contribute new guides!

---

## 📚 Related Documentation

- [Blueprint Image Guide](../reference/BLUEPRINT_IMAGE_GUIDE.md) - Visual documentation standard
- [Code Style Guide](../../CODE_STYLE.md) - C++ and Blueprint conventions
- [Architecture Document](../../ARCHITECTURE.md) - System design patterns
- [MVP Documentation](../mvp/README.md) - Trade Simulator focus

---

## 🔄 Recent Updates

### January 2026

- ✅ Created master template (`BLUEPRINT_GUIDE_TEMPLATE.md`)
- ✅ Established writing standards (`BLUEPRINT_GUIDE_STANDARDS.md`)
- ✅ Simplified Trading UI guide as example
- ⏳ Working on Station Actor guide
- ⏳ Planning Cargo System guide

### Coming Soon

- Trading Station Actor guide
- Ship Cargo Management guide
- Docking UI guide
- Market Display Widget guide

---

**Questions about Blueprint documentation?**
See the [Blueprint Guide Standards](../reference/BLUEPRINT_GUIDE_STANDARDS.md) or open an issue.

---

**Last Updated**: 2026-01-03
**Maintained by**: Adastrea Development Team
