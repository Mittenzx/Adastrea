# Playtesting and Asset Verification Checklist

## Overview
This comprehensive checklist ensures that all faction system and station editor features are properly implemented, tested, and ready for production use. Use this document during development iterations and before releasing content to the team.

---

## Table of Contents
1. [Pre-Testing Setup](#pre-testing-setup)
2. [Faction Data Asset Verification](#faction-data-asset-verification)
3. [Station Editor Functionality](#station-editor-functionality)
4. [Faction Integration Testing](#faction-integration-testing)
5. [Blueprint Implementation Checks](#blueprint-implementation-checks)
6. [Performance and Optimization](#performance-and-optimization)
7. [Content Quality Assurance](#content-quality-assurance)
8. [Documentation and Handoff](#documentation-and-handoff)

---

## Pre-Testing Setup

### Environment Preparation

**Before starting any tests:**

- [ ] **Clean Build**
  - Close Unreal Engine
  - Delete `Intermediate`, `Saved`, and `Binaries` folders
  - Rebuild project from Visual Studio/Rider
  - Verify no compilation errors

- [ ] **Version Control**
  - Ensure working directory is clean
  - Create a new branch for testing (if needed)
  - Commit current work before destructive tests

- [ ] **Test Level Setup**
  - Create or load a test level: `Content/Maps/Test_StationEditor`
  - Place at least one Space Station actor
  - Add proper lighting for visual tests
  - Add a player start point

- [ ] **Debug Configuration**
  - Enable relevant log categories
  - Set log verbosity to Display or Verbose
  - Enable on-screen debug messages (if needed)

### Asset Preparation

- [ ] **Create Test Factions** (minimum 3)
  - One low-tech faction (Tech Level 1-3)
  - One mid-tech faction (Tech Level 5-6)
  - One high-tech faction (Tech Level 8-10)

- [ ] **Create Test Modules** (minimum 5 types)
  - Basic modules (Docking, Power, Storage)
  - Advanced modules (Processing, Defence)
  - One module per ModuleGroup enum value

- [ ] **Reference Documentation**
  - Have all documentation files open for cross-reference
  - Keep this checklist visible during testing

---

## Faction Data Asset Verification

### Asset Integrity Checks

**For EACH Faction Data Asset:**

#### Basic Properties
- [ ] **Faction Name**
  - Is not empty
  - Is unique across all factions
  - Contains no special characters (for display only)
  - Displays correctly in UI

- [ ] **Faction ID**
  - Is not empty or "None"
  - Is unique (no two factions share the same ID)
  - Uses valid naming convention (CamelCase or snake_case)
  - Contains no spaces or special characters
  - Matches exactly in all relationship references

- [ ] **Description**
  - Is between 2-4 sentences
  - Contains no typos or grammatical errors
  - Fits within UI text boxes without overflow
  - Accurately represents faction identity

- [ ] **Colors**
  - Primary Color is distinct and visible in space environment
  - Secondary Color contrasts well with primary
  - Colors are not identical to other factions
  - Colors display correctly on UI elements
  - Colors are visible against both light and dark backgrounds

#### Numerical Attributes
- [ ] **Initial Reputation**
  - Is between -100 and 100
  - Makes narrative sense (hostile factions should be negative)
  - Is balanced for gameplay

- [ ] **Technology Level**
  - Is between 1 and 10
  - Matches faction's narrative (advanced factions should be 7+)
  - Affects available modules correctly

- [ ] **Military Strength**
  - Is between 1 and 10
  - Aligns with faction description
  - Is balanced across all factions (not all 10s or all 1s)

- [ ] **Economic Power**
  - Is between 1 and 10
  - Makes sense with faction role (traders should be higher)
  - Is balanced for gameplay

- [ ] **Hostile By Default**
  - Matches Initial Reputation (hostile should be negative)
  - Is appropriate for faction type
  - Works correctly in gameplay

#### Trait System Verification

**For EACH Trait in the faction:**

- [ ] **Trait Name**
  - Is descriptive and clear
  - Contains no typos
  - Is unique within the faction

- [ ] **Trait ID**
  - Is not empty or "None"
  - Uses valid naming convention (CamelCase)
  - Can be used in Blueprint/C++ code
  - Is unique within the faction (can duplicate across factions)

- [ ] **Trait Description**
  - Clearly explains what the trait does
  - Specifies the modifier value and its effect
  - Contains no typos

- [ ] **Modifier Value**
  - Is an appropriate value (typically between -1.0 and 1.0)
  - Has correct sign (positive for bonus, negative for penalty)
  - Is balanced with other traits

**Trait System Function Tests:**

- [ ] `GetTraits()` returns all traits
- [ ] `HasTrait(TraitID)` correctly identifies traits
- [ ] `GetTraitByID(TraitID)` retrieves correct trait
- [ ] `GetTraitModifier(TraitID)` returns correct value
- [ ] Multiple traits with same ID stack modifiers correctly

#### Diplomacy System Verification

**For EACH Relationship in the faction:**

- [ ] **Target Faction ID**
  - Is not empty or "None"
  - Matches an actual faction's ID exactly (case-sensitive)
  - Target faction data asset exists

- [ ] **Relationship Value**
  - Is between -100 and 100
  - Makes narrative sense
  - Matches alliance/war status logically

- [ ] **Alliance Status**
  - Is false when at war
  - Matches relationship value (should be positive if allied)
  - Makes narrative sense

- [ ] **War Status**
  - Is false when allied
  - Matches relationship value (should be negative if at war)
  - Makes narrative sense

- [ ] **Trade Modifier**
  - Is between 0.0 and 2.0
  - Is 0.0 if at war (no trade)
  - Is higher (>1.0) if allied
  - Makes economic sense

**Diplomacy Function Tests:**

- [ ] `GetRelationship(FactionID)` retrieves correct data
- [ ] `IsAlliedWith(FactionID)` returns correct boolean
- [ ] `IsAtWarWith(FactionID)` returns correct boolean
- [ ] `GetRelationshipValue(FactionID)` returns correct value
- [ ] `GetTradeModifier(FactionID)` returns correct multiplier
- [ ] Non-existent faction IDs return appropriate defaults

#### Cross-Faction Validation

- [ ] **No Duplicate Faction IDs** across all assets
- [ ] **Relationship symmetry** (if desired)
  - If Faction A allies Faction B, does B ally A?
  - Document intentional asymmetric relationships
- [ ] **Alliance networks make sense**
  - No faction allied with everyone
  - Reasonable number of allies (2-4 typically)
- [ ] **Conflict networks make sense**
  - Wars have clear reasons in descriptions
  - Not everyone at war with everyone
- [ ] **Color uniqueness** - all factions visually distinct

---

## Station Editor Functionality

### C++ API Testing

**Core Station Functions:**

Test with a placed Space Station actor:

- [ ] `GetFaction()` returns assigned faction (or null if none)
- [ ] `SetFaction(NewFaction)` correctly assigns faction
- [ ] `AddModule(Module)` adds module to station
- [ ] `AddModuleAtLocation(Module, Location)` adds at specific position
- [ ] `RemoveModule(Module)` removes module correctly
- [ ] `MoveModule(Module, NewLocation)` repositions module
- [ ] `GetModules()` returns correct module list
- [ ] `GetModulesByType(Type)` filters correctly
- [ ] `GetModuleCount()` returns accurate count

**Advanced Station Functions:**

- [ ] `HasFactionTrait(TraitID)` returns correct result
- [ ] `GetFactionTraitModifier(TraitID)` returns correct value
- [ ] `IsAlliedWithFaction(FactionID)` works correctly
- [ ] `IsAtWarWithFaction(FactionID)` works correctly
- [ ] `GetFactionRelationship(FactionID)` returns correct value
- [ ] All functions handle null faction gracefully (no crashes)

**Module Functions:**

Test with a placed Space Station Module actor:

- [ ] `GetModuleFaction()` returns assigned faction (or null)
- [ ] `SetModuleFaction(NewFaction)` correctly assigns faction
- [ ] Module faction can differ from station faction
- [ ] Module properties (Type, Power, Group) are correct

### Station Editor Widget Testing

**Widget Initialization:**

- [ ] Widget extends `UStationEditorWidget` correctly
- [ ] `NativeConstruct` is called on creation
- [ ] Station reference can be set via `SetStation()`
- [ ] `CurrentStation` variable is accessible

**Module Addition:**

- [ ] `AddModule(ModuleClass, Location)` spawns module
- [ ] Module is added to station
- [ ] Module appears at correct location
- [ ] Returns valid module reference on success
- [ ] Returns nullptr on failure
- [ ] Handles invalid ModuleClass gracefully
- [ ] Handles null CurrentStation gracefully

**Module Removal:**

- [ ] `RemoveModule(Module)` removes from station
- [ ] Returns true on success
- [ ] Returns false on failure
- [ ] Handles invalid Module reference gracefully
- [ ] Handles null CurrentStation gracefully

**Module Movement:**

- [ ] `MoveModule(Module, NewLocation)` repositions module
- [ ] Returns true on success
- [ ] Returns false on failure
- [ ] Module ends at correct position
- [ ] Handles invalid references gracefully

**List Functions:**

- [ ] `GetAllModules()` returns current modules
- [ ] List updates after add/remove operations
- [ ] Returns empty array if no station set

**Validation Functions:**

- [ ] `IsValidPlacement(Location)` checks bounds
- [ ] Returns false for locations too far from station
- [ ] Can be extended in Blueprint

**Faction Integration:**

- [ ] `SetStationFaction(Faction)` assigns faction to current station
- [ ] `GetAvailableModuleTypes()` returns correct list
- [ ] Available types change with tech level
- [ ] `CanAddModuleForFaction(ModuleClass)` checks tech requirements
- [ ] Returns false for modules above tech level
- [ ] Returns true for allowed modules

---

## Faction Integration Testing

### Tech Level Restrictions

**Create three test scenarios:**

**Scenario 1: Low Tech Faction (Tech Level 3)**
- [ ] Can add basic modules (Docking, Power, Storage, Habitation, Connection)
- [ ] CANNOT add Processing modules (requires level 5)
- [ ] CANNOT add Defence modules (requires level 7)
- [ ] CANNOT add Public modules (requires level 7)
- [ ] `GetAvailableModuleTypes()` returns only basic types
- [ ] `CanAddModuleForFaction()` returns false for advanced modules

**Scenario 2: Mid Tech Faction (Tech Level 6)**
- [ ] Can add basic modules
- [ ] CAN add Processing modules
- [ ] CANNOT add Defence modules (requires level 7)
- [ ] CANNOT add Public modules (requires level 7)
- [ ] `GetAvailableModuleTypes()` includes Processing
- [ ] `CanAddModuleForFaction()` correctly validates each type

**Scenario 3: High Tech Faction (Tech Level 9)**
- [ ] Can add ALL module types
- [ ] `GetAvailableModuleTypes()` includes all types
- [ ] `CanAddModuleForFaction()` returns true for all modules

### Trait System Integration

**Test with faction that has multiple traits:**

- [ ] Station can access faction traits via `HasFactionTrait()`
- [ ] Trait modifiers are retrievable via `GetFactionTraitModifier()`
- [ ] Multiple traits with same ID stack correctly
- [ ] Traits with different IDs are independent
- [ ] Trait system works in Blueprint graphs
- [ ] Visual feedback shows active traits in UI

**Example Test Cases:**

1. **Trait: "EliteTraining" with modifier 0.25**
   - [ ] `HasFactionTrait("EliteTraining")` returns true
   - [ ] `GetFactionTraitModifier("EliteTraining")` returns 0.25
   - [ ] Can be used in Blueprint calculations

2. **Multiple traits with same ID**
   - [ ] Modifiers stack (0.25 + 0.15 = 0.40)
   - [ ] All traits are returned by `GetTraits()`

### Diplomacy System Integration

**Test with two stations of different factions:**

**Setup:**
- Station A: Faction "NovaVanguard"
- Station B: Faction "SolarisUnion"
- Relationship: Allied (value +60, alliance true)

**Tests:**
- [ ] Station A correctly identifies alliance with Station B faction
- [ ] `IsAlliedWithFaction("SolarisUnion")` returns true from Station A
- [ ] `GetFactionRelationship("SolarisUnion")` returns 60
- [ ] Trade modifier is correctly retrieved
- [ ] Relationships work bidirectionally (if defined in both factions)

**War Test:**
- Station A: Faction "NovaVanguard"
- Station B: Faction "CelestialSyndicate"
- Relationship: War (value -70, war true)

**Tests:**
- [ ] `IsAtWarWithFaction("CelestialSyndicate")` returns true
- [ ] `IsAlliedWithFaction("CelestialSyndicate")` returns false
- [ ] Trade modifier is 0.0 (or very low)
- [ ] Relationship value is negative

---

## Blueprint Implementation Checks

### Widget Blueprint Verification

**UI Layout:**
- [ ] All required widgets are present
- [ ] Layout is responsive to different resolutions
- [ ] Text is readable at all sizes
- [ ] Buttons are clickable and sized appropriately
- [ ] Scroll boxes work correctly

**Faction Selection UI:**
- [ ] Faction dropdown/combo box populates
- [ ] All factions appear in list
- [ ] Selecting a faction updates station
- [ ] Faction info panel displays correct data
- [ ] Colors update correctly
- [ ] Trait list displays all traits
- [ ] Relationship info is visible (if implemented)

**Module Palette:**
- [ ] All available module types are shown
- [ ] Disabled modules show tech requirement tooltips
- [ ] Module icons are correct for each type
- [ ] Clicking a module prepares it for placement

**Module List:**
- [ ] Displays all current modules
- [ ] Updates when modules added/removed
- [ ] Selecting a module highlights it
- [ ] Module details are correct
- [ ] Faction indicators show if module has custom faction

**Action Buttons:**
- [ ] Add module button works
- [ ] Remove module button works (when module selected)
- [ ] Move module functionality works
- [ ] Save button (if implemented)
- [ ] Cancel/Close button works

### Event Graph Verification

**Initialization:**
- [ ] `Event Construct` executes correctly
- [ ] Station reference is set
- [ ] UI is populated with initial data
- [ ] No errors in output log

**Module Addition Flow:**
- [ ] Button click triggers add logic
- [ ] Module class selection works
- [ ] Placement location is calculated
- [ ] Validation checks execute
- [ ] Module spawns correctly
- [ ] UI updates after addition
- [ ] Error messages show for invalid operations

**Module Removal Flow:**
- [ ] Selection from list works
- [ ] Confirmation dialog appears (if implemented)
- [ ] Module is removed from station
- [ ] UI updates after removal
- [ ] Error messages show for invalid operations

**Faction Selection Flow:**
- [ ] Dropdown selection triggers event
- [ ] Faction is assigned to station
- [ ] Info panel updates
- [ ] Available modules refresh
- [ ] No errors occur

### Blueprint Performance

- [ ] No excessive use of `Event Tick`
- [ ] Property bindings are not overused
- [ ] References are cached where appropriate
- [ ] Lists are updated incrementally (not rebuilt entirely)
- [ ] No frame rate drops in editor
- [ ] Widget loads quickly (<1 second)

---

## Performance and Optimization

### Runtime Performance Tests

**Test with varying numbers of modules:**

**Small Station (5-10 modules):**
- [ ] No frame rate impact
- [ ] UI updates instantly
- [ ] Module operations are smooth

**Medium Station (20-30 modules):**
- [ ] Frame rate remains stable (>30 FPS)
- [ ] UI updates within 1 frame
- [ ] Module list scrolls smoothly

**Large Station (50+ modules):**
- [ ] Frame rate acceptable (>20 FPS)
- [ ] UI remains responsive
- [ ] Consider optimizations if sluggish

### Memory Usage

- [ ] No memory leaks detected
- [ ] Widgets clean up when closed
- [ ] Preview modules are destroyed when not needed
- [ ] No orphaned actors in world outliner

### Content Size

- [ ] Faction data assets are small (<10 KB each)
- [ ] No unnecessary large textures in UI
- [ ] Module Blueprints are optimized
- [ ] Total content package is reasonable size

---

## Content Quality Assurance

### Visual Quality

**In-Editor:**
- [ ] All UI elements are properly aligned
- [ ] Fonts are consistent and readable
- [ ] Colors are pleasing and accessible
- [ ] Icons are clear and appropriate
- [ ] Spacing is consistent

**In-Game:**
- [ ] Station modules are visible
- [ ] Module attachments are correct
- [ ] Faction colors display on stations
- [ ] Preview modules show correct transparency
- [ ] Selection highlights are visible

### Audio (If Implemented)

- [ ] Button click sounds work
- [ ] Success/error notification sounds play
- [ ] Module placement sound plays
- [ ] No audio clipping or distortion

### Text and Localization

- [ ] All text is spelled correctly
- [ ] Grammar is correct
- [ ] Text fits in UI elements
- [ ] No text overflow or truncation
- [ ] Consistent terminology throughout
- [ ] Localization keys are used (if applicable)

### User Experience

**Ease of Use:**
- [ ] Controls are intuitive
- [ ] Feedback is clear (success/error)
- [ ] Tooltips are helpful
- [ ] Workflow is smooth (no awkward steps)
- [ ] Can accomplish tasks without documentation

**Error Handling:**
- [ ] Errors are caught gracefully
- [ ] Error messages are helpful
- [ ] No crashes from invalid input
- [ ] UI doesn't break from edge cases

**Accessibility:**
- [ ] Colors are distinguishable (colorblind friendly)
- [ ] Text is large enough to read
- [ ] Contrast is sufficient
- [ ] Important info isn't color-only

---

## Documentation and Handoff

### Documentation Completeness

- [ ] **README.md** is up to date
- [ ] **BlueprintFactionAssetGuide.md** is complete
- [ ] **BlueprintWorkflowTemplates.md** is complete
- [ ] **FactionSetupGuide.md** has all 10 faction templates
- [ ] **STATION_EDITOR_README.md** is accurate
- [ ] **Playtesting checklist** (this document) is available
- [ ] **Iteration guide** (next section) is complete
- [ ] **Designer onboarding guide** is ready

### Code Documentation

- [ ] All C++ functions have comments
- [ ] Header files have class descriptions
- [ ] Public functions have parameter descriptions
- [ ] Complex logic has inline comments
- [ ] Blueprint nodes have tooltips

### Example Assets

- [ ] At least 3 example faction data assets exist
- [ ] Example module Blueprints exist (one per type)
- [ ] Example station Blueprint exists
- [ ] Example editor widget Blueprint exists (optional)

### Team Handoff

- [ ] Schedule walkthrough with team
- [ ] Share documentation links
- [ ] Demonstrate key workflows
- [ ] Answer questions
- [ ] Collect feedback for iteration

---

## Final Checklist

Before marking the feature as complete:

### Core Functionality
- [ ] All C++ code compiles without errors
- [ ] All Blueprint graphs have no errors
- [ ] All tests in this document pass
- [ ] No critical bugs remain

### Content
- [ ] All required assets are created
- [ ] Assets are organized properly
- [ ] Naming conventions are followed
- [ ] No placeholder content remains

### Documentation
- [ ] All documentation is complete
- [ ] Documentation is accurate
- [ ] Examples are provided
- [ ] Quick reference guides are available

### Performance
- [ ] Performance is acceptable
- [ ] No memory leaks
- [ ] Optimizations are implemented
- [ ] Large-scale tests pass

### Quality
- [ ] Visual quality is acceptable
- [ ] User experience is smooth
- [ ] Error handling is robust
- [ ] Accessibility is considered

### Process
- [ ] Code is committed to version control
- [ ] PR is created (if applicable)
- [ ] Code review is completed
- [ ] Team is notified

---

## Issue Tracking

Use this section to track issues found during playtesting:

### Critical Issues (Must Fix)
- Issue #1: [Description]
  - Status: Open/In Progress/Resolved
  - Assigned: [Name]
- Issue #2: [Description]
  - Status: Open/In Progress/Resolved
  - Assigned: [Name]

### High Priority (Should Fix)
- Issue #1: [Description]
- Issue #2: [Description]

### Low Priority (Nice to Have)
- Issue #1: [Description]
- Issue #2: [Description]

### Known Limitations (Document)
- Limitation #1: [Description and workaround]
- Limitation #2: [Description and workaround]

---

## Regression Testing

After any fixes or changes, re-test:

- [ ] Previously failing tests now pass
- [ ] No new bugs introduced
- [ ] Performance not degraded
- [ ] Documentation updated if needed

---

## Sign-Off

**Tested By:** _________________  
**Date:** _________________  
**Version:** _________________  
**Result:** Pass / Fail / Pass with Issues  

**Notes:**
```
[Additional notes, observations, or recommendations]
```

---

## Appendix: Testing Scripts

### Quick Test Blueprint Graph

Create this in a test level Blueprint for rapid testing:

```
Event BeginPlay
  ↓
Get All Actors of Class (SpaceStation)
  ↓
ForEach Loop
    ↓
  Print Faction Name
    ↓
  Print Tech Level
    ↓
  Print Module Count
    ↓
  Test Has Trait ("EliteTraining")
    ↓
  Print Result
```

### Console Commands for Testing

```
// Show all faction data assets
DisplayAll FactionDataAsset

// Show all stations
DisplayAll SpaceStation

// Clear logs
CLS
```

---

Use this checklist throughout development and before each release to ensure quality! ✅
