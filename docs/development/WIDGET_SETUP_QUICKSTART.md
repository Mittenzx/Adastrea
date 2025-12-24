# Widget Setup Quick Start

## Quick Answer to "What Do I Need to Add?"

Based on the recent widget C++ implementations, here's exactly what you need to do to make widgets work:

### TL;DR - Three Simple Steps

1. **Create Widget Blueprint** - Design your UI in Unreal Editor
2. **Set Widget Class** - Configure it in your PlayerController Blueprint  
3. **Bind Input** - Connect keyboard/mouse input to toggle functions

That's it! The C++ code already handles all the initialization.

---

## What's Already Done (You Don't Need to Add This)

✅ **Widget C++ Classes** - Already implemented:
- `UAdastreaHUDWidget` - Main HUD
- `UStationEditorWidget` - Station editor
- `UShipStatusWidget` - Ship stats
- `UInventoryWidget` - Inventory management
- `UTradingInterfaceWidget` - Trading interface
- `UStationManagementWidget` - Station management
- `UMainMenuWidget` - Pause menu

✅ **PlayerController Integration** - Already implemented:
- `CreateWidget<T>()` calls
- `AddToViewport()` / `RemoveFromParent()` logic
- Input mode switching (Game vs UI)
- Widget lifecycle management
- Initialization functions

✅ **Initialization Logic** - Already implemented:
- HUD: Auto-creates on `BeginPlay()`
- Other widgets: Created on-demand when toggled
- Automatic initialization with required data

---

## What You Need to Do

### Step 1: Create Widget Blueprints (One-Time Setup)

For each widget you want to use:

1. **In Unreal Editor**, go to `Content/UI/` folder
2. **Right-click** → **User Interface** → **Widget Blueprint**
3. **Name it**: `WBP_[WidgetName]` (e.g., `WBP_Inventory`)
4. **Open** the widget blueprint
5. **Click "Class Settings"** in toolbar
6. **Set Parent Class** to the C++ widget class:
   - For inventory: `InventoryWidget`
   - For trading: `TradingInterfaceWidget`
   - For station mgmt: `StationManagementWidget`
   - For HUD: `AdastreaHUDWidget`
   - etc.
7. **Design your UI** in the Designer view
8. **Compile and Save**

**Example Widget Hierarchy:**
```
Content/UI/
├── WBP_HUD                    (Parent: AdastreaHUDWidget)
├── WBP_Inventory              (Parent: InventoryWidget)
├── WBP_Trading                (Parent: TradingInterfaceWidget)
├── WBP_StationEditor          (Parent: StationEditorWidget)
├── WBP_StationManagement      (Parent: StationManagementWidget)
├── WBP_ShipStatus             (Parent: ShipStatusWidget)
└── WBP_MainMenu               (Parent: MainMenuWidget)
```

---

### Step 2: Configure PlayerController Blueprint (One-Time Setup)

1. **Open** `BP_AdastreaPlayerController` (or your PlayerController Blueprint)
2. **In the Details panel**, find these categories:
   - **HUD**
   - **Ship Status**
   - **Main Menu**
   - **Station Editor**
   - **Inventory**
   - **Trading**
   - **Station Management**

3. **Set the Widget Class** for each:
   - HUD Widget Class → `WBP_HUD`
   - Ship Status Widget Class → `WBP_ShipStatus`
   - Main Menu Widget Class → `WBP_MainMenu`
   - Station Editor Widget Class → `WBP_StationEditor`
   - Inventory Widget Class → `WBP_Inventory`
   - Trading Widget Class → `WBP_Trading`
   - Station Management Widget Class → `WBP_StationManagement`

4. **Compile and Save**

**Screenshot of what to set:**
```
┌─────────────────────────────────────┐
│ BP_AdastreaPlayerController         │
├─────────────────────────────────────┤
│ Details Panel                       │
│                                     │
│ ▼ HUD                               │
│   HUD Widget Class: WBP_HUD        │
│                                     │
│ ▼ Inventory                         │
│   Inventory Widget Class: WBP_Inventory
│                                     │
│ ▼ Trading                           │
│   Trading Widget Class: WBP_Trading│
│                                     │
│ ... (and so on)                     │
└─────────────────────────────────────┘
```

---

### Step 3: Bind Input Actions (One-Time Setup)

#### Option A: Using Enhanced Input (Recommended)

1. **Open** `DA_InputConfig` or your Input Mapping Context
2. **Add Input Actions** for each widget toggle:
   - `IA_ToggleInventory` → `I` key
   - `IA_ToggleShipStatus` → `C` key
   - `IA_ToggleMainMenu` → `Escape` key
   - `IA_ToggleStationEditor` → `N` key (when near station)
   - etc.

3. **In your Input handling code** (or Blueprint), bind actions:
   ```cpp
   // C++ example
   EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Triggered, 
       PlayerController, &AAdastreaPlayerController::ToggleInventory);
   ```

   Or in Blueprint:
   - Add Event for `IA_ToggleInventory`
   - Call `Toggle Inventory` node

#### Option B: Using Legacy Input

In your PlayerController Blueprint:
1. **Override** `SetupInputComponent`
2. **Add Input Action** nodes:
   - Action: "ToggleInventory" → Call `Toggle Inventory`
   - Action: "ToggleShipStatus" → Call `Toggle Ship Status`
   - etc.

---

## That's It! Everything Else is Automatic

Once you complete the 3 steps above:

✅ HUD appears automatically on game start  
✅ Press `I` (or your key) → Inventory opens  
✅ Press `I` again → Inventory closes  
✅ Mouse cursor appears/disappears automatically  
✅ Input mode switches automatically (Game ↔ UI)  
✅ Widget is initialized with correct data automatically  

---

## Widget-Specific Initialization

Each widget has automatic initialization:

### HUD Widget
- **Auto-creates** on `BeginPlay()`
- **Auto-initializes** with `InitializeHUD()`
- Always visible at bottom layer
- No user action needed

### Inventory Widget
- **Created** when first toggled
- **Auto-initializes** with player's `InventoryComponent`
- Finds the component automatically
- Just press the key to open

### Trading Widget
- **Created** when `OpenTrading(faction)` is called
- **Auto-initializes** with the faction you're trading with
- Call from interaction with NPC/station:
  ```cpp
  PlayerController->OpenTrading(MyFaction);
  ```

### Station Editor Widget
- **Created** when toggled near a station
- **Auto-finds** nearest station within radius
- **Auto-initializes** with station reference
- Just press the key when near a station

### Ship Status Widget
- **Created** when toggled
- **Auto-initializes** with controlled spaceship
- Just press the key while in a ship

### Station Management Widget
- **Created** when `OpenStationManagement(station)` is called
- **Auto-initializes** with the station reference
- Call from docking/interaction:
  ```cpp
  PlayerController->OpenStationManagement(DockedStation);
  ```

---

## Common Scenarios

### Scenario 1: "I press the key but nothing happens"

**Check:**
1. Is the Widget Class set in PlayerController Blueprint?
2. Is the Input Action bound correctly?
3. Check the Output Log for errors

**Log messages to look for:**
```
LogAdastrea: Error: InventoryWidgetClass is not set! Assign it in Blueprint.
```

**Fix:** Set the widget class in BP_AdastreaPlayerController

---

### Scenario 2: "Widget appears but is blank"

**Possible causes:**
1. Widget Blueprint has no UI components
2. Blueprint didn't override C++ functions to update UI

**Fix:**
- Add UI components in Designer (Text, Images, Buttons, etc.)
- Override BlueprintNativeEvent functions to bind data to UI

**Example:** Override `UpdateHealth` in WBP_HUD:
```
Event UpdateHealth (Current Health, Max Health)
  → Get HealthBar widget
  → Set Percent = Current Health / Max Health
```

---

### Scenario 3: "Widget appears but I can't interact with it"

**Check:**
- Is the widget receiving input?
- Is input mode set correctly?

**The C++ code already handles this**, but if you override `ShowWidget` functions:
```cpp
// Ensure this is called when showing:
SetInputMode(FInputModeGameAndUI());
bShowMouseCursor = true;
```

---

## Testing Your Setup

### Quick Test Checklist

1. **Play in Editor** (PIE)
2. **Check HUD** - Should appear automatically
3. **Press Inventory key** - Widget should appear
4. **Click UI elements** - Should be interactive
5. **Press Inventory key again** - Widget should close
6. **Check Output Log** - Should see creation messages

**Expected log output:**
```
LogAdastrea: Log: AdastreaPlayerController: Created and initialized HUD widget
LogAdastrea: Log: CreateInventoryWidget: Successfully created inventory widget
LogAdastrea: Log: ShowInventory: Inventory now visible
LogAdastrea: Log: HideInventory: Inventory hidden
```

---

## Advanced: Creating a New Widget

If you want to add a completely new widget type:

1. **Create C++ Widget Class** (optional, can use Blueprint only)
2. **Add to PlayerController** following the pattern:
   - Widget class property
   - Widget instance property
   - Toggle/Open/Close functions
   - Create/Show/Hide helper functions
3. **Follow Steps 1-3** above

**See full details in:** `WIDGET_INITIALIZATION_GUIDE.md`

---

## Reference: Available Toggle Functions

Call these from Blueprint or C++:

| Function | Widget | Description |
|----------|--------|-------------|
| `ToggleInventory()` | Inventory | Open/close inventory |
| `ToggleShipStatus()` | Ship Status | Open/close ship stats |
| `ToggleMainMenu()` | Main Menu | Open/close pause menu |
| `ToggleStationEditor()` | Station Editor | Open/close editor (near station) |
| `OpenTrading(Faction)` | Trading | Open trading with faction |
| `CloseTrading()` | Trading | Close trading |
| `OpenStationManagement(Station)` | Station Mgmt | Open station management |
| `CloseStationManagement()` | Station Mgmt | Close station management |

**Check functions:**
| Function | Returns | Description |
|----------|---------|-------------|
| `IsInventoryOpen()` | bool | Is inventory visible? |
| `IsShipStatusOpen()` | bool | Is ship status visible? |
| `IsMainMenuOpen()` | bool | Is menu visible? |
| `IsStationEditorOpen()` | bool | Is editor visible? |
| `IsTradingOpen()` | bool | Is trading visible? |
| `IsStationManagementOpen()` | bool | Is management visible? |

**Get functions:**
| Function | Returns | Description |
|----------|---------|-------------|
| `GetInventoryWidget()` | UInventoryWidget* | Get widget instance |
| `GetHUDWidget()` | UAdastreaHUDWidget* | Get HUD instance |
| `GetShipStatusWidget()` | UShipStatusWidget* | Get widget instance |
| ... | | (etc.) |

---

## Summary

### You ONLY need to:
1. ✅ Create Widget Blueprints (WBP_*)
2. ✅ Set Widget Classes in PlayerController Blueprint
3. ✅ Bind Input Actions to toggle functions

### You DON'T need to:
- ❌ Call `CreateWidget` yourself (automatic)
- ❌ Call `AddToViewport` yourself (automatic)
- ❌ Manage input modes (automatic)
- ❌ Initialize widget data (automatic)
- ❌ Write widget lifecycle code (automatic)

**Everything else is already implemented in C++!**

---

## Need More Details?

See the complete guide: `WIDGET_INITIALIZATION_GUIDE.md`

**Covers:**
- Detailed architecture
- All initialization patterns
- Blueprint creation workflow
- Troubleshooting
- Best practices
- Adding custom widgets

---

**Last Updated**: 2025-12-17  
**For Questions**: See WIDGET_INITIALIZATION_GUIDE.md or ask in Discord
