# Way System - Quick Reference

## At a Glance

| Component | Description | Example |
|-----------|-------------|---------|
| **Way** | Small specialized guild (50-1000 members) | Dwarven Mining Way, Engineers' Guild |
| **Network** | Micro-alliance of 2-5 Ways with shared Precepts | Craftsmen's Compact, Frontier Alliance |
| **Precept** | Core value that defines what a Way stands for | Craftsmanship, Discovery, Honor |
| **Feat** | Legendary accomplishment that grants reputation | "Master Ship Builder", "First Explorer" |
| **Spillover** | Reputation gain transferred to network members | 40% spillover = +40 rep to allies |
| **Network Bonus** | Reputation multiplier for aligned Feats | 1.8x bonus for crafting-aligned actions |

## Quick Commands

### Blueprint Nodes

```
// Register Network
Get Game Instance → Get Subsystem (VerseSubsystem) → Register Network

// Check Network Membership
Get Networks For Way → Returns array of networks

// Get Network Reputation
Get Network Verse Score → Returns player reputation (0-100)

// Check Network Benefits
Qualifies For Network Bonuses → Returns true/false

// Award Feat with Spillover
Record Feat With Network Effects → Auto-applies spillover
```

### Key Functions

| Function | Purpose | Returns |
|----------|---------|---------|
| `RegisterNetwork(Network)` | Add network to system | void |
| `GetNetworksForWay(Way)` | Find Way's networks | Array of Networks |
| `GetNetworkVerseScore(Network)` | Player's network rep | Float (0-100) |
| `QualifiesForNetworkBonuses(Network)` | Check threshold | Bool |
| `RecordFeatWithNetworkEffects(Feat, bApply)` | Award with spillover | Bool (success) |

## Configuration Values

### Way Properties

| Property | Range | Description | Typical Value |
|----------|-------|-------------|---------------|
| **Technology Level** | 1-10 | How advanced the Way is | 6 (standard) |
| **Military Strength** | 1-10 | Combat capability | 5 (moderate) |
| **Economic Power** | 1-10 | Wealth and resources | 7 (prosperous) |
| **Influence Power** | 1-10 | Political/social influence | 6 (notable) |
| **Member Count** | 50-5000+ | Number of guild members | 250 (small) |
| **Voting Weight** | 5-30 | Council voting power | 15 (standard) |

### Network Mechanics

| Setting | Range | Description | Recommended |
|---------|-------|-------------|-------------|
| **Spillover %** | 0-100% | Rep transferred to members | 30-50% |
| **Alignment Bonus** | 1.0-3.0x | Feat rep multiplier | 1.5-2.0x |
| **Minimum Threshold** | 0-100 | Rep needed for bonuses | 20-40 |
| **Member Count** | 2-5 | Ways in network | 3 |

### Quality Tiers

| Tier | Description | Price Multiplier |
|------|-------------|------------------|
| **Basic** | Cheap, functional | 0.5x |
| **Standard** | Average quality | 1.0x |
| **Quality** | Above average | 1.5x |
| **Premium** | High quality | 2.5x |
| **Elite** | Exceptional | 5.0x |
| **Legendary** | Best in class | 10.0x |

## Industry Types

| Industry | Common Roles | Typical Quality | Example Ways |
|----------|--------------|-----------------|--------------|
| **Mining** | Miners, Extractors | Quality | Dwarven Mining Way |
| **Manufacturing** | Craftsmen, Engineers | Premium-Elite | Pure Crafters |
| **Exploration** | Scouts, Cartographers | Standard-Quality | Scout's League |
| **Trading** | Merchants, Brokers | Standard | Merchant Coalition |
| **Shipbuilding** | Builders, Designers | Quality-Premium | Ship-Builders Union |
| **Research** | Scientists, Analysts | Premium-Elite | Scholars' Syndicate |
| **Security** | Guards, Enforcers | Quality | Protector's Order |
| **Medical** | Doctors, Medics | Premium | Medical Services |

## Common Precepts

### Crafting & Production
- **Craftsmanship** (90) - Excellence in creation
- **Mastery** (85) - Skill perfection
- **Innovation** (70) - Technological advancement
- **Efficiency** (75) - Optimal processes

### Exploration & Discovery
- **Discovery** (95) - Finding the unknown
- **Freedom** (80) - Independence
- **Curiosity** (85) - Desire to learn
- **Ambition** (70) - Reaching higher

### Combat & Honor
- **Honor** (95) - Ethical conduct
- **Strength** (85) - Power and might
- **Justice** (75) - Fairness
- **Loyalty** (80) - Faithfulness

### Trade & Prosperity
- **Prosperity** (90) - Wealth accumulation
- **Cunning** (70) - Strategic thinking
- **Negotiation** (75) - Deal-making
- **Efficiency** (80) - Profitability

## Example Networks

### Craftsmen's Compact (Engineering)
```yaml
Members: 3 (Engineers, Ship-Builders, Refinery)
Precepts: Craftsmanship (90), Mastery (85), Innovation (70)
Spillover: 40% | Bonus: 1.8x | Threshold: 30
```

### Frontier Alliance (Exploration)
```yaml
Members: 3 (Scouts, Cartographers, Pioneers)
Precepts: Discovery (95), Freedom (80), Survival (70)
Spillover: 35% | Bonus: 2.0x | Threshold: 25
```

