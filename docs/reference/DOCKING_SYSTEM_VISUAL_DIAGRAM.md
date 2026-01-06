# Docking System Visual Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    SPACE STATION ACTOR                          │
│                    (ASpaceStation)                              │
│                                                                 │
│  TArray<ASpaceStationModule*> Modules                          │
│  ├─ BP_Module_DockingBay (Large)                              │
│  ├─ BP_Module_DockingPort (Small)                             │
│  ├─ BP_Module_Reactor                                         │
│  └─ ... (other modules)                                       │
└─────────────────────────────────────────────────────────────────┘
                    │
                    │ Contains
                    ↓
┌─────────────────────────────────────────────────────────────────┐
│          MODULE 1: ADockingBayModule (Large)                   │
│          Blueprint: BP_Module_DockingBay                        │
│                                                                 │
│  Properties:                                                    │
│  ├─ ModuleType: "Docking Bay"                                 │
│  ├─ ModulePower: -50 (consumes 50 units)                     │
│  ├─ ModuleGroup: Docking                                      │
│  └─ Capacity: 3-6 large ships                                │
│                                                                 │
│  TArray<USceneComponent*> DockingPoints (6 slots):            │
│  ┌────────────┬────────────┬────────────┬────────────┐      │
│  │  Slot 1    │  Slot 2    │  Slot 3    │  Slot 4    │      │
│  │ (100,0,0)  │ (200,0,0)  │ (300,0,0)  │ (400,0,0)  │      │
│  │  FREE      │ OCCUPIED   │  FREE      │  FREE      │      │
│  │            │ (Ship A)   │            │            │      │
│  └────────────┴────────────┴────────────┴────────────┘      │
│  ┌────────────┬────────────┐                                  │
│  │  Slot 5    │  Slot 6    │                                  │
│  │ (500,0,0)  │ (600,0,0)  │                                  │
│  │ OCCUPIED   │  FREE      │                                  │
│  │ (Ship B)   │            │                                  │
│  └────────────┴────────────┘                                  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│          MODULE 2: ADockingPortModule (Small)                  │
│          Blueprint: BP_Module_DockingPort                       │
│                                                                 │
│  Properties:                                                    │
│  ├─ ModuleType: "Docking Port"                                │
│  ├─ ModulePower: -10 (consumes 10 units)                     │
│  ├─ ModuleGroup: Docking                                      │
│  └─ Capacity: 1 small ship                                   │
│                                                                 │
│  TArray<USceneComponent*> DockingPoints (1 slot):             │
│  ┌────────────┐                                                │
│  │  Slot 1    │                                                │
│  │  (0,0,0)   │                                                │
│  │ OCCUPIED   │                                                │
│  │ (Ship C)   │                                                │
│  └────────────┘                                                │
└─────────────────────────────────────────────────────────────────┘

LEGEND:
───────
Module Classes (C++)          = Gameplay logic, power, capacity
Scene Component Pointers      = USceneComponent* docking point instances (transforms)
Array                         = Slot management and iteration
FREE/OCCUPIED                 = Runtime slot tracking
```

## Ship Docking Workflow

```
Step 1: Ship Requests Docking
┌──────────────┐
│ Player Ship  │  "Request docking at Station Alpha"
│ (BP_Player)  │
└──────────────┘
       │
       │ Request
       ↓
┌──────────────────────────────────────┐
│ Station: Find Available Module       │
│ Iterate: Modules array               │
│ Filter: DockingBay or DockingPort    │
│ Check: Distance to ship               │
└──────────────────────────────────────┘
       │
       │ Found: BP_Module_DockingBay
       ↓
Step 2: Find Free Slot
┌──────────────────────────────────────┐
│ Module: FindFreeDockingSlot()        │
│ Iterate: DockingPoints array         │
│ Check: Each slot for occupation      │
│ Return: First free USceneComponent*  │
└──────────────────────────────────────┘
       │
       │ Returns: Slot 3 (300, 0, 0)
       ↓
Step 3: Guide Ship to Slot
┌──────────────────────────────────────┐
│ Ship: NavigationComponent            │
│ Get: DockingPoint location/rotation  │
│ Move: To (300, 0, 0)                 │
│ Rotate: Match docking point rotation │
└──────────────────────────────────────┘
       │
       │ Arrival
       ↓
