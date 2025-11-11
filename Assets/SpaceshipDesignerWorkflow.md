# Spaceship Designer Workflow Guide

This guide provides practical workflows for designers working with the Spaceship Data Asset system, including best practices, common tasks, and tips for efficient ship creation and iteration.

---

## Quick Start Workflow

### New Designer - First Ship (10 minutes)

1. **Open Content Browser** → Navigate to `Content/Spaceships/`
2. **Right-click** → Miscellaneous → Data Asset → SpaceshipDataAsset
3. **Name it:** `DA_Ship_MyFirstScout`
4. **Open the asset** (double-click)
5. **Fill in Basic Info:**
   - Ship Name: "My First Scout"
   - Description: "A test scout ship"
   - Ship Class: "Scout"
   - Ship ID: "MyFirstScout"
6. **Open** `Assets/SpaceshipTemplates/Scout_Pathfinder.yaml`
7. **Copy values** from YAML into your asset
8. **Save** the asset
9. **Test** in a Blueprint

✅ You now have a working scout ship!

---

## Standard Ship Creation Workflow

### For Experienced Designers (30-60 minutes per ship)

#### Phase 1: Planning (5 minutes)
1. Define the ship's primary role
2. List 3 key strengths
3. List 2-3 acceptable weaknesses
4. Choose size category
5. Determine rarity tier

#### Phase 2: Setup (5 minutes)
1. Create Data Asset with proper naming
2. Fill in Basic Info section
3. Set Ship ID (critical for code references)
4. Write initial description

#### Phase 3: Core Stats (10 minutes)
1. Set Hull Strength based on size category
2. Set Cargo Capacity based on role
3. Determine crew requirements
4. Assign modular points

#### Phase 4: Role-Specific Stats (15 minutes)
1. **If Combat Role:**
   - Prioritize: Armor, Shields, Weapons, Point Defense
   - Set high values (7-10) for combat stats
   - Keep cargo and diplomacy low

2. **If Trade Role:**
   - Prioritize: Cargo Capacity, Fuel, Jump Range
   - Set high cargo (2000+)
   - Keep weapons minimal (1-2)

3. **If Exploration Role:**
   - Prioritize: Sensors, Jump Range, Fuel, Speed
   - Set high sensor range (15,000+)
   - Good stealth (6-8)

4. **If Mining Role:**
   - Prioritize: Mining Rating, Cargo, Drones, Repair
   - Max mining rating (8-10)
   - Large cargo (5,000+)

5. **If Support Role:**
   - Prioritize: Medical, Science, Repair, Life Support
   - Focus on utility ratings (7-10)
   - Moderate combat capability

6. **If Command Role:**
   - Prioritize: Sensors, Hangar, Diplomacy, EWar, AI
   - High crew capacity (100+)
   - Strong but balanced combat stats

#### Phase 5: Balance Check (10 minutes)
1. Calculate combat rating (Blueprint or estimate)
2. Calculate mobility rating
3. Calculate operational cost
4. Verify role suitability
5. Adjust outliers
6. Ensure trade-offs exist

#### Phase 6: Polish (5 minutes)
1. Write detailed lore notes
2. Choose color scheme
3. Set manufacturer and year
4. Assign rarity tier
5. Final description pass

#### Phase 7: Testing (10 minutes)
1. Create test Blueprint
2. Assign data asset
3. Spawn in test level
4. Verify all stats display correctly
5. Test role suitability functions
6. Document any issues

---

## Rapid Iteration Workflow

### Creating Ship Variants (5-10 minutes each)

#### Method 1: Duplicate and Modify
1. **Right-click** existing ship asset
2. **Duplicate**
3. **Rename** to variant name (e.g., "PathfinderMkII")
4. **Open** both assets side-by-side
5. **Modify 2-3 key stats**
6. **Update** Ship ID, Name, Description, Lore
7. **Adjust** rarity if needed
8. **Save**

