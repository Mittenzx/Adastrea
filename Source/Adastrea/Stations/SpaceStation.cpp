// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/SpaceStation.h"
#include "Factions/FactionDataAsset.h"
#include "AdastreaLog.h"

ASpaceStation::ASpaceStation()
{
    PrimaryActorTick.bCanEverTick = false;
    OwningFaction = nullptr;
    
    // Initialize health/integrity values
    CurrentStructuralIntegrity = 10000.0f;  // Stations are much tougher than ships
    MaxStructuralIntegrity = 10000.0f;
    bIsDestroyed = false;
    
    // Default station name
    StationName = FText::FromString(TEXT("Space Station"));
}

void ASpaceStation::BeginPlay()
{
    Super::BeginPlay();
}

void ASpaceStation::AddModule(ASpaceStationModule* Module)
{
    // Input validation
    if (!Module)
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("SpaceStation::AddModule - Invalid module pointer"));
        return;
    }

    // Check if module is already added
    if (Modules.Contains(Module))
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("SpaceStation::AddModule - Module already exists in station"));
        return;
    }

    // Add module to array
    Modules.Add(Module);
    
    // Attach the module to this station
    Module->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    
    UE_LOG(LogAdastreaStations, Log, TEXT("SpaceStation::AddModule - Successfully added module to station %s"), *GetName());
}

bool ASpaceStation::AddModuleAtLocation(ASpaceStationModule* Module, FVector RelativeLocation)
{
    // Input validation
    if (!Module)
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("SpaceStation::AddModuleAtLocation - Invalid module pointer"));
        return false;
    }

    // Attach the module to this station
    Module->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    Module->SetActorRelativeLocation(RelativeLocation);

    // Add to modules array if not already present
    if (!Modules.Contains(Module))
    {
        Modules.Add(Module);
    }

    UE_LOG(LogAdastreaStations, Log, TEXT("SpaceStation::AddModuleAtLocation - Added module at location (%.2f, %.2f, %.2f)"), 
        RelativeLocation.X, RelativeLocation.Y, RelativeLocation.Z);
    
    return true;
}

bool ASpaceStation::RemoveModule(ASpaceStationModule* Module)
{
    // Input validation
    if (!Module)
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("SpaceStation::RemoveModule - Invalid module pointer"));
        return false;
    }

    // Check if module exists in station
    if (!Modules.Contains(Module))
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("SpaceStation::RemoveModule - Module not found in station"));
        return false;
    }

    // Remove from modules array
    Modules.Remove(Module);
    
    // Detach the module from this station
    Module->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    UE_LOG(LogAdastreaStations, Log, TEXT("SpaceStation::RemoveModule - Successfully removed module from station %s"), *GetName());
    return true;
}

bool ASpaceStation::MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation)
{
    // Input validation
    if (!Module)
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("SpaceStation::MoveModule - Invalid module pointer"));
        return false;
    }

    // Check if module exists in station
    if (!Modules.Contains(Module))
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("SpaceStation::MoveModule - Module not found in station"));
        return false;
    }

    // Update the module's relative location
    Module->SetActorRelativeLocation(NewRelativeLocation);

    UE_LOG(LogAdastreaStations, Log, TEXT("SpaceStation::MoveModule - Moved module to (%.2f, %.2f, %.2f)"), 
        NewRelativeLocation.X, NewRelativeLocation.Y, NewRelativeLocation.Z);
    
    return true;
}

TArray<ASpaceStationModule*> ASpaceStation::GetModules() const
{
    return Modules;
}

TArray<ASpaceStationModule*> ASpaceStation::GetModulesByType(const FString& ModuleType) const
{
    TArray<ASpaceStationModule*> MatchingModules;

    // Input validation
    if (ModuleType.IsEmpty())
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("SpaceStation::GetModulesByType - Empty module type"));
        return MatchingModules;
    }

    // Reserve space for better performance if we have a reasonable number of modules
    MatchingModules.Reserve(Modules.Num() / 4);  // Estimate 25% might match

    // Find all matching modules
    for (ASpaceStationModule* Module : Modules)
    {
        // Null check for safety
        if (Module && Module->ModuleType == ModuleType)
        {
            MatchingModules.Add(Module);
        }
    }

    return MatchingModules;
}

