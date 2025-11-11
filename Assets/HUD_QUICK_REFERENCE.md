# HUD System Quick Reference

> **Quick reference for using the HUD, Inventory, and Ship Customization systems**

---

## HUD System

### Creating a HUD Widget

```
1. Create Widget Blueprint → AdastreaHUDWidget
2. Design UI with health bar, shield bar, speed display, etc.
3. Bind to HealthPercent, ShieldPercent, CurrentSpeedValue
4. Add to Player Controller on BeginPlay
```

### Updating HUD

```blueprint
Get HUD Widget → Update Health (Current, Max)
Get HUD Widget → Update Shield (Current, Max)
Get HUD Widget → Update Speed (Current, Max)
Get HUD Widget → Show Alert (Message, Duration, IsWarning)
```

---

## Inventory System

### Item Data Asset

**Create**: Right-click → Data Asset → InventoryItemDataAsset

**Properties**:
- ItemName, Description, Icon
- Category: Weapon, Resource, Consumable, etc.
- Rarity: Common → Unique
- MaxStackSize, Weight, BaseValue

### Inventory Component

**Add to**: Player Controller or Spaceship

**Settings**:
- MaxSlots: 20-100
- MaxWeight: 0 (unlimited) or specific value
- bAutoStack: true

### Common Operations

```blueprint
// Add Item
Add Item (Item Asset, Quantity) → Returns items added

// Remove Item
Remove Item (Item Asset, Quantity) → Returns items removed

// Check Item
Has Item (Item Asset, Quantity) → Returns true/false

// Get Count
Get Item Count (Item Asset) → Returns total quantity

// Sort
Sort Inventory

// Use
Use Item (Slot Index) → Returns success
```

### Inventory Widget

```blueprint
// Initialize
Initialize Inventory (Inventory Component)

// Refresh
Refresh Inventory

// Filter
Filter By Category (Category)
Clear Filters

// Events
On Slot Selected (Slot Index)
On Use Item (Slot Index)
On Drop Item (Slot Index, Quantity)
```

---

## Ship Customization System

### Module Data Asset

**Create**: Right-click → Data Asset → ShipCustomizationDataAsset

**Properties**:
- CustomizationName, Description, Icon
- CustomizationType: WeaponModule, EngineModule, etc.
- RequiredHardpoint: Small, Medium, Large, Utility, Internal
- PowerConsumption, Mass, BaseValue
- StatModifiers: Map of stat name → value

### Customization Widget

```blueprint
// Initialize
Initialize Customization (Spaceship)

// Install/Remove
Install Module (Module, Hardpoint Index) → Returns success
Remove Module (Hardpoint Index) → Returns success

// Preview
Preview Module Stats (Module, Hardpoint Index)
Clear Stats Preview

// Queries
Get Hardpoints → Returns array of hardpoints
Get Installed Modules → Returns array of modules
Get Total Power Consumption → Returns power used
Can Install Module (Module, Hardpoint) → Returns true/false

// Apply
Apply Customizations
Reset Customizations
```

---

## Quick Examples

### Example 1: Add Loot Item

```blueprint
On Loot Container Opened:
  → Get Player Inventory Component
  → Add Item (Loot Item, 1)
  → If Added > 0:
    → Show Message "Looted {Item Name}"
```

### Example 2: Use Consumable

```blueprint
On Use Item Button Clicked:
  → Get Inventory Component
  → Use Item (Selected Slot)
  → If Success:
    → Apply Item Effect (heal, buff, etc.)
    → Update UI
```

### Example 3: Install Weapon Module

```blueprint
On Install Button Clicked:
  → Get Ship Customization Widget
  → Install Module (Selected Module, Selected Hardpoint)
  → If Success:
    → Remove Module from Inventory
    → Play Install Animation
    → Update Stats Display
```

### Example 4: Show Low Health Warning

```blueprint
In HUD Widget, Event UpdateHealth:
  → If HealthPercent < 0.25:
    → Show Alert ("Low Health!", 0, true)
    → Flash Health Bar Red
```

---

## Common Patterns

### Inventory + Trading

```blueprint
Purchase Item:
  → Check Player Credits >= Price
  → Add Item to Inventory
  → If Added Successfully:
    → Deduct Credits
  → Else:
    → Show "Inventory Full"
```

### Ship Customization + Inventory

```blueprint
Install Module:
  → Check Player Has Module in Inventory
  → Attempt Install
  → If Success:
    → Remove from Inventory
    → Add to Ship
```

### HUD + Combat

```blueprint
On Take Damage:
  → Update Health on HUD
  → If Health Low:
    → Show Alert
  
On Shield Hit:
  → Update Shield on HUD
  → Flash Shield Effect
```

---

## File Locations

**C++ Classes**:
- `Source/Adastrea/UI/AdastreaHUDWidget.h/cpp`
- `Source/Adastrea/UI/InventoryComponent.h/cpp`
- `Source/Adastrea/UI/InventoryWidget.h/cpp`
- `Source/Adastrea/UI/InventoryItemDataAsset.h/cpp`
- `Source/Adastrea/UI/ShipCustomizationWidget.h/cpp`
- `Source/Adastrea/UI/ShipCustomizationDataAsset.h/cpp`

**Documentation**:
- `Assets/HUDSystemGuide.md` - Complete HUD guide
- `Assets/InventorySystemGuide.md` - Complete inventory guide
- `Assets/ShipCustomizationGuide.md` - Complete customization guide

**Templates**:
- `Assets/InventoryTemplates/*.yaml` - Item templates
- `Assets/ShipCustomizationTemplates/*.yaml` - Module templates

---

## Troubleshooting

### HUD not showing
- Check widget added to viewport
- Verify visibility set to Visible
- Ensure not hidden by other UI

### Inventory not updating
- Bind to OnInventoryChanged event
- Call RefreshInventory() after changes
- Check widget reference is valid

### Module won't install
- Verify hardpoint type matches
- Check power capacity not exceeded
- Ensure tech level requirement met
- Verify faction restrictions

---

**See full guides for detailed information and examples**
