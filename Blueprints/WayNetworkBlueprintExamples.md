# Way Network Blueprint Examples

## Overview

This document provides copy-paste Blueprint node setups for integrating Way Networks into your game. All examples use visual scripting only—no C++ required.

## Example 1: Register All Networks on Game Start

**Purpose**: Automatically register all network Data Assets when game begins

**Location**: Game Mode Blueprint → Event BeginPlay

### Blueprint Nodes

```
Event BeginPlay
│
├─→ Get Game Instance
│   └─→ Get Subsystem (VerseSubsystem)
│       │
│       ├─→ Load Asset (DA_Network_CraftsmenCompact)
│       │   └─→ Register Network
│       │
│       ├─→ Load Asset (DA_Network_FrontierAlliance)
│       │   └─→ Register Network
│       │
│       ├─→ Load Asset (DA_Network_HonorCircle)
│       │   └─→ Register Network
│       │
│       ├─→ Load Asset (DA_Network_MerchantCoalition)
│       │   └─→ Register Network
│       │
│       └─→ Load Asset (DA_Network_ScholarsSyndicate)
│           └─→ Register Network
│
└─→ Print String: "Way Networks Initialized"
```

### Step-by-Step Setup

1. Open your Game Mode Blueprint
2. Find or create **Event BeginPlay**
3. Drag from execution pin → **Get Game Instance**
4. Drag from Game Instance → **Get Subsystem**
   - Select Class: `VerseSubsystem`
5. For each network:
   - Add **Load Object** node
   - Set Object Class: `WayNetworkDataAsset`
   - Set Path: Your network asset path
   - Drag from loaded object → **Register Network** on VerseSubsystem
6. Connect all Register Network nodes in sequence
7. Add **Print String** at end for confirmation

## Example 2: Display Network Information Widget

**Purpose**: Show player's standing with a specific network

**Location**: Custom Widget Blueprint (WBP_NetworkInfo)

### Widget Structure

```
Canvas Panel
├─ Vertical Box (Network_Container)
│  ├─ Text (Network_Name)
│  ├─ Text (Network_Description)
│  ├─ Progress Bar (Reputation_Bar)
│  ├─ Text (Reputation_Label)
│  └─ Scroll Box (Member_List)
│     └─ [Dynamically added member widgets]
└─ Button (View_Benefits_Button)
```

### Blueprint Graph - On Construct

```
Event Construct
│
├─→ Get Game Instance
│   └─→ Get Subsystem (VerseSubsystem)
│       │
│       ├─→ Variable: Store as "Verse System"
│       │
│       └─→ [Network Asset Variable]
│           │
│           ├─→ Get Network Name
│           │   └─→ Set Text (Network_Name)
│           │
│           ├─→ Get Description
│           │   └─→ Set Text (Network_Description)
│           │
│           ├─→ Get Network Verse Score (VerseSubsystem)
│           │   │
│           │   ├─→ Divide by 100.0 (normalize to 0-1)
│           │   │   └─→ Set Percent (Reputation_Bar)
│           │   │
│           │   └─→ Format Text: "Reputation: {0}"
│           │       └─→ Set Text (Reputation_Label)
│           │
│           ├─→ Qualifies For Network Bonuses (VerseSubsystem)
│           │   └─→ Branch
│           │       ├─ True → Set Is Enabled (View_Benefits_Button) = True
│           │       └─ False → Set Is Enabled (View_Benefits_Button) = False
│           │
│           └─→ Get Members
│               └─→ ForEachLoop
│                   │
│                   └─→ Create Widget (WBP_MemberListItem)
│                       │
│                       ├─→ Set Member Way Name
│                       ├─→ Set Influence Level
│                       ├─→ Set Commitment Level
│                       │
│                       └─→ Add Child to Scroll Box (Member_List)
```

### Variables Needed

- **Network Asset** (WayNetworkDataAsset Reference)
  - Instance Editable: True
  - Category: "Configuration"
  
- **Verse System** (VerseSubsystem Reference)
  - Local variable in function

## Example 3: Award Feat with Network Effects

**Purpose**: Complete a quest and award Feat with automatic network spillover

**Location**: Quest Completion Function

### Blueprint Nodes

