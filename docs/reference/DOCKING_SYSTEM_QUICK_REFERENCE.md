# Docking System Quick Reference

**Specific Answer to: "Why do we have two docking module blueprints plus docking point scene components and docking point scene array?"**

---

## The Complete Answer

You have **4 distinct things** in the docking system, each with a specific purpose:

### 1. DockingBayModule (C++ Class)
**File**: `Source/Adastrea/Public/Stations/DockingBayModule.h`

```cpp
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ADockingBayModule : public ASpaceStationModule
{
    // Large docking facility
    // Power: 50 units
    // Capacity: 3-6 ships (large vessels)
};
```

**Purpose**: **Gameplay logic** for large docking bays
- Handles freighters and capital ships
- High power consumption (50 units)
- Expensive to build/maintain
- Has hangar doors, tractor beams, docking assistance

**Blueprint**: `BP_Module_DockingBay` (extends this C++ class)

---

### 2. DockingPortModule (C++ Class)
**File**: `Source/Adastrea/Public/Stations/DockingPortModule.h`

```cpp
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ADockingPortModule : public ASpaceStationModule
{
    // Small docking point
    // Power: 10 units
    // Capacity: 1 ship (small vessels)
};
```

**Purpose**: **Gameplay logic** for small docking ports
- Handles shuttles and fighters only
- Low power consumption (10 units)
- Cheap alternative to full bays
- Basic docking clamp only

**Blueprint**: `BP_Module_DockingPort` (extends this C++ class)

**Why Both Classes?**
- **Resource Management**: Bays cost 5x more power
- **Gameplay Variety**: Different ship sizes need different facilities
- **Economic Balance**: Ports are budget option, bays are premium
- **Station Design**: Mix and match for optimal layouts

---

### 3. DockingPoint (Scene Component)

**What is it?**
- A `USceneComponent` instance
- Represents a **physical attachment point** where ships dock
- Stores **transform data** (position, rotation, scale)
- Has **no gameplay logic** - just spatial information

**Implementation Example:**
```cpp
// Inside ADockingBayModule or ADockingPortModule
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Docking")
USceneComponent* DockingPoint1;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Docking")
USceneComponent* DockingPoint2;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Docking")
USceneComponent* DockingPoint3;
```

**Purpose**: **Transform/spatial data** for physical docking
- Marks **where** ships physically attach
- Editor-visible (can position in viewport)
- Inherits transform from parent module (moves with module)
- Can have trigger volumes attached

**Why Scene Components?**
1. **Transform Inheritance**: Automatically move/rotate with module
2. **Editor Visualization**: See docking points in level editor
3. **Modular Positioning**: Each point positioned independently
4. **Collision/Overlap**: Can detect when ship enters/leaves

---

### 4. DockingPoints Array

**What is it?**
- `TArray<USceneComponent*>` - Array of scene components
- Stores **multiple docking points** per module
- Allows modules to have **different slot counts**

**Implementation:**
```cpp
// In module base class or derived docking classes
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Docking")
TArray<USceneComponent*> DockingPoints;
```

**Purpose**: **Manage multiple simultaneous docking slots**
- **Scalability**: Easily add/remove slots
- **Iteration**: Loop through to find free slots
- **Flexibility**: Different modules = different slot counts
- **Runtime Tracking**: Know which slots are occupied

**Example Usage:**
```cpp
// Find free docking point
for (USceneComponent* Point : DockingPoints)
{
    if (!IsSlotOccupied(Point))
    {
        return Point; // Found free slot
    }
}
```

**Typical Slot Counts:**
- **DockingBay**: 3-6 slots (multiple large ships)
- **DockingPort**: 1 slot (single small ship)

---

## Visual Comparison

