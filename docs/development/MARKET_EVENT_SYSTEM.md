# Market Event System for Trade Simulator MVP

**Purpose**: Add dynamic market events that create opportunities and challenges for traders.

**Design Goal**: Make the economy feel alive and responsive, creating emergent gameplay moments.

---

## Overview

### Event Philosophy

**Purpose**:
- Create profit opportunities (shortages = high prices)
- Add variety to trading (not repetitive)
- Generate stories ("Remember that drought?")
- Teach supply/demand (educational)

**NOT**:
- Punish player unfairly (events are opportunities)
- Random chaos (events are logical)
- Too frequent (special occasions)

---

## Event Types

### 1. Supply Shortage Events

#### Drought (Agricultural Stations)

**Trigger**: 10% chance every 30 minutes  
**Duration**: 5-10 minutes  
**Effect**: Food/water prices increase 50-100%

**Gameplay**:
```
Player at Trade Hub hears rumor: "Drought at Farming Collective"
 ↓
Flies to Agricultural Station
 ↓
Water price: 3cr → 6cr (+100%)
 ↓
Buys elsewhere, sells at Agricultural
 ↓
Reverse arbitrage profit!
```

**Configuration**:
```yaml
event:
  id: drought_agricultural
  name: "Severe Drought"
  description: "Crop failures have reduced food and water supply"
  station_type: agricultural
  trigger_chance: 0.1
  check_interval: 1800 seconds
  duration: 300-600 seconds
  
  effects:
    - item: water_purified
      price_multiplier: 2.0
      supply_multiplier: 0.3
      demand_multiplier: 3.0
    
    - item: protein_packs
      price_multiplier: 1.5
      supply_multiplier: 0.5
      demand_multiplier: 2.0
    
    - item: luxury_food
      price_multiplier: 1.8
      supply_multiplier: 0.4
      demand_multiplier: 2.5
  
  notification:
    title: "MARKET ALERT: Drought"
    message: "Severe drought at {station_name}. Food prices soaring!"
    icon: weather_drought
    sound: alert_warning
```

**Player Experience**:
- Notification appears
- Rush to affected station
- Buy from unaffected stations, sell to affected
- Time-limited opportunity
- Profit: 2-3x normal

---

#### Equipment Breakdown (Industrial Stations)

**Trigger**: 15% chance every 30 minutes  
**Duration**: 10-15 minutes  
**Effect**: Manufacturing output drops, prices rise

**Gameplay**:
```
Industrial Station: "Equipment malfunction!"
 ↓
Ship components production halted
 ↓
Components price: 350cr → 700cr (+100%)
 ↓
Player imports from Research Station
 ↓
Sells at Industrial for huge profit
```

**Configuration**:
```yaml
event:
  id: breakdown_industrial
  name: "Equipment Breakdown"
  description: "Manufacturing equipment requires repairs"
  station_type: industrial
  trigger_chance: 0.15
  check_interval: 1800 seconds
  duration: 600-900 seconds
  
  effects:
    - item: ship_components
      price_multiplier: 2.0
      supply_multiplier: 0.2
      demand_multiplier: 4.0
    
    - item: basic_alloys
      price_multiplier: 1.3
      supply_multiplier: 0.6
      demand_multiplier: 1.5
    
    - item: refined_metals
      price_multiplier: 1.4
      supply_multiplier: 0.5
      demand_multiplier: 1.8
  
  notification:
    title: "ALERT: Equipment Failure"
    message: "Manufacturing halted at {station_name}. Component shortage expected!"
    icon: warning_machinery
    sound: alert_critical
```

---

#### Research Delay (Research Stations)

**Trigger**: 8% chance every 40 minutes  
**Duration**: 15-20 minutes  
**Effect**: Tech production slows, high-value items scarce

**Gameplay**:
```
Research Station: "Experiment failure setback"
 ↓
AI Cores / Advanced Electronics scarce
 ↓
AI Cores: 5,000cr → 10,000cr (+100%)
 ↓
Supply from other stations or wait it out
 ↓
Speculation opportunity
```

**Configuration**:
```yaml
event:
  id: delay_research
  name: "Research Setback"
  description: "Experiment failure delays advanced tech production"
  station_type: research
  trigger_chance: 0.08
  check_interval: 2400 seconds
  duration: 900-1200 seconds
  
  effects:
    - item: ai_cores
      price_multiplier: 2.0
      supply_multiplier: 0.1
      demand_multiplier: 5.0
    
    - item: antimatter_cores
      price_multiplier: 1.8
      supply_multiplier: 0.15
      demand_multiplier: 4.0
    
    - item: advanced_electronics
      price_multiplier: 1.6
      supply_multiplier: 0.3
      demand_multiplier: 3.0
  
  notification:
    title: "RESEARCH ALERT"
    message: "Major setback at {station_name}. Advanced tech in short supply!"
    icon: science_alert
    sound: alert_info
```

