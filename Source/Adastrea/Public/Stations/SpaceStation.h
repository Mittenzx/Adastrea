#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stations/SpaceStationModule.h"
#include "Interfaces/IDamageable.h"
#include "Interfaces/ITargetable.h"
#include "Interfaces/IFactionMember.h"
#include "SpaceStation.generated.h"

/**
 * Core space station actor with modular construction system
 * 
 * Space stations are large structures that can be built from individual modules.
 * This class manages the collection of modules and provides functionality for
 * adding, removing, and querying modules.
 * 
 * Features:
 * - Dynamic module management (add/remove/move)
 * - Module filtering by type
 * 
 * Implements:
 * - IDamageable: Can receive damage from weapons
 * - ITargetable: Can be targeted by weapons and sensors
 * - IFactionMember: Belongs to a faction for diplomacy and AI
 * 
 * Usage:
 * 1. Create Blueprint based on this class
 * 2. Add modules using AddModule() or AddModuleAtLocation()
 * 3. Query modules with GetModules() or GetModulesByType()
 * 
 * See Also: STATION_EDITOR_README.md for detailed implementation guide
 */
UCLASS()
class ADASTREA_API ASpaceStation : public AActor, public IDamageable, public ITargetable, public IFactionMember
{
    GENERATED_BODY()

public:
    ASpaceStation();

    // Array of attached modules
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Station")
    TArray<ASpaceStationModule*> Modules;

    /**
     * Add a module to the station (simple version without position)
     * @param Module The module to add
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    void AddModule(ASpaceStationModule* Module);

    /**
     * Add a module to the station at a specific location
     * @param Module The module to add
     * @param RelativeLocation Position relative to the station
     * @return True if module was successfully added
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    bool AddModuleAtLocation(ASpaceStationModule* Module, FVector RelativeLocation);

    /**
     * Remove a module from the station
     * @param Module The module to remove
     * @return True if module was found and removed
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    bool RemoveModule(ASpaceStationModule* Module);

    /**
     * Move a module to a new location within the station
     * @param Module The module to move
     * @param NewRelativeLocation New position relative to the station
     * @return True if module was found and moved
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);

    /**
     * Get all attached modules
     * @return Array of all modules attached to this station
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModules() const;

    /**
     * Get all modules of a specific type
     * @param ModuleType The type identifier to filter by
     * @return Array of matching modules
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModulesByType(const FString& ModuleType) const;

    /**
     * Get the number of attached modules
     * @return Total count of modules
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    int32 GetModuleCount() const;

    /**
     * Set the faction that owns this station
     * @param NewFaction The faction to assign to this station
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    void SetFaction(class UFactionDataAsset* NewFaction);

    // ====================
    // INTERFACE IMPLEMENTATIONS
    // ====================

    // IDamageable Interface
    virtual float ApplyDamage_Implementation(float Damage, EDamageType DamageType, AActor* Instigator, AActor* DamageCauser) override;
    virtual bool CanTakeDamage_Implementation() const override;
    virtual float GetHealthPercentage_Implementation() const override;
    virtual bool IsDestroyed_Implementation() const override;
    virtual float GetMaxHealth_Implementation() const override;
    virtual float GetCurrentHealth_Implementation() const override;

    // ITargetable Interface
    virtual bool CanBeTargeted_Implementation() const override;
    virtual int32 GetTargetPriority_Implementation() const override;
    virtual FText GetTargetDisplayName_Implementation() const override;
    virtual UTexture2D* GetTargetIcon_Implementation() const override;
    virtual FVector GetAimPoint_Implementation() const override;
    virtual float GetTargetSignature_Implementation() const override;
    virtual float GetDistanceFromLocation_Implementation(FVector FromLocation) const override;
    virtual bool IsHostileToActor_Implementation(AActor* Observer) const override;

    // IFactionMember Interface
    virtual UFactionDataAsset* GetFaction_Implementation() const override;
    virtual bool IsAlliedWith_Implementation(const TScriptInterface<IFactionMember>& Other) const override;
    virtual bool IsHostileTo_Implementation(const TScriptInterface<IFactionMember>& Other) const override;
    virtual int32 GetRelationshipWith_Implementation(const TScriptInterface<IFactionMember>& Other) const override;
    virtual bool IsNeutral_Implementation() const override;
    virtual FText GetFactionDisplayName_Implementation() const override;
    virtual bool CanEngageInCombat_Implementation() const override;
    virtual float GetTradePriceModifier_Implementation(UFactionDataAsset* TraderFaction) const override;

protected:
    virtual void BeginPlay() override;

    /** The faction that owns this station (can be null for neutral stations) */
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Station")
    class UFactionDataAsset* OwningFaction;

    /** Current structural integrity (health) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Status")
    float CurrentStructuralIntegrity;

    /** Maximum structural integrity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Status")
    float MaxStructuralIntegrity;

    /** Flag indicating if station is destroyed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station Status")
    bool bIsDestroyed;

    /** Display name for this station */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Station")
    FText StationName;
};
