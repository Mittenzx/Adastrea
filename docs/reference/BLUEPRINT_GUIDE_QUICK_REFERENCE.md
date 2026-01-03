# Blueprint Guide Quick Reference

> **Quick comparison of old vs new Blueprint guide format**

**Last Updated**: 2026-01-03

---

## 🔄 Format Comparison

### Old Format (Legacy Guides)

```markdown
# Some Blueprint Guide

Here's how to make a thing...

Step 1: Create the Blueprint
- Do this
- Do that

Step 2: Add some functionality
- Oh by the way, you need to create a variable called X
- And a function called Y
- Connect these nodes...

[Continues with mixed instructions]
```

**Problems**:
- ❌ No prerequisites listed
- ❌ Variables created mid-implementation
- ❌ Functions created when needed
- ❌ Inconsistent structure
- ❌ Missing testing/troubleshooting

---

### New Format (2026 Standard)

```markdown
# [Blueprint Name] - Blueprint Guide

Complexity: Intermediate | Time: 45-60 min

## Overview
[What you'll build in 2-3 sentences]

## Prerequisites
- [ ] C++ class compiled
- [ ] Data Assets created
- [ ] Parent Blueprint exists

## Dependencies
[Table of required Blueprints and C++ classes]

## Variables to Create (UPFRONT!)
Variable 1: Name
- Type: [Type]
- Category: [Category]
- Purpose: [Why you need this]

Variable 2: Name
[Repeat for all variables]

## Functions to Create (UPFRONT!)
Function 1: Name
- Purpose: [What it does]
- Inputs: [List]
- Outputs: [List]

[Repeat for all functions]

## Step-by-Step Implementation
### Step 1: [Goal]
1. [Specific action]
2. [Specific action]
Expected Result: [What you should see]

### Step 2: [Goal]
[Continue...]

## Testing
Test 1: [What to test]
Steps: [How to test]
Success: [What success looks like]

## Troubleshooting
Issue 1: [Problem]
Symptoms: [What you see]
Solutions: [How to fix]
```

**Benefits**:
- ✅ Prerequisites checked upfront
- ✅ All variables created before implementation
- ✅ All functions created before implementation
- ✅ Consistent structure
- ✅ Testing and troubleshooting included
- ✅ Clear time estimates

---

## 📋 Template Sections Checklist

**Every guide MUST have**:

- [ ] **Title with metadata** (Type, Complexity, Time)
- [ ] **Overview** (What you'll build)
- [ ] **Prerequisites** (What's needed first)
- [ ] **Dependencies** (Other Blueprints/C++ classes)
- [ ] **Variables to Create** (Complete list upfront)
- [ ] **Functions to Create** (Complete list upfront)
- [ ] **Step-by-Step** (Numbered implementation)
- [ ] **Testing** (How to verify)
- [ ] **Troubleshooting** (Common issues)

**Optional sections**:
- Widget Layout (for UI Blueprints)
- Optional Enhancements (advanced features)
- Related Guides (links)
- Notes for Advanced Users

---

## 🎯 Key Innovation: Upfront Requirements

### Old Way ❌

```
Step 3: Create variable "PlayerCredits"
Step 7: Oh, also create "CurrentMarket" variable
Step 12: You'll need a "RefreshUI" function
```

**Problem**: User discovers requirements too late, has to backtrack.

### New Way ✅

```
## Variables to Create

Create these BEFORE starting implementation:

1. PlayerCredits
2. CurrentMarket
3. SelectedItem
[Complete list]

## Functions to Create

Create these BEFORE starting implementation:

1. RefreshUI
2. UpdatePlayerInfo
3. HandleTransaction
[Complete list]

## Step-by-Step Implementation

Now that you have all variables and functions...
Step 1: [Use the variables/functions you created]
```

**Benefit**: User creates everything upfront, no surprises.

---

## 📝 Writing Quick Tips

### Do's ✅

- **Be specific**: "Click the 'Compile' button in the toolbar"
- **Use active voice**: "Create a variable" not "A variable should be created"
- **Number steps**: Helps users track progress
- **State expected results**: "You should see items in the list"
- **Provide time estimates**: Users can plan accordingly

### Don'ts ❌

- **Don't assume knowledge**: Explain or link to explanations
- **Don't use jargon**: Or define it first
- **Don't skip steps**: Every action should be listed
- **Don't be vague**: "Configure the thing" is not helpful
- **Don't forget testing**: Users need to verify it works

---

## 🔍 Guide Status Symbols

When browsing Blueprint guides, look for these symbols:

| Symbol | Meaning |
|--------|---------|
| ✅ | **Updated** - Follows new template |
| 🔄 | **Being Updated** - Work in progress |
| ⚠️ | **Legacy** - Old format, still works |
| ⏳ | **Coming Soon** - Planned |
| 🗑️ | **Deprecated** - No longer recommended |

---

## 📚 Resources

**For Guide Authors**:
- [Blueprint Guide Template](BLUEPRINT_GUIDE_TEMPLATE.md) - Copy this
- [Blueprint Guide Standards](BLUEPRINT_GUIDE_STANDARDS.md) - How to write guides
- [Example: Trading UI](../../Blueprints/TradingSystemBlueprintGuide_SIMPLIFIED.md) - Reference

**For Guide Users**:
- [Blueprints README](../../Blueprints/README.md) - Navigate all guides
- [Blueprint Guide for Beginners](../development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md) - Learn basics
- [Blueprint Image Guide](BLUEPRINT_IMAGE_GUIDE.md) - Visual documentation

---

## 🎓 Quick Start for Authors

**Creating a new guide? Follow these steps:**

1. **Copy the template**:
   ```bash
   cp docs/reference/BLUEPRINT_GUIDE_TEMPLATE.md Blueprints/MyGuide.md
   ```

2. **Fill in metadata**:
   - Blueprint name
   - Type (Actor/Widget/Component)
   - Complexity level
   - Time estimate

3. **Write overview** (2-3 sentences)

4. **List prerequisites** (with checkboxes)

5. **List ALL variables upfront**

6. **List ALL functions upfront**

7. **Write step-by-step** (with goals and expected results)

8. **Add testing** (specific tests)

9. **Add troubleshooting** (common issues)

10. **Test your guide** (have someone follow it)

---

## 📊 Migration Status

### Phase 1: Template Creation ✅
- Master template created
- Standards document written
- Example guide completed

### Phase 2: High-Priority Updates 🔄
- [ ] Trading Station Actor
- [ ] Ship Cargo System
- [ ] Docking UI
- [ ] Market Display Widget

### Phase 3: System Guides ⏳
- [ ] Ship Systems
- [ ] Station Systems
- [ ] Combat guides
- [ ] Faction guides

---

## 💡 Example Comparison

### Old Trading UI Guide (Legacy)

**Structure**:
- Overview
- Quick Start
- Examples (mixed with implementation)
- Widget Examples
- Blueprint Actor Examples
- Integration Checklist
- Common Issues

**Issues**:
- Variables mentioned throughout
- No clear prerequisite list
- Functions created inline
- Hard to follow linearly

### New Trading UI Guide (Simplified)

**Structure**:
1. Overview (clear goal)
2. Prerequisites (checklist)
3. Dependencies (table)
4. Variables to Create (complete list)
5. Functions to Create (complete list)
6. Widget Layout (visual hierarchy)
7. Step-by-Step (numbered, sequential)
8. Testing (specific tests)
9. Troubleshooting (solutions)

**Benefits**:
- Linear progression
- All requirements upfront
- Easy to follow
- No surprises

---

## 🎯 Success Metrics

A Blueprint guide is successful if:

1. **Beginner can complete it** without getting stuck
2. **Variables listed upfront** so no backtracking
3. **Functions listed upfront** so no surprises
4. **Testing works** and catches issues
5. **Troubleshooting helps** when problems occur
6. **Structure is consistent** with other guides

---

**Questions?**  
See [Blueprint Guide Standards](BLUEPRINT_GUIDE_STANDARDS.md) for complete details.

---

**Quick Reference Version**: 1.0  
**Created**: 2026-01-03