---

### 2. Supply Surplus Events

#### Bumper Crop (Agricultural Stations)

**Trigger**: 12% chance every 30 minutes  
**Duration**: 10-15 minutes  
**Effect**: Food/water prices DROP 40-60% (buying opportunity)

**Gameplay**:
```
Agricultural Station: "Record harvest!"
 ↓
Water: 3cr → 1.5cr (-50%)
 ↓
Player loads up on cheap goods
 ↓
Sells normally at other stations
 ↓
Better margins on regular trades
```

**Configuration**:
```yaml
event:
  id: bumper_crop_agricultural
  name: "Bumper Crop"
  description: "Record harvest! Food prices plummet"
  station_type: agricultural
  trigger_chance: 0.12
  check_interval: 1800 seconds
  duration: 600-900 seconds
  
  effects:
    - item: water_purified
      price_multiplier: 0.5
      supply_multiplier: 3.0
      demand_multiplier: 0.4
    
    - item: protein_packs
      price_multiplier: 0.6
      supply_multiplier: 2.5
      demand_multiplier: 0.5
    
    - item: luxury_food
      price_multiplier: 0.7
      supply_multiplier: 2.0
      demand_multiplier: 0.6
  
  notification:
    title: "MARKET OPPORTUNITY"
    message: "Record harvest at {station_name}! Food prices at all-time low!"
    icon: celebration
    sound: alert_positive
```

---

#### Supply Convoy Arrival (Industrial Stations)

**Trigger**: 10% chance every 40 minutes  
**Duration**: 8-12 minutes  
**Effect**: Materials abundant, prices drop

**Gameplay**:
```
Industrial Station: "Supply convoy arrived!"
 ↓
Metals/Alloys prices drop 30-50%
 ↓
Stock up on cheap materials
 ↓
Sell to Research/Luxury stations
 ↓
Profit from bulk purchasing
```

**Configuration**:
```yaml
event:
  id: convoy_industrial
  name: "Supply Convoy"
  description: "Large shipment of raw materials arrived"
  station_type: industrial
  trigger_chance: 0.1
  check_interval: 2400 seconds
  duration: 480-720 seconds
  
  effects:
    - item: basic_alloys
      price_multiplier: 0.7
      supply_multiplier: 2.0
      demand_multiplier: 0.5
    
    - item: refined_metals
      price_multiplier: 0.6
      supply_multiplier: 2.5
      demand_multiplier: 0.4
    
    - item: ship_components
      price_multiplier: 0.8
      supply_multiplier: 1.5
      demand_multiplier: 0.6
  
  notification:
    title: "SUPPLY ALERT"
    message: "Massive convoy arrived at {station_name}! Material prices dropped!"
    icon: delivery_truck
    sound: alert_positive
```

---

### 3. Demand Spike Events

#### Labor Strike (Industrial Stations)

**Trigger**: 5% chance every 45 minutes  
**Duration**: 10-20 minutes  
**Effect**: Workers demand better conditions, import needs spike

**Gameplay**:
```
Industrial Station: "Workers on strike!"
 ↓
Food/Medicine demand SURGES
 ↓
Protein Packs: 25cr → 180cr (at industrial!)
 ↓
Import from Agricultural Station
 ↓
Sell to Industrial for 500%+ profit
```

**Configuration**:
```yaml
event:
  id: strike_industrial
  name: "Labor Strike"
  description: "Workers demand better conditions and supplies"
  station_type: industrial
  trigger_chance: 0.05
  check_interval: 2700 seconds
  duration: 600-1200 seconds
  
  effects:
    - item: protein_packs
      price_multiplier: 7.0 # Huge demand!
      supply_multiplier: 1.0
      demand_multiplier: 10.0
    
    - item: basic_medicine
      price_multiplier: 4.0
      supply_multiplier: 1.0
      demand_multiplier: 6.0
    
    - item: water_purified
      price_multiplier: 10.0 # Desperate need!
      supply_multiplier: 1.0
      demand_multiplier: 15.0
  
  notification:
    title: "CRISIS: Labor Strike"
    message: "{station_name} workers demanding supplies! Food and medicine prices skyrocket!"
    icon: protest
    sound: alert_critical
    priority: high
```