```
Custom Event: OnQuestCompleted
│
├─→ Get Game Instance
│   └─→ Get Subsystem (VerseSubsystem)
│       │
│       └─→ Variable: [Feat Asset to Award]
│           └─→ Record Feat With Network Effects
│               ├─ Feat To Record: [Your Feat Asset]
│               ├─ Apply Network Effects: TRUE
│               │
│               └─→ Branch
│                   ├─ Success:
│                   │  └─→ Show Notification: "Feat Achieved!"
│                   │
│                   └─ Failure:
│                      └─→ Print String: "Feat already completed"
```

### With Notification Details

```
Record Feat With Network Effects (Success Branch)
│
├─→ Get Networks For Way ([Quest Giver Way])
│   └─→ ForEachLoop
│       │
│       └─→ Get Network Verse Score
│           └─→ Branch (Score > 25)
│               │
│               └─ True:
│                  └─→ Show UI Notification
│                      ├─ Title: "Network Reputation Increased"
│                      ├─ Message: "[Network Name] approves of your actions"
│                      └─ Icon: [Network Icon]
```

## Example 4: Network-Based Discount System

**Purpose**: Apply trading discounts for network members

**Location**: Trading Post Blueprint → On Player Interact

### Blueprint Nodes

```
Event OnPlayerInteract
│
├─→ Get Station Faction Way
│   │
│   └─→ Get Game Instance
│       └─→ Get Subsystem (VerseSubsystem)
│           │
│           └─→ Get Networks For Way (Station Faction)
│               │
│               └─→ ForEachLoop
│                   │
│                   └─→ Qualifies For Network Bonuses
│                       └─→ Branch
│                           │
│                           ├─ True:
│                           │  │
│                           │  ├─→ Set Variable: Base Discount = 10%
│                           │  │
│                           │  ├─→ Get Network Verse Score
│                           │  │   └─→ Divide by 1000 (bonus discount)
│                           │  │       └─→ Add to Base Discount
│                           │  │           └─→ Clamp (0, 25)
│                           │  │               └─→ Apply Discount Multiplier
│                           │  │
│                           │  └─→ Show UI: "Network Member Benefits Active"
│                           │
│                           └─ False:
│                              └─→ Normal Prices
```

### Discount Calculation Details

```
Calculate Network Discount Function
│
Input: Network (WayNetworkDataAsset)
Output: Discount Percent (Float)
│
├─→ Get Network Verse Score (VerseSubsystem)
│   │
│   ├─→ Base Discount = 10.0
│   │
│   ├─→ Bonus Discount = (Score / 10.0)
│   │   └─→ Clamp (0, 15)
│   │
│   └─→ Total Discount = Base + Bonus
│       └─→ Clamp (10, 25)
│           └─→ Return Value
```

## Example 5: Network Quest Generator

**Purpose**: Generate quests that benefit entire network

**Location**: Quest Manager Blueprint

### Blueprint Nodes

```
Function: Generate Network Quest
│
Input: Quest Giver Way (WayDataAsset)
Output: Quest Data (Struct)
│
├─→ Get Game Instance
│   └─→ Get Subsystem (VerseSubsystem)
│       │
│       └─→ Get Networks For Way (Quest Giver)
│           │
│           ├─→ Array Length > 0
│           │   └─→ Branch
│           │       │
│           │       ├─ True (In Network):
│           │       │  │
│           │       │  ├─→ Select Random Network
│           │       │  │   │
│           │       │  │   ├─→ Create Quest: "Network Defense"
│           │       │  │   │   ├─ Objective: Defend all member stations
│           │       │  │   │   ├─ Reward: Rep with ALL members
│           │       │  │   │   └─ Bonus: Network-exclusive item
│           │       │  │   │
│           │       │  │   └─→ OR Create Quest: "Joint Research"
│           │       │  │       ├─ Objective: Gather resources for network
│           │       │  │       ├─ Reward: Rep + Credits
│           │       │  │       └─ Bonus: Unlocks network technology
│           │       │  │
│           │       │  └─→ Return Quest Data
│           │       │
│           │       └─ False (Solo):
│           │          └─→ Create Standard Quest
│           │              └─→ Return Quest Data
│           │
│           └─→ Return Value
```

### Network Quest Types

