# Way Network Designer Workflow

## Overview

This workflow guide shows how designers can create and integrate Way Networks into gameplay without writing C++ code. Everything is done through Blueprint Data Assets and visual scripting.

## Workflow Steps

### Phase 1: Planning Your Network

#### Step 1: Identify Common Precepts

Before creating a network, identify which Ways share common values:

**Example Analysis:**
```
Engineers' Guild values:
- Craftsmanship (90)
- Mastery (85)
- Innovation (70)

Ship-Builders Union values:
- Craftsmanship (85)
- Mastery (80)
- Efficiency (75)

Refinery Collective values:
- Craftsmanship (75)
- Efficiency (80)
- Prosperity (65)

Shared Precepts: Craftsmanship (all), Mastery (2/3)
→ Good candidates for a network!
```

**Precept Alignment Table:**

| Way | Craftsmanship | Mastery | Innovation | Efficiency |
|-----|--------------|---------|-----------|-----------|
| Engineers' Guild | 90 | 85 | 70 | - |
| Ship-Builders | 85 | 80 | - | 75 |
| Refinery Collective | 75 | - | - | 80 |
| **Network Avg** | **83** | **83** | **70** | **78** |

Choose the 2-4 highest shared values for network's Shared Precepts.

#### Step 2: Define Network Purpose

Write a clear description of what binds the network:

**Good Example:**
> "The Craftsmen's Compact is an alliance of master builders united by their dedication to engineering excellence. They share techniques, resources, and apprentices to advance spacecraft construction."

**Poor Example:**
> "Some guilds that work together sometimes."

#### Step 3: Determine Network Size

Choose network size based on purpose:

- **2 members**: Binary partnership (e.g., Research + Archives)
- **3 members**: Classic triangle (most stable)
- **4 members**: Complex dynamics, balanced
- **5 members**: Maximum size, diverse perspectives

**Recommendation**: Start with 3 members, expand if needed.

### Phase 2: Creating the Network

#### Step 1: Create Member Ways (if needed)

For each guild that will join the network:

1. **Content Browser** → Right-click → Blueprint Class
2. Parent: `WayDataAsset`
3. Name: `DA_Way_[GuildName]`
4. Configure:
   - Way Name: Display name
   - Way ID: Unique identifier
   - bIsSchool: true/false
   - Core Precepts: 3-5 values with importance
   - Colors: Primary and secondary

**Checklist for Each Way:**
- [ ] Unique Way ID
- [ ] Clear description
- [ ] 3-5 Core Precepts defined
- [ ] Importance values set (0-100)
- [ ] Colors configured
- [ ] Saved in appropriate folder

#### Step 2: Create Network Data Asset

1. **Content Browser** → Right-click → Blueprint Class
2. Parent: `WayNetworkDataAsset`
3. Name: `DA_Network_[NetworkName]`
4. Save in: `Content/DataAssets/Networks/`

#### Step 3: Configure Basic Identity

Open the network asset and set:

**Basic Info:**
- Network Name: "The Craftsmen's Compact"
- Network ID: `CraftsmenCompact` (no spaces)
- Description: Write purpose (see Phase 1 Step 2)
- Network Color: Choose representative color

**Tips:**
- Network ID must be unique across all networks
- Use consistent naming (e.g., all Compacts, all Alliances)
- Color should be distinct from member Ways' colors

#### Step 4: Add Member Ways

In the Member Ways array, add one element per guild:

**For Each Member:**
1. Click `+` to add array element
2. Set Member Way: Select Way Data Asset
3. Set Influence Level: 50-100 (higher = more network control)
4. Set Commitment Level: 50-100 (higher = stronger reactions)
5. Set Role Description: Optional flavor text

**Influence Guidelines:**
- **80-100**: Network leader, primary voice
- **60-80**: Major partner, significant influence
- **40-60**: Equal partner, shared decisions
- **20-40**: Junior partner, limited influence

