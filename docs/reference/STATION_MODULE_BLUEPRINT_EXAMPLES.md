# Station Module System - Blueprint Examples

**Practical Blueprint examples** for using the module aggregation system

---

## Example 1: Opening Trading UI on Station Dock

### Scenario
Player docks at a station. Check if station has a marketplace before opening trading UI.

### Blueprint Event Graph

```
┌─────────────────────────────────────────────────────────────┐
│ Event: On Docked (Player docks at station)                  │
│   Inputs: Station (ASpaceStation reference)                 │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Has Marketplace?                                │
│   (BlueprintCallable, Pure)                                 │
│   Returns: bool                                             │
└─────────────────────────────────────────────────────────────┘
                          ▼
                    ┌─────┴─────┐
                    │  Branch   │
                    └─────┬─────┘
                ┌─────────┴──────────┐
                ▼                    ▼
         ┌────────────┐      ┌────────────────────┐
         │    TRUE    │      │       FALSE        │
         └────────────┘      └────────────────────┘
                ▼                    ▼
┌──────────────────────────┐ ┌─────────────────────────────┐
│ Get Marketplace Module   │ │ Show Notification:          │
│   Input: Station         │ │ "No trading available"      │
│   Returns: Marketplace*  │ │                             │
└──────────────────────────┘ └─────────────────────────────┘
                ▼
┌──────────────────────────┐
│ Open Trading UI Widget   │
│   Pass: Marketplace      │
│   Pass: Player Ship      │
└──────────────────────────┘
```

### Blueprint Node Sequence

1. **Event: On Docked** (Custom event or interface call)
2. **Has Marketplace?** (Station → Has Marketplace)
3. **Branch** (Condition: result from step 2)
4. If **TRUE**:
   - **Get Marketplace Module** (Station → Get Marketplace Module)
   - **Create Widget** (WBP_TradingUI)
   - **Add to Viewport**
5. If **FALSE**:
   - **Print String** or **Show Notification**: "No marketplace at this station"

---

## Example 2: Station Status Display Widget

### Scenario
Create a UI widget that shows station facilities and power status.

### Widget Blueprint (WBP_StationStatus)

#### Properties

```
Station (ASpaceStation reference) - Set by parent widget or player controller
```

#### Event Graph - Update Display

```
┌─────────────────────────────────────────────────────────────┐
│ Event: Construct (Widget created)                           │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ Call: Update Station Info                                   │
│   (Custom function in this widget)                          │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ Function: Update Station Info                               │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Get Station Name                                │
│   Returns: FText                                            │
│   → Set Text: StationNameTextBlock                          │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Get Module Count By Group (Docking)             │
│   Input: Module Group = Docking                             │
│   Returns: int32                                            │
│   → Format Text: "Docking Bays: {0}"                        │
│   → Set Text: DockingCountTextBlock                         │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Get Module Count By Group (Storage)             │
│   Input: Module Group = Storage                             │
│   Returns: int32                                            │
│   → Format Text: "Cargo Holds: {0}"                         │
│   → Set Text: StorageCountTextBlock                         │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Get Power Balance                               │
│   Returns: float                                            │
│   → Branch (Is Power Balance > 0?)                          │
│       TRUE:  Set Color: Green                               │
│              Format Text: "+{0} MW (Surplus)"               │
│       FALSE: Set Color: Red                                 │
│              Format Text: "{0} MW (Deficit)"                │
│   → Set Text: PowerStatusTextBlock                          │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Has Marketplace?                                │
│   Returns: bool                                             │
│   → Set Visibility: TradingIconImage                        │
│       TRUE:  Visible                                        │
│       FALSE: Hidden or Collapsed                            │
└─────────────────────────────────────────────────────────────┘
```

#### Widget Hierarchy (Designer Tab)

```
Canvas Panel
├─ StationNameTextBlock (Text: "Station Name")
├─ Facilities Panel (Vertical Box)
│  ├─ DockingCountTextBlock (Text: "Docking Bays: 0")
│  ├─ StorageCountTextBlock (Text: "Cargo Holds: 0")
│  └─ PowerStatusTextBlock (Text: "Power: 0 MW")
└─ Icons Panel (Horizontal Box)
   └─ TradingIconImage (Visibility: Collapsed)
```

---

## Example 3: Filter Modules by Type

### Scenario
Get all docking modules and check their status for a "Docking Status" screen.

