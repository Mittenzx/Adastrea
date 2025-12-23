#include "AI/FactionLogic.h"
#include "Public/AdastreaLog.h"

UFactionLogic::UFactionLogic()
{
    // Set sensible defaults
    CurrentStrategy = EFactionStrategyType::Consolidation; // Start with defensive strategy
    CurrentBehaviorMode = EAIBehaviorMode::Peaceful; // Early game peaceful
    
    MilitaryStrength = 50.0f;
    EconomicStrength = 50.0f;
    DiplomaticInfluence = 50.0f;
    TerritorySize = 50.0f;
    
    bIsAtWar = false;
    bInEconomicCrisis = false;
    
    HoursUntilStrategicReview = 24.0f; // Review strategy daily
    UpdateInterval = 5.0f; // Update every 5 seconds (less frequent than personnel)
}

void UFactionLogic::InitializeAI_Implementation()
{
    Super::InitializeAI_Implementation();
    
    if (!FactionData)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionLogic initialized without FactionData asset"));
        return;
    }
    
    UE_LOG(LogAdastrea, Log, TEXT("Faction AI initialized: %s"), *FactionData->FactionName.ToString());
    
    // Initialize relationships from faction data
    if (FactionData->Relationships.Num() > 0)
    {
        for (const FFactionRelationship& Relationship : FactionData->Relationships)
        {
            // Load the target faction using cached helper
            if (UFactionDataAsset* TargetFaction = GetLoadedFaction(Relationship.TargetFaction))
            {
                if (Relationship.RelationshipStrength >= 75)
                {
                    AddAlly(TargetFaction->FactionID);
                }
                else if (Relationship.RelationshipStrength <= -75)
                {
                    AddEnemy(TargetFaction->FactionID);
                }
            }
        }
    }
    
    // Set initial strategy based on faction traits
    bool bIsAggressive = false;
    bool bIsMercantile = false;
    
    for (const FFactionTrait& Trait : FactionData->Traits)
    {
        if (Trait.TraitID == FName(TEXT("Aggressive")) || Trait.TraitID == FName(TEXT("Militaristic")))
        {
            bIsAggressive = true;
        }
        else if (Trait.TraitID == FName(TEXT("Trading")) || Trait.TraitID == FName(TEXT("Mercantile")))
        {
            bIsMercantile = true;
        }
    }
    
    if (bIsAggressive)
    {
        CurrentStrategy = EFactionStrategyType::Military;
        CurrentBehaviorMode = EAIBehaviorMode::Aggressive;
    }
    else if (bIsMercantile)
    {
        CurrentStrategy = EFactionStrategyType::Economic;
        CurrentBehaviorMode = EAIBehaviorMode::Trading;
    }
    
    UpdateStrengthRatings();
}

void UFactionLogic::OnTickAI_Implementation(float DeltaTime)
{
    Super::OnTickAI_Implementation(DeltaTime);
    
    if (!FactionData)
    {
        return;
    }
    
    // Convert to hours (assuming DeltaTime is in seconds)
    float DeltaHours = DeltaTime / 3600.0f;
    HoursUntilStrategicReview -= DeltaHours;
    
    // Periodic strategic review
    if (HoursUntilStrategicReview <= 0.0f)
    {
        EvaluateStrategicSituation();
        UpdateStrengthRatings();
        HoursUntilStrategicReview = 24.0f; // Reset to daily review
        
        if (ShouldChangeStrategy())
        {
            // Strategy change is handled in ShouldChangeStrategy_Implementation
        }
    }
}

EAIPriority UFactionLogic::EvaluateCurrentPriority_Implementation()
{
    // Critical: Under attack or economic collapse
    if (bIsAtWar && MilitaryStrength < 30.0f)
    {
        return EAIPriority::Critical;
    }
    
    if (bInEconomicCrisis)
    {
        return EAIPriority::Critical;
    }
    
    // High: Strategic opportunities or threats
    if (IsVulnerable())
    {
        return EAIPriority::High;
    }
    
    // Medium: Normal operations
    if (CurrentStrategy == EFactionStrategyType::Expansion || 
        CurrentStrategy == EFactionStrategyType::Military)
    {
        return EAIPriority::Medium;
    }
    
    // Low: Maintenance and consolidation
    return EAIPriority::Low;
}