```
┌─────────────────────────────────────────────────────────┐
│         ADockingBayModule (Large Facility)              │
│  Power: -50 units | Capacity: 6 ships | Cost: High     │
└─────────────────────────────────────────────────────────┘
                    ↓ Has Array
┌─────────────────────────────────────────────────────────┐
│      TArray<USceneComponent*> DockingPoints (6)         │
└─────────────────────────────────────────────────────────┘
       ↓ Contains            ↓ Contains          ↓ Contains
┌──────────────┐      ┌──────────────┐    ┌──────────────┐
│ SceneComp #1 │      │ SceneComp #2 │    │ SceneComp #6 │
│ Location:    │      │ Location:    │    │ Location:    │
│ (100,0,0)    │      │ (200,0,0)    │... │ (600,0,0)    │
│ Occupied:No  │      │ Occupied:Yes │    │ Occupied:No  │
└──────────────┘      └──────────────┘    └──────────────┘

vs.

┌─────────────────────────────────────────────────────────┐
│         ADockingPortModule (Small Port)                 │
│  Power: -10 units | Capacity: 1 ship | Cost: Low       │
└─────────────────────────────────────────────────────────┘
                    ↓ Has Array
┌─────────────────────────────────────────────────────────┐
│      TArray<USceneComponent*> DockingPoints (1)         │
└─────────────────────────────────────────────────────────┘
                    ↓ Contains
              ┌──────────────┐
              │ SceneComp #1 │
              │ Location:    │
              │ (0,0,0)      │
              │ Occupied:No  │
              └──────────────┘
```

---

## How They Work Together

### Step 1: Module Provides Logic
```
ADockingBayModule (C++ class)
- Defines power consumption (50 units)
- Defines ship size limits (large vessels)
- Manages docking permissions
- Handles tractor beam activation
- Controls hangar door open/close
```

### Step 2: Scene Components Provide Location
```
USceneComponent* DockingPoint1
- Position: FVector(100, 50, 0)
- Rotation: FRotator(0, 90, 0)
- Scale: FVector(1, 1, 1)
- Occupied: false
```

### Step 3: Array Manages Multiple Slots
```
TArray<USceneComponent*> DockingPoints
- [0] = DockingPoint1 (free)
- [1] = DockingPoint2 (occupied by ShipA)
- [2] = DockingPoint3 (free)
- [3] = DockingPoint4 (occupied by ShipB)
- [4] = DockingPoint5 (free)
- [5] = DockingPoint6 (free)
```

### Step 4: Ship Requests Docking
```
1. Ship: "Request docking at station"
2. Station: Finds DockingBayModule (or DockingPortModule)
3. Module: Checks DockingPoints array for free slot
4. Module: Returns DockingPoint3 (first free slot)
5. Ship: Moves to DockingPoint3->GetComponentLocation()
6. Ship: Rotates to DockingPoint3->GetComponentRotation()
7. Module: Marks DockingPoint3 as occupied
8. Module: Opens hangar door
9. Ship: Lands and disables controls
10. Module: Triggers OnShipDocked event
```

---

## Code Examples

### Finding Free Docking Slot

```cpp
USceneComponent* ADockingBayModule::FindFreeDockingSlot()
{
    for (USceneComponent* Point : DockingPoints)
    {
        if (Point && !IsSlotOccupied(Point))
        {
            return Point;
        }
    }
    return nullptr; // No free slots
}
```

### Docking Ship to Slot

```cpp
void ASpaceship::DockAtSlot(USceneComponent* DockingSlot)
{
    if (!DockingSlot) return;
    
    // Get target transform
    FVector TargetLocation = DockingSlot->GetComponentLocation();
    FRotator TargetRotation = DockingSlot->GetComponentRotation();
    
    // Move ship to docking point
    SetActorLocationAndRotation(TargetLocation, TargetRotation);
    
    // Mark slot as occupied
    MarkSlotOccupied(DockingSlot, true);
    
    // Disable ship controls
    DisableInput(GetController());
    
    // Open UI
    OpenTradingInterface();
}
```

### Blueprint Usage

```
Event BeginPlay (BP_SpaceStation)
├─ Spawn BP_Module_DockingBay at (0, 0, 0)
├─ Spawn BP_Module_DockingPort at (500, 0, 0)
└─ Add modules to Modules array

Event Ship Requests Docking
├─ Get player ship location
├─ Find nearest docking module
│  └─ Iterate through Modules array
│     ├─ Check if module is DockingBay or DockingPort
│     └─ Calculate distance
├─ Call FindFreeDockingSlot on module
├─ If slot found:
│  ├─ Grant docking permission
│  ├─ Ship autopilot to slot location
│  ├─ Mark slot occupied
│  └─ Open trading interface
└─ Else:
   └─ Show "All docking bays full" message
```

---

## Design Rationale

### Why Not Just One Docking Class?

