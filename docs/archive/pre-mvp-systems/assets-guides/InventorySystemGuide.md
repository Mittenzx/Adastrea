# Inventory System Guide

> **Complete guide to implementing and using the inventory management system in Adastrea**

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Core Classes](#core-classes)
- [Creating Items](#creating-items)
- [Inventory Component](#inventory-component)
- [Inventory UI](#inventory-ui)
- [Blueprint Integration](#blueprint-integration)
- [Examples](#examples)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

---

## Overview

The inventory system provides a flexible, data-driven approach to managing items in Adastrea. The system supports:

- Item storage with slots and stacking
- Weight and capacity management
- Item categories and rarity
- Sorting and filtering
- Drag-and-drop UI
- Integration with trading and crafting

### System Architecture

```
UInventoryItemDataAsset (Item Definitions)
         ↓
UInventoryComponent (Storage & Logic)
         ↓
UInventoryWidget (UI Display)
```

---

## Quick Start

### 1. Create an Item Data Asset

1. In Content Browser, right-click → **Miscellaneous > Data Asset**
2. Select **InventoryItemDataAsset**
3. Name it `DA_Item_LaserWeapon`
4. Configure properties:
   - Item Name: "Laser Weapon Mk.I"
   - Category: Weapon
   - Rarity: Uncommon
   - Max Stack Size: 1
   - Weight: 25.0
   - Base Value: 5000

### 2. Add Inventory Component

In your Player Controller or Spaceship Blueprint:

1. Add Component → **InventoryComponent**
2. Set **Max Slots**: 40
3. Set **Max Weight**: 10000.0
4. Enable **Auto Stack**: true

### 3. Create Inventory Widget

1. Create Widget Blueprint → **InventoryWidget**
2. Name it `WBP_Inventory`
3. Design layout with:
   - Grid of item slots
   - Item details panel
   - Sort/filter buttons

### 4. Initialize System

```
Event BeginPlay
  → Get Inventory Component
  → Create Widget (WBP_Inventory)
  → Initialize Inventory (pass component)
  → Add to Viewport (hidden initially)

On Key Press 'I'
  → Toggle Inventory Widget Visibility
```

---

## Core Classes

### UInventoryItemDataAsset

**Location**: `Source/Adastrea/UI/InventoryItemDataAsset.h`

Defines an item that can be stored in inventory.

**Properties**:

```cpp
// Basic Information
FText ItemName              // Display name
FText Description           // Detailed description
UTexture2D* Icon           // UI icon

// Classification
EInventoryItemCategory Category    // Weapon, Module, Resource, etc.
EInventoryItemRarity Rarity       // Common to Unique

// Properties
int32 MaxStackSize         // Maximum stack (1 = no stacking)
float Weight              // Weight per unit
int32 BaseValue           // Value in credits
bool bCanBeSold           // Tradeable
bool bCanBeDropped        // Droppable
bool bIsQuestItem         // Quest item flag

// Usage
bool bCanBeUsed           // Consumable
float UsageCooldown       // Cooldown after use

// Advanced
TMap<FName, FString> CustomProperties  // Extensible properties
```

**Functions**:

```cpp
// Get rarity color for UI
FLinearColor GetRarityColor() const;

// Get custom property by name
FString GetCustomProperty(FName PropertyName, const FString& DefaultValue) const;

// Check if items can stack
bool CanStackWith(const UInventoryItemDataAsset* OtherItem) const;
```

### Item Categories

```cpp
enum class EInventoryItemCategory
{
    Weapon,          // Ship weapons
    ShipModule,      // Ship modifications
    Consumable,      // One-time use items
    Resource,        // Raw materials
    TradeGood,       // Tradeable goods
    Quest,           // Quest items
    Equipment,       // Equipment pieces
    Ammunition,      // Weapon ammo
    Misc            // Miscellaneous
};
```

### Item Rarity

```cpp
enum class EInventoryItemRarity
{
    Common,          // Gray  (0.8, 0.8, 0.8)
    Uncommon,        // Green (0.2, 0.8, 0.2)
    Rare,           // Blue  (0.2, 0.4, 1.0)
    Epic,           // Purple (0.7, 0.2, 0.9)
    Legendary,      // Orange (1.0, 0.5, 0.0)
    Unique          // Gold  (1.0, 0.8, 0.0)
};
```

---

## Creating Items

### Basic Item Template

```yaml
ItemName: "Basic Laser"
Description: "Standard energy weapon for light ships"
Category: Weapon
Rarity: Common
MaxStackSize: 1
Weight: 15.0
BaseValue: 2500
bCanBeSold: true
bCanBeDropped: true
bIsQuestItem: false
bCanBeUsed: false
CustomProperties:
  - DamageType: "Energy"
  - PowerConsumption: "50"
```

### Resource Item

```yaml
ItemName: "Iron Ore"
Description: "Raw iron ore for crafting and trading"
Category: Resource
Rarity: Common
MaxStackSize: 999
Weight: 1.0
BaseValue: 10
bCanBeSold: true
bCanBeDropped: true
```

### Consumable Item

```yaml
ItemName: "Repair Kit"
Description: "Emergency ship repair kit. Restores 25% hull integrity."
Category: Consumable
Rarity: Uncommon
MaxStackSize: 5
Weight: 2.5
BaseValue: 500
bCanBeUsed: true
UsageCooldown: 60.0
CustomProperties:
  - HealAmount: "25"
```

### Quest Item

```yaml
ItemName: "Encrypted Data Core"
Description: "Mysterious data core with encrypted files"
Category: Quest
Rarity: Unique
MaxStackSize: 1
Weight: 0.5
BaseValue: 0
bCanBeSold: false
bCanBeDropped: false
bIsQuestItem: true
```

---

## Inventory Component

### UInventoryComponent

**Location**: `Source/Adastrea/UI/InventoryComponent.h`

Manages item storage and operations.

**Properties**:

```cpp
int32 MaxSlots           // Maximum number of slots
float MaxWeight          // Maximum weight capacity (0 = unlimited)
bool bAutoStack          // Automatically stack compatible items
```

**Key Functions**:

#### Adding Items

```cpp
// Add item to inventory
int32 AddItem(UInventoryItemDataAsset* Item, int32 Quantity);

// Returns: Number of items actually added
```

**Blueprint Example**:
```
Add Item
  → Item: DA_Item_LaserWeapon
  → Quantity: 1
  → Return: Items Added
```

#### Removing Items

```cpp
// Remove item by data asset
int32 RemoveItem(UInventoryItemDataAsset* Item, int32 Quantity);

// Remove from specific slot
int32 RemoveItemFromSlot(int32 SlotIndex, int32 Quantity);
```

#### Querying Inventory

```cpp
// Get item in slot
FInventorySlot GetItemInSlot(int32 SlotIndex) const;

// Check if has item
bool HasItem(UInventoryItemDataAsset* Item, int32 Quantity) const;

// Count total quantity
int32 GetItemCount(UInventoryItemDataAsset* Item) const;

// Get current weight
float GetCurrentWeight() const;

// Get free slots
int32 GetFreeSlotCount() const;

// Get all slots
TArray<FInventorySlot> GetAllSlots() const;
```

#### Inventory Management

```cpp
// Clear all items
void ClearInventory();

// Sort by category and rarity
void SortInventory();

// Use/consume item
bool UseItem(int32 SlotIndex);
```

### Events

```cpp
// Called when item added
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, 
    UInventoryItemDataAsset*, Item, 
    int32, Quantity, 
    int32, SlotIndex);

// Called when item removed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, 
    UInventoryItemDataAsset*, Item, 
    int32, Quantity);

// Called when inventory changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
```

**Binding in Blueprint**:
```
Event BeginPlay
  → Get Inventory Component
  → Bind Event to OnItemAdded
    → Print String: "Added {Item Name} x{Quantity}"
```

---

## Inventory UI

### UInventoryWidget

**Location**: `Source/Adastrea/UI/InventoryWidget.h`

Displays and manages inventory interface.

**Key Functions**:

#### Initialize Widget

```cpp
void InitializeInventory(UInventoryComponent* InInventoryComponent);
```

Connects widget to inventory component.

#### Refresh Display

```cpp
void RefreshInventory();        // Refresh entire display
void UpdateSlot(int32 SlotIndex); // Update single slot
```

#### User Interactions

```cpp
void OnSlotSelected(int32 SlotIndex);            // Slot clicked
void OnUseItem(int32 SlotIndex);                 // Use item
void OnDropItem(int32 SlotIndex, int32 Quantity); // Drop item
```

#### Filtering

```cpp
void FilterByCategory(EInventoryItemCategory Category);
void ClearFilters();
```

#### Item Details

```cpp
void ShowItemDetails(UInventoryItemDataAsset* Item);
void HideItemDetails();
```

---

## Blueprint Integration

### Adding Items from Gameplay

#### From Trading

```
On Purchase Item
  → Get Player Inventory Component
  → Add Item (Purchased Item, Quantity)
  → If Result > 0:
    → Deduct Credits
    → Show Success Message
  → Else:
    → Show "Inventory Full" Message
```

#### From Loot

```
On Loot Container Opened
  → For Each Item in Container:
    → Add Item to Player Inventory
    → If Added Successfully:
      → Show Loot Notification
      → Remove from Container
```

#### From Crafting

```
On Craft Item
  → Check Has Required Resources
  → If Has All:
    → Remove Resource Items
    → Add Crafted Item
    → Play Craft Animation
```

### Using Items

#### Consumable Usage

```
On Use Item
  → Get Item from Slot
  → If Item.bCanBeUsed:
    → Apply Item Effect (heal, buff, etc.)
    → Remove Item from Inventory
    → Play Use Animation
    → Start Cooldown Timer
```

#### Equipment Installation

```
On Equip Item
  → Get Item from Slot
  → If Item.Category == ShipModule:
    → Open Ship Customization UI
    → Attempt Install Module
    → If Success:
      → Remove from Inventory
```

---

## Examples

### Example 1: Basic Inventory System

**Setup**:
1. Add InventoryComponent to PlayerController
2. Create simple grid UI with 20 slots
3. Add item icons and stack counts

**Features**:
- 20 slot grid
- Basic add/remove
- Stack display
- Simple tooltip on hover

### Example 2: Weight-Based Cargo Hold

**Setup**:
1. Configure MaxWeight on InventoryComponent
2. Display weight bar in UI
3. Prevent adding items over capacity

**Implementation**:
```
Before Adding Item
  → Calculate New Weight = Current + (Item.Weight * Quantity)
  → If New Weight <= Max Weight:
    → Add Item
  → Else:
    → Show "Overweight" Message
    → Calculate Max Can Add = (Max - Current) / Item.Weight
    → Offer to add partial quantity
```

### Example 3: Categorized Inventory

**Setup**:
1. Create tab buttons for each category
2. Filter display based on selected tab
3. Show category counts

**Implementation**:
```
On Category Tab Clicked (Category)
  → Filter By Category (Category)
  → Refresh Inventory Display
  → Highlight Selected Tab

Display Each Slot:
  → If Filter Active:
    → Only show matching category
  → Else:
    → Show all items
```

### Example 4: Quick Access Toolbar

**Setup**:
1. Create separate "hotbar" widget
2. Link specific inventory slots
3. Allow quick usage via hotkeys

**Implementation**:
```
Hotbar Slot 1-9:
  → Maps to Inventory Slots 0-8
  
On Hotkey Press (1-9):
  → Get Mapped Inventory Slot
  → Use Item in Slot
  → Update Hotbar Display
```

---

## Best Practices

### Item Design

1. **Clear Categories**:
   - Use appropriate categories for organization
   - Consider player's mental model
   - Group related items together

2. **Balanced Stacking**:
   - Resources: High stack sizes (100-999)
   - Equipment: No stacking (1)
   - Consumables: Moderate stacking (5-50)

3. **Meaningful Rarity**:
   - Common: Basic, abundant items
   - Uncommon: Useful, occasional finds
   - Rare: Valuable, specific purpose
   - Epic: Powerful, game-changing
   - Legendary: Extremely rare, unique abilities
   - Unique: One-of-a-kind, story items

### Performance

1. **Efficient Updates**:
   ```
   Don't refresh entire inventory for single item change
   Use UpdateSlot for individual changes
   Batch multiple changes before refresh
   ```

2. **Lazy Loading**:
   ```
   Load item icons on demand
   Cache frequently used items
   Unload unused assets
   ```

3. **Event Management**:
   ```
   Bind only necessary events
   Unbind when widget destroyed
   Use weak references where possible
   ```

### User Experience

1. **Visual Feedback**:
   - Highlight new items
   - Animate item additions
   - Show full inventory warnings
   - Indicate stackable items

2. **Sorting Options**:
   - By category
   - By rarity
   - By value
   - By weight
   - Custom user sorting

3. **Quick Actions**:
   - Context menu on right-click
   - Drag and drop support
   - Keyboard shortcuts
   - Batch operations

---

## Troubleshooting

### Items Not Stacking

**Symptom**: Same items occupy multiple slots

**Solutions**:
1. Check AutoStack is enabled:
   ```
   InventoryComponent.bAutoStack = true
   ```

2. Verify MaxStackSize:
   ```
   Item.MaxStackSize > 1
   ```

3. Ensure items are identical:
   ```
   Must be same Data Asset reference
   ```

### Weight Capacity Issues

**Symptom**: Can't add items even with free slots

**Solutions**:
1. Check current weight:
   ```
   Get Current Weight
   Compare to Max Weight
   ```

2. Display weight to player:
   ```
   Show "Weight: 750/1000" in UI
   ```

3. Implement weight management:
   ```
   Allow player to drop heavy items
   Provide cargo expansion options
   ```

### UI Not Updating

**Symptom**: Inventory display doesn't reflect changes

**Solutions**:
1. Verify event binding:
   ```
   Check OnInventoryChanged is bound
   Add debug print in event handler
   ```

2. Force refresh:
   ```
   Call RefreshInventory() after operations
   ```

3. Check widget visibility:
   ```
   Ensure widget is in viewport
   Verify not hidden or collapsed
   ```

---

## YAML Import Templates

See `Assets/InventoryTemplates/` for complete YAML examples:

- `WeaponItems.yaml` - Weapon definitions
- `ResourceItems.yaml` - Resource items
- `ConsumableItems.yaml` - Consumable items
- `ModuleItems.yaml` - Ship modules
- `QuestItems.yaml` - Quest item examples

---

## Additional Resources

- **Trading System**: See `Assets/TradingSystemGuide.md`
- **Ship Customization**: See `Assets/ShipCustomizationGuide.md`
- **Data Asset Creation**: See `YAML_IMPORT_GUIDE.md`
- **Blueprint Guide**: See `BLUEPRINT_CREATION_GUIDE.md`

---

**Last Updated**: November 11, 2025  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
