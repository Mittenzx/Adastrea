# AI Trader Behavior System for Trade Simulator MVP

**Purpose**: Configure NPC trader AI to create a living, breathing economy in the Trade Simulator MVP.

**Design Goal**: Make markets feel alive with autonomous traders, without overwhelming the player.

---

## Overview

### AI Trader Philosophy

**Purpose**:
- Make stations feel alive (ships coming/going)
- Create market price dynamics (supply/demand changes)
- Provide ambient activity (not competing directly with player)
- Tutorial by observation (player learns by watching)

**NOT**:
- Direct competition with player (AI doesn't "win")
- Unfair advantages (AI follows same rules)
- Overwhelming presence (player is still main actor)

---

## AI Trader Types

### Type 1: Local Shuttle (Common, 40% of AI traders)

**Profile**:
- **Ship**: Small cargo vessel (5-10 units)
- **Behavior**: Short-range, repetitive routes
- **Trade Pattern**: Low-value bulk goods (water, food, fuel)
- **Frequency**: 1 trade every 2-3 minutes
- **Intelligence**: Low (basic buy low, sell high)

**Purpose**: Ambient traffic, price stabilization

**Example**:
```
Shuttle "Aqua Express"
Route: Agricultural Station ↔ Industrial Station
Cargo: Water (always)
Quantity: 5 units
Profit: Small (50-100cr per run)
Behavior: Repetitive, predictable
```

**Configuration**:
```yaml
ai_trader:
  name: "Local Shuttle"
  ship_size: small
  cargo_capacity: 5-10
  trade_items:
    - water_purified
    - protein_packs
    - hydrogen_fuel
  route_type: fixed_shuttle
  trade_frequency: 120-180 seconds
  intelligence: low
  priority: volume
```

---

### Type 2: Merchant Trader (Common, 35% of AI traders)

**Profile**:
- **Ship**: Medium cargo vessel (20-40 units)
- **Behavior**: Multiple-station routes
- **Trade Pattern**: Mix of goods (medium value)
- **Frequency**: 1 trade every 5-8 minutes
- **Intelligence**: Medium (considers prices and demand)

**Purpose**: Market diversity, economic simulation

**Example**:
```
Trader "Stellar Merchant"
Route: 3-4 stations (varies)
Cargo: Mixed (food, medicine, components)
Quantity: 20-30 units total
Profit: Medium (500-1,500cr per run)
Behavior: Opportunistic, adapts to prices
```

**Configuration**:
```yaml
ai_trader:
  name: "Merchant Trader"
  ship_size: medium
  cargo_capacity: 20-40
  trade_items:
    - protein_packs
    - luxury_food
    - basic_medicine
    - ship_components
    - basic_computers
  route_type: opportunistic
  trade_frequency: 300-480 seconds
  intelligence: medium
  priority: profit
```

---

### Type 3: Specialist Hauler (Uncommon, 15% of AI traders)

**Profile**:
- **Ship**: Large cargo vessel (60-80 units)
- **Behavior**: Long-range, infrequent
- **Trade Pattern**: Single high-value cargo (luxury, tech)
- **Frequency**: 1 trade every 10-15 minutes
- **Intelligence**: High (waits for good prices)

**Purpose**: High-value market impact, endgame role model

**Example**:
```
Hauler "Platinum Express"
Route: Research → Luxury Station
Cargo: Advanced Electronics OR AI Cores
Quantity: 50-60 units
Profit: High (5,000-15,000cr per run)
Behavior: Patient, waits for optimal prices
```

**Configuration**:
```yaml
ai_trader:
  name: "Specialist Hauler"
  ship_size: large
  cargo_capacity: 60-80
  trade_items:
    - advanced_electronics
    - ai_cores
    - antimatter_cores
    - rare_art
    - luxury_food
  route_type: long_distance
  trade_frequency: 600-900 seconds
  intelligence: high
  priority: high_value
```

---

### Type 4: Newbie Trader (Rare, 10% of AI traders)

**Profile**:
- **Ship**: Small cargo vessel (3-5 units)
- **Behavior**: Inefficient, learning
- **Trade Pattern**: Suboptimal (sometimes loses money!)
- **Frequency**: 1 trade every 4-6 minutes
- **Intelligence**: Very low (makes mistakes)

**Purpose**: Show player they're doing well, tutorial by contrast

**Example**:
```
Newbie "First Timer"
Route: Random, often backtracking
Cargo: Whatever they can afford
Quantity: 2-3 units
Profit: Negative to small (often loses credits!)
Behavior: Chaotic, relatable
```

**Configuration**:
```yaml
ai_trader:
  name: "Newbie Trader"
  ship_size: tiny
  cargo_capacity: 3-5
  trade_items:
    - any_affordable
  route_type: random
  trade_frequency: 240-360 seconds
  intelligence: very_low
  priority: learning
  can_lose_money: true
```

---

## AI Behavior Parameters

### Intelligence Levels

**Very Low** (Newbie):
- Doesn't check prices before buying
- Sometimes buys high, sells low
- Random route selection
- No cargo optimization
- 50% chance of profitable trade

**Low** (Shuttle):
- Checks price difference (buy < sell)
- Fixed route only
- No market event awareness
- Basic cargo management
- 70% chance of profitable trade

**Medium** (Merchant):
- Compares prices across 3-5 stations
- Adapts route based on profit
- Aware of stock levels
- Optimizes cargo mix
- 85% chance of profitable trade

**High** (Specialist):
- Waits for optimal prices (patience)
- Considers supply/demand trends
- Responds to market events
- Maximizes profit per trade
- 95% chance of profitable trade

### Trade Frequency

**Purpose**: Prevent market flooding, maintain balance

**Formula**:
```
Trade Interval = Base Interval + Random(0, Variance)

Local Shuttle:     120s + Random(0, 60s)  = 120-180s (2-3 min)
Merchant Trader:   300s + Random(0, 180s) = 300-480s (5-8 min)
Specialist Hauler: 600s + Random(0, 300s) = 600-900s (10-15 min)
Newbie Trader:     240s + Random(0, 120s) = 240-360s (4-6 min)
```

**Why Variance?**
- Prevents synchronization (all traders at once)
- Creates natural traffic patterns
- Feels organic, not robotic

---

## Station AI Configuration

### Agricultural Station (Farming Collective Market)

**AI Trader Count**: 6 total

**Breakdown**:
- 3× Local Shuttle (50%) - Water/food routes
- 2× Merchant Trader (33%) - Mixed goods
- 1× Newbie Trader (17%) - Learning

**Traffic Pattern**:
- Quiet station (rural economy)
- Steady water exports
- Few incoming ships (self-sufficient)

**Config**:
```yaml
station_ai:
  station_id: agricultural_station
  total_traders: 6
  
  traders:
    - type: local_shuttle
      count: 3
      primary_export: water_purified
      destinations:
        - industrial_station
        - trade_hub
    
    - type: merchant_trader
      count: 2
      trade_mix:
        - protein_packs
        - luxury_food
      destinations:
        - luxury_station
        - trade_hub
    
    - type: newbie_trader
      count: 1
      behavior: random
      learning: true
```

---

### Industrial Station (Industrial Exchange)

**AI Trader Count**: 8 total

**Breakdown**:
- 2× Local Shuttle (25%) - Fuel/materials
- 4× Merchant Trader (50%) - Components/alloys
- 2× Specialist Hauler (25%) - High-value tech

**Traffic Pattern**:
- Busy hub (manufacturing center)
- Constant incoming materials
- Outgoing finished goods
- Most active station

**Config**:
```yaml
station_ai:
  station_id: industrial_station
  total_traders: 8
  
  traders:
    - type: local_shuttle
      count: 2
      primary_export: basic_alloys
      destinations:
        - trade_hub
        - research_station
    
    - type: merchant_trader
      count: 4
      trade_mix:
        - ship_components
        - refined_metals
        - basic_computers
      destinations:
        - research_station
        - trade_hub
        - luxury_station
    
    - type: specialist_hauler
      count: 2
      trade_mix:
        - advanced_electronics
        - ship_components
      destinations:
        - research_station
        - luxury_station
      patience: high
```

---

### Research Station (Research Consortium)

**AI Trader Count**: 5 total

**Breakdown**:
- 1× Local Shuttle (20%) - Supply runner
- 2× Merchant Trader (40%) - Tech goods
- 2× Specialist Hauler (40%) - Advanced tech

**Traffic Pattern**:
- Moderate traffic (specialized)
- High-value exports
- Few bulk goods
- Smart traders only

**Config**:
```yaml
station_ai:
  station_id: research_station
  total_traders: 5
  
  traders:
    - type: local_shuttle
      count: 1
      primary_import: protein_packs
      primary_export: basic_medicine
      destinations:
        - trade_hub
    
    - type: merchant_trader
      count: 2
      trade_mix:
        - basic_computers
        - advanced_electronics
        - advanced_medicine
      destinations:
        - industrial_station
        - luxury_station
        - trade_hub
    
    - type: specialist_hauler
      count: 2
      trade_mix:
        - ai_cores
        - antimatter_cores
        - advanced_electronics
      destinations:
        - luxury_station
        - trade_hub
      patience: very_high
```

---

### Luxury Station (Luxury Bazaar)

**AI Trader Count**: 4 total

**Breakdown**:
- 0× Local Shuttle (0%) - No basic goods
- 2× Merchant Trader (50%) - Luxury imports
- 2× Specialist Hauler (50%) - High-end goods

**Traffic Pattern**:
- Low traffic (exclusive)
- High-value only
- Slow turnover
- Wealthy clientele

**Config**:
```yaml
station_ai:
  station_id: luxury_station
  total_traders: 4
  
  traders:
    - type: merchant_trader
      count: 2
      trade_mix:
        - luxury_food
        - entertainment_systems
        - rare_art
      destinations:
        - agricultural_station
        - trade_hub
    
    - type: specialist_hauler
      count: 2
      trade_mix:
        - rare_art
        - antimatter_cores
        - advanced_electronics
      destinations:
        - research_station
        - trade_hub
      patience: extreme
```

---

### Trade Hub (Trade Hub Market)

**AI Trader Count**: 10 total

**Breakdown**:
- 4× Local Shuttle (40%) - Hub-and-spoke
- 4× Merchant Trader (40%) - Arbitrage
- 1× Specialist Hauler (10%) - High-value
- 1× Newbie Trader (10%) - Tourist

**Traffic Pattern**:
- Busiest station (central hub)
- All trade types
- Constant activity
- Clearinghouse for economy

**Config**:
```yaml
station_ai:
  station_id: trade_hub
  total_traders: 10
  
  traders:
    - type: local_shuttle
      count: 4
      trade_mix: any_basic
      destinations:
        - agricultural_station
        - industrial_station
      hub_and_spoke: true
    
    - type: merchant_trader
      count: 4
      trade_mix: any_medium_value
      destinations:
        - all_stations
      arbitrage: true
    
    - type: specialist_hauler
      count: 1
      trade_mix: any_high_value
      destinations:
        - luxury_station
        - research_station
    
    - type: newbie_trader
      count: 1
      behavior: tourist
      learning: true
```

---

## Implementation

### C++ AI Trader Component

```cpp
UCLASS()
class UAITraderComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader")
    EAITraderType TraderType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader")
    int32 CargoCapacity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader")
    TArray<UTradeItemDataAsset*> PreferredItems;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader")
    EAIIntelligence IntelligenceLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader")
    float TradeFrequencySeconds;
    
    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category="AI Trader")
    ASpaceStation* CurrentStation;
    
    UPROPERTY(BlueprintReadOnly, Category="AI Trader")
    ASpaceStation* TargetStation;
    
    UPROPERTY(BlueprintReadOnly, Category="AI Trader")
    TMap<FName, int32> CurrentCargo;
    
    UPROPERTY(BlueprintReadOnly, Category="AI Trader")
    int32 Credits;
    
    // AI Logic
    UFUNCTION(BlueprintCallable, Category="AI Trader")
    void DecideNextTrade();
    
    UFUNCTION(BlueprintCallable, Category="AI Trader")
    bool EvaluateTradeOpportunity(UTradeItemDataAsset* Item, ASpaceStation* BuyStation, ASpaceStation* SellStation);
    
    UFUNCTION(BlueprintCallable, Category="AI Trader")
    void ExecuteTrade();
    
private:
    FTimerHandle TradeTimer;
    
    void ScheduleNextTrade();
    float CalculateProfitPotential(UTradeItemDataAsset* Item, float BuyPrice, float SellPrice, int32 Quantity);
};
```

### Blueprint AI Trader Actor

```
BP_AITrader (Actor)
├─ ShipMesh (Static Mesh)
├─ AITraderComponent (C++)
├─ MovementComponent (for flying between stations)
└─ CollisionComponent (for docking)

Event Graph:
- BeginPlay → Initialize AI, Schedule First Trade
- OnReachStation → Dock, Execute Trade, Fly to Next
- OnTradeComplete → Update cargo, Update credits
- OnLowCredits → Return to safe station
```

---

## Balancing Guidelines

### AI Trading Volume vs Player

**Rule**: AI should trade 30-50% of player volume

**Example**:
- Player makes 10 trades/hour
- 33 total AI traders make 100 trades/hour combined
- But distributed across 5 stations and staggered
- Player sees 2-3 AI trades per their trade
- Feels alive, not overwhelming

### Price Impact

**AI trades should**:
- Slightly affect prices (±5% per trade)
- Reset toward base price over time
- Not create runaway inflation/deflation
- Allow player profitable arbitrage

**Formula**:
```cpp
float NewPrice = BasePrice * (1.0 + (Demand - Supply) * 0.05);
NewPrice = FMath::Clamp(NewPrice, BasePrice * 0.7, BasePrice * 1.5);
```

### Credit Distribution

**AI Credits**:
- Local Shuttle: 500-2,000cr
- Merchant Trader: 5,000-20,000cr
- Specialist Hauler: 50,000-200,000cr
- Newbie Trader: 300-1,000cr

**Purpose**: Prevent AI from dominating markets

---

## Testing

### Test 1: AI Behavior Observation

1. Sit at station for 5 minutes
2. Count AI arrivals: Should see 3-5 traders
3. Observe trade types: Mix of small and medium
4. Check prices: Should fluctuate ±10-15%

### Test 2: Player Competition

1. Find profitable route (e.g., water arbitrage)
2. Make 10 trades
3. Check if AI also trades water
4. Verify: Player still profits (AI didn't ruin it)

### Test 3: Market Impact

1. Note water price at agricultural: 3cr
2. AI shuttle buys 100 units
3. Check new price: Should be 3.15-3.30cr (+5-10%)
4. Wait 2 minutes
5. Price should drift back toward 3cr

---

## Summary

**AI Trader Mix**:
- 40% Local Shuttles (ambient traffic)
- 35% Merchant Traders (economy simulation)
- 15% Specialist Haulers (high-value role models)
- 10% Newbie Traders (relatability, humor)

**Station Traffic**:
- Agricultural: 6 traders (quiet)
- Industrial: 8 traders (busy)
- Research: 5 traders (specialized)
- Luxury: 4 traders (exclusive)
- Trade Hub: 10 traders (busiest)
- **Total**: 33 AI traders across 5 stations

**AI Intelligence**:
- Makes markets feel alive
- Doesn't overwhelm player
- Creates price dynamics
- Provides learning examples

**Implementation**: C++ component + Blueprint actors

---

**Design Goal**: "Living economy" without "unfair competition"

---

**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Part of**: Trade Simulator MVP (Week 5-8 Structured Version)
