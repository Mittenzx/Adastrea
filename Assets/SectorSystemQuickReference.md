# Sector System - Quick Reference

One-page reference for the sector governance system.

## What is a Sector?

**Physical:** 200km x 200km x 200km region of space  
**Governance:** Managed by a Sector Council (practical administration, not politics)  
**Purpose:** Taxes, budget allocation, security, infrastructure, policies

## Core Components

### 1. SpaceSectorMap Actor (Physical Marker)

```cpp
Location: Sector center coordinates
Size: 20,000,000 units per side (200km)
Bounds: ±10,000,000 units from center

Functions:
  GetSectorBounds() -> FBox
  IsPositionInSector(Position) -> bool
  GetRandomPositionInSector() -> FVector
```

**Placement:** Multiples of 20,000,000 units apart

### 2. SectorCouncilDataAsset (Governance)

```cpp
Identity:
  SectorName, SectorID, Description

Council:
  TotalSeats (7-15), PassThreshold (51-70%)
  Representatives (Ways with voting weight)

Taxation:
  SecurityTaxRate (5-25%)
  InfrastructureTaxRate (3-15%)
  AdministrationTaxRate (1-10%)

Budget: (Must sum to 100%)
  SecurityBudgetPercent (20-80%)
  InfrastructureBudgetPercent (10-60%)
  EmergencyReservePercent (10-30%)
  TreasuryFunds

ActivePolicies:
  Array of sector-specific rules
```

## Available Templates

| Template | Theme | Tax | Security | Infrastructure | Best For |
|----------|-------|-----|----------|----------------|----------|
| **CoreSystemsSector** | Balanced | 23% | 30% | 55% | New players, safe zones |
| **FrontierMiningSector** | Dangerous | 23% | 70% | 15% | Mining, risk/reward |
| **TradeHubSector** | Commerce | 18% | 40% | 45% | Trading gameplay |
| **ResearchSector** | Scientific | 23% | 20% | 60% | Technology focus |
| **MilitaryBorderSector** | War Zone | 35% | 80% | 10% | Combat missions |
| **IndependentColoniesSector** | Freedom | 7% | 40% | 35% | Frontier, self-reliance |
| **ContestedTerritorySector** | Political | 30% | 55% | 25% | Intrigue, diplomacy |

## Quick Setup (5 Steps)

### Step 1: Choose Template
Pick from `Assets/SectorTemplates/` based on desired gameplay

### Step 2: Create Data Asset
```
Content/DataAssets/Sectors/ → Right-click
→ Blueprint Class → SectorCouncilDataAsset
→ Name: DA_Council_YourSectorName
```

### Step 3: Configure Identity
```
Sector Name: "Your Sector"
Sector ID: YourSectorID (unique!)
Description: Brief description
```

### Step 4: Set Council
```
Total Seats: 8-15
Pass Threshold: 51-70%

Add Representatives:
  - Select Way Data Asset
  - Set voting weight (10-25)
  - Set years of service
```

### Step 5: Configure Finances
```
Taxation:
  Security: 5-25%
  Infrastructure: 3-15%
  Administration: 1-10%

Budget (sum to 100%):
  Security: 20-80%
  Infrastructure: 10-60%
  Reserve: 10-30%
  
Treasury: 100K - 10M credits
```

## Common Values

### Council Configuration

| Attribute | Typical Range | Default |
|-----------|---------------|---------|
| Total Seats | 7-15 | 10 |
| Pass Threshold | 51-70% | 55% |
| Voting Weight (per rep) | 10-25 | 15 |

### Pass Threshold Guide

| Threshold | Type | Effect |
|-----------|------|--------|
| 51% | Simple majority | Quick decisions |
| 55-60% | Moderate | Balanced |
| 66% | Supermajority | Careful deliberation |
| 70%+ | Consensus | Gridlock likely |

### Tax Rates by Sector Type

| Sector Type | Security | Infrastructure | Admin | Total |
|-------------|----------|----------------|-------|-------|
| Safe Core | 5-8% | 8-12% | 3-5% | 16-25% |
| Frontier | 15-20% | 3-5% | 2-3% | 20-28% |
| Trade Hub | 5-8% | 6-10% | 2-4% | 13-22% |
| War Zone | 20-30% | 3-6% | 3-5% | 26-41% |

### Budget Allocation by Sector Type

| Sector Type | Security | Infrastructure | Reserve |
|-------------|----------|----------------|---------|
| Safe Core | 20-35% | 45-60% | 15-20% |
| Frontier | 60-75% | 10-20% | 15-20% |
| Trade Hub | 35-45% | 40-50% | 10-15% |
| Research | 15-25% | 55-70% | 15-20% |
| War Zone | 75-85% | 5-15% | 10-15% |

## Placement Reference

### Standard Grid Pattern

```
Sector at (0, 0, 0)         → Origin
Sector at (20000000, 0, 0)  → 200km East
Sector at (0, 20000000, 0)  → 200km North
Sector at (0, 0, 20000000)  → 200km Up
```

### Distance Calculator

```
1 sector apart  = 200 km  = 6.7 min at 500m/s
2 sectors apart = 400 km  = 13.3 min at 500m/s
3 sectors apart = 600 km  = 20 min at 500m/s
Diagonal        = 283 km  = 9.4 min at 500m/s
```