**Commitment Guidelines:**
- **80-100**: Fully committed, strong reactions
- **60-80**: Dedicated, moderate reactions
- **40-60**: Moderate commitment
- **20-40**: Loose affiliation, weak reactions

**Example Configuration:**
```
Member 1: Engineers' Guild
  Influence: 70 (Major partner)
  Commitment: 90 (Very dedicated)
  Role: "Masters of propulsion systems"

Member 2: Ship-Builders Union
  Influence: 80 (Network leader)
  Commitment: 85 (Very dedicated)
  Role: "Expert hull designers"

Member 3: Refinery Collective
  Influence: 50 (Equal partner)
  Commitment: 70 (Dedicated)
  Role: "Material suppliers"
```

#### Step 5: Define Shared Precepts

In the Shared Precepts array, add 2-4 common values:

**Selection Process:**
1. Look at member Ways' Core Precepts
2. Find Precepts shared by 2+ members
3. Average the importance values
4. Keep only the strongest alignments

**Example:**
```
Shared Precept 1: Craftsmanship
  Importance: 90 (average of 90, 85, 75)
  Description: "Excellence in all things built"

Shared Precept 2: Mastery
  Importance: 85 (average of 85, 80)
  Description: "Continuous skill improvement"

Shared Precept 3: Innovation
  Importance: 70 (only one member, but thematically important)
  Description: "Push technological boundaries"
```

**Guidelines:**
- 2-4 Shared Precepts ideal
- All members should share at least 2
- Importance 70+ recommended for strong identity
- Importance 50-70 for secondary values
- Importance <50 consider removing

#### Step 6: Configure Network Mechanics

Set the gameplay parameters:

**Reputation Spillover Percent (0-100):**
- Determines how much reputation transfers between members
- **30-40%**: Standard network (recommended)
- **40-50%**: Tight alliance
- **50-60%**: Very strong bonds
- **60-70%**: Almost a single faction

**Network Alignment Bonus (1.0-3.0x):**
- Multiplier for Feats matching Shared Precepts
- **1.5-1.8x**: Moderate bonus (recommended)
- **1.8-2.2x**: Strong bonus
- **2.2-3.0x**: Very powerful (use for specialized networks)

**Minimum Reputation Threshold (0-100):**
- Reputation needed with any member to get network bonuses
- **20-30**: Easy access (recommended for starting networks)
- **30-40**: Standard requirement
- **40-50**: Significant investment
- **50+**: Elite/veteran only

**Example Configuration:**
```
Reputation Spillover: 40%
  → Gain 100 rep with Engineers = 40 rep with other members

Network Alignment Bonus: 1.8x
  → Complete crafting Feat = 1.8x reputation gain

Minimum Threshold: 30
  → Need 30 rep with any member to unlock network bonuses
```

#### Step 7: Set Network Status

Configure active state and history:

- **bIsActive**: Set to `true` for active networks
- **Formation Date**: Set for lore/flavor (optional)

**Use Cases for Inactive Networks:**
- Historical/dissolved alliances
- Networks that activate based on events
- Story-gated networks player must unlock

### Phase 3: Integrating into Game

#### Step 1: Register Network with VerseSubsystem

Create a Blueprint that runs on game start:

**In your Game Mode or Level Blueprint:**

1. **Event Begin Play**
2. **Get Game Instance** → **Get Subsystem** (VerseSubsystem)
3. **Load Asset Class** (WayNetworkDataAsset) → Reference your network
4. **Register Network** (VerseSubsystem node)

**Blueprint Nodes:**
```
Event BeginPlay
  → Get Game Instance
  → Get Subsystem (Class: VerseSubsystem)
  → Load Asset (DA_Network_CraftsmenCompact)
  → Register Network
```

**Tip:** Create a Manager Blueprint that registers all networks at once.

#### Step 2: Create Network Information Widget

