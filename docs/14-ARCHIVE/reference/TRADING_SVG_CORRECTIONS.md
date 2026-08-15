# Trading System SVG Corrections

**Date**: December 31, 2025
**Issue**: Incorrect event/function names in Blueprint SVG files and documentation

---

## Summary of Issues

The Blueprint SVG documentation files referenced non-existent events and used incorrect naming. This document outlines the corrections needed to align with the actual C++ implementation in `UTradingInterfaceWidget`.

---

## Actual C++ Implementation

### BlueprintCallable Functions (Call these from Blueprints)

```cpp
// Market Management
bool OpenMarket(UMarketDataAsset* Market);
void CloseMarket();

// Information Queries
TArray<FMarketInventoryEntry> GetAvailableItems() const;
TArray<FMarketInventoryEntry> GetFilteredItems(ETradeItemCategory Category) const;
int32 GetItemPrice(UTradeItemDataAsset* Item, int32 Quantity) const;
int32 GetPlayerCredits() const;
void GetCargoSpaceInfo(float& OutUsedSpace, float& OutTotalSpace, float& OutAvailableSpace) const;
int32 GetPlayerProfit() const;

// Shopping Cart
bool AddToCart(UTradeItemDataAsset* Item, int32 Quantity);
void RemoveFromCart(UTradeItemDataAsset* Item);
void UpdateCartQuantity(UTradeItemDataAsset* Item, int32 NewQuantity);
void ClearCart();
int32 GetCartTotal() const;
int32 GetCartItemCount() const;
bool IsCartEmpty() const;

// Transaction Execution
bool ExecuteTrade();

// UI Controls
void SetCategoryFilter(ETradeItemCategory Category);
void SetSortMode(const FString& Mode);
void ToggleBuySellView();
void SetSelectedItem(UTradeItemDataAsset* Item);

// Validation
bool CanAffordCart() const;
bool HasCargoSpaceForCart() const;
bool ValidateTransaction(FText& OutErrorMessage) const;
```

### BlueprintImplementableEvents (Implement these in Blueprints to respond to C++)

```cpp
void OnMarketInventoryUpdated();
void OnCartUpdated();
void OnTradeCompleted(bool bSuccess, const FText& ErrorMessage);
void OnCreditsUpdated(int32 NewCredits);
void OnCargoSpaceUpdated(float UsedSpace, float TotalSpace);
```

---

## File-by-File Corrections

### 1. `trading_ui_open.svg`

**❌ INCORRECT:**
- Shows `EVENT: On Market Opened` (does not exist)

**✅ CORRECT:**
```
1. FUNCTION: OpenMarket(Market Data)
   → Call this from Blueprint to open the trading UI

2. EVENT: OnMarketInventoryUpdated
   → BlueprintImplementableEvent triggered by OpenMarket C++ function

3. FUNCTION: GetAvailableItems()
   → Returns TArray<FMarketInventoryEntry>

4. FLOW CONTROL: ForEach Loop
   → Iterate through inventory entries

5. Create WBP_TradeItemRow widgets and add to ScrollBox
```

**Workflow:**
1. Player docks at station → Call `OpenMarket(MarketDataAsset)`
2. C++ processes the open, stores reference, then broadcasts `OnMarketInventoryUpdated`
3. Blueprint implements `OnMarketInventoryUpdated` event
4. Blueprint calls `GetAvailableItems()` to get inventory
5. Blueprint loops through items and creates UI widgets

**Status**: ✅ CORRECTED - See `trading_ui_open_CORRECTED.svg`

---

### 2. `market_item_display.svg`

**❌ INCORRECT:**
- Shows `EVENT: Open Market`

**✅ CORRECT:**
Same as #1 - Should use `FUNCTION: OpenMarket()` followed by `EVENT: OnMarketInventoryUpdated`

**Status**: ⚠️ NEEDS CORRECTION

---

### 3. `buy_validation_flow.svg`

