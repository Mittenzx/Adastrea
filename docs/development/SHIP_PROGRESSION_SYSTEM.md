# Ship Progression System for Trade Simulator MVP

**Purpose**: Define the ship upgrade path for Trade Simulator MVP, providing clear progression for players.

**Design Goal**: "Buy low, sell high, upgrade ship, repeat with better margins"

---

## Overview

### Progression Philosophy

**Core Loop**:
```
Start with small ship (10 cargo)
   ↓
Make profitable trades
   ↓
Earn enough credits
   ↓
Upgrade to medium ship (30 cargo)
   ↓
More profitable per run
   ↓
Upgrade to large ship (100 cargo)
   ↓
Maximum profit per run
```

**Key Metrics**:
- **Time to Medium Ship**: 15-20 minutes of gameplay
- **Time to Large Ship**: 30-45 minutes of gameplay
- **Upgrade Cost** should feel achievable but meaningful

---

## Ship Tier Definitions

### Tier 1: Compact Trader (Starter Ship)

**Display Name**: "Compact Trader"  
**Internal ID**: `ship_starter_compact`  
**Cost**: **0cr** (starting ship)  
**Availability**: Given to player at game start

**Stats**:
```yaml
cargo_capacity: 10 units
max_speed: 300 m/s
acceleration: 200 m/s²
turn_rate: 90 deg/s
fuel_capacity: 100 units
fuel_efficiency: 1.0 (standard)
```

**Economic Profile**:
- **Profit per full trade**: 840-1,200cr (depending on route)
- **Trades needed for upgrade**: 40-60 trades to reach 50,000cr
- **Time to upgrade**: 15-20 minutes

**Strengths**:
- Fast and nimble (good for beginners)
- Low operating costs
- Easy to dock

**Weaknesses**:
- Very limited cargo space
- Low profit margins per run
- Can't carry high-value bulky items

**Best Strategy**:
- Focus on high-value, low-volume goods (medicine, electronics)
- Quick runs with small profits
- Learn market prices and routes

---

### Tier 2: Merchant Hauler (Mid-Tier Ship)

**Display Name**: "Merchant Hauler"  
**Internal ID**: `ship_medium_hauler`  
**Cost**: **50,000cr**  
**Availability**: Unlock after earning 50,000cr total

**Stats**:
```yaml
cargo_capacity: 30 units
max_speed: 250 m/s (-17% vs starter)
acceleration: 150 m/s² (-25% vs starter)
turn_rate: 60 deg/s (-33% vs starter)
fuel_capacity: 200 units (+100% vs starter)
fuel_efficiency: 0.9 (10% better)
```

**Economic Profile**:
- **Profit per full trade**: 2,520-3,600cr (3x starter ship)
- **Trades needed for next upgrade**: 70-80 trades to reach 250,000cr
- **Time to upgrade**: 25-35 minutes

**Strengths**:
- 3x cargo capacity (3x profit potential)
- Better fuel efficiency (longer routes)
- Can carry bulky goods

**Weaknesses**:
- Slower than starter ship
- Less maneuverable
- Higher docking difficulty

**Best Strategy**:
- Focus on volume trades (food, water, fuel)
- Longer routes with higher profits
- Optimize cargo mix for maximum profit

**Upgrade Investment Return**:
- Cost: 50,000cr
- Extra profit per trade: ~2,000cr
- Break-even: 25 trades (~8-10 minutes)
- After break-even, pure profit acceleration

---

### Tier 3: Trade Liner (Advanced Ship)

**Display Name**: "Trade Liner"  
**Internal ID**: `ship_large_liner`  
**Cost**: **250,000cr**  
**Availability**: Unlock after earning 250,000cr total (or owning Merchant Hauler)

**Stats**:
```yaml
cargo_capacity: 100 units
max_speed: 350 m/s (+17% vs starter, +40% vs medium)
acceleration: 180 m/s² (-10% vs starter, +20% vs medium)
turn_rate: 45 deg/s (-50% vs starter, -25% vs medium)
fuel_capacity: 500 units (+400% vs starter, +150% vs medium)
fuel_efficiency: 0.8 (20% better than starter)
shields: Basic deflector shields (new!)
```

**Economic Profile**:
- **Profit per full trade**: 8,400-12,000cr (10x starter ship!)
- **Maximum earning potential**: 12,000cr per 3-minute run = 240,000cr/hour
- **Endgame ship**: No further upgrades in MVP

**Strengths**:
- Massive cargo capacity (100 units)
- Fastest speed (fast runs)
- Best fuel efficiency
- Shields (protection feature)