### Blueprint Function

```
┌─────────────────────────────────────────────────────────────┐
│ Function: Get Docking Status                                │
│   Input: Station (ASpaceStation reference)                  │
│   Output: StatusText (FText)                                │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Get Modules By Group (Docking)                  │
│   Input: Module Group = Docking                             │
│   Returns: Array<ASpaceStationModule*>                      │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ Get Array Length                                            │
│   Returns: int32 (number of docking modules)                │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ Branch (Array Length > 0?)                                  │
└─────────────────────────────────────────────────────────────┘
        ▼                                    ▼
   ┌────────┐                         ┌──────────┐
   │  TRUE  │                         │  FALSE   │
   └────────┘                         └──────────┘
        ▼                                    ▼
┌──────────────────────┐         ┌──────────────────────┐
│ ForEach Loop         │         │ Set StatusText:      │
│   Array: Docking     │         │ "No docking bays"    │
│   Modules            │         └──────────────────────┘
│                      │
│ Loop Body:           │
│ - Get Module Type    │
│ - Append to String   │
│   "• {ModuleType}\n" │
└──────────────────────┘
        ▼
┌──────────────────────┐
│ Set StatusText:      │
│ "Available Docking:" │
│ + Accumulated String │
└──────────────────────┘
```

---

## Example 4: Power Management Warning

### Scenario
Show warning if station has negative power balance (insufficient power).

### Blueprint Tick Event (or Timer)

```
┌─────────────────────────────────────────────────────────────┐
│ Event: Tick (or Event: Timer - Check Power Status)          │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Get Power Balance                               │
│   Returns: float                                            │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ Branch (Power Balance < 0?)                                 │
└─────────────────────────────────────────────────────────────┘
        ▼                                    ▼
   ┌────────┐                         ┌──────────┐
   │  TRUE  │                         │  FALSE   │
   └────────┘                         └──────────┘
        ▼                                    ▼
┌──────────────────────┐         ┌──────────────────────┐
│ Show Warning         │         │ Hide Warning         │
│ "Power Deficit!"     │         │ (if visible)         │
│                      │         └──────────────────────┘
│ Get Total Power      │
│ Consumption          │
│ Get Total Power      │
│ Generation           │
│                      │
│ Format:              │
│ "Consuming: {0} MW"  │
│ "Generating: {1} MW" │
│ "Deficit: {2} MW"    │
└──────────────────────┘
```

---

## Example 5: Prerequisites Check for Module Addition

### Scenario
Check if station can support a new module (e.g., marketplace requires docking).

### Blueprint Function (Editor/Construction Script)

```
┌─────────────────────────────────────────────────────────────┐
│ Function: Can Add Marketplace                               │
│   Input: Station (ASpaceStation reference)                  │
│   Output: CanAdd (bool), ErrorMessage (FText)               │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Has Docking Capability?                         │
│   Returns: bool                                             │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ Branch (Has Docking?)                                       │
└─────────────────────────────────────────────────────────────┘
        ▼                                    ▼
   ┌────────┐                         ┌──────────┐
   │  TRUE  │                         │  FALSE   │
   └────────┘                         └──────────┘
        ▼                                    ▼
┌──────────────────────┐         ┌──────────────────────────┐
│ [Station] → Has      │         │ Set CanAdd: false        │
│ Marketplace?         │         │ Set ErrorMessage:        │
│   Returns: bool      │         │ "Requires docking bay"   │
└──────────────────────┘         └──────────────────────────┘
        ▼
┌──────────────────────┐
│ Branch (Already has?)│
└──────────────────────┘
   ▼              ▼
 TRUE          FALSE
   ▼              ▼
┌────────────┐ ┌────────────┐
│ CanAdd:    │ │ CanAdd:    │
│ false      │ │ true       │
│ Message:   │ │ Message:   │
│ "Already  │ │ "OK"       │
│  exists"   │ │            │
└────────────┘ └────────────┘
```

---

## Example 6: Dynamic Module Spawning (Advanced)

### Scenario
Allow player to purchase and add new modules to their station (Post-MVP feature).

### Blueprint Event Graph

