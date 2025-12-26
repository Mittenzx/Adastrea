// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BillboardComponent.h"
#include "SpaceSectorMap.generated.h"

/**
 * Space Sector Map marker for 200km x 200km open world sectors
 * 
 * This actor serves as a sector boundary marker for large-scale open world space maps,
 * following X4: Foundations sector sizing conventions. Each sector represents a
 * 200,000m x 200,000m area of space that can be populated with stations, ships,
 * asteroids, and other celestial objects.
 * 
 * Features:
 * - Editor visualization with billboard component
 * - Blueprint-friendly configuration
 * - Sector boundary helpers
 * - Support for procedural generation workflows
 * - Named sectors for organization
 * 
 * Usage:
 * 1. Place ASpaceSectorMap actor in your level
 * 2. Set the SectorName to identify this sector
 * 3. Use GetSectorBounds() to query the sector's boundaries
 * 4. Position objects within the sector using helper functions
 * 5. Create multiple sectors to build a connected universe
 * 
 * Sector Size: 200km x 200km (200,000 x 200,000 Unreal Units)
 * - This matches X4's sector scale for familiar open-world feel
 * - Large enough for diverse content placement
 * - Small enough to maintain reasonable object density
 * 
 * See Also: Docs/EmptyMapSetup.md for complete map creation workflow
 */
UCLASS()
class ADASTREA_API ASpaceSectorMap : public AActor
{
	GENERATED_BODY()

public:
	ASpaceSectorMap();

	// Sector size in Unreal Units (centimeters)
	// 200 km = 200 km × 1000 m/km = 200,000 m = 200,000 m × 100 cm/m = 20,000,000 cm
	static constexpr float SectorSize = 20000000.0f;

	/**
	 * Name of this sector for identification and organization
	 * Examples: "Alpha Centauri", "Trading Hub Alpha", "Asteroid Field 7"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector Info")
	FText SectorName;

	/**
	 * Optional description for this sector
	 * Use for notes about planned content, themes, or special features
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector Info", meta=(MultiLine=true))
	FText Description;

	/**
	 * Billboard component for editor visualization
	 * Visible in editor to help identify and select sector markers
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UBillboardComponent* SectorMarker;

	/**
	 * Get the world-space bounds of this sector as a box
	 * @return Box representing the sector's 200km x 200km boundaries
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector")
	FBox GetSectorBounds() const;

	/**
	 * Get the half-size of the sector (100km from center)
	 * Useful for random placement and bounds checking
	 * @return Half the sector size (10,000,000 units = 100km)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector")
	float GetSectorHalfSize() const;

	/**
	 * Check if a world position is within this sector's boundaries
	 * @param WorldPosition The position to check
	 * @return True if the position is within the sector bounds
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector")
	bool IsPositionInSector(const FVector& WorldPosition) const;

	/**
	 * Get a random position within this sector
	 * Useful for procedural content placement
	 * @return Random world position within sector bounds
	 */
	UFUNCTION(BlueprintCallable, Category="Sector")
	FVector GetRandomPositionInSector() const;

	/**
	 * Get the center position of this sector in world space
	 * @return Center position of the sector
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector")
	FVector GetSectorCenter() const;

	/**
	 * Get grid coordinates for this sector
	 * Used for universe grid positioning
	 * @return Grid coordinates based on sector size
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector")
	FIntVector GetGridCoordinates() const;

	/**
	 * Get all actors within this sector
	 * @param ActorClass Optional class filter (nullptr for all actors)
	 * @return Array of actors within sector bounds
	 */
	UFUNCTION(BlueprintCallable, Category="Sector")
	TArray<AActor*> GetActorsInSector(TSubclassOf<AActor> ActorClass = nullptr) const;

	/**
	 * Get count of actors in sector
	 * Optimized version that doesn't return actor array
	 * @param ActorClass Optional class filter
	 * @return Number of actors in sector
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector")
	int32 GetActorCountInSector(TSubclassOf<AActor> ActorClass = nullptr) const;

	/**
	 * Find neighboring sectors (adjacent grid positions)
	 * @return Array of sectors adjacent to this one
	 */
	UFUNCTION(BlueprintCallable, Category="Sector")
	TArray<ASpaceSectorMap*> GetNeighboringSectors() const;

	/**
	 * Calculate distance to another sector
	 * @param OtherSector The sector to measure distance to
	 * @return Distance in Unreal Units (centimeters)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector")
	float GetDistanceToSector(const ASpaceSectorMap* OtherSector) const;

	/**
	 * Validate sector positioning and configuration
	 * Checks for overlapping sectors and invalid positions
	 * @return True if sector configuration is valid
	 */
	UFUNCTION(BlueprintCallable, Category="Sector|Debug")
	bool ValidateSectorConfiguration() const;

	/**
	 * Get debug information about this sector
	 * @return Debug string with sector details
	 */
	UFUNCTION(BlueprintCallable, Category="Sector|Debug")
	FString GetDebugInfo() const;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif

private:
	/** Cache for discovered neighboring sectors */
	UPROPERTY()
	TArray<ASpaceSectorMap*> CachedNeighboringSectors;

	/** Whether neighbor cache is dirty and needs refresh */
	UPROPERTY()
	bool bNeighborCacheDirty;

	/** Refresh the neighbor cache */
	void RefreshNeighborCache();
};
