# Blueprint Guide Writing Standards

> **How to write clear, consistent Blueprint guides for Adastrea**

**Last Updated**: 2026-01-03  
**Status**: Official Standard  
**Applies to**: All Blueprint documentation

---

## 📋 Purpose

This document explains:
- How to use the Blueprint Guide Template
- What makes a good Blueprint guide
- Standards for consistency across all guides
- Examples of good vs bad documentation

**Goal**: Every Blueprint guide should be easy to follow, even for complete beginners.

---

## 🎯 The Problem We're Solving

**Before standardization**, Blueprint guides had:
- ❌ Inconsistent structure (no clear pattern)
- ❌ Missing prerequisites (users didn't know what to set up first)
- ❌ Unclear dependencies (which other Blueprints are needed)
- ❌ No upfront variable/function list (users discovered requirements too late)
- ❌ Varying levels of detail (too complex or too brief)
- ❌ Hard to follow for beginners

**After standardization**, all guides will:
- ✅ Follow the same structure
- ✅ List prerequisites upfront
- ✅ Clearly state dependencies
- ✅ List all variables and functions to create BEFORE implementation
- ✅ Provide step-by-step instructions
- ✅ Include testing and troubleshooting
- ✅ Be accessible to beginners

---

## 📄 The Template Structure

### Mandatory Sections (Must Have)

Every Blueprint guide MUST include these sections in this order:

1. **Title and Metadata** - What this guide covers
2. **Overview** - What you'll build and why
3. **Prerequisites** - What you need before starting
4. **Dependencies** - Other Blueprints and C++ classes needed
5. **Setup Checklist** - How to create the Blueprint
6. **Variables to Create** - Complete list upfront
7. **Functions to Create** - Complete list upfront
8. **Step-by-Step Implementation** - Numbered instructions
9. **Testing** - How to verify it works
10. **Troubleshooting** - Common issues and solutions

### Optional Sections (Include if Relevant)

- **Widget Layout** - For UI Blueprints only
- **Optional Enhancements** - For advanced features
- **Related Guides** - Links to related documentation
- **Notes for Advanced Users** - Customization points

---

## 📝 How to Use the Template

### Step 1: Copy the Template

1. Open `docs/reference/BLUEPRINT_GUIDE_TEMPLATE.md`
2. Copy the entire contents
3. Create a new file in the appropriate location:
   - `Blueprints/` - For general Blueprint guides
   - `docs/mvp/` - For MVP-specific guides
   - `docs/setup/` - For initial setup guides
4. Name it: `[BlueprintName]_GUIDE.md` or `[BlueprintName]BlueprintGuide.md`

### Step 2: Fill in the Metadata

Replace the template placeholders:

```markdown
# [Blueprint Name] - Blueprint Guide
```

Becomes:

```markdown
# Trading UI Widget - Blueprint Guide
```

Fill in:
- **Blueprint Type**: Actor / Widget / Component / Data Asset
- **Complexity**: Beginner / Intermediate / Advanced
- **Time to Complete**: Realistic time estimate
- **Last Updated**: Today's date

### Step 3: Write the Overview

**Keep it brief** - 2-3 sentences maximum.

**Good Example**:
```markdown
### What You'll Build

A trading UI that allows players to buy items from a space station's market 
and sell items from their cargo hold. The interface shows available items, 
prices, player credits, and cargo space.
```

**Bad Example** (too long):
```markdown
### What You'll Build

In this comprehensive guide, we will walk through the process of creating 
a sophisticated trading interface that includes multiple features such as 
dynamic price updates, real-time inventory management, advanced filtering 
capabilities, and much more...
```

### Step 4: List Prerequisites FIRST

**Critical**: List everything the user needs BEFORE they start.

Use checkboxes so users can verify:

```markdown
- [x] `UTradingInterfaceWidget` C++ class compiled
- [x] At least one `UMarketDataAsset` created
- [ ] Blueprint parent class exists
```

**If a prerequisite is missing**, provide a link to how to get it:

```markdown
**If you're missing any prerequisites**, see:
- [Trading System Setup Guide](path/to/guide.md)
```

### Step 5: List Dependencies

Create tables for clarity:

```markdown
### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `UTradingInterfaceWidget` | TradingSystem | Trading functions |
```

### Step 6: List ALL Variables Upfront

**This is crucial** - users need to create variables BEFORE implementing logic.

Use this format for each variable:

```markdown
### Variable 1: CurrentMarket

- **Type**: `Market Data Asset` (Object Reference)
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "The market currently being traded with"
- **Default Value**: None
```

**Provide a quick-copy template**:

```markdown
Name: CurrentMarket
Type: Market Data Asset
Category: Trading
Editable: No
Tooltip: "The market currently being traded with"
```

### Step 7: List ALL Functions Upfront

List what functions to create before implementation:

```markdown
### Function 1: RefreshItemList

**Purpose**: Updates the UI to show current market inventory

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 3](#step-3-implement-refreshitemlist) below
```

Link to the actual implementation step.

### Step 8: Write Step-by-Step Instructions

**Each step should**:
- Have a clear goal
- Include a visual diagram (if possible)
- Provide numbered instructions
- State the expected result

**Good step structure**:

```markdown
### Step 3: Implement RefreshItemList

**Goal**: Populate the item list with market inventory

**Blueprint Nodes**:

[Include diagram or text flowchart]

**Detailed Instructions**:

1. Create a new function: `RefreshItemList`
2. Add node: `Clear Children`
3. Add node: `Get Available Items`
4. [Continue...]

**Expected Result**: The item list shows all market items
```

**Use text flowcharts** when SVG diagrams aren't available:

```markdown
Event Construct
  ↓
Get Player Controller
  ↓
Cast to [YourPlayerControllerClass]
  ↓
Get Player Credits
  └─ Set PlayerCredits (variable)
```

### Step 9: Write Testing Instructions

**Provide specific tests**:

```markdown
### Test 1: UI Opens and Shows Data

**What to Test**: Widget displays market items correctly

**Steps**:
1. Create a test map
2. Add a trigger
3. Play in editor
4. [Specific actions]

**Success Criteria**: 
- UI opens without errors
- Items are visible
- Credits display correctly

**If it fails**: See [Troubleshooting](#issue-name)
```

### Step 10: Write Troubleshooting Section

**Format issues clearly**:

```markdown
### Common Issue 1: Transaction Fails

**Symptoms**:
- Button does nothing
- Error message in log

**Causes**:
- Variable not set
- Data Asset is null

**Solutions**:
1. [Specific step]
2. [Specific step]
3. [Specific step]
```

---

## ✅ Quality Checklist

Before publishing a Blueprint guide, verify:

### Content Quality

- [ ] All sections from the template are included
- [ ] Prerequisites are listed upfront
- [ ] Dependencies are clearly stated
- [ ] ALL variables listed before implementation
- [ ] ALL functions listed before implementation
- [ ] Steps are numbered and sequential
- [ ] Each step has a clear goal
- [ ] Expected results are stated
- [ ] Testing instructions are specific
- [ ] Troubleshooting covers common issues

### Writing Quality

- [ ] Language is clear and beginner-friendly
- [ ] No jargon without explanation
- [ ] Consistent terminology throughout
- [ ] No overly long paragraphs
- [ ] Active voice used ("Click the button" not "The button should be clicked")
- [ ] Spelling and grammar checked

### Technical Accuracy

- [ ] All C++ class names are correct
- [ ] All Blueprint names follow naming conventions
- [ ] All node names match Unreal Engine exactly
- [ ] File paths are correct
- [ ] Links to other guides work

### Accessibility

- [ ] Suitable for complete beginners
- [ ] Complex concepts explained simply
- [ ] Visual aids included where helpful
- [ ] Code examples are copy-pasteable

---

## 🎨 Visual Standards

### Blueprint Node Diagrams

**Preferred**: SVG diagrams using the Blueprint image tool

```markdown
![Step 1 Flow](../reference/images/blueprints/trading_step1.svg)
```

**Alternative**: Text-based flowcharts

```markdown
Event Construct
  ↓
Get Player Controller
  ├─ Input: None
  └─ Output: Player Controller Reference
  ↓
Cast to BP_PlayerController
  ├─ Input: Object to cast
  └─ Output: Casted reference (or Invalid)
```

**Guidelines for text flowcharts**:
- Use `↓` for single execution flow
- Use `├─` and `└─` for branches
- Indent consistently
- Show inputs and outputs for clarity

### Widget Layouts

**Use hierarchical text structure**:

```markdown
Canvas Panel (Root)
├── Background Panel
│   ├── Image
│   └── Border
├── Main Content
│   └── Vertical Box
│       ├── Header
│       └── Content
```

### Tables

**Use tables for structured data**:

```markdown
| Variable | Type | Purpose |
|----------|------|---------|
| Name | Type | Description |
```

---

## 📊 Complexity Levels

### Beginner Guides

**Characteristics**:
- Assumes no Blueprint experience
- Explains every step in detail
- Includes screenshots/diagrams
- No assumed knowledge
- Time: 15-30 minutes

**Example**: "Create a Simple Button Widget"

### Intermediate Guides

**Characteristics**:
- Assumes basic Blueprint knowledge
- Explains complex nodes
- Some assumed knowledge (event graphs, variables)
- Time: 30-60 minutes

**Example**: "Trading UI Widget" (from template example)

### Advanced Guides

**Characteristics**:
- Assumes strong Blueprint knowledge
- Focuses on advanced patterns
- Minimal explanation of basics
- Time: 60+ minutes

**Example**: "AI Trader System with Dynamic Route Planning"

---

## 🔄 Updating Existing Guides

### Process for Updating

1. **Identify guide to update**
2. **Open the template** (`BLUEPRINT_GUIDE_TEMPLATE.md`)
3. **Create new file** with `_SIMPLIFIED` or `_V2` suffix
4. **Copy content** from old guide
5. **Restructure** to match template
6. **Fill in missing sections** (variables, functions, prerequisites)
7. **Test instructions** to ensure they work
8. **Replace old guide** or mark as deprecated

### Migration Priority

**High Priority** (Update First):
- MVP-critical guides (Trading, Ship Controls, UI)
- Frequently-used guides (Station creation, Data Assets)
- Guides with known issues or confusion

**Medium Priority**:
- System-specific guides (Combat, Faction, Personnel)
- Advanced features

**Low Priority**:
- Deprecated features
- Experimental systems

---

## 💡 Writing Tips

### Do's

- ✅ **Be specific**: "Click the 'Compile' button" not "Compile the Blueprint"
- ✅ **Use active voice**: "Create a variable" not "A variable should be created"
- ✅ **Number steps**: Helps users track progress
- ✅ **State expected results**: Users can verify they're on track
- ✅ **Link to related guides**: Help users find more info
- ✅ **Include time estimates**: Users can plan accordingly
- ✅ **Provide troubleshooting**: Anticipate common issues

### Don'ts

- ❌ **Don't assume knowledge**: Explain or link to explanations
- ❌ **Don't use jargon unnecessarily**: "Event Graph" not "EG"
- ❌ **Don't skip steps**: Every action should be listed
- ❌ **Don't use vague language**: "Somewhere around here" is not helpful
- ❌ **Don't write novels**: Keep it concise
- ❌ **Don't forget to test**: Instructions must actually work

---

## 📚 Examples

### Good Variable Documentation

```markdown
### Variable 1: PlayerCredits

- **Type**: `Integer`
- **Category**: `Player Info`
- **Instance Editable**: No
- **Tooltip**: "Player's current money available for trading"
- **Default Value**: `0` (set at runtime from player controller)

**Usage**: This variable stores the player's credits and is updated 
after each transaction. It's displayed in the UI and used to validate 
purchases.
```

### Bad Variable Documentation

```markdown
### PlayerCredits

Int variable for credits. Make it.
```

### Good Step Documentation

```markdown
### Step 3: Create Item List Display

**Goal**: Show all available items from the market

**Blueprint Nodes**:

[Diagram showing the node flow]

**Detailed Instructions**:

1. In the Event Graph, create a new custom function named `RefreshItemList`
2. Add a `Clear Children` node:
   - Target: `ItemListScrollBox` (drag from variable)
3. Add a `Get Available Items` node:
   - This is inherited from the C++ parent class
   - Returns an array of market items
4. Add a `ForEach Loop` node:
   - Connect the array from step 3 to the input
5. Inside the loop body:
   - Add `Create Widget` node
   - Class: Select `WBP_TradeItemRow`
   - Owning Player: `Get Player Controller`
6. Set item data on the created widget:
   - Call `SetItemName` (pass the item name from array element)
   - Call `SetPrice` (pass the current price)
7. Add the widget to the scroll box:
   - Call `Add Child` on `ItemListScrollBox`
   - Content: The widget created in step 5

**Expected Result**: When called, this function clears the list and 
repopulates it with all market items. You should see item names and 
prices displayed in the UI.

**To Test**: Call this function from Event Construct and check the list 
populates in PIE (Play in Editor).
```

### Bad Step Documentation

```markdown
### Step 3: Item List

Make a function that shows items. Use ForEach loop. Add widgets to scroll box.
```

---

## 🔧 Tools and Resources

### Blueprint Documentation Tools

- **SVG Generator**: `Tools/generate_blueprint_images.py`
- **Template**: `docs/reference/BLUEPRINT_GUIDE_TEMPLATE.md`
- **Image Guide**: `docs/reference/BLUEPRINT_IMAGE_GUIDE.md`
- **Beginner Guide**: `docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md`

### Unreal Engine References

- Blueprint Visual Scripting: [Unreal Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/blueprint-visual-scripting-in-unreal-engine)
- UMG UI Designer: [Unreal Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/umg-ui-designer-for-unreal-engine)

---

## 📋 Quick Reference: Template Sections

| Section | Purpose | Required? |
|---------|---------|-----------|
| Title & Metadata | Identify the guide | ✅ Yes |
| Overview | What you'll build | ✅ Yes |
| Prerequisites | What's needed first | ✅ Yes |
| Dependencies | Other Blueprints/classes | ✅ Yes |
| Setup Checklist | How to create the BP | ✅ Yes |
| Variables to Create | List ALL variables upfront | ✅ Yes |
| Functions to Create | List ALL functions upfront | ✅ Yes |
| Widget Layout | UI structure | ⚠️ UI only |
| Step-by-Step | Implementation instructions | ✅ Yes |
| Testing | How to verify | ✅ Yes |
| Troubleshooting | Common issues | ✅ Yes |
| Optional Enhancements | Advanced features | ⬜ Optional |
| Related Guides | Links to more info | ⬜ Optional |
| What You Learned | Summary | ⬜ Optional |
| Version History | Track changes | ⬜ Optional |

---

## 🎯 Success Criteria

A Blueprint guide is successful if:

1. **A beginner can follow it** without getting stuck
2. **Variables and functions are listed upfront** so users know what to create
3. **Every step has a clear goal** and expected result
4. **Testing instructions work** and catch issues
5. **Troubleshooting covers actual problems** users encounter
6. **The guide is consistent** with other guides in structure
7. **Links work** and references are accurate

---

## 🔄 Feedback and Iteration

### How to Improve Guides

1. **Watch beginners use the guide** - note where they get stuck
2. **Collect feedback** - ask what was confusing
3. **Update the guide** - clarify problem areas
4. **Test again** - verify improvements work

### Reporting Issues

If you find issues with a guide:
1. Open a GitHub issue
2. Tag it with `documentation`
3. Specify which guide and which step
4. Describe what was unclear
5. Suggest an improvement if possible

---

## 📞 Questions?

- **Template unclear?** See the example: `Blueprints/TradingSystemBlueprintGuide_SIMPLIFIED.md`
- **Need help with images?** See `docs/reference/BLUEPRINT_IMAGE_GUIDE.md`
- **General Blueprint help?** See `docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md`

---

**Last Updated**: 2026-01-03  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
