# Phase 2.1: Trading System Function Categorization

**System**: Trading  
**Current Functions**: 70  
**Target Functions**: 15-20  
**Reduction**: ~71%

---

## CargoComponent.h (14 functions)

### ✅ KEEP (Designer-facing - Essential for MVP)

**Operations (3 functions)**:
- `AddCargo()` - Essential: Player needs to buy cargo
- `RemoveCargo()` - Essential: Player needs to sell cargo  
- `ClearCargo()` - Essential: Clear all cargo (useful for testing/admin)

**Core Queries (3 functions)**:
- `GetAvailableCargoSpace()` - Essential: Check if can buy more
- `HasSpaceFor()` - Essential: Validate purchase before transaction
- `GetItemQuantity()` - Essential: Show what player has

**Total Keep**: 6 functions

### ⚠️ REVIEW (Possibly redundant or derivable)

**Convenience Queries (8 functions)**:
- `GetUsedCargoSpace()` - Derivable: Capacity - Available
- `GetCargoUtilization()` - Derivable: Used / Capacity
- `HasItem()` - Convenience wrapper around GetItemQuantity()
- `GetCargoContents()` - UI-focused: Could be private with events
- `GetUniqueItemCount()` - Analytics: CargoInventory.Num()
- `IsEmpty()` - Trivial: CargoInventory.Num() == 0
- `IsFull()` - Trivial: GetAvailableCargoSpace() <= 0

**Recommendation**: Remove 5-7 of these, keep 1-3 most useful for UI

### ❌ REMOVE (Internal/Private)

None identified - FindCargoEntryIndex() is already private

---

## PlayerTraderComponent.h (11 functions)

**Review Needed**: View file to categorize

Expected breakdown:
- **Keep**: BuyItem(), SellItem(), GetCredits(), CanAfford()
- **Review**: Financial queries, transaction history
- **Remove**: Internal calculations, debug helpers

---

## EconomyManager.h (7 functions)

**Review Needed**: View file to categorize

Expected breakdown:
- **Keep**: GetItemPrice(), ExecuteTransaction()
- **Review**: Market queries, simulation controls
- **Remove**: Internal economy calculations

---

## TradeItemDataAsset.h (9 functions)

**Review Needed**: View file to categorize

Expected breakdown:
- **Keep**: GetBasePrice(), GetVolume()
- **Review**: Legality checks, property getters
- **Remove**: Internal calculations (should be pure getters from data)

---

## MarketDataAsset.h (6 functions)

**Review Needed**: View file to categorize

Expected breakdown:
- **Keep**: GetPrice(), IsItemAvailable()
- **Review**: Inventory queries
- **Remove**: Internal market calculations

---

## TradeContractDataAsset.h (11 functions)

**MVP Relevance**: ⚠️ **LOW** - Contracts are not in MVP scope

Expected action:
- **Defer**: All 11 functions (contracts out of MVP scope per trade-simulator-mvp.instructions.md)
- Revisit post-MVP if contracts added

---

## AITraderComponent.h (8 functions)

**MVP Relevance**: ⚠️ **LOW** - AI traders not essential for MVP

Expected breakdown:
- **Keep**: 1-2 if needed for NPC traders
- **Review**: Trading behavior queries
- **Remove/Defer**: AI internals, knowledge systems

---

## TradeTransaction.h (5 functions)

**MVP Relevance**: ⚠️ **MEDIUM** - Transaction history useful but not critical

Expected breakdown:
- **Keep**: 1-2 for basic history
- **Remove**: Analytics, detailed history queries

---

## Categorization Summary

### Immediate Actions (CargoComponent)

| Function | Decision | Reason |
|----------|----------|--------|
| AddCargo | ✅ KEEP | Core trading operation |
| RemoveCargo | ✅ KEEP | Core trading operation |
| ClearCargo | ✅ KEEP | Useful utility |
| GetAvailableCargoSpace | ✅ KEEP | Essential query |
| HasSpaceFor | ✅ KEEP | Validation |
| GetItemQuantity | ✅ KEEP | Essential query |
| GetUsedCargoSpace | ❌ REMOVE | Derivable |
| GetCargoUtilization | ❌ REMOVE | Derivable |
| HasItem | ⚠️ REVIEW | Convenience wrapper |
| GetCargoContents | ⚠️ REVIEW | UI-focused |
| GetUniqueItemCount | ❌ REMOVE | Trivial |
| IsEmpty | ❌ REMOVE | Trivial |
| IsFull | ❌ REMOVE | Trivial |

**Reduction**: 14 → 6-8 functions (43-57% reduction)

### Next Steps

1. ✅ Analyze CargoComponent (DONE)
2. ⏳ Analyze PlayerTraderComponent (NEXT)
3. ⏳ Analyze EconomyManager
4. ⏳ Analyze remaining Trading files
5. ⏳ Create migration guide
6. ⏳ Implement changes incrementally

---

## Migration Guide Template

### For Removed Functions

**Before**:
```cpp
if (CargoComponent->IsEmpty())
{
    // Do something
}
```

**After**:
```cpp
if (CargoComponent->GetItemQuantity(nullptr) == 0 || 
    CargoComponent->GetAvailableCargoSpace() >= CargoComponent->CargoCapacity)
{
    // Do something
}

// OR use events:
// Bind to OnCargoSpaceChanged and track state
```

### For Changed Patterns

Document each breaking change with:
1. What was removed
2. Why it was removed
3. Alternative approach
4. Code example

---

## Testing Checklist

After each change:
- [ ] Code compiles
- [ ] Can buy cargo at station
- [ ] Can sell cargo at station
- [ ] Cargo capacity limits enforced
- [ ] UI updates correctly
- [ ] No Blueprint errors

---

**Status**: CargoComponent Analysis Complete  
**Next**: PlayerTraderComponent Analysis  
**Updated**: 2025-12-27