bool UFactionLogic::DecideNextAction_Implementation(EAIPriority CurrentPriority)
{
    if (!FactionData)
    {
        return false;
    }
    
    switch (CurrentPriority)
    {
        case EAIPriority::Critical:
        {
            // Emergency response
            if (bIsAtWar)
            {
                EMilitaryActionType MilitaryAction = DecideMilitaryAction();
                UE_LOG(LogAdastrea, Log, TEXT("Faction %s: Critical military action: %s"),
                    *FactionData->FactionName.ToString(),
                    *UEnum::GetValueAsString(MilitaryAction));
            }
            else if (bInEconomicCrisis)
            {
                EEconomicActionType EconomicAction = DecideEconomicAction();
                UE_LOG(LogAdastrea, Log, TEXT("Faction %s: Critical economic action: %s"),
                    *FactionData->FactionName.ToString(),
                    *UEnum::GetValueAsString(EconomicAction));
            }
            return true;
        }
        
        case EAIPriority::High:
        case EAIPriority::Medium:
        {
            // Strategic actions based on current strategy
            switch (CurrentStrategy)
            {
                case EFactionStrategyType::Military:
                    DecideMilitaryAction();
                    break;
                case EFactionStrategyType::Economic:
                    DecideEconomicAction();
                    break;
                case EFactionStrategyType::Diplomacy:
                    // Check relationships with other factions
                    // In real implementation, would iterate through known factions
                    break;
                default:
                    break;
            }
            return true;
        }
        
        case EAIPriority::Low:
        case EAIPriority::Idle:
        default:
            // Routine maintenance
            return false;
    }
}

EDiplomaticActionType UFactionLogic::DecideDiplomaticAction_Implementation(FName TargetFactionID, int32 CurrentRelationship)
{
    if (!FactionData)
    {
        return EDiplomaticActionType::None;
    }
    
    // Check if already allied or at war
    if (IsAlliedWith(TargetFactionID))
    {
        // Maintain alliance with aid if they're struggling
        if (CurrentRelationship > 80)
        {
            return EDiplomaticActionType::SendAid;
        }
        return EDiplomaticActionType::None;
    }
    
    if (IsEnemyOf(TargetFactionID))
    {
        // Consider peace if we're losing or economically weak
        if (MilitaryStrength < 40.0f || bInEconomicCrisis)
        {
            return EDiplomaticActionType::OfferPeace;
        }
        return EDiplomaticActionType::None;
    }
    
    // Neutral faction - decide based on relationship and strategy
    if (CurrentRelationship > 60)
    {
        // Friendly - consider alliance
        if (CurrentStrategy == EFactionStrategyType::Diplomacy)
        {
            return EDiplomaticActionType::ProposeAlliance;
        }
        else if (CurrentStrategy == EFactionStrategyType::Economic)
        {
            return EDiplomaticActionType::OpenTrade;
        }
    }
    else if (CurrentRelationship < -60)
    {
        // Hostile - consider war if we're strong and aggressive
        bool bIsAggressive = false;
        for (const FFactionTrait& Trait : FactionData->Traits)
        {
            if (Trait.TraitID == FName(TEXT("Aggressive")) || Trait.TraitID == FName(TEXT("Militaristic")))
            {
                bIsAggressive = true;
                break;
            }
        }
        
        if (bIsAggressive && MilitaryStrength > 60.0f && CurrentStrategy == EFactionStrategyType::Military)
        {
            return EDiplomaticActionType::DeclareWar;
        }
    }
    
    // Default: improve relations through trade
    if (CurrentStrategy == EFactionStrategyType::Economic)
    {
        return EDiplomaticActionType::OpenTrade;
    }
    
    return EDiplomaticActionType::None;
}

EEconomicActionType UFactionLogic::DecideEconomicAction_Implementation()
{
    if (!FactionData)
    {
        return EEconomicActionType::None;
    }
    
    // Crisis management
    if (bInEconomicCrisis)
    {
        return EEconomicActionType::SellSurplus;
    }
    
    // Strategy-based decisions
    switch (CurrentStrategy)
    {
        case EFactionStrategyType::Economic:
            if (EconomicStrength < 60.0f)
            {
                return EEconomicActionType::InvestInProduction;
            }
            else
            {
                return EEconomicActionType::EstablishTradeRoute;
            }
            
        case EFactionStrategyType::Expansion:
            if (EconomicStrength > 70.0f)
            {
                return EEconomicActionType::BuildStation;
            }
            break;
            
        case EFactionStrategyType::Military:
            if (EconomicStrength > 60.0f)
            {
                return EEconomicActionType::StockpileResources;
            }
            break;
            
        default:
            break;
    }
    
    // Default: maintain and improve production
    return EEconomicActionType::InvestInProduction;
}

