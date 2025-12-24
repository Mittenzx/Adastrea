# Widget Architecture Visual Diagram

## Widget System Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                         ADASTREA WIDGET SYSTEM                       │
└─────────────────────────────────────────────────────────────────────┘

         ┌────────────────────────────────────────────────┐
         │      AdastreaPlayerController (C++)            │
         │  - Manages widget lifecycle                    │
         │  - Handles CreateWidget calls                  │
         │  - Controls input modes                        │
         │  - Viewport management                         │
         └────────────────┬───────────────────────────────┘
                          │
                          │ Creates & Manages
                          ▼
    ┌─────────────────────────────────────────────────────────┐
    │                    WIDGET INSTANCES                      │
    └─────────────────────────────────────────────────────────┘

┌─────────────┬─────────────┬─────────────┬──────────────┬─────────────┐
│             │             │             │              │             │
▼             ▼             ▼             ▼              ▼             ▼
HUD        Inventory    Trading    Station        Ship        Main
Widget      Widget      Widget     Editor       Status       Menu
(Auto)    (Toggle)    (Toggle)    (Toggle)     (Toggle)    (Toggle)
```

---

## Widget Lifecycle Flowchart

### Pattern 1: Auto-Created Widget (HUD)

```
Game Start (BeginPlay)
        │
        ├─→ Check: HUDWidgetClass set?
        │           │
        │           ├─→ YES: Continue
        │           └─→ NO:  Log error, exit
        │
        ├─→ CreateWidget<UAdastreaHUDWidget>()
        │           │
        │           └─→ Widget Instance Created
        │
        ├─→ AddToViewport(0)  ← Z-order 0 (bottom)
        │
        ├─→ InitializeHUD()
        │           │
        │           └─→ Set default values
        │
        └─→ Widget Visible
                    │
                    └─→ NativeTick updates every frame
```

---

### Pattern 2: On-Demand Widget (Inventory, Trading, etc.)

```
User Input (Press Key)
        │
        ├─→ ToggleWidget() called
        │           │
        │           ├─→ Check: Widget already open?
        │           │           │
        │           │           ├─→ YES: HideWidget() → End
        │           │           └─→ NO:  Continue
        │           │
        │           └─→ ShowWidget()
        │                       │
        │                       ├─→ CreateWidget() (if not exists)
        │                       │           │
        │                       │           ├─→ Check: WidgetClass set?
        │                       │           │           │
        │                       │           │           ├─→ YES: Continue
        │                       │           │           └─→ NO:  Error
        │                       │           │
        │                       │           └─→ CreateWidget<T>()
        │                       │
        │                       ├─→ Initialize with data
        │                       │   (e.g., InventoryComponent)
        │                       │
        │                       ├─→ AddToViewport()
        │                       │
        │                       ├─→ SetInputMode(GameAndUI)
        │                       │
        │                       ├─→ bShowMouseCursor = true
        │                       │
        │                       └─→ Widget Visible
        │
        └─→ User Input (Press Key Again)
                    │
                    └─→ HideWidget()
                                │
                                ├─→ RemoveFromParent()
                                │
                                ├─→ SetInputMode(GameOnly)
                                │
                                ├─→ bShowMouseCursor = false
                                │
                                └─→ Widget Hidden (but still exists)
```

---

## Widget Integration Architecture

```
┌───────────────────────────────────────────────────────────────────┐
│                     BLUEPRINT CONFIGURATION                        │
│                  BP_AdastreaPlayerController                       │
│                                                                    │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │ Class Settings (EditDefaultsOnly properties)             │   │
│  │                                                           │   │
│  │  HUD Widget Class            = WBP_HUD                   │   │
│  │  Inventory Widget Class      = WBP_Inventory             │   │
│  │  Trading Widget Class        = WBP_Trading               │   │
│  │  Ship Status Widget Class    = WBP_ShipStatus            │   │
│  │  Station Editor Widget Class = WBP_StationEditor         │   │
│  │  Main Menu Widget Class      = WBP_MainMenu              │   │
│  │  Station Mgmt Widget Class   = WBP_StationManagement     │   │
│  └──────────────────────────────────────────────────────────┘   │
└────────────────────────────┬──────────────────────────────────────┘
                             │ References
                             ▼