#### Method 2: Template Mixing
1. **Create** new Data Asset
2. **Open** two template YAML files
3. **Take hull/core stats** from Template A
4. **Take weapons/combat** from Template B
5. **Adjust** for balance
6. **Create unique** lore and identity
7. **Save**

#### Example Variants to Create:

**From Scout Template:**
- Early/Late game versions (adjust sensors ±30%)
- Civilian vs Military (adjust weapons, stealth)
- Long-range vs Short-range (adjust fuel, jump range)

**From Gunship Template:**
- Light/Medium/Heavy variants (scale hull, weapons proportionally)
- Interceptor (boost speed, reduce weapons)
- Assault (boost boarding, add marines)

**From Trader Template:**
- Bulk Hauler (max cargo, reduce speed)
- Fast Courier (reduce cargo, boost speed)
- Armed Merchant (add weapons, reduce cargo slightly)

---

## Balancing Workflow

### Ensuring Fair and Fun Ships

#### Step 1: Role Verification
```
✓ Does ship excel at its primary role? (Rating 7-10)
✓ Does ship perform adequately at secondary roles? (Rating 5-7)
✓ Does ship struggle with opposing roles? (Rating 1-3)
```

#### Step 2: Power Level Check
Calculate total "power points":
- Combat stats: Sum of (Armor/10 + Shields/100 + Weapons*100 + PD*10)
- Mobility stats: Sum of (Speed/10 + Accel/10 + Maneuver*10 + Jump*5)
- Utility stats: Sum of all ratings * 10
- Cargo: Points / 100

**Target Ranges:**
- Fighter: 100-200 points
- Corvette: 200-400 points
- Frigate: 400-800 points
- Cruiser: 800-1,500 points
- Battleship: 1,500-2,500 points
- Capital: 2,500+ points

#### Step 3: Cost-Benefit Analysis
```
Operational Cost should scale with power:
- Low Power (100-400): $100-500/day
- Medium Power (400-1000): $500-1500/day
- High Power (1000-2000): $1500-3000/day
- Very High (2000+): $3000+/day
```

#### Step 4: Comparative Testing
1. Create test scenario Blueprint
2. Spawn multiple ships
3. Compare stats side-by-side
4. Identify outliers
5. Adjust and re-test

#### Step 5: Gameplay Testing
- Can player afford to operate this ship?
- Does ship feel unique compared to others?
- Is ship fun to use in its role?
- Are there compelling reasons to upgrade/sidegrade?

---

## Common Tasks & Solutions

### Task: "Create a balanced combat ship"

**Workflow:**
1. Start with Warhammer template
2. Adjust size (hull strength) to desired tier
3. Scale weapons proportionally (1 slot per 500 hull)
4. Set armor to 5% of hull strength
5. Set shields to 60% of hull strength
6. Set point defense based on size (5-8 for most)
7. Reduce speed inversely to hull (more hull = slower)
8. Set crew to hull/200
9. Minimize cargo (hull/20)
10. Set operational cost to (crew * 10) + (hull * 0.01)

### Task: "Create an economy-friendly starter ship"

**Workflow:**
1. Start with Merchant or Scout template
2. Reduce all stats by 20-30%
3. Set operational cost to $200/day or less
4. Set rarity to "Common"
5. Use basic/generic manufacturer
6. Keep crew requirements low (5 or less)
7. Focus on one role, mediocre at others
8. Test that player can complete starter missions

### Task: "Create an endgame reward ship"

**Workflow:**
1. Start with Sovereign or create custom
2. Increase key stats by 50-100%
3. Add multiple advanced systems (EWar, AI, etc.)
4. Set operational cost high ($5,000+/day)
5. Set rarity to "Legendary"
6. Write compelling lore (prototype, unique, historic)
7. Ensure it's powerful but not game-breaking
8. Test that it feels like a worthwhile reward

### Task: "Create a faction-specific ship"

**Workflow:**
1. Review faction traits and attributes
2. Choose template matching faction style
3. Adjust stats to reflect faction strengths
   - Military faction: +combat stats
   - Trade-focused faction: +cargo, +operational efficiency (or design for Way System guilds)
   - Scientific faction: +sensors, +science
4. Use faction colors for Primary/Secondary Color
5. Set manufacturer to faction-appropriate company (or Way guild if ship is guild-specific)
6. Write lore connecting to faction history
7. Consider adding faction requirement in Blueprint

**Note:** For ships operated by economic guilds, see the [Way System Guide](WaySystemGuide.md) for guild-specific ship design considerations.

### Task: "Balance a ship that's too powerful"

**Checklist:**
- [ ] Reduce hull strength by 10-20%
- [ ] Reduce 2-3 highest stats by 20-30%
- [ ] Increase operational cost by 30-50%
- [ ] Increase maintenance level by 1-2 points
- [ ] Reduce fuel capacity or increase consumption
- [ ] Consider increasing crew requirements
- [ ] Keep rarity at "Rare" or higher
- [ ] Add meaningful trade-offs (reduce mobility or cargo)

### Task: "Make a ship more interesting"

**Ideas:**
- Add specialized capability (mining, medical, diplomacy)
- Create asymmetric stats (very high in one area, very low in another)
- Add lore that suggests unique mechanics
- Give it a signature trait (fastest, toughest, stealthiest)
- Add visual distinction via unique colors
- Create complementary ships (fleet composition)

---

## Blueprint Integration Workflows

### Workflow 1: Ship Selection UI

**Goal:** Let player choose ship based on role

```
1. Create Widget with ship slots
2. For each slot:
   - Create SpaceshipDataAsset variable
   - Display: Ship Name, Class, Combat/Mobility/Utility ratings
   - Use GetCombatRating(), GetMobilityRating(), GetUtilityRating()
3. On selection:
   - Show full stats panel
   - Show role suitability indicators
   - Calculate and show operational cost
4. On confirm:
   - Spawn ship with selected data asset
   - Initialize ship stats from data
```

### Workflow 2: Dynamic Ship Stats

**Goal:** Apply data asset stats to spawned ship

```
1. In ship Blueprint BeginPlay:
   - Get SpaceshipDataAsset reference
   - Read HullStrength → Set Max Health component
   - Read MaxSpeed → Set movement component speed
   - Read WeaponSlots → Spawn weapon components
   - Read ShieldStrength → Initialize shield system
2. Create stat update function:
   - Accepts damage, returns remaining hull
   - Factors in armor rating for damage reduction
   - Triggers shield recharge after delay
```

### Workflow 3: Ship Upgrade System

**Goal:** Let players improve their ship

```
1. Create upgrade data structure:
   - Cost (credits)
   - Stat to modify
   - Modification amount
   - Requirements (tech level, slots)
2. On upgrade purchase:
   - Check CustomizableSlots available
   - Verify cost can be paid
   - Modify runtime stat (don't change asset!)
   - Decrement available slots
   - Save upgraded ship state
3. Display:
   - Show base stats from asset
   - Show modified stats with upgrades
   - Use different colors for upgraded stats
```

### Workflow 4: Role-Based Missions

**Goal:** Gate missions by ship capability

```
1. Define mission requirements:
   - MinCombatRating: 60
   - MinCargoCapacity: 1000
   - OR specific role: "Mining"
2. On mission select:
   - Get player's current ship data asset
   - Check IsSuitableForRole("Mining")
   - OR compare stats to minimums
   - If passes: allow mission
   - If fails: show requirement message
3. Provide alternative:
   - Suggest suitable ship types
   - Show where to acquire them
   - Allow rental for single mission
```

---

## Productivity Tips

### Speed Up Asset Creation

1. **Use Templates Library**
   - Keep YAML files open in text editor
   - Use split-screen with Unreal Engine
   - Copy-paste entire sections