EMilitaryActionType UFactionLogic::DecideMilitaryAction_Implementation()
{
    if (!FactionData)
    {
        return EMilitaryActionType::None;
    }
    
    // Crisis response
    if (bIsAtWar)
    {
        if (MilitaryStrength < 40.0f)
        {
            return EMilitaryActionType::DefendAssets;
        }
        else if (MilitaryStrength > 70.0f)
        {
            return EMilitaryActionType::LaunchOffensive;
        }
        else
        {
            return EMilitaryActionType::PatrolTerritory;
        }
    }
    
    // Strategy-based decisions
    switch (CurrentStrategy)
    {
        case EFactionStrategyType::Military:
            if (MilitaryStrength < 60.0f)
            {
                return EMilitaryActionType::BuildFleet;
            }
            else
            {
                return EMilitaryActionType::UpgradeFleet;
            }
            
        case EFactionStrategyType::Expansion:
            if (MilitaryStrength > 50.0f)
            {
                return EMilitaryActionType::SecureTerritory;
            }
            break;
            
        case EFactionStrategyType::Consolidation:
        case EFactionStrategyType::Survival:
            return EMilitaryActionType::DefendAssets;
            
        default:
            break;
    }
    
    // Default: maintain patrol
    return EMilitaryActionType::PatrolTerritory;
}

void UFactionLogic::EvaluateStrategicSituation_Implementation()
{
    if (!FactionData)
    {
        return;
    }
    
    // Evaluate threats
    int32 TotalEnemies = Enemies.Num();
    int32 TotalAllies = Allies.Num();
    
    bIsAtWar = (TotalEnemies > 0);
    bInEconomicCrisis = (EconomicStrength < 30.0f);
    
    // Log strategic situation
    UE_LOG(LogAdastrea, Verbose, TEXT("Faction %s: Strategic Situation - Military: %.1f, Economic: %.1f, Diplomatic: %.1f, Territory: %.1f"),
        *FactionData->FactionName.ToString(),
        MilitaryStrength,
        EconomicStrength,
        DiplomaticInfluence,
        TerritorySize);
}

bool UFactionLogic::ShouldChangeStrategy_Implementation()
{
    if (!FactionData)
    {
        return false;
    }
    
    EFactionStrategyType NewStrategy = CurrentStrategy;
    
    // Emergency strategy changes
    if (bIsAtWar && MilitaryStrength < 50.0f)
    {
        NewStrategy = EFactionStrategyType::Survival;
    }
    else if (bInEconomicCrisis)
    {
        NewStrategy = EFactionStrategyType::Economic;
    }
    // Opportunistic strategy changes
    else if (IsInStrongPosition())
    {
        NewStrategy = EFactionStrategyType::Expansion;
    }
    // Defensive strategy changes
    else if (IsVulnerable())
    {
        NewStrategy = EFactionStrategyType::Consolidation;
    }
    
    if (NewStrategy != CurrentStrategy)
    {
        UE_LOG(LogAdastrea, Log, TEXT("Faction %s: Strategy changing from %s to %s"),
            *FactionData->FactionName.ToString(),
            *UEnum::GetValueAsString(CurrentStrategy),
            *UEnum::GetValueAsString(NewStrategy));
        
        SetStrategy(NewStrategy);
        return true;
    }
    
    return false;
}

void UFactionLogic::SelectActionTargets_Implementation(EFactionStrategyType ActionType, int32 MaxTargets, TArray<FName>& OutTargets)
{
    OutTargets.Empty();
    
    if (!FactionData)
    {
        return;
    }
    
    // In a real implementation, this would query game world for potential targets
    // For now, we'll use relationships from faction data
    
    TArray<TPair<FName, int32>> PotentialTargets;
    
    for (const FFactionRelationship& Relationship : FactionData->Relationships)
    {
        // Load the target faction using cached helper
        UFactionDataAsset* TargetFaction = GetLoadedFaction(Relationship.TargetFaction);
        if (!TargetFaction)
        {
            continue;
        }
        
        FName TargetID = TargetFaction->FactionID;
        int32 RelationshipValue = Relationship.RelationshipStrength;
        
        // Filter based on action type
        switch (ActionType)
        {
            case EFactionStrategyType::Military:
                // Target enemies or weak neutrals
                if (IsEnemyOf(TargetID) || RelationshipValue < -40)
                {
                    PotentialTargets.Add(TPair<FName, int32>(TargetID, -RelationshipValue)); // Prioritize most hostile
                }
                break;
                
            case EFactionStrategyType::Diplomacy:
                // Target neutrals and potential allies
                if (!IsAlliedWith(TargetID) && !IsEnemyOf(TargetID) && RelationshipValue > 0)
                {
                    PotentialTargets.Add(TPair<FName, int32>(TargetID, RelationshipValue)); // Prioritize friendliest
                }
                break;
                
            case EFactionStrategyType::Economic:
                // Target trading partners (neutrals and allies)
                if (!IsEnemyOf(TargetID))
                {
                    PotentialTargets.Add(TPair<FName, int32>(TargetID, RelationshipValue));
                }
                break;
                
            default:
                break;
        }
    }
    
    // Sort by priority (second element of pair)
    PotentialTargets.Sort([](const TPair<FName, int32>& A, const TPair<FName, int32>& B) {
        return A.Value > B.Value;
    });
    
    // Take top MaxTargets
    for (int32 i = 0; i < FMath::Min(MaxTargets, PotentialTargets.Num()); ++i)
    {
        OutTargets.Add(PotentialTargets[i].Key);
    }
}