┌───────────────────────────────────────────────────────────────────┐
│                      WIDGET BLUEPRINTS                             │
│                      (Content/UI/)                                 │
│                                                                    │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌────────────┐│
│  │ WBP_HUD    │  │WBP_Invntry │  │WBP_Trading │  │WBP_ShipStat││
│  │            │  │            │  │            │  │            ││
│  │ Parent:    │  │ Parent:    │  │ Parent:    │  │ Parent:    ││
│  │ Adastrea   │  │ Inventory  │  │ Trading    │  │ ShipStatus ││
│  │ HUDWidget  │  │ Widget     │  │ Interface  │  │ Widget     ││
│  │            │  │            │  │ Widget     │  │            ││
│  └────────────┘  └────────────┘  └────────────┘  └────────────┘│
│                                                                    │
│  Each Blueprint:                                                   │
│  - Inherits C++ functionality                                      │
│  - Adds UI design (visual layout)                                  │
│  - Can override BlueprintNativeEvents                              │
└────────────────────────────┬──────────────────────────────────────┘
                             │ Derives from
                             ▼
┌───────────────────────────────────────────────────────────────────┐
│                      C++ WIDGET CLASSES                            │
│                  (Source/Adastrea/UI/)                             │
│                                                                    │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌────────────┐│
│  │UAdastrea   │  │UInventory  │  │UTrading    │  │UShipStatus ││
│  │HUDWidget   │  │Widget      │  │Interface   │  │Widget      ││
│  │            │  │            │  │Widget      │  │            ││
│  │- Update    │  │- Init with │  │- Init with │  │- Init with ││
│  │  Health    │  │  Component │  │  Faction   │  │  Spaceship ││
│  │- Update    │  │- Refresh   │  │- Execute   │  │- Refresh   ││
│  │  Shield    │  │- Filter    │  │  Trade     │  │  Display   ││
│  │- Update    │  │- Sort      │  │- Cart Mgmt │  │- Show Stats││
│  │  Speed     │  │            │  │            │  │            ││
│  └────────────┘  └────────────┘  └────────────┘  └────────────┘│
└───────────────────────────────────────────────────────────────────┘
```

---

## Input Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                  INPUT SYSTEM                                │
└─────────────────────────────────────────────────────────────┘

Player Presses Key (e.g., 'I' for Inventory)
        │
        ▼
┌────────────────────────────┐
│ Enhanced Input System      │  or  │ Legacy Input System     │
│ - DA_InputConfig           │      │ - Input Action Mapping  │
│ - IA_ToggleInventory       │      │ - "ToggleInventory"     │
└────────────┬───────────────┘      └──────────┬──────────────┘
             │                                 │
             └─────────────┬───────────────────┘
                           ▼
              ┌───────────────────────────┐
              │ PlayerController receives │
              │ ToggleInventory event     │
              └─────────────┬─────────────┘
                            ▼
              ┌───────────────────────────┐
              │ AAdastreaPlayerController │
              │ ::ToggleInventory()       │
              └─────────────┬─────────────┘
                            ▼
                  ┌─────────────────┐
                  │ Widget visible? │
                  └────┬───────┬────┘
                       │       │
                   YES │       │ NO
                       │       │
                       ▼       ▼
              ┌──────────┐  ┌──────────┐
              │   Hide   │  │   Show   │
              │  Widget  │  │  Widget  │
              └──────────┘  └──────────┘
```

---

## Data Flow: Widget Initialization

### Example: Inventory Widget

```
┌─────────────────────────────────────────────────────────────────┐
│                     INITIALIZATION FLOW                          │
└─────────────────────────────────────────────────────────────────┘

Player calls ToggleInventory()
        │
        ├─→ ShowInventory()
        │           │
        │           ├─→ CreateInventoryWidget()
        │           │           │
        │           │           └─→ Widget Instance Created
        │           │
        │           ├─→ Find Player's Inventory Component
        │           │           │
        │           │           ├─→ GetPawn()
        │           │           │
        │           │           └─→ FindComponentByClass<UInventoryComponent>()
        │           │
        │           ├─→ Widget->InitializeInventory(InventoryComponent)
        │           │           │
        │           │           ├─→ Store component reference
        │           │           │
        │           │           ├─→ Bind to OnInventoryChanged event
        │           │           │
        │           │           └─→ Call RefreshInventory()
        │           │
        │           ├─→ AddToViewport()
        │           │
        │           └─→ NativeConstruct() called automatically
        │                       │
        │                       └─→ Blueprint can override to add custom logic
        │
        └─→ Widget displays with live inventory data
                    │
                    └─→ Updates automatically when inventory changes
```

