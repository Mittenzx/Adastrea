# Docking System Visual Reference

This document provides visual diagrams for the simplified docking system.

---

## Simple Docking Flow (Player Experience)

```
┌─────────────────────────────────────────────────────────────┐
│                    PLAYER DOCKING FLOW                      │
└─────────────────────────────────────────────────────────────┘

    Player Ship                                Station
         🚀                                      🏭
          │
          │  1. Fly toward station
          │     (approach within 2000 units)
          ↓
    ┌──────────┐
    │ In Range │ ───────→  Docking Prompt Shows
    └──────────┘           "Press F to Dock"
          │
          │  2. Press F key
          │
          ↓
    ┌──────────────┐
    │ RequestDock  │ ─────→ Validate:
    │              │         • Station nearby?
    │              │         • Slots available?
    │              │         • Within range?
    └──────────────┘
          │
          │  All checks pass ✓
          │
          ↓
    ┌──────────────┐
    │ Teleport     │ ─────→ Instant move to docking point
    │ to Dock      │         (no animation, just snap)
    └──────────────┘
          │
          ↓
    ┌──────────────┐
    │ Docked!      │ ─────→ • Hide ship
    │              │         • Disable controls
    │              │         • Open trade UI 💰
    └──────────────┘
          │
          │  3. Trade goods
          │     (buy low, sell high)
          │
          ↓
    ┌──────────────┐
    │ Press Undock │
    │ (F key)      │
    └──────────────┘
          │
          ↓
    ┌──────────────┐
    │ Undocked     │ ─────→ • Show ship
    │              │         • Enable controls
    │              │         • Close trade UI
    │              │         • Push away from station
    └──────────────┘
          │
          │  4. Fly to next station
          │     and repeat! 🔄
          ↓
         🚀 → [Next Station]
```

---

## Technical Code Flow

```
┌────────────────────────────────────────────────────────────────┐
│                     C++ FUNCTION CALL FLOW                     │
└────────────────────────────────────────────────────────────────┘

Player Input (F key)
       │
       ↓
┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ RequestDocking()         ┃  Source/Adastrea/Private/Ships/Spaceship.cpp:997
┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛
       │
       ├─→ Check: NearbyStation != nullptr
       │   └─→ If null: Log warning, return
       │
       ├─→ Check: Already docked?
       │   └─→ If yes: Call Undock() instead
       │
       ├─→ Cast to ADockingBayModule
       │   └─→ If null: Log warning, return
       │
       ├─→ Check: HasAvailableDocking()
       │   └─→ If no: Log "slots full", return
       │
       ├─→ Get available docking point
       │   └─→ If null: Log error, return
       │
       ├─→ Check distance to docking point
       │   └─→ If > DockingRange: Log "too far", return
       │
       ↓
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ NavigateToDockingPoint()        ┃  Line 1049
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
       │
       ├─→ Get docking point location
       ├─→ Get docking point rotation
       ├─→ SetActorLocationAndRotation() ← INSTANT teleport
       │
       ↓
┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ CompleteDocking()        ┃  Line 1070
┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛
       │
       ├─→ Set bIsDocked = true
       ├─→ Notify station: DockShip()
       ├─→ DisableInput(PlayerController)
       ├─→ SetActorHiddenInGame(true)
       ├─→ Create TradingWidget
       ├─→ AddToViewport()
       ├─→ Set input mode to UI only
       │
       └─→ Player can now trade! 💰
```

---

## Distance Check Visualization

```
┌─────────────────────────────────────────────────────────────┐
│                    DOCKING RANGE CHECK                      │
└─────────────────────────────────────────────────────────────┘

                    Station with Docking Point
                           🏭
                           │
                           │ DockingPoint
                           ↓
                    ┌──────●──────┐
                    │             │
                    │             │
                ●   │   ●     ●   │   ●
              TOO   │  GOOD   │   │  TOO
              FAR   │  RANGE  │   │  FAR
                    │  (≤2000)│   │
                    │         │   │
                    └─────────────┘
                        Range
                        Circle

Legend:
  ● = Docking Point (on station)
  ● = Ship position

Distance Calculation:
  Distance = FVector::Dist(ShipLocation, DockingPointLocation)

  if (Distance <= DockingRange)
      ✓ Allow docking
  else
      ✗ "Too far from docking point" warning
```

---

## Station Docking Point Setup

```
┌─────────────────────────────────────────────────────────────┐
│              STATION BLUEPRINT HIERARCHY                    │
└─────────────────────────────────────────────────────────────┘

BP_SpaceStation
  │
  ├─ StaticMesh (station model)
  │
  ├─ DockingBayModule (Component)
  │    │
  │    ├─ Properties:
  │    │    • MaxDockedShips = 4
  │    │    • DockingPoints = [array of scene components]
  │    │
  │    └─ DockingPoints Array:
  │         [0] → DockingPoint_01
  │         [1] → DockingPoint_02
  │         [2] → DockingPoint_03
  │         [3] → DockingPoint_04
  │
  ├─ DockingPoint_01 (SceneComponent)
  │    • Location: (500, 0, 100)  ← Ship will teleport here
  │    • Rotation: (0, 90, 0)     ← Ship will match this rotation
  │
  ├─ DockingPoint_02 (SceneComponent)
  │    • Location: (500, 200, 100)
  │    • Rotation: (0, 90, 0)
  │
  ├─ DockingPoint_03 (SceneComponent)
  │    • Location: (500, -200, 100)
  │    • Rotation: (0, 90, 0)
  │
  └─ DockingPoint_04 (SceneComponent)
       • Location: (500, 0, 300)
       • Rotation: (0, 90, 0)

⚠️ Important: DockingPoints array must reference these scene components!
```

