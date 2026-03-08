# Material Checking System Design - MVP Phase 2

**Created**: March 8, 2026 - 11:10 GMT  
**Status**: Design Document (Stub)  
**Purpose**: Document requirements for material checking system referenced in TODO  
**Related**: `ShipUpgradeComponent.cpp` line 370 - `// TODO: [MVP-PHASE-2] Implement proper material checking when inventory system is ready`

---

## Overview

This document outlines the design for the material checking system required for ship upgrades in Adastrea's Trade Simulator MVP Phase 2.

## Current State (MVP Phase 1)

In MVP Phase 1 (Trade Simulator), material checking is stubbed:

```cpp
// MVP NOTE: Material checking is stubbed for Trade Simulator MVP
// In full implementation, this would check player/ship inventory for required materials
if (Req.RequiredMaterials.Num() > 0)
{
    UE_LOG(LogAdastrea, Verbose, TEXT("Upgrade requires materials (MVP stub): %s"), *Upgrade->GetName());
    // TODO: [MVP-PHASE-2] Implement proper material checking when inventory system is ready
    // For MVP Phase 1 (Trade Simulator), assume player has required materials
    // Related: InventorySystem component, MaterialDatabase, PlayerEconomy
}
```

## Requirements for MVP Phase 2

### Core Functionality
1. **Material Validation**: Check if player has required materials for upgrades
2. **Inventory Integration**: Connect to InventorySystem component
3. **Material Consumption**: Deduct materials when upgrades are purchased
4. **Feedback System**: Provide clear UI feedback about material requirements

### System Components

#### 1. MaterialDatabase
- Central registry of all materials in the game
- Material definitions (ID, name, description, icon, value)
- Material categories (Common, Rare, Exotic, etc.)

#### 2. InventorySystem Component
- Tracks player/ship inventory contents
- Provides API for checking material quantities
- Handles material addition/removal

#### 3. PlayerEconomy System
- Manages player credits and resources
- Integrates with trading system
- Provides economic context for material values

#### 4. UpgradeMaterialValidator
- Validates material requirements for upgrades
- Provides detailed failure reasons
- Integrates with UI for player feedback

### Data Structures

```cpp
// Material definition
struct FMaterialDefinition
{
    FName MaterialID;
    FText DisplayName;
    FText Description;
    UTexture2D* Icon;
    float BaseValue;
    EMaterialRarity Rarity;
    // ... other properties
};

// Material requirement for upgrades
struct FMaterialRequirement
{
    FName MaterialID;
    int32 RequiredQuantity;
    bool bConsumeOnPurchase;
};

// Inventory entry
struct FInventoryMaterialEntry
{
    FName MaterialID;
    int32 CurrentQuantity;
    int32 MaximumCapacity;
};
```

### API Design

```cpp
class UMaterialCheckingSystem : public UObject
{
public:
    // Check if player has required materials
    bool HasRequiredMaterials(const TArray<FMaterialRequirement>& Requirements);
    
    // Get detailed validation results
    FMaterialValidationResult ValidateMaterials(const TArray<FMaterialRequirement>& Requirements);
    
    // Consume materials (if validation passes)
    bool ConsumeMaterials(const TArray<FMaterialRequirement>& Requirements);
    
    // Get material information
    FMaterialDefinition* GetMaterialDefinition(FName MaterialID);
};
```

### Integration Points

1. **ShipUpgradeComponent** - Primary consumer of material checking
2. **TradingSystem** - Material buying/selling
3. **CraftingSystem** - Future system for material combination
4. **QuestSystem** - Material collection objectives
5. **UI System** - Display material requirements and inventory

### UI/UX Requirements

1. **Material Requirements Display**
   - Show required materials in upgrade UI
   - Color-code based on availability (green=has, red=missing)
   - Show quantities (e.g., "3/5 Titanium")

2. **Inventory Display**
   - Grid/list view of player materials
   - Filtering by category/rarity
   - Quick actions (use, sell, transfer)

3. **Feedback Messages**
   - Clear error messages for missing materials
   - Success confirmation when materials are consumed
   - Visual feedback (animations, sound effects)

### Implementation Phases

#### Phase 2.1: Basic Validation (MVP Phase 2)
- Implement MaterialDatabase with hardcoded materials
- Create basic InventorySystem tracking
- Integrate with ShipUpgradeComponent
- Simple UI feedback

#### Phase 2.2: Enhanced Features
- Material buying/selling in markets
- Inventory management UI
- Material storage in stations/ships
- Advanced filtering and search

#### Phase 2.3: Advanced Systems
- Material crafting/refining
- Rare material acquisition
- Economic simulation (supply/demand)
- Player-to-player trading

### Testing Strategy

1. **Unit Tests**
   - Material validation logic
   - Inventory management
   - Edge cases (zero quantities, invalid materials)

2. **Integration Tests**
   - Upgrade system integration
   - UI feedback system
   - Save/load functionality

3. **User Testing**
   - Player understanding of material requirements
   - UI clarity and usability
   - Transaction flow satisfaction

### Dependencies

1. **InventorySystem** - Must be implemented first
2. **MaterialDatabase** - Data asset definitions
3. **UI Framework** - Material display components
4. **Economy System** - Material valuation

### Risks and Mitigations

1. **Risk**: Over-engineering material system
   - **Mitigation**: Start with minimal viable implementation
   - Focus on core validation only

2. **Risk**: Performance with large material lists
   - **Mitigation**: Implement efficient data structures
   - Use caching for frequently accessed materials

3. **Risk**: Player confusion about material requirements
   - **Mitigation**: Clear UI with tooltips
   - Tutorial system for new players

### Success Metrics

1. **Technical**
   - 100% unit test coverage for validation logic
   - <10ms material check performance
   - Zero memory leaks in material tracking

2. **User Experience**
   - >90% player understanding of material requirements
   - <2 clicks to check material availability
   - Positive feedback on material transaction flow

3. **Gameplay**
   - Material system enhances upgrade progression
   - Balanced material availability vs. requirements
   - Engaging material acquisition loop

---

## Next Steps

1. **Create MaterialDatabase** - Define core materials for MVP
2. **Implement InventorySystem** - Basic material tracking
3. **Update ShipUpgradeComponent** - Integrate material validation
4. **Create UI Components** - Display material requirements
5. **Add Tests** - Ensure system reliability

## References

1. [Inventory System Research](INVENTORY_SYSTEM_RESEARCH.md)
2. [ShipUpgradeComponent.cpp](../Source/Adastrea/Private/Ships/ShipUpgradeComponent.cpp) - Line 370 TODO
3. [TODO Tracking](../TODO_TRACKING_2026-03-08.md) - Material checking priority

---

*This document will be updated as the material checking system is designed and implemented during MVP Phase 2.*