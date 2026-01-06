# Docking System Visual Diagram

## Station and Module Architecture

```mermaid
graph TD
    Station[ASpaceStation<br/>TArray&lt;ASpaceStationModule*&gt; Modules]
    
    Station --> DockingBay[ADockingBayModule<br/>Large Facility<br/>Power: -50 units<br/>Capacity: 3-6 ships]
    Station --> DockingPort[ADockingPortModule<br/>Small Port<br/>Power: -10 units<br/>Capacity: 1 ship]
    Station --> Reactor[Other Modules<br/>Reactor, CargoBay, etc.]
    
    DockingBay --> BaySlots[TArray&lt;USceneComponent*&gt;<br/>DockingPoints: 6 slots]
    DockingPort --> PortSlots[TArray&lt;USceneComponent*&gt;<br/>DockingPoints: 1 slot]
    
    BaySlots --> Slot1[Slot 1: 100,0,0<br/>Status: FREE]
    BaySlots --> Slot2[Slot 2: 200,0,0<br/>Status: OCCUPIED<br/>Ship A]
    BaySlots --> Slot3[Slot 3: 300,0,0<br/>Status: FREE]
    BaySlots --> Slot4[Slot 4-6: ...<br/>Status: Various]
    
    PortSlots --> PSlot1[Slot 1: 0,0,0<br/>Status: OCCUPIED<br/>Ship C]
    
    style Station fill:#1B4F72,stroke:#fff,color:#fff
    style DockingBay fill:#2E7D32,stroke:#fff,color:#fff
    style DockingPort fill:#2E7D32,stroke:#fff,color:#fff
    style BaySlots fill:#5DADE2,stroke:#fff,color:#000
    style PortSlots fill:#5DADE2,stroke:#fff,color:#000
```

### Legend

| Component | Description |
|-----------|-------------|
| **Module Classes (C++)** | Gameplay logic, power, capacity |
| **Scene Component Pointers** | USceneComponent* docking point instances (transforms) |
| **Array** | Slot management and iteration |
| **FREE/OCCUPIED** | Runtime slot tracking |

---

## Ship Docking Workflow

```mermaid
sequenceDiagram
    participant Ship as Player Ship<br/>(BP_PlayerShip)
    participant Station as Space Station<br/>(ASpaceStation)
    participant Module as Docking Module<br/>(ADockingBayModule)
    participant Slot as Docking Slot<br/>(USceneComponent*)
    
    Ship->>Station: Request docking
    Note over Station: Find Available Module<br/>Iterate modules array<br/>Filter: DockingBay or DockingPort
    Station->>Module: FindFreeDockingSlot()
    Note over Module: Iterate DockingPoints array<br/>Check each slot for occupation
    Module-->>Station: Return Slot 3 (300, 0, 0)
    Station->>Ship: Grant docking permission<br/>Target: Slot 3
    Note over Ship: NavigationComponent<br/>Move to location (300, 0, 0)<br/>Match rotation
    Ship->>Slot: Arrive at docking point
    Ship->>Module: MarkSlotOccupied(Slot 3, Ship, true)
    Note over Module: Store ship reference<br/>Disable ship controls<br/>Open trading UI
    Module->>Ship: Docking complete
    Note over Ship: Controls disabled<br/>Trading interface open
```

---

## Docking Process Steps

```mermaid
flowchart TD
    Start([Ship Requests Docking])
    FindModule[Station: Find Available Module<br/>Iterate: Modules array<br/>Filter: DockingBay or DockingPort]
    CheckDistance{Distance<br/>acceptable?}
    FindSlot[Module: FindFreeDockingSlot<br/>Iterate: DockingPoints array]
    HasFreeSlot{Free slot<br/>found?}
    MoveShip[Ship: Navigate to Slot<br/>Get location & rotation<br/>Move to position]
    MarkOccupied[Module: MarkSlotOccupied<br/>Slot, Ship, true]
    DisableControls[Ship: Disable controls]
    OpenUI[Station: Open trading interface]
    Complete([Docking Complete])
    Reject([Docking Rejected:<br/>All bays full])
    
    Start --> FindModule
    FindModule --> CheckDistance
    CheckDistance -->|Yes| FindSlot
    CheckDistance -->|No| Reject
    FindSlot --> HasFreeSlot
    HasFreeSlot -->|Yes| MoveShip
    HasFreeSlot -->|No| Reject
    MoveShip --> MarkOccupied
    MarkOccupied --> DisableControls
    DisableControls --> OpenUI
    OpenUI --> Complete
    
    style Start fill:#C80000,stroke:#fff,color:#fff
    style Complete fill:#2E7D32,stroke:#fff,color:#fff
    style Reject fill:#D32F2F,stroke:#fff,color:#fff
```

---

## Code Flow

### 1. Station finds available docking module

```cpp
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
```

### 2. Module finds free slot

```cpp
USceneComponent* ADockingBayModule::FindFreeDockingSlot()
{
    for (USceneComponent* Point : DockingPoints)
    {
        if (!IsSlotOccupied(Point))
            return Point;
    }
    return nullptr;
}
```

### 3. Ship moves to slot

