// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerModComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerModData.h"
#include "GameFramework/Actor.h"
#include "AdastreaLog.h"

UPlayerModComponent::UPlayerModComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UPlayerModComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UPlayerModComponent, ActiveMods);
}

bool UPlayerModComponent::InstallMod(UPlayerModDataAsset* ModAsset)
{
    // Input validation
    if (!ModAsset)
    {
        UE_LOG(LogAdastreaPlayerMods, Warning, TEXT("PlayerModComponent::InstallMod - Invalid ModAsset"));
        return false;
    }

    if (GetOwnerRole() < ROLE_Authority)
    {
        // Client requests server to install
        ServerRequestInstall(ModAsset);
        return true;
    }

    // Server-side installation logic
    const FPlayerModDefinition& Def = ModAsset->ModDefinition;
    
    // Check if already installed
    if (FindActiveEntry(Def.ModID))
    {
        UE_LOG(LogAdastreaPlayerMods, Warning, TEXT("PlayerModComponent::InstallMod - Mod %s already installed"), *Def.ModID.ToString());
        return false;
    }

    // Create and add new mod entry
    FActiveModEntry NewEntry;
    NewEntry.ModAsset = ModAsset;
    NewEntry.TimeRemaining = (Def.Type == EModType::Active) ? 0.f : -1.f;
    NewEntry.Stacks = 1;
    ActiveMods.Add(NewEntry);

    // Apply passive stat modifiers immediately
    if (Def.Type == EModType::Passive)
    {
        ApplyStatModifiers(Def, 1);
    }

    UE_LOG(LogAdastreaPlayerMods, Log, TEXT("PlayerModComponent::InstallMod - Successfully installed %s"), *Def.ModID.ToString());
    return true;
}

bool UPlayerModComponent::UninstallMod(FName ModID)
{
    // Input validation
    if (ModID.IsNone())
    {
        UE_LOG(LogAdastreaPlayerMods, Warning, TEXT("PlayerModComponent::UninstallMod - Invalid ModID"));
        return false;
    }

    FActiveModEntry* Entry = FindActiveEntry(ModID);
    if (!Entry)
    {
        return false;
    }

    // Remove passive stat modifiers
    if (Entry->ModAsset.IsValid())
    {
        UPlayerModDataAsset* Asset = Entry->ModAsset.Get();
        if (Asset && Asset->ModDefinition.Type == EModType::Passive)
        {
            RemoveStatModifiers(Asset->ModDefinition);
        }
    }
    
    // Remove entry from active mods
    ActiveMods.RemoveAll([ModID](const FActiveModEntry& E)
    { 
        return E.ModAsset.IsValid() && E.ModAsset->ModDefinition.ModID == ModID; 
    });
    
    UE_LOG(LogAdastreaPlayerMods, Log, TEXT("PlayerModComponent::UninstallMod - Successfully uninstalled %s"), *ModID.ToString());
    return true;
}

bool UPlayerModComponent::IsModInstalled(FName ModID) const
{
    // Input validation
    if (ModID.IsNone())
    {
        return false;
    }

    for (const FActiveModEntry& Entry : ActiveMods)
    {
        if (Entry.ModAsset.IsValid() && Entry.ModAsset->ModDefinition.ModID == ModID)
        {
            return true;
        }
    }
    return false;
}

bool UPlayerModComponent::ActivateMod(FName ModID)
{
    // Input validation
    if (ModID.IsNone())
    {
        UE_LOG(LogAdastreaPlayerMods, Warning, TEXT("PlayerModComponent::ActivateMod - Invalid ModID"));
        return false;
    }

    if (GetOwnerRole() < ROLE_Authority)
    {
        ServerRequestActivate(ModID);
        return true;
    }

    FActiveModEntry* Entry = FindActiveEntry(ModID);
    if (!Entry || !Entry->ModAsset.IsValid())
    {
        UE_LOG(LogAdastreaPlayerMods, Warning, TEXT("PlayerModComponent::ActivateMod - Mod %s not found"), *ModID.ToString());
        return false;
    }

    UPlayerModDataAsset* Asset = Entry->ModAsset.Get();
    if (!Asset)
    {
        return false;
    }

    const FPlayerModDefinition& Def = Asset->ModDefinition;
    if (Def.Type != EModType::Active)
    {
        UE_LOG(LogAdastreaPlayerMods, Warning, TEXT("PlayerModComponent::ActivateMod - Mod %s is not an Active type"), *ModID.ToString());
        return false;
    }

    // Set duration and apply stat modifiers
    Entry->TimeRemaining = Def.Cooldown;
    ApplyStatModifiers(Def, Entry->Stacks);

    UE_LOG(LogAdastreaPlayerMods, Log, TEXT("PlayerModComponent::ActivateMod - Successfully activated %s"), *ModID.ToString());
    return true;
}

void UPlayerModComponent::ServerRequestInstall_Implementation(UPlayerModDataAsset* ModAsset)
{
    InstallMod(ModAsset);
}

void UPlayerModComponent::ServerRequestActivate_Implementation(FName ModID)
{
    ActivateMod(ModID);
}

void UPlayerModComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Validate DeltaTime
    if (DeltaTime < 0.0f)
    {
        return;
    }

    if (GetOwnerRole() == ROLE_Authority)
    {
        bool bChanged = false;
        
        // Update active mod timers (iterate in reverse to safely remove)
        for (int32 i = ActiveMods.Num() - 1; i >= 0; --i)
        {
            FActiveModEntry& Entry = ActiveMods[i];
            if (Entry.TimeRemaining > 0.f)
            {
                Entry.TimeRemaining -= DeltaTime;
                
                // Handle expiration
                if (Entry.TimeRemaining <= 0.f)
                {
                    if (Entry.ModAsset.IsValid())
                    {
                        UPlayerModDataAsset* Asset = Entry.ModAsset.Get();
                        if (Asset)
                        {
                            RemoveStatModifiers(Asset->ModDefinition);
                            UE_LOG(LogAdastreaPlayerMods, Log, TEXT("PlayerModComponent::Tick - Mod %s expired"), 
                                *Asset->ModDefinition.ModID.ToString());
                        }
                    }
                    
                    // Remove single-use active mods
                    ActiveMods.RemoveAt(i);
                    bChanged = true;
                }
            }
        }
    }
}

void UPlayerModComponent::OnRep_ActiveMods()
{
    // Clients can update HUD/VFX on mod list change
    UE_LOG(LogAdastreaPlayerMods, Verbose, TEXT("PlayerModComponent::OnRep_ActiveMods - Active mods replicated"));
}

void UPlayerModComponent::ApplyStatModifiers(const FPlayerModDefinition& Def, int32 Stacks)
{
    // Iterate through all stat modifiers defined in the mod
    for (const TPair<FName, float>& StatPair : Def.StatModifiers)
    {
        const FName& StatName = StatPair.Key;
        const float ModifierValue = StatPair.Value * Stacks;

        // Log the stat modification
        UE_LOG(LogAdastreaPlayerMods, Log, TEXT("PlayerModComponent::ApplyStatModifiers - %s: Applying %s modifier of %f (Stacks: %d)"), 
            *Def.ModID.ToString(), *StatName.ToString(), ModifierValue, Stacks);

        // TODO: When UPlayerStatsComponent is implemented, integrate here:
        // Example implementation:
        // if (UPlayerStatsComponent* StatsComp = GetOwner()->FindComponentByClass<UPlayerStatsComponent>())
        // {
        //     StatsComp->AddStatModifier(StatName, ModifierValue, Def.ModID);
        // }
        
        // For now, we store the intention and log it for future integration
        // The stat modification system will need to be hooked up when PlayerStatsComponent exists
    }
    
    UE_LOG(LogAdastreaPlayerMods, Verbose, TEXT("PlayerModComponent::ApplyStatModifiers - Applied %d stat modifiers for %s"), 
        Def.StatModifiers.Num(), *Def.ModID.ToString());
}

void UPlayerModComponent::RemoveStatModifiers(const FPlayerModDefinition& Def)
{
    // Iterate through all stat modifiers to remove
    for (const TPair<FName, float>& StatPair : Def.StatModifiers)
    {
        const FName& StatName = StatPair.Key;

        // Log the stat removal
        UE_LOG(LogAdastreaPlayerMods, Log, TEXT("PlayerModComponent::RemoveStatModifiers - %s: Removing %s modifier"), 
            *Def.ModID.ToString(), *StatName.ToString());

        // TODO: When UPlayerStatsComponent is implemented, integrate here:
        // Example implementation:
        // if (UPlayerStatsComponent* StatsComp = GetOwner()->FindComponentByClass<UPlayerStatsComponent>())
        // {
        //     StatsComp->RemoveStatModifier(StatName, Def.ModID);
        // }
        
        // The stat modification system will reverse the effects when PlayerStatsComponent exists
    }
    
    UE_LOG(LogAdastreaPlayerMods, Verbose, TEXT("PlayerModComponent::RemoveStatModifiers - Removed %d stat modifiers for %s"), 
        Def.StatModifiers.Num(), *Def.ModID.ToString());
}

FActiveModEntry* UPlayerModComponent::FindActiveEntry(FName ModID)
{
    if (ModID.IsNone())
    {
        return nullptr;
    }

    for (FActiveModEntry& Entry : ActiveMods)
    {
        if (Entry.ModAsset.IsValid() && Entry.ModAsset->ModDefinition.ModID == ModID)
        {
            return &Entry;
        }
    }
    return nullptr;
}

// Blueprint helper implementations

bool UPlayerModComponent::IsModInstalledByID(FName ModID) const
{
    return IsModInstalled(ModID);
}

float UPlayerModComponent::GetModCooldownPercent(FName ModID) const
{
    if (ModID.IsNone())
    {
        return 0.f;
    }

    const FActiveModEntry* Entry = nullptr;
    for (const FActiveModEntry& E : ActiveMods)
    {
        if (E.ModAsset.IsValid() && E.ModAsset->ModDefinition.ModID == ModID)
        {
            Entry = &E;
            break;
        }
    }
    
    if (!Entry || !Entry->ModAsset.IsValid())
    {
        return 0.f;
    }

    const UPlayerModDataAsset* Asset = Entry->ModAsset.Get();
    if (!Asset)
    {
        return 0.f;
    }

    const float Total = Asset->ModDefinition.Cooldown;
    if (Total <= 0.f)
    {
        return 0.f;
    }

    const float Remaining = FMath::Max(0.f, Entry->TimeRemaining);
    const float Progress = FMath::Clamp((Total - Remaining) / Total, 0.f, 1.f);
    
    return Progress;
}

float UPlayerModComponent::GetModRemainingTime(FName ModID) const
{
    if (ModID.IsNone())
    {
        return 0.f;
    }

    for (const FActiveModEntry& Entry : ActiveMods)
    {
        if (Entry.ModAsset.IsValid() && Entry.ModAsset->ModDefinition.ModID == ModID)
        {
            return FMath::Max(0.f, Entry.TimeRemaining);
        }
    }
    return 0.f;
}