**Weaknesses**:
- Large and harder to maneuver
- Expensive to repair (if combat added post-MVP)
- Requires good piloting skills for docking

**Best Strategy**:
- Mix high-value and high-volume goods
- Optimize routes for maximum profit/time
- Long-distance runs (more stations in future)

**Upgrade Investment Return**:
- Cost: 250,000cr
- Extra profit per trade: ~8,000cr vs medium
- Break-even: 31 trades (~12-15 minutes)
- Unlock "endgame" status

---

## Upgrade Unlock System

### Implementation: Simple Credit Threshold

```cpp
// In PlayerTraderComponent or GameInstance

bool CanUpgradeToShip(FName ShipID)
{
    int32 TotalEarnings = GetTotalLifetimeEarnings();
    
    if (ShipID == "ship_medium_hauler")
    {
        return TotalEarnings >= 50000;
    }
    else if (ShipID == "ship_large_liner")
    {
        return TotalEarnings >= 250000;
    }
    
    return false;
}
```

### Progression Milestones

| Milestone | Credits Earned | Ship Unlocked | Cargo | Expected Time |
|-----------|----------------|---------------|-------|---------------|
| Start | 0cr | Compact Trader | 10 | 0 min |
| First Upgrade | 50,000cr | Merchant Hauler | 30 | 15-20 min |
| Max Upgrade | 250,000cr | Trade Liner | 100 | 45-60 min |

---

## Visual Customization (Optional - Post-MVP)

### Tier 1: Compact Trader
- **Color Scheme**: Basic gray/white
- **Hull**: Small, boxy
- **Lights**: Minimal (nav lights only)
- **Decals**: None

### Tier 2: Merchant Hauler
- **Color Scheme**: Player choice (5 options)
- **Hull**: Medium, cargo containers visible
- **Lights**: Nav + cargo bay lights
- **Decals**: Company logo slot (optional)

### Tier 3: Trade Liner
- **Color Scheme**: Premium colors (10 options)
- **Hull**: Large, sleek, professional
- **Lights**: Full lighting (nav, cargo, engine glow)
- **Decals**: 2 logo slots + custom text

---

## Ship Upgrade UI

### Shipyard/Upgrade Interface

**Location**: At any station with marketplace (reuse docking UI)

**UI Elements**:
```
Shipyard Upgrade Menu
├─ Current Ship Display
│   └─ Ship name, stats, cargo capacity
├─ Available Upgrades (list)
│   ├─ Merchant Hauler (50,000cr)
│   │   ├─ Stats comparison
│   │   ├─ "Unlock" or "Locked" button
│   │   └─ Requirements: "Earn 50,000cr total"
│   └─ Trade Liner (250,000cr)
│       ├─ Stats comparison
│       ├─ "Unlock" or "Locked" button
│       └─ Requirements: "Earn 250,000cr total"
└─ [Back] button
```

### Stat Comparison Display

**Example**:
```
Merchant Hauler vs Compact Trader:

Cargo Capacity:  10 → 30 units (+200%) ✅
Max Speed:       300 → 250 m/s (-17%) ❌
Fuel Capacity:   100 → 200 units (+100%) ✅
Fuel Efficiency: 1.0 → 0.9 (+10%) ✅

Profit Potential: 3x per trade ✅
Cost: 50,000cr

[Upgrade Now] or [Not Yet]
```

---

## Balancing Considerations

### Progression Pace

**Target**: Players should feel constant progression

**Check**:
- First 5 minutes: Learn controls, make first trade
- 10 minutes: Multiple successful trades, understand markets
- 15 minutes: Ready for first upgrade (feeling accomplished)
- 30 minutes: Own medium ship, higher profits (feeling powerful)
- 45 minutes: Working toward large ship (endgame in sight)
- 60 minutes: Own large ship, maximum profits (MVP complete)

### Profit Scaling

**Formula**: Profit scales linearly with cargo capacity

```
Compact Trader:   10 cargo × 84cr/unit = 840cr/trade
Merchant Hauler:  30 cargo × 84cr/unit = 2,520cr/trade
Trade Liner:     100 cargo × 84cr/unit = 8,400cr/trade
```

**Why Linear?**
- Simple to understand
- Predictable progression
- No exponential runaway

### Upgrade Cost Justification

**Merchant Hauler (50,000cr)**:
- 60 trades @ 840cr/trade = 50,400cr
- OR: 20 trades @ 2,500cr/trade = 50,000cr
- **Time**: 15-20 minutes

**Trade Liner (250,000cr)**:
- 100 trades @ 2,500cr/trade = 250,000cr
- OR: 30 trades @ 8,400cr/trade = 252,000cr
- **Time**: 25-35 minutes (after owning medium ship)

