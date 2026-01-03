# Blueprint Guide Migration Guide

> **How to update existing Blueprint guides to the new standard**

**Last Updated**: 2026-01-03  
**For**: Blueprint guide authors and maintainers

---

## 📋 Purpose

This guide helps you update existing Blueprint guides to follow the new standardized format established in January 2026.

**Goal**: Transform inconsistent, hard-to-follow guides into clear, step-by-step instructions.

---

## 🎯 Migration Priority

### High Priority (Do First)

**MVP-Critical Guides** - Essential for Trade Simulator demo:

1. ✅ **Trading UI Widget** - Already updated
2. ⏳ **Trading Station Actor** - Next up
3. ⏳ **Ship Cargo Management** - After station
4. ⏳ **Docking System UI** - After cargo
5. ⏳ **Market Display Widget** - After docking

### Medium Priority

**Frequently-Used Guides**:
- Ship HUD systems
- Station building guides
- Data Asset creation guides
- Input system guides

### Low Priority

**Advanced/Experimental**:
- AI trading systems
- Procedural generation
- Advanced UI patterns
- Deprecated features

---

## 🔄 Migration Process

### Step 1: Assess the Current Guide

**Read the existing guide and identify**:

- [ ] What's the main Blueprint being created?
- [ ] What type is it? (Actor/Widget/Component/Data Asset)
- [ ] What's the complexity level? (Beginner/Intermediate/Advanced)
- [ ] What C++ classes does it use?
- [ ] What other Blueprints does it depend on?
- [ ] What variables does it create? (List ALL of them)
- [ ] What functions does it create? (List ALL of them)
- [ ] What's missing? (Prerequisites, testing, troubleshooting?)

**Take notes** - You'll use these in the new guide.

---

### Step 2: Create New Guide File

**Don't overwrite the original yet!**

1. Copy the template:
   ```bash
   cp docs/reference/BLUEPRINT_GUIDE_TEMPLATE.md \
      Blueprints/[BlueprintName]Guide_SIMPLIFIED.md
   ```

2. Open both files side-by-side:
   - Original guide (for content)
   - New template (for structure)

---

### Step 3: Fill in the Template

#### 3.1 Title and Metadata

**From original**:
```markdown
# Trading System - Blueprint Integration Guide
## Overview
This guide shows how to integrate...
```

**To new format**:
```markdown
# Trading UI Widget - Blueprint Guide

Blueprint Type: Widget Blueprint
Complexity: Intermediate
Time to Complete: 45-60 minutes
Last Updated: 2026-01-03
```

**How**:
- Extract the specific Blueprint name
- Determine type (Actor/Widget/Component)
- Estimate complexity based on original length/detail
- Estimate time (test if possible)

---

#### 3.2 Overview Section

**Extract key sentences** from original introduction.

**Keep it brief**: 2-3 sentences maximum.

**From original** (too long):
```markdown
This comprehensive guide walks you through the process of creating 
a sophisticated trading interface that includes dynamic price updates, 
real-time inventory management, advanced filtering capabilities...
[continues for 5+ sentences]
```

**To new format** (concise):
```markdown
### What You'll Build

A trading UI that allows players to buy items from a space station's 
market and sell items from their cargo hold. The interface shows 
available items, prices, player credits, and cargo space.
```

---

#### 3.3 Prerequisites

**From original guide, extract what's needed first**.

**Look for mentions like**:
- "You need to have X compiled"
- "Create Y Data Asset first"
- "This assumes you have Z"

**Convert to checklist**:
```markdown
## ✅ Prerequisites

Before starting, ensure you have:

- [ ] `UTradingInterfaceWidget` C++ class compiled
- [ ] At least one `UMarketDataAsset` created
- [ ] At least three `UTradeItemDataAsset` created
- [ ] Unreal Engine 5.6+

**If you're missing any prerequisites**, see:
- [Link to how to get it]
```

---

#### 3.4 Dependencies

**Scan original guide for**:
- C++ classes mentioned
- Other Blueprints referenced
- Parent classes used

**Create tables**:
```markdown
## 🔗 Dependencies

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `UTradingInterfaceWidget` | TradingSystem | Trading functions |

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `BP_Station` | Hosts trading terminal | `Content/Stations/` |
```

---

#### 3.5 Variables to Create (CRITICAL!)

**This is the most important section!**

**Scan original guide** for ANY mention of variables:
- "Create a variable called..."
- "Add a variable..."
- "You'll need a variable..."

