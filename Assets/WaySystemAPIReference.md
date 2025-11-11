# Way System - API Reference

## Overview

This document provides a complete reference for the Way System C++ classes, Blueprint nodes, and integration patterns. All examples work in both C++ and Blueprint.

## Table of Contents

- [Core Classes](#core-classes)
- [VerseSubsystem](#versesubsystem)
- [WayDataAsset](#waydataasset)
- [WayNetworkDataAsset](#waynetworkdataasset)
- [Verse Component](#verse-component)
- [Structs and Enums](#structs-and-enums)
- [Blueprint Integration](#blueprint-integration)
- [Common Patterns](#common-patterns)

## Core Classes

### Class Hierarchy

```
UObject
├── UPrimaryDataAsset
│   ├── UWayDataAsset
│   └── UWayNetworkDataAsset
└── UGameInstanceSubsystem
    └── UVerseSubsystem

UActorComponent
└── UVerse (Player Component)
```

## VerseSubsystem

**File**: `Source/Adastrea/Way/VerseSubsystem.h`  
**Base Class**: `UGameInstanceSubsystem`  
**Purpose**: Manages Way Networks and reputation calculations globally

### Public Functions

#### RegisterNetwork

```cpp
UFUNCTION(BlueprintCallable, Category = "Verse|Network")
void RegisterNetwork(UWayNetworkDataAsset* Network);
```

**Description**: Registers a Way Network with the subsystem. Must be called before network can be used.

**Parameters**:
- `Network` - The network Data Asset to register

**Blueprint**: Yes  
**Thread-Safe**: No  
**Call Time**: Game initialization (BeginPlay)

**Example (C++)**:
```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UVerseSubsystem* VerseSystem = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
    if (VerseSystem && CraftsmenNetwork)
    {
        VerseSystem->RegisterNetwork(CraftsmenNetwork);
    }
}
```

**Example (Blueprint)**:
```
Event BeginPlay
  → Get Game Instance
  → Get Subsystem (VerseSubsystem)
  → Register Network (DA_Network_CraftsmenCompact)
```

---

#### UnregisterNetwork

```cpp
UFUNCTION(BlueprintCallable, Category = "Verse|Network")
void UnregisterNetwork(UWayNetworkDataAsset* Network);
```

**Description**: Removes a network from the subsystem. Use for dissolved or inactive networks.

**Parameters**:
- `Network` - The network to unregister

**Blueprint**: Yes  
**Thread-Safe**: No

---

#### GetNetworksForWay

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Verse|Network")
TArray<UWayNetworkDataAsset*> GetNetworksForWay(const UWayDataAsset* Way) const;
```

**Description**: Finds all networks that a specific Way belongs to.

**Parameters**:
- `Way` - The Way to search for

**Returns**: Array of networks containing this Way

**Blueprint**: Yes (Pure)  
**Thread-Safe**: Yes (const)

**Example (C++)**:
```cpp
UWayDataAsset* EngineersGuild = LoadObject<UWayDataAsset>(...);
TArray<UWayNetworkDataAsset*> Networks = VerseSystem->GetNetworksForWay(EngineersGuild);

for (UWayNetworkDataAsset* Network : Networks)
{
    UE_LOG(LogTemp, Log, TEXT("Engineers belong to: %s"), *Network->NetworkName.ToString());
}
```

**Example (Blueprint)**:
```
Get Networks For Way (Engineers Guild)
  → ForEachLoop
  → Print Network Name
```

---

#### GetNetworkVerseScore

```cpp
UFUNCTION(BlueprintCallable, Category = "Verse|Network")
float GetNetworkVerseScore(const UWayNetworkDataAsset* Network) const;
```

**Description**: Calculates player's total reputation with a network based on member reputations.

**Parameters**:
- `Network` - The network to calculate score for

**Returns**: Float (0-100+) representing network reputation

**Blueprint**: Yes  
**Thread-Safe**: No (queries player data)

**Calculation**: Averages player reputation across all network members, weighted by member influence.

**Example (C++)**:
```cpp
float NetworkRep = VerseSystem->GetNetworkVerseScore(CraftsmenNetwork);
if (NetworkRep >= 50.0f)
{
    // Player has good standing with network
}
```

**Example (Blueprint)**:
```
Get Network Verse Score (Craftsmen Compact)
  → Branch (>= 50)
  → True: Grant Network Benefits
```

---

#### QualifiesForNetworkBonuses

```cpp
UFUNCTION(BlueprintCallable, Category = "Verse|Network")
bool QualifiesForNetworkBonuses(const UWayNetworkDataAsset* Network) const;
```

**Description**: Checks if player meets minimum reputation threshold for network bonuses.

**Parameters**:
- `Network` - The network to check qualification for

**Returns**: `true` if player qualifies, `false` otherwise

**Blueprint**: Yes  
**Thread-Safe**: No

**Requirements**:
- Network must be registered and active
- Player must have `MinimumReputationThreshold` rep with at least one member
- Network must have `bIsActive = true`

**Example (C++)**:
```cpp
if (VerseSystem->QualifiesForNetworkBonuses(CraftsmenNetwork))
{
    ApplyNetworkDiscounts(Player, CraftsmenNetwork);
}
```

**Example (Blueprint)**:
```
Qualifies For Network Bonuses (Frontier Alliance)
  → Branch
  → True: Enable Exclusive Missions
  → False: Show "Improve Reputation" Message
```

---

#### RecordFeatWithNetworkEffects

```cpp
UFUNCTION(BlueprintCallable, Category = "Verse|Feat")
bool RecordFeatWithNetworkEffects(UFeatDataAsset* Feat, bool bApplyNetworkEffects);
```

**Description**: Records a Feat completion and automatically applies network spillover and bonuses.

**Parameters**:
- `Feat` - The Feat Data Asset that was completed
- `bApplyNetworkEffects` - If `true`, applies spillover and bonuses; if `false`, direct reputation only

**Returns**: `true` if Feat was recorded successfully

**Blueprint**: Yes  
**Thread-Safe**: No

**Behavior**:
1. Identifies Ways whose Precepts align with Feat
2. Awards direct reputation to matching Ways
3. If `bApplyNetworkEffects = true`:
   - Applies network alignment bonuses (e.g., 1.8x multiplier)
   - Distributes spillover reputation to network members
   - Triggers network achievement events

**Example (C++)**:
```cpp
void AQuestManager::OnQuestCompleted(UFeatDataAsset* QuestFeat)
{
    UVerseSubsystem* VerseSystem = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
    
    // Award Feat with full network effects
    bool bSuccess = VerseSystem->RecordFeatWithNetworkEffects(QuestFeat, true);
    
    if (bSuccess)
    {
        ShowFeatCompletionUI(QuestFeat);
    }
}
```

**Example (Blueprint)**:
```
On Quest Complete
  → Get VerseSubsystem
  → Record Feat With Network Effects
    - Feat: Quest Feat Asset
    - Apply Network Effects: TRUE
  → Branch (Success)
  → True: Show "Network Reputation Increased" Notification
```

---

#### GetAllRegisteredNetworks

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Verse|Network")
TArray<UWayNetworkDataAsset*> GetAllRegisteredNetworks() const;
```

**Description**: Returns all networks currently registered with the subsystem.

**Returns**: Array of all registered networks

**Blueprint**: Yes (Pure)  
**Thread-Safe**: Yes (const)

**Example (C++)**:
```cpp
TArray<UWayNetworkDataAsset*> AllNetworks = VerseSystem->GetAllRegisteredNetworks();
UE_LOG(LogTemp, Log, TEXT("Total networks: %d"), AllNetworks.Num());
```

---

## WayDataAsset

**File**: `Source/Adastrea/Way/Way.h`  
**Base Class**: `UPrimaryDataAsset`  
**Purpose**: Defines a specialized guild with industry, quality, and Precepts

### Key Properties

#### Identity

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Identity")
FText WayName;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Identity")
FName WayID;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Identity")
bool bIsSchool;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Identity", meta=(MultiLine=true))
FText Description;
```

**Description**:
- `WayName`: Display name shown in UI
- `WayID`: Unique identifier for code (no spaces)
- `bIsSchool`: `true` = knowledge/training focus, `false` = commerce/production
- `Description`: Lore and flavor text

---

#### Specialization

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Specialization")
EWayIndustry PrimaryIndustry;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Specialization")
TArray<EWayIndustry> SecondaryIndustries;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Specialization")
EQualityTier QualityReputation;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Specialization")
int32 MemberCount;
```

**Description**:
- `PrimaryIndustry`: Main specialization (Mining, Manufacturing, etc.)
- `SecondaryIndustries`: Additional focuses (optional)
- `QualityReputation`: Reputation tier (Basic, Standard, Quality, Premium, Elite, Legendary)
- `MemberCount`: Approximate number of guild members

---

#### Core Precepts

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Way|Philosophy")
TArray<FWayPrecept> CorePrecepts;
```

**Struct**: `FWayPrecept`
```cpp
struct FWayPrecept
{
    EPrecept Precept;                    // The value (Craftsmanship, Honor, etc.)
    int32 ImportanceValue;               // 0-100, how much Way values this
    FText Description;                   // Explanation of what it means to this Way
};
```

**Example**:
```cpp
FWayPrecept Craftsmanship;
Craftsmanship.Precept = EPrecept::Craftsmanship;
Craftsmanship.ImportanceValue = 90;
Craftsmanship.Description = FText::FromString("Excellence in all things built");
```

---

### Public Functions

#### GetSuppliers

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Way|Trade")
TArray<UWayDataAsset*> GetSuppliers() const;
```

**Description**: Returns all Ways that supply resources to this Way.

**Returns**: Array of supplier Ways

**Blueprint**: Yes (Pure)

---

#### GetCustomers

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Way|Trade")
TArray<UWayDataAsset*> GetCustomers() const;
```

**Description**: Returns all Ways that consume resources from this Way.

**Returns**: Array of customer Ways

**Blueprint**: Yes (Pure)

---

#### HasSupplyRelationship

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Way|Trade")
bool HasSupplyRelationship(const UWayDataAsset* OtherWay) const;
```

**Description**: Checks if this Way has any supply chain relationship with another Way.

**Parameters**:
- `OtherWay` - Way to check relationship with

**Returns**: `true` if supply relationship exists

**Blueprint**: Yes (Pure)

---

## WayNetworkDataAsset

**File**: `Source/Adastrea/Way/WayNetwork.h`  
**Base Class**: `UPrimaryDataAsset`  
**Purpose**: Defines a micro-alliance of 2-5 Ways with shared Precepts

### Key Properties

#### Identity

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Identity")
FText NetworkName;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Identity")
FName NetworkID;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Identity", meta=(MultiLine=true))
FText Description;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Identity")
FLinearColor NetworkColor;
```

---

#### Members

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Members")
TArray<FWayNetworkMember> Members;
```

**Struct**: `FWayNetworkMember`
```cpp
struct FWayNetworkMember
{
    UWayDataAsset* MemberWay;            // Reference to the Way
    int32 InfluenceLevel;                // 0-100, voting power in network
    int32 CommitmentLevel;               // 0-100, how strongly they react
    FText RoleDescription;               // Optional flavor text
};
```

**Example**:
```cpp
FWayNetworkMember Engineer;
Engineer.MemberWay = EngineersGuildAsset;
Engineer.InfluenceLevel = 70;
Engineer.CommitmentLevel = 90;
Engineer.RoleDescription = FText::FromString("Masters of propulsion systems");
```

---

#### Philosophy

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Philosophy")
TArray<FWayPrecept> SharedPrecepts;
```

**Description**: 2-4 Precepts that all members share and define the network's identity.

---

#### Mechanics

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Mechanics", meta=(ClampMin=0, ClampMax=100))
int32 ReputationSpilloverPercent;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Mechanics", meta=(ClampMin=1.0, ClampMax=3.0))
float NetworkAlignmentBonus;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Mechanics", meta=(ClampMin=0, ClampMax=100))
int32 MinimumReputationThreshold;
```

**Description**:
- `ReputationSpilloverPercent`: How much rep transfers to other members (30-50% typical)
- `NetworkAlignmentBonus`: Reputation multiplier for aligned Feats (1.5-2.0x typical)
- `MinimumReputationThreshold`: Rep needed with any member to qualify for bonuses (20-40 typical)

---

#### Status

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Status")
bool bIsActive;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Status")
FDateTime FormationDate;
```

---

### Public Functions

#### IsMember

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network")
bool IsMember(const UWayDataAsset* Way) const;
```

**Description**: Checks if a Way is a member of this network.

**Parameters**:
- `Way` - Way to check membership for

**Returns**: `true` if Way is a member

**Blueprint**: Yes (Pure)

---

#### GetMemberInfluence

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network")
int32 GetMemberInfluence(const UWayDataAsset* Way) const;
```

**Description**: Gets the influence level of a member Way.

**Parameters**:
- `Way` - Member Way

**Returns**: Influence level (0-100), or 0 if not a member

**Blueprint**: Yes (Pure)

---

#### GetMembers

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network")
TArray<UWayDataAsset*> GetMembers() const;
```

**Description**: Returns all member Ways in the network.

**Returns**: Array of member Ways

**Blueprint**: Yes (Pure)

---

## Verse Component

**File**: `Source/Adastrea/Player/Verse.h`  
**Base Class**: `UActorComponent`  
**Purpose**: Tracks player reputation with Ways and calculates Verse scores

### Key Properties

```cpp
UPROPERTY(BlueprintReadOnly, Category = "Verse")
TMap<UWayDataAsset*, int32> WayReputation;

UPROPERTY(BlueprintReadOnly, Category = "Verse")
TArray<UFeatDataAsset*> CompletedFeats;
```

### Public Functions

#### GetReputationWithWay

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Verse")
int32 GetReputationWithWay(const UWayDataAsset* Way) const;
```

**Description**: Gets player's current reputation with a specific Way.

**Parameters**:
- `Way` - Way to check reputation with

**Returns**: Reputation value (0-100+)

**Blueprint**: Yes (Pure)

---

#### AddReputation

```cpp
UFUNCTION(BlueprintCallable, Category = "Verse")
void AddReputation(UWayDataAsset* Way, int32 Amount);
```

**Description**: Adds reputation with a Way. Can be negative to decrease reputation.

**Parameters**:
- `Way` - Way to modify reputation with
- `Amount` - Amount to add (positive or negative)

**Blueprint**: Yes

---

#### HasCompletedFeat

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Verse")
bool HasCompletedFeat(const UFeatDataAsset* Feat) const;
```

**Description**: Checks if player has completed a specific Feat.

**Parameters**:
- `Feat` - Feat to check

**Returns**: `true` if completed

**Blueprint**: Yes (Pure)

---

#### CalculateVerseReputation

```cpp
UFUNCTION(BlueprintCallable, Category = "Verse")
int32 CalculateVerseReputation(UWayDataAsset* GroupWay) const;
```

**Description**: Calculates total Verse reputation for a Way based on Precept alignment.

**Parameters**:
- `GroupWay` - Way to calculate for

**Returns**: Verse score (0-100+)

**Blueprint**: Yes

---

## Structs and Enums

### EWayIndustry

```cpp
UENUM(BlueprintType)
enum class EWayIndustry : uint8
{
    Mining,
    Refining,
    Manufacturing,
    Shipbuilding,
    Trading,
    Exploration,
    Research,
    Agriculture,
    Medical,
    Security,
    Salvage,
    Entertainment,
    Transport,
    Engineering,
    Custom
};
```

---

### EQualityTier

```cpp
UENUM(BlueprintType)
enum class EQualityTier : uint8
{
    Basic,
    Standard,
    Quality,
    Premium,
    Elite,
    Legendary
};
```

---

### EPrecept

```cpp
UENUM(BlueprintType)
enum class EPrecept : uint8
{
    Craftsmanship,
    Mastery,
    Innovation,
    Discovery,
    Honor,
    Strength,
    Justice,
    Loyalty,
    Prosperity,
    Cunning,
    Freedom,
    Survival,
    Ambition,
    Efficiency,
    Tradition,
    Progress,
    // ... and more
};
```

---

### FSupplyChainRelationship

```cpp
USTRUCT(BlueprintType)
struct FSupplyChainRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UWayDataAsset* SupplierWay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UWayDataAsset* ConsumerWay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ResourceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SupplyQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PricePerUnit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsExclusive;
};
```

---

## Blueprint Integration

### Getting VerseSubsystem

```
Get Game Instance
  → Get Subsystem (Class: VerseSubsystem)
  → Store as variable or use directly
```

### Registering Networks on Game Start

```
Event BeginPlay (Game Mode)
  → Get Game Instance
  → Get Subsystem (VerseSubsystem)
  → Load Asset (DA_Network_Craftsmen)
  → Register Network
  → Load Asset (DA_Network_Frontier)
  → Register Network
  → Print "Networks Initialized"
```

### Checking Network Qualification

```
On Player Enter Station
  → Get Station Faction
  → Get Networks For Way (Station Faction)
  → ForEachLoop
    → Qualifies For Network Bonuses
    → Branch
      → True: Apply 10% Discount
      → False: Normal Prices
```

### Awarding Feat with Spillover

```
On Quest Complete
  → Get VerseSubsystem
  → Record Feat With Network Effects
    - Feat: Quest Feat Asset
    - Apply Network Effects: TRUE
  → Branch (Success)
    → True: Show Success Notification
    → False: Show "Already Completed"
```

### Displaying Network Info

```
Widget: On Construct
  → Get VerseSubsystem
  → Get Network Asset (from variable)
  → Get Network Name → Set Text
  → Get Network Verse Score → Set Progress Bar
  → Qualifies For Network Bonuses → Enable Benefits Button
  → Get Members → ForEachLoop → Add Member Widgets
```

---

## Common Patterns

### Pattern: Initialize Networks

```cpp
void AMyGameMode::InitializeNetworks()
{
    UVerseSubsystem* Verse = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
    
    TArray<FString> NetworkPaths = {
        TEXT("/Game/DataAssets/Networks/DA_Network_Craftsmen"),
        TEXT("/Game/DataAssets/Networks/DA_Network_Frontier"),
        TEXT("/Game/DataAssets/Networks/DA_Network_Honor"),
        TEXT("/Game/DataAssets/Networks/DA_Network_Merchants"),
        TEXT("/Game/DataAssets/Networks/DA_Network_Scholars")
    };
    
    for (const FString& Path : NetworkPaths)
    {
        UWayNetworkDataAsset* Network = LoadObject<UWayNetworkDataAsset>(nullptr, *Path);
        if (Network && Verse)
        {
            Verse->RegisterNetwork(Network);
        }
    }
}
```

---

### Pattern: Apply Network Discounts

```cpp
float CalculateNetworkDiscount(UWayDataAsset* StationWay, APlayerController* Player)
{
    UVerseSubsystem* Verse = Player->GetGameInstance()->GetSubsystem<UVerseSubsystem>();
    TArray<UWayNetworkDataAsset*> Networks = Verse->GetNetworksForWay(StationWay);
    
    float BestDiscount = 0.0f;
    
    for (UWayNetworkDataAsset* Network : Networks)
    {
        if (Verse->QualifiesForNetworkBonuses(Network))
        {
            float NetworkScore = Verse->GetNetworkVerseScore(Network);
            float Discount = 10.0f + (NetworkScore / 10.0f);  // 10-20% discount
            BestDiscount = FMath::Max(BestDiscount, Discount);
        }
    }
    
    return FMath::Clamp(BestDiscount, 0.0f, 25.0f);
}
```

---

### Pattern: Network Quest Generator

```cpp
FQuestData GenerateNetworkQuest(UWayDataAsset* QuestGiver)
{
    UVerseSubsystem* Verse = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
    TArray<UWayNetworkDataAsset*> Networks = Verse->GetNetworksForWay(QuestGiver);
    
    FQuestData Quest;
    
    if (Networks.Num() > 0)
    {
        // Quest affects entire network
        UWayNetworkDataAsset* Network = Networks[0];
        Quest.Title = FText::Format(
            FText::FromString("Network Mission: {0}"),
            Network->NetworkName
        );
        Quest.bIsNetworkQuest = true;
        Quest.AffectedNetwork = Network;
    }
    else
    {
        // Standard solo quest
        Quest.Title = FText::FromString("Solo Mission");
        Quest.bIsNetworkQuest = false;
    }
    
    return Quest;
}
```

---

### Pattern: Dynamic Network Formation

```cpp
void CheckForNetworkFormation(TArray<UWayDataAsset*> HighRepWays)
{
    // Find Ways with 2+ shared Precepts
    for (int32 i = 0; i < HighRepWays.Num(); i++)
    {
        for (int32 j = i + 1; j < HighRepWays.Num(); j++)
        {
            int32 SharedPrecepts = CountSharedPrecepts(HighRepWays[i], HighRepWays[j]);
            
            if (SharedPrecepts >= 2)
            {
                // Create new network
                UWayNetworkDataAsset* NewNetwork = CreateDynamicNetwork(
                    HighRepWays[i],
                    HighRepWays[j]
                );
                
                UVerseSubsystem* Verse = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
                Verse->RegisterNetwork(NewNetwork);
                
                ShowNetworkFormationEvent(NewNetwork);
            }
        }
    }
}
```

---

## Performance Considerations

### Caching Network Lookups

```cpp
// Cache network lookups to avoid repeated searches
TMap<UWayDataAsset*, TArray<UWayNetworkDataAsset*>> CachedNetworks;

TArray<UWayNetworkDataAsset*> GetCachedNetworks(UWayDataAsset* Way)
{
    if (!CachedNetworks.Contains(Way))
    {
        UVerseSubsystem* Verse = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
        CachedNetworks.Add(Way, Verse->GetNetworksForWay(Way));
    }
    
    return CachedNetworks[Way];
}
```

### Lazy Network Score Calculation

```cpp
// Only calculate network scores when needed
float GetCachedNetworkScore(UWayNetworkDataAsset* Network)
{
    static TMap<UWayNetworkDataAsset*, float> ScoreCache;
    static float LastUpdateTime = 0.0f;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update cache every 5 seconds
    if (CurrentTime - LastUpdateTime > 5.0f)
    {
        ScoreCache.Empty();
        LastUpdateTime = CurrentTime;
    }
    
    if (!ScoreCache.Contains(Network))
    {
        UVerseSubsystem* Verse = GetGameInstance()->GetSubsystem<UVerseSubsystem>();
        ScoreCache.Add(Network, Verse->GetNetworkVerseScore(Network));
    }
    
    return ScoreCache[Network];
}
```

---

## Version History

**v1.0** (2025-11-10)
- Initial API reference
- Core classes documented
- Blueprint integration examples
- Common patterns included

---

**Next**: See [WaySystemGuide.md](WaySystemGuide.md) for comprehensive system overview.
