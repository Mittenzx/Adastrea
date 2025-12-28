# Phase 2: Blueprint API Migration Guide

**Version**: 1.0  
**Date**: 2025-12-28  
**Status**: In Progress  
**Scope**: Trading System Changes

---

## Overview

This guide documents all Blueprint API changes made during Phase 2 of the UE5 Practices Implementation. It provides migration paths for affected Blueprints and alternative approaches for removed functionality.

**Purpose**:
- Help content team adapt to API changes
- Provide clear before/after examples
- Document rationale for changes
- Minimize disruption to workflow

---

## Trading System Changes

### Summary

**Total Functions Before**: 70  
**Total Functions After**: 34-38  
**Reduction**: 46-51% (32-36 functions)

**Categories**:
- ✅ **Keep (No Changes)**: 18-25 functions (perfectly scoped)
- ⚠️ **Review/Cleanup**: 13-16 functions (convenience bloat)
- ❌ **Remove**: 6 functions (redundant)
- ⏸️ **Defer**: 20-22 functions (post-MVP features)

---

## CargoComponent Changes

**File**: `Source/Adastrea/Trading/CargoComponent.h`  
**Before**: 13 BlueprintCallable functions  
**After**: 6-8 BlueprintCallable functions  
**Reduction**: 46-54%

### Functions Kept (No Changes Needed)

These functions remain BlueprintCallable and work exactly as before:

#### ✅ AddCargo(ItemID, Quantity)
- **Status**: No changes
- **Purpose**: Add cargo to inventory
- **Usage**: Same as before

#### ✅ RemoveCargo(ItemID, Quantity)
- **Status**: No changes
- **Purpose**: Remove cargo from inventory
- **Usage**: Same as before

#### ✅ ClearCargo()
- **Status**: No changes
- **Purpose**: Clear all cargo (testing/admin)
- **Usage**: Same as before

#### ✅ GetAvailableCargoSpace()
- **Status**: No changes
- **Purpose**: Get remaining cargo capacity
- **Usage**: Same as before

#### ✅ HasSpaceFor(ItemID, Quantity)
- **Status**: No changes
- **Purpose**: Check if can fit cargo
- **Usage**: Same as before

#### ✅ GetItemQuantity(ItemID)
- **Status**: No changes
- **Purpose**: Get quantity of specific item
- **Usage**: Same as before

---

### Functions Recommended for Removal

These convenience functions add Blueprint bloat and can be replaced with simple logic:

#### ❌ GetUsedCargoSpace() → REMOVE

**Before** (Blueprint):
```
[CargoComponent] → [GetUsedCargoSpace] → [Print]
```

**After** (Blueprint - Simple Calculation):
```
[CargoComponent] → [CargoCapacity] → [Save As Capacity]
[CargoComponent] → [GetAvailableCargoSpace] → [Save As Available]
[Capacity] - [Available] → [Print]
```

**Rationale**: This is just `CargoCapacity - GetAvailableCargoSpace()`. No need for dedicated function.

---

#### ❌ GetCargoUtilization() → REMOVE

**Before** (Blueprint):
```
[CargoComponent] → [GetCargoUtilization] → [× 100] → [Print "X% Full"]
```

**After** (Blueprint - Calculate Percentage):
```
[CargoComponent] → [CargoCapacity] → [Save As Capacity]
[CargoComponent] → [GetAvailableCargoSpace] → [Save As Available]
[Available] ÷ [Capacity] → [1 - Result] → [× 100] → [Print "X% Full"]
```

**Rationale**: Simple calculation. Doesn't warrant dedicated function.

---

#### ❌ HasItem(ItemID) → REMOVE

**Before** (Blueprint):
```
[CargoComponent] → [HasItem ItemID] → [Branch]
```

**After** (Blueprint - Check Quantity):
```
[CargoComponent] → [GetItemQuantity ItemID] → [> 0] → [Branch]
```

**Rationale**: Just checks if `GetItemQuantity() > 0`. Redundant wrapper.

---

#### ❌ IsEmpty() → REMOVE

**Before** (Blueprint):
```
[CargoComponent] → [IsEmpty] → [Branch]
```

**After** (Blueprint - Check Capacity):
```
[CargoComponent] → [GetAvailableCargoSpace] → [==] [CargoCapacity] → [Branch]
```

**Or simpler**:
```
[CargoComponent] → [GetItemQuantity] (any item) → [== 0] → [Branch]
```

