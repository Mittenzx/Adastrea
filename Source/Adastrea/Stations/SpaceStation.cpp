#include "Stations/SpaceStation.h"
#include "Public/Factions/FactionDataAsset.h"
#include "AdastreaLog.h"

ASpaceStation::ASpaceStation()
{
    PrimaryActorTick.bCanEverTick = false;
    Faction = nullptr;
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

TArray<ASpaceStationModule*> ASpaceStation::GetModulesByType(const FString& ModuleType)
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

UFactionDataAsset* ASpaceStation::GetFaction() const
{
    return Faction;
}

void ASpaceStation::SetFaction(UFactionDataAsset* NewFaction)
{
    Faction = NewFaction;
}

// ====================
// Advanced Trait & Diplomacy Hooks
// ====================

bool ASpaceStation::HasFactionTrait(FName TraitID) const
{
    // Early validation
    if (!Faction || TraitID.IsNone())
    {
        return false;
    }
    
    return Faction->HasTrait(TraitID);
}

float ASpaceStation::GetFactionTraitModifier(FName TraitID) const
{
    // Early validation
    if (!Faction || TraitID.IsNone())
    {
        return 0.0f;
    }
    
    return Faction->GetTraitModifier(TraitID);
}

bool ASpaceStation::IsAlliedWithFaction(FName OtherFactionID) const
{
    // Early validation
    if (!Faction || OtherFactionID.IsNone())
    {
        return false;
    }
    
    return Faction->IsAlliedWith(OtherFactionID);
}

bool ASpaceStation::IsAtWarWithFaction(FName OtherFactionID) const
{
    // Early validation
    if (!Faction || OtherFactionID.IsNone())
    {
        return false;
    }
    
    return Faction->IsAtWarWith(OtherFactionID);
}

int32 ASpaceStation::GetFactionRelationship(FName OtherFactionID) const
{
    // Early validation
    if (!Faction || OtherFactionID.IsNone())
    {
        return 0;
    }
    
    return Faction->GetRelationshipValue(OtherFactionID);
}