**Option A (Current)**: Two classes
- ✅ Clear gameplay distinction (bay vs port)
- ✅ Different power costs
- ✅ Different visual designs
- ✅ Extensible (add DockingRingModule, etc.)

**Option B (Alternative)**: One class with size parameter
- ❌ Less clear design intent
- ❌ More complex configuration
- ❌ Harder to balance
- ❌ Less designer-friendly

**Verdict**: Separate classes better for **clarity and gameplay balance**

---

### Why Scene Components for Docking Points?

**Option A (Current)**: USceneComponent
- ✅ Built-in transform inheritance
- ✅ Editor visualization
- ✅ Collision/overlap support
- ✅ Standard Unreal pattern

**Option B (Alternative)**: Raw FVector array
- ❌ No transform inheritance
- ❌ No editor visualization
- ❌ Manual transform calculations
- ❌ More error-prone

**Verdict**: Scene components are **standard Unreal practice**

---

### Why Array Instead of Fixed Slots?

**Option A (Current)**: TArray<USceneComponent*>
- ✅ Scalable (1 to N slots)
- ✅ Easy iteration
- ✅ Flexible per module
- ✅ Runtime additions possible

**Option B (Alternative)**: Fixed properties
```cpp
USceneComponent* Slot1;
USceneComponent* Slot2;
USceneComponent* Slot3;
// etc...
```
- ❌ Fixed slot count
- ❌ Code duplication
- ❌ Hard to iterate
- ❌ Not scalable

**Verdict**: Array is **more flexible and maintainable**

---

## Summary Table

| Component | Type | Purpose | Why Needed |
|-----------|------|---------|------------|
| **DockingBayModule** | C++ Class | Gameplay logic for large bays | Distinct gameplay (power, cost, capacity) |
| **DockingPortModule** | C++ Class | Gameplay logic for small ports | Budget alternative with different stats |
| **DockingPoint** | Scene Component | Physical attachment point | Transform data, editor visualization |
| **DockingPoints Array** | TArray | Multiple slot management | Scalability, iteration, runtime tracking |

---

## Common Questions

**Q: Can I just use one docking class and skip the other?**
- **A**: Yes for prototyping, but you lose gameplay variety and balance options. MVP could start with just DockingPortModule.

**Q: Do I need scene components? Can I just use vectors?**
- **A**: Scene components are standard Unreal practice. They provide transform inheritance, editor tools, and collision. Raw vectors require manual management.

**Q: Why not just have one docking slot per module?**
- **A**: Large bays need multiple slots for realism and gameplay. Small ports have 1 slot. Array allows flexibility.

**Q: Can I add more docking types later?**
- **A**: Yes! Easy to add `ADockingRingModule`, `ACapitalShipDockModule`, etc. Each inherits from `ASpaceStationModule`.

**Q: For MVP, which do I need?**
- **A**: Start with **one** docking module type (either Bay or Port). Add scene component array. That's enough for trading MVP.

---

## MVP Simplification

For **Trade Simulator MVP**, you can simplify:

```cpp
// Minimal MVP docking
ADockingPortModule (just one class)
├─ TArray<USceneComponent*> DockingPoints (1 slot)
└─ Functions:
   ├─ FindFreeDockingSlot()
   ├─ MarkSlotOccupied()
   └─ OnShipDocked()
```

**Create:**
- ✅ 1 C++ docking class (DockingPortModule is simpler)
- ✅ 1 Blueprint: `BP_Module_DockingPort`
- ✅ Add 1 scene component per module
- ✅ Store in TArray for consistency

**Skip until post-MVP:**
- ❌ DockingBayModule (large bays)
- ❌ Multiple slots per module
- ❌ Complex docking animations
- ❌ Tractor beam effects

---

## Related Files

**C++ Headers:**
- `Source/Adastrea/Public/Stations/DockingBayModule.h`
- `Source/Adastrea/Public/Stations/DockingPortModule.h`
- `Source/Adastrea/Public/Stations/SpaceStationModule.h` (base class)
- `Source/Adastrea/Public/Stations/SpaceStation.h`

**Documentation:**
- `docs/reference/BLUEPRINT_ARCHITECTURE_GUIDE.md` (main guide)
- `docs/reference/BLUEPRINT_TREE_DIAGRAM.md` (visual tree)
- `Assets/StationEditorSystemGuide.md` (station system)

---

**End of Docking System Quick Reference**
