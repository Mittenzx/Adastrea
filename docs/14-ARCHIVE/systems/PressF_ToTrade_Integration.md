# Press F to Trade - Integration Guide

## Overview

The "Press F to Trade" feature allows players to initiate trading with nearby space stations when within interaction range. This document explains how to integrate the UI prompt and handle the interaction key.

## C++ Implementation (Already Complete)

The following has been implemented in `AAdastreaPlayerController`:

- **TradingInteractionRadius** (default: 2000 units) - Configurable distance for trading
- **StationCheckInterval** (default: 0.5 seconds) - How often to check for nearby stations
- **AttemptTradeWithNearestStation()** - Called when player presses interaction key
- **IsNearTradableStation()** - Check if currently near a station
- **GetNearestTradableStation()** - Get the nearby station reference
- **OnNearbyTradableStationChanged()** - Blueprint event when proximity state changes

## Blueprint Integration

### Step 1: Create HUD Widget for "Press F to Trade" Prompt

1. Create a new Widget Blueprint (e.g., `WBP_TradingPrompt`)
2. Add a Text block with "Press F to Trade" message
3. Add optional station name display
4. Design with visibility (show/hide based on proximity)

### Step 2: Handle OnNearbyTradableStationChanged Event

In your Player Controller Blueprint (derived from `AAdastreaPlayerController`):

```
Event OnNearbyTradableStationChanged
├─ Branch (IsNearStation?)
│  ├─ True: Show "Press F to Trade" UI
│  │   ├─ Create Widget (if not exists)
│  │   ├─ Set Station Name
│  │   └─ Add to Viewport
│  └─ False: Hide "Press F to Trade" UI
│      └─ Remove from Parent
```

**Blueprint Nodes**:
- **Event OnNearbyTradableStationChanged** (IsNearStation, Station)
- Branch on `IsNearStation`
- If True: Create/Show widget, set station info
- If False: Hide/Remove widget

### Step 3: Bind Interaction Key (F Key)

In your Input Mapping Context or Enhanced Input setup:

1. Create/Find Input Action for "Trade Interaction" (default: F key)
2. Bind to `AttemptTradeWithNearestStation()` function
3. Ensure action is only active when controlling a spaceship

**Example Input Binding**:
```
Input Action: IA_TradeInteraction
├─ Key: F
└─ Triggered -> Call AttemptTradeWithNearestStation
```

## Configuration

### Adjusting Interaction Distance

In your Player Controller Blueprint or C++ defaults:
- Set `TradingInteractionRadius` to desired distance (in world units)
- Recommended: 1500-3000 units depending on station size
- Larger stations may need larger radius

### Adjusting Check Frequency

In your Player Controller Blueprint or C++ defaults:
- Set `StationCheckInterval` to desired frequency (in seconds)
- Lower values = more responsive but higher CPU cost
- Recommended: 0.3-1.0 seconds
- Default: 0.5 seconds is a good balance

## Example Implementation

### Minimal HUD Widget Blueprint

**WBP_TradingPrompt** (Blueprint Widget):

**Variables**:
- `StationName` (Text) - Name of the nearby station

**Widget Hierarchy**:
```
Canvas Panel
└─ Vertical Box (Center-Bottom of screen)
    ├─ Text Block: "Press F to Trade"
    │   └─ Font Size: 24
    │   └─ Color: White with slight glow
    └─ Text Block: [Station Name]
        └─ Font Size: 18
        └─ Color: Yellow
```

**Functions**:
- `SetStation(Station)` - Update station name display
- `ShowPrompt()` - Make widget visible
- `HidePrompt()` - Make widget invisible

### Player Controller Blueprint Event

**BP_AdastreaPlayerController** (Blueprint):

```
Event OnNearbyTradableStationChanged
├─ Is Near Station?
│  ├─ True:
│  │   ├─ Is Valid (Trading Prompt Widget)?
│  │   │   ├─ False: Create Widget (WBP_TradingPrompt)
│  │   │   └─ True: (Widget already exists)
│  │   ├─ Set Station (Station parameter)
│  │   ├─ Add to Viewport (if not already)
│  │   └─ Show Prompt
│  └─ False:
│      ├─ Is Valid (Trading Prompt Widget)?
│      │   ├─ True: Hide Prompt
│      │   └─ Remove from Parent
```

### Input Binding

In your Enhanced Input Mapping Context (e.g., `IMC_Spaceship`):

```
Input Action: IA_TradeInteraction
├─ Mappings:
│   └─ F (Keyboard)
└─ Triggered:
    └─ Get Player Controller
        └─ Cast to AdastreaPlayerController
            └─ Attempt Trade With Nearest Station
```

## Testing Checklist

- [ ] "Press F to Trade" prompt appears when approaching station
- [ ] Prompt shows correct station name
- [ ] Prompt disappears when leaving station range
- [ ] Pressing F opens trading interface
- [ ] Trading interface shows correct faction
- [ ] Feature only works when controlling a spaceship
- [ ] No errors in log when entering/leaving range
- [ ] Performance is acceptable (no stuttering)

## Troubleshooting

### Prompt doesn't appear
- Check `TradingInteractionRadius` is large enough
- Verify `OnNearbyTradableStationChanged` event is implemented
- Check Blueprint widget is assigned
- Look for errors in Output Log

### Pressing F doesn't open trading
- Verify Input Action is bound to `AttemptTradeWithNearestStation`
- Check you're controlling a spaceship (not on foot)
- Ensure you're within `TradingInteractionRadius`
- Verify station has a valid faction

### Station has no faction error
- Ensure space station has `OwningFaction` set
- Check station implements `IFactionMember` interface
- Assign a faction in station's properties

### Performance issues
- Increase `StationCheckInterval` (e.g., 1.0 second)
- Reduce `TradingInteractionRadius` if too large
- Check number of stations in level (timer scales with station count)

## Advanced Customization

### Custom Interaction Distance per Station

If you need different interaction distances for different stations:

1. Add property to Space Station Blueprint
2. Override `GetNearestTradableStation()` in Blueprint
3. Check station-specific radius instead of global one

### Multiple Interaction Types

If you want different interactions based on station type:

1. Check station type/modules in `OnNearbyTradableStationChanged`
2. Show different prompts (e.g., "Press F to Dock", "Press F to Trade")
3. Create separate functions for different interactions

### Prompt Animations

Add visual polish to the prompt:

1. Fade in/out when showing/hiding
2. Pulse effect on text
3. Distance indicator (closer = larger text)
4. Station icon/logo display

## Related Systems

- **Trading System**: See `TradingInterfaceWidget.h` for trading UI
- **Faction System**: See `FactionDataAsset.h` for faction configuration
- **Input System**: See `InputConfigDataAsset.h` for input setup
- **HUD System**: See `AdastreaHUDWidget.h` for HUD integration

## API Reference

### Blueprint-Callable Functions

**AttemptTradeWithNearestStation()**
- Opens trading UI with nearest station's faction
- Only works when near a station (within TradingInteractionRadius)
- Automatically gets station's faction for trading

**IsNearTradableStation() -> bool**
- Returns true if currently within trading range of a station
- Use for conditional logic in Blueprints

**GetNearestTradableStation() -> ASpaceStation***
- Returns reference to the nearby station (or null)
- Use to get station name, location, etc.

### Blueprint Events

**OnNearbyTradableStationChanged(IsNearStation, Station)**
- Called when player enters or leaves station trading range
- `IsNearStation`: true if entered range, false if left
- `Station`: reference to the station (or null if left)
- Use this to show/hide UI prompts

### Configurable Properties

**TradingInteractionRadius** (float, default: 2000.0)
- Maximum distance to trade with stations
- Editable in Player Controller Blueprint
- Larger = detect stations from farther away

**StationCheckInterval** (float, default: 0.5)
- How often to check for nearby stations (in seconds)
- Editable in Player Controller Blueprint
- Lower = more responsive, higher CPU cost

---

**Last Updated**: 2025-12-26
**Version**: 1.0
**Author**: Adastrea Development Team