```
Select Network Quest Type
│
├─→ Random Integer (1-5)
│   │
│   ├─ Case 1: Network Defense
│   │  ├─ Description: "Defend network members from pirate attack"
│   │  ├─ Objectives: Protect X member stations
│   │  └─ Reward: High rep with all members + credits
│   │
│   ├─ Case 2: Joint Research
│   │  ├─ Description: "Collect samples for network research project"
│   │  ├─ Objectives: Gather Y resources from Z locations
│   │  └─ Reward: Rep with all members + special module
│   │
│   ├─ Case 3: Diplomatic Mission
│   │  ├─ Description: "Negotiate on behalf of the network"
│   │  ├─ Objectives: Complete dialogue challenges
│   │  └─ Reward: Network rep + diplomatic access
│   │
│   ├─ Case 4: Supply Run
│   │  ├─ Description: "Deliver supplies to all network members"
│   │  ├─ Objectives: Visit all member stations with cargo
│   │  └─ Reward: Network rep + trading discount voucher
│   │
│   └─ Case 5: Network Expansion
│      ├─ Description: "Help recruit new member to network"
│      ├─ Objectives: Complete favor for potential member
│      └─ Reward: Network rep + possible new member
```

## Example 6: Network Rivalry System

**Purpose**: Create competing networks with player choice

**Location**: Story Event Blueprint

### Blueprint Nodes

```
Event: Player Must Choose Network
│
Input: Network A, Network B (both WayNetworkDataAssets)
│
├─→ Show UI: Network Choice Dialog
│   │
│   ├─→ Display Network A Info
│   │   ├─ Name, Description, Members
│   │   ├─ Shared Precepts
│   │   └─ Benefits if chosen
│   │
│   ├─→ Display Network B Info
│   │   └─ [Same as above]
│   │
│   └─→ Wait for Player Choice
│       │
│       ├─→ Button A Clicked:
│       │   │
│       │   ├─→ Increase Rep with Network A (+ 50 all members)
│       │   ├─→ Decrease Rep with Network B (- 30 all members)
│       │   ├─→ Set Quest Flag: "Supports_Network_A"
│       │   └─→ Show Consequence Dialog
│       │
│       └─→ Button B Clicked:
│           └─→ [Mirror of Button A logic]
```

### Consequence Tracking

```
Function: Apply Network Choice Consequences
│
Input: Chosen Network, Rival Network
│
├─→ Get Game Instance
│   └─→ Get Subsystem (VerseSubsystem)
│       │
│       ├─→ Get Members (Chosen Network)
│       │   └─→ ForEachLoop
│       │       └─→ [Store member Ways to "Allied" list]
│       │
│       ├─→ Get Members (Rival Network)
│       │   └─→ ForEachLoop
│       │       └─→ [Store member Ways to "Hostile" list]
│       │
│       └─→ Update Quest System
│           ├─ Enable quests from Allied members
│           ├─ Disable quests from Hostile members
│           └─ Trigger rivalry events
```

## Example 7: Dynamic Network Formation

**Purpose**: Form new networks based on player actions

**Location**: Reputation Manager Blueprint

### Blueprint Nodes

```
Function: Check For Dynamic Network Formation
│
Called: After any reputation change
│
├─→ Get All Ways Player Has Rep With
│   └─→ Filter: Rep > 50
│       │
│       └─→ ForEachLoop (Combination Pairs)
│           │
│           └─→ Check Shared Precepts
│               │
│               ├─→ Count Common Precepts >= 2
│               │   └─→ Branch
│               │       │
│               │       └─ True:
│               │          │
│               │          ├─→ Check: Network Already Exists?
│               │          │   └─→ Branch
│               │          │       │
│               │          │       └─ False:
│               │          │          │
│               │          │          ├─→ Create Network Asset (Runtime)
│               │          │          │   ├─ Add both Ways as members
│               │          │          │   ├─ Set Shared Precepts
│               │          │          │   └─ Set default mechanics
│               │          │          │
│               │          │          ├─→ Register Network (VerseSubsystem)
│               │          │          │
│               │          │          └─→ Show Event: "New Alliance Formed!"
│               │          │              └─→ Display network info to player
│               │
│               └─→ Continue loop
```

## Example 8: Network Benefits UI Panel

**Purpose**: Show all active network benefits

**Location**: Player Menu Widget (WBP_NetworkBenefits)