### Example: Trading Widget

```
┌─────────────────────────────────────────────────────────────────┐
│                     INITIALIZATION FLOW                          │
└─────────────────────────────────────────────────────────────────┘

NPC/Station interaction calls OpenTrading(FactionDataAsset)
        │
        ├─→ CreateTradingWidget()
        │           │
        │           └─→ Widget Instance Created
        │
        ├─→ Widget->SetTradePartner(FactionDataAsset)
        │           │
        │           ├─→ Store faction reference
        │           │
        │           ├─→ Calculate prices based on faction relations
        │           │
        │           ├─→ Load available items
        │           │
        │           └─→ Fire OnTradeItemsUpdated event
        │
        ├─→ ShowTrading()
        │           │
        │           └─→ AddToViewport()
        │
        └─→ Widget displays with faction-specific pricing
                    │
                    └─→ Player can browse and execute trades
```

---

## Memory Management

```
┌─────────────────────────────────────────────────────────────────┐
│                    WIDGET MEMORY LIFECYCLE                       │
└─────────────────────────────────────────────────────────────────┘

Widget Creation
        │
        ├─→ CreateWidget<T>() called
        │           │
        │           └─→ Widget allocated in memory
        │
        ├─→ Store in PlayerController property (UPROPERTY)
        │           │
        │           └─→ Marked for Garbage Collection tracking
        │
        └─→ Widget persists in memory (not destroyed on hide)

Widget Usage
        │
        ├─→ AddToViewport() - Visible
        │
        ├─→ RemoveFromParent() - Hidden (but still in memory)
        │
        └─→ AddToViewport() again - Reused (no recreation needed)

Widget Destruction
        │
        ├─→ PlayerController destroyed (level change, game end)
        │           │
        │           └─→ Widget reference cleared
        │
        ├─→ NativeDestruct() called automatically
        │           │
        │           └─→ Cleanup (unbind events, release resources)
        │
        └─→ Garbage Collector destroys widget
```

**Key Points:**
- Widgets are created once and reused
- Hidden widgets stay in memory for performance
- UPROPERTY() ensures GC tracking
- NativeDestruct() handles cleanup
- No manual memory management needed

---

## Input Mode States

```
┌─────────────────────────────────────────────────────────────────┐
│                      INPUT MODE STATES                           │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────┐
│  FInputModeGameOnly     │  ← Gameplay (no UI open)
│  - No mouse cursor      │
│  - Game receives input  │
│  - UI does not respond  │
└────────┬────────────────┘
         │
         │ OpenWidget()
         ▼
┌─────────────────────────┐
│  FInputModeGameAndUI    │  ← UI Open (inventory, trading, etc.)
│  - Mouse cursor shown   │
│  - Game still updates   │
│  - UI receives input    │
│  - Can interact with UI │
└────────┬────────────────┘
         │
         │ CloseWidget()
         ▼
┌─────────────────────────┐
│  FInputModeGameOnly     │  ← Back to gameplay
│  - No mouse cursor      │
│  - Game receives input  │
└─────────────────────────┘

Alternative for some widgets:

┌─────────────────────────┐
│  FInputModeUIOnly       │  ← Full UI focus (pause menu)
│  - Mouse cursor shown   │
│  - Game paused          │
│  - Only UI receives     │
└─────────────────────────┘
```

**Automatic Management:**
```cpp
// When showing UI widget:
SetInputMode(FInputModeGameAndUI());
bShowMouseCursor = true;

// When hiding UI widget:
SetInputMode(FInputModeGameOnly());
bShowMouseCursor = false;
```

---

## Widget Layer Management (Z-Order)

