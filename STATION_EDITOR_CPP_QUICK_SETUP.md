# Station Editor C++ - Quick Setup Guide

## 🚀 Quick Setup (5 Minutes)

### Step 1: Compile C++ Code
```bash
# Build the project in Unreal Editor
# File → Generate Visual Studio Project Files (if needed)
# Build → Compile (Ctrl+F7 or via Build button in toolbar)
```

### Step 2: Create Main Editor Widget

1. **Create WBP_StationEditor**:
   - Content Browser → `Content/UI/Station/`
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_StationEditor`
   - **IMPORTANT**: Set Parent Class to `StationEditorWidgetCpp` (C++ class)

2. **Add Required Widgets** (names MUST match exactly):
   - `ModuleListScrollBox` (Scroll Box)
   - `PowerDisplayText` (Text Block)
   - `ModuleCountDisplay` (Text Block)
   - `PowerBalanceBar` (Progress Bar)
   - `CloseButton` (Button)
   - `QueueScrollBox` (Scroll Box)

3. **Set Widget Variables**:
   - `ModuleListItemClass`: `WBP_ModuleListItem`
   - `QueueItemClass`: `WBP_ConstructionQueueItem`

### Step 3: Create Child Widgets

**WBP_ModuleListItem** (Parent: `ModuleListItemWidget`):
- Named widgets: `ModuleNameText`, `DescriptionText`, `CostText`, `BuildButton`

**WBP_ConstructionQueueItem** (Parent: `ConstructionQueueItemWidget`):
- Named widgets: `ItemNameText`, `BuildProgressBar`, `TimeRemainingText`, `CancelButton`

### Step 4: Create Module Catalog

1. Content Browser → `Content/DataAssets/Stations/`
2. Create → Miscellaneous → Data Asset
3. Class: `StationModuleCatalog`
4. Name: `DA_StationModuleCatalog`
5. Add 5 test entries (see detailed guide for values)

### Step 5: Configure Input

**Option A - Using Existing StationEditorAction**:
1. Open `Content/Input/DA_InputConfig` (if exists)
2. Find `StationEditorAction`
3. Verify it's bound to F9 key in IMC_Spaceship

**Option B - Create New Action**:
1. Create `IA_OpenStationEditor` (Digital bool, Pressed trigger)
2. Add to `IMC_Spaceship` with F9 key

### Step 6: Configure Player Controller

1. Open your player controller Blueprint (e.g., `BP_SpaceshipController`)
2. Set these properties:
   - `Station Editor Widget Class`: `WBP_StationEditor`
   - `Module Catalog`: `DA_StationModuleCatalog`
   - `Station Search Radius`: `5000.0`

3. **Bind Input** (if not using DA_InputConfig):
   - Event Graph → Add Event for `IA_OpenStationEditor` or `StationEditorAction`
   - Connect to `Toggle Station Editor` (inherited C++ function)

### Step 7: Test

1. Place a `BP_SpaceStation` in your test level
2. Play (Alt+P)
3. Get near station (within 5000 units)
4. Press **F9**
5. Editor should open with module list

---

## ✅ Widget Name Checklist

**WBP_StationEditor**:
- [ ] ModuleListScrollBox
- [ ] PowerDisplayText
- [ ] ModuleCountDisplay
- [ ] PowerBalanceBar
- [ ] CloseButton
- [ ] QueueScrollBox

**WBP_ModuleListItem**:
- [ ] ModuleNameText
- [ ] DescriptionText
- [ ] CostText
- [ ] BuildButton

**WBP_ConstructionQueueItem**:
- [ ] ItemNameText
- [ ] BuildProgressBar
- [ ] TimeRemainingText
- [ ] CancelButton

---

## ⚠️ Common Issues

### "Could not find BindWidget" Error
- Widget names are **case-sensitive**
- Must match C++ property names **exactly**
- Check Output Log for specific missing widget name

### Editor Doesn't Open
- Check `StationEditorWidgetClass` is set in controller
- Verify station exists in level
- Check station is within search radius
- Look for errors in Output Log

### Module List Empty
- Verify `ModuleCatalog` is assigned in controller
- Check DA_StationModuleCatalog has entries
- Ensure `ModuleListItemClass` is set in WBP_StationEditor

### Input Not Working
- Verify input action is bound correctly
- Check Enhanced Input Subsystem is active
- Ensure controller has input enabled

---

## 📚 Detailed Documentation

- Full guide: `STATION_EDITOR_CPP_BLUEPRINT_GUIDE.md`
- Testing guide: `Assets/StationEditorPIETestingGuide.md`
- System guide: `Assets/StationEditorSystemGuide.md`

---

## 🎯 Success Criteria

After setup:
- ✅ F9 opens editor UI
- ✅ Module list shows 5 modules
- ✅ Statistics display power/modules
- ✅ Close button works
- ✅ No compilation errors
- ✅ No BindWidget warnings

---

**Quick Setup Time**: ~5 minutes  
**Full Testing Time**: ~15 minutes  
**Version**: 1.0