**Current Content:**
- `EVENT: On Buy Button Clicked`
- `FUNCTION: Calculate Total Cost`
- `BRANCH: Can Afford?`
- `BRANCH: Has Cargo Space?`

**Analysis:**
- "On Buy Button Clicked" is a UI button event, not from C++
- The validation workflow is conceptually correct
- Should clarify this is a custom Blueprint event, not from C++

**✅ RECOMMENDATION:**
```
1. Custom EVENT: On Buy Button Clicked (from UMG Button)
2. FUNCTION: GetItemPrice(Item, Quantity)
   → Returns total price
3. BRANCH: Can Afford?
   → Compare with GetPlayerCredits()
4. BRANCH: Has Cargo Space?
   → Check GetCargoSpaceInfo()
5. FUNCTION: AddToCart(Item, Quantity)
   → If all validations pass
```

**Status**: ⚠️ NEEDS CLARIFICATION (add "Custom Event" label)

---

### 4. `add_to_cart_flow.svg`

**Current Content:**
- `EVENT: Add to Cart`

**Analysis:**
- `AddToCart()` is a BlueprintCallable function, not an event
- Should show function call, not event

**✅ CORRECT:**
```
1. Custom EVENT: On Add To Cart Button Clicked (UMG)
2. Validate inputs (stock, credits, cargo)
3. FUNCTION: AddToCart(Item, Quantity)
   → C++ function that adds to shopping cart
4. EVENT: OnCartUpdated
   → BlueprintImplementableEvent triggered by AddToCart
5. Update UI to show cart contents
```

**Status**: ✅ CORRECTED (subsequent commit after initial PR)

---

### 5. `execute_trade_flow.svg`

**Current Content:**
- `EVENT: Complete Trade Clicked`
- `FUNCTION: Validate Transaction`
- `BRANCH: Transaction Valid?`
- `FUNCTION: Execute Trade`

**Analysis:**
- Mostly correct conceptually
- Should clarify event is from UMG button
- `ExecuteTrade()` is correct function name

**✅ RECOMMENDATION:**
```
1. Custom EVENT: On Complete Trade Button Clicked (UMG)
2. FUNCTION: ValidateTransaction(Out Error Message)
   → Returns bool + error text
3. BRANCH: Is Valid?
4. True Path: FUNCTION: ExecuteTrade()
   → C++ executes transaction
5. False Path: Show error message
6. EVENT: OnTradeCompleted(Success, Error)
   → Response event from C++
```

**Status**: ⚠️ NEEDS MINOR CLARIFICATION

---

### 6. `buy_sell_toggle.svg`

**Current Content:**
- `EVENT: Buy Button Clicked`
- `EVENT: Sell Button Clicked`

**Analysis:**
- These are UMG button events, correct
- Both should call `ToggleBuySellView()`

**✅ CORRECT:**
```
1. Custom EVENT: On Buy Button Clicked (UMG)
   → FUNCTION: ToggleBuySellView()

2. Custom EVENT: On Sell Button Clicked (UMG)
   → FUNCTION: ToggleBuySellView()
```

**Status**: ✅ MOSTLY CORRECT (clarify "Custom Event")

---

## Key Concepts to Clarify in Documentation

### 1. Three Types of Events in Blueprint Workflows

**A. Custom Blueprint Events (UMG Widget Events)**
- Example: `On Button Clicked`, `On Slider Value Changed`
- Source: UMG Designer UI
- These are NOT in C++ code
- Should be labeled: `Custom EVENT: On Button Clicked`

**B. BlueprintImplementableEvents (C++ → Blueprint)**
- Example: `OnMarketInventoryUpdated`, `OnCartUpdated`
- Source: C++ code (`BlueprintImplementableEvent`)
- Blueprint implements these to respond to C++ events
- Should be labeled: `EVENT: OnMarketInventoryUpdated`