Step 4: Complete Docking
┌──────────────────────────────────────┐
│ Module: MarkSlotOccupied(Slot 3,    │
│         DockedShip, true)            │
│ Ship: Disable controls               │
│ Station: Open trading interface      │
│ VFX: Play docking effects           │
└──────────────────────────────────────┘
```

## Code Flow

```cpp
// 1. Station finds available docking module
ADockingBayModule* FindNearestDockingModule(ASpaceship* Ship)
{
    for (ASpaceStationModule* Module : Modules)
    {
        if (ADockingBayModule* Bay = Cast<ADockingBayModule>(Module))
        {
            if (Bay->HasFreeSlots())
                return Bay;
        }
    }
    return nullptr;
}

// 2. Module finds free slot
USceneComponent* ADockingBayModule::FindFreeDockingSlot()
{
    for (USceneComponent* Point : DockingPoints)
    {
        if (!IsSlotOccupied(Point))
            return Point;
    }
    return nullptr;
}

// 3. Ship moves to slot
void ASpaceship::DockAtSlot(USceneComponent* Slot)
{
    FVector TargetLoc = Slot->GetComponentLocation();
    FRotator TargetRot = Slot->GetComponentRotation();
    
    NavigationComponent->MoveToLocation(TargetLoc, TargetRot);
    
    // On arrival:
    OnDockingComplete(Slot);
}

// 4. Mark slot as occupied / freed
void ADockingBayModule::MarkSlotOccupied(USceneComponent* Slot, ASpaceship* DockedShip, bool bOccupied)
{
    if (!Slot || !DockedShip)
    {
        return;
    }

    if (bOccupied)
    {
        // Store ship reference
        OccupiedSlots.Add(Slot, DockedShip);

        // Disable ship controls while docked
        DockedShip->DisableInput();

        // Open trading UI
        OpenTradingInterface();
    }
    else
    {
        // Slot is now free
        OccupiedSlots.Remove(Slot);
    }
}
```

## Why This Design?

```
┌─────────────────────────────────────────────────────────────┐
│ QUESTION: Why not just one "Docking" class?                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ OPTION A (Current): Two Classes                            │
│ ✅ Clear gameplay distinction                              │
│ ✅ Different power costs (50 vs 10)                        │
│ ✅ Different visual designs                                │
│ ✅ Different capacity (6 vs 1)                             │
│ ✅ Extensible (can add more types)                         │
│                                                             │
│ OPTION B (Alternative): One class with parameters          │
│ ❌ Less clear design intent                                │
│ ❌ More complex configuration                              │
│ ❌ Harder to balance                                       │
│                                                             │
│ VERDICT: Separate classes = Better gameplay variety        │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ QUESTION: Why USceneComponent instead of FVector?          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ OPTION A (Current): USceneComponent                        │
│ ✅ Transform inheritance (moves with module)               │
│ ✅ Editor visualization (see in viewport)                  │
│ ✅ Collision/overlap support                               │
│ ✅ Standard Unreal pattern                                 │
│                                                             │
│ OPTION B (Alternative): Raw FVector                        │
│ ❌ No transform inheritance                                │
│ ❌ No editor visualization                                 │
│ ❌ Manual calculations needed                              │
│                                                             │
│ VERDICT: Scene components = Standard best practice         │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ QUESTION: Why TArray instead of fixed properties?          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ OPTION A (Current): TArray<USceneComponent*>               │
│ ✅ Scalable (1 to N slots)                                 │
│ ✅ Easy iteration (for loops)                              │
│ ✅ Flexible per module                                     │
│ ✅ Runtime additions possible                              │
│                                                             │
│ OPTION B (Alternative): Slot1, Slot2, Slot3...            │
│ ❌ Fixed slot count                                        │
│ ❌ Code duplication                                        │
│ ❌ Hard to iterate                                         │
│                                                             │
│ VERDICT: Array = More flexible and maintainable            │
└─────────────────────────────────────────────────────────────┘
```

## Summary

**4 Distinct Layers:**

1. **Module Classes** (ADockingBayModule, ADockingPortModule)
   - Purpose: Gameplay logic
   - Defines: Power, capacity, cost
   - Why: Gameplay variety

2. **Scene Components** (USceneComponent* DockingPoint)
   - Purpose: Transform data
   - Stores: Location, rotation
   - Why: Editor tools, inheritance

3. **Array** (TArray<USceneComponent*> DockingPoints)
   - Purpose: Slot management
   - Enables: Multiple slots, iteration
   - Why: Flexibility, scalability

4. **Runtime Tracking** (Occupied/Free state)
   - Purpose: Manage docking
   - Tracks: Which slots have ships
   - Why: Prevent double-docking

**Single Responsibility Principle:**
- Each layer has ONE clear purpose
- Separation = Easier maintenance
- Modular = Extensible design