2. **Create Preset Collections**
   - Save common stat combinations
   - Document your personal presets
   - Share presets with team

3. **Use Naming Conventions**
   - `DA_Ship_[Class]_[Name]` for assets
   - `[Name][Class]` for Ship IDs
   - Consistent manufacturer names

4. **Batch Create Variants**
   - Duplicate 5 times, then modify all
   - Faster than one-at-a-time
   - Good for faction fleets

5. **Keyboard Shortcuts**
   - Tab through fields quickly
   - Use copy-paste for repeated values
   - Learn Unreal's duplication shortcut

### Avoid Common Mistakes

❌ **Don't:**
- Forget to set Ship ID (causes blueprint errors)
- Make every stat high (no trade-offs)
- Ignore operational costs (economy breaking)
- Copy-paste without changing Ship ID (conflicts)
- Create ships in isolation (poor balance)

✅ **Do:**
- Set unique Ship ID for every asset
- Enforce trade-offs (strengths require weaknesses)
- Calculate and set realistic operational costs
- Always rename Ship ID when duplicating
- Compare new ships to existing fleet

### Organization Best Practices

**Folder Structure:**
```
Content/
└── Spaceships/
    ├── Scouts/
    │   ├── DA_Ship_Pathfinder
    │   └── DA_Ship_PathfinderMkII
    ├── Combat/
    │   ├── DA_Ship_Warhammer
    │   └── DA_Ship_WarhammerHeavy
    ├── Civilian/
    │   ├── DA_Ship_Merchant
    │   └── DA_Ship_Starliner
    └── Factions/
        ├── SolarisUnion/
        └── IroncladConsortium/
```

**Naming Conventions:**
- Data Assets: `DA_Ship_[Name]`
- Ship IDs: `[Name][Class]` (no spaces)
- Blueprints using ships: `BP_[Name]_Ship`

**Documentation:**
- Keep a spreadsheet of all ships
- Track: Name, Role, Size, Rarity, Status (WIP/Done/Testing)
- Note balance issues and needed changes
- Share with team regularly

---

## Testing Checklist

### Before Marking Ship as "Done"

**Functional Testing:**
- [ ] Asset opens without errors
- [ ] Ship ID is unique
- [ ] All required fields filled in
- [ ] Stats are within valid ranges
- [ ] Blueprint spawns ship correctly
- [ ] Stats display in UI correctly
- [ ] Helper functions return expected values

**Balance Testing:**
- [ ] Combat/Mobility/Utility ratings calculated
- [ ] Role suitability matches design intent
- [ ] Operational cost is reasonable for power level
- [ ] Ship has clear strengths and weaknesses
- [ ] Ship is comparable to similar-tier ships
- [ ] Size category matches hull/crew values

**Polish Testing:**
- [ ] Description is clear and engaging
- [ ] Lore notes add flavor
- [ ] Colors are visually distinct
- [ ] Manufacturer name makes sense
- [ ] Year built fits universe timeline
- [ ] Rarity tier matches stats/uniqueness

**Integration Testing:**
- [ ] Works with existing game systems
- [ ] Can be acquired through normal gameplay
- [ ] Affordable for intended progression point
- [ ] Upgrades work if system exists
- [ ] Faction association if applicable

---

## Advanced Workflows

### Creating a Complete Faction Fleet

**Goal:** Design 5-7 ships for one faction

**Workflow (2-3 hours):**
1. Review faction traits and style
2. Choose ship roles needed:
   - 1 Scout/Explorer
   - 2-3 Combat ships (light, medium, heavy)
   - 1 Cargo/Support
   - 1 Capital/Flagship
3. Create all assets from templates
4. Apply faction color scheme to all
5. Use consistent manufacturer
6. Write connected lore (fleet history)
7. Ensure tech level consistency
8. Balance as a group (synergy)
9. Test fleet composition in combat scenario