**Total Time to Max**: 40-55 minutes (within 60-minute demo target!)

---

## Ship Data Asset Configuration

### Compact Trader Data Asset

```yaml
# Content/DataAssets/Ships/DA_Ship_CompactTrader.uasset (YAML for reference)

basic_info:
  display_name: "Compact Trader"
  description: "A small, nimble trading vessel perfect for beginners. Limited cargo space but easy to handle."
  ship_id: "ship_starter_compact"
  manufacturer: "StarTrader Inc."
  ship_class: "Light Freighter"
  
core_stats:
  hull_strength: 100.0
  shield_capacity: 0.0 (no shields)
  power_output: 500.0
  crew_capacity: 1
  
cargo_stats:
  cargo_capacity: 10 # UNITS
  cargo_volume: 10.0 # CUBIC METERS
  max_cargo_weight: 1000.0 # KG
  
mobility_stats:
  max_speed: 300.0 # M/S
  acceleration: 200.0 # M/S²
  turn_rate: 90.0 # DEG/S
  
utility_stats:
  fuel_capacity: 100.0
  fuel_consumption: 1.0 (standard)
  sensor_range: 5000.0
  
operational_stats:
  purchase_cost: 0 (starter ship)
  maintenance_cost: 10.0 # PER HOUR
  insurance_cost: 50.0 # PER DAY
  
unlock_requirements:
  required_level: 0
  required_credits: 0
  required_reputation: 0
```

### Merchant Hauler Data Asset

```yaml
# Content/DataAssets/Ships/DA_Ship_MerchantHauler.uasset (YAML for reference)

basic_info:
  display_name: "Merchant Hauler"
  description: "A mid-sized cargo vessel designed for profitable trading. Three times the cargo capacity of starter ships."
  ship_id: "ship_medium_hauler"
  manufacturer: "Galactic Freight Co."
  ship_class: "Medium Freighter"
  
core_stats:
  hull_strength: 250.0
  shield_capacity: 0.0 (no shields)
  power_output: 800.0
  crew_capacity: 2
  
cargo_stats:
  cargo_capacity: 30 # UNITS
  cargo_volume: 30.0 # CUBIC METERS
  max_cargo_weight: 3000.0 # KG
  
mobility_stats:
  max_speed: 250.0 # M/S
  acceleration: 150.0 # M/S²
  turn_rate: 60.0 # DEG/S
  
utility_stats:
  fuel_capacity: 200.0
  fuel_consumption: 0.9 (10% better)
  sensor_range: 7000.0
  
operational_stats:
  purchase_cost: 50000 # CREDITS
  maintenance_cost: 30.0 # PER HOUR
  insurance_cost: 150.0 # PER DAY
  
unlock_requirements:
  required_level: 0
  required_credits: 50000 # TOTAL LIFETIME EARNINGS
  required_reputation: 0
```

### Trade Liner Data Asset

```yaml
# Content/DataAssets/Ships/DA_Ship_TradeLiner.uasset (YAML for reference)

basic_info:
  display_name: "Trade Liner"
  description: "The ultimate trading vessel. Massive cargo capacity, excellent speed, and basic shielding. The dream of every trader."
  ship_id: "ship_large_liner"
  manufacturer: "Apex Logistics"
  ship_class: "Heavy Freighter"
  
core_stats:
  hull_strength: 500.0
  shield_capacity: 200.0 # HAS SHIELDS!
  power_output: 1500.0
  crew_capacity: 5
  
cargo_stats:
  cargo_capacity: 100 # UNITS
  cargo_volume: 100.0 # CUBIC METERS
  max_cargo_weight: 10000.0 # KG
  
mobility_stats:
  max_speed: 350.0 # M/S (FASTEST!)
  acceleration: 180.0 # M/S²
  turn_rate: 45.0 # DEG/S
  
utility_stats:
  fuel_capacity: 500.0
  fuel_consumption: 0.8 (20% better)
  sensor_range: 10000.0
  
operational_stats:
  purchase_cost: 250000 # CREDITS
  maintenance_cost: 100.0 # PER HOUR
  insurance_cost: 500.0 # PER DAY
  
unlock_requirements:
  required_level: 0
  required_credits: 250000 # TOTAL LIFETIME EARNINGS
  required_reputation: 0
```

---

## Implementation Checklist

### Phase 1: Data Assets (30 min)
- [ ] Create `DA_Ship_CompactTrader` in Unreal
- [ ] Create `DA_Ship_MerchantHauler` in Unreal
- [ ] Create `DA_Ship_TradeLiner` in Unreal
- [ ] Configure all properties per YAML specs
- [ ] Test loading in editor (no errors)

