#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerModData.h"
#include "PlayerModComponent.generated.h"

class UPlayerModDataAsset;

/**
 * Active mod entry structure for tracking installed player mods
 */
USTRUCT(BlueprintType)
struct FActiveModEntry
{
    GENERATED_BODY()

    // Reference to the mod data asset
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mod")
    TSoftObjectPtr<UPlayerModDataAsset> ModAsset;

    // Time remaining for active mod duration or cooldown
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mod")
    float TimeRemaining = 0.f;

    // Number of stacks for this mod
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mod")
    int32 Stacks = 1;
};

/**
 * Player modification component
 * Handles installation, activation, and management of player mods
 * 
 * Features:
 * - Server-authoritative mod installation
 * - Active and passive mod types
 * - Duration and cooldown tracking
 * - Network replication support
 * 
 * Usage:
 * 1. Attach to player actor
 * 2. Call InstallMod() to add a mod
 * 3. Call ActivateMod() for active-type mods
 * 4. Query mod status via helper functions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UPlayerModComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerModComponent();

    /**
     * Installs a mod (server authoritative)
     * @param ModAsset The mod data asset to install
     * @return True if install request was accepted
     */
    UFUNCTION(BlueprintCallable, Category = "Mods")
    bool InstallMod(UPlayerModDataAsset* ModAsset);

    /**
     * Uninstall a mod by ID
     * @param ModID The identifier of the mod to uninstall
     * @return True if mod was found and uninstalled
     */
    UFUNCTION(BlueprintCallable, Category = "Mods")
    bool UninstallMod(FName ModID);

    /**
     * Check if a mod is currently installed
     * @param ModID The identifier of the mod to check
     * @return True if the mod is installed
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mods")
    bool IsModInstalled(FName ModID) const;

    /**
     * Activates an installed mod (for Active type mods)
     * @param ModID The identifier of the mod to activate
     * @return True if activation was successful
     */
    UFUNCTION(BlueprintCallable, Category = "Mods")
    bool ActivateMod(FName ModID);

    /**
     * Tick component override for updating mod durations
     */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ====================
    // Blueprint Helper Functions for UI
    // ====================

    /**
     * Blueprint-friendly check if mod is installed by ModID
     * @param ModID The identifier of the mod to check
     * @return True if the mod is installed
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mods|UI")
    bool IsModInstalledByID(FName ModID) const;

    /**
     * Get cooldown progress as percentage (0 = ready, 1 = full cooldown)
     * @param ModID The identifier of the mod
     * @return Cooldown progress from 0 to 1
     */
    UFUNCTION(BlueprintCallable, Category = "Mods|UI")
    float GetModCooldownPercent(FName ModID) const;

    /**
     * Get remaining time for active cooldown or duration
     * @param ModID The identifier of the mod
     * @return Remaining time in seconds (0 if none)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mods|UI")
    float GetModRemainingTime(FName ModID) const;

protected:
    /**
     * Setup replication properties
     */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /**
     * Server RPC to request mod installation
     * @param ModAsset The mod data asset to install
     */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestInstall(UPlayerModDataAsset* ModAsset);
    bool ServerRequestInstall_Validate(UPlayerModDataAsset* ModAsset) { return ModAsset != nullptr; }
    void ServerRequestInstall_Implementation(UPlayerModDataAsset* ModAsset);

    /**
     * Server RPC to request mod activation
     * @param ModID The identifier of the mod to activate
     */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestActivate(FName ModID);
    bool ServerRequestActivate_Validate(FName ModID) { return !ModID.IsNone(); }
    void ServerRequestActivate_Implementation(FName ModID);

    /**
     * Active mods replicated to clients
     */
    UPROPERTY(ReplicatedUsing=OnRep_ActiveMods)
    TArray<FActiveModEntry> ActiveMods;

    /**
     * Replication notification for active mods
     */
    UFUNCTION()
    void OnRep_ActiveMods();

    /**
     * Apply stat modifiers from a mod
     * @param Def The mod definition containing modifier values
     * @param Stacks Number of stacks to apply
     */
    void ApplyStatModifiers(const FPlayerModDefinition& Def, int32 Stacks);

    /**
     * Remove stat modifiers from a mod
     * @param Def The mod definition containing modifier values
     */
    void RemoveStatModifiers(const FPlayerModDefinition& Def);

    /**
     * Find an active mod entry by ID
     * @param ModID The identifier of the mod to find
     * @return Pointer to the entry if found, nullptr otherwise
     */
    FActiveModEntry* FindActiveEntry(FName ModID);
};