### Honor Circle (Combat)
```yaml
Members: 4 (Warriors, Duelists, Protectors, Bounty Hunters)
Precepts: Honor (95), Strength (85), Justice (75)
Spillover: 30% | Bonus: 1.7x | Threshold: 35
```

### Merchant Coalition (Trade)
```yaml
Members: 3 (Traders, Brokers, Transport Union)
Precepts: Prosperity (90), Cunning (75), Efficiency (80)
Spillover: 35% | Bonus: 1.6x | Threshold: 30
```

### Scholars' Syndicate (Research)
```yaml
Members: 3 (Researchers, Archivists, Scientists)
Precepts: Innovation (90), Mastery (85), Discovery (80)
Spillover: 40% | Bonus: 2.0x | Threshold: 35
```

## Reputation Calculation

### Direct Feat Completion
```
Base Rep Gain = Feat Value (e.g., 50)
Precept Alignment = How well Feat matches Way Precepts (0-100%)
Final Rep = Base × Alignment%

Example:
  Feat: "Master Craftsman" (50 points)
  Way: Engineers' Guild (Craftsmanship: 90)
  Alignment: 90%
  Result: 50 × 0.9 = 45 reputation
```

### Network Spillover
```
Primary Way Rep = 45 (from above)
Network Spillover = 40%
Other Members Rep = 45 × 0.4 = 18 reputation each

Example:
  Engineers' Guild: +45 rep (direct)
  Ship-Builders Union: +18 rep (spillover)
  Refinery Collective: +18 rep (spillover)
```

### Network Bonus
```
If player qualifies for network bonuses:
  Base Rep = 45
  Network Bonus = 1.8x
  Final Rep = 45 × 1.8 = 81 reputation

Qualification requires:
  - Minimum threshold met (e.g., 30 rep with any member)
  - Network active in VerseSubsystem
  - Feat Precepts align with network
```

## Common Use Cases

### Create a Way (5 minutes)
1. Content Browser → Blueprint → WayDataAsset
2. Set Identity (name, ID, colors)
3. Configure Attributes (tech, military, economic, influence)
4. Define Specialization (industry, quality, member count)
5. Add Produced/Consumed Resources
6. Set Core Precepts (3-5 values)
7. Save

### Create a Network (5 minutes)
1. Content Browser → Blueprint → WayNetworkDataAsset
2. Set Network Name and ID
3. Add 2-5 Member Ways with Influence/Commitment
4. Define Shared Precepts (copy from members)
5. Configure Mechanics (spillover, bonus, threshold)
6. Save

### Register Network (Blueprint)
1. Event BeginPlay
2. Get Game Instance → Get Subsystem (VerseSubsystem)
3. Load Asset (your network)
4. Register Network

### Check Player Status (Blueprint)
1. Get VerseSubsystem
2. Get Network Verse Score (returns 0-100)
3. Qualifies For Network Bonuses (returns bool)
4. Branch: Apply benefits or deny

### Award Feat with Spillover (Blueprint)
1. Get VerseSubsystem
2. Record Feat With Network Effects
   - Feat To Record: Your Feat asset
   - Apply Network Effects: TRUE
3. Returns success/failure

## Troubleshooting

| Issue | Check | Solution |
|-------|-------|----------|
| **Network not showing** | Registered? | Call RegisterNetwork on game start |
| **Spillover not working** | Using correct function? | Use RecordFeatWithNetworkEffects |
| **Bonuses not applying** | Reputation threshold? | Ensure player has min rep with a member |
| **Network score zero** | Precepts configured? | Add Core Precepts to member Ways |
| **Reputation not gaining** | Feat alignment? | Verify Feat Precepts match Way values |

## File Locations

### Data Assets
- Ways: `Content/DataAssets/Ways/`
- Networks: `Content/DataAssets/Networks/`
- Feats: `Content/DataAssets/Feats/`

### Source Code
- Way: `Source/Adastrea/Way/Way.h`
- Network: `Source/Adastrea/Way/WayNetwork.h`
- Subsystem: `Source/Adastrea/Way/VerseSubsystem.h`
- Verse Component: `Source/Adastrea/Player/Verse.h`

### Documentation
- Main Guide: `Assets/WaySystemGuide.md`
- Overview: `Assets/WaySystemOverview.md`
- Creation: `Assets/WayCreationGuide.md`
- Network Guide: `Assets/WayNetworkGuide.md`
- Network Quick Start: `Assets/WayNetworkQuickStart.md`
- Network Workflow: `Assets/WayNetworkWorkflow.md`
- Blueprint Examples: `Blueprints/WayNetworkBlueprintExamples.md`

### Templates
- Network Templates: `Assets/WayNetworkTemplates/`

## Next Steps

1. **New to Ways?** → Read [WaySystemOverview.md](WaySystemOverview.md)
2. **Create First Way** → Follow [WayCreationGuide.md](WayCreationGuide.md)
3. **Create First Network** → Use [WayNetworkQuickStart.md](WayNetworkQuickStart.md)
4. **Blueprint Integration** → See [WayNetworkBlueprintExamples.md](../Blueprints/WayNetworkBlueprintExamples.md)
5. **Deep Dive** → Read [WaySystemGuide.md](WaySystemGuide.md)

---

**Quick Tip**: Start with the Craftsmen's Compact template—it's the easiest to understand and test!

**Version**: 1.0  
**Last Updated**: 2025-11-10
