# Sector Council Templates

This directory contains YAML templates for creating Sector Council Data Assets. These templates provide ready-to-use configurations for different types of sector governance.

## Overview

Sector Councils are the practical governance bodies for 200km x 200km regions of space. Each sector has unique characteristics based on its location, economy, and the Ways that operate there.

## Available Templates

### 1. Core Systems Sector (`CoreSystemsSector.yaml`)
**Theme:** Established, balanced governance  
**Tax Rate:** 23% (balanced)  
**Security:** 30% (moderate - safe area)  
**Infrastructure:** 55% (high - well maintained)  
**Best For:** New player starting areas, tutorial zones, stable economy

**Key Features:**
- Mature, cooperative council
- High infrastructure investment
- Low crime and danger
- Supermajority (60%) required for changes
- Prosperous and well-organized

---

### 2. Frontier Mining Sector (`FrontierMiningSector.yaml`)
**Theme:** Rough mining territory  
**Tax Rate:** 23% (security-heavy)  
**Security:** 70% (high - pirate threats)  
**Infrastructure:** 15% (minimal)  
**Best For:** Mining gameplay, risk/reward scenarios

**Key Features:**
- Pragmatic, survival-focused council
- High security spending due to pirates
- Low infrastructure - bare minimum
- Quick decision making (51% majority)
- Independent, self-reliant culture

---

### 3. Trade Hub Sector (`TradeHubSector.yaml`)
**Theme:** Bustling commerce center  
**Tax Rate:** 18% (lowest overall)  
**Security:** 40% (moderate - protect trade)  
**Infrastructure:** 45% (high - trade facilities)  
**Best For:** Trading gameplay, economic focus

**Key Features:**
- Merchant-dominated council
- Excellent trade infrastructure
- Low taxes to encourage trade volume
- Wealthy treasury from commerce
- Cooperative council dynamics

---

### 4. Research Sector (`ResearchSector.yaml`)
**Theme:** Scientific research hub  
**Tax Rate:** 23% (infrastructure-heavy)  
**Security:** 20% (low - peaceful)  
**Infrastructure:** 60% (highest - research facilities)  
**Best For:** Technology focus, innovation, experimental gameplay

**Key Features:**
- Scholar-dominated council
- State-of-the-art facilities
- Research grant programs
- High decision threshold (66%) - careful consideration
- Intellectual property focus

---

### 5. Military Border Sector (`MilitaryBorderSector.yaml`)
**Theme:** Fortified border zone  
**Tax Rate:** 35% (highest)  
**Security:** 80% (highest - military operations)  
**Infrastructure:** 10% (military only)  
**Best For:** Combat focus, military missions, tension

**Key Features:**
- Military-dominated council
- Extreme security focus
- Emergency protocols active
- Strict regulations and inspections
- High-tension environment

---

### 6. Independent Colonies Sector (`IndependentColoniesSector.yaml`)
**Theme:** Minimal governance  
**Tax Rate:** 7% (ultra-low)  
**Security:** 40% (volunteer militia)  
**Infrastructure:** 35% (DIY)  
**Best For:** Libertarian gameplay, self-reliance, frontier experience

**Key Features:**
- Weak central authority by design
- Maximum autonomy for stations
- Volunteer defense forces
- High threshold (70%) - gridlock by design
- Self-reliant culture

---

### 7. Contested Territory Sector (`ContestedTerritorySector.yaml`)
**Theme:** Political instability  
**Tax Rate:** 30% (high)  
**Security:** 55% (internal + external threats)  
**Infrastructure:** 25% (neglected)  
**Best For:** Political intrigue, faction gameplay, diplomacy

**Key Features:**
- Competing power blocs
- Political gridlock
- Shifting alliances
- Player can be tie-breaker
- High drama and intrigue

## Quick Start Guide

### Option 1: Use Template Directly in Unreal

1. **Create Data Asset:**
   - In Unreal Editor, navigate to `Content/DataAssets/Sectors/`
   - Right-click → Miscellaneous → Data Asset → Data Asset Class: `SectorCouncilDataAsset`
   - Name: `DA_Council_SectorName` (e.g., `DA_Council_CoreSystems`)

2. **Copy Values from YAML:**
   - Open the `.yaml` template file
   - Copy each value into the corresponding property in the Data Asset
   - Save the Data Asset

3. **Create Sector Actor:**
   - In your level, add an `ASpaceSectorMap` actor
   - Set the `SectorName` to match your council
   - Set the `SectorID` to match your council's `SectorID`
   - Position it where you want the sector center

### Option 2: Use YAML Import Script (Future)

```bash
# Will be implemented in future update
python Tools/YAMLtoDataAsset.py --type SectorCouncil --input CoreSystemsSector.yaml
```

## Customization Tips

### Adjusting Tax Rates

```yaml
Taxation:
  SecurityTaxRate: 8        # 0-50% - Protection/policing
  InfrastructureTaxRate: 10 # 0-50% - Maintenance/facilities  
  AdministrationTaxRate: 5  # 0-50% - Government operations
```

**Total tax rate** = Sum of all three rates  
**Typical range**: 15-35%

### Budget Allocation

```yaml
Budget:
  SecurityBudgetPercent: 30      # Must sum to 100%
  InfrastructureBudgetPercent: 55
  EmergencyReservePercent: 15
```

**Must always sum to 100%**

### Council Voting

```yaml
Council:
  TotalSeats: 12            # Number of council positions
  PassThreshold: 60         # % of votes needed to pass (51-100)
```

- **51%**: Simple majority - quick decisions
- **60%**: Moderate supermajority - balanced
- **66%**: High supermajority - careful deliberation
- **70%+**: Near consensus - gridlock likely

### Representative Voting Weight

```yaml
Representatives:
  - VotingWeight: 18  # Typical range: 10-25
    YearsOfService: 12  # Experience on council
```

**Voting weight guidelines:**
- **10-12**: Minor Ways, new members
- **13-16**: Moderate influence
- **17-20**: Major Ways, experienced
- **21-25**: Dominant players, leadership

## Design Philosophy

### Sector Types by Player Experience

**New Players:**
- Core Systems (safe, balanced)
- Trade Hub (economic focus)

**Intermediate Players:**
- Frontier Mining (risk/reward)
- Research Sector (tech focus)

**Advanced Players:**
- Military Border (combat)
- Independent Colonies (self-reliance)
- Contested Territory (political)

### Creating Compelling Sectors

1. **Clear Identity:** Each sector should have a distinct theme
2. **Logical Governance:** Council composition matches sector type
3. **Balanced Tensions:** Mix cooperation and conflict
4. **Player Agency:** Room for player influence
5. **Economic Logic:** Tax and budget make sense for the area

### Common Sector Archetypes

| Archetype | Tax | Security | Infrastructure | Mood |
|-----------|-----|----------|----------------|------|
| **Safe Haven** | Moderate | Low | High | Calm, prosperous |
| **Frontier** | Low-Moderate | High | Low | Rough, opportunity |
| **Commerce Hub** | Low | Moderate | High | Bustling, wealthy |
| **War Zone** | High | Very High | Low | Tense, dangerous |
| **Backwater** | Very Low | Low | Low | Isolated, lawless |
| **Academic** | Moderate | Low | Very High | Intellectual, peaceful |
| **Industrial** | Moderate | Moderate | Moderate | Working-class, grimy |

## Integration with Way System

Sector Councils work closely with the Way System:

1. **Representation:** Ways with presence in a sector get council seats
2. **Voting Weight:** Based on Way's influence in that sector  
3. **Trade Impact:** Council policies affect Way economies
4. **Political Gameplay:** Ways compete for influence in contested sectors
5. **Dynamic Control:** Sector governance can shift as Ways gain/lose power

## Testing Your Sector

After creating a sector council:

1. **Balance Check:**
   - Does budget allocation sum to 100%?
   - Do representative voting weights make sense?
   - Is pass threshold appropriate for the governance style?

2. **Thematic Check:**
   - Does the council composition match the sector theme?
   - Do policies reflect the sector's priorities?
   - Are tax rates and budgets logical for the area?

3. **Gameplay Check:**
   - Does the sector create interesting player choices?
   - Are there opportunities for player influence?
   - Does it integrate well with nearby sectors?

4. **Technical Check:**
   - Is SectorID unique across all sectors?
   - Are all Way references valid?
   - Do policy types match the enum values?

## Next Steps

1. Review the templates and choose one that fits your needs
2. Create the Data Asset in Unreal Editor
3. Place a SpaceSectorMap actor in your level
4. Populate the sector with stations, ships, and content
5. Test the gameplay experience

For more information:
- **Way System:** See `Assets/WaySystemGuide.md`
- **Space Sectors:** See `Docs/EmptyMapSetup.md`
- **Station Placement:** See `Assets/StationManagementGuide.md`

## Contributing New Templates

When creating new sector templates:

1. Follow the YAML structure of existing templates
2. Include comprehensive notes about intended gameplay
3. Provide council dynamics description
4. Test the balance in-game
5. Submit with example use cases

---

**Last Updated:** November 2024  
**Version:** 1.0  
**Maintained by:** Adastrea Development Team
