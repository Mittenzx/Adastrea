// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Interfaces/IDamageable.h"
#include "Interfaces/ITargetable.h"
#include "StationModuleTypes.h"
#include "SpaceStationModule.generated.h"

// Forward declarations removed - faction system removed per Trade Simulator MVP scope

/**
 * Base class for all space station modules
 * 
 * Modules are the building blocks of space stations. Each module represents a
 * functional component that can be attached to a station.
 * 
 * Features:
 * - Type classification for module identification
 * - Power requirement tracking
 * - Group categorization for organization
 * - Faction assignment (modules can have different owners than the parent station)
 * 
 * Implements:
 * - IDamageable: Can receive damage (may cause module destruction)
 * - ITargetable: Can be targeted individually by precise weapons
 * 
 * Usage:
 * - Create Blueprint based on this class for specific module types
 * - Configure ModuleType, ModulePower, and ModuleGroup
 * - Optionally assign a ModuleFaction for multi-faction stations
 * - Add to ASpaceStation using AddModule() or AddModuleAtLocation()
 */
UCLASS()
class ADASTREA_API ASpaceStationModule : public AActor, public IDamageable, public ITargetable
{
    GENERATED_BODY()

public:
    ASpaceStationModule();

    // Type identifier for this module (e.g., "Docking Bay", "Reactor Core")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
    FString ModuleType;

    // Power consumption or generation (negative = generates, positive = consumes)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
    float ModulePower;

    // Functional group for organization and filtering
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
    EStationModuleGroup ModuleGroup;

    // REMOVED: Faction system not needed for Trade Simulator MVP
    // Module ownership handled at station level if needed

    /**
     * Get the type identifier for this module
     * @return Module type string (e.g., "Docking Bay", "Reactor Core")
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module")
    FString GetModuleType() const { return ModuleType; }

    /**
     * Get the power consumption or generation of this module
     * @return Power value (positive = consumes, negative = generates)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module")
    float GetModulePower() const { return ModulePower; }

    /**
     * Get the functional group this module belongs to
     * @return Module group enum value
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module")
    EStationModuleGroup GetModuleGroup() const { return ModuleGroup; }

    /**
     * Check if this module generates power (negative power value)
     * @return True if module generates power, false if it consumes
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module")
    bool IsGeneratingPower() const { return ModulePower < 0.0f; }

    /**
     * Get the mesh component for this module
     * @return The static mesh component used for visual representation
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module")
    UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }

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

protected:
    /** Static mesh component for visual representation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    /** Current module integrity (health) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Status")
    float CurrentModuleIntegrity;

    /** Maximum module integrity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Status")
    float MaxModuleIntegrity;

    /** Flag indicating if module is destroyed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Module Status")
    bool bIsDestroyed;
};