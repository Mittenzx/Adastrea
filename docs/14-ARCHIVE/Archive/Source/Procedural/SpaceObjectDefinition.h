// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpaceObjectDefinition.generated.h"

// Forward declarations
class UFactionDataAsset;
class USpaceshipDataAsset;

/**
 * Type of space object that can be spawned in a sector
 */
UENUM(BlueprintType)
enum class ESpaceObjectType : uint8
{
	Station			UMETA(DisplayName = "Space Station"),
	Asteroid		UMETA(DisplayName = "Asteroid"),
	AsteroidField	UMETA(DisplayName = "Asteroid Field"),
	Cloud			UMETA(DisplayName = "Space Cloud/Nebula"),
	Ship			UMETA(DisplayName = "Spaceship"),
	Debris			UMETA(DisplayName = "Space Debris"),
	Anomaly			UMETA(DisplayName = "Anomaly"),
	Waypoint		UMETA(DisplayName = "Navigation Waypoint")
};

/**
 * Distribution pattern for object placement
 */
UENUM(BlueprintType)
enum class EDistributionPattern : uint8
{
	Random			UMETA(DisplayName = "Random (Uniform Distribution)"),
	Clustered		UMETA(DisplayName = "Clustered (Groups)"),
	Grid			UMETA(DisplayName = "Grid Pattern"),
	Orbital			UMETA(DisplayName = "Orbital (Around Center)"),
	Scattered		UMETA(DisplayName = "Scattered (Wide Distribution)")
};

/**
 * Definition for a type of space object that can be spawned
 * Used by the sector generator to configure what objects to create
 */
USTRUCT(BlueprintType)
struct FSpaceObjectDefinition
{
	GENERATED_BODY()

	/** Type of object to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition")
	ESpaceObjectType ObjectType;

	/** Blueprint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition")
	TSubclassOf<AActor> ActorClass;

	/** Minimum number of this object type to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition", meta = (ClampMin = "0"))
	int32 MinCount;

	/** Maximum number of this object type to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition", meta = (ClampMin = "0"))
	int32 MaxCount;

	/** Distribution pattern for placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition")
	EDistributionPattern DistributionPattern;

	/** Scale variation (1.0 = no variation, 2.0 = up to 2x size difference) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ScaleVariation;

	/** Minimum distance from sector center (in Unreal Units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition", meta = (ClampMin = "0"))
	float MinDistanceFromCenter;

	/** Maximum distance from sector center (0 = use sector bounds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition", meta = (ClampMin = "0"))
	float MaxDistanceFromCenter;

	/** Faction to assign ownership (optional, nullptr = neutral) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition")
	UFactionDataAsset* OwnerFaction;

	/** Ship data to use if ObjectType is Ship */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition", meta = (EditCondition = "ObjectType == ESpaceObjectType::Ship"))
	USpaceshipDataAsset* ShipData;

	/** Whether to apply random rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Definition")
	bool bRandomRotation;

	FSpaceObjectDefinition()
		: ObjectType(ESpaceObjectType::Asteroid)
		, ActorClass(nullptr)
		, MinCount(1)
		, MaxCount(5)
		, DistributionPattern(EDistributionPattern::Random)
		, ScaleVariation(1.5f)
		, MinDistanceFromCenter(0.0f)
		, MaxDistanceFromCenter(0.0f)
		, OwnerFaction(nullptr)
		, ShipData(nullptr)
		, bRandomRotation(true)
	{
	}
};
