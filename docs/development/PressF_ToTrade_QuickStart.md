# Quick Start: Press F to Trade Blueprint Setup

This is a minimal, step-by-step guide to get "Press F to Trade" working in your game.

## Prerequisites

- Adastrea project with updated Player Controller (includes Press F to Trade feature)
- At least one Space Station in your level
- Player controlling a spaceship

## Step 1: Create the Trading Prompt Widget (5 minutes)

### Create Widget Blueprint

1. In Unreal Editor, go to **Content Browser**
2. Navigate to `Content/UI/` (or your UI folder)
3. Right-click → **User Interface** → **Widget Blueprint**
4. Name it `WBP_TradingPrompt`

### Design the Widget

1. Open `WBP_TradingPrompt`
2. Add a **Canvas Panel** as root
3. Add a **Vertical Box** (centered at bottom of screen)
4. Inside Vertical Box, add:
   - **Text Block**: Set text to "Press F to Trade"
     - Font Size: 28
     - Color: White or Yellow
     - Shadow: Add drop shadow for readability
   - **Text Block**: Named `StationNameText`
     - Font Size: 20
     - Color: Light Blue
     - This will show the station name

### Add Widget Variables

In the **Variables** panel:
- Add variable: `StationName` (Type: Text)

### Create Functions

**Function: SetStation**
```
Input: Station (Type: Space Station object reference)
├─ Get Station Name (from Station)
└─ Set StationName variable
```

**Function: ShowPrompt**
```
└─ Set Visibility → Visible
```

**Function: HidePrompt**
```
└─ Set Visibility → Hidden
```

### Bind Text
In Designer mode:
- Select `StationNameText`
- In Details → Content → Text
- Click **Bind** → **Create Binding**
- Return `StationName` variable

---

## Step 2: Update Your Player Controller Blueprint (5 minutes)

### Create or Open Player Controller Blueprint

1. Find your Player Controller Blueprint (e.g., `BP_AdastreaPlayerController`)
   - It should be derived from `AAdastreaPlayerController`
2. If you don't have one, create it:
   - Right-click in Content Browser → **Blueprint Class**
   - Search for `AdastreaPlayerController`
   - Name it `BP_AdastreaPlayerController`

### Add Trading Prompt Variable

In **Variables** panel:
- Add variable: `TradingPromptWidget` (Type: `WBP_TradingPrompt` object reference)

### Implement OnNearbyTradableStationChanged Event

In **Event Graph**:

1. Right-click → **Add Event** → **Event On Nearby Tradable Station Changed**

2. Create this Blueprint logic:

```
Event OnNearbyTradableStationChanged (IsNearStation, Station)
├─ Branch (Is Near Station?)
│  
├─ TRUE BRANCH (Player is near station):
│  ├─ Is Valid? (TradingPromptWidget)
│  │  ├─ FALSE: Create Widget (WBP_TradingPrompt class)
│  │  │  └─ Set TradingPromptWidget variable
│  │  └─ TRUE: (Widget exists, continue)
│  │
│  ├─ Set Station (Station parameter)
│  ├─ Is In Viewport?
│  │  └─ FALSE: Add to Viewport
│  └─ Show Prompt
│  
└─ FALSE BRANCH (Player left station range):
   ├─ Is Valid? (TradingPromptWidget)
   │  └─ TRUE:
   │     ├─ Hide Prompt
   │     └─ Remove from Parent
```

**Detailed Node Connections**:
```
1. Event OnNearbyTradableStationChanged
   └─ Branch (condition: IsNearStation pin)

2. TRUE Pin:
   └─ IsValid? (TradingPromptWidget)
      ├─ Is Not Valid → Create Widget (Class: WBP_TradingPrompt)
      │                 └─ Set TradingPromptWidget
      └─ Is Valid → (continue)
   
   └─ Call Function: SetStation (on TradingPromptWidget)
      ├─ Input: Station (from event)
   
   └─ IsInViewport? (on TradingPromptWidget)
      └─ FALSE → Add to Viewport (on TradingPromptWidget)
   
   └─ Call Function: ShowPrompt (on TradingPromptWidget)

3. FALSE Pin:
   └─ IsValid? (TradingPromptWidget)
      └─ Is Valid → Call Function: HidePrompt
                    └─ Remove from Parent
```

---

## Step 3: Set Up Input Binding (5 minutes)

### Option A: Using Enhanced Input (Recommended)

1. Open your Input Mapping Context (e.g., `IMC_Spaceship`)
2. Add new mapping:
   - **Name**: `TradeInteraction`
   - **Key**: F (Keyboard)
   - **Modifiers**: None
   - **Triggers**: Pressed

3. In your Player Controller or Spaceship Blueprint:
   - Find where Enhanced Input actions are bound
   - Add binding for `IA_TradeInteraction`:

```
IA_TradeInteraction (Triggered)
└─ Get Player Controller
   └─ Cast to AdastreaPlayerController
      └─ Attempt Trade With Nearest Station
```

### Option B: Using Legacy Input

1. Open **Project Settings** → **Input**
2. Under **Action Mappings**, add:
   - **Name**: `TradeInteraction`
   - **Key**: F

3. In Player Controller Event Graph:

```
InputAction TradeInteraction (Pressed)
└─ Attempt Trade With Nearest Station
```

---

## Step 4: Configure Properties (2 minutes)

### In Player Controller Blueprint

1. Select the **Class Defaults** tab
2. Find **Player | Trading** category
3. Configure:
   - **Trading Interaction Radius**: 2000.0 (adjust as needed)
     - Larger values = detect stations from farther away
     - Recommended: 1500-3000 for small/medium stations
   - **Station Check Interval**: 0.5 (seconds)
     - Lower = more responsive, higher CPU cost
     - Recommended: 0.3-1.0 seconds

### Set Player Controller in Game Mode

1. Open your Game Mode Blueprint (e.g., `BP_AdastreaGameMode`)
2. In **Class Defaults**:
   - **Player Controller Class**: Set to `BP_AdastreaPlayerController`

---

## Step 5: Test In-Game (5 minutes)

### Set Up Test Level

1. Ensure you have:
   - Space Station actor in level with faction assigned
   - Playable spaceship
   - Your modified Game Mode set as default

2. Play in Editor (PIE)

### Testing Checklist

- [ ] Fly spaceship toward station
- [ ] "Press F to Trade" prompt appears when close enough
- [ ] Station name displays correctly
- [ ] Prompt disappears when flying away
- [ ] Pressing F opens trading interface
- [ ] Trading interface shows station's faction
- [ ] No errors in Output Log

### Troubleshooting

**Prompt doesn't appear:**
- Check Trading Interaction Radius isn't too small
- Verify station has a faction assigned
- Look for errors in Output Log
- Ensure Event OnNearbyTradableStationChanged is implemented

**Pressing F doesn't work:**
- Check Input Action is bound correctly
- Ensure you're controlling a spaceship
- Verify you're within interaction radius
- Check Output Log for "Not controlling a spaceship" warning

**Station has no faction error:**
- Select station in level
- In Details panel, set **Owning Faction** property
- Ensure faction asset exists in project

---

## Advanced: Customize Interaction Distance Per Station

If you want different stations to have different interaction ranges:

### In Space Station Blueprint

1. Add variable: `CustomTradingRadius` (Float, default: 2000.0)
2. Make it **Instance Editable**

### In Player Controller Blueprint

Modify the proximity check logic to use station-specific radius instead of global one. This is an advanced customization and requires Blueprint knowledge.

---

## Summary

You should now have:
- ✅ Trading prompt widget that appears near stations
- ✅ F key bound to trading action
- ✅ Trading interface opens when near station
- ✅ Configurable interaction distance

**Total Setup Time**: ~20 minutes

**Next Steps**:
- Customize prompt appearance and animations
- Add distance indicator or progress bar
- Implement different prompts for different station types
- Add sound effects for entering/leaving range

For more details, see: `docs/systems/PressF_ToTrade_Integration.md`

---

**Last Updated**: 2025-12-26
**Version**: 1.0
**Difficulty**: Beginner-Friendly
