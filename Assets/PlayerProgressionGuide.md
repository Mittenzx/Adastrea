# Player Progression System - Complete Guide

> **Version**: 1.0.0  
> **Last Updated**: November 2025  
> **System**: Player Progression & Advancement

## Table of Contents

- [Overview](#overview)
- [PlayerProgressionComponent](#playerprogressioncomponent)
- [PlayerUnlockComponent](#playerunlockcomponent)
- [Achievement System](#achievement-system)
- [Ship Upgrade System](#ship-upgrade-system)
- [Integration Guide](#integration-guide)
- [Blueprint Examples](#blueprint-examples)
- [Best Practices](#best-practices)

---

## Overview

The Player Progression system provides a comprehensive framework for character advancement, unlockable content, achievements, and ship upgrades. All systems follow Adastrea's data-driven design philosophy with full Blueprint exposure.

### Core Components

1. **PlayerProgressionComponent** - Experience, leveling, and skill trees
2. **PlayerUnlockComponent** - Content unlocking system
3. **AchievementDataAsset** - Achievement definitions
4. **AchievementManagerSubsystem** - Achievement tracking and rewards
5. **ShipUpgradeDataAsset** - Ship upgrade definitions
6. **ShipUpgradeComponent** - Upgrade installation and stat bonuses

### Design Principles

- **Data-Driven**: All content configurable via Data Assets
- **Designer-Friendly**: Full Blueprint integration
- **Modular**: Components work independently or together
- **Extensible**: Easy to add new progression systems
- **Balanced**: Configurable scaling and costs

---

## PlayerProgressionComponent

### Features

- **Experience System**: Award XP for any player action
- **Level Progression**: Levels 1-100 with configurable scaling
- **Skill Trees**: 7 skill categories with 10 ranks each
- **Skill Points**: Earned on level up, invested in skills
- **Stat Bonuses**: Skills provide 5% bonus per rank (up to 50%)

### Skill Categories

1. **Combat** - Weapon damage, accuracy, critical hits
2. **Piloting** - Ship handling, speed, maneuverability
3. **Engineering** - Repairs, power efficiency, system upgrades
4. **Trading** - Better prices, larger cargo, market access
5. **Exploration** - Scanning range, discovery rewards, fuel efficiency
6. **Diplomacy** - Faction reputation gains, negotiation bonuses
7. **Leadership** - Crew effectiveness, morale, command radius

### Setup

#### 1. Add Component to Player Pawn

```cpp
// C++ Example
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
UPlayerProgressionComponent* ProgressionComponent;

// In Constructor
ProgressionComponent = CreateDefaultSubobject<UPlayerProgressionComponent>(TEXT("ProgressionComponent"));
```

Or in Blueprint:
- Open player pawn Blueprint
- Add Component → Player Progression Component
- Configure default settings in Details panel

#### 2. Configure Settings

Key properties to set:
- **BaseXPRequirement**: Starting XP for level 2 (default: 1000)
- **XPScalingFactor**: Exponential scaling per level (default: 1.15)
- **SkillPointsPerLevel**: Points awarded per level (default: 1)
- **MaxLevel**: Maximum player level (default: 100)

### Blueprint Usage

#### Award Experience

```blueprint
// Award XP for quest completion
Get Player Progression Component
└─> Award Experience
    ├─ XP Amount: 500
    └─ Show Notification: True
```

#### Check for Level Up

```blueprint
// Automatic level up on XP threshold
Get Player Progression Component
├─> Can Level Up (return: Boolean)
└─> Level Up (return: Boolean)
```

#### Invest Skill Points

```blueprint
// Player invests in Combat skill
Get Player Progression Component
└─> Invest Skill Points
    ├─ Category: Combat
    └─ Points: 1
```

#### Query Skill Bonuses

```blueprint
// Get combat skill bonus
Get Player Progression Component
└─> Get Skill Bonus
    ├─ Category: Combat
    └─ Return: 1.25 (25% bonus at Rank 5)
```

### Events

Listen to progression events in Blueprint:

```blueprint
// Event: On Level Up
Event On Level Up
├─ New Level: Integer
├─ Skill Points Gained: Integer
└─> [Show Level Up UI]

// Event: On Experience Awarded
Event On Experience Awarded
├─ XP Amount: Integer
├─ Total XP: Integer
└─> [Show XP Notification]

// Event: On Skill Invested
Event On Skill Invested
├─ Category: Skill Category
├─ New Rank: Integer
├─ Points Remaining: Integer
└─> [Update Skill Tree UI]
```

### C++ Usage

```cpp
// Award XP
UPlayerProgressionComponent* ProgressionComp = Player->FindComponentByClass<UPlayerProgressionComponent>();
if (ProgressionComp)
{
    ProgressionComp->AwardExperience(500, true);
}

// Check skill bonus
float CombatBonus = ProgressionComp->GetSkillBonus(ESkillCategory::Combat);
float Damage = BaseDamage * CombatBonus; // Apply 5% per rank
```

### XP Scaling Formula

```
XP_Required(Level) = BaseXPRequirement * (ScalingFactor ^ (Level - 2))

Example with defaults:
Level 2: 1000 XP
Level 3: 1150 XP (1000 * 1.15^1)
Level 4: 1322 XP (1000 * 1.15^2)
Level 10: 3518 XP
Level 50: 1,091,736 XP
Level 100: Very high XP requirement
```

---

## PlayerUnlockComponent

### Features

- **Content Unlocking**: Ships, modules, weapons, equipment, features
- **Flexible Requirements**: Level, achievements, quests, reputation, credits, items
- **Progress Tracking**: View unlock progress and requirements
- **Unlock Trees**: Prerequisites create unlock chains

### Unlock Types

1. **Ship** - New spaceship classes
2. **Module** - Station/ship modules
3. **Weapon** - Weapon systems
4. **Equipment** - Special equipment
5. **Blueprint** - Crafting blueprints
6. **Feature** - Game features/abilities

### Requirement Types

1. **Level** - Player must reach specific level
2. **Achievement** - Must complete achievement
3. **Quest** - Must complete specific quest
4. **Reputation** - Must have faction reputation
5. **Credits** - Must pay credit cost
6. **Item** - Must possess specific item

### Setup

#### Create Unlock Entry

```cpp
// In Blueprint or C++
FUnlockEntry NewUnlock;
NewUnlock.UnlockID = FName("Ship_Warhammer");
NewUnlock.Type = EUnlockType::Ship;
NewUnlock.DisplayName = FText::FromString("Warhammer-class Battleship");
NewUnlock.Description = FText::FromString("Powerful combat vessel");
NewUnlock.bUnlockedByDefault = false;

// Add requirements
FUnlockRequirement LevelReq;
LevelReq.Type = EUnlockRequirementType::Level;
LevelReq.RequiredValue = 10;
NewUnlock.Requirements.Add(LevelReq);

FUnlockRequirement CreditReq;
CreditReq.Type = EUnlockRequirementType::Credits;
CreditReq.RequiredValue = 100000;
NewUnlock.Requirements.Add(CreditReq);
```

### Blueprint Usage

#### Try to Unlock Content

```blueprint
// Attempt to unlock ship
Get Player Unlock Component
└─> Try Unlock
    ├─ Unlock ID: "Ship_Warhammer"
    ├─ Ignore Requirements: False
    └─ Return: Boolean (success)
```

#### Check Unlock Status

```blueprint
// Check if ship is unlocked
Get Player Unlock Component
└─> Is Unlocked
    ├─ Unlock ID: "Ship_Warhammer"
    └─ Return: Boolean
```

#### Get Unlock Progress

```blueprint
// Check unlock progress (0.0 to 1.0)
Get Player Unlock Component
└─> Get Unlock Progress
    ├─ Unlock ID: "Ship_Warhammer"
    └─ Return: Float (0.5 = 50% of requirements met)
```

#### Query Unlocked Content

```blueprint
// Get all unlocked ships
Get Player Unlock Component
└─> Get Unlocked By Type
    ├─ Type: Ship
    └─ Return: Array of FName (ship IDs)
```

### Events

```blueprint
// Event: Content Unlocked
Event On Content Unlocked
├─ Unlock ID: Name
├─ Type: Unlock Type
└─> [Show Unlock Notification]

// Event: Unlock Failed
Event On Unlock Failed
├─ Unlock ID: Name
├─ Reason: Text
└─> [Show Error Message]
```

---

## Achievement System

### Features

- **9 Categories**: Combat, Trading, Exploration, Diplomacy, Construction, Personnel, Progression, Story, Hidden
- **5 Rarity Tiers**: Common (10 pts), Uncommon (25 pts), Rare (50 pts), Epic (100 pts), Legendary (250 pts)
- **Progress Tracking**: Track towards achievement goals
- **Stat System**: Automatic tracking via gameplay stats
- **Rewards**: Credits, XP, unlocks, items, titles

### Creating Achievements

#### 1. Create Achievement Data Asset

In Content Browser:
- Right-click → Miscellaneous → Data Asset
- Select `AchievementDataAsset` as parent
- Name: `DA_Achievement_FirstBlood`

#### 2. Configure Achievement

```yaml
# Achievement Configuration
AchievementID: FirstBlood
DisplayName: "First Blood"
Description: "Destroy your first enemy ship"
Category: Combat
Rarity: Common
IsHidden: false

ProgressData:
  TargetProgress: 1
  
TrackingStatName: Ships_Destroyed
AutoTrackedByStat: true

Rewards:
  Credits: 1000
  ExperiencePoints: 500
  UnlockID: Weapon_PlasmaCannon
```

### Blueprint Usage

#### Register Achievements

```blueprint
// On Game Start
Get Achievement Manager Subsystem
└─> Register Achievements
    └─ Achievements: [Array of Achievement Data Assets]
```

#### Update Achievement Progress

```blueprint
// Manually update achievement
Get Achievement Manager Subsystem
└─> Update Achievement Progress
    ├─ Achievement ID: "FirstBlood"
    ├─ Progress Increment: 1
    └─ Auto Award: True
```

#### Update Stat (Auto-tracks achievements)

```blueprint
// Player destroys enemy ship
Get Achievement Manager Subsystem
└─> Update Achievement Stat
    ├─ Stat Name: "Ships_Destroyed"
    ├─ Stat Increment: 1
    └─ Auto Award Achievements: True
```

#### Query Achievements

```blueprint
// Get combat achievements
Get Achievement Manager Subsystem
└─> Get Achievements By Category
    ├─ Category: Combat
    └─ Return: Array of Achievement Trackers
```

### Common Stat Names

Use these consistent stat names for tracking:

- **Combat**: `Ships_Destroyed`, `Damage_Dealt`, `Kills_Total`
- **Trading**: `Trades_Completed`, `Credits_Earned`, `Cargo_Delivered`
- **Exploration**: `Systems_Discovered`, `Planets_Scanned`, `Distance_Traveled`
- **Diplomacy**: `Faction_Relationships_Improved`, `Treaties_Signed`
- **Construction**: `Stations_Built`, `Modules_Installed`

### Achievement Events

```blueprint
// Event: Achievement Unlocked
Event On Achievement Unlocked
├─ Achievement ID: Name
├─ Achievement: Achievement Data Asset
└─> [Show Achievement Popup]

// Event: Progress Updated
Event On Achievement Progress Updated
├─ Achievement ID: Name
├─ Current Progress: Integer
├─ Target Progress: Integer
└─> [Update Achievement UI]
```

---

## Ship Upgrade System

### Features

- **8 Categories**: Weapons, Engines, Shields, Hull, Systems, Cargo, Power, Sensors
- **5 Tiers**: Progressive upgrade levels
- **Stat Modifiers**: Additive and multiplicative bonuses
- **Upgrade Trees**: Prerequisites create tech trees
- **Stack Support**: Some upgrades can be installed multiple times
- **Compatibility**: Restrict upgrades to ship types

### Creating Upgrades

#### 1. Create Upgrade Data Asset

```yaml
# Enhanced Thrusters Mk2
UpgradeID: Upgrade_Engines_Thrusters_Mk2
DisplayName: "Enhanced Thrusters Mk2"
Description: "Improved engine performance"
Category: Engines
Tier: Tier2

StatModifiers:
  - StatName: MaxSpeed
    AdditiveBonus: 0
    MultiplicativeBonus: 1.2  # +20% speed
  - StatName: Acceleration
    AdditiveBonus: 50
    MultiplicativeBonus: 1.0

Requirements:
  CreditCost: 5000
  RequiredPlayerLevel: 5
  PrerequisiteUpgrades:
    - Upgrade_Engines_Thrusters_Mk1

CompatibleShipTypes: []  # Empty = all ships
IsUnique: true
MaxStackCount: 1
```

### Blueprint Usage

#### Install Upgrade

```blueprint
// Install upgrade on ship
Get Ship Upgrade Component
└─> Install Upgrade
    ├─ Upgrade: [Upgrade Data Asset]
    ├─ Ignore Requirements: False
    └─ Return: Boolean (success)
```

#### Check Stat Bonus

```blueprint
// Get modified speed
Get Ship Upgrade Component
└─> Get Stat Modifier
    ├─ Stat Name: "MaxSpeed"
    ├─ Base Value: 100.0
    └─ Return: 120.0 (+20% from upgrade)
```

#### Query Installed Upgrades

```blueprint
// Get all weapon upgrades
Get Ship Upgrade Component
└─> Get Upgrades By Category
    ├─ Category: Weapons
    └─> Return: Array of Installed Upgrades
```

### Stat Modifier System

Upgrades apply in this order:
1. **Additive Bonuses**: Sum all additive values
2. **Multiplicative Bonuses**: Multiply result by all multipliers

Example:
```
Base Speed: 100
Upgrade 1: Additive +10, Multiplicative 1.2
Upgrade 2: Additive +5, Multiplicative 1.1

Result = ((100 + 10 + 5) * 1.2) * 1.1 = 151.8
```

---

## Integration Guide

### Quest System Integration

```blueprint
// In Quest Completion Handler
Event On Quest Completed
├─ Quest: Quest Data Asset
└─> Branch
    ├─ Condition: Quest.Rewards.ExperiencePoints > 0
    └─ True:
        └─> Get Player Progression Component
            └─> Award Experience
                ├─ XP Amount: Quest.Rewards.ExperiencePoints
                └─ Show Notification: True
```

### Combat System Integration

```cpp
// Apply skill bonuses to combat damage
UPlayerProgressionComponent* ProgressionComp = Player->FindComponentByClass<UPlayerProgressionComponent>();
if (ProgressionComp)
{
    float CombatSkillBonus = ProgressionComp->GetSkillBonus(ESkillCategory::Combat);
    float FinalDamage = BaseDamage * CombatSkillBonus;
    
    // Apply damage with bonus
    Target->TakeDamage(FinalDamage);
    
    // Award XP for kill
    ProgressionComp->AwardExperience(50);
    
    // Track stat for achievements
    UAchievementManagerSubsystem* AchievementMgr = GetGameInstance()->GetSubsystem<UAchievementManagerSubsystem>();
    if (AchievementMgr)
    {
        AchievementMgr->UpdateAchievementStat(FName("Ships_Destroyed"), 1);
    }
}
```

### Trading System Integration

```cpp
// Apply trading skill to prices
UPlayerProgressionComponent* ProgressionComp = Player->FindComponentByClass<UPlayerProgressionComponent>();
float TradingSkillBonus = ProgressionComp->GetSkillBonus(ESkillCategory::Trading);

// Better prices with higher skill (inverse for buying)
float BuyPrice = BasePrice / TradingSkillBonus;
float SellPrice = BasePrice * TradingSkillBonus;
```

---

## Blueprint Examples

### Complete Level-Up Flow

```blueprint
// Player gains XP and levels up
Function Award Quest XP
├─ Parameters: Quest Data Asset
└─> Get Player Progression Component
    ├─> Award Experience
    │   ├─ XP Amount: Quest.Rewards.ExperiencePoints
    │   └─ Show Notification: True
    └─> Branch: Can Level Up?
        └─ True:
            ├─> Level Up
            ├─> Play Sound: Level Up Sound
            ├─> Show Widget: Level Up Screen
            └─> Check Unlocks
                └─> Get Player Unlock Component
                    └─> For Each Unlock Entry
                        └─> Branch: Meets Requirements?
                            └─ True: Try Unlock
```

### Skill Tree UI

```blueprint
// Display skill tree in UI
Function Update Skill Tree UI
└─> Get Player Progression Component
    ├─> Get All Skills
    │   └─> For Each Skill
    │       └─> Create Widget: Skill Button
    │           ├─ Category: Skill.Category
    │           ├─ Rank: Skill.Rank
    │           ├─ Bonus: Calculate Skill Bonus(Skill.Rank)
    │           └─ Can Invest: Can Invest In Skill?
    └─> Get Available Skill Points
        └─> Update Text: Points Remaining
```

---

## Best Practices

### XP Balance

- **Early Levels**: Fast progression (levels 1-10)
- **Mid Levels**: Moderate pace (levels 10-50)
- **Late Levels**: Slow grind (levels 50-100)
- **Quest XP**: 100-1000 per quest based on difficulty
- **Combat XP**: 10-100 per enemy based on threat level
- **Discovery XP**: 50-500 per discovery

### Skill Point Economy

- 1 skill point per level = 99 total points at max level
- 7 categories × 10 ranks = 70 points to max all
- Players can specialize or generalize
- Consider skill respec option (credit cost or limited uses)

### Unlock Progression

- **Level 1-10**: Basic ships and modules
- **Level 10-25**: Advanced equipment
- **Level 25-50**: Specialized ships and rare modules
- **Level 50-75**: Epic tier content
- **Level 75-100**: Legendary endgame content

### Achievement Design

- **Common**: Tutorial and basic gameplay (10-20% completion)
- **Uncommon**: Regular gameplay milestones (30-50% completion)
- **Rare**: Challenging objectives (5-15% completion)
- **Epic**: Very difficult tasks (1-5% completion)
- **Legendary**: Extreme challenges (< 1% completion)

### Ship Upgrade Balance

- **Tier 1**: 5-10% improvements, low cost
- **Tier 2**: 15-20% improvements, moderate cost
- **Tier 3**: 25-35% improvements, high cost
- **Tier 4**: 40-50% improvements, very high cost
- **Tier 5**: 60-75% improvements, extreme cost

### Performance Considerations

- Cache skill bonuses if calculated frequently
- Use events for UI updates, not Tick
- Batch achievement stat updates when possible
- Save progression data regularly (auto-save)

---

## Troubleshooting

### Player Not Leveling Up

- Check `CanLevelUp()` returns true
- Verify XP calculation is correct
- Ensure `MaxLevel` not reached
- Check logs for XP award messages

### Unlocks Not Working

- Verify requirement components exist (Progression, Reputation, etc.)
- Check `MeetsRequirements()` for failure reason
- Ensure UnlockID matches exactly
- Test with `bIgnoreRequirements = true`

### Achievements Not Triggering

- Confirm achievement is registered on game start
- Check stat name matches exactly (case-sensitive)
- Verify `bAutoTrackedByStat` is true
- Use `UpdateAchievementProgress()` for manual tracking

### Upgrades Not Applying

- Check ship type compatibility
- Verify no conflicting upgrades installed
- Ensure upgrade slots available
- Test stat modifier calculation

---

## Next Steps

1. **Create Content**: Design achievements, unlocks, upgrades
2. **Balance Testing**: Test XP scaling and progression pace
3. **UI Implementation**: Create progression menus and notifications
4. **Save Integration**: Test save/load of progression data
5. **Player Feedback**: Implement visual/audio feedback for progression events

## Related Documentation

- [SaveSystemGuide.md](SaveSystemGuide.md) - Persistence and save/load
- [AchievementSystemGuide.md](AchievementSystemGuide.md) - Detailed achievement documentation
- [ShipUpgradeGuide.md](ShipUpgradeGuide.md) - Ship upgrade system details
- [QuestSystemGuide.md](QuestSystemGuide.md) - Quest integration
- [SpaceshipDataAssetGuide.md](SpaceshipDataAssetGuide.md) - Ship system integration

---

**Last Updated**: November 2025  
**Version**: 1.0.0  
**Maintained by**: Adastrea Development Team