void UFactionLogic::HandleDiplomaticEvent_Implementation(FName SourceFactionID, EDiplomaticActionType Action, bool bAccept)
{
    if (!FactionData)
    {
        return;
    }
    
    UE_LOG(LogAdastrea, Log, TEXT("Faction %s: Received diplomatic action %s from %s"),
        *FactionData->FactionName.ToString(),
        *UEnum::GetValueAsString(Action),
        *SourceFactionID.ToString());
    
    switch (Action)
    {
        case EDiplomaticActionType::ProposeAlliance:
            if (bAccept)
            {
                AddAlly(SourceFactionID);
                RemoveEnemy(SourceFactionID);
            }
            break;
            
        case EDiplomaticActionType::DeclareWar:
            AddEnemy(SourceFactionID);
            RemoveAlly(SourceFactionID);
            bIsAtWar = true;
            // Switch to defensive strategy
            SetStrategy(EFactionStrategyType::Survival);
            break;
            
        case EDiplomaticActionType::OfferPeace:
            if (bAccept)
            {
                RemoveEnemy(SourceFactionID);
                bIsAtWar = (Enemies.Num() > 0);
            }
            break;
            
        case EDiplomaticActionType::OpenTrade:
            // In real implementation, would establish trade route
            UE_LOG(LogAdastrea, Log, TEXT("Trade route opened with %s"), *SourceFactionID.ToString());
            break;
            
        default:
            break;
    }
}

// ====================
// Utility Functions
// ====================

bool UFactionLogic::IsAlliedWith(FName OtherFactionID) const
{
    return Allies.Contains(OtherFactionID);
}

bool UFactionLogic::IsEnemyOf(FName OtherFactionID) const
{
    return Enemies.Contains(OtherFactionID);
}

void UFactionLogic::AddAlly(FName OtherFactionID)
{
    if (!Allies.Contains(OtherFactionID))
    {
        Allies.Add(OtherFactionID);
        RemoveEnemy(OtherFactionID); // Can't be both ally and enemy
    }
}

void UFactionLogic::RemoveAlly(FName OtherFactionID)
{
    Allies.Remove(OtherFactionID);
}

void UFactionLogic::AddEnemy(FName OtherFactionID)
{
    if (!Enemies.Contains(OtherFactionID))
    {
        Enemies.Add(OtherFactionID);
        RemoveAlly(OtherFactionID); // Can't be both ally and enemy
    }
}

void UFactionLogic::RemoveEnemy(FName OtherFactionID)
{
    Enemies.Remove(OtherFactionID);
}

float UFactionLogic::CalculateOverallPower() const
{
    // Weighted average of all strength ratings
    return (MilitaryStrength * 0.4f + EconomicStrength * 0.3f + 
            DiplomaticInfluence * 0.2f + TerritorySize * 0.1f);
}

bool UFactionLogic::IsInStrongPosition() const
{
    return MilitaryStrength > 50.0f && EconomicStrength > 50.0f && DiplomaticInfluence > 50.0f;
}

bool UFactionLogic::IsVulnerable() const
{
    return MilitaryStrength < 30.0f || EconomicStrength < 30.0f || TerritorySize < 20.0f;
}