**Rationale**: Trivial check. No dedicated function needed.

---

#### ❌ IsFull() → REMOVE

**Before** (Blueprint):
```
[CargoComponent] → [IsFull] → [Branch]
```

**After** (Blueprint - Check Available):
```
[CargoComponent] → [GetAvailableCargoSpace] → [≤ 0] → [Branch]
```

**Rationale**: Just checks if `GetAvailableCargoSpace() <= 0`. Redundant.

---

#### ❌ GetUniqueItemCount() → REMOVE

**Before** (Blueprint):
```
[CargoComponent] → [GetUniqueItemCount] → [Print "X types"]
```

**After** (Blueprint - Count Contents):
```
[CargoComponent] → [GetCargoContents] → [Length] → [Print "X types"]
```

**Rationale**: Just returns `CargoInventory.Num()`. Use GetCargoContents().Length instead.

---

#### ⚠️ GetCargoContents() → CONSIDER KEEPING

**Status**: Review for removal or make private with events

**Current Use**:
```
[CargoComponent] → [GetCargoContents] → [ForEachLoop] → [Display in UI]
```

**Alternative** (Event-driven):
```cpp
// C++ side: Broadcast events when cargo changes
UPROPERTY(BlueprintAssignable)
FOnCargoChanged OnCargoChanged;

// Blueprint listens for events instead of polling
```

**Recommendation**: 
- If UI needs live updates: Keep and use events
- If only for debugging: Make private, use debug tools

---

## PlayerTraderComponent Changes

**File**: `Source/Adastrea/Trading/PlayerTraderComponent.h`  
**Before**: 11 BlueprintCallable functions  
**After**: 11 BlueprintCallable functions  
**Reduction**: 0% (perfectly scoped)

### ✅ No Changes Required

All 11 functions in PlayerTraderComponent are MVP-critical and well-designed:

- `BuyItem()` - CRITICAL for trading gameplay
- `SellItem()` - CRITICAL for trading gameplay
- `CanAfford()` - Essential validation
- `GetBuyCost()` - Essential UI display
- `GetSellValue()` - Essential UI display
- `GetCredits()` - Essential UI display
- `GetProfit()` - Essential progression tracking
- `AddCredits()` - Extensibility for rewards
- `RemoveCredits()` - Extensibility for expenses
- `GetProfitPercentage()` - UI polish
- `ResetFinances()` - Testing/debugging

**Recommendation**: 🎉 **NO CHANGES NEEDED** - This component is exemplary.

---

## EconomyManager Changes

**File**: `Source/Adastrea/Trading/EconomyManager.h`  
**Before**: 7 BlueprintCallable functions  
**After**: 7 BlueprintCallable functions  
**Reduction**: 0% (well-scoped)

### ✅ No Changes Required

All 7 functions are essential for MVP economy simulation:

- `GetItemPrice()` - CRITICAL for pricing
- `RecordTransaction()` - CRITICAL for supply/demand
- `RegisterMarket()` - Essential setup
- `UnregisterMarket()` - Clean design completeness
- `IsMarketRegistered()` - Validation
- `GetActiveMarkets()` - Debugging/future UI
- `UpdateEconomy()` - Testing/balancing

**Recommendation**: 🎉 **NO CHANGES NEEDED** - Clean subsystem design.

---

## TradeItemDataAsset Changes

**File**: `Source/Adastrea/Trading/TradeItemDataAsset.h`  
**Before**: 9 BlueprintCallable functions  
**After**: 2-3 BlueprintCallable functions  
**Reduction**: 67-78%

### Functions Kept

#### ✅ GetDisplayName()
- **Status**: Keep
- **Purpose**: Get localized item name
- **Usage**: Same as before

#### ✅ GetBasePrice()
- **Status**: Keep
- **Purpose**: Get base market price
- **Usage**: Same as before

#### ⚠️ GetVolumePerUnit()
- **Status**: Review - could be direct property access
- **Purpose**: Get cargo space per unit
- **Consider**: Just access `VolumePerUnit` property directly

---

### Functions Removed (Logic → Data Pattern)

#### ❌ CalculateBulkDiscount() → REMOVE

**Problem**: Data Assets should not contain complex logic

**Before** (Data Asset has logic):
```cpp
// TradeItemDataAsset.h
float CalculateBulkDiscount(int32 Quantity) const;
```

**After** (Logic moved to EconomyManager):
```cpp
// EconomyManager.h
float CalculateItemPrice(UTradeItemDataAsset* Item, int32 Quantity, bool IsBuying);
```

**Blueprint Migration**:
```
Before: [TradeItem] → [CalculateBulkDiscount] → [Price]
After:  [EconomyManager] → [CalculateItemPrice ItemData Quantity IsBuying] → [Price]
```

**Rationale**: Pricing logic belongs in EconomyManager, not in data assets.

---

#### ❌ CalculatePriceWithReputation() → REMOVE

**Before**:
```cpp
float CalculatePriceWithReputation(int32 FactionReputation) const;
```

**After** (EconomyManager handles this):
```cpp
// EconomyManager already considers faction relationships in GetItemPrice()
float Price = EconomyManager->GetItemPrice(ItemData, StationID);
```

**Blueprint Migration**:
```
Before: [TradeItem] → [CalculatePriceWithReputation] → [Price]
After:  [EconomyManager] → [GetItemPrice ItemData StationID] → [Price]
```

**Rationale**: Economy manager has full context (reputation, supply/demand, etc).

---

#### ❌ IsLegalAt(StationID) → REMOVE (Post-MVP)

**Status**: Deferred to post-MVP (legality system out of scope)

**Before**:
```cpp
bool IsLegalAt(FName StationID) const;
```

**After** (Post-MVP):
```cpp
// For MVP: All items are legal everywhere
bool bIsLegal = true;

// TODO: Implement LegalityManager in post-MVP phase
```

**Blueprint Migration**:
```
Before: [TradeItem] → [IsLegalAt StationID] → [Branch]
After:  [TRUE] → [Branch]  // All items legal in MVP
```

**Rationale**: Legality/contraband system deferred until MVP validated.

---

#### ❌ RequiresPermit(FactionID) → REMOVE (Post-MVP)

**Status**: Deferred to post-MVP (permit system out of scope)

**Before**:
```cpp
bool RequiresPermit(FName FactionID) const;
```

**After** (Post-MVP):
```cpp
// For MVP: No permits required
bool bRequiresPermit = false;

// TODO: Implement PermitSystem in post-MVP phase
```

**Blueprint Migration**:
```
Before: [TradeItem] → [RequiresPermit FactionID] → [Branch]
After:  [FALSE] → [Branch]  // No permits in MVP
```

---

#### ❌ CalculateContrabandFine(Quantity) → REMOVE (Post-MVP)

**Status**: Deferred to post-MVP

**Before**:
```cpp
float CalculateContrabandFine(int32 Quantity) const;
```

**After** (Post-MVP):
```cpp
// For MVP: No contraband system
float Fine = 0.0f;

// TODO: Implement ContrabandSystem in post-MVP phase
```

---

#### ❌ IsHighValue() → REMOVE

**Before**:
```cpp
bool IsHighValue() const;
```

**After** (Simple comparison):
```cpp
const float LuxuryThreshold = 1000.0f;
bool bIsHighValue = TradeItem->BasePrice >= LuxuryThreshold;
```

**Blueprint Migration**:
```
Before: [TradeItem] → [IsHighValue] → [Branch]
After:  [TradeItem] → [BasePrice] → [≥ 1000] → [Branch]
```

**Rationale**: Simple threshold check, no function needed.

---

## MarketDataAsset Changes

**File**: `Source/Adastrea/Trading/MarketDataAsset.h`  
**Before**: 6 BlueprintCallable functions  
**After**: 6 BlueprintCallable functions  
**Reduction**: 0% (clean design)

### ✅ No Changes Required

All 6 functions are essential and well-designed:

- `GetItemPrice()` - CRITICAL for pricing
- `UpdateStock()` - CRITICAL for supply/demand
- `GetCurrentStock()` - Essential queries
- `CanSellToMarket()` - Essential validation
- `CanBuyFromMarket()` - Essential validation
- `GetPriceMultiplier()` - Essential calculations

**Recommendation**: 🎉 **NO CHANGES NEEDED** - Excellent Data Asset design.

---

## Deferred Post-MVP Systems

These entire systems are deferred until after MVP validation:

### TradeContractDataAsset (11 functions) - ⏸️ DEFERRED

**Status**: Out of MVP scope  
**Reason**: Contract/quest system not needed for core trading loop

**All functions deferred**:
- Quest generation
- Contract validation
- Reward calculations
- Deadline tracking
- Reputation impacts

