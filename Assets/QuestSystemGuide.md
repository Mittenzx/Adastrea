# Quest System Guide

## Overview

The Adastrea Quest System provides a comprehensive framework for mission generation, tracking, and completion. It supports both hand-crafted story quests and procedurally generated missions with full integration into the faction and reputation systems.

## System Status: ✅ COMPLETE

The Quest System is production-ready with the following features:
- **QuestDataAsset** - Data-driven quest definitions
- **QuestManagerSubsystem** - Quest state tracking and progression
- **Multiple Quest Types** - Delivery, Combat, Exploration, and more
- **Objective Tracking** - Automatic progress updates
- **Reward System** - Credits, reputation, items, XP
- **Quest Chains** - Sequential quest progression

## Core Components

### QuestDataAsset

Primary Blueprint Data Asset for defining quests.

**Quest Types:**
- **Delivery** - Transport cargo/passengers
- **Exploration** - Discover locations
- **Combat** - Destroy targets
- **Escort** - Protect ship/convoy
- **Mining** - Collect resources
- **Research** - Scan/analyze objects
- **Diplomatic** - Negotiate/meet NPCs
- **Rescue** - Save ships/personnel
- **Sabotage** - Infiltrate/disable
- **Custom** - Custom objectives

### QuestManagerSubsystem

Game Instance Subsystem that manages all quest state.

**Key Features:**
- Accept/abandon/complete quest management
- Objective progress tracking
- Automatic notification system
- Quest persistence across level loads
- Event broadcasts for UI integration
- Procedural quest generation

## Creating Quests

### 1. Hand-Crafted Quests

**Step 1: Create Quest Data Asset**
1. Right-click in Content Browser
2. Blueprint Class → QuestDataAsset
3. Name it `DA_Quest_YourQuestName`

**Step 2: Configure Quest**
```cpp
// Quest Info
QuestName = "Emergency Delivery"
Description = "Transport medical supplies to Frontier Station before time runs out"
QuestID = "quest_emergency_delivery_01"
QuestType = Delivery
QuestGiverWayID = "way_medical_guild"
Difficulty = 5
bIsMainQuest = false
bIsRepeatable = false
TimeLimit = 600.0  // 10 minutes
```

**Step 3: Add Objectives**
```cpp
// Objective 1: Collect supplies
FQuestObjective Obj1;
Obj1.Type = CollectItems;
Obj1.Description = "Collect medical supplies from cargo depot";
Obj1.ItemID = "item_medical_supplies";
Obj1.RequiredQuantity = 10;
Obj1.bIsOptional = false;

// Objective 2: Deliver supplies
FQuestObjective Obj2;
Obj2.Type = ReachLocation;
Obj2.Description = "Deliver supplies to Frontier Station";
Obj2.TargetLocation = FVector(50000, 30000, 1000);
Obj2.RequiredQuantity = 1;
Obj2.bIsOptional = false;
```

**Step 4: Configure Rewards**
```cpp
// Main Rewards
Rewards.Credits = 5000;
Rewards.WayID = "way_medical_guild";
Rewards.ReputationGain = 25;
Rewards.ExperiencePoints = 500;

// Bonus Rewards (if all objectives complete)
BonusRewards.Credits = 2000;
BonusRewards.ReputationGain = 10;
```

### 2. Procedurally Generated Quests

**Generate Quest at Runtime:**
```cpp
UQuestManagerSubsystem* QuestManager = GetGameInstance()->GetSubsystem<UQuestManagerSubsystem>();

// Generate random delivery quest
UQuestDataAsset* GeneratedQuest = QuestManager->GenerateRandomQuest(
    EQuestType::Delivery,  // Quest type
    5,                      // Difficulty (1-10)
    "way_trade_guild"      // Quest giver
);

// Accept the generated quest
if (GeneratedQuest)
{
    QuestManager->AcceptQuest(GeneratedQuest);
}
```

## Quest Management

### Accepting Quests

```cpp
UQuestManagerSubsystem* QuestManager = GetGameInstance()->GetSubsystem<UQuestManagerSubsystem>();
UQuestDataAsset* Quest = LoadObject<UQuestDataAsset>(...);

if (QuestManager->AcceptQuest(Quest))
{
    // Quest accepted successfully
    UE_LOG(LogTemp, Log, TEXT("Quest '%s' accepted"), *Quest->QuestName.ToString());
}
else
{
    // Failed - check prerequisites or already active
}
```

### Abandoning Quests

```cpp
FName QuestID = "quest_emergency_delivery_01";

if (QuestManager->AbandonQuest(QuestID))
{
    // Quest abandoned
}
```

### Completing Quests

```cpp
// Complete quest manually (after all objectives done)
if (QuestManager->CompleteQuest(QuestID))
{
    // Quest completed, rewards given
}
```

### Failing Quests

```cpp
FString FailureReason = "Player ship was destroyed";
QuestManager->FailQuest(QuestID, FailureReason);
```

## Objective Tracking

### Automatic Notifications

The system automatically updates objectives when events occur:

**Location Reached:**
```cpp
// Called when player reaches a location
FVector PlayerLocation = GetActorLocation();
QuestManager->NotifyLocationReached(PlayerLocation);

// Automatically checks all active quests for ReachLocation objectives
// within 500 meters and completes them
```

**Item Collected:**
```cpp
// Called when player picks up items
FName ItemID = "item_medical_supplies";
int32 Quantity = 5;
QuestManager->NotifyItemCollected(ItemID, Quantity);

// Updates all active quests with CollectItems/DeliverItem objectives
```

**Target Destroyed:**
```cpp
// Called when player destroys an enemy
TSubclassOf<AActor> EnemyClass = AEnemyShip::StaticClass();
QuestManager->NotifyTargetDestroyed(EnemyClass);

// Updates all active quests with DestroyTarget objectives
```

### Manual Progress Update

```cpp
FName QuestID = "quest_mining_operation";
int32 ObjectiveIndex = 0;  // First objective
int32 NewProgress = 50;    // Current amount collected

QuestManager->UpdateObjectiveProgress(QuestID, ObjectiveIndex, NewProgress);
```

### Complete Specific Objective

```cpp
// Mark objective as complete immediately
QuestManager->CompleteObjective(QuestID, ObjectiveIndex);
```

## Querying Quest State

### Get Active Quests

```cpp
TArray<FActiveQuest> ActiveQuests = QuestManager->GetActiveQuests();

for (const FActiveQuest& Quest : ActiveQuests)
{
    UE_LOG(LogTemp, Log, TEXT("Quest: %s"), *Quest.Quest->QuestName.ToString());
    UE_LOG(LogTemp, Log, TEXT("Status: %d"), (int32)Quest.Status);
    UE_LOG(LogTemp, Log, TEXT("Time Remaining: %.1f"), Quest.TimeRemaining);
}
```

### Get Specific Quest

```cpp
FActiveQuest QuestData;
if (QuestManager->GetQuestByID(QuestID, QuestData))
{
    // Quest found
    float Completion = QuestData.Quest->GetCompletionPercentage();
    UE_LOG(LogTemp, Log, TEXT("Quest is %.0f%% complete"), Completion);
}
```

### Check Quest Status

```cpp
// Is quest active?
bool bIsActive = QuestManager->IsQuestActive(QuestID);

// Has quest been completed before?
bool bCompleted = QuestManager->HasCompletedQuest(QuestID);

// Get counts
int32 ActiveCount = QuestManager->GetActiveQuestCount();
int32 CompletedCount = QuestManager->GetCompletedQuestCount();
```

### Filter Quests

**By Type:**
```cpp
TArray<FActiveQuest> CombatQuests = QuestManager->GetQuestsByType(EQuestType::Combat);
```

**By Quest Giver:**
```cpp
TArray<FActiveQuest> GuildQuests = QuestManager->GetQuestsFromGiver("way_trade_guild");
```

## Quest Events

### Subscribe to Events

```cpp
// In your UI widget or game mode
void UMyQuestUI::BindQuestEvents()
{
    UQuestManagerSubsystem* QuestManager = GetGameInstance()->GetSubsystem<UQuestManagerSubsystem>();
    
    // Quest accepted
    QuestManager->OnQuestAccepted.AddDynamic(this, &UMyQuestUI::HandleQuestAccepted);
    
    // Objective updated
    QuestManager->OnObjectiveUpdated.AddDynamic(this, &UMyQuestUI::HandleObjectiveUpdated);
    
    // Objective completed
    QuestManager->OnObjectiveCompleted.AddDynamic(this, &UMyQuestUI::HandleObjectiveCompleted);
    
    // Quest completed
    QuestManager->OnQuestCompleted.AddDynamic(this, &UMyQuestUI::HandleQuestCompleted);
    
    // Quest failed
    QuestManager->OnQuestFailed.AddDynamic(this, &UMyQuestUI::HandleQuestFailed);
}

void UMyQuestUI::HandleQuestAccepted(UQuestDataAsset* Quest)
{
    // Show quest accepted notification
    ShowNotification(FText::Format(
        FText::FromString("Quest Accepted: {0}"),
        Quest->QuestName
    ));
}

void UMyQuestUI::HandleObjectiveCompleted(FName QuestID, int32 ObjectiveIndex)
{
    // Update quest UI
    RefreshQuestDisplay();
}
```

## Objective Types

### ReachLocation
Navigate to specific coordinates.

```cpp
FQuestObjective Obj;
Obj.Type = EObjectiveType::ReachLocation;
Obj.Description = FText::FromString("Travel to coordinates");
Obj.TargetLocation = FVector(10000, 5000, 0);
Obj.RequiredQuantity = 1;
```

### DeliverItem
Transport items to location.

```cpp
FQuestObjective Obj;
Obj.Type = EObjectiveType::DeliverItem;
Obj.Description = FText::FromString("Deliver medical supplies");
Obj.ItemID = "item_medical_supplies";
Obj.RequiredQuantity = 10;
```

### DestroyTarget
Eliminate enemy ships/stations.

```cpp
FQuestObjective Obj;
Obj.Type = EObjectiveType::DestroyTarget;
Obj.Description = FText::FromString("Destroy pirate fighters");
Obj.TargetActorClass = APirateFighter::StaticClass();
Obj.RequiredQuantity = 5;
```

### CollectItems
Gather resources from environment.

```cpp
FQuestObjective Obj;
Obj.Type = EObjectiveType::CollectItems;
Obj.Description = FText::FromString("Mine iron ore");
Obj.ItemID = "resource_iron_ore";
Obj.RequiredQuantity = 100;
```

### ScanObject
Use sensors to analyze targets.

```cpp
FQuestObjective Obj;
Obj.Type = EObjectiveType::ScanObject;
Obj.Description = FText::FromString("Scan anomaly");
Obj.TargetActorClass = AAnomalyActor::StaticClass();
Obj.RequiredQuantity = 1;
```

### TalkToNPC
Interact with specific character.

```cpp
FQuestObjective Obj;
Obj.Type = EObjectiveType::TalkToNPC;
Obj.Description = FText::FromString("Speak with station master");
Obj.TargetActorClass = AStationMaster::StaticClass();
Obj.RequiredQuantity = 1;
```

### ProtectTarget
Keep target alive for duration.

```cpp
FQuestObjective Obj;
Obj.Type = EObjectiveType::ProtectTarget;
Obj.Description = FText::FromString("Defend cargo freighter");
Obj.TargetActorClass = ACargoFreighter::StaticClass();
Obj.RequiredQuantity = 1;  // Must survive
```

### SurviveTime
Stay alive for specified time.

```cpp
FQuestObjective Obj;
Obj.Type = EObjectiveType::SurviveTime;
Obj.Description = FText::FromString("Survive ambush");
Obj.RequiredQuantity = 300;  // 300 seconds = 5 minutes
```

## Prerequisites

### Required Quests

```cpp
FQuestPrerequisite Prereq;
Prereq.RequiredQuest = LoadObject<UQuestDataAsset>(...);
// Player must complete this quest first
```

### Reputation Requirements

```cpp
FQuestPrerequisite Prereq;
Prereq.RequiredWayID = "way_trade_guild";
Prereq.MinimumReputation = 50;  // Need 50+ rep with Trade Guild
```

### Level Requirements

```cpp
FQuestPrerequisite Prereq;
Prereq.MinimumPlayerLevel = 10;  // Player must be level 10+
```

## Quest Chains

### Sequential Quests

```cpp
// Quest 1
UQuestDataAsset* Quest1 = ...;
Quest1->NextQuestInChain = Quest2;

// Quest 2
UQuestDataAsset* Quest2 = ...;
Quest2->PreviousQuestInChain = Quest1;
Quest2->NextQuestInChain = Quest3;

// Quest 3
UQuestDataAsset* Quest3 = ...;
Quest3->PreviousQuestInChain = Quest2;
```

**Unlocking Chain Quests:**
When Quest1 is completed, Quest2 becomes available automatically.

## Reward System

### Credits

```cpp
Rewards.Credits = 5000;  // Give 5000 credits
```

### Reputation

```cpp
Rewards.WayID = "way_engineers_guild";
Rewards.ReputationGain = 25;  // +25 reputation
// Can be negative for losing reputation
```

### Items

```cpp
// Give multiple items
Rewards.ItemRewards.Add("item_rare_component", 1);
Rewards.ItemRewards.Add("item_credits_voucher", 5);
```

### Experience Points

```cpp
Rewards.ExperiencePoints = 1000;  // Give 1000 XP
```

### Bonus Rewards

Given when ALL objectives (including optional) are completed:

```cpp
BonusRewards.Credits = 2000;           // Extra 2000 credits
BonusRewards.ReputationGain = 10;      // Extra 10 reputation
BonusRewards.ItemRewards.Add("item_blueprint", 1);
```

## Integration Examples

### With Combat System

```cpp
// In combat component
void UCombatHealthComponent::OnShipDestroyed_Implementation(AController* Killer)
{
    // Notify quest system
    UQuestManagerSubsystem* QuestManager = GetGameInstance()->GetSubsystem<UQuestManagerSubsystem>();
    TSubclassOf<AActor> ShipClass = GetOwner()->GetClass();
    QuestManager->NotifyTargetDestroyed(ShipClass);
}
```

### With Navigation System

```cpp
// In navigation component
void UNavigationComponent::OnDestinationReached_Implementation()
{
    // Notify quest system
    UQuestManagerSubsystem* QuestManager = GetGameInstance()->GetSubsystem<UQuestManagerSubsystem>();
    FVector Location = GetOwner()->GetActorLocation();
    QuestManager->NotifyLocationReached(Location);
}
```

### With Way System (Guild Reputation)

```cpp
// Quest affects guild (Way) reputation
void GiveQuestRewards(const FQuestReward& Rewards)
{
    // Update Way reputation via Verse component
    UVerse* PlayerVerse = PlayerPawn->FindComponentByClass<UVerse>();
    if (PlayerVerse && !Rewards.WayID.IsEmpty())
    {
        // Award reputation with the quest-giving Way
        PlayerVerse->ModifyWayReputation(
            FName(Rewards.WayID),
            Rewards.ReputationGain
        );
        
        // Record as a Feat if significant achievement
        if (Rewards.ReputationGain >= 50)
        {
            // This will trigger network spillover if Way is in a network
            UVerseSubsystem* VerseSystem = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
            VerseSystem->RecordFeatWithNetworkEffects(
                PlayerVerse,
                FName(Rewards.WayID),
                FFeatData(/* feat details */),
                Rewards.ReputationGain
            );
        }
    }
}
```

**Note:** Quests are primarily given by Ways (specialized guilds), not political factions. For political/military missions, use faction-specific quest systems. See [Way System Guide](WaySystemGuide.md) for guild reputation mechanics.

## UI Integration

### Display Active Quests

```cpp
void UQuestLogWidget::RefreshQuestList()
{
    UQuestManagerSubsystem* QuestManager = GetGameInstance()->GetSubsystem<UQuestManagerSubsystem>();
    TArray<FActiveQuest> ActiveQuests = QuestManager->GetActiveQuests();
    
    QuestListView->ClearItems();
    
    for (const FActiveQuest& Quest : ActiveQuests)
    {
        UQuestEntryWidget* Entry = CreateWidget<UQuestEntryWidget>(this, QuestEntryClass);
        Entry->SetQuestData(Quest);
        QuestListView->AddItem(Entry);
    }
}
```

### Show Quest Progress

```cpp
void UQuestDetailsWidget::DisplayQuest(FName QuestID)
{
    FActiveQuest QuestData;
    if (QuestManager->GetQuestByID(QuestID, QuestData))
    {
        QuestTitle->SetText(QuestData.Quest->QuestName);
        QuestDescription->SetText(QuestData.Quest->Description);
        
        // Show objectives
        for (int32 i = 0; i < QuestData.ObjectiveStates.Num(); ++i)
        {
            const FQuestObjective& Obj = QuestData.ObjectiveStates[i];
            
            FString ProgressText = FString::Printf(
                TEXT("%s [%d/%d]"),
                *Obj.Description.ToString(),
                Obj.CurrentProgress,
                Obj.RequiredQuantity
            );
            
            AddObjectiveEntry(ProgressText, Obj.bIsCompleted);
        }
        
        // Show completion percentage
        float Completion = QuestData.Quest->GetCompletionPercentage();
        ProgressBar->SetPercent(Completion / 100.0f);
    }
}
```

## Procedural Quest Generation

### Basic Generation

The system includes basic procedural generation:

```cpp
UQuestDataAsset* Quest = QuestManager->GenerateRandomQuest(
    EQuestType::Combat,     // Type
    7,                      // Difficulty
    "way_security_guild"   // Giver
);
```

### Custom Generation

Extend generation with custom logic:

```cpp
UQuestDataAsset* GenerateCustomQuest(EQuestType Type, int32 Difficulty)
{
    UQuestDataAsset* Quest = NewObject<UQuestDataAsset>();
    
    // Set base properties
    Quest->QuestType = Type;
    Quest->Difficulty = Difficulty;
    Quest->bIsRepeatable = true;
    
    // Generate based on type
    switch (Type)
    {
    case EQuestType::Delivery:
        GenerateDeliveryObjectives(Quest, Difficulty);
        break;
    case EQuestType::Combat:
        GenerateCombatObjectives(Quest, Difficulty);
        break;
    // ... more types
    }
    
    // Scale rewards by difficulty
    Quest->Rewards.Credits = Difficulty * 1000;
    Quest->Rewards.ExperiencePoints = Difficulty * 100;
    
    return Quest;
}
```

## Best Practices

### Quest Design

1. **Clear Objectives:**
   - Write clear, specific objective descriptions
   - Use descriptive waypoint names
   - Provide reasonable quantity requirements

2. **Balanced Rewards:**
   - Scale rewards with difficulty
   - Consider time investment
   - Reputation should match faction alignment

3. **Optional Objectives:**
   - Use for bonus challenges
   - Don't make critical to completion
   - Reward generously for 100% completion

4. **Time Limits:**
   - Only use when thematically appropriate
   - Provide reasonable time buffer
   - Consider player skill levels

### Technical Practices

1. **Unique Quest IDs:**
   - Use descriptive, unique identifiers
   - Follow naming convention: `quest_category_name_number`

2. **Performance:**
   - Limit active quests (10-15 max recommended)
   - Don't check objectives every frame
   - Use event-driven updates

3. **Save Data:**
   - Quest state persists in QuestManagerSubsystem
   - Integrate with game save system
   - Save completed quest IDs

## Troubleshooting

### Quest Won't Accept

**Check:**
- Prerequisites met?
- Quest already active?
- QuestDataAsset valid?

### Objectives Not Updating

**Check:**
- Using correct notification functions?
- Quest ID matches?
- Objective type matches notification type?

### Rewards Not Given

**Check:**
- Called CompleteQuest() function?
- All required objectives complete?
- Reward implementation in GiveQuestRewards()?

## Future Enhancements

Potential additions:

1. **Dynamic Objectives:**
   - Objectives that change based on player actions
   - Branching quest paths

2. **Quest Voting:**
   - Faction members vote on which quests to offer
   - Dynamic quest pool based on sector state

3. **Cooperative Quests:**
   - Multi-player quest objectives
   - Shared progress tracking

4. **Quest Generator Templates:**
   - More sophisticated procedural generation
   - Context-aware quest creation

5. **Quest Modifiers:**
   - Difficulty modifiers (hard mode)
   - Time attack variants
   - Permadeath challenge quests

## See Also

- `QuestDataAsset.h` - Quest data structure
- `QuestManagerSubsystem.h` - Quest management system
- `Way.h` - Faction integration
- `FeatDataAsset.h` - Achievement system
- `CONTENT_CREATION_QUICKSTART.md` - Content creation guide

---

**Last Updated**: 2025-11-11  
**Version**: 1.0  
**Status**: Production Ready  
**Components**: QuestDataAsset, QuestManagerSubsystem  
**Lines of Code**: ~1,100 lines (headers + implementations)