**List them ALL upfront**:

```markdown
## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: CurrentMarket

- **Type**: `Market Data Asset` (Object Reference)
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "The market currently being traded with"
- **Default Value**: None

### Variable 2: PlayerCredits

[Continue for ALL variables]
```

**Tips**:
- Don't skip any variables
- Include type, category, editability
- Add tooltips to explain purpose
- Note default values if any

---

#### 3.6 Functions to Create (ALSO CRITICAL!)

**Scan original guide** for custom functions:
- "Create a function..."
- "Add a function called..."
- Custom events

**List them ALL upfront**:

```markdown
## 🔧 Functions to Create

**Create these functions BEFORE implementing logic:**

### Function 1: RefreshItemList

**Purpose**: Updates the UI to show current market inventory

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 3](#step-3-implement-refreshitemlist)

[Continue for ALL functions]
```

**Link to actual implementation** step.

---

#### 3.7 Step-by-Step Implementation

**Convert original content** into numbered, sequential steps.

**Each step needs**:
1. Clear goal
2. Blueprint nodes (diagram or text)
3. Detailed instructions
4. Expected result

**From original** (unclear):
```markdown
Add some nodes to handle the transaction. Connect them properly.
```

**To new format** (specific):
```markdown
### Step 5: Implement Trade Execution

**Goal**: Complete the buy/sell transaction

**Blueprint Nodes**:

[Diagram or flowchart]

**Detailed Instructions**:

1. Select `Button_Trade` and add **OnClicked** event
2. Add node: `Is Valid` (Target: `SelectedItem`)
3. Add node: `Branch`
4. If True path:
   - Call `Execute Trade` (from C++ parent)
   - Check return value
5. If successful:
   - Call `RefreshItemList`
   - Play success sound

**Expected Result**: Clicking trade button executes the transaction 
and updates the UI.
```

**Tips**:
- Keep steps focused (one goal per step)
- Number every instruction
- State expected results
- Add validation steps

---

#### 3.8 Testing

**Original guides often skip this!**

**Create specific tests**:

```markdown
## 🧪 Testing Your Blueprint

### Test 1: UI Opens and Shows Data

**What to Test**: Widget displays correctly

**Steps**:
1. Create test map
2. Add trigger to open UI
3. Set CurrentMarket variable
4. Play in editor
5. Trigger UI

**Success Criteria**: 
- UI opens without errors
- Items are visible
- Credits display

**If it fails**: See [Troubleshooting](#issue-name)
```

**Create 3-5 tests** covering:
- Basic functionality
- Edge cases
- Error conditions

---

#### 3.9 Troubleshooting

**Look in original guide** for "Common Issues", "FAQ", "Problems", etc.

**Add any issues YOU encountered** while testing.

**Format consistently**:

```markdown
## 🐛 Troubleshooting

### Common Issue 1: [Problem Name]

**Symptoms**:
- [What user sees]
- [Error messages]

**Causes**:
- [Possible cause 1]
- [Possible cause 2]

**Solutions**:
1. [Step to fix]
2. [Step to fix]
3. [Step to fix]
```

**Include 3-5 common issues** minimum.

---

### Step 4: Test the New Guide

**CRITICAL**: Have someone follow the new guide!

**Testing checklist**:

- [ ] Can they complete it without getting stuck?
- [ ] Are prerequisites accurate?
- [ ] Are all variables listed upfront?
- [ ] Are all functions listed upfront?
- [ ] Do the steps work sequentially?
- [ ] Do expected results match reality?
- [ ] Do tests catch issues?
- [ ] Does troubleshooting help?

**Fix issues** found during testing.

---

### Step 5: Replace or Deprecate Original

**Option A: Replace** (recommended for high-priority):
1. Review new guide thoroughly
2. Rename original: `[Name]Guide_LEGACY.md`
3. Rename new: `[Name]Guide.md`
4. Update all links to point to new guide

**Option B: Keep Both** (for complex guides):
1. Keep original as `[Name]Guide.md`
2. Add new as `[Name]Guide_SIMPLIFIED.md`
3. Add note to original pointing to simplified version
4. Eventually replace when confident

---

## 📝 Migration Checklist

**For each guide you migrate**:

