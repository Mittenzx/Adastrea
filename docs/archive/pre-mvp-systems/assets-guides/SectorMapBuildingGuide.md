# Sector Map Building Guide

A complete end-to-end tutorial for building a sector map from scratch in Adastrea.

## Table of Contents

- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Step 1: Plan Your Sector](#step-1-plan-your-sector)
- [Step 2: Create the Level Map](#step-2-create-the-level-map)
- [Step 3: Place the SpaceSectorMap Actor](#step-3-place-the-spacesectormap-actor)
- [Step 4: Create Sector Council Data Asset](#step-4-create-sector-council-data-asset)
- [Step 5: Populate Your Sector](#step-5-populate-your-sector)
- [Step 6: Test and Validate](#step-6-test-and-validate)
- [Common Issues and Troubleshooting](#common-issues-and-troubleshooting)
- [Next Steps and Related Documentation](#next-steps-and-related-documentation)

## Overview

This guide walks you through building a complete, playable sector from nothing. By the end, you'll have:

- A level map containing your sector
- A `ASpaceSectorMap` actor marking the 200km × 200km × 200km boundaries
- A `USectorCouncilDataAsset` defining governance, taxation, and policies
- Populated content (stations, ships, resources)
- A validated, tested sector ready for gameplay

### What is a Sector?

A sector in Adastrea is a 200km × 200km × 200km region of space (20,000,000 Unreal units per side). Each sector has:

| Component | Purpose |
|-----------|---------|
| **SpaceSectorMap Actor** | Physical marker defining boundaries in the level |
| **Sector Council Data Asset** | Governance configuration (taxes, budget, policies) |
| **Content** | Stations, ships, asteroids, and other objects |
| **Integration** | Connections to Way, Faction, Trading, and Quest systems |

### Time Estimate

| Experience Level | Time to Complete |
|------------------|------------------|
| Beginner | 60-90 minutes |
| Intermediate | 30-45 minutes |
| Advanced | 15-20 minutes |

## Prerequisites

Before starting, ensure you have:

- [ ] Unreal Engine 5.6 installed and project loaded
- [ ] Familiarity with Unreal Editor basics (navigating, placing actors)
- [ ] Understanding of Data Assets (see `Assets/DesignerOnboarding.md`)
- [ ] Access to at least one Way Data Asset (for council representatives)
- [ ] Decided on your sector's theme and purpose

### Recommended Reading

Review these guides for background knowledge (not required but helpful):

- `Assets/SectorCreationGuide.md` - Deep dive into council governance
- `Assets/SectorMapPlacementGuide.md` - Detailed placement strategies
- `Assets/SectorTemplates/README.md` - Template documentation

## Step 1: Plan Your Sector

Before opening the editor, plan your sector on paper or in a document.

### 1.1 Define Your Sector's Identity

Answer these questions:

| Question | Example Answer |
|----------|----------------|
| **What is the sector's name?** | "Epsilon Mining Reach" |
| **What is its primary purpose?** | Mining operations and ore processing |
| **What gameplay does it support?** | Mining quests, resource gathering, trading |
| **Who controls it?** | Mining Cooperative + Freight Haulers |
| **What is the danger level?** | Medium-high (pirate raids) |
| **What makes it unique?** | Rich asteroid belts, rare ore deposits |

### 1.2 Choose a Template or Create Custom

Select a starting point from `Assets/SectorTemplates/`:

| Template | Best For |
|----------|----------|
| `CoreSystemsSector.yaml` | Safe zones, tutorial areas |
| `FrontierMiningSector.yaml` | Mining, risk/reward |
| `TradeHubSector.yaml` | Commerce, economic focus |
| `ResearchSector.yaml` | Technology, innovation |
| `MilitaryBorderSector.yaml` | Combat, military missions |
| `IndependentColoniesSector.yaml` | Frontier, minimal governance |
| `ContestedTerritorySector.yaml` | Political intrigue, diplomacy |

For our example, we'll use the **Frontier Mining Sector** template as a base.

### 1.3 Determine Sector Coordinates

Plan where your sector fits in the universe:

```
Coordinate System:
- Sectors are placed at multiples of 20,000,000 units
- Adjacent sectors share boundaries (no gaps)
- Origin (0, 0, 0) is typically the starting sector

Example Grid Position:
Sector                  | Grid Position | World Coordinates
------------------------|---------------|------------------
Core Systems (origin)   | (0, 0, 0)     | (0, 0, 0)
Epsilon Mining Reach    | (2, -1, 0)    | (40000000, -20000000, 0)
```

**Formula:** `World Position = Grid Position × 20,000,000`

### 1.4 Planning Checklist

Before proceeding, complete this checklist:

- [ ] Sector name decided
- [ ] Template selected (or custom values planned)
- [ ] Grid coordinates determined
- [ ] Primary Ways/factions identified for council
- [ ] Content type decided (stations, asteroids, etc.)
- [ ] Neighboring sectors identified (if any)

## Step 2: Create the Level Map

Now we'll create the Unreal Engine level that will contain your sector.

### 2.1 Create a New Level

1. In Unreal Editor, go to **File → New Level**
2. Select **Empty Level** (we'll build from scratch)
3. **Save** the level: **File → Save Current Level As...**
4. Navigate to `Content/Maps/Sectors/`
5. Name it: `L_Sector_YourSectorName` (e.g., `L_Sector_EpsilonMiningReach`)

### 2.2 Configure World Settings

1. Open **World Settings** (Window → World Settings)
2. Configure these settings:

```
World Settings:
  Game Mode Override: None (uses project default)
  Kill Z: -100000000 (far below sector floor)
  
World Partition (if using):
  Enable Streaming: true (for large sectors)
  Runtime Grid: (2000000, 2000000, 2000000)
```

### 2.3 Add Basic Lighting

For a space environment:

1. Add a **Directional Light** (simulates distant star)
   - Rotation: (0, -45, 0) or desired angle
   - Intensity: 3.0-10.0 (adjust for desired brightness)
   - Temperature: 6500K (neutral white) or customize

2. Add a **Sky Atmosphere** component (optional, for atmospheric sectors)

3. Add a **Skybox** or procedural starfield (project-specific)

### 2.4 Set Up Post-Processing (Optional)

Add a **Post Process Volume** for space ambiance:

```
Post Process Volume Settings:
  Infinite Extent: true
  
  Bloom:
    Intensity: 0.5
    
  Exposure:
    Auto Exposure: Manual
    Exposure Compensation: 0
    
  Color Grading:
    Saturation: (1.0, 1.0, 1.0, 1.0)
    (Adjust for sector mood)
```

### 2.5 Save and Verify

1. Press **Ctrl+S** to save
2. Verify the level appears in `Content/Maps/Sectors/`
3. Check that it loads without errors

## Step 3: Place the SpaceSectorMap Actor

The `ASpaceSectorMap` actor defines your sector's physical boundaries.

### 3.1 Add the Actor

1. Open the **Place Actors** panel (Window → Place Actors)
2. Search for `SpaceSectorMap`
3. Drag the actor into your level
4. It will appear as a billboard icon in the viewport

### 3.2 Position the Actor

Set the actor's location to your planned sector center:

**In the Details panel:**

```
Transform:
  Location: (40000000, -20000000, 0)  // Your planned coordinates
  Rotation: (0, 0, 0)                  // Keep at zero
  Scale: (1, 1, 1)                     // DO NOT CHANGE!
```

**Important:** Never modify the scale of `ASpaceSectorMap`. The sector size is a constant (20,000,000 units) and scaling would break boundary calculations.

### 3.3 Configure Sector Properties

**In the Details panel, Sector Info category:**

```
Sector Info:
  Sector Name: "Epsilon Mining Reach"
  Description: "A dangerous frontier sector rich in rare ores. 
                Mining operations thrive here despite frequent 
                pirate raids. High risk, high reward."
```

### 3.4 Verify Sector Bounds

The sector boundaries extend ±10,000,000 units from the center:

| Property | Value |
|----------|-------|
| Center | (40000000, -20000000, 0) |
| Min Corner | (30000000, -30000000, -10000000) |
| Max Corner | (50000000, -10000000, 10000000) |
| Size | 200km × 200km × 200km |

You can verify these in Blueprint using `GetSectorBounds()`.

### 3.5 Save the Level

Press **Ctrl+S** to save your progress.

## Step 4: Create Sector Council Data Asset

The council Data Asset defines governance for your sector.

### 4.1 Create the Data Asset

1. In **Content Browser**, navigate to `Content/DataAssets/Sectors/`
2. Right-click → **Miscellaneous → Data Asset**
3. Select **SectorCouncilDataAsset** as the Data Asset class
4. Name it: `DA_Council_EpsilonMiningReach`
5. Double-click to open

### 4.2 Configure Identity Section

```
Identity:
  Sector Name: "Epsilon Mining Reach"
  Sector ID: EpsilonMiningReach    // Unique, no spaces!
  Description: "A dangerous frontier sector rich in rare ores..."
```

**Important:** The `Sector Name` here should match the name on your `SpaceSectorMap` actor.

### 4.3 Configure Council Section

For a frontier mining sector with pragmatic governance:

```
Council:
  Total Seats: 8
  Pass Threshold: 51    // Simple majority - quick decisions
```

### 4.4 Add Representatives

Click the **+** button on the Representatives array for each council member:

**Representative 1 - Mining Cooperative (Dominant):**
```
Representative:
  Representing Way: DA_Way_MiningCooperative   // Select your Way Data Asset
  Representative Name: "Foreman Jack Steele"
  Voting Weight: 22
  Years of Service: 8
  Is Player Controlled: false
```

**Representative 2 - Freight Haulers:**
```
Representative:
  Representing Way: DA_Way_FreightHaulers
  Representative Name: "Captain Rosa Martinez"
  Voting Weight: 18
  Years of Service: 5
  Is Player Controlled: false
```

**Representative 3 - Security Contractors:**
```
Representative:
  Representing Way: DA_Way_SecurityContractors
  Representative Name: "Commander Viktor Kross"
  Voting Weight: 15
  Years of Service: 3
  Is Player Controlled: false
```

**Representative 4 - Independent Miners:**
```
Representative:
  Representing Way: DA_Way_IndependentMiners
  Representative Name: "Mira Chen"
  Voting Weight: 12
  Years of Service: 2
  Is Player Controlled: false
```

**Tip:** Voting weights should sum to a reasonable total (60-100 is typical). Higher weights = more influence.

### 4.5 Configure Taxation

For a frontier sector with high security needs:

```
Taxation:
  Security Tax Rate: 15      // High - pirate protection
  Infrastructure Tax Rate: 5 // Low - bare minimum
  Administration Tax Rate: 3 // Low - minimal bureaucracy
  
  // Total: 23%
```

### 4.6 Configure Budget Allocation

**Remember:** These must sum to exactly 100%.

```
Budget:
  Security Budget Percent: 70       // High - active patrols needed
  Infrastructure Budget Percent: 15 // Low - basic maintenance only
  Emergency Reserve Percent: 15     // Standard reserve
  
  Treasury Funds: 500000            // Modest starting funds
```

### 4.7 Add Active Policies (Optional)

Click **+** on Active Policies to add sector-specific rules:

**Example Policy - Anti-Piracy Patrols:**
```
Policy:
  Policy Type: PatrolFrequency
  Policy Name: "Anti-Piracy Sweeps"
  Description: "Frequent security sweeps to deter pirate activity"
  Policy Value: 80     // High patrol frequency
  Is Active: true
```

### 4.8 Save and Verify

1. Click **Save** in the Data Asset editor
2. Verify `IsBudgetBalanced()` returns true (budget sums to 100%)
3. Check that all Way references are valid

## Step 5: Populate Your Sector

Now add content to make your sector feel alive.

### 5.1 Content Planning

For a frontier mining sector, plan content like this:

| Content Type | Quantity | Purpose |
|--------------|----------|---------|
| Major Mining Station | 1 | Central hub, ore processing |
| Mining Platforms | 4-6 | Active mining operations |
| Refueling Depot | 1 | Ship services |
| Security Outpost | 1-2 | Patrol base |
| Asteroid Fields | 2-3 | Mineable resources |
| NPC Mining Ships | 10-20 | Active operations |
| Patrol Ships | 3-5 | Security presence |
| Pirate Spawn Points | 2-4 | Random encounters |

### 5.2 Using Sector Bounds for Placement

Use the `ASpaceSectorMap` functions to place content correctly:

**Blueprint Example:**

```
Get Reference to SpaceSectorMap Actor
    ↓
Call GetRandomPositionInSector()
    ↓
Spawn Station/Ship at returned position
    ↓
Verify with IsPositionInSector(SpawnLocation)
```

**C++ Example:**

```cpp
// Get a random position for spawning
FVector SpawnLocation = SectorMapActor->GetRandomPositionInSector();

// Verify it's within bounds
if (SectorMapActor->IsPositionInSector(SpawnLocation))
{
    // Spawn your actor here
    GetWorld()->SpawnActor<ASpaceStation>(StationClass, SpawnLocation, FRotator::ZeroRotator);
}
```

### 5.3 Place Major Stations

1. Add your main station actors to the level
2. Position them within sector bounds
3. Configure each station's Data Asset references
4. Set faction ownership appropriately

**Example Station Placement:**

```
Main Mining Hub:
  Location: (40500000, -20000000, 0)    // Slightly off-center
  Faction: Mining Cooperative
  
Refueling Depot:
  Location: (42000000, -22000000, 500000)  // Upper quadrant
  Faction: Freight Haulers
  
Security Outpost:
  Location: (38000000, -18000000, -200000)  // Patrol position
  Faction: Security Contractors
```

### 5.4 Add Asteroid Fields

Create asteroid field areas for mining gameplay:

1. Add asteroid field actors or volume triggers
2. Position within sector bounds
3. Configure ore types and quantities
4. Set respawn rates for game balance

### 5.5 Configure NPC Spawning

Set up spawn points or managers for dynamic content:

- Mining ships that travel between stations and asteroid fields
- Patrol ships that sweep the sector
- Pirate spawners at sector edges for encounters
- Trading ships passing through

### 5.6 Add Navigation Markers

Place nav points for autopilot and quest objectives:

- Jump gate positions (if connecting to other sectors)
- Trade route waypoints
- Patrol route markers
- Points of interest

### 5.7 Content Checklist

Before testing, verify:

- [ ] All content is within sector bounds
- [ ] Stations have valid Data Asset references
- [ ] NPC spawners are configured
- [ ] Asteroid fields are mineable
- [ ] At least one docking location exists
- [ ] No content overlaps inappropriately

## Step 6: Test and Validate

Thorough testing ensures your sector works correctly.

### 6.1 Pre-Flight Checklist

Before entering Play mode, verify:

**Data Asset Validation:**
- [ ] Budget allocation sums to exactly 100%
- [ ] Sector ID is unique across all sectors
- [ ] All Way references point to valid Data Assets
- [ ] Representative voting weights are reasonable (10-25)
- [ ] Pass threshold is between 51-100%
- [ ] Tax rates don't exceed 50% each
- [ ] Treasury funds are positive

**Level Validation:**
- [ ] SpaceSectorMap actor is placed and configured
- [ ] Sector Name matches between actor and Data Asset
- [ ] All stations are within sector bounds
- [ ] At least one spawn point for the player exists

### 6.2 Console Verification

1. Press **Play** in the editor
2. Open the console (~ key)
3. Check for sector initialization:

```
LogAdastrea: SpaceSectorMap: Initialized sector 'Epsilon Mining Reach' at location (40000000.00, -20000000.00, 0.00)
```

4. Look for any warnings or errors

### 6.3 Boundary Testing

Test that sector boundaries work correctly:

**Blueprint Test:**

```
1. Get player location
2. Call IsPositionInSector(PlayerLocation)
3. Print result to screen
4. Fly to sector edges and verify transitions
```

**Expected Results:**
- Inside sector center: returns `true`
- At sector edges (±100km from center): returns `true`
- Outside sector: returns `false`

### 6.4 Gameplay Testing

Test the player experience:

| Test | How to Verify |
|------|---------------|
| **Navigation** | Fly through the sector, check frame rate |
| **Docking** | Approach a station, verify docking works |
| **Mining** | Target asteroids, verify mining functions |
| **Combat** | Encounter pirates, test combat systems |
| **Trading** | Access station markets, verify prices |
| **Quests** | Check if sector generates appropriate missions |

### 6.5 Council Function Testing

Verify council Data Asset functions work:

**In Blueprint or C++:**

```cpp
// Test tax calculation
int64 TaxOwed = SectorCouncil->CalculateTaxOwed(100000);
// Expected: ~23,000 for our 23% total tax rate

// Test budget balance
bool bBalanced = SectorCouncil->IsBudgetBalanced();
// Expected: true

// Test voting (votes_for, votes_against)
bool bWouldPass = SectorCouncil->WouldVotePass(45, 22);
// Expected: true (45 votes for is > 51% of 67 total weight)
```

### 6.6 Integration Testing

Test integration with other systems:

**Way System:**
- Council representatives appear correctly
- Way reputation affects sector interactions

**Trading System:**
- Sector tax rates affect transaction prices
- Budget affects available services

**Combat System:**
- Security budget affects patrol presence
- Danger level matches expectations

**Quest System:**
- Sector generates appropriate mission types
- Objectives reference sector locations correctly

### 6.7 Testing Checklist

Complete before considering the sector finished:

- [ ] Sector initializes without errors
- [ ] Boundaries work correctly
- [ ] All content is accessible
- [ ] Performance is acceptable (30+ FPS)
- [ ] Council functions return expected values
- [ ] Integration with other systems works
- [ ] Gameplay is fun and matches intended theme

## Common Issues and Troubleshooting

### Issue: Sector Doesn't Initialize

**Symptoms:**
- No initialization log message
- Sector bounds queries return incorrect values

**Solutions:**
1. Verify `ASpaceSectorMap` actor is in the level
2. Check that the level is being loaded
3. Ensure no compilation errors in sector classes

### Issue: Budget Doesn't Sum to 100%

**Symptoms:**
- `IsBudgetBalanced()` returns `false`
- Console warnings about budget

**Solutions:**
```
Current:                     Fixed:
Security: 70%               Security: 70%
Infrastructure: 15%    →    Infrastructure: 15%
Reserve: 10%                Reserve: 15%    // Was 10%
Total: 95%                  Total: 100%
```

### Issue: Council Can't Pass Any Votes

**Symptoms:**
- All votes fail
- `WouldVotePass()` always returns `false`

**Solutions:**
1. Lower the Pass Threshold (try 51%)
2. Increase voting weights for cooperative Ways
3. Add more aligned representatives

### Issue: Content Outside Sector Bounds

**Symptoms:**
- Objects appear but aren't "in" the sector
- `IsPositionInSector()` returns `false` for visible content

**Solutions:**
1. Use `GetRandomPositionInSector()` for placement
2. Verify sector center coordinates are correct
3. Check that content coordinates are in the correct range

### Issue: Performance Problems

**Symptoms:**
- Low frame rate (below 30 FPS)
- Hitching when moving through sector

**Solutions:**
1. Reduce number of active NPC ships
2. Enable LOD for distant objects
3. Use World Partition for streaming
4. Reduce asteroid field density
5. Check for excessive tick operations

### Issue: Sector Name Mismatch

**Symptoms:**
- Systems don't recognize the sector
- UI shows wrong sector name

**Solutions:**
- Ensure `SectorName` in `ASpaceSectorMap` matches `SectorName` in `USectorCouncilDataAsset`
- Ensure `SectorID` is unique and consistent where referenced

### Issue: Ways Not Appearing in Council

**Symptoms:**
- Representatives show null Way references
- Council member list is empty

**Solutions:**
1. Verify Way Data Assets exist and are saved
2. Check that Representative `RepresentingWay` field points to valid asset
3. Ensure Way Data Assets are in the correct content folder

### Quick Debugging Reference

| Problem | First Check | Second Check | Third Check |
|---------|-------------|--------------|-------------|
| No initialization | Actor in level? | Level loading? | Console errors? |
| Wrong bounds | Coordinates correct? | Scale at 1,1,1? | Not overlapping? |
| Budget error | Sum to 100%? | All positive? | Values reasonable? |
| No votes pass | Threshold too high? | Weights balanced? | Enough reps? |
| Missing content | Within bounds? | Spawning enabled? | References valid? |

## Next Steps and Related Documentation

### Expanding Your Sector

After your initial sector is working:

1. **Add More Content:** Additional stations, ships, and resources
2. **Create Quests:** Sector-specific missions and objectives
3. **Connect Sectors:** Add jump gates to adjacent sectors
4. **Polish Visuals:** Improve lighting, add VFX, refine atmosphere
5. **Balance Gameplay:** Adjust difficulty, rewards, and pacing

### Creating Additional Sectors

Use this guide as a template for future sectors:

1. Plan sector identity and coordinates
2. Create level with SpaceSectorMap actor
3. Create and configure council Data Asset
4. Populate with appropriate content
5. Test thoroughly

### Related Documentation

For deeper dives into specific topics:

| Topic | Document |
|-------|----------|
| Council Governance Details | `Assets/SectorCreationGuide.md` |
| Placement Strategies | `Assets/SectorMapPlacementGuide.md` |
| Quick Reference | `Assets/SectorSystemQuickReference.md` |
| Sector Templates | `Assets/SectorTemplates/README.md` |
| Way System | `Assets/WaySystemGuide.md` |
| Station Placement | `Assets/StationManagementGuide.md` |
| Trading Integration | `Assets/TradingSystemGuide.md` |
| Quest Integration | `Assets/QuestSystemGuide.md` |

### Summary: Complete Workflow

Here's the entire process in brief:

```
1. PLAN
   └── Define theme, purpose, coordinates, and governance
   
2. CREATE LEVEL
   └── New empty level → Save as L_Sector_Name
   
3. PLACE ACTOR
   └── Add SpaceSectorMap → Position at sector center → Configure name
   
4. CREATE DATA ASSET
   └── New SectorCouncilDataAsset → Configure identity, council, taxes, budget
   
5. POPULATE
   └── Add stations, ships, asteroids → All within sector bounds
   
6. TEST
   └── Verify initialization → Test boundaries → Gameplay test → Integration test
   
7. ITERATE
   └── Fix issues → Balance gameplay → Polish content → Ship it!
```

### Getting Help

If you encounter issues not covered here:

1. Check the console for error messages
2. Review related documentation (linked above)
3. Verify all Data Asset references are valid
4. Test with a minimal configuration first
5. Compare against working template sectors

---

**Last Updated:** November 2025  
**Version:** 1.0  
**Maintained by:** Adastrea Development Team