int32 ASpaceStation::GetModuleCount() const
{
    return Modules.Num();
}

void ASpaceStation::SetFaction(UFactionDataAsset* NewFaction)
{
    // Note: Null factions are allowed - stations can be neutral/unaffiliated
    OwningFaction = NewFaction;
    
    if (NewFaction)
    {
        UE_LOG(LogAdastreaStations, Log, TEXT("SpaceStation::SetFaction - Set faction for station %s to %s"), 
            *GetName(), *NewFaction->GetFactionName().ToString());
    }
    else
    {
        UE_LOG(LogAdastreaStations, Log, TEXT("SpaceStation::SetFaction - Cleared faction for station %s (now neutral)"), *GetName());
    }
}

// ====================
// IDamageable Interface Implementation
// ====================

float ASpaceStation::ApplyDamage_Implementation(float Damage, EDamageType DamageType, AActor* Instigator, AActor* DamageCauser)
{
    if (!CanTakeDamage_Implementation())
    {
        return 0.0f;
    }

    // TODO: Apply damage type modifiers based on station shields/armor
    // Stations might have different resistances than ships
    float ActualDamage = FMath::Min(Damage, CurrentStructuralIntegrity);
    CurrentStructuralIntegrity -= ActualDamage;

    UE_LOG(LogAdastreaStations, Log, TEXT("%s took %.1f damage from %s. Integrity: %.1f/%.1f"),
        *GetName(), ActualDamage,
        Instigator ? *Instigator->GetName() : TEXT("Unknown"),
        CurrentStructuralIntegrity, MaxStructuralIntegrity);

    // Check if station is destroyed
    if (CurrentStructuralIntegrity <= 0.0f)
    {
        CurrentStructuralIntegrity = 0.0f;
        bIsDestroyed = true;
        UE_LOG(LogAdastreaStations, Warning, TEXT("%s has been destroyed!"), *GetName());
        // TODO: Trigger destruction effects, module separation, debris spawning
    }

    return ActualDamage;
}

bool ASpaceStation::CanTakeDamage_Implementation() const
{
    // Cannot take damage if already destroyed
    if (bIsDestroyed)
    {
        return false;
    }

    // TODO: Add additional checks for:
    // - Invulnerability (quest protection, safe zones)
    // - Shield coverage
    
    return true;
}

float ASpaceStation::GetHealthPercentage_Implementation() const
{
    if (MaxStructuralIntegrity <= 0.0f)
    {
        return 0.0f;
    }

    return CurrentStructuralIntegrity / MaxStructuralIntegrity;
}

bool ASpaceStation::IsDestroyed_Implementation() const
{
    return bIsDestroyed;
}

float ASpaceStation::GetMaxHealth_Implementation() const
{
    return MaxStructuralIntegrity;
}

float ASpaceStation::GetCurrentHealth_Implementation() const
{
    return CurrentStructuralIntegrity;
}

// ====================
// ITargetable Interface Implementation
// ====================

bool ASpaceStation::CanBeTargeted_Implementation() const
{
    // Cannot target destroyed stations
    if (bIsDestroyed)
    {
        return false;
    }

    // Stations are always visible (no cloaking)
    // TODO: Add checks for safe zones if needed

    return true;
}

int32 ASpaceStation::GetTargetPriority_Implementation() const
{
    // Stations are high-priority strategic targets
    // Priority based on:
    // - Size (stations are important)
    // - Module count (more modules = higher value)
    // - Faction relationship
    
    int32 BasePriority = 75;  // High priority by default
    
    // Increase priority based on module count
    int32 ModulePriority = FMath::Min(Modules.Num() / 2, 20);  // Up to +20 for large stations
    
    return FMath::Clamp(BasePriority + ModulePriority, 0, 100);
}

FText ASpaceStation::GetTargetDisplayName_Implementation() const
{
    return StationName;
}

UTexture2D* ASpaceStation::GetTargetIcon_Implementation() const
{
    // TODO: Return station icon
    // For now return nullptr, UI can use default icon
    return nullptr;
}