```
┌─────────────────────────────────────────────────────────────┐
│ Event: On Purchase Module Button Clicked                    │
│   Inputs: ModuleClass (TSubclassOf<ASpaceStationModule>)    │
│           Location (FVector)                                │
└─────────────────────────────────────────────────────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────┐
│ Check Player Credits Sufficient                             │
│   Branch: Can Afford?                                       │
└─────────────────────────────────────────────────────────────┘
        ▼
   ┌────────┐
   │  TRUE  │
   └────────┘
        ▼
┌─────────────────────────────────────────────────────────────┐
│ Spawn Actor from Class                                      │
│   Class: ModuleClass                                        │
│   Transform: Location + Station Transform                   │
│   Owner: Station                                            │
│   Returns: ASpaceStationModule*                             │
└─────────────────────────────────────────────────────────────┘
        ▼
┌─────────────────────────────────────────────────────────────┐
│ [Station] → Add Module At Location                          │
│   Input: Spawned Module                                     │
│   Input: Relative Location                                  │
│   Returns: bool (success)                                   │
└─────────────────────────────────────────────────────────────┘
        ▼
┌─────────────────────────────────────────────────────────────┐
│ Branch (Add Module Success?)                                │
└─────────────────────────────────────────────────────────────┘
        ▼                                    ▼
   ┌────────┐                         ┌──────────┐
   │  TRUE  │                         │  FALSE   │
   └────────┘                         └──────────┘
        ▼                                    ▼
┌──────────────────────┐         ┌──────────────────────┐
│ Deduct Credits       │         │ Destroy Spawned      │
│ Show Success Message │         │ Module               │
│ Update UI            │         │ Show Error Message   │
└──────────────────────┘         └──────────────────────┘
```

---

## Common Blueprint Nodes Reference

### Station Capability Checks

| Node Name | Category | Inputs | Outputs | Pure? |
|-----------|----------|--------|---------|-------|
| **Has Docking Capability** | Station\|Capabilities | Station* | bool | ✅ |
| **Has Marketplace** | Station\|Capabilities | Station* | bool | ✅ |
| **Has Cargo Storage** | Station\|Capabilities | Station* | bool | ✅ |

### Power Management

| Node Name | Category | Inputs | Outputs | Pure? |
|-----------|----------|--------|---------|-------|
| **Get Total Power Consumption** | Station\|Modules | Station* | float | ✅ |
| **Get Total Power Generation** | Station\|Modules | Station* | float | ✅ |
| **Get Power Balance** | Station\|Modules | Station* | float | ✅ |

### Module Queries

| Node Name | Category | Inputs | Outputs | Pure? |
|-----------|----------|--------|---------|-------|
| **Get Module Count By Group** | Station\|Modules | Station*, ModuleGroup | int32 | ✅ |
| **Get Modules By Group** | Station\|Modules | Station*, ModuleGroup | Array | ❌ |
| **Get Modules** | Station | Station* | Array | ❌ |
| **Get Marketplace Module** | Station\|Trading | Station* | Module* | ❌ |
| **Get Marketplace Modules** | Station\|Trading | Station* | Array | ❌ |

---

## Tips for Blueprint Implementation

### Performance

1. **Cache Station Reference**: Don't search for station every frame
2. **Use Pure Nodes**: Capability checks are pure (no side effects)
3. **Update on Events**: Don't check every tick, use events or timers
4. **Batch Updates**: Update UI once, not per property

### Best Practices

1. **Check Nulls**: Always validate station reference is valid
2. **Handle Empty Arrays**: Check array length before ForEach
3. **Use Categories**: Organize your custom events by category
4. **Comment Your Graphs**: Explain complex logic

### Common Patterns

```
Pattern 1: Null Check
[Get Station] → [Is Valid?] → Branch
                                 ↓
                            TRUE: Continue
                            FALSE: Return/Log Error

Pattern 2: Array Processing
[Get Modules] → [Length > 0?] → Branch
                                   ↓
                              TRUE: ForEach Loop
                              FALSE: Show "No modules"

Pattern 3: Conditional UI
[Has Capability] → [Set Visibility]
                    TRUE: Visible
                    FALSE: Collapsed
```

---

## Related Documentation

- **[ADDING_MODULES_TO_STATIONS.md](ADDING_MODULES_TO_STATIONS.md)** - Complete guide
- **[STATION_MODULE_ARCHITECTURE.md](STATION_MODULE_ARCHITECTURE.md)** - System architecture
- **C++ API**: `Source/Adastrea/Public/Stations/SpaceStation.h`

---

**Last Updated**: 2026-01-12  
**Status**: Complete - Ready for MVP Blueprint Implementation
