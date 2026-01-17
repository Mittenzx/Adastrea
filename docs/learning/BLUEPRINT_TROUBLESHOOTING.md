# Blueprint Troubleshooting Guide

> **Common Blueprint problems and how to fix them**

**Last Updated**: January 17, 2026  
**Purpose**: Diagnose and fix common Blueprint issues  
**Audience**: All Blueprint developers

---

## 🎯 Purpose

This guide helps you identify, diagnose, and fix common Blueprint problems. Each issue includes:
- ❌ The problem (what you see)
- 🔍 The cause (why it happens)
- ✅ The solution (how to fix it)
- 💡 Prevention tips

---

## 📚 Problem Categories

1. [Compilation Errors](#compilation-errors)
2. [Execution Problems](#execution-problems)
3. [Variable Issues](#variable-issues)
4. [Reference Problems](#reference-problems)
5. [UI Problems](#ui-problems)
6. [Performance Issues](#performance-issues)
7. [Logic Errors](#logic-errors)

---

## Compilation Errors

### Problem 1: "Error: Accessed None"

**❌ What You See:**
- Red compilation error
- "Error: Accessed None" or "Accessed None trying to read property..."
- Blueprint won't compile

**🔍 Cause:**
- Trying to use a variable/reference that is null
- Object doesn't exist or hasn't been created yet

**✅ Solution:**

**Before (Wrong):**
```
Get Station Reference → Get Market Data
(If Station is null, crashes!)
```

**After (Correct):**
```
Get Station Reference
  → Is Valid?
    → Branch
      ├─ True → Get Market Data → Use It
      └─ False → Print "Station is null!" → Handle Error
```

**💡 Prevention:**
- **Always** validate references with `Is Valid?`
- Initialize variables in BeginPlay
- Check for null before using objects

---

### Problem 2: "Error: Variable X is not compatible"

**❌ What You See:**
- Red error on connection wire
- "Variable X is not compatible with Y"
- Can't connect pins

**🔍 Cause:**
- Trying to connect incompatible types
- Example: Float to Integer, String to Boolean

**✅ Solution:**

**Wrong:**
```
Get Player Credits (Float) ──→ Print String (String)
(Types don't match!)
```

**Correct:**
```
Get Player Credits (Float)
  → To String → Print String
```

**Common Conversions:**
- Float/Integer → String: Use "To String"
- String → Float/Integer: Use "String to Float/Int"
- Boolean → String: Use "Select" node or "Branch"
- Any → String: Usually auto-converts, but "To String" is explicit

**💡 Prevention:**
- Use proper variable types from the start
- Add conversion nodes between incompatible types
- Let Unreal auto-convert when possible (right-click wire)

---

### Problem 3: "Error: Missing Blueprint Parent Class"

**❌ What You See:**
- Red compilation error
- "Parent class X is missing"
- Blueprint can't open or compile

**🔍 Cause:**
- C++ parent class was renamed or deleted
- Plugin containing parent class is disabled

**✅ Solution:**

1. **Check if parent class exists:**
   - Look in C++ Source folder
   - Check if plugin is enabled

2. **If class was renamed:**
   - Reparent Blueprint to new class name
   - Right-click Blueprint → Asset Actions → Reparent Blueprint

3. **If class was deleted:**
   - Create new Blueprint with correct parent
   - Copy logic from old Blueprint

**💡 Prevention:**
- Don't delete C++ classes that have Blueprint children
- Document parent class dependencies
- Keep plugins enabled if Blueprints depend on them

---

## Execution Problems

### Problem 4: "Blueprint Logic Never Runs"

**❌ What You See:**
- Print String nodes don't show anything
- Actions never happen
- Blueprint appears "dead"

**🔍 Cause:**
- Execution pins not connected
- Event never triggers
- Actor not placed in level
- Blueprint not set as default class

**✅ Solution:**

**Check 1: Execution Pins Connected?**
```
❌ Wrong:
Event BeginPlay     [Print String]
(Not connected - execution stops!)

✅ Correct:
Event BeginPlay ──→ [Print String]
```

**Check 2: Is Event Triggering?**
```
Add debug print at start:

Event BeginPlay
  → Print "BeginPlay Called!"
  → (Your logic)
```

**Check 3: Actor in Level?**
- Look in Outliner (right panel)
- Make sure actor is spawned or placed

**Check 4: Correct Default Class?**
- Game Mode → Default Pawn Class set?
- Widget → Added to Viewport?

**💡 Prevention:**
- Always connect execution pins (white arrows)
- Test with Print String frequently
- Verify actor placement
- Check game mode settings

---

### Problem 5: "Event Runs Multiple Times Unexpectedly"

**❌ What You See:**
- Action happens twice (or more)
- Duplicated print messages
- Multiple UI elements created

**🔍 Cause:**
- Multiple instances of actor in level
- Event bound multiple times
- Multiple function calls

**✅ Solution:**

**Check 1: How Many Actors?**
- Open Outliner
- Search for your actor name
- Are there multiple copies?

**Check 2: Event Bound Multiple Times?**
```
Wrong:
Event Tick
  → Bind Event to OnClicked (binds 60x per second!)

Correct:
Event BeginPlay (only once)
  → Bind Event to OnClicked
```

**Check 3: Function Called Multiple Times?**
```
Use Print String with different colors to trace calls:

Function: MyFunction
  → Print "Function Called" (Red color)
```

**💡 Prevention:**
- Bind events once (in BeginPlay)
- Check how many actors exist
- Use "Do Once" node if action should only happen once

---

## Variable Issues

### Problem 6: "Variable Has Wrong Value"

**❌ What You See:**
- Variable is 0 when it should be 100
- Variable is empty string
- Variable is null/none

**🔍 Cause:**
- Variable not initialized
- Set before initialized
- Overwritten unexpectedly

**✅ Solution:**

**Solution 1: Set Default Value**
```
Select Variable → Details Panel → Default Value = 100
```

**Solution 2: Initialize in BeginPlay**
```
Event BeginPlay
  → Set PlayerCredits = 1000
  → Print PlayerCredits (verify it's set)
```

**Solution 3: Debug Where It Changes**
```
Set PlayerCredits = X
  → Get PlayerCredits
    → Print String (shows value immediately after set)
```

**💡 Prevention:**
- Always set default values
- Initialize variables in BeginPlay
- Print variable values after modifying them
- Use meaningful default values (not 0 or empty)

---

### Problem 7: "Variable Changes Unexpectedly"

**❌ What You See:**
- Variable has correct value, then suddenly changes
- Can't find where it's being modified

**🔍 Cause:**
- Multiple places modifying same variable
- Race condition (multiple updates happening)
- External system changing it

**✅ Solution:**

**Solution 1: Find All References**
- Right-click variable → Find References
- See everywhere it's used
- Look for unexpected "Set" nodes

**Solution 2: Add Debug Prints**
```
Every time you Set the variable:

Set PlayerCredits = X
  → Print "Credits set to X at [Location]"
    (Use different colors for different locations)
```

**Solution 3: Use Breakpoints**
- Click left edge of "Set Variable" node
- Red dot appears (breakpoint)
- Play in editor
- Game pauses when variable is set
- Examine call stack to see who called it

**💡 Prevention:**
- Minimize places that modify variables
- Use functions to modify variables (one place)
- Document why variable is changed

---

## Reference Problems

### Problem 8: "Can't Get Reference to Actor"

**❌ What You See:**
- "Get X" returns null/none
- Can't find actor in level
- Reference variable is empty

**🔍 Cause:**
- Actor doesn't exist in level
- Actor hasn't spawned yet
- Wrong actor class
- Searching in wrong way

**✅ Solution:**

**Solution 1: Get All Actors**
```
Get All Actors of Class (BP_TradingStation)
  → Get (Index 0) → First Station
  → Is Valid? → Check if exists
```

**Solution 2: Store Reference When Created**
```
Spawn Actor (BP_TradingStation)
  → Store in Variable: MyStation
  → (Now you have reliable reference)
```

**Solution 3: Cast from Overlap**
```
Event OnOverlapBegin (Other Actor)
  → Cast to BP_TradingStation
    → Success → Got reference!
    → Failure → Not the right type
```

**💡 Prevention:**
- Store references when actors spawn
- Use overlap/collision to detect actors
- Check if actor exists before using

---

### Problem 9: "Cast Failed"

**❌ What You See:**
- Cast node goes to "Cast Failed" pin
- Can't access functions on casted object

**🔍 Cause:**
- Object is not the type you're casting to
- Object is null
- Wrong parent class

**✅ Solution:**

**Check 1: Is Object Valid?**
```
Get Object
  → Is Valid?
    → Branch
      ├─ True → Try Cast
      └─ False → "Object is null!"
```

**Check 2: Is It Right Type?**
```
Get Object
  → Cast to BP_TradingStation
    ├─ Success → It's a trading station!
    └─ Failure → Not a trading station
        → Print Object Class Name (see what it actually is)
```

**Check 3: Check Inheritance**
- Is BP_TradingStation a child of ASpaceStation?
- Can only cast to parent or child classes

**💡 Prevention:**
- Validate object before casting
- Use "Get Class" to see actual type
- Check inheritance hierarchy

---

## UI Problems

### Problem 10: "Widget Not Showing on Screen"

**❌ What You See:**
- Widget created but not visible
- UI elements don't appear

**🔍 Cause:**
- Widget not added to viewport
- Z-Order issues (hidden behind other widgets)
- Widget off-screen
- Widget has 0 size or opacity

**✅ Solution:**

**Solution 1: Add to Viewport**
```
Create Widget (WBP_TradingUI)
  → Add to Viewport
    → Print "Widget added"
```

**Solution 2: Check Z-Order**
```
Add to Viewport (Z Order = 999)
(Higher numbers = on top)
```

**Solution 3: Check Visibility**
- Select widget in hierarchy
- Details → Is Visible = true
- Details → Render Opacity = 1.0

**Solution 4: Check Size**
- Widget might be 0x0 size
- Set explicit size or use "Size to Content"

**💡 Prevention:**
- Always "Add to Viewport" after creating widget
- Use high Z-Order for popups
- Test with visible colors/borders initially

---

### Problem 11: "Button Not Responding to Clicks"

**❌ What You See:**
- Clicking button does nothing
- OnClicked event never fires

**🔍 Cause:**
- Event not bound
- Button not "Is Enabled"
- Overlapping invisible widget blocking clicks
- Wrong collision/visibility settings

**✅ Solution:**

**Solution 1: Bind Event**
```
Event Construct (Widget)
  → Get Buy Button
    → Bind Event to OnClicked
      → Custom Function: OnBuyClicked
```

**Solution 2: Check Is Enabled**
```
Get Button
  → Set Is Enabled (true)
```

**Solution 3: Check Visibility**
- Button → Visibility = Visible (not "Not Hit-Testable")
- Make sure nothing is blocking button

**Solution 4: Debug Click**
```
Button → OnClicked
  → Print "Button Clicked!" (Red, large font)
```

**💡 Prevention:**
- Bind button events in "Event Construct"
- Verify button is enabled
- Test clicks with debug prints

---

## Performance Issues

### Problem 12: "Game Running Slow / Low FPS"

**❌ What You See:**
- Low frame rate (< 30 FPS)
- Stuttering
- Input lag

**🔍 Cause:**
- Event Tick doing too much
- Expensive operations every frame
- Too many actors
- Memory leaks

**✅ Solution:**

**Solution 1: Find Expensive Tick**
```
Event Tick
  → Print "Tick called" (will spam console - BAD!)
```

**Check if you're using Tick:**
- Open every Blueprint
- Search for "Event Tick"
- See what it's doing

**Solution 2: Replace Tick with Timer**
```
❌ Bad:
Event Tick
  → Update Economy Prices

✅ Good:
Event BeginPlay
  → Set Timer by Event (UpdatePrices, 60 sec, looping)

Custom Event: UpdatePrices
  → Update Economy Prices
```

**Solution 3: Profile Performance**
- Type `stat fps` in console
- Type `stat game` for detailed stats
- Find what's slow

**💡 Prevention:**
- **Avoid Event Tick entirely**
- Use timers for periodic updates
- Use events for changes
- Cache expensive calculations

---

### Problem 13: "Memory Leak / Increasing Memory Usage"

**❌ What You See:**
- Memory usage keeps growing
- Game eventually crashes
- Task Manager shows high RAM usage

**🔍 Cause:**
- Creating widgets/actors without destroying them
- Circular references preventing garbage collection
- Event bindings not cleared

**✅ Solution:**

**Solution 1: Destroy Unused Actors**
```
Spawn Actor → Store Reference
(When done)
→ Destroy Actor (Reference)
```

**Solution 2: Remove Unused Widgets**
```
Create Widget → Add to Viewport
(When done)
→ Remove from Parent
```

**Solution 3: Unbind Events**
```
When done with actor:
→ Unbind All Events
→ Destroy Actor
```

**💡 Prevention:**
- Destroy what you create
- Remove widgets when not needed
- Unbind events before destroying
- Use object pooling for frequently created objects

---

## Logic Errors

### Problem 14: "Wrong Execution Path Taken"

**❌ What You See:**
- Branch goes wrong direction
- Unexpected behavior
- Logic seems backwards

**🔍 Cause:**
- Wrong comparison operator
- Inverted logic
- Condition calculated incorrectly

**✅ Solution:**

**Solution 1: Debug Condition**
```
Get Condition Value
  → Print String (show true/false)
  → Branch
    ├─ True → Print "Taking True Path"
    └─ False → Print "Taking False Path"
```

**Solution 2: Check Comparison**
```
❌ Wrong:
PlayerCredits > ItemPrice
(backwards!)

✅ Correct:
PlayerCredits >= ItemPrice
(player needs at least price amount)
```

**Solution 3: Test Edge Cases**
- What if values are equal?
- What if one is 0?
- What if negative?

**💡 Prevention:**
- Print condition values before Branch
- Test all edge cases
- Use descriptive variable names
- Comment complex logic

---

### Problem 15: "Infinite Loop Crash"

**❌ What You See:**
- Unreal Editor freezes
- Must force close
- "Infinite loop detected" error

**🔍 Cause:**
- While Loop with condition that never becomes false
- Recursive function with no exit
- Missing loop break

**✅ Solution:**

**Dangerous Pattern:**
```
❌ Never do this:
While Loop
  Condition: true (always!)
  → (Never exits!)
```

**Safe Pattern:**
```
✅ Always have exit:
While Loop
  Condition: Count < 100
  → Increment Count
  → (Exits after 100 iterations)
```

**Add Safety:**
```
Variable: SafetyCounter = 0

While Loop (Condition)
  → Increment SafetyCounter
  → Branch (SafetyCounter > 1000)
    ├─ True → Break (emergency exit!)
    └─ False → Continue
```

**💡 Prevention:**
- Always have loop exit condition
- Add safety counter to while loops
- Use For Loop when possible (has built-in limit)
- Test loops with small numbers first

---

## Quick Diagnostic Checklist

When something goes wrong:

- [ ] **Compile** - Does it compile without errors?
- [ ] **Print String** - Add debug prints everywhere
- [ ] **Check Connections** - All execution pins connected?
- [ ] **Validate References** - Use Is Valid?
- [ ] **Check Variables** - Print their values
- [ ] **Test Events** - Are events triggering?
- [ ] **Look at Outliner** - How many actors exist?
- [ ] **Check Defaults** - Variables have default values?
- [ ] **Profile** - Check FPS and performance
- [ ] **Simplify** - Comment out sections to isolate problem

---

## Getting Help

### Before Asking for Help

1. **Try Everything Above** - Most problems have solutions here
2. **Create Minimal Example** - Isolate the problem
3. **Take Screenshots** - Show your Blueprint
4. **Document What You Tried** - List solutions attempted

### When Asking

**Include:**
- ✅ What you're trying to do
- ✅ What's happening instead
- ✅ Screenshot of Blueprint
- ✅ Error messages
- ✅ What you've tried

**Don't just say:**
- ❌ "It doesn't work"
- ❌ "Help"
- ❌ "Blueprint broken"

---

## Prevention Checklist

### Before Making Changes

- [ ] Save current working version
- [ ] Make one change at a time
- [ ] Test after each change
- [ ] Use source control (Git)

### While Building

- [ ] Add Print String nodes frequently
- [ ] Test in small steps
- [ ] Validate all references
- [ ] Initialize all variables
- [ ] Connect all execution pins

### After Completing

- [ ] Final test in PIE (Play In Editor)
- [ ] Check for warnings
- [ ] Remove debug Print Strings
- [ ] Document complex logic
- [ ] Commit to source control

---

## Common Error Messages Decoded

| Error Message | What It Means | Solution |
|--------------|---------------|----------|
| "Accessed None" | Using null reference | Add Is Valid? check |
| "Index out of bounds" | Array access beyond size | Check array length first |
| "Divide by zero" | Math error | Check divisor != 0 |
| "Infinite loop detected" | Loop never exits | Add loop exit condition |
| "Missing parent class" | C++ class gone | Check C++ code or plugin |
| "Type mismatch" | Wrong pin types | Add conversion node |

---

## Additional Resources

- [Blueprint Fundamentals](BLUEPRINT_FUNDAMENTALS.md) - Learn the basics
- [Blueprint Node Reference](BLUEPRINT_NODE_REFERENCE.md) - Node types explained
- [Blueprint Patterns](BLUEPRINT_PATTERNS.md) - Proven solutions
- [Unreal Documentation](https://docs.unrealengine.com/5.6/en-US/blueprints-visual-scripting-in-unreal-engine/)

---

**Remember**: Every Blueprint developer encounters these problems. The key is systematic debugging!

**When stuck**: Take a break, come back fresh, and try the checklist above.
