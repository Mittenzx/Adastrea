# [Blueprint Name] - Blueprint Guide

> **Standard Blueprint Guide Template for Adastrea**  
> Copy this template to create new Blueprint guides with consistent structure.

**Blueprint Type**: [Actor / Widget / Component / Data Asset]  
**Complexity**: [Beginner / Intermediate / Advanced]  
**Time to Complete**: [Estimated time in minutes]  
**Last Updated**: [Date]

---

## 📋 Overview

### What You'll Build

[2-3 sentence description of what this Blueprint does and why it's needed]

**End Result**: [Describe what the player/user will experience]

### Why This Blueprint Matters

[1-2 sentences explaining how this fits into the game/MVP]

---

## ✅ Prerequisites

Before starting, ensure you have:

- [ ] [Required C++ class compiled] (e.g., `UTradingInterfaceWidget` in TradingSystem module)
- [ ] [Required Data Assets created] (e.g., `DA_Market_Station01`)
- [ ] [Required parent Blueprints] (e.g., `BP_PlayerShip`)
- [ ] [Engine version requirement] (e.g., UE 5.6+)

**If you're missing any prerequisites**, see:
- [Link to relevant setup guide]
- [Link to parent Blueprint guide]

---

## 🔗 Dependencies

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `BP_Example1` | [What it provides] | `Content/Path/To/BP` |
| `WBP_Example2` | [What it provides] | `Content/UI/` |

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `UExampleWidget` | ExampleModule | [Key functions available] |
| `AExampleActor` | CoreModule | [Key functionality] |

---

## 🎯 Setup Checklist

### Step 1: Create the Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/[YourFolder]/`
3. **Right-click** → [Blueprint Type] → [Specific Type]
4. **Choose Parent Class**: `[ParentClassName]`
5. **Name it**: `[BP_ExactName]`
6. **Double-click** to open the Blueprint Editor

### Step 2: Configure Class Settings

1. Click **Class Settings** (toolbar button)
2. Set **Parent Class**: `[C++ClassName]` (if not already set)
3. Under **Class Options**:
   - [ ] Enable [specific option if needed]
   - [ ] Set [specific property if needed]

---

## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: [VariableName]

- **Type**: [Type] (e.g., `MarketDataAsset Reference`)
- **Category**: `[CategoryName]`
- **Instance Editable**: [Yes/No]
- **Tooltip**: "[Description of what this stores]"
- **Default Value**: [If applicable]

### Variable 2: [VariableName]

- **Type**: [Type]
- **Category**: `[CategoryName]`
- **Instance Editable**: [Yes/No]
- **Tooltip**: "[Description]"

[Repeat for all required variables]

**Quick Copy Template:**
```
Name: [VariableName]
Type: [DataType]
Category: [Category]
Editable: [Yes/No]
Tooltip: [Description]
```

---

## 🔧 Functions to Create

**Create these functions BEFORE implementing the main logic:**

### Function 1: [FunctionName]

**Purpose**: [What this function does]

**Inputs**:
- `[InputName]` ([Type]) - [Description]
- `[InputName2]` ([Type]) - [Description]

**Outputs**:
- `[ReturnName]` ([Type]) - [Description]

**Implementation**: See [Step X](#step-x-implement-functionname) below

### Function 2: [FunctionName]

[Repeat structure]

---

## 📐 Widget Layout (For UI Blueprints Only)

**Delete this section if not a Widget Blueprint**

### Visual Hierarchy

```
Canvas Panel (Root)
├── Background Panel
│   ├── Image (Background)
│   └── Border (Frame)
├── Main Content
│   ├── Vertical Box
│   │   ├── Header (Text)
│   │   ├── Content Area
│   │   │   └── [Specific widgets]
│   │   └── Footer (Buttons)
│   └── Status Text
└── Overlay (Popups)
```

### Key Widget Names (Important!)

| Widget | Name in Hierarchy | Bound Variable |
|--------|-------------------|----------------|
| [Widget type] | `WidgetName` | `VariableName` |
| [Widget type] | `WidgetName2` | `VariableName2` |

**Why Names Matter**: You'll reference these in Blueprint code by name.

---

## 🛠️ Step-by-Step Implementation

### Step 1: [First Major Step]

**Goal**: [What you'll accomplish in this step]

**Blueprint Nodes**:

![Step 1 Blueprint Flow](../reference/images/blueprints/[filename].svg)

**Instructions**:

1. In the **Event Graph**, locate `Event [EventName]`
2. **Drag from execution pin** and add `[NodeName]`
3. **Connect**:
   - Input: [Description]
   - Output: [Description]
4. **Set parameters**:
   - [Parameter1]: [Value]
   - [Parameter2]: [Value]

**Expected Result**: [What you should see/test]

---

### Step 2: [Second Major Step]

**Goal**: [What you'll accomplish]

**Blueprint Nodes**:

```
[Event/Function Name]
  ↓
[Node 1: Description]
  ├─ Input: [What goes in]
  └─ Output: [What comes out]
  ↓
[Node 2: Description]
  ├─ Input: [What goes in]
  └─ Output: [What comes out]
  ↓
[Final Node: Description]
  └─ Result: [End state]
```

**Detailed Instructions**:

1. [Specific action]
2. [Specific action]
3. [Specific action]

**Validation**: [How to check it works]

---

### Step 3: [Third Major Step]

[Continue pattern for all major steps]

---

## 🧪 Testing Your Blueprint

### Test 1: [Test Name]

**What to Test**: [Specific functionality]

**Steps**:
1. [Action to take]
2. [Action to take]
3. [Expected result]

**Success Criteria**: [What you should see]

**If it fails**: See [Troubleshooting Section](#common-issue-1)

---

### Test 2: [Test Name]

[Repeat structure]

---

## 🐛 Troubleshooting

### Common Issue 1: [Problem Description]

**Symptoms**:
- [What you see]
- [Error messages]

**Causes**:
- [Possible cause 1]
- [Possible cause 2]

**Solutions**:
1. [Solution step 1]
2. [Solution step 2]
3. [Solution step 3]

---

### Common Issue 2: [Problem Description]

[Repeat structure]

---

## ✨ Optional Enhancements

**After the basic Blueprint works, consider adding:**

1. **[Enhancement Name]**
   - What it adds: [Description]
   - Complexity: [Simple/Medium/Complex]
   - Guide: [Link to advanced guide or brief instructions]

2. **[Enhancement Name]**
   - [Repeat structure]

---

## 📚 Related Guides

**If you want to learn more:**

- [Guide 1 Name] - [What it covers] (`docs/path/to/guide.md`)
- [Guide 2 Name] - [What it covers] (`docs/path/to/guide.md`)
- [API Reference] - [C++ class documentation] (`docs/reference/API.md`)

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ [Key concept 1]
- ✅ [Key concept 2]
- ✅ [Key concept 3]

**Next Recommended Blueprint**: [Suggestion for natural next step]

---

## 📝 Notes for Advanced Users

**Customization Points:**
- [Where you can modify behavior]
- [What variables control what]
- [Performance considerations]

**Blueprint Best Practices Applied:**
- [Specific best practice used]
- [Why certain approach was chosen]

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | [Date] | Initial guide |
| 1.1 | [Date] | [What changed] |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review the [Blueprint Image Guide](BLUEPRINT_IMAGE_GUIDE.md) for node documentation
- See [Troubleshooting Guide](../development/TROUBLESHOOTING.md)

---

**Template Version**: 1.0  
**Created**: 2026-01-03  
**Maintained by**: Adastrea Development Team