### Phase 2: Unlock System (45 min)
- [ ] Add `TotalLifetimeEarnings` to PlayerTraderComponent
- [ ] Implement `CanUpgradeToShip()` function
- [ ] Add event `OnShipUnlocked` delegate
- [ ] Test unlock logic with debug commands

### Phase 3: Upgrade UI (60 min)
- [ ] Create `WBP_ShipyardUI` widget
- [ ] Display current ship stats
- [ ] Display available upgrades
- [ ] Implement "Upgrade" button logic
- [ ] Add stat comparison display
- [ ] Test upgrade flow in-game

### Phase 4: Ship Switching (30 min)
- [ ] Implement `SwitchShip()` function
- [ ] Persist ship choice in save game
- [ ] Transfer cargo between ships
- [ ] Update UI after switch
- [ ] Test switching multiple times

### Phase 5: Balance Testing (60 min)
- [ ] Playtest: Time to first upgrade (target: 15-20 min)
- [ ] Playtest: Time to second upgrade (target: 25-35 min after first)
- [ ] Verify profit scaling (3x, 10x)
- [ ] Adjust costs if progression too fast/slow
- [ ] Re-test after adjustments

---

## Testing Scenarios

### Test 1: First Upgrade Path

**Goal**: Verify 15-20 minute progression to Merchant Hauler

**Steps**:
1. Start new game
2. Make 40-60 trades (water route)
3. Reach 50,000cr
4. Check if "Merchant Hauler" unlocks
5. Purchase and switch ships
6. Verify 3x cargo capacity

**Success Criteria**:
- Time: 15-20 minutes
- Clear progression feel
- Upgrade feels rewarding

### Test 2: Full Progression

**Goal**: Verify 45-60 minute progression to Trade Liner

**Steps**:
1. Start new game
2. Reach Merchant Hauler (15-20 min)
3. Make 70-80 trades with medium ship
4. Reach 250,000cr (25-35 min additional)
5. Unlock Trade Liner
6. Purchase and switch
7. Verify 10x profit vs starter

**Success Criteria**:
- Total time: 45-60 minutes
- Smooth progression curve
- Each upgrade feels meaningful

### Test 3: Economic Viability

**Goal**: Verify profit calculations are correct

**Use Tool**:
```bash
python3 Tools/trade_profit_calculator.py --best --capital 1000 --cargo 10
python3 Tools/trade_profit_calculator.py --best --capital 50000 --cargo 30
python3 Tools/trade_profit_calculator.py --best --capital 250000 --cargo 100
```

**Expected Results**:
- Starter: 840-1,200cr per optimal trade
- Medium: 2,520-3,600cr per optimal trade
- Large: 8,400-12,000cr per optimal trade

---

## Future Expansion (Post-MVP)

### Additional Ships (Weeks 13+)

**Tier 1.5**: "Scout Trader" (15 cargo, very fast)  
**Tier 2.5**: "Bulk Hauler" (50 cargo, very slow, cheap)  
**Tier 4**: "Mega Freighter" (200+ cargo, requires crew)

### Ship Specializations

- **Speed Trader**: Fast, less cargo, good for perishables
- **Bulk Hauler**: Slow, massive cargo, low-value goods
- **Luxury Liner**: Medium cargo, bonus for luxury goods
- **Armed Trader**: Weapons, protection (if combat added)

### Crew System (Post-MVP)

- Large ships require crew
- Crew have skills (piloting, trading, engineering)
- Hiring costs and wages
- Crew bonuses (better deals, faster travel)

---

## Summary

**3 Ships**:
1. Compact Trader - 10 cargo, free, starter
2. Merchant Hauler - 30 cargo, 50,000cr, mid-tier
3. Trade Liner - 100 cargo, 250,000cr, endgame

**Progression Time**:
- First upgrade: 15-20 minutes
- Second upgrade: 40-55 minutes total
- MVP complete: ~60 minutes

**Profit Scaling**:
- Linear with cargo capacity
- 3x and 10x multipliers
- Break-even on upgrades in ~10-15 minutes

**Implementation**:
- Data Assets (simple)
- Unlock system (credit threshold)
- Upgrade UI (reuse trading UI style)
- Testing (use profit calculator tool)

---

**Design Philosophy**: Clear, achievable progression that feels rewarding at each step.

**One polished gameplay loop beats ten half-finished systems.**

---

**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Part of**: Trade Simulator MVP (Week 1-4 Prototype Phase)
