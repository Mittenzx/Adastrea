# UI Widget Quick Reference

**Last Updated**: 2026-01-01  
**Purpose**: Quick reference for all UI widgets in Adastrea

---

## 🎯 Quick Answers

### Q: Why do I have two trading interface WBP?
**A**: You **had** two (now fixed). We kept `WBP_TradingInterface` and removed the duplicate `WBP_Trading_Interface`.

### Q: Should I use blueprints in content/blueprints/ui/ or content/ui/?
**A**: **Always use `Content/UI/`**. The `Content/Blueprints/UI/` directory has been removed.

### Q: Where do I create new UI widgets?
**A**: `Content/UI/[System]/` using `WBP_[System][Purpose]` naming format.

---

## 📂 Current UI Widget Directory

### All Widget Locations

| System | Widgets | Location |
|--------|---------|----------|
| **Trading** | Trading Interface | `Content/UI/Trading/` |
| **HUD** | Main HUD, Mini Map, Damage Indicator, Objective Tracker, Spaceship | `Content/UI/HUD/` |
| **Menus** | Main Menu, Pause Menu, Settings Menu, Save/Load Menu | `Content/UI/Menus/` |
| **Inventory** | Inventory, Inventory Widget | `Content/UI/Inventory/` |
| **Ship** | Ship Customization | `Content/UI/Ship/` |
| **Ships** | Ship Status | `Content/UI/Ships/` |
| **Stations** | Station Editor, Management, Module List, Construction Queue | `Content/UI/Stations/` |
| **Map** | Sector Map, Universe Map | `Content/UI/Map/` |
| **Quests** | Quest Log | `Content/UI/Quests/` |
| **Communication** | Chat Window | `Content/UI/Communication/` |
| **Dialogue** | Dialogue Box | `Content/UI/Dialogue/` |
| **Notifications** | Notification, Notification Popup | `Content/UI/Notifications/` |

---

## 📝 Widget Details

### Trading (1 widget)

**`WBP_TradingInterface`**
- **Parent Class**: `UTradingInterfaceWidget` (C++)
- **Location**: `Content/UI/Trading/WBP_TradingInterface.uasset`
- **Purpose**: Main trading interface for buy/sell at stations
- **Used in**: MVP Trade Simulator
- **Documentation**: `docs/mvp/TRADING_UI_BLUEPRINT_GUIDE.md`

### HUD (5 widgets)

**`WBP_HUD_Main`**
- **Location**: `Content/UI/HUD/WBP_HUD_Main.uasset`
- **Purpose**: Main in-game heads-up display

**`WBP_Mini_Map`**
- **Location**: `Content/UI/HUD/WBP_Mini_Map.uasset`
- **Purpose**: Mini-map navigation widget

**`WBP_Damage_Indicator`**
- **Location**: `Content/UI/HUD/WBP_Damage_Indicator.uasset`
- **Purpose**: Shows damage direction indicators

**`WBP_Objective_Tracker`**
- **Location**: `Content/UI/HUD/WBP_Objective_Tracker.uasset`
- **Purpose**: Tracks current objectives/missions

**`WBP_Spaceship`**
- **Location**: `Content/UI/HUD/WBP_Spaceship.uasset`
- **Purpose**: Spaceship status display in HUD

### Menus (4 widgets)

**`WBP_MainMenu`**
- **Location**: `Content/UI/Menus/WBP_MainMenu.uasset`
- **Purpose**: Game start main menu

**`WBP_PauseMenu`**
- **Location**: `Content/UI/Menus/WBP_PauseMenu.uasset`
- **Purpose**: In-game pause menu

**`WBP_SettingsMenu`**
- **Location**: `Content/UI/Menus/WBP_SettingsMenu.uasset`
- **Purpose**: Game settings and options

**`WBP_Save_Load_Menu`**
- **Location**: `Content/UI/Menus/WBP_Save_Load_Menu.uasset`
- **Purpose**: Save game management

### Inventory (2 widgets)

**`WBP_Inventory`**
- **Location**: `Content/UI/Inventory/WBP_Inventory.uasset`
- **Purpose**: Main inventory interface

**`WBP_InventoryWidget`**
- **Location**: `Content/UI/Inventory/WBP_InventoryWidget.uasset`
- **Purpose**: Inventory component widget (possibly item display)
- **Note**: May be variant or component of main inventory

### Ship (1 widget)

**`WBP_Ship_Customization`**
- **Location**: `Content/UI/Ship/WBP_Ship_Customization.uasset`
- **Purpose**: Ship customization interface

### Ships (1 widget)

**`WBP_ShipStatus`**
- **Location**: `Content/UI/Ships/WBP_ShipStatus.uasset`
- **Purpose**: Ship status display widget

### Stations (4 widgets)

**`WBP_StationEditor`**
- **Location**: `Content/UI/Stations/WBP_StationEditor.uasset`
- **Purpose**: Station building/editing interface

**`WBP_StationManagement`**
- **Location**: `Content/UI/Stations/WBP_StationManagement.uasset`
- **Purpose**: Station management overview

**`WBP_ModuleListItem`**
- **Location**: `Content/UI/Stations/WBP_ModuleListItem.uasset`
- **Purpose**: Individual module list entry widget

