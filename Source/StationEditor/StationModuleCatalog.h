// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Stations/StationModuleTypes.h"
#include "StationModuleCatalog.generated.h"

// Forward declarations
class ASpaceStationModule;

/**
 * Build cost structure for station modules
 * Contains all resources required to construct a module
 */
USTRUCT(BlueprintType)
struct STATIONEDITOR_API FStationBuildCost
{
	GENERATED_BODY()

	/** Base credit cost for this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Build Cost", meta=(ClampMin=0))
	int32 Credits = 0;

	/** Material requirements - Maps resource name to quantity required */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Build Cost")
	TMap<FName, int32> Materials;

	/** Time in seconds required to build this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Build Cost", meta=(ClampMin=0.0f, UIMin=0.0f))
	float BuildTime = 60.0f;

	/** Constructor with default values */
	FStationBuildCost()
		: Credits(0)
		, BuildTime(60.0f)
	{
	}
};

/**
 * Entry in the module catalog describing an available module type
 */
USTRUCT(BlueprintType)
struct STATIONEDITOR_API FStationModuleEntry
{
	GENERATED_BODY()

	/** The class to spawn for this module type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Entry")
	TSubclassOf<ASpaceStationModule> ModuleClass;

	/** Display name shown in the editor UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Entry")
	FText DisplayName;

	/** Description of the module's function */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Entry", meta=(MultiLine=true))
	FText Description;

	/** Resources required to build this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Entry")
	FStationBuildCost BuildCost;

	/** Preview mesh to display when placing this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Entry")
	TSoftObjectPtr<UStaticMesh> PreviewMesh;

	/** Minimum technology level required to unlock this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Entry", meta=(ClampMin=1, ClampMax=10, UIMin=1, UIMax=10))
	int32 RequiredTechLevel = 1;

	/** Functional group this module belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Entry")
	EStationModuleGroup ModuleGroup = EStationModuleGroup::Other;

	/** Constructor with default values */
	FStationModuleEntry()
		: RequiredTechLevel(1)
		, ModuleGroup(EStationModuleGroup::Other)
	{
	}
};

/**
 * Station Module Catalog - Data Asset containing all available module types
 * 
 * This Data Asset serves as a registry of all module types that can be placed
 * in the station editor. Designers can create multiple catalogs for different
 * scenarios (e.g., different tech levels, faction-specific modules).
 * 
 * Usage:
 * 1. Create a Blueprint Data Asset based on this class
 * 2. Add entries for each available module type
 * 3. Configure display names, descriptions, costs, and requirements
 * 4. Reference this catalog in the StationEditorManager
 * 
 * @see UStationEditorManager
 * @see FStationModuleEntry
 */
UCLASS(BlueprintType)
class STATIONEDITOR_API UStationModuleCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	UStationModuleCatalog();

	/** Array of all available module entries in this catalog */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module Catalog")
	TArray<FStationModuleEntry> ModuleEntries;

	/**
	 * Get all module entries in the catalog
	 * @return Array of all module entries
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module Catalog")
	TArray<FStationModuleEntry> GetAllModules() const;

	/**
	 * Get module entries filtered by module group
	 * @param Group The module group to filter by (use EStationModuleGroup::All for no filtering)
	 * @return Array of module entries matching the specified group
	 */
	UFUNCTION(BlueprintCallable, Category="Module Catalog")
	TArray<FStationModuleEntry> GetModulesByGroup(EStationModuleGroup Group) const;

	/**
	 * Get module entries filtered by maximum tech level
	 * @param MaxTechLevel The maximum tech level to include (1-10)
	 * @return Array of module entries with required tech level <= MaxTechLevel
	 */
	UFUNCTION(BlueprintCallable, Category="Module Catalog")
	TArray<FStationModuleEntry> GetModulesByTechLevel(int32 MaxTechLevel) const;

	/**
	 * Get module entries filtered by both group and tech level
	 * @param Group The module group to filter by (use EStationModuleGroup::All for no group filtering)
	 * @param MaxTechLevel The maximum tech level to include
	 * @return Array of module entries matching both criteria
	 */
	UFUNCTION(BlueprintCallable, Category="Module Catalog")
	TArray<FStationModuleEntry> GetModulesFiltered(EStationModuleGroup Group, int32 MaxTechLevel) const;

	/**
	 * Find a module entry by its class
	 * @param ModuleClass The class to search for
	 * @param OutEntry The found entry (only valid if function returns true)
	 * @return True if entry was found, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category="Module Catalog")
	bool FindModuleByClass(TSubclassOf<ASpaceStationModule> ModuleClass, FStationModuleEntry& OutEntry) const;

	/**
	 * Get the number of modules in the catalog
	 * @return Total count of module entries
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module Catalog")
	int32 GetModuleCount() const;

	/**
	 * Check if a module class exists in the catalog
	 * @param ModuleClass The class to check
	 * @return True if the class exists in the catalog
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module Catalog")
	bool ContainsModule(TSubclassOf<ASpaceStationModule> ModuleClass) const;
};
