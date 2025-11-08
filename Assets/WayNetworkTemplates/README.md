# Way Network Templates

This directory contains YAML templates for example Way Networks—micro-alliances of small guilds with shared Precepts.

## Available Templates

### 1. CraftsmenCompact.yaml
**Purpose**: Engineering and Ship Construction Alliance

**Members**: 3 guilds
- Engineers' Guild (Influence: 70, Commitment: 90)
- Ship-Builders Union (Influence: 80, Commitment: 85)
- Refinery Collective (Influence: 50, Commitment: 70)

**Shared Precepts**:
- Craftsmanship (90)
- Mastery (85)
- Innovation (70)

**Mechanics**:
- Spillover: 40%
- Bonus: 1.8x
- Threshold: 30

**Use Case**: Players focused on ship building and engineering excellence

---

### 2. FrontierAlliance.yaml
**Purpose**: Exploration and Discovery Network

**Members**: 3 guilds
- Scout's League (Influence: 75, Commitment: 95)
- Cartographer's Society (Influence: 60, Commitment: 80)
- Pioneer Traders (Influence: 65, Commitment: 75)

**Shared Precepts**:
- Discovery (95)
- Freedom (80)
- Survival (70)

**Mechanics**:
- Spillover: 35%
- Bonus: 2.0x
- Threshold: 25

**Use Case**: Players exploring uncharted space and mapping new territories

---

### 3. HonorCircle.yaml
**Purpose**: Warrior Guilds and Combat Training

**Members**: 4 guilds
- Warrior's Guild (Influence: 85, Commitment: 90)
- Duelist Academy (Influence: 70, Commitment: 85)
- Protector's Order (Influence: 75, Commitment: 80)
- Bounty Hunter Collective (Influence: 60, Commitment: 70)

**Shared Precepts**:
- Honor (95)
- Strength (85)
- Justice (75)

**Mechanics**:
- Spillover: 30%
- Bonus: 1.7x
- Threshold: 35

**Use Case**: Players pursuing honorable combat and martial excellence

**Warning**: Dishonorable actions hurt reputation with entire network

---

### 4. MerchantCoalition.yaml
**Purpose**: Trade and Commerce Network

**Members**: 3 guilds
- Free Traders Guild (Influence: 80, Commitment: 85)
- Logistics Syndicate (Influence: 70, Commitment: 90)
- Market Analysts (Influence: 65, Commitment: 75)

**Shared Precepts**:
- Prosperity (90)
- Efficiency (85)
- Cunning (70)

**Mechanics**:
- Spillover: 45%
- Bonus: 1.6x
- Threshold: 20

**Use Case**: Players focused on trading, market optimization, and profit

---

### 5. ScholarsSyndicate.yaml
**Purpose**: Research and Knowledge Network

**Members**: 3 guilds
- Research Collective (Influence: 85, Commitment: 95)
- Technology Institute (Influence: 75, Commitment: 85)
- Archivists' Society (Influence: 55, Commitment: 80)

**Shared Precepts**:
- Discovery (90)
- Innovation (85)
- Mastery (75)

**Mechanics**:
- Spillover: 35%
- Bonus: 1.9x
- Threshold: 30

**Use Case**: Players pursuing research, technology, and academic excellence

## How to Use These Templates

### Step 1: Choose a Template

Pick the template that best fits your gameplay needs:
- Engineering/crafting focused? → CraftsmenCompact
- Exploration focused? → FrontierAlliance
- Combat focused? → HonorCircle
- Trading focused? → MerchantCoalition
- Research focused? → ScholarsSyndicate

### Step 2: Create Member Ways

Before creating the network, create the individual Way Data Assets for each member guild. Use the YAML as reference for Precept values.

**Example for Engineers' Guild:**
```yaml
WayName: "Engineers' Guild"
WayID: "EngineersGuild"
bIsSchool: true
CorePrecepts:
  - Precept: Craftsmanship
    ImportanceValue: 90
  - Precept: Mastery
    ImportanceValue: 85
  - Precept: Innovation
    ImportanceValue: 70
```

### Step 3: Create Network Data Asset

In Unreal Editor:
1. Content Browser → Right-click → Blueprint Class
2. Parent Class: `WayNetworkDataAsset`
3. Name: `DA_Network_[TemplateName]`
4. Open and configure using YAML values

### Step 4: Copy Configuration