**`WBP_ConstructionQueueItem`**
- **Location**: `Content/UI/Stations/WBP_ConstructionQueueItem.uasset`
- **Purpose**: Construction queue entry widget

### Map (2 widgets)

**`WBP_SectorMap`**
- **Location**: `Content/UI/Map/WBP_SectorMap.uasset`
- **Purpose**: Local sector navigation map

**`WBP_UniverseMap`**
- **Location**: `Content/UI/Map/WBP_UniverseMap.uasset`
- **Purpose**: Galaxy-wide universe map

### Quests (1 widget)

**`WBP_QuestLog`**
- **Location**: `Content/UI/Quests/WBP_QuestLog.uasset`
- **Purpose**: Active quests and objectives log

### Communication (1 widget)

**`WBP_Chat_Window`**
- **Location**: `Content/UI/Communication/WBP_Chat_Window.uasset`
- **Purpose**: In-game chat/communication window

### Dialogue (1 widget)

**`WBP_DialogueBox`**
- **Location**: `Content/UI/Dialogue/WBP_DialogueBox.uasset`
- **Purpose**: NPC dialogue interface

### Notifications (2 widgets)

**`WBP_Notification`**
- **Location**: `Content/UI/Notifications/WBP_Notification.uasset`
- **Purpose**: Standard notification widget

**`WBP_Notification_Popup`**
- **Location**: `Content/UI/Notifications/WBP_Notification_Popup.uasset`
- **Purpose**: Popup notification variant

---

## 🎨 Naming Convention

### Format
`WBP_[System][Purpose]` or `WBP_[System]_[Purpose]`

### Examples
✅ Good:
- `WBP_TradingInterface` (no extra underscores)
- `WBP_HUD_Main` (system_purpose)
- `WBP_MainMenu` (compound word)
- `WBP_Ship_Customization` (multi-word purpose)

❌ Bad (removed):
- `BP_UI_Trading` (wrong prefix)
- `WBP_Trading_Interface` (unnecessary underscore)
- `WBP_Main_Menu` (unnecessary underscore)

### Prefixes

| Prefix | Type | Example |
|--------|------|---------|
| `WBP_` | Widget Blueprint | `WBP_TradingInterface` |
| `BP_` | Actor/Component Blueprint | `BP_Ship_Fighter` |
| `DA_` | Data Asset | `DA_Ship_Pathfinder` |
| `M_` | Material | `M_Metal_Base` |
| `T_` | Texture | `T_Ship_Hull_D` |

---

## 🚀 Creating New UI Widgets

### Step-by-Step

1. **Navigate to Content/UI/**
   - Choose appropriate subdirectory (HUD, Menus, Trading, etc.)
   - Or create new subdirectory for new system

2. **Create Widget Blueprint**
   - Right-click → User Interface → Widget Blueprint
   - Name using `WBP_[System][Purpose]` format

3. **Set Parent Class** (if needed)
   - Open Widget Blueprint
   - Reparent to C++ class (e.g., `UTradingInterfaceWidget`)

4. **Design UI**
   - Add widgets in Designer tab
   - Implement logic in Graph tab

### Example: Creating Trading Widget

```
Location: Content/UI/Trading/
Name: WBP_TradingInterface
Parent: UTradingInterfaceWidget
Purpose: Main trading interface
```

---

## 📖 Related Documentation

- **Organization Standards**: `Content/CONTENT_ORGANIZATION.md`
- **Cleanup Details**: `docs/development/UI_FILE_STRUCTURE_FINAL.md`
- **Trading UI Guide**: `docs/mvp/TRADING_UI_BLUEPRINT_GUIDE.md`
- **Code Style**: `CODE_STYLE.md`

---

## ⚠️ Removed/Deprecated Widgets

### Legacy Files (removed 2026-01-01)

These files were removed during UI cleanup:

**Duplicates with underscore variations:**
- ❌ `WBP_Trading_Interface` → use `WBP_TradingInterface`
- ❌ `WBP_Main_Menu` → use `WBP_MainMenu`
- ❌ `WBP_Pause_Menu` → use `WBP_PauseMenu`
- ❌ `WBP_Settings_Menu` → use `WBP_SettingsMenu`
- ❌ `WBP_Quest_Log` → use `WBP_QuestLog`
- ❌ `UI/Ships/WBP_ShipCustomization` → use `UI/Ship/WBP_Ship_Customization`

**Legacy BP_UI_* files:**
- ❌ `BP_UI_Trading` → use `WBP_TradingInterface`
- ❌ `BP_UI_HUD` → use `WBP_HUD_Main`
- ❌ `BP_UI_Inventory` → use `WBP_Inventory`
- ❌ `BP_UI_Quest_Log` → use `WBP_QuestLog`
- ❌ `BP_UI_Ship_Customization` → use `WBP_Ship_Customization`

**Recovery**: If needed, these can be restored from Git history (commit prior to 2026-01-01).

---

## 📊 Statistics

- **Total UI Widgets**: 25
- **Systems with UI**: 12
- **Largest System**: HUD (5 widgets)
- **Naming Compliance**: 100% (all WBP_* format)
- **Organization**: Single directory structure (Content/UI/)
- **Duplicates**: 0 (all removed)

---

**Always check this guide before creating new UI widgets!** 🎨
