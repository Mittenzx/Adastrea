# Sector Creation Guide

Complete guide for creating sectors with governance councils in Adastrea.

## Table of Contents

- [Overview](#overview)
- [What is a Sector?](#what-is-a-sector)
- [Quick Start (10 Minutes)](#quick-start-10-minutes)
- [Detailed Walkthrough](#detailed-walkthrough)
- [Design Patterns](#design-patterns)
- [Testing and Validation](#testing-and-validation)
- [Advanced Topics](#advanced-topics)

## Overview

Sectors are 200km x 200km regions of space governed by councils of Way representatives. Each sector has:

1. **Physical Presence:** `ASpaceSectorMap` actor marking the sector boundaries
2. **Governance:** `USectorCouncilDataAsset` defining the council and policies
3. **Content:** Stations, ships, asteroids, and other objects within the sector
4. **Economy:** Trading, production, and resource flows
5. **Security:** Patrol routes, threat levels, and protection

## What is a Sector?

### Physical Scale

- **Size:** 200km x 200km x 200km (200,000m x 200,000m x 200,000m)
- **Unreal Units:** 20,000,000 units per side
- **Scale:** Matches X4: Foundations for familiar open-world feel
- **Typical Contents:** 5-20 stations, 20-100 ships, resource fields

### Governance Model

Sectors are governed by **Sector Councils** - practical administrative bodies where Ways vote on:
- Tax rates for security, infrastructure, administration
- Budget allocation across security, infrastructure, emergency reserves
- Policies affecting trade, patrols, resource allocation
- Sector-wide concerns and emergencies

This is **NOT politics** - it's practical governance. Think city council, not parliament.

### Integration with Game Systems

Sectors integrate with:
- **Way System:** Ways operate in sectors, have council representation
- **Trading System:** Sector policies affect prices and trade
- **Combat System:** Security budgets affect patrol presence
- **Quest System:** Sector problems generate missions
- **Faction System:** Factions may control multiple sectors

## Quick Start (10 Minutes)

### Step 1: Choose Your Template

Pick a template from `Assets/SectorTemplates/` based on your desired gameplay:

| Template | Theme | Best For |
|----------|-------|----------|
| CoreSystemsSector | Safe, balanced | New players, tutorial |
| FrontierMiningSector | Dangerous, rewarding | Mining gameplay |
| TradeHubSector | Commerce focused | Trading gameplay |
| ResearchSector | Technology focused | Innovation gameplay |
| MilitaryBorderSector | Combat focused | Military gameplay |
| IndependentColoniesSector | Minimal governance | Frontier gameplay |
| ContestedTerritorySector | Political intrigue | Diplomacy gameplay |

### Step 2: Create Council Data Asset

**In Unreal Editor:**

1. Navigate to `Content/DataAssets/Sectors/`
2. Right-click → Miscellaneous → Data Asset (or search for "Data Asset")
3. Select `SectorCouncilDataAsset` as the Data Asset class
4. Name it: `DA_Council_YourSectorName`
5. Double-click to open

### Step 3: Configure Basic Identity

```
Identity Section:
  Sector Name: "Your Sector Name"
  Sector ID: YourSectorID (no spaces, unique identifier)
  Description: "Brief description of the sector"
```

### Step 4: Set Council Parameters

```
Council Section:
  Total Seats: 8-15 (typical)
  Pass Threshold: 51-70 (% needed to pass votes)
```

### Step 5: Add Representatives

For each Way operating in the sector:

1. Click `+` on Representatives array
2. **Representing Way:** Select Way Data Asset
3. **Representative Name:** Name of council member
4. **Voting Weight:** 10-25 (influence level)
5. **Years of Service:** 0-20 (experience)
6. **Is Player Controlled:** false (or true for player seat)

### Step 6: Configure Taxes and Budget

```
Taxation:
  Security Tax Rate: 5-25%
  Infrastructure Tax Rate: 3-15%
  Administration Tax Rate: 1-10%

Budget: (must sum to 100%)
  Security Budget Percent: 20-80%
  Infrastructure Budget Percent: 10-60%
  Emergency Reserve Percent: 10-30%
  Treasury Funds: 100,000 - 10,000,000
```

### Step 7: Add Policies (Optional)

1. Click `+` on Active Policies array
2. **Policy Type:** Select from dropdown
3. **Policy Name:** Descriptive name
4. **Description:** What the policy does
5. **Policy Value:** 0-100 (effectiveness/strength)
6. **Is Active:** true

### Step 8: Create Sector Map Actor

**In Level Editor:**

1. Place Actors → Search: `SpaceSectorMap`
2. Drag into level
3. Set **Sector Name** to match council name
4. Position at sector center coordinates
5. Use the actor as reference for placing content

### Step 9: Test in Play Mode

1. Hit Play
2. Check console log for sector initialization
3. Verify sector bounds with debug drawing
4. Test Way integration

## Detailed Walkthrough

### Understanding Sector Council Structure

#### Identity Section

**Purpose:** Basic information about the sector

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
FText SectorName;  // "Core Systems", "Frontier Mining"

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
FName SectorID;  // CoreSystems, FrontierMining (unique!)

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
FText Description;  // Multi-line description
```

**Design Tips:**
- Make SectorID unique across all sectors
- Use descriptive names that fit the theme
- Write descriptions that convey the sector's character

#### Council Membership

**Purpose:** Define who governs and how decisions are made

```cpp
TArray<FCouncilRepresentative> Representatives;
int32 TotalSeats;      // Number of council positions
int32 PassThreshold;   // % needed to pass (51-100)
```

**Representative Structure:**
```cpp
struct FCouncilRepresentative
{
    UWayDataAsset* RepresentingWay;  // Which Way they represent
    FText RepresentativeName;         // Individual's name
    int32 VotingWeight;              // 10-25 typical
    int32 YearsOfService;            // Experience level
    bool bIsPlayerControlled;        // Player seat?
}
```

**Voting Weight Guidelines:**

| Weight | Influence | Use For |
|--------|-----------|---------|
| 10-12 | Minor | Small Ways, new members |
| 13-16 | Moderate | Established Ways, medium influence |
| 17-20 | Major | Important Ways, experienced members |
| 21-25 | Dominant | Controlling Ways, leadership |

**Pass Threshold Guidelines:**

| Threshold | Type | Effect |
|-----------|------|--------|
| 51% | Simple majority | Quick decisions, less debate |
| 55-60% | Moderate majority | Balanced governance |
| 66% | Supermajority | Careful deliberation required |
| 70%+ | Near consensus | Gridlock likely, rare changes |

#### Taxation System

**Purpose:** Generate revenue for sector operations

```cpp
int32 SecurityTaxRate;        // 0-50% (typical: 5-20%)
int32 InfrastructureTaxRate;  // 0-50% (typical: 3-15%)
int32 AdministrationTaxRate;  // 0-50% (typical: 1-10%)
```

**Tax Rate Guidelines:**

| Sector Type | Security | Infrastructure | Admin | Total |
|-------------|----------|----------------|-------|-------|
| Safe Core | 5-8% | 8-12% | 3-5% | 16-25% |
| Frontier | 15-20% | 3-5% | 2-3% | 20-28% |
| Trade Hub | 5-8% | 6-10% | 2-4% | 13-22% |
| War Zone | 20-30% | 3-6% | 3-5% | 26-41% |
| Backwater | 3-5% | 1-3% | 1-2% | 5-10% |

**Tax Functions:**

```cpp
// Calculate total tax on a transaction
int64 CalculateTaxOwed(int64 TransactionValue);

// Calculate just security tax
int64 CalculateSecurityTax(int64 TransactionValue);

// Get total tax rate
int32 GetTotalTaxRate();
```

#### Budget Allocation

**Purpose:** Determine how tax revenue is spent

```cpp
int32 SecurityBudgetPercent;        // Must sum to 100%
int32 InfrastructureBudgetPercent;
int32 EmergencyReservePercent;
int64 TreasuryFunds;  // Current funds available
```

**Budget Guidelines:**

| Sector Type | Security | Infrastructure | Reserve |
|-------------|----------|----------------|---------|
| Safe Core | 20-35% | 45-60% | 15-20% |
| Frontier | 60-75% | 10-20% | 15-20% |
| Trade Hub | 35-45% | 40-50% | 10-15% |
| Research | 15-25% | 55-70% | 15-20% |
| War Zone | 75-85% | 5-15% | 10-15% |

**Budget Functions:**

```cpp
// Calculate allocated funds
int64 GetSecurityBudget();
int64 GetInfrastructureBudget();
int64 GetEmergencyReserve();

// Validate allocation
bool IsBudgetBalanced();  // Returns true if sums to 100%
```

#### Active Policies

**Purpose:** Sector-specific rules and initiatives

**Policy Types:**
```cpp
enum class ESectorPolicyType
{
    SecurityBudget,      // Security funding level
    TradeRegulation,     // Trade rules and tariffs
    ResourceAllocation,  // How resources are distributed
    InfrastructureFund,  // Station maintenance funding
    EmergencyReserve,    // Emergency response funding
    PatrolFrequency      // Security patrol scheduling
}
```

**Policy Structure:**
```cpp
struct FSectorPolicy
{
    ESectorPolicyType PolicyType;
    FText PolicyName;
    FText Description;
    int32 PolicyValue;     // 0-100 (strength/effectiveness)
    bool bIsActive;
    FDateTime EnactedDate;
}
```

**Example Policies:**

```yaml
# High Security
PolicyType: SecurityBudget
PolicyName: "Maximum Security Protocol"
Description: "All available resources directed to security forces"
PolicyValue: 95
IsActive: true

# Free Trade
PolicyType: TradeRegulation
PolicyName: "Open Markets Act"
Description: "Minimal restrictions on trade and commerce"
PolicyValue: 10
IsActive: true

# Infrastructure Investment
PolicyType: InfrastructureFund
PolicyName: "Station Renewal Program"
Description: "Major investment in upgrading facilities"
PolicyValue: 85
IsActive: true
```

### Creating the Sector Map Actor

The `ASpaceSectorMap` actor marks the physical sector in your level.

#### Placement Guidelines

**Sector Spacing:**
- **Adjacent Sectors:** 200km apart (exactly sector size)
- **Separated Sectors:** Multiples of 200km for clean boundaries
- **Overlapping:** Avoid - causes boundary confusion

**Example Layout:**
```
Sector A: (0, 0, 0)
Sector B: (20000000, 0, 0)      // 200km east
Sector C: (0, 20000000, 0)      // 200km north
Sector D: (20000000, 20000000, 0)  // 200km northeast
```

#### Configuration

```cpp
class ASpaceSectorMap
{
    FText SectorName;        // Must match council SectorName
    FText Description;       // Same as council or level-specific
    UBillboardComponent* SectorMarker;  // Editor visualization
    
    // Constants
    static constexpr float SectorSize = 20000000.0f;  // 200km
    
    // Functions
    FBox GetSectorBounds();
    bool IsPositionInSector(const FVector& WorldPosition);
    FVector GetRandomPositionInSector();
}
```

#### Content Population

Once the sector map is placed, populate it:

1. **Major Stations:** 3-8 large stations
2. **Minor Stations:** 5-15 small stations/platforms
3. **Ships:** 20-50 NPC ships
4. **Resources:** Asteroid fields, gas clouds
5. **Points of Interest:** Wrecks, anomalies, landmarks

**Use the sector actor as reference:**
```cpp
// Get random position within sector
FVector Position = SectorMapActor->GetRandomPositionInSector();

// Check if position is in sector
bool bInSector = SectorMapActor->IsPositionInSector(TestPosition);
```

## Design Patterns

### Pattern 1: Safe Starting Zone

**Use Case:** Tutorial, new player area

**Characteristics:**
- Low taxes (15-20%)
- Balanced budget
- High infrastructure (50-60%)
- Low security needs (25-35%)
- Cooperative council
- Moderate pass threshold (55-60%)

**Example:** Core Systems template

### Pattern 2: Dangerous Frontier

**Use Case:** Risk/reward gameplay, experienced players

**Characteristics:**
- Moderate taxes (20-25%)
- Security-focused budget (65-75%)
- Minimal infrastructure (10-15%)
- High pirate activity
- Pragmatic council
- Simple majority (51%)

**Example:** Frontier Mining template

### Pattern 3: Economic Hub

**Use Case:** Trading gameplay, commerce focus

**Characteristics:**
- Low taxes (15-20%)
- Balanced security/infrastructure (40/45)
- Merchant-dominated council
- Pro-business policies
- Cooperative dynamics
- Wealthy treasury

**Example:** Trade Hub template

### Pattern 4: Political Powder Keg

**Use Case:** Intrigue, diplomacy, faction gameplay

**Characteristics:**
- Moderate-high taxes (25-35%)
- Competing power blocs
- Deadlocked council
- High pass threshold (60-70%)
- Shifting alliances
- Player can be kingmaker

**Example:** Contested Territory template

### Pattern 5: Military Zone

**Use Case:** Combat focus, military missions

**Characteristics:**
- High taxes (30-40%)
- Extreme security focus (75-85%)
- Military-dominated council
- Strict regulations
- Emergency protocols
- Quick decisions (51%)

**Example:** Military Border template

### Pattern 6: Anarchist Paradise

**Use Case:** Minimal governance, self-reliance

**Characteristics:**
- Ultra-low taxes (5-10%)
- Weak central authority
- High decision threshold (70%+)
- Volunteer security
- Maximum autonomy
- Small treasury

**Example:** Independent Colonies template

## Testing and Validation

### Validation Checklist

**Data Asset Validation:**
- [ ] Budget allocation sums to 100%
- [ ] SectorID is unique
- [ ] All Way references are valid Data Assets
- [ ] Representative voting weights are reasonable (10-25)
- [ ] Pass threshold is 51-100%
- [ ] Tax rates don't exceed 50% each
- [ ] Treasury funds are positive
- [ ] Policy types match enum values

**Gameplay Validation:**
- [ ] Sector theme is clear and consistent
- [ ] Council composition matches sector type
- [ ] Policies align with sector priorities
- [ ] Tax rates feel appropriate for the area
- [ ] Budget allocation supports gameplay goals
- [ ] Player has opportunities for influence
- [ ] Integrates well with neighboring sectors

**Technical Validation:**
- [ ] Sector Map actor placed in level
- [ ] Sector Name matches between actor and council
- [ ] No overlapping sector boundaries
- [ ] Sector is populated with appropriate content
- [ ] Ways referenced in council exist in game
- [ ] Sector appears in sector selection UI

### Testing Procedure

1. **Create Data Asset:** Follow Quick Start guide
2. **Place Actor:** Add SpaceSectorMap to level
3. **Enter Play Mode:** Check console for errors
4. **Test Functions:**
   ```cpp
   // Blueprint or C++
   GetSectorBounds()
   GetTotalTaxRate()
   IsBudgetBalanced()
   GetRepresentatives()
   ```
5. **Verify Integration:** Test with Way system
6. **Gameplay Test:** Experience the sector as a player

### Common Issues

**Issue:** Budget doesn't sum to 100%
**Solution:** Adjust percentages so Security + Infrastructure + Reserve = 100

**Issue:** Council can't pass any votes
**Solution:** Lower pass threshold or adjust voting weights

**Issue:** Sector feels empty
**Solution:** Add more stations and ships using the sector bounds

**Issue:** Taxes too high/low
**Solution:** Compare to template examples, adjust to sector type

## Advanced Topics

### Dynamic Sector Control

Sectors can change hands between factions/Ways over time:

```cpp
// Track sector control
struct FSectorControl
{
    FName SectorID;
    UWayDataAsset* ControllingWay;
    float ControlPercentage;  // 0-100
    TArray<UWayDataAsset*> ContestingWays;
}
```

**Implementation ideas:**
- Council composition shifts as Ways gain/lose influence
- Voting weights adjust based on sector presence
- Policies change as control shifts
- Player actions affect sector control

### Interconnected Sectors

Create networks of related sectors:

**Trade Routes:**
- Link sectors with established trade lanes
- Patrol coverage extends between sectors
- Economic dependencies create supply chains

**Security Zones:**
- Border sectors share security responsibilities
- Joint patrols across boundaries
- Threat intelligence sharing

**Economic Regions:**
- Multiple sectors form economic bloc
- Shared currency or trade agreements
- Coordinated policies

### Procedural Sector Generation

Generate sectors procedurally:

1. **Choose archetype** (safe, frontier, trade, etc.)
2. **Roll random values** within archetype ranges
3. **Generate representative names**
4. **Create appropriate policies**
5. **Balance budget automatically**
6. **Populate with content**

### Sector Events

Dynamic events that affect sectors:

**Emergency Events:**
- Pirate raids (increase security budget)
- Disease outbreaks (redirect to medical)
- Economic crashes (reduce treasury)
- Infrastructure failures (emergency repairs)

**Positive Events:**
- Resource discoveries (increase wealth)
- Trade booms (lower taxes possible)
- Technological breakthroughs (better facilities)
- Diplomatic victories (reduce security needs)

**Council Events:**
- Elections (new representatives)
- Policy votes (activate/deactivate policies)
- Budget crises (reallocation required)
- Corruption scandals (affect trust)

### Integration with Quest System

Sectors generate quests based on their state:

**Security-focused sectors:**
- Patrol missions
- Pirate hunting
- Escort quests
- Security contracts

**Trade-focused sectors:**
- Delivery missions
- Trade negotiations
- Price arbitrage opportunities
- Market stabilization

**Infrastructure-focused sectors:**
- Station repair missions
- Construction contracts
- Resource gathering
- Equipment installation

**Political sectors:**
- Diplomatic missions
- Vote swaying
- Espionage
- Alliance building

## Best Practices

### Do:
✓ Create sectors with clear, distinct identities  
✓ Match council composition to sector theme  
✓ Balance taxes and budgets logically  
✓ Test gameplay experience thoroughly  
✓ Document unusual configurations  
✓ Leave room for player influence  
✓ Consider neighboring sectors  
✓ Use templates as starting points  

### Don't:
✗ Make all sectors the same  
✗ Create unwinnable governance (gridlock with no escape)  
✗ Ignore budget balance (must sum to 100%)  
✗ Forget to place SpaceSectorMap actor  
✗ Use duplicate SectorIDs  
✗ Make taxes absurdly high (>50% total is extreme)  
✗ Neglect testing with actual gameplay  
✗ Create sectors in isolation  

## Quick Reference

### Typical Sector Specs

| Attribute | Typical Range | Default |
|-----------|---------------|---------|
| Total Seats | 7-15 | 10 |
| Pass Threshold | 51-70% | 55% |
| Voting Weight | 10-25 | 15 |
| Security Tax | 5-25% | 10% |
| Infrastructure Tax | 3-15% | 7% |
| Admin Tax | 1-10% | 3% |
| Security Budget | 20-80% | 40% |
| Infrastructure Budget | 10-60% | 40% |
| Reserve Budget | 10-30% | 20% |
| Treasury | 100K-10M | 1M |

### Council Function Reference

```cpp
// Membership
GetRepresentatives() -> TArray<FCouncilRepresentative>
GetRepresentativeForWay(Way) -> FCouncilRepresentative
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
GetPolicyByType(Type) -> FSectorPolicy
HasActivePolicy(Type) -> bool

// Voting
SimulateVote(Policy, For, Against) -> FCouncilVote
WouldVotePass(VotesFor, VotesAgainst) -> bool
```

## Next Steps

1. **Review Templates:** Study the 7 provided templates
2. **Choose Theme:** Pick a sector type that fits your game area
3. **Create Council:** Follow Quick Start guide
4. **Place Actor:** Add SpaceSectorMap to level
5. **Populate:** Add stations and content
6. **Test:** Play through the sector
7. **Iterate:** Refine based on gameplay feel

## Additional Resources

- **Templates:** `Assets/SectorTemplates/`
- **Way System:** `Assets/WaySystemGuide.md`
- **Empty Map Setup:** `Docs/EmptyMapSetup.md`
- **Station Management:** `Assets/StationManagementGuide.md`
- **Trading Integration:** `Assets/TradingSystemGuide.md`

---

**Last Updated:** November 2024  
**Version:** 1.0  
**Maintained by:** Adastrea Development Team