### Content
- [ ] Title and metadata filled in
- [ ] Overview is clear and concise (2-3 sentences)
- [ ] Prerequisites listed with checkboxes
- [ ] Dependencies in table format
- [ ] ALL variables listed upfront
- [ ] ALL functions listed upfront
- [ ] Widget layout (if UI Blueprint)
- [ ] Step-by-step numbered instructions
- [ ] Each step has goal and expected result
- [ ] Testing section with specific tests
- [ ] Troubleshooting with common issues

### Quality
- [ ] Language is beginner-friendly
- [ ] No jargon without explanation
- [ ] Consistent terminology
- [ ] Active voice used
- [ ] Links work
- [ ] File paths are correct

### Validation
- [ ] Tested by someone who didn't write it
- [ ] All steps work as written
- [ ] Expected results are accurate
- [ ] Tests catch real issues
- [ ] Troubleshooting helps

---

## 🛠️ Tools for Migration

### Text Processing

**Extract variables** from old guide:
```bash
grep -i "variable" OldGuide.md | grep -i "create\|add"
```

**Extract functions**:
```bash
grep -i "function\|event" OldGuide.md | grep -i "create\|add"
```

### Side-by-Side Editing

**Use a diff tool** to compare:
- Original guide (left)
- Template (right)
- Fill template with original content

**Recommended editors**:
- VS Code (split view)
- Sublime Text (split view)
- Any editor with dual pane support

---

## 📊 Progress Tracking

### Create Migration Tracker

**In Blueprints/README.md**, maintain status:

```markdown
| Guide | Status | Assignee | Notes |
|-------|--------|----------|-------|
| Trading UI | ✅ Updated | Team | Complete |
| Station Actor | 🔄 In Progress | John | 50% done |
| Ship Cargo | ⏳ Planned | - | Next up |
```

### Update Regularly

**After each guide migration**:
1. Update status in README
2. Add to CHANGELOG.md
3. Notify team in Discord/Slack
4. Link new guide in relevant docs

---

## 💡 Tips for Success

### Do's ✅

- **Start with simple guides** to practice the process
- **Test thoroughly** before marking complete
- **Get feedback** from actual users
- **Iterate** based on feedback
- **Update incrementally** don't try to do all at once

### Don'ts ❌

- **Don't rush** quality over speed
- **Don't skip testing** untested guides are useless
- **Don't lose content** keep originals until confident
- **Don't work alone** get reviews from others
- **Don't forget to update links** in other docs

---

## 📚 Resources

**Templates and Standards**:
- [Blueprint Guide Template](BLUEPRINT_GUIDE_TEMPLATE.md)
- [Blueprint Guide Standards](BLUEPRINT_GUIDE_STANDARDS.md)
- [Quick Reference](BLUEPRINT_GUIDE_QUICK_REFERENCE.md)

**Examples**:
- [Trading UI Simplified](../../Blueprints/TradingSystemBlueprintGuide_SIMPLIFIED.md)
- [Original Trading Guide](../../Blueprints/TradingSystemBlueprintGuide.md)

**Related**:
- [Blueprint Image Guide](BLUEPRINT_IMAGE_GUIDE.md)
- [Blueprints README](../../Blueprints/README.md)

---

## 🎯 Success Criteria

A migration is successful when:

1. **New guide is clearer** than original
2. **Beginners can follow it** without getting stuck
3. **All requirements upfront** no surprises
4. **Testing works** catches issues
5. **Feedback is positive** from users
6. **Consistent with template** follows standard

---

## 🔄 Example Migration

### Before: Trading System Guide (Original)

**Issues**:
- 560 lines, overwhelming
- Variables mentioned throughout
- Functions created inline
- No clear structure
- Limited testing
- Minimal troubleshooting

### After: Trading UI Guide (Simplified)

**Improvements**:
- Clear structure with template
- All variables listed upfront
- All functions listed upfront
- Numbered steps with goals
- Specific tests
- Comprehensive troubleshooting
- Beginner-friendly

**Time to Migrate**: ~4 hours (including testing)

---

## 📞 Need Help?

**Stuck on migration?**
- Review the [Blueprint Guide Standards](BLUEPRINT_GUIDE_STANDARDS.md)
- Check the [Trading UI example](../../Blueprints/TradingSystemBlueprintGuide_SIMPLIFIED.md)
- Ask in Discord/GitHub with specific questions
- Tag it with `documentation` label

---

**Migration Guide Version**: 1.0  
**Created**: 2026-01-03  
**Maintained by**: Adastrea Documentation Team