**This is the "jackpot" event - rare but hugely profitable!**

---

#### Celebrity Visit (Luxury Stations)

**Trigger**: 6% chance every 50 minutes  
**Duration**: 5-10 minutes  
**Effect**: Luxury goods in high demand

**Gameplay**:
```
Luxury Station: "Galactic celebrity arriving!"
 ↓
Rare Art/Luxury Food demand spikes
 ↓
Prices +150-200%
 ↓
Supply luxury goods fast
 ↓
Premium prices, quick turnaround
```

**Configuration**:
```yaml
event:
  id: celebrity_luxury
  name: "Celebrity Visit"
  description: "Galactic celebrity and entourage arriving"
  station_type: luxury
  trigger_chance: 0.06
  check_interval: 3000 seconds
  duration: 300-600 seconds
  
  effects:
    - item: rare_art
      price_multiplier: 3.0
      supply_multiplier: 1.0
      demand_multiplier: 5.0
    
    - item: luxury_food
      price_multiplier: 2.5
      supply_multiplier: 1.0
      demand_multiplier: 4.0
    
    - item: entertainment_systems
      price_multiplier: 2.0
      supply_multiplier: 1.0
      demand_multiplier: 3.0
  
  notification:
    title: "VIP ARRIVAL"
    message: "Celebrity entourage at {station_name}! Luxury goods in high demand!"
    icon: vip_star
    sound: alert_opportunity
```

---

## Event System Implementation

### C++ Market Event Manager

```cpp
USTRUCT(BlueprintType)
struct FMarketEvent
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EventID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EventName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EventDescription;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMarketEventEffect> Effects;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DurationSeconds;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMarketEventNotification Notification;
};

USTRUCT(BlueprintType)
struct FMarketEventEffect
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTradeItemDataAsset* AffectedItem;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PriceMultiplier; // 2.0 = double price, 0.5 = half price
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SupplyMultiplier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DemandMultiplier;
};

UCLASS()
class UMarketEventManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Event Triggering
    UFUNCTION(BlueprintCallable, Category="Market Events")
    void CheckForEvents(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category="Market Events")
    void TriggerEvent(FMarketEvent Event, ASpaceStation* Station);
    
    UFUNCTION(BlueprintCallable, Category="Market Events")
    void EndEvent(FMarketEvent Event);
    
    // Event Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Market Events")
    TArray<FMarketEvent> GetActiveEvents() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Market Events")
    bool IsEventActive(FName EventID) const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Market Events")
    FMarketEvent GetEventByID(FName EventID) const;

private:
    UPROPERTY()
    TArray<FMarketEvent> ActiveEvents;
    
    UPROPERTY()
    TMap<FName, float> EventTimers;
    
    void ApplyEventEffects(const FMarketEvent& Event, ASpaceStation* Station);
    void RemoveEventEffects(const FMarketEvent& Event, ASpaceStation* Station);
};
```

### Blueprint Integration

**In BP_TradingGameMode**:
```
Event Tick:
  ↓
MarketEventManager->CheckForEvents(DeltaTime)
  ↓
If Event Triggered:
  ↓
  Show Notification (WBP_MarketEventAlert)
  ↓
  Apply Effects to Market
  ↓
  Start Event Timer
  ↓
  On Timer End: Remove Effects
```

---

## Event Notification UI

### WBP_MarketEventAlert Widget

**Design**:
```
┌─────────────────────────────────────────┐
│ 🚨 MARKET ALERT                         │
│─────────────────────────────────────────│
│                                         │
│  [Icon]  LABOR STRIKE                   │
│          Industrial Exchange            │
│                                         │
│  Workers demanding supplies!            │
│  Food and medicine prices soaring!      │
│                                         │
│  Affected Items:                        │
│  • Protein Packs: +700%                 │
│  • Medicine: +400%                      │
│  • Water: +1000%                        │
│                                         │
│  Duration: 12 minutes remaining         │
│                                         │
│  [Navigate] [Dismiss]                   │
└─────────────────────────────────────────┘
```

**Widget Hierarchy**:
```
Canvas Panel
├─ Background Image (semi-transparent overlay)
├─ Alert Container (Vertical Box)
│   ├─ Header (Horizontal Box)
│   │   ├─ Icon (Image)
│   │   └─ Title (Text Block)
│   ├─ Station Name (Text Block)
│   ├─ Description (Text Block)
│   ├─ Affected Items List (Scroll Box)
│   │   └─ Item Rows (Text Blocks)
│   ├─ Duration (Text Block)
│   └─ Actions (Horizontal Box)
│       ├─ Navigate Button
│       └─ Dismiss Button
```

**Animation**:
- Slide in from top (0.3 sec)
- Stay for 5 seconds
- Fade out if not interacted (1 sec)
- or
- Minimize to notification bar (persistent)

---

## Event Balancing

### Frequency Guidelines

**Per Station** (30-minute period):
- 0-1 major events (strikes, celebrity)
- 1-2 medium events (droughts, convoys)
- 2-3 minor price fluctuations (non-event)

**Across All Stations** (30-minute period):
- 3-5 total events
- Never more than 2 simultaneous major events
- At least 5 minutes between events at same station

### Profit Potential

**Event Profitability Ranking**:

1. **Labor Strike** (Industrial):
   - Water arbitrage: 10x profit (3cr → 30cr sell price)
   - Rarest event (5% chance)
   - Highest reward

2. **Celebrity Visit** (Luxury):
   - Luxury goods: 3-4x profit
   - Short duration (5-10 min)
   - Time pressure

3. **Drought** (Agricultural):
   - Reverse arbitrage: 2-3x profit
   - Medium duration (5-10 min)
   - Accessible

4. **Equipment Breakdown** (Industrial):
   - Component trading: 2x profit
   - Common event (15% chance)
   - Good for beginners

5. **Bumper Crop** (Agricultural):
   - Better margins: +50% profit per trade
   - Long duration (10-15 min)
   - Stock-up opportunity

---

## Event Chains (Advanced - Post-MVP)

### Example: Industrial Crisis Chain

**Phase 1**: Equipment Breakdown (10 min)
- Manufacturing halts
- Component prices rise

**Phase 2**: Labor Strike (5 min)
- Workers worried about job security
- Demand for supplies spikes
- Prices SOAR

**Phase 3**: Emergency Convoy (Automatic)
- Relief convoy arrives
- Prices normalize
- Crisis resolved

**Total Duration**: 25-30 minutes  
**Player Experience**: Multi-phase opportunity with escalating profits

---

## Testing

### Test Event System

```bash
# Use scenario builder
python3 Tools/mvp_scenario_builder.py --scenario economic_crash --output test_events.json
```

**Manual Test**:
1. Force trigger event (debug command)
2. Verify notification appears
3. Check affected item prices
4. Make trades during event
5. Verify profits are higher
6. Wait for event to end
7. Check prices return to normal

**Expected Results**:
- Notifications clear and actionable
- Price changes match event specs
- Profitable arbitrage possible
- Events end cleanly
- No crashes or bugs

---

## Implementation Checklist

**Phase 1: Data Structures** (30 min):
- [ ] Create FMarketEvent struct
- [ ] Create FMarketEventEffect struct
- [ ] Create UMarketEventManager subsystem
- [ ] Test compilation

**Phase 2: Event Definitions** (45 min):
- [ ] Create 6 event Data Assets
- [ ] Configure effects per event
- [ ] Set probabilities and durations
- [ ] Test loading in editor

**Phase 3: Trigger System** (60 min):
- [ ] Implement CheckForEvents()
- [ ] Implement TriggerEvent()
- [ ] Apply price modifications
- [ ] Test event firing

**Phase 4: Notification UI** (60 min):
- [ ] Create WBP_MarketEventAlert
- [ ] Implement slide-in animation
- [ ] Add Navigate button logic
- [ ] Test in-game display

**Phase 5: Testing** (45 min):
- [ ] Test each event type
- [ ] Verify profit calculations
- [ ] Check event frequencies
- [ ] Balance adjustments

---

## Summary

**6 Event Types**:
- Drought (agricultural shortage)
- Equipment Breakdown (industrial shortage)
- Research Delay (tech shortage)
- Bumper Crop (agricultural surplus)
- Supply Convoy (industrial surplus)
- Labor Strike (demand spike - JACKPOT!)

**Effects**:
- Price changes: 50-1000% (depending on event)
- Duration: 5-20 minutes
- Frequency: 3-5 events per 30 minutes (all stations)

**Player Experience**:
- Opportunities, not punishments
- Clear notifications
- Time-limited profit windows
- Emergent stories ("Remember that strike?")

**Implementation**: C++ event manager + Blueprint integration + UI notifications

---

**Design Goal**: "Living economy with emergent opportunities"

---

**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Part of**: Trade Simulator MVP (Week 5-8 Structured Version)
