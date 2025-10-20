#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerModData.h"
#include "PlayerModComponent.generated.h"

class UPlayerModDataAsset;

USTRUCT(BlueprintType)
struct FActiveModEntry
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UPlayerModDataAsset> ModAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float TimeRemaining = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 Stacks = 1;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UPlayerModComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerModComponent();

    /** Installs a mod (server only authoritative). Returns true if install request accepted. */
    UFUNCTION(BlueprintCallable, Category = "Mods")
    bool InstallMod(UPlayerModDataAsset* ModAsset);

    /** Uninstall mod by ID */
    UFUNCTION(BlueprintCallable, Category = "Mods")
    bool UninstallMod(FName ModID);

    /** Checks if mod installed */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mods")
    bool IsModInstalled(FName ModID) const;

    /** Activates an installed mod (for Active types) */
    UFUNCTION(BlueprintCallable, Category = "Mods")
    bool ActivateMod(FName ModID);

    /** Called each tick to update durations */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Blueprint callable helpers for UI ------------------------------------------------

    /** Blueprint-friendly check by ModID */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mods|UI")
    bool IsModInstalledByID(FName ModID) const;

    /** Returns 0..1 percent of cooldown progress (0 = ready, 1 = full) */
    UFUNCTION(BlueprintCallable, Category = "Mods|UI")
    float GetModCooldownPercent(FName ModID) const;

    /** Remaining time in seconds for active cooldown/duration (0 if none) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mods|UI")
    float GetModRemainingTime(FName ModID) const;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Server RPC to request an install */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestInstall(UPlayerModDataAsset* ModAsset);
    bool ServerRequestInstall_Validate(UPlayerModDataAsset* ModAsset) { return true; }
    void ServerRequestInstall_Implementation(UPlayerModDataAsset* ModAsset);

    /** Server RPC to request activation */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestActivate(FName ModID);
    bool ServerRequestActivate_Validate(FName ModID) { return true; }
    void ServerRequestActivate_Implementation(FName ModID);

    /** Active mods replicated to clients */
    UPROPERTY(ReplicatedUsing=OnRep_ActiveMods)
    TArray<FActiveModEntry> ActiveMods;

    UFUNCTION()
    void OnRep_ActiveMods();

    /** Internal helpers */
    void ApplyStatModifiers(const FPlayerModDefinition& Def, int32 Stacks);
    void RemoveStatModifiers(const FPlayerModDefinition& Def);
    FActiveModEntry* FindActiveEntry(FName ModID);
};


#include "PlayerModComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerModData.h"
#include "GameFramework/Actor.h"

UPlayerModComponent::UPlayerModComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UPlayerModComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DORELIFETIME(UPlayerModComponent, ActiveMods);
}

bool UPlayerModComponent::InstallMod(UPlayerModDataAsset* ModAsset)
{
    if (!ModAsset) return false;

    if (GetOwnerRole() < ROLE_Authority)
    {
        // client requests server to install
        ServerRequestInstall(ModAsset);
        return true;
    }

    // Server-side installation logic:
    const FPlayerModDefinition& Def = ModAsset->ModDefinition;
    if (FindActiveEntry(Def.ModID)) return false; // already installed or cannot stack

    // Check installation risk, power/slot availability etc. (hook in your game rules)
    FActiveModEntry NewEntry;
    NewEntry.ModAsset = ModAsset;
    NewEntry.TimeRemaining = (Def.Type == EModType::Active) ? 0.f : -1.f; // passive = no duration
    NewEntry.Stacks = 1;
    ActiveMods.Add(NewEntry);

    // Apply passive stat modifiers immediately
    if (Def.Type == EModType::Passive)
    {
        ApplyStatModifiers(Def, 1);
    }

    return true;
}

bool UPlayerModComponent::UninstallMod(FName ModID)
{
    FActiveModEntry* Entry = FindActiveEntry(ModID);
    if (!Entry) return false;

    // if passive, remove stat mods
    if (Entry->ModAsset.IsValid())
    {
        UPlayerModDataAsset* Asset = Entry->ModAsset.Get();
        if (Asset && Asset->ModDefinition.Type == EModType::Passive)
        {
            RemoveStatModifiers(Asset->ModDefinition);
        }
    }
    ActiveMods.RemoveAll([ModID](const FActiveModEntry& E){ return E.ModAsset.IsValid() && E.ModAsset->ModDefinition.ModID == ModID; });
    return true;
}