```
┌─────────────────────────────────────────────────────────────────┐
│                      VIEWPORT Z-ORDER                            │
│                    (Higher = Front)                              │
└─────────────────────────────────────────────────────────────────┘

        ┌────────────────────────────────┐
        │  Top Layer (Z: 100+)           │  ← Tooltips, Dialogs
        ├────────────────────────────────┤
        │  UI Layer (Z: 50-99)           │  ← Menus, Trading, Inventory
        ├────────────────────────────────┤
        │  HUD Layer (Z: 0-49)           │  ← HUD, Status Bars
        ├────────────────────────────────┤
        │  Game World (3D)               │  ← Ships, Stations, Space
        └────────────────────────────────┘

Widget Z-Order assignments:
- AddToViewport(0)   ← HUD (bottom of UI stack)
- AddToViewport()    ← Default (next available)
- AddToViewport(100) ← High priority (top of UI stack)

Example:
HUDWidget->AddToViewport(0);           // Always at bottom
InventoryWidget->AddToViewport();      // Middle layer
TooltipWidget->AddToViewport(100);     // Always on top
```

---

## Complete Widget Integration Example

```
┌─────────────────────────────────────────────────────────────────┐
│          COMPLETE FLOW: Adding a New Widget Type                 │
└─────────────────────────────────────────────────────────────────┘

STEP 1: C++ Class (already done for existing widgets)
────────────────────────────────────────────────────────
    Source/Adastrea/UI/MyWidget.h/.cpp
            │
            ├─→ Inherit from UUserWidget
            ├─→ Add UCLASS macros
            ├─→ Add BlueprintNativeEvent functions
            └─→ Implement initialization logic

STEP 2: PlayerController Integration (already done)
────────────────────────────────────────────────────
    AdastreaPlayerController.h/.cpp
            │
            ├─→ Add widget class property (TSubclassOf<UMyWidget>)
            ├─→ Add widget instance property (UMyWidget*)
            ├─→ Add ToggleMyWidget() function
            ├─→ Add CreateMyWidget() helper
            ├─→ Add ShowMyWidget() helper
            ├─→ Add HideMyWidget() helper
            └─→ Add IsMyWidgetOpen() and GetMyWidget() functions

STEP 3: Create Blueprint Widget (DESIGNER TASK)
────────────────────────────────────────────────
    Content/UI/WBP_MyWidget
            │
            ├─→ Set Parent Class to UMyWidget
            ├─→ Design UI layout in Designer
            ├─→ Override BlueprintNativeEvents as needed
            └─→ Compile and Save

STEP 4: Configure Blueprint (DESIGNER TASK)
────────────────────────────────────────────
    BP_AdastreaPlayerController
            │
            └─→ Set "My Widget Class" property to WBP_MyWidget

STEP 5: Bind Input (DESIGNER TASK)
────────────────────────────────────
    Input Configuration (DA_InputConfig or legacy)
            │
            ├─→ Create Input Action (IA_ToggleMyWidget)
            ├─→ Bind key (e.g., 'M')
            └─→ Link to ToggleMyWidget() function

STEP 6: Test
────────────────────────────────────
    Play in Editor
            │
            ├─→ Widget class set? ✓
            ├─→ Press key → Widget appears? ✓
            ├─→ UI interactive? ✓
            ├─→ Press key → Widget hides? ✓
            └─→ Check logs for errors ✓

DONE! Widget fully integrated.
```

---

## Summary: Key Components

```
┌────────────────────────────────────────────────────────────┐
│                   WIDGET SYSTEM COMPONENTS                  │
├────────────────────────────────────────────────────────────┤
│                                                             │
│  1. C++ Widget Classes (Base functionality)                │
│     └─→ Source/Adastrea/UI/*.h/.cpp                       │
│                                                             │
│  2. PlayerController (Widget manager)                      │
│     └─→ Source/Adastrea/Player/AdastreaPlayerController   │
│                                                             │
│  3. Blueprint Widgets (Visual design)                      │
│     └─→ Content/UI/WBP_*.uasset                           │
│                                                             │
│  4. Blueprint Configuration (Widget class assignments)     │
│     └─→ Content/Blueprints/BP_AdastreaPlayerController    │
│                                                             │
│  5. Input Bindings (Key → Function mapping)                │
│     └─→ DA_InputConfig or Input Action Mappings           │
│                                                             │
└────────────────────────────────────────────────────────────┘
```

---

**For More Information:**
- Quick Start: `WIDGET_SETUP_QUICKSTART.md`
- Complete Guide: `WIDGET_INITIALIZATION_GUIDE.md`
- Code Examples: See existing widgets in `Source/Adastrea/UI/`

**Last Updated**: 2025-12-17
