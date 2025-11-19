# Blueprint Display Guide - Best Practices for Visual Documentation

## Overview

This guide establishes consistent, clear, and maintainable standards for documenting Unreal Engine Blueprint logic in markdown files. It provides multiple visualization formats optimized for different use cases and viewing platforms.

**Last Updated**: November 19, 2025  
**Version**: 1.0.0

---

## Table of Contents

1. [Why Better Blueprint Documentation?](#why-better-blueprint-documentation)
2. [Visual Format Standards](#visual-format-standards)
3. [Format 1: Mermaid Flowcharts](#format-1-mermaid-flowcharts-recommended)
4. [Format 2: Enhanced Text Flow](#format-2-enhanced-text-flow)
5. [Format 3: Node Cards](#format-3-node-cards)
6. [Format 4: Quick Reference Tables](#format-4-quick-reference-tables)
7. [Format 5: Hybrid Approach](#format-5-hybrid-approach-best-for-complex-logic)
8. [Color Coding Standards](#color-coding-standards)
9. [Node Type Reference](#node-type-reference)
10. [Best Practices](#best-practices)
11. [Migration Guide](#migration-guide)
12. [Templates](#templates)

---

## Why Better Blueprint Documentation?

### Problems with Current Approach
- ❌ ASCII art is hard to read and maintain
- ❌ Inconsistent formatting across documents
- ❌ No visual hierarchy or color coding
- ❌ Difficult to distinguish node types
- ❌ Poor rendering on mobile/GitHub

### Benefits of New Standards
- ✅ Clear visual hierarchy
- ✅ Color-coded node types
- ✅ Platform-agnostic rendering
- ✅ Easy to update and maintain
- ✅ Professional appearance
- ✅ Better for learning and reference

---

## Visual Format Standards

### When to Use Each Format

| Format | Best For | Complexity | GitHub Support |
|--------|----------|------------|----------------|
| **Mermaid Flowcharts** | Complex logic flows, branching | High | ✅ Native |
| **Enhanced Text Flow** | Linear sequences, simple flows | Low-Medium | ✅ Always |
| **Node Cards** | Detailed node documentation | Medium | ✅ Always |
| **Quick Reference Tables** | Function summaries, overviews | Low | ✅ Always |
| **Hybrid Approach** | Complete systems, tutorials | High | ✅ Always |

---

## Format 1: Mermaid Flowcharts (Recommended)

### Overview
Mermaid diagrams render natively on GitHub and provide professional flowchart visualization.

### Advantages
- 🎨 **Visual**: Automatic layout and styling
- 🔄 **Interactive**: Clickable nodes on GitHub
- 🎯 **Clear**: Easy to follow execution flow
- 📱 **Responsive**: Works on all devices

### Basic Structure

```mermaid
graph TD
    Start[Event Begin Play] --> Check{Is Valid?}
    Check -->|Yes| Process[Process Data]
    Check -->|No| Error[Show Error]
    Process --> End[Complete]
    Error --> End
    
    style Start fill:#4CAF50,stroke:#2E7D32,color:#fff
    style Check fill:#2196F3,stroke:#1565C0,color:#fff
    style Process fill:#FF9800,stroke:#E65100,color:#fff
    style Error fill:#F44336,stroke:#C62828,color:#fff
    style End fill:#9E9E9E,stroke:#616161,color:#fff
```

### Example: Trading System Buy Flow

```mermaid
graph TD
    A[🖱️ OnBuyButtonClicked] --> B[Get Selected Item]
    B --> C{Item Valid?}
    C -->|No| D[❌ Show Error:<br/>Select an item]
    C -->|Yes| E[Get Item Price]
    E --> F[Calculate Total Cost]
    F --> G{Player Has Credits?}
    G -->|No| H[❌ Show Error:<br/>Insufficient funds]
    G -->|Yes| I[Deduct Credits]
    I --> J[Add Item to Inventory]
    J --> K[▶️ Play Buy Sound]
    K --> L[🔄 Refresh UI]
    L --> M[✅ Success]
    D --> N[End]
    H --> N
    M --> N
    
    style A fill:#4CAF50,stroke:#2E7D32,stroke-width:3px,color:#fff
    style C fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style G fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style D fill:#F44336,stroke:#C62828,stroke-width:2px,color:#fff
    style H fill:#F44336,stroke:#C62828,stroke-width:2px,color:#fff
    style M fill:#4CAF50,stroke:#2E7D32,stroke-width:2px,color:#fff
    style I fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style J fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style K fill:#9C27B0,stroke:#6A1B9A,stroke-width:2px,color:#fff
    style L fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
```

### Mermaid Node Types

```mermaid
graph LR
    Event[📅 Event Node] --> Function[⚙️ Function Call]
    Function --> Branch{❓ Branch/Decision}
    Branch -->|True| Action[🔧 Action Node]
    Branch -->|False| Loop[🔄 Loop Node]
    Action --> Variable[📦 Variable Set]
    Variable --> Pure[🔍 Pure Function]
    Pure --> Output[📤 Return/Output]
    
    style Event fill:#4CAF50,stroke:#2E7D32,stroke-width:2px,color:#fff
    style Function fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style Branch fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style Action fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style Loop fill:#00BCD4,stroke:#006064,stroke-width:2px,color:#fff
    style Variable fill:#9C27B0,stroke:#6A1B9A,stroke-width:2px,color:#fff
    style Pure fill:#009688,stroke:#004D40,stroke-width:2px,color:#fff
    style Output fill:#9E9E9E,stroke:#424242,stroke-width:2px,color:#fff
```

### Complex Example: Station Module Addition

```mermaid
graph TD
    A[🖱️ Button: Add Module] --> B[📦 Get Selected Module Class]
    B --> C{✓ Is Valid?}
    C -->|No| D[❌ Error: Select module type]
    C -->|Yes| E[📍 Get Placement Location]
    E --> F[⚙️ Is Valid Placement?]
    F -->|No| G[❌ Error: Invalid location]
    F -->|Yes| H[🔧 Can Add Module For Faction?]
    H -->|No| I[📊 Get Current Faction]
    I --> J[🔢 Get Tech Level]
    J --> K[📝 Format Error Message]
    K --> L[❌ Show Tech Requirement Error]
    H -->|Yes| M[➕ Add Module<br/>C++ Function]
    M --> N{✓ Module Created?}
    N -->|No| O[❌ Error: Creation failed]
    N -->|Yes| P[🔄 Update Module List UI]
    P --> Q[🎵 Play Success Sound]
    Q --> R[🧹 Clear Selection]
    R --> S[✅ Success Complete]
    D --> T[End]
    G --> T
    L --> T
    O --> T
    S --> T
    
    style A fill:#4CAF50,stroke:#2E7D32,stroke-width:3px,color:#fff
    style C fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style F fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style H fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style N fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style D fill:#F44336,stroke:#C62828,stroke-width:2px,color:#fff
    style G fill:#F44336,stroke:#C62828,stroke-width:2px,color:#fff
    style L fill:#F44336,stroke:#C62828,stroke-width:2px,color:#fff
    style O fill:#F44336,stroke:#C62828,stroke-width:2px,color:#fff
    style M fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style P fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style Q fill:#9C27B0,stroke:#6A1B9A,stroke-width:2px,color:#fff
    style S fill:#4CAF50,stroke:#2E7D32,stroke-width:2px,color:#fff
```

### Mermaid Best Practices

**DO:**
- ✅ Use emojis for quick visual identification
- ✅ Keep node text concise (under 30 characters)
- ✅ Use color coding consistently
- ✅ Add descriptive labels to decision branches
- ✅ Group related nodes visually

**DON'T:**
- ❌ Create overly complex diagrams (split into multiple)
- ❌ Use inconsistent styling
- ❌ Forget to label branches
- ❌ Mix different visual styles

---

## Format 2: Enhanced Text Flow

### Overview
Structured text format with clear visual hierarchy, perfect for linear sequences and simple documentation.

### Structure

```
🎯 Function: [Function Name]
📥 Inputs: [Input parameters]
📤 Outputs: [Return values]

──────────────────────────────────
EXECUTION FLOW
──────────────────────────────────

🟢 START → Event Begin Play
    │
    ├─→ 📦 Get Player Controller
    │      └─→ Cast to AdastreaPlayerController
    │            ├─→ ✅ Success Path
    │            │     ├─→ 🔧 Get Current Station
    │            │     │     └─→ 📦 Store in Variable: CurrentStation
    │            │     │
    │            │     ├─→ ⚙️ Initialize Faction Data
    │            │     │     └─→ Call: Set Station Faction()
    │            │     │
    │            │     └─→ 🔄 Refresh All UI Elements
    │            │           ├─→ Update Module List
    │            │           ├─→ Update Faction Display
    │            │           └─→ Update Tech Level Indicator
    │            │
    │            └─→ ❌ Failure Path
    │                  └─→ 📝 Log Warning: "Invalid Player Controller"
    │
    └─→ 🏁 END
```

### Example: Price Calculation Function

```
🎯 Function: Calculate Item Price
📥 Inputs:
   • BasePrice (Float) - Base item cost
   • Quantity (Integer) - Number of units
   • IsBuying (Boolean) - True for buy, false for sell
📤 Outputs:
   • TotalPrice (Float) - Final calculated price
   • PricePerUnit (Float) - Price per single unit

──────────────────────────────────
EXECUTION FLOW
──────────────────────────────────

🟢 START
    │
    ├─→ 🔍 Validate Inputs
    │     ├─→ ❓ Is BasePrice > 0?
    │     │     ├─→ ✅ Yes → Continue
    │     │     └─→ ❌ No → Return 0.0
    │     │
    │     └─→ ❓ Is Quantity > 0?
    │           ├─→ ✅ Yes → Continue
    │           └─→ ❌ No → Return 0.0
    │
    ├─→ 💰 Calculate Base Total
    │     └─→ BaseTotalPrice = BasePrice × Quantity
    │
    ├─→ 🔧 Apply Price Modifiers
    │     ├─→ ❓ Is Buying?
    │     │     ├─→ ✅ Yes → Apply Buy Markup (×1.15)
    │     │     └─→ ❌ No → Apply Sell Markdown (×0.85)
    │     │
    │     └─→ 📈 Apply Supply/Demand Modifier
    │           └─→ Get Market Supply/Demand Ratio
    │                 └─→ ModifiedPrice = Price × Ratio
    │
    ├─→ 🏛️ Apply Transaction Tax
    │     └─→ TaxAmount = ModifiedPrice × TaxRate
    │           └─→ FinalPrice = ModifiedPrice + TaxAmount
    │
    ├─→ 📤 Calculate Per-Unit Price
    │     └─→ PricePerUnit = FinalPrice / Quantity
    │
    └─→ 🏁 RETURN
          ├─→ TotalPrice = FinalPrice
          └─→ PricePerUnit = PricePerUnit

──────────────────────────────────
💡 NOTES
──────────────────────────────────
• All calculations use floating-point precision
• Tax rate is retrieved from market configuration
• Supply/demand ratio ranges from 0.5 to 2.0
• Price is clamped to minimum of 1 credit
```

### Visual Elements Key

| Symbol | Meaning | Usage |
|--------|---------|-------|
| 🟢 | Start | Function/event entry point |
| 🏁 | End | Function/event completion |
| 📥 | Input | Function parameters |
| 📤 | Output | Return values |
| 🔧 | Action | Execute function or operation |
| ⚙️ | Process | Complex operation or calculation |
| 📦 | Variable | Get or set variable |
| ❓ | Branch | Decision point |
| ✅ | True Path | Condition satisfied |
| ❌ | False Path | Condition failed or error |
| 🔄 | Loop | Iteration or repeat |
| 🔍 | Check | Validation or query |
| 💡 | Note | Additional information |
| 📝 | Log | Debug or log output |
| 🎵 | Audio | Sound/music playback |
| 🎨 | Visual | UI or visual effect |
| 📊 | Data | Data retrieval or processing |
| 🌐 | Network | Online/multiplayer operation |

---

## Format 3: Node Cards

### Overview
Detailed visual cards for documenting individual Blueprint nodes with all properties and connections.

### Structure

<details>
<summary>📦 <b>Get Faction Reputation</b> - Pure Function</summary>

```
┌─────────────────────────────────────────────────────────────┐
│ 🔍 GET FACTION REPUTATION                                   │
│ Type: Pure Function (no execution pins)                     │
│ Category: Faction System                                    │
├─────────────────────────────────────────────────────────────┤
│ 📥 INPUTS                                                    │
│   • Faction (FactionDataAsset) - Target faction to query    │
│                                                              │
│ 📤 OUTPUTS                                                   │
│   • Reputation (Integer) - Current reputation value         │
│   • Found (Boolean) - True if faction relationship exists   │
├─────────────────────────────────────────────────────────────┤
│ 📋 DESCRIPTION                                               │
│   Retrieves the player's current reputation with the        │
│   specified faction. Returns 0 if no relationship exists.   │
│                                                              │
│ 💡 USAGE                                                     │
│   Used to check if player meets reputation requirements     │
│   for missions, trading discounts, or access permissions.   │
│                                                              │
│ ⚠️ IMPORTANT                                                 │
│   • Always check 'Found' output before using reputation     │
│   • Returns 0 for unknown factions (not an error)           │
│   • Reputation range: -100 (Hostile) to +100 (Allied)       │
├─────────────────────────────────────────────────────────────┤
│ 📝 EXAMPLE USAGE                                             │
│                                                              │
│   Get Faction Reputation                                    │
│     Input: DA_Faction_SolarisUnion                          │
│     Output: Reputation = 50, Found = true                   │
│         ↓                                                    │
│   Branch (Found = true)                                     │
│     ├─→ True: Display reputation in UI                      │
│     └─→ False: Show "No relationship established"           │
└─────────────────────────────────────────────────────────────┘
```

</details>

### Example: Combat Function Node Card

<details>
<summary>🎯 <b>Fire Weapon</b> - C++ Function Call</summary>

```
┌─────────────────────────────────────────────────────────────┐
│ 🔫 FIRE WEAPON                                              │
│ Type: BlueprintCallable Function                            │
│ Category: Combat System                                     │
│ Source: WeaponComponent.h                                   │
├─────────────────────────────────────────────────────────────┤
│ 📥 INPUTS                                                    │
│   • Target (Actor) - Actor to fire at (optional)            │
│   • FireLocation (Vector) - Override spawn point (optional) │
│   • bAutoLead (Boolean) - Auto-calculate lead (default: true)│
│                                                              │
│ 📤 OUTPUTS                                                   │
│   • Success (Boolean) - True if weapon fired                │
│   • ProjectileSpawned (Actor) - Reference to spawned        │
│     projectile (null for hitscan)                           │
├─────────────────────────────────────────────────────────────┤
│ 📋 DESCRIPTION                                               │
│   Fires the currently equipped weapon at the target or      │
│   in the specified direction. Handles ammunition, heat,     │
│   cooldown, and projectile spawning automatically.          │
│                                                              │
│ 🔧 BEHAVIOR                                                  │
│   1. Checks if weapon is ready to fire                      │
│   2. Consumes ammunition (if required)                      │
│   3. Generates heat and starts cooldown                     │
│   4. Spawns projectile or traces line for hitscan           │
│   5. Applies recoil and visual effects                      │
│   6. Returns success status                                 │
│                                                              │
│ 💡 USAGE TIPS                                                │
│   • Call on input action for manual firing                  │
│   • Use in timer for automatic firing                       │
│   • Check Success output before playing effects             │
│   • Auto-lead requires valid target with velocity           │
│                                                              │
│ ⚠️ REQUIREMENTS                                              │
│   • Weapon must be equipped and loaded                      │
│   • Component must be initialized                           │
│   • Sufficient power available (for energy weapons)         │
│   • Not overheated or on cooldown                           │
├─────────────────────────────────────────────────────────────┤
│ 📝 BLUEPRINT EXAMPLE                                         │
│                                                              │
│   [Input Action: Fire Weapon]                               │
│          ↓                                                   │
│   Get WeaponComponent                                       │
│          ↓                                                   │
│   Fire Weapon                                               │
│     • Target: CurrentTarget                                 │
│     • bAutoLead: true                                       │
│          ↓                                                   │
│   Branch (Success?)                                         │
│     ├─→ True:  Play Firing Sound                            │
│     │          Play Muzzle Flash VFX                        │
│     │          Apply Screen Shake                           │
│     └─→ False: Play Dry Fire Sound                          │
│                Show "Weapon Not Ready" UI                   │
└─────────────────────────────────────────────────────────────┘
```

</details>

---

## Format 4: Quick Reference Tables

### Overview
Compact tabular format for summarizing multiple functions or nodes at a glance.

### Function Reference Table

| Function | Category | Inputs | Outputs | Purpose |
|----------|----------|--------|---------|---------|
| 🎯 **Fire Weapon** | Combat | Target, Location | Success, Projectile | Fires equipped weapon |
| 🔍 **Get Current Target** | Targeting | - | Target Actor | Returns active target |
| 💰 **Calculate Price** | Trading | Item, Qty, IsBuy | Price | Computes transaction price |
| 📦 **Get Reputation** | Faction | Faction | Value, Found | Queries faction standing |
| ⚡ **Apply Damage** | Combat | Amount, Type, Actor | Actual Damage | Deals damage to target |
| 🔄 **Refresh UI** | UI | - | - | Updates all UI elements |
| 🏛️ **Add Module** | Station | Type, Location | Module | Adds station module |
| 🔧 **Can Add Module** | Station | Type, Faction | Boolean | Checks tech requirements |

### Event Reference Table

| Event | Trigger | Parameters | Description |
|-------|---------|------------|-------------|
| 🎮 **Begin Play** | Actor spawned | - | Initialization logic |
| 🖱️ **Button Clicked** | UI interaction | - | Handle button press |
| 💥 **Take Damage** | Combat | Damage, Type, Instigator | Process incoming damage |
| 🎯 **Target Acquired** | Targeting | New Target | New target locked |
| 💰 **Transaction Complete** | Trading | Item, Amount, Cost | Trade finished |
| 📊 **Reputation Changed** | Faction | Faction, Old, New | Standing updated |
| ⚙️ **Module Added** | Station | Module, Success | Module placement done |

### Variable Reference Table

| Variable | Type | Category | Default | Description |
|----------|------|----------|---------|-------------|
| 📦 **CurrentStation** | SpaceStation | Station System | null | Active station reference |
| 🎯 **SelectedTarget** | Actor | Combat | null | Currently targeted actor |
| 💰 **PlayerCredits** | Integer | Economy | 10000 | Available currency |
| 📊 **FactionReputation** | Map<Faction,Int> | Faction System | Empty | All faction standings |
| 🔫 **EquippedWeapon** | WeaponData | Combat | null | Active weapon asset |
| 🏠 **CurrentHomeworld** | HomeworldData | Player | null | Starting homeworld |

---

## Format 5: Hybrid Approach (Best for Complex Logic)

### Overview
Combines multiple formats for comprehensive documentation of complex systems.

### Example: Complete Trading Transaction Flow

#### 1. High-Level Overview (Mermaid)

```mermaid
graph TD
    A[Start Transaction] --> B[Validate]
    B --> C[Calculate Cost]
    C --> D[Process Payment]
    D --> E[Update Inventory]
    E --> F[Complete]
    
    style A fill:#4CAF50,stroke:#2E7D32,color:#fff
    style F fill:#4CAF50,stroke:#2E7D32,color:#fff
```

#### 2. Detailed Flow (Enhanced Text)

```
🎯 Function: Execute Trading Transaction
📥 Inputs: Item, Quantity, IsBuying, Market
📤 Outputs: Success, ErrorMessage

──────────────────────────────────
PHASE 1: VALIDATION
──────────────────────────────────

🔍 Validate Inputs
    ├─→ ❓ Item != null?
    │     └─→ ❌ No → Return Error: "Invalid item"
    │
    ├─→ ❓ Quantity > 0?
    │     └─→ ❌ No → Return Error: "Invalid quantity"
    │
    └─→ ❓ Market != null?
          └─→ ❌ No → Return Error: "Invalid market"

──────────────────────────────────
PHASE 2: PRICE CALCULATION
──────────────────────────────────

💰 Calculate Transaction Cost
    ├─→ Get Base Price from Market
    ├─→ Apply Buy/Sell Modifier
    ├─→ Apply Supply/Demand
    ├─→ Apply Faction Relationship Discount
    └─→ Calculate Tax

──────────────────────────────────
PHASE 3: EXECUTE TRANSACTION
──────────────────────────────────

[Continue with payment processing...]
```

#### 3. Key Functions (Node Cards)

<details>
<summary>📦 <b>Get Item Price</b></summary>

[Full node card documentation here...]

</details>

#### 4. Function Summary (Table)

| Step | Function | Purpose |
|------|----------|---------|
| 1 | Validate Transaction | Check all inputs |
| 2 | Calculate Cost | Compute final price |
| 3 | Process Payment | Deduct/add credits |
| 4 | Update Inventory | Add/remove items |
| 5 | Refresh UI | Update displays |

---

## Color Coding Standards

### Standard Color Palette

```mermaid
graph LR
    Event[🟢 Events<br/>Begin Play, Input Actions] 
    Function[🟠 Functions<br/>Any function call]
    Branch[🔵 Branches<br/>Decisions, Switches]
    Loop[🟦 Loops<br/>ForEach, While]
    Variable[🟣 Variables<br/>Get/Set operations]
    Pure[🟩 Pure Functions<br/>Getters, calculations]
    Error[🔴 Errors<br/>Failures, warnings]
    Success[🟢 Success<br/>Completion, validation]
    
    style Event fill:#4CAF50,stroke:#2E7D32,stroke-width:2px,color:#fff
    style Function fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style Branch fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style Loop fill:#00BCD4,stroke:#006064,stroke-width:2px,color:#fff
    style Variable fill:#9C27B0,stroke:#6A1B9A,stroke-width:2px,color:#fff
    style Pure fill:#009688,stroke:#004D40,stroke-width:2px,color:#fff
    style Error fill:#F44336,stroke:#C62828,stroke-width:2px,color:#fff
    style Success fill:#4CAF50,stroke:#2E7D32,stroke-width:2px,color:#fff
```

### Color Meanings

| Color | Node Type | Hex Code | When to Use |
|-------|-----------|----------|-------------|
| 🟢 Green | Events, Success | `#4CAF50` | Start points, successful outcomes |
| 🟠 Orange | Functions | `#FF9800` | Function calls, actions |
| 🔵 Blue | Branches | `#2196F3` | Decisions, conditionals |
| 🟦 Cyan | Loops | `#00BCD4` | Iterations, repeated operations |
| 🟣 Purple | Variables | `#9C27B0` | Get/Set variable operations |
| 🟩 Teal | Pure Functions | `#009688` | Calculations without side effects |
| 🔴 Red | Errors | `#F44336` | Error states, failures |
| ⚪ Gray | End Points | `#9E9E9E` | Completion, exit points |

---

## Node Type Reference

### Complete Node Type Guide

| Node Type | Symbol | Mermaid Shape | Color | Example |
|-----------|--------|---------------|-------|---------|
| Event | 📅 | `[Event Name]` | Green | `[📅 Begin Play]` |
| Function Call | ⚙️ | `[Function]` | Orange | `[⚙️ Fire Weapon]` |
| Branch | ❓ | `{Decision?}` | Blue | `{❓ Is Valid?}` |
| Switch | 🔀 | `{Switch}` | Blue | `{🔀 Enum Value}` |
| Loop | 🔄 | `[Loop]` | Cyan | `[🔄 ForEach]` |
| Variable Get | 📦 | `[Get Var]` | Purple | `[📦 PlayerHealth]` |
| Variable Set | 📝 | `[Set Var]` | Purple | `[📝 Set Score]` |
| Pure Function | 🔍 | `[Pure Func]` | Teal | `[🔍 Calculate]` |
| Cast | 🎭 | `[Cast To]` | Orange | `[🎭 Cast to Player]` |
| Spawn Actor | ➕ | `[Spawn]` | Orange | `[➕ Spawn Projectile]` |
| Delay | ⏱️ | `[Delay]` | Gray | `[⏱️ Wait 2s]` |
| Print/Log | 📝 | `[Print]` | Gray | `[📝 Log Message]` |
| Return | 📤 | `[Return]` | Gray | `[📤 Return Value]` |
| Error | ❌ | `[Error]` | Red | `[❌ Show Error]` |
| Success | ✅ | `[Success]` | Green | `[✅ Complete]` |

### Special Nodes

| Node Type | Symbol | Description |
|-----------|--------|-------------|
| 🎮 Input Event | Player input action |
| 🖱️ UI Event | Button click, widget interaction |
| 💥 Combat Event | Damage taken, weapon fired |
| 💰 Economy Event | Purchase, sale, payment |
| 🏛️ System Event | Module added, faction changed |
| 🎵 Audio | Play sound/music |
| 🎨 VFX | Visual effects, particles |
| 📊 Data | Data processing, calculations |
| 🌐 Network | Multiplayer/online operations |

---

## Best Practices

### General Guidelines

#### DO ✅
1. **Be Consistent**: Use the same format throughout a document
2. **Use Color Coding**: Help readers quickly identify node types
3. **Add Context**: Include descriptions and usage notes
4. **Show Data Flow**: Clearly indicate how data moves between nodes
5. **Document Assumptions**: Note requirements and prerequisites
6. **Include Examples**: Show real usage scenarios
7. **Keep It Simple**: Split complex flows into multiple diagrams
8. **Use Emojis Wisely**: For quick visual scanning (but don't overdo it)

#### DON'T ❌
1. **Mix Formats**: Don't combine different styles inconsistently
2. **Overcomplicate**: Break complex logic into smaller chunks
3. **Skip Error Paths**: Always document failure cases
4. **Forget Parameters**: Document all inputs and outputs
5. **Use Tiny Text**: Ensure readability at normal zoom
6. **Ignore Updates**: Keep docs in sync with code changes
7. **Copy-Paste Blindly**: Adapt templates to your specific needs

### Accessibility

- Use high contrast colors for visibility
- Include text descriptions with visual diagrams
- Avoid relying solely on color to convey meaning
- Ensure emoji fallbacks for plain text viewers
- Test rendering on GitHub, VS Code, and browser

### Performance

- Keep Mermaid diagrams under 30 nodes
- Use collapsed sections for large card collections
- Consider linking to separate files for huge flows
- Optimize image sizes if using screenshots

---

## Migration Guide

### Converting Old ASCII Diagrams

#### Before (Old ASCII Style)

```
┌─────────────────────────────────────┐
│ Function: Initialize Faction        │
│ Reputation From Homeworld           │
└─────────────────────────────────────┘

Input: SelectedHomeworld
   ↓
┌──────────────────────────────┐
│ Is Valid (SelectedHomeworld) │
└──────────────────────────────┘
   ↓
Branch
 ├─ False ──→ Print String
 │            "Invalid Homeworld"
 │               ↓
 │            Return False
 │
 └─ True ──→ Set CurrentHomeworld
              (Store on Player Character)
```

#### After (New Mermaid Style)

```mermaid
graph TD
    A[📥 Input: SelectedHomeworld] --> B{✓ Is Valid?}
    B -->|No| C[❌ Print Error:<br/>Invalid Homeworld]
    C --> D[📤 Return False]
    B -->|Yes| E[📦 Set CurrentHomeworld]
    E --> F[✅ Continue Processing]
    
    style A fill:#9C27B0,stroke:#6A1B9A,color:#fff
    style B fill:#2196F3,stroke:#1565C0,color:#fff
    style C fill:#F44336,stroke:#C62828,color:#fff
    style E fill:#FF9800,stroke:#E65100,color:#fff
    style F fill:#4CAF50,stroke:#2E7D32,color:#fff
```

### Conversion Checklist

- [ ] Identify the flow type (linear, branching, complex)
- [ ] Choose appropriate new format
- [ ] Convert nodes to standardized symbols
- [ ] Apply color coding
- [ ] Add emoji icons for visual scanning
- [ ] Include input/output documentation
- [ ] Add usage examples
- [ ] Test rendering on GitHub
- [ ] Update any references to the diagram

### Quick Migration Tool

Use this pattern matcher to identify sections needing updates:

```bash
# Find old ASCII diagrams
grep -r "└─" --include="*.md" .

# Find old arrow flows
grep -r "├─→" --include="*.md" .

# Find blueprint graphs without mermaid
grep -B5 "Blueprint Graph:" --include="*.md" . | grep -v "mermaid"
```

---

## Templates

### Template 1: Simple Function (Mermaid)

```mermaid
graph TD
    A[📅 Function Start] --> B[📦 Get Input Data]
    B --> C[⚙️ Process Data]
    C --> D{✓ Success?}
    D -->|Yes| E[✅ Return Success]
    D -->|No| F[❌ Return Error]
    
    style A fill:#4CAF50,stroke:#2E7D32,color:#fff
    style D fill:#2196F3,stroke:#1565C0,color:#fff
    style E fill:#4CAF50,stroke:#2E7D32,color:#fff
    style F fill:#F44336,stroke:#C62828,color:#fff
```

### Template 2: Enhanced Text Flow

```
🎯 Function: [Function Name]
📥 Inputs: [Parameters]
📤 Outputs: [Returns]

──────────────────────────────────
EXECUTION FLOW
──────────────────────────────────

🟢 START
    ├─→ [Step 1]
    ├─→ [Step 2]
    │     └─→ [Sub-step]
    └─→ 🏁 END
```

### Template 3: Node Card

<details>
<summary>⚙️ <b>[Function Name]</b> - [Node Type]</summary>

```
┌─────────────────────────────────────────────────────────────┐
│ [ICON] [FUNCTION NAME]                                      │
│ Type: [Node Type]                                           │
│ Category: [Category]                                        │
├─────────────────────────────────────────────────────────────┤
│ 📥 INPUTS                                                    │
│   • [Input 1] ([Type]) - [Description]                     │
│                                                              │
│ 📤 OUTPUTS                                                   │
│   • [Output 1] ([Type]) - [Description]                    │
├─────────────────────────────────────────────────────────────┤
│ 📋 DESCRIPTION                                               │
│   [What this node does]                                     │
│                                                              │
│ 💡 USAGE                                                     │
│   [When to use this node]                                   │
│                                                              │
│ ⚠️ IMPORTANT                                                 │
│   [Critical notes and warnings]                             │
└─────────────────────────────────────────────────────────────┘
```

</details>

### Template 4: Function Summary Table

| Function | Inputs | Outputs | Purpose |
|----------|--------|---------|---------|
| [Name] | [Params] | [Returns] | [Description] |

---

## Examples by System

### Combat System Example

See how combat system functions are documented:

```mermaid
graph TD
    A[🎮 Input: Fire Weapon] --> B[📦 Get WeaponComponent]
    B --> C{✓ Weapon Ready?}
    C -->|No| D[❌ Play Dry Fire Sound]
    C -->|Yes| E[⚙️ Fire Weapon]
    E --> F[💥 Spawn Projectile]
    F --> G[🎵 Play Fire Sound]
    G --> H[🎨 Spawn Muzzle Flash]
    H --> I[⚙️ Apply Recoil]
    I --> J[✅ Complete]
    D --> J
    
    style A fill:#4CAF50,stroke:#2E7D32,stroke-width:3px,color:#fff
    style C fill:#2196F3,stroke:#1565C0,stroke-width:2px,color:#fff
    style E fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style F fill:#FF9800,stroke:#E65100,stroke-width:2px,color:#fff
    style D fill:#F44336,stroke:#C62828,stroke-width:2px,color:#fff
    style J fill:#4CAF50,stroke:#2E7D32,stroke-width:2px,color:#fff
```

### Trading System Example

[See full trading examples in TradingSystemBlueprintGuide.md]

### UI System Example

[See full UI examples in BlueprintWorkflowTemplates.md]

---

## Tools and Resources

### Recommended Tools

- **Mermaid Live Editor**: https://mermaid.live/
- **VS Code Extension**: Markdown Preview Mermaid Support
- **GitHub**: Native Mermaid rendering
- **Draw.io**: For complex diagrams

### Testing Your Diagrams

1. Copy your Mermaid code
2. Paste into https://mermaid.live/
3. Verify it renders correctly
4. Copy back to your markdown file
5. Test on GitHub preview

### Online Resources

- **Mermaid Documentation**: https://mermaid.js.org/
- **GitHub Flavored Markdown**: https://guides.github.com/features/mastering-markdown/
- **Emoji Cheat Sheet**: https://github.com/ikatyang/emoji-cheat-sheet

---

## Conclusion

By following these standards, Blueprint documentation in Adastrea will be:

- ✅ **Clearer** - Easy to understand at a glance
- ✅ **Consistent** - Same style across all docs
- ✅ **Professional** - Publication-quality visuals
- ✅ **Maintainable** - Simple to update and modify
- ✅ **Accessible** - Works for all skill levels

**Remember**: The goal is to help developers and designers understand Blueprint logic quickly and accurately. Choose the format that best serves your readers!

---

**Questions or Suggestions?**

Open an issue on GitHub or contribute improvements to this guide!

---

**Document Version**: 1.0.0  
**Last Updated**: November 19, 2025  
**Maintained by**: Adastrea Documentation Team
