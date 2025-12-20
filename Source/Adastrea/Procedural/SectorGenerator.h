// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceObjectDefinition.h"
#include "SectorGenerator.generated.h"

// Forward declarations
class ASpaceSectorMap;
class USectorGeneratorConfig;

/**
 * Sector Generator Actor
 * 
 * Procedurally populates a sector with stations, ships, asteroids, clouds, and other space objects.
 * Uses Data Asset configuration for designer-friendly content generation.
 * 
 * Features:
 * - Data-driven configuration via USectorGeneratorConfig
 * - Multiple distribution patterns (random, clustered, grid, orbital)
 * - Faction-based ownership assignment
 * - Procedural name generation
 * - Collision avoidance and spacing rules
 * - Preview mode for testing
 * - Blueprint-friendly API
 * 
 * Usage:
 * 1. Place ASectorGenerator in your level
 * 2. Assign target ASpaceSectorMap
 * 3. Set GeneratorConfig Data Asset
 * 4. Call GenerateSector() in editor or at runtime
 * 5. Use ClearSector() to remove generated content
 * 
 * Editor Workflow:
 * - Use "Generate" button in Details panel
 * - Preview generation before committing
 * - Tweak config and regenerate until satisfied
 * - Clear and regenerate as needed
 */
UCLASS(BlueprintType)
class ADASTREA_API ASectorGenerator : public AActor
{
	GENERATED_BODY()

public:
	ASectorGenerator();

	// ====================
	// Configuration
	// ====================

	/** Target sector to populate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	ASpaceSectorMap* TargetSector;

	/** Configuration for generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	USectorGeneratorConfig* GeneratorConfig;

	/** Whether to show debug visualizations during generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator|Debug")
	bool bShowDebugVisuals;

	/** Whether to generate in preview mode (doesn't save actors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator|Debug")
	bool bPreviewMode;

	// ====================
	// Generation Functions
	// ====================

	/**
	 * Generate sector content based on configuration
	 * @return True if generation succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Sector Generator", meta = (CallInEditor = "true"))
	bool GenerateSector();

	/**
	 * Clear all generated content from sector
	 * Only removes actors that were spawned by this generator
	 */
	UFUNCTION(BlueprintCallable, Category = "Sector Generator", meta = (CallInEditor = "true"))
	void ClearSector();

	/**
	 * Regenerate sector (clear then generate)
	 * @return True if regeneration succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Sector Generator", meta = (CallInEditor = "true"))
	bool RegenerateSector();

	/**
	 * Validate generator configuration
	 * @param OutErrors Array to fill with validation errors
	 * @return True if configuration is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Sector Generator")
	bool ValidateConfiguration(TArray<FText>& OutErrors) const;

	/**
	 * Get count of currently generated objects
	 * @return Number of objects spawned by this generator
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sector Generator")
	int32 GetGeneratedObjectCount() const;

	/**
	 * Get list of all generated actors
	 * @return Array of actors spawned by this generator
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sector Generator")
	TArray<AActor*> GetGeneratedActors() const;

	// ====================
	// Generation Events (BlueprintNativeEvent for extensibility)
	// ====================

	/**
	 * Called before sector generation begins
	 * Override in Blueprint to add custom pre-generation logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Sector Generator|Events")
	void OnPreGeneration();

	/**
	 * Called after sector generation completes
	 * Override in Blueprint to add custom post-generation logic
	 * @param bSuccess Whether generation succeeded
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Sector Generator|Events")
	void OnPostGeneration(bool bSuccess);

	/**
	 * Called when an object is spawned
	 * Override in Blueprint to customize spawned objects
	 * @param SpawnedActor The actor that was just spawned
	 * @param Definition The definition used to spawn it
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Sector Generator|Events")
	void OnObjectSpawned(AActor* SpawnedActor, const FSpaceObjectDefinition& Definition);

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ====================
	// Internal Generation Logic
	// ====================

	/**
	 * Spawn objects based on a definition
	 * @param Definition Object definition to spawn
	 * @param OutSpawnedActors Array to fill with spawned actors
	 * @return Number of objects successfully spawned
	 */
	int32 SpawnObjectsFromDefinition(const FSpaceObjectDefinition& Definition, TArray<AActor*>& OutSpawnedActors);

	/**
	 * Find valid spawn location based on distribution pattern
	 * @param Definition Object definition containing placement rules
	 * @param OutLocation Output location if found
	 * @param OutRotation Output rotation if found
	 * @return True if valid location found
	 */
	bool FindValidSpawnLocation(const FSpaceObjectDefinition& Definition, FVector& OutLocation, FRotator& OutRotation);

	/**
	 * Check if location is valid (no overlaps, within bounds)
	 * @param Location Location to check
	 * @param MinSpacing Minimum spacing from other objects
	 * @return True if location is valid
	 */
	bool IsLocationValid(const FVector& Location, float MinSpacing) const;

	/**
	 * Generate positions based on distribution pattern
	 * @param Pattern Distribution pattern to use
	 * @param Count Number of positions to generate
	 * @param MinDistance Minimum distance from center
	 * @param MaxDistance Maximum distance from center
	 * @return Array of positions
	 */
	TArray<FVector> GeneratePositionsByPattern(EDistributionPattern Pattern, int32 Count, float MinDistance, float MaxDistance);

	/**
	 * Apply faction ownership to spawned actor
	 * @param Actor Actor to assign faction to
	 * @param Faction Faction to assign
	 */
	void ApplyFactionOwnership(AActor* Actor, UFactionDataAsset* Faction);

	/**
	 * Generate and assign sector name
	 */
	void GenerateSectorName();

	// ====================
	// State Tracking
	// ====================

	/** Array of actors spawned by this generator (for cleanup) */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> GeneratedActors;

	/** Random stream for reproducible generation */
	UPROPERTY()
	FRandomStream RandomStream;

	/** Flag indicating if generation is in progress */
	UPROPERTY()
	bool bIsGenerating;

	/** Cached sector bounds for optimization */
	FBox CachedSectorBounds;

	/** Track occupied positions for spacing checks */
	TArray<FVector> OccupiedPositions;
};