**C. BlueprintCallable Functions (Blueprint → C++)**
- Example: `OpenMarket()`, `AddToCart()`, `ExecuteTrade()`
- Source: C++ code (`BlueprintCallable`)
- Blueprint calls these to invoke C++ logic
- Should be labeled: `FUNCTION: OpenMarket`

### 2. Typical Workflow Pattern

```
UMG Button (Custom Event)
  ↓
Call C++ Function (BlueprintCallable)
  ↓
C++ processes logic
  ↓
C++ broadcasts event (BlueprintImplementableEvent)
  ↓
Blueprint responds to event
  ↓
Update UI
```

**Example: Opening Trading UI**
```
Player docks (Custom Event)
  ↓
OpenMarket(MarketData) [BlueprintCallable Function]
  ↓
C++ initializes market, loads inventory
  ↓
OnMarketInventoryUpdated [BlueprintImplementableEvent]
  ↓
Blueprint gets inventory: GetAvailableItems()
  ↓
Blueprint populates UI widgets
```

---

## Action Items

### High Priority

- [x] ✅ Create corrected `trading_ui_open_CORRECTED.svg` (commit 66e8616)
- [x] ⚠️ Update `market_item_display.svg` to match corrected workflow (commit ab6d6fa)
- [x] ⚠️ Update `add_to_cart_flow.svg` to show function calls correctly (subsequent commit)
- [ ] ⚠️ Add "Custom Event" labels to UMG button events in all SVGs

### Medium Priority

- [ ] 📝 Update `WBP_TradingUI.md` to reference corrected SVGs
- [ ] 📝 Update `TradingSystemBlueprintGuide.md` with correct workflows
- [ ] 📝 Add explanation of three event types to guides
- [ ] 📝 Create a "Common Mistakes" section in documentation

### Low Priority (Nice to Have)

- [ ] 🎨 Regenerate all SVGs using Python tool for consistency
- [ ] 📚 Add color-coding: UMG events (purple), C++ events (red), C++ functions (blue)
- [ ] 📖 Create a visual legend showing event types
- [ ] 🔗 Cross-reference all Blueprint guides with C++ headers

---

## Testing Checklist

After corrections are made:

- [ ] All SVG files render correctly in GitHub
- [ ] All event names match C++ headers exactly
- [ ] All function names match C++ headers exactly
- [ ] Documentation workflow matches actual implementation
- [ ] Custom events are clearly labeled as "Custom Event" or "UMG Event"
- [ ] BlueprintImplementableEvents are labeled as "EVENT:"
- [ ] BlueprintCallable functions are labeled as "FUNCTION:"

---

## Reference Files

**C++ Source:**
- `Source/Adastrea/Public/UI/TradingInterfaceWidget.h` - Primary trading widget class

**Documentation:**
- `docs/BP/WBP_TradingUI.md` - Trading UI implementation guide
- `Blueprints/TradingSystemBlueprintGuide.md` - General trading Blueprint guide
- `docs/reference/BLUEPRINT_IMAGE_GUIDE.md` - SVG creation standards

**SVG Files:**
- `docs/reference/images/blueprints/trading_ui_open.svg` (❌ NEEDS FIX)
- `docs/reference/images/blueprints/market_item_display.svg` (❌ NEEDS FIX)
- `docs/reference/images/blueprints/buy_validation_flow.svg` (⚠️ CLARIFY)
- `docs/reference/images/blueprints/add_to_cart_flow.svg` (❌ NEEDS FIX)
- `docs/reference/images/blueprints/execute_trade_flow.svg` (⚠️ CLARIFY)
- `docs/reference/images/blueprints/buy_sell_toggle.svg` (✅ MOSTLY OK)

---

**Next Steps:**
1. Replace old SVG files with corrected versions
2. Update all documentation referencing these workflows
3. Add clarification about event types
4. Test in actual Blueprint implementation

---

**Last Updated**: 2025-12-31
**Status**: In Progress
**Reviewed By**: Copilot Agent