Build a UI widget to display network info:

**Widget Components:**
- Text: Network Name
- Text: Network Description
- Progress Bar: Player's network reputation
- List View: Member Ways with influence bars
- Button: View network benefits
- Button: Accept network quest

**Blueprint Logic:**
```
On Construct:
  Get VerseSubsystem
  Get Network Asset (DA_Network_CraftsmenCompact)
  
  Set Network Name Text
  Set Description Text
  
  Network Verse Score → Update Progress Bar
  Qualifies For Network Bonuses → Enable Benefits Button
  
  For Each Member in Get Members:
    Add List Item (Member Way Name, Influence Level)
```

#### Step 3: Network-Aware Quest System

Create quests that affect entire networks:

**Quest Blueprint:**
```
Quest Giver = Engineers' Guild

On Quest Accept:
  Get Networks For Way (Engineers' Guild)
  → Store Network References
  
On Quest Complete:
  Award Feat to Player
  Record Feat With Network Effects (auto spillover)
  
  For Each Network:
    If Qualifies For Network Bonuses:
      Grant Network-Exclusive Reward
```

**Network Quest Types:**
1. **Joint Contract**: Involves multiple member guilds
2. **Network Defense**: Protect all members from threat
3. **Diplomatic Mission**: Strengthen network bonds
4. **Research Project**: All members contribute

#### Step 4: Network-Based Trading

Apply network bonuses to trading:

**Trading Post Blueprint:**
```
On Player Enter:
  Get Station Faction
  Get Networks For Way (Station Faction)
  
  For Each Network:
    If Player Qualifies For Network Bonuses:
      Apply Discount (5-15%)
      Unlock Network-Exclusive Items
      Show "Network Member Benefits" UI
```

**Example Benefits:**
- 10% discount on all goods
- Access to high-quality materials
- Priority on limited stock
- Network-exclusive ship modules

### Phase 4: Testing and Iteration

#### Step 1: Initial Testing Checklist

- [ ] Network appears in VerseSubsystem registered list
- [ ] All member Ways load correctly
- [ ] Shared Precepts display properly
- [ ] Network info widget shows correct data
- [ ] Qualifies check works with test reputation values

#### Step 2: Reputation Testing

Award test Feats and verify:

1. **Complete a Feat aligned with network Precepts**
   - Expected: Base reputation + network bonus
   - Check: Reputation with primary Way increases
   
2. **Check spillover to other members**
   - Expected: Other members gain spillover %
   - Verify: Reputation increases match calculation

3. **Test network bonus threshold**
   - Set reputation below threshold
   - Expected: No network bonuses
   - Raise above threshold
   - Expected: Bonuses activate

#### Step 3: Balance Tuning

Adjust values based on testing:

**If reputation gains too fast:**
- Lower spillover percentage
- Reduce network alignment bonus
- Increase minimum threshold

**If reputation gains too slow:**
- Increase spillover percentage
- Raise network alignment bonus
- Lower minimum threshold

**If network feels too powerful:**
- Reduce number of members
- Lower alignment bonus
- Increase threshold for bonuses

**If network feels insignificant:**
- Add more members
- Increase spillover percentage
- Raise alignment bonus
- Add more visible benefits

### Phase 5: Advanced Features

#### Dynamic Network Formation

Create networks that form based on player actions:

**Blueprint Logic:**
```
Check Player Reputation with Multiple Ways:
  If Rep > 50 with Way A, Way B, Way C
  AND Ways share 2+ Precepts
  
  Create New Network Asset (runtime)
  Add Ways as Members
  Register Network with VerseSubsystem
  
  Show "New Alliance Formed" Event
```

#### Network Conflicts

Create opposing networks with conflicting values:

**Example:**
- Craftsmen's Compact values Craftsmanship
- Profit Cartel values Prosperity and Efficiency
- Player must choose one to support