FVector ASpaceStation::GetAimPoint_Implementation() const
{
    // Return center of station (aim for center mass)
    return GetActorLocation();
}

float ASpaceStation::GetTargetSignature_Implementation() const
{
    // Stations have very high signatures (large, lots of power)
    // They are easy to detect and cannot hide
    return 2.0f;  // Enhanced signature
}

float ASpaceStation::GetDistanceFromLocation_Implementation(FVector FromLocation) const
{
    return FVector::Dist(GetActorLocation(), FromLocation);
}

bool ASpaceStation::IsHostileToActor_Implementation(AActor* Observer) const
{
    if (!Observer)
    {
        return false;
    }

    // Check if observer implements IFactionMember
    if (Observer->Implements<UFactionMember>())
    {
        return IsHostileTo_Implementation(TScriptInterface<IFactionMember>(Observer));
    }

    // Default to non-hostile if can't determine faction
    return false;
}

// ====================
// IFactionMember Interface Implementation
// ====================

UFactionDataAsset* ASpaceStation::GetFaction_Implementation() const
{
    return OwningFaction;
}

bool ASpaceStation::IsAlliedWith_Implementation(const TScriptInterface<IFactionMember>& Other) const
{
    if (!Other.GetObject())
    {
        return false;
    }

    // Same faction = always allied
    UFactionDataAsset* OtherFaction = IFactionMember::Execute_GetFaction(Other.GetObject());
    if (OwningFaction && OtherFaction && OwningFaction == OtherFaction)
    {
        return true;
    }

    // Check diplomatic relations
    int32 Relationship = GetRelationshipWith_Implementation(Other);
    return Relationship >= 26;  // Friendly or allied
}

bool ASpaceStation::IsHostileTo_Implementation(const TScriptInterface<IFactionMember>& Other) const
{
    if (!Other.GetObject())
    {
        return false;
    }

    // Same faction = never hostile
    UFactionDataAsset* OtherFaction = IFactionMember::Execute_GetFaction(Other.GetObject());
    if (OwningFaction && OtherFaction && OwningFaction == OtherFaction)
    {
        return false;
    }

    // Check diplomatic relations
    int32 Relationship = GetRelationshipWith_Implementation(Other);
    return Relationship <= -26;  // Hostile or at war
}

int32 ASpaceStation::GetRelationshipWith_Implementation(const TScriptInterface<IFactionMember>& Other) const
{
    if (!Other.GetObject() || !OwningFaction)
    {
        return 0;  // Neutral if no faction
    }

    UFactionDataAsset* OtherFaction = IFactionMember::Execute_GetFaction(Other.GetObject());
    if (!OtherFaction)
    {
        return 0;  // Neutral if other has no faction
    }

    if (OwningFaction == OtherFaction)
    {
        return 100;  // Same faction = maximum positive relationship
    }

    // TODO: Query faction relationship from FactionDiplomacyManager
    // For now return neutral
    return 0;
}

bool ASpaceStation::IsNeutral_Implementation() const
{
    return OwningFaction == nullptr;
}

FText ASpaceStation::GetFactionDisplayName_Implementation() const
{
    if (OwningFaction)
    {
        return OwningFaction->FactionName;
    }

    return FText::FromString(TEXT("Independent"));
}

bool ASpaceStation::CanEngageInCombat_Implementation() const
{
    // Cannot engage if destroyed
    if (bIsDestroyed)
    {
        return false;
    }

    // TODO: Add checks for:
    // - Armed modules (turrets, weapons)
    // - Safe zone restrictions

    return true;
}

float ASpaceStation::GetTradePriceModifier_Implementation(UFactionDataAsset* TraderFaction) const
{
    if (!OwningFaction || !TraderFaction)
    {
        return 1.0f;  // Base price if no faction
    }

    if (OwningFaction == TraderFaction)
    {
        return 0.85f;  // 15% discount for same faction (stations offer better prices than ships)
    }

    // TODO: Calculate modifier based on faction relationship
    // Positive relationship = better prices
    // Negative relationship = worse prices or refusal to trade
    
    return 1.0f;  // Base price for now
}