### Creating a Ship Class Lineage

**Goal:** Early/Mid/Late game progression

**Example: Scout Line**
1. **Early:** Surveyor Scout
   - 50% of Pathfinder stats
   - Rarity: Common
   - Cost: $150/day
   
2. **Mid:** Pathfinder Scout (use template)
   - 100% baseline
   - Rarity: Uncommon
   - Cost: $180/day
   
3. **Late:** Pathfinder Mk II
   - 130% of Pathfinder stats
   - +2 advanced systems
   - Rarity: Rare
   - Cost: $250/day
   
4. **Endgame:** Vanguard Scout
   - 150% of Pathfinder stats
   - Maximum sensors and stealth
   - Rarity: Legendary
   - Cost: $400/day

### Creating Specialized Mission Ships

Ships designed for specific gameplay scenarios:

**Stealth Insertion Ship:**
- Max stealth (10)
- Max EWar (10)
- Good speed (700+)
- Minimal weapons
- Small crew (5-10)
- High boarding capability

**Hospital Ship:**
- Max medical (10)
- High crew capacity (200+)
- Good life support (9-10)
- Minimal weapons
- Strong shields (defense only)
- Large drone capacity (medical drones)

**Science Vessel:**
- Max science (10)
- Max sensors (30,000+ range)
- High sensor resolution (10)
- Good mobility
- Extensive drones (100+)
- Advanced AI (8+)

---

## Collaboration Workflows

### Working with Other Designers

**Communication:**
- Document your ships in shared spreadsheet
- Mark WIP ships clearly
- Note any dependent ships (fleet members)
- Call out balance concerns
- Share interesting stat combinations

**Review Process:**
1. Create ship
2. Self-test with checklist
3. Mark as "Ready for Review"
4. Another designer tests
5. Provide feedback
6. Iterate
7. Mark as "Approved"

**Merge Conflicts Prevention:**
- Check out assets before editing
- Communicate when working on same faction
- Don't modify other designer's ships without asking
- Keep personal test ships in separate folder

### Working with Programmers

**What to Communicate:**
- New stat fields needed
- Helper functions desired
- Blueprint integration needs
- Performance concerns (too many ships)
- Bug reports (stats not applying)

**What Programmers Need from You:**
- Consistent Ship ID naming
- Filled-in all required fields
- Reasonable stat ranges
- Test cases for new features
- Clear reproduction steps for bugs

---

## Iteration and Improvement

### Monthly Balance Pass Workflow

1. **Collect Data:**
   - Which ships are most used?
   - Which ships are never used?
   - Player feedback on difficulty
   - Mission completion rates per ship type

2. **Identify Issues:**
   - Overpowered ships (used exclusively)
   - Underpowered ships (never used)
   - Missing roles (gameplay gaps)
   - Balance outliers

3. **Plan Changes:**
   - Nerf overpowered by 10-20%
   - Buff underpowered by 15-30%
   - Create ships for missing roles
   - Adjust operational costs

4. **Implement:**
   - Update all affected assets
   - Update documentation
   - Increment version numbers
   - Note changes in patch notes

5. **Test:**
   - Playtest with changes
   - Verify balance improved
   - Check no new issues introduced
   - Get team feedback

6. **Release:**
   - Commit changes
   - Update changelog
   - Communicate to team
   - Monitor player response

---

## Conclusion

These workflows provide structure for efficient ship creation and management. Adapt them to your team's needs, and don't be afraid to develop your own shortcuts and methods.

For more resources:
- **SpaceshipDataAssetGuide.md** - Detailed stat explanations
- **SpaceshipTemplates.md** - Pre-made ship references
- **SpaceshipInteriorGuide.md** - Complete interior implementation guide
- **ShipControlGuide.md** - Ship control system from interiors
- **Assets/SpaceshipTemplates/** - YAML template files

Happy ship designing!