int32 UFactionLogic::GetRelationshipWith(FName OtherFactionID) const
{
    if (!FactionData)
    {
        return 0;
    }
    
    for (const FFactionRelationship& Relationship : FactionData->Relationships)
    {
        // Load the target faction using cached helper
        UFactionDataAsset* TargetFaction = GetLoadedFaction(Relationship.TargetFaction);
        if (TargetFaction && TargetFaction->FactionID == OtherFactionID)
        {
            return Relationship.RelationshipStrength;
        }
    }
    
    return 0;
}

FText UFactionLogic::GetFactionName() const
{
    if (FactionData)
    {
        return FactionData->FactionName;
    }
    return FText::FromString(TEXT("Unknown Faction"));
}

FString UFactionLogic::GetFactionStateDescription() const
{
    if (!FactionData)
    {
        return TEXT("No faction data");
    }
    
    return FString::Printf(TEXT("%s - Strategy: %s, Power: %.1f, Allies: %d, Enemies: %d"),
        *FactionData->FactionName.ToString(),
        *UEnum::GetValueAsString(CurrentStrategy),
        CalculateOverallPower(),
        Allies.Num(),
        Enemies.Num());
}

void UFactionLogic::SetStrategy(EFactionStrategyType NewStrategy)
{
    if (CurrentStrategy != NewStrategy)
    {
        EFactionStrategyType OldStrategy = CurrentStrategy;
        CurrentStrategy = NewStrategy;
        
        // Update behavior mode based on strategy
        switch (NewStrategy)
        {
            case EFactionStrategyType::Military:
                SetBehaviorMode(EAIBehaviorMode::Aggressive);
                break;
            case EFactionStrategyType::Economic:
                SetBehaviorMode(EAIBehaviorMode::Trading);
                break;
            case EFactionStrategyType::Diplomacy:
                SetBehaviorMode(EAIBehaviorMode::Diplomatic);
                break;
            case EFactionStrategyType::Expansion:
                SetBehaviorMode(EAIBehaviorMode::Exploration);
                break;
            case EFactionStrategyType::Survival:
                SetBehaviorMode(EAIBehaviorMode::Defensive);
                break;
            default:
                SetBehaviorMode(EAIBehaviorMode::Peaceful);
                break;
        }
    }
}

void UFactionLogic::UpdateStrengthRatings()
{
    // In a real implementation, these would be calculated from actual game state
    // For now, simulate gradual changes based on strategy
    
    switch (CurrentStrategy)
    {
        case EFactionStrategyType::Military:
            MilitaryStrength = FMath::Min(100.0f, MilitaryStrength + 1.0f);
            EconomicStrength = FMath::Max(0.0f, EconomicStrength - 0.5f);
            break;
            
        case EFactionStrategyType::Economic:
            EconomicStrength = FMath::Min(100.0f, EconomicStrength + 1.5f);
            MilitaryStrength = FMath::Max(0.0f, MilitaryStrength - 0.3f);
            break;
            
        case EFactionStrategyType::Diplomacy:
            DiplomaticInfluence = FMath::Min(100.0f, DiplomaticInfluence + 1.0f);
            break;
            
        case EFactionStrategyType::Expansion:
            TerritorySize = FMath::Min(100.0f, TerritorySize + 0.8f);
            EconomicStrength = FMath::Max(0.0f, EconomicStrength - 0.5f);
            break;
            
        default:
            // Consolidation: slow, balanced growth
            MilitaryStrength = FMath::Min(100.0f, MilitaryStrength + 0.2f);
            EconomicStrength = FMath::Min(100.0f, EconomicStrength + 0.3f);
            break;
    }
    
    // War drains economy
    if (bIsAtWar)
    {
        EconomicStrength = FMath::Max(0.0f, EconomicStrength - 1.0f);
    }
}

// ====================
// Internal Helper Functions
// ====================

UFactionDataAsset* UFactionLogic::GetLoadedFaction(const TSoftObjectPtr<UFactionDataAsset>& SoftPtr)
{
    if (!SoftPtr.IsValid() && !SoftPtr.IsPending())
    {
        return nullptr;
    }

    FSoftObjectPath Path = SoftPtr.ToSoftObjectPath();
    
    // Check cache first
    if (TObjectPtr<UFactionDataAsset>* CachedFaction = LoadedFactionCache.Find(Path))
    {
        if (CachedFaction->IsValid())
        {
            return CachedFaction->Get();
        }
        // Remove invalid cached entry
        LoadedFactionCache.Remove(Path);
    }

    // Load and cache the faction
    UFactionDataAsset* LoadedFaction = SoftPtr.LoadSynchronous();
    if (LoadedFaction)
    {
        LoadedFactionCache.Add(Path, LoadedFaction);
    }

    return LoadedFaction;
}