**Post-MVP Reactivation**: Can be reactivated after MVP proves market interest.

---

### AITraderComponent (6-7 advanced functions) - ⏸️ DEFERRED

**Status**: Basic AI kept (1-2 functions), advanced AI deferred

**Kept for MVP**:
- Basic AI trading simulation (background economy)

**Deferred for Post-MVP**:
- Advanced decision-making
- Route optimization
- Competitive AI traders
- Fleet management

---

### TradeTransaction (3-4 analytics functions) - ⏸️ DEFERRED

**Status**: Basic tracking kept (1-2 functions), analytics deferred

**Kept for MVP**:
- Basic transaction recording

**Deferred for Post-MVP**:
- Transaction history UI
- Profit/loss analytics
- Trading statistics dashboard

---

## Implementation Timeline

### Week 1: Migration Preparation
- [ ] Review this migration guide with content team
- [ ] Identify affected Blueprints (if any exist)
- [ ] Create backup branch
- [ ] Test current trading loop functionality

### Week 2: CargoComponent Changes
- [ ] Remove 5-7 convenience functions
- [ ] Update any affected Blueprints
- [ ] Test trading loop still works
- [ ] Document any issues

### Week 3: TradeItemDataAsset Changes
- [ ] Remove pricing logic functions
- [ ] Ensure EconomyManager handles all pricing
- [ ] Update Blueprints to use EconomyManager
- [ ] Test pricing still accurate

### Week 4: Deferred Systems
- [ ] Comment out TradeContractDataAsset functions
- [ ] Reduce AITraderComponent exposure
- [ ] Reduce TradeTransaction exposure
- [ ] Verify MVP gameplay unaffected

---

## Testing Checklist

After each change:

### Functional Testing
- [ ] Can buy cargo at station
- [ ] Can sell cargo at station
- [ ] Prices update correctly
- [ ] Cargo space tracked correctly
- [ ] Credits update correctly
- [ ] Supply/demand simulation works

### Blueprint Testing
- [ ] All trading Blueprints compile
- [ ] No broken node references
- [ ] UI displays correctly
- [ ] Events fire properly

### Performance Testing
- [ ] No performance regression
- [ ] GC behavior unchanged
- [ ] Frame rate stable

---

## Rollback Plan

If issues arise:

1. **Individual Function Rollback**:
   - Restore specific function
   - Document why rollback needed
   - Investigate alternative approach

2. **Component Rollback**:
   - Revert entire component changes
   - Restore from feature branch
   - Re-analyze function necessity

3. **Full Phase Rollback**:
   - Revert to Phase 1 complete state
   - Re-evaluate Phase 2 strategy
   - Adjust timeline and scope

---

## Communication Plan

### Before Changes
- [ ] Present migration guide to content team
- [ ] Identify any concerns or blockers
- [ ] Adjust timeline if needed

### During Changes
- [ ] Daily standup updates
- [ ] Immediate notification of breaking changes
- [ ] Shared testing sessions

### After Changes
- [ ] Retrospective meeting
- [ ] Document lessons learned
- [ ] Update best practices guide

---

## Contact and Support

**Questions about this migration?**
- Check this guide first
- Review PHASE2_TRADING_SYSTEM_CATEGORIZATION.md for detailed rationale
- Consult UE5_PRACTICES_IMPLEMENTATION_CHECKLIST.md for overall context

**Found an issue?**
- Document the problem
- Note which function/component
- Describe expected vs actual behavior
- Contact development team immediately

---

## Appendix: Common Patterns

### Pattern 1: Replace Convenience Getter with Direct Access

**Before**:
```
[Component] → [GetConvenienceValue] → [Use Value]
```

**After**:
```
[Component] → [PropertyName] → [Use Value]
```

---

### Pattern 2: Replace Data Asset Logic with Subsystem Logic

**Before**:
```
[DataAsset] → [CalculateSomething] → [Result]
```

**After**:
```
[Subsystem] → [CalculateSomething DataAsset Context] → [Result]
```

---

### Pattern 3: Replace Boolean Function with Comparison

**Before**:
```
[Component] → [IsSomething] → [Branch]
```

**After**:
```
[Component] → [GetValue] → [Compare] → [Branch]
```

---

**Last Updated**: 2025-12-28  
**Version**: 1.0  
**Status**: Trading System Complete, Ready for Implementation  
**Next**: Ships and Stations system analysis
