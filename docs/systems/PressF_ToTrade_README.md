# Press F to Trade - System Overview

## Feature Description

Allows players to initiate trading with nearby space stations when within interaction range (default: 2000 units). Simple, intuitive interaction: fly near station, press F to trade.

---

## System Flow Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                     PLAYER CONTROLLER                         │
│                  (AAdastreaPlayerController)                  │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  BeginPlay()                                        │    │
│  │  └─ Start Timer (StationCheckInterval = 0.5s)      │    │
│  │     └─ Calls CheckForNearbyTradableStations()      │    │
│  └─────────────────────────────────────────────────────┘    │
│                           │                                   │
│                           ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  CheckForNearbyTradableStations() [Every 0.5s]     │    │
│  │  ├─ Get player location                            │    │
│  │  ├─ Find all space stations in world               │    │
│  │  ├─ Find nearest within TradingInteractionRadius   │    │
│  │  └─ If state changed:                              │    │
│  │     └─ Fire OnNearbyTradableStationChanged event   │    │
│  └─────────────────────────────────────────────────────┘    │
│                           │                                   │
│          ┌────────────────┴────────────────┐                │
│          ▼                                  ▼                 │
│    [Near Station]                    [Left Range]            │
│          │                                  │                 │
│          ▼                                  ▼                 │
│  ┌──────────────────┐            ┌──────────────────┐       │
│  │ Event Fired:     │            │ Event Fired:     │       │
│  │ bIsNear = TRUE   │            │ bIsNear = FALSE  │       │
│  │ Station = [Ref]  │            │ Station = null   │       │
│  └──────────────────┘            └──────────────────┘       │
└──────────────────────────────────────────────────────────────┘
           │                                  │
           ▼                                  ▼
┌──────────────────────────────────────────────────────────────┐
│                    BLUEPRINT LAYER                            │
│                (Designer Implementation)                      │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  OnNearbyTradableStationChanged(bIsNear, Station)  │    │
│  │                                                      │    │
│  │  IF bIsNear:                                        │    │
│  │    ├─ Create/Get Trading Prompt Widget             │    │
│  │    ├─ Set Station Name                             │    │
│  │    ├─ Add to Viewport                              │    │
│  │    └─ Show "Press F to Trade"                      │    │
│  │                                                      │    │
│  │  ELSE:                                              │    │
│  │    ├─ Hide Trading Prompt Widget                   │    │
│  │    └─ Remove from Viewport                         │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  Input: F Key Pressed                              │    │
│  │  └─ Call: AttemptTradeWithNearestStation()        │    │
│  └─────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────────┐
│              BACK TO PLAYER CONTROLLER                        │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  AttemptTradeWithNearestStation()                  │    │
│  │  ├─ Check if controlling spaceship                 │    │
│  │  ├─ Get nearest tradable station                   │    │
│  │  ├─ Get station's faction (IFactionMember)         │    │
│  │  └─ Call: OpenTrading(faction)                     │    │
│  └─────────────────────────────────────────────────────┘    │
│                           │                                   │
│                           ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  OpenTrading(faction) [Existing System]            │    │
│  │  ├─ Create/Get Trading Widget                      │    │
│  │  ├─ Set Trade Partner (faction)                    │    │
│  │  ├─ Load available items                           │    │
│  │  ├─ Add to Viewport                                │    │
│  │  └─ Switch to UI input mode                        │    │
│  └─────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────┘
                            │
                            ▼
              ┌─────────────────────────┐
              │   TRADING INTERFACE     │
              │  (Player can buy/sell)  │
              └─────────────────────────┘
```

---

## Component Interaction

```
┌─────────────────┐        detects        ┌─────────────────┐
│ Player Ship     │─────────────────────>  │ Space Station   │
│ (APawn)         │      proximity         │ (AActor)        │
└─────────────────┘     (2000 units)       └─────────────────┘
         │                                           │
         │                                           │
         │  controlled by                            │ has faction
         │                                           │
         ▼                                           ▼
┌─────────────────┐      monitors         ┌─────────────────┐
│ Player          │─────────────────────>  │ Faction Data    │
│ Controller      │   via timer (0.5s)    │ Asset           │
└─────────────────┘                        └─────────────────┘
         │
         │  fires event
         │
         ▼
┌─────────────────┐
│ Blueprint       │
│ (UI Prompt)     │
└─────────────────┘
         │
         │  on F key
         │
         ▼
┌─────────────────┐
│ Trading UI      │
│ (Buy/Sell)      │
└─────────────────┘
```

---

## Distance Relationships

```
Player Ship Position:

                    TradingInteractionRadius
                    ◄─────── 2000 units ────────►
                    
    [Ship] ───────────────────────────────── [Station]
      │                                           │
      └─────────────────────────────────────────┘
                  ▲                         ▲
              Too Far                  Within Range
           (No prompt)               (Show "Press F")
```

### Multiple Stations:

```
                2000 units              2000 units
               ◄──────────►            ◄──────────►
               
    [Station A] ─────── [Ship] ─────────── [Station B]
         │                 │                    │
         X                 ✓                    X
    Out of range      Nearest station    Out of range
                      (Shows prompt for
                       Station B only)
```

---

## State Machine

```
┌─────────────────────────────────────────────────────────┐
│                    PLAYER STATES                        │
└─────────────────────────────────────────────────────────┘

    ┌──────────────┐
    │   Flying     │
    │  (No prompt) │
    └──────────────┘
           │
           │ Enter 2000 unit radius
           │
           ▼
    ┌──────────────┐
    │ Near Station │────> Press F ────> Trading UI Opens
    │ (Show prompt)│
    └──────────────┘
           │
           │ Leave radius
           │
           ▼
    ┌──────────────┐
    │   Flying     │
    │  (No prompt) │
    └──────────────┘
```

---

## Configuration Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `TradingInteractionRadius` | float | 2000.0 | Distance to detect stations (world units) |
| `StationCheckInterval` | float | 0.5 | How often to check proximity (seconds) |
| `NearbyTradableStation` | ASpaceStation* | nullptr | Cached nearest station reference |
| `bWasNearTradableStation` | bool | false | Previous proximity state |

**Performance Impact**:
- Timer checks: Every 0.5 seconds (configurable)
- GetAllActorsOfClass: Once per check
- Distance calculation: For each station (typically < 10)
- Total CPU cost: Minimal (~0.1ms per check)

---

## API Quick Reference

### C++ Functions (Callable from Blueprint)

```cpp
// Check if player is near any tradable station
bool IsNearTradableStation() const;

// Get the nearest station within interaction radius
ASpaceStation* GetNearestTradableStation() const;

// Attempt to open trading with nearest station
void AttemptTradeWithNearestStation();
```

### Blueprint Events

```cpp
// Called when proximity state changes
Event OnNearbyTradableStationChanged(
    bool bIsNearStation,           // True if entered range
    ASpaceStation* Station         // Station reference (or null)
)
```

---

## Example Usage Scenarios

### Scenario 1: Approaching Station

```
Time: 0.0s  - Flying toward station at 3000 units away
Time: 1.5s  - Timer check: Still too far (3000 > 2000)
Time: 2.0s  - Timer check: Still too far (2500 > 2000)
Time: 2.5s  - Timer check: NOW IN RANGE! (1800 < 2000)
            - Fire Event: OnNearbyTradableStationChanged(true, Station)
            - UI: Show "Press F to Trade - Station Alpha"
Time: 3.0s  - Player presses F
            - Call: AttemptTradeWithNearestStation()
            - Result: Trading UI opens with Station Alpha's faction
```

### Scenario 2: Flying Between Stations

```
Position:  [Station A] ─── 4000 units ─── [Player] ─── 1500 units ─── [Station B]

Result: Player is near Station B (1500 < 2000)
        Player is NOT near Station A (4000 > 2000)
        
UI Shows: "Press F to Trade - Station B"

If player presses F: Opens trading with Station B's faction
```

### Scenario 3: No Faction on Station

```
Time: 0.0s  - Player approaches station without faction
Time: 1.0s  - Timer check: In range (1500 < 2000)
            - Fire Event: OnNearbyTradableStationChanged(true, Station)
            - UI: Show "Press F to Trade - Mystery Station"
Time: 2.0s  - Player presses F
            - Call: AttemptTradeWithNearestStation()
            - Check: Station has no faction
            - Result: Warning log, no trading UI
            - UI can show: "This station is not accepting traders"
```

---

## Files Modified

- `Source/Adastrea/Public/Player/AdastreaPlayerController.h` (+40 lines)
- `Source/Adastrea/Player/AdastreaPlayerController.cpp` (+120 lines)

---

## See Also

- **Quick Start Guide**: `docs/development/PressF_ToTrade_QuickStart.md`
- **Integration Guide**: `docs/systems/PressF_ToTrade_Integration.md`
- **Trading System**: `Source/Adastrea/Public/UI/TradingInterfaceWidget.h`
- **Faction System**: `Source/Adastrea/Public/Factions/FactionDataAsset.h`

---

**Last Updated**: 2025-12-26  
**Version**: 1.0  
**Status**: Production Ready