**Conflict Blueprint:**
```
On Player Supports Network A:
  Increase Rep with Network A members
  Decrease Rep with Network B members
  
  Trigger Network Rivalry Event
```

#### Network Dissolution

Make networks that can break apart:

**Dissolution Conditions:**
- Member reputation falls too low
- Network goals fail
- Political events trigger split
- Player actions cause fracture

**Blueprint Logic:**
```
Check Network Stability:
  For Each Member:
    If Commitment < Threshold
    OR Relationship with Other Members < 0
    
    Set Network bIsActive = false
    Unregister Network
    Show "Network Dissolved" Event
```

## Common Workflows

### Workflow A: Simple Craft Network

**Goal**: Create a crafting guild network

1. Create 3 craft-focused Ways (1 hour)
2. Create network asset (15 min)
3. Register on game start (5 min)
4. Test with crafting Feats (15 min)
5. Add UI display (30 min)

**Total Time**: ~2 hours

### Workflow B: Dynamic Story Network

**Goal**: Network forms during story quest

1. Create 2-3 Ways (30 min)
2. Create network asset, set inactive (10 min)
3. Build quest chain (1 hour)
4. Trigger network activation (15 min)
5. Test quest flow (30 min)

**Total Time**: ~2.5 hours

### Workflow C: Competing Networks

**Goal**: Two rival networks for player choice

1. Create 6 Ways (3 per network) (2 hours)
2. Create 2 network assets (30 min)
3. Set up rivalry logic (1 hour)
4. Create choice events (1 hour)
5. Test both paths (1 hour)

**Total Time**: ~5.5 hours

## Troubleshooting

### Network Not Showing Up

**Check:**
- [ ] Network registered with VerseSubsystem?
- [ ] bIsActive set to true?
- [ ] Member Ways valid and loaded?
- [ ] Network ID unique?

### Spillover Not Working

**Check:**
- [ ] Using RecordFeatWithNetworkEffects?
- [ ] bApplyNetworkEffects set to true?
- [ ] Spillover percentage > 0?
- [ ] Feat Precepts align with network?

### Bonuses Not Applying

**Check:**
- [ ] Player reputation above threshold?
- [ ] Network alignment bonus configured?
- [ ] Feat Precepts match Shared Precepts?
- [ ] Network active in VerseSubsystem?

## Best Practices

### DO:
✓ Start with 3-member networks
✓ Use 30-50% spillover for standard networks
✓ Set 1.5-2.0x alignment bonuses
✓ Write clear network descriptions
✓ Test reputation gains thoroughly
✓ Create visible network benefits
✓ Make networks feel meaningful

### DON'T:
✗ Create networks with <2 members
✗ Use >70% spillover (too powerful)
✗ Set alignment bonus >3.0x (unbalanced)
✗ Add Ways with no shared Precepts
✗ Make network mechanics invisible
✗ Forget to register networks
✗ Create too many networks at once

## Next Steps

1. **Start Simple**: Create one 3-member network
2. **Test Thoroughly**: Verify reputation and spillover
3. **Add UI**: Make network visible to players
4. **Integrate Benefits**: Give tangible rewards
5. **Expand**: Add more networks once first works
6. **Create Conflicts**: Add opposing networks for drama

## Resources

- **Full Guide**: `WayNetworkGuide.md`
- **Quick Start**: `WayNetworkQuickStart.md`
- **Templates**: `/WayNetworkTemplates/` directory
- **Example Networks**: 5 ready-to-use configs

## Summary

Way Networks workflow:
1. **Plan**: Identify shared Precepts
2. **Create**: Build Data Assets
3. **Configure**: Set mechanics
4. **Register**: Add to VerseSubsystem
5. **Integrate**: Connect to gameplay
6. **Test**: Verify functionality
7. **Polish**: Add UI and benefits

Total time for first network: 2-3 hours
Time for additional networks: 30-60 minutes each

Go forth and create interesting micro-alliances!