```cpp
void ASpaceship::DockAtSlot(USceneComponent* Slot)
{
    FVector TargetLoc = Slot->GetComponentLocation();
    FRotator TargetRot = Slot->GetComponentRotation();
    
    NavigationComponent->MoveToLocation(TargetLoc, TargetRot);
    
    // On arrival:
    OnDockingComplete(Slot);
}
```

### 4. Mark slot as occupied / freed

```cpp
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

---

## Design Rationale

### Why Two Module Classes?

```mermaid
graph LR
    Question[Why DockingBay AND DockingPort?]
    
    Question --> Variety[Gameplay Variety]
    Question --> Resource[Resource Management]
    Question --> Balance[Economic Balance]
    
    Variety --> V1[Different ship sizes]
    Variety --> V2[Strategic choices]
    
    Resource --> R1[Bay: 50 power, 6 slots]
    Resource --> R2[Port: 10 power, 1 slot]
    
    Balance --> B1[Expensive vs budget]
    Balance --> B2[Versatile vs specialized]
    
    style Question fill:#FF8C00,stroke:#fff,color:#fff
    style Variety fill:#2E7D32,stroke:#fff,color:#fff
    style Resource fill:#1B4F72,stroke:#fff,color:#fff
    style Balance fill:#8E44AD,stroke:#fff,color:#fff
```

**Comparison:**

| Feature | DockingBayModule | DockingPortModule |
|---------|------------------|-------------------|
| **Size** | Large | Small |
| **Power** | -50 units | -10 units |
| **Capacity** | 3-6 large ships | 1 small ship |
| **Cost** | High | Low |
| **Use Case** | Freighters, capital ships | Shuttles, fighters |

### Why Scene Component Pointers?

```mermaid
graph TD
    Choice{Why USceneComponent*?}
    
    Choice -->|Option A| SceneComp[USceneComponent Pointers]
    Choice -->|Option B| RawVectors[Raw FVector]
    
    SceneComp --> SC1[✅ Transform inheritance]
    SceneComp --> SC2[✅ Editor visualization]
    SceneComp --> SC3[✅ Collision/overlap support]
    SceneComp --> SC4[✅ Standard Unreal pattern]
    
    RawVectors --> RV1[❌ No transform inheritance]
    RawVectors --> RV2[❌ No editor visualization]
    RawVectors --> RV3[❌ Manual calculations]
    
    SceneComp --> Winner[VERDICT: Scene components]
    
    style Choice fill:#FF8C00,stroke:#fff,color:#fff
    style SceneComp fill:#2E7D32,stroke:#fff,color:#fff
    style RawVectors fill:#D32F2F,stroke:#fff,color:#fff
    style Winner fill:#FFD700,stroke:#000,color:#000
```

### Why Array Instead of Fixed Properties?

```mermaid
graph TD
    Choice{Why TArray?}
    
    Choice -->|Option A| Array[TArray&lt;USceneComponent*&gt;]
    Choice -->|Option B| Fixed[Slot1, Slot2, Slot3...]
    
    Array --> A1[✅ Scalable 1 to N]
    Array --> A2[✅ Easy iteration]
    Array --> A3[✅ Flexible per module]
    Array --> A4[✅ Runtime additions]
    
    Fixed --> F1[❌ Fixed slot count]
    Fixed --> F2[❌ Code duplication]
    Fixed --> F3[❌ Hard to iterate]
    
    Array --> Winner[VERDICT: Array is better]
    
    style Choice fill:#FF8C00,stroke:#fff,color:#fff
    style Array fill:#2E7D32,stroke:#fff,color:#fff
    style Fixed fill:#D32F2D,stroke:#fff,color:#fff
    style Winner fill:#FFD700,stroke:#000,color:#000
```

---

## Summary

### 4 Distinct Layers

```mermaid
graph TB
    subgraph Layer4[4. Runtime Tracking]
        RT[Occupied/Free State<br/>Track which slots have ships]
    end
    
    subgraph Layer3[3. Array - Slot Management]
        Array[TArray&lt;USceneComponent*&gt;<br/>Multiple slots per module<br/>Scalability & iteration]
    end
    
    subgraph Layer2[2. Scene Component Pointers]
        SCP[USceneComponent* DockingPoint<br/>Physical attachment points<br/>Transform data]
    end
    
    subgraph Layer1[1. Module Classes]
        Bay[ADockingBayModule<br/>Large, 50 power, 6 slots]
        Port[ADockingPortModule<br/>Small, 10 power, 1 slot]
    end
    
    Layer1 --> Layer2
    Layer2 --> Layer3
    Layer3 --> Layer4
    
    style Layer1 fill:#1B4F72,stroke:#fff,color:#fff
    style Layer2 fill:#2E7D32,stroke:#fff,color:#fff
    style Layer3 fill:#5DADE2,stroke:#000,color:#000
    style Layer4 fill:#8E44AD,stroke:#fff,color:#fff
```

### Single Responsibility Principle

| Layer | Purpose | Why Needed |
|-------|---------|------------|
| **Module Classes** | Gameplay logic | Distinct gameplay (power, cost, capacity) |
| **Scene Component Pointers** | Transform data | Editor tools, inheritance |
| **Array** | Slot management | Scalability, iteration, runtime tracking |
| **Runtime Tracking** | State management | Prevent double-docking, track occupancy |

---

**Key Insight**: Each layer has ONE clear purpose. Separation = Easier maintenance. Modular = Extensible design.