Transfer values from YAML to your Data Asset:
- Network Name, ID, Description
- Member Ways with Influence/Commitment
- Shared Precepts
- Mechanics (Spillover, Bonus, Threshold)

### Step 5: Register Network

Add to your Game Mode's BeginPlay:
```cpp
// Blueprint:
Get Game Instance → Get Subsystem (VerseSubsystem) 
→ Register Network (Your Network Asset)
```

## Customization Tips

### Adjusting Spillover

**Lower spillover (20-30%)**:
- More independent guilds
- Smaller network effects
- Player must work with each individually

**Higher spillover (40-60%)**:
- Tighter alliance
- Strong network identity
- Reputation spreads quickly

### Adjusting Bonuses

**Lower bonus (1.2-1.5x)**:
- Subtle encouragement
- Balanced gameplay
- Multiple paths viable

**Higher bonus (2.0-3.0x)**:
- Strong incentive
- Specialized playstyle
- Rewards commitment

### Adding/Removing Members

**2-3 members**:
- Simple relationships
- Clear identity
- Easy to balance

**4-5 members**:
- Complex dynamics
- Diverse perspectives
- Rich storytelling

**6+ members**:
- Too complex
- Consider splitting
- Hard to balance

## Combining Templates

You can create opposing networks for player choice:

**Example 1: Craft vs Commerce**
- Craftsmen's Compact (quality focus)
- vs Merchant Coalition (profit focus)
- Player must choose priorities

**Example 2: Honor vs Pragmatism**
- Honor Circle (honorable combat)
- vs Generic Mercenaries (any means necessary)
- Moral choice with gameplay impact

**Example 3: Exploration vs Exploitation**
- Frontier Alliance (discovery)
- vs Mining Cartels (resource extraction)
- Environmental/philosophical choice

## Creating New Templates

When creating your own network templates:

1. **Identify Theme**: What binds these guilds?
2. **Choose 2-5 Members**: Related but distinct
3. **Find 2-4 Shared Precepts**: Common values
4. **Set Mechanics**: Balance for your game
5. **Write Description**: Clear purpose
6. **Test Balance**: Verify reputation gains

### Template Checklist

- [ ] 2-5 member guilds defined
- [ ] Each member has role description
- [ ] 2-4 shared Precepts identified
- [ ] Influence levels assigned (total ~200-300)
- [ ] Commitment levels set (70-95 typical)
- [ ] Spillover percentage chosen (30-50% typical)
- [ ] Bonus multiplier set (1.5-2.0x typical)
- [ ] Threshold determined (20-40 typical)
- [ ] Description written
- [ ] Formation date set (optional)

## Integration Examples

### Quest Integration
```
Player completes engineering quest
→ Awards "Master Craftsman" Feat
→ Craftsmen's Compact detects aligned Precepts
→ All 3 member guilds gain reputation
→ Player unlocks network benefits
```

### Trading Integration
```
Player trades at Engineers' Guild station
→ Has 40 rep with Engineers' Guild
→ Qualifies for Craftsmen's Compact bonuses
→ Gets 10% network member discount
→ Access to exclusive ship modules
```

### Reputation Integration
```
Player gains 100 rep with Engineers' Guild
→ Spillover: 40% to other members
→ Ship-Builders gain 40 rep
→ Refinery Collective gains 40 rep
→ All guilds now friendly
```

## Troubleshooting

### Template Not Working

**Check**:
- Member Way assets exist and are valid
- Network ID is unique
- Shared Precepts match member Precepts
- All values are within valid ranges

### Reputation Not Spreading

**Check**:
- Network registered with VerseSubsystem
- bIsActive set to true
- Using RecordFeatWithNetworkEffects
- Spillover percentage > 0

### Bonuses Not Applying

**Check**:
- Player reputation above threshold
- Feat Precepts align with network
- Network alignment bonus configured
- Code checking for network bonuses

## Documentation

For complete information, see:
- **Quick Start**: `../WayNetworkQuickStart.md`
- **Full Guide**: `../WayNetworkGuide.md`
- **Workflow**: `../WayNetworkWorkflow.md`
- **Blueprint Examples**: `../../Blueprints/WayNetworkBlueprintExamples.md`

## Summary

These templates provide ready-to-use network configurations:
- 5 themed networks covering major playstyles
- Balanced mechanics for typical gameplay
- Documented for easy customization
- Integration examples included

Copy, customize, and create your own networks!
