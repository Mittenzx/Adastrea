// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpaceObjectDefinition.h"
#include "SectorGeneratorConfig.generated.h"

/**
 * Sector naming theme for procedural name generation
 */
UENUM(BlueprintType)
enum class ESectorNamingTheme : uint8
{
	Military		UMETA(DisplayName = "Military (Tactical designations)"),
	Mining			UMETA(DisplayName = "Mining (Resource-focused)"),
	Trade			UMETA(DisplayName = "Trade (Commerce-focused)"),
	Scientific		UMETA(DisplayName = "Scientific (Research-focused)"),
	Frontier		UMETA(DisplayName = "Frontier (Colonial)"),
	Greek			UMETA(DisplayName = "Greek Letters (Alpha, Beta, etc.)"),
	Numeric			UMETA(DisplayName = "Numeric (Sector-001, etc.)"),
	Descriptive		UMETA(DisplayName = "Descriptive (Based on content)")
};

/**
 * Configuration Data Asset for Sector Generator
 * 
 * Defines all settings for procedurally generating sector content.
 * Designers can create multiple configs for different sector types
 * (e.g., safe zones, mining sectors, combat zones, etc.)
 * 
 * Usage:
 * 1. Create Blueprint based on this class
 * 2. Configure object definitions for what to spawn
 * 3. Set sector parameters (size, density, theme)
 * 4. Reference from ASectorGenerator actor
 * 5. Generate sector content in-editor or at runtime
 */
UCLASS(BlueprintType)
class ADASTREA_API USectorGeneratorConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USectorGeneratorConfig();

	// ====================
	// Basic Info
	// ====================

	/** Display name for this configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
	FText ConfigName;

	/** Description of what this config generates */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info", meta = (MultiLine = true))
	FText Description;

	// ====================
	// Sector Settings
	// ====================

	/** Naming theme for generated sector */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sector Settings")
	ESectorNamingTheme NamingTheme;

	/** Optional custom sector name (if empty, will be generated) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sector Settings")
	FText CustomSectorName;

	/** Random seed for reproducible generation (0 = use random) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sector Settings", meta = (ClampMin = "0"))
	int32 RandomSeed;

	/** Overall object density multiplier (1.0 = default, 2.0 = twice as many objects) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sector Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DensityMultiplier;

	// ====================
	// Object Definitions
	// ====================

	/** List of object types to spawn in the sector */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Definitions")
	TArray<FSpaceObjectDefinition> ObjectDefinitions;

	// ====================
	// Generation Rules
	// ====================

	/** Minimum distance between spawned objects (collision avoidance) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation Rules", meta = (ClampMin = "0"))
	float MinObjectSpacing;

	/** Maximum attempts to find valid spawn location per object */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation Rules", meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxSpawnAttempts;

	/** Whether to create a central station/hub */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation Rules")
	bool bCreateCentralHub;

	/** Central hub actor class (if bCreateCentralHub is true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation Rules", meta = (EditCondition = "bCreateCentralHub"))
	TSubclassOf<AActor> CentralHubClass;

	/** Whether to organize objects into zones/clusters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation Rules")
	bool bCreateZones;

	/** Number of zones if bCreateZones is true */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation Rules", meta = (EditCondition = "bCreateZones", ClampMin = "1", ClampMax = "10"))
	int32 NumZones;

	// ====================
	// Blueprint Functions
	// ====================

	/**
	 * Get total object count this config will generate
	 * @return Approximate total number of objects (sum of max counts)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sector Generator")
	int32 GetTotalObjectCount() const;

	/**
	 * Validate configuration for errors
	 * @param OutErrors Array to fill with validation errors
	 * @return True if configuration is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Sector Generator")
	bool ValidateConfiguration(TArray<FText>& OutErrors) const;

	/**
	 * Get object definitions of a specific type
	 * @param ObjectType Type to filter by
	 * @return Array of matching object definitions
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sector Generator")
	TArray<FSpaceObjectDefinition> GetObjectDefinitionsByType(ESpaceObjectType ObjectType) const;

#if WITH_EDITOR
	/**
	 * Validate Data Asset properties
	 */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