## Key Functions

### Council Functions

```cpp
// Membership
GetRepresentatives() -> TArray<FCouncilRepresentative>
HasRepresentative(Way) -> bool
GetTotalVotingWeight() -> int32

// Taxation
GetTotalTaxRate() -> int32
CalculateTaxOwed(Value) -> int64
CalculateSecurityTax(Value) -> int64

// Budget
GetSecurityBudget() -> int64
GetInfrastructureBudget() -> int64
GetEmergencyReserve() -> int64
IsBudgetBalanced() -> bool

// Policies
GetActivePolicies() -> TArray<FSectorPolicy>
HasActivePolicy(Type) -> bool

// Voting
SimulateVote(Policy, For, Against) -> FCouncilVote
WouldVotePass(VotesFor, VotesAgainst) -> bool
```

### Sector Map Functions

```cpp
// Boundaries
GetSectorBounds() -> FBox
GetSectorHalfSize() -> float (10,000,000)
GetSectorCenter() -> FVector

// Queries
IsPositionInSector(WorldPosition) -> bool
GetRandomPositionInSector() -> FVector
```

## Policy Types

```cpp
enum ESectorPolicyType
{
    SecurityBudget,      // Security funding level
    TradeRegulation,     // Trade rules/tariffs
    ResourceAllocation,  // Resource distribution
    InfrastructureFund,  // Facility maintenance
    EmergencyReserve,    // Emergency funding
    PatrolFrequency      // Security patrols
}
```

## Representative Structure

```cpp
struct FCouncilRepresentative
{
    UWayDataAsset* RepresentingWay;  // Which Way
    FText RepresentativeName;         // Individual name
    int32 VotingWeight;              // 10-25 influence
    int32 YearsOfService;            // Experience
    bool bIsPlayerControlled;        // Player seat?
}
```

## Validation Checklist

Before using a sector:

- [ ] Budget sums to 100%
- [ ] SectorID is unique
- [ ] All Way references valid
- [ ] Voting weights reasonable (10-25)
- [ ] Pass threshold 51-100%
- [ ] Tax rates ≤ 50% each
- [ ] Treasury funds positive
- [ ] SpaceSectorMap actor in level
- [ ] Names match between actor and Data Asset

## Common Patterns

### Safe Starting Zone
Tax: 15-20%, Security: 25-35%, Infrastructure: 50-60%, Cooperative council

### Dangerous Frontier
Tax: 20-25%, Security: 65-75%, Infrastructure: 10-15%, Pragmatic council

### Economic Hub
Tax: 15-20%, Security: 40%, Infrastructure: 45%, Merchant-dominated

### Political Powder Keg
Tax: 25-35%, Security: 55%, Infrastructure: 25%, Competing blocs, gridlock

### Military Zone
Tax: 30-40%, Security: 75-85%, Infrastructure: 10%, Military control

### Minimal Governance
Tax: 5-10%, Security: 40%, Infrastructure: 35%, High pass threshold

## Integration Points

**Way System:** Ways get council seats based on sector presence  
**Trading System:** Sector policies affect prices and trade  
**Combat System:** Security budget affects patrol presence  
**Quest System:** Sector issues generate missions  
**Faction System:** Factions may control sectors

## File Locations

**Templates:** `Assets/SectorTemplates/`  
**Data Assets:** `Content/DataAssets/Sectors/`  
**Documentation:**
- `Assets/SectorCreationGuide.md` - Complete guide
- `Assets/SectorMapPlacementGuide.md` - Placement guide
- `Assets/SectorTemplates/README.md` - Template docs

## Quick Commands

```cpp
// Blueprint usage
SectorCouncil->GetTotalTaxRate()
SectorCouncil->CalculateTaxOwed(TransactionValue)
SectorCouncil->IsBudgetBalanced()

SectorMapActor->GetSectorBounds()
SectorMapActor->IsPositionInSector(TestPosition)
SectorMapActor->GetRandomPositionInSector()
```

## Example Sector

```yaml
Identity:
  Sector Name: "Trading Post Alpha"
  Sector ID: TradingPostAlpha

Council:
  Total Seats: 8
  Pass Threshold: 55%
  Representatives:
    - Merchant Coalition (18 votes)
    - Freight Haulers (15 votes)
    - Station Services (12 votes)

Taxation:
  Security: 7%
  Infrastructure: 8%
  Administration: 3%
  Total: 18%

Budget:
  Security: 35%
  Infrastructure: 50%
  Reserve: 15%
  Treasury: 1,000,000 credits
```

## Tips

✓ Start with templates  
✓ Plan layout before placing  
✓ Use grid spacing (20M units)  
✓ Test budget sums to 100%  
✓ Match names exactly  
✓ Consider travel times  
✓ Document your layout  

✗ Don't overlap sectors  
✗ Don't use random positions  
✗ Don't forget Z axis  
✗ Don't change actor scale  
✗ Don't use duplicate IDs  

---

**See Also:**
- Full Guide: `Assets/SectorCreationGuide.md`
- Placement: `Assets/SectorMapPlacementGuide.md`
- Templates: `Assets/SectorTemplates/README.md`
- Way System: `Assets/WaySystemGuide.md`

**Version:** 1.0 | **Last Updated:** November 2024