### Widget Graph

```
Event Construct
│
├─→ Get Game Instance
│   └─→ Get Subsystem (VerseSubsystem)
│       │
│       └─→ Get All Registered Networks (custom function)
│           │
│           └─→ ForEachLoop
│               │
│               └─→ Qualifies For Network Bonuses
│                   └─→ Branch
│                       │
│                       └─ True:
│                          │
│                          └─→ Create Widget (WBP_NetworkBenefitCard)
│                              │
│                              ├─→ Set Network Name
│                              ├─→ Set Network Icon
│                              │
│                              ├─→ Calculate Active Benefits:
│                              │   ├─ Trading discount %
│                              │   ├─ Reputation bonus multiplier
│                              │   ├─ Access to exclusive items
│                              │   └─ Available network quests
│                              │
│                              └─→ Add to Benefits Panel
```

### Benefit Card Structure

```
WBP_NetworkBenefitCard (Widget Component)
│
├─ Image: Network Icon
├─ Text: Network Name
├─ Text: "Active Benefits:"
└─ Vertical Box:
   ├─ Text: "• Trading Discount: X%"
   ├─ Text: "• Reputation Bonus: X.Xx"
   ├─ Text: "• Exclusive Access: [items]"
   └─ Text: "• Network Quests: X available"
```

## Helper Functions

### Get All Networks For Player

```
Function: Get Player Active Networks
│
Output: Array of WayNetworkDataAssets
│
├─→ Get Game Instance
│   └─→ Get Subsystem (VerseSubsystem)
│       │
│       └─→ Get All Registered Networks (iterate internally)
│           │
│           └─→ ForEachLoop
│               │
│               └─→ Qualifies For Network Bonuses
│                   └─→ Branch
│                       │
│                       ├─ True → Add to Output Array
│                       └─ False → Skip
│
└─→ Return Output Array
```

### Calculate Total Network Influence

```
Function: Calculate Player Network Power
│
Output: Float (0-100)
│
├─→ Get Player Active Networks
│   │
│   └─→ ForEachLoop
│       │
│       ├─→ Get Network Verse Score
│       │   └─→ Add to Running Total
│       │
│       └─→ Get Member Count
│           └─→ Multiply total by member count
│
└─→ Divide by 100 → Clamp (0, 100) → Return
```

## Testing Blueprints

### Test Harness Blueprint

```
Actor: BP_NetworkTestHarness
│
├─→ Event BeginPlay
│   │
│   ├─→ Register Test Networks
│   │
│   ├─→ Create Test Player Reputation
│   │   ├─ Way A: 40 rep
│   │   ├─ Way B: 60 rep
│   │   └─ Way C: 20 rep
│   │
│   └─→ Run Test Sequence
│
├─→ Custom Event: Test Spillover
│   └─→ Award Feat
│       └─→ Log reputation changes
│
├─→ Custom Event: Test Bonuses
│   └─→ Check qualifications
│       └─→ Log which bonuses active
│
└─→ Custom Event: Test Network Formation
    └─→ Try dynamic network creation
        └─→ Log success/failure
```

## Common Patterns

### Pattern: Check Before Action

```
Before granting network benefit:
│
├─→ Is Network Active?
├─→ Does Player Qualify?
├─→ Are prerequisites met?
│
└─→ Branch → Grant or Deny
```

### Pattern: Safe Network Access

```
Get Network Asset:
│
├─→ Is Valid?
│   └─→ Branch
│       ├─ True → Use Network
│       └─ False → Log Warning, Use Fallback
```

### Pattern: Multi-Network Iteration

```
For all player's networks:
│
└─→ ForEachLoop
    │
    ├─→ Is Valid and Active?
    ├─→ Process Network
    └─→ Continue
```

## Summary

Key Blueprint functions:
- **Register Network**: On game start
- **Get Networks For Way**: Find Way's networks
- **Get Network Verse Score**: Player's network rep
- **Qualifies For Network Bonuses**: Check threshold
- **Record Feat With Network Effects**: Auto spillover

Create these Blueprints first:
1. Network Manager (registers all networks)
2. Network Info Widget (displays rep)
3. Network Benefits UI (shows bonuses)
4. Quest Generator (network quests)

All examples are copy-paste ready and require no C++ knowledge!