---

## Input System Configuration

```
┌─────────────────────────────────────────────────────────────┐
│                   INPUT BINDING CHAIN                       │
└─────────────────────────────────────────────────────────────┘

1. Input Action Asset
   ┌────────────────────────┐
   │  IA_Dock               │
   │  (Content/Input/       │
   │   Actions/)            │
   │                        │
   │  Value Type: Digital   │
   │  (Boolean)             │
   └────────────────────────┘
              │
              │ Referenced by
              ↓
2. Input Mapping Context
   ┌────────────────────────┐
   │  IMC_Spaceship         │
   │  (Content/Input/)      │
   │                        │
   │  Mappings:             │
   │  • F key → IA_Dock     │
   └────────────────────────┘
              │
              │ Referenced by
              ↓
3. Input Config Data Asset
   ┌────────────────────────┐
   │  DA_InputConfig        │
   │  (Content/DataAssets/  │
   │   Input/)              │
   │                        │
   │  DockAction = IA_Dock  │
   └────────────────────────┘
              │
              │ Used by
              ↓
4. Spaceship C++ Code
   ┌──────────────────────────────────────────┐
   │  ASpaceship::SetupPlayerInputComponent() │
   │                                          │
   │  if (DockAction)                         │
   │  {                                       │
   │    BindAction(DockAction,                │
   │               Triggered,                 │
   │               RequestDocking);           │
   │  }                                       │
   └──────────────────────────────────────────┘
```

---

## Comparison: Old vs New System

```
┌──────────────────────────────────────────────────────────────────┐
│                    OLD SYSTEM (Timeline-Based)                   │
└──────────────────────────────────────────────────────────────────┘

Press F
  ↓
Store Start Transform (location, rotation)
Store Target Transform (location, rotation)
  ↓
Start Timeline (3 seconds duration)
  ↓
Every Frame (UpdateDockingMovement):
  • Calculate Alpha (0.0 → 1.0)
  • Lerp position: Start → Target
  • Slerp rotation: Start → Target
  • SetActorLocationAndRotation()
  ↓
After 3 seconds (OnDockingMovementComplete):
  • Call CompleteDocking()

⚠️ Problems:
  • Required UCurveFloat asset configuration
  • Complex for beginners
  • 6 extra variables to store transforms
  • Timeline component overhead
  • Harder to debug


┌──────────────────────────────────────────────────────────────────┐
│                    NEW SYSTEM (Instant Teleport)                 │
└──────────────────────────────────────────────────────────────────┘

Press F
  ↓
Get Target Transform (location, rotation)
  ↓
SetActorLocationAndRotation() ← ONE LINE!
  ↓
CompleteDocking() ← IMMEDIATELY

✅ Benefits:
  • No curve asset needed
  • Simple to understand
  • No extra variables
  • Instant feedback
  • Easy to debug
  • Perfect for MVP testing
```

---

## Debugging Checklist

```
┌─────────────────────────────────────────────────────────────┐
│                  DEBUGGING DOCKING ISSUES                   │
└─────────────────────────────────────────────────────────────┘

Issue: "Press F and nothing happens"
  
  ☐ Check Output Log (Window → Developer Tools → Output Log)
      Look for: LogAdastreaShips warnings
  
  ☐ Check input binding in editor
      Edit → Project Settings → Input
      Look for "Dock" action mapped to F
  
  ☐ Check DA_InputConfig
      Open asset, verify DockAction = IA_Dock
  
  ☐ Check distance to station
      Use "DisplayAll ASpaceship DockingRange" in console
      Use "DisplayAll DockingBayModule Location" to see where points are
  
  ☐ Verify station has DockingBayModule
      Select station actor in world
      Check Components panel for DockingBayModule
  
  ☐ Verify docking points exist
      Select DockingBayModule
      Check Details → Docking → DockingPoints array
      Should have at least 1 scene component

Common Log Messages:
  "No station in range"          → NearbyStation is nullptr
  "Too far from docking point"   → Distance > DockingRange (2000)
  "No docking slots available"   → All docking points occupied
  "Station is not docking module"→ Station lacks DockingBayModule
```

---

## Key Property Reference

| Class | Property | Type | Default | Range |
|-------|----------|------|---------|-------|
| ASpaceship | DockingRange | float | 2000.0 | 100-10000 |
| ASpaceship | bIsDocked | bool | false | - |
| ASpaceship | bIsDocking | bool | false | - |
| ADockingBayModule | MaxDockedShips | int32 | 4 | 1-20 |
| ADockingBayModule | CurrentDockedShips | int32 | 0 | - |
| ADockingBayModule | DockingPoints | Array | Empty | - |

---

**For more details, see:**
- Full Setup Guide: `docs/reference/SIMPLE_DOCKING_SETUP.md`
- Source Code: `Source/Adastrea/Public/Ships/Spaceship.h`
- Implementation: `Source/Adastrea/Private/Ships/Spaceship.cpp`