bool UPlayerModComponent::IsModInstalled(FName ModID) const
{
    for (const FActiveModEntry& E : ActiveMods)
    {
        if (E.ModAsset.IsValid() && E.ModAsset->ModDefinition.ModID == ModID)
            return true;
    }
    return false;
}

bool UPlayerModComponent::ActivateMod(FName ModID)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        ServerRequestActivate(ModID);
        return true;
    }

    FActiveModEntry* Entry = FindActiveEntry(ModID);
    if (!Entry || !Entry->ModAsset.IsValid()) return false;

    UPlayerModDataAsset* Asset = Entry->ModAsset.Get();
    if (!Asset) return false;

    const FPlayerModDefinition& Def = Asset->ModDefinition;
    if (Def.Type != EModType::Active) return false;

    // Example: set duration and apply stat modifiers temporarily
    Entry->TimeRemaining = Def.Cooldown; // use cooldown as duration example
    ApplyStatModifiers(Def, Entry->Stacks);

    // TODO: trigger VFX, sound, gameplay events via BlueprintImplementableEvent or delegates

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
    if (GetOwnerRole() == ROLE_Authority)
    {
        bool bChanged = false;
        for (int32 i = ActiveMods.Num() - 1; i >= 0; --i)
        {
            FActiveModEntry& E = ActiveMods[i];
            if (E.TimeRemaining > 0.f)
            {
                E.TimeRemaining -= DeltaTime;
                if (E.TimeRemaining <= 0.f)
                {
                    // expiration: remove temp modifiers
                    if (E.ModAsset.IsValid())
                    {
                        UPlayerModDataAsset* Asset = E.ModAsset.Get();
                        if (Asset)
                        {
                            RemoveStatModifiers(Asset->ModDefinition);
                        }
                    }
                    // For active mods we may keep the entry to track cooldowns or remove it here
                    // Remove entry if single-use active
                    ActiveMods.RemoveAt(i);
                    bChanged = true;
                }
            }
        }
        if (bChanged)
        {
            // ActiveMods changed; replication will notify clients via OnRep_ActiveMods
        }
    }
}

void UPlayerModComponent::OnRep_ActiveMods()
{
    // Clients can update HUD/VFX on mod list change
}

void UPlayerModComponent::ApplyStatModifiers(const FPlayerModDefinition& Def, int32 Stacks)
{
    // Hook: modify the owner's stats (pseudo)
    // Example: find a stats component and apply additive/multiplier values
    // This is game-specific; expose blueprint events or call a StatsComponent API
    // e.g. Owner->FindComponentByClass<UPlayerStatsComponent>()->ModifyStat(...)
}

void UPlayerModComponent::RemoveStatModifiers(const FPlayerModDefinition& Def)
{
    // Reverse ApplyStatModifiers
}

FActiveModEntry* UPlayerModComponent::FindActiveEntry(FName ModID)
{
    for (FActiveModEntry& E : ActiveMods)
    {
        if (E.ModAsset.IsValid() && E.ModAsset->ModDefinition.ModID == ModID)
            return &E;
    }
    return nullptr;
}

// --- Blueprint helper implementations -------------------------------------------------

bool UPlayerModComponent::IsModInstalledByID(FName ModID) const
{
    return IsModInstalled(ModID);
}

float UPlayerModComponent::GetModCooldownPercent(FName ModID) const
{
    const FActiveModEntry* Entry = nullptr;
    for (const FActiveModEntry& E : ActiveMods)
    {
        if (E.ModAsset.IsValid() && E.ModAsset->ModDefinition.ModID == ModID)
        {
            Entry = &E;
            break;
        }
    }
    if (!Entry) return 0.f;

    if (!Entry->ModAsset.IsValid()) return 0.f;
    const UPlayerModDataAsset* Asset = Entry->ModAsset.Get();
    if (!Asset) return 0.f;

    float Total = Asset->ModDefinition.Cooldown;
    if (Total <= 0.f) return 0.f;

    float Remaining = FMath::Max(0.f, Entry->TimeRemaining);
    float Progress = FMath::Clamp((Total - Remaining) / Total, 0.f, 1.f);
    return Progress;
}

float UPlayerModComponent::GetModRemainingTime(FName ModID) const
{
    for (const FActiveModEntry& E : ActiveMods)
    {
        if (E.ModAsset.IsValid() && E.ModAsset->ModDefinition.ModID == ModID)
            return FMath::Max(0.f, E.TimeRemaining);
    }
    return 0.f;
}