// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NameGenerator.generated.h"

/**
 * Utility class for generating procedural names for sectors, stations, and other entities
 * 
 * Provides various naming patterns and themes for creating unique, lore-friendly names.
 * All functions are static and can be called from C++ or Blueprints.
 * 
 * Usage:
 * - C++: FString Name = UNameGenerator::GenerateSectorName(ESectorNamingTheme::Military);
 * - Blueprint: Call "Generate Sector Name" function
 */
UCLASS(BlueprintType)
class ADASTREA_API UNameGenerator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Generate a sector name based on theme
	 * @param Theme The naming theme to use
	 * @param Seed Random seed for reproducible generation (0 = use random)
	 * @return Generated sector name
	 */
	UFUNCTION(BlueprintCallable, Category = "Name Generator")
	static FString GenerateSectorName(uint8 Theme, int32 Seed = 0);

	/**
	 * Generate a station name
	 * @param StationType Type of station (e.g., "Trading", "Military", "Mining")
	 * @param Seed Random seed for reproducible generation (0 = use random)
	 * @return Generated station name
	 */
	UFUNCTION(BlueprintCallable, Category = "Name Generator")
	static FString GenerateStationName(const FString& StationType, int32 Seed = 0);

	/**
	 * Generate a ship name
	 * @param ShipClass Ship class/type
	 * @param Seed Random seed for reproducible generation (0 = use random)
	 * @return Generated ship name
	 */
	UFUNCTION(BlueprintCallable, Category = "Name Generator")
	static FString GenerateShipName(const FString& ShipClass, int32 Seed = 0);

	/**
	 * Check if a name is unique in the current world
	 * @param Name Name to check
	 * @param Context World context for searching
	 * @return True if name is unique
	 */
	UFUNCTION(BlueprintCallable, Category = "Name Generator", meta = (WorldContext = "Context"))
	static bool IsNameUnique(const FString& Name, UObject* Context);

	/**
	 * Generate a unique sector name (ensures no duplicates exist)
	 * @param Theme The naming theme to use
	 * @param Context World context for uniqueness checking
	 * @param MaxAttempts Maximum attempts to find unique name
	 * @return Generated unique sector name
	 */
	UFUNCTION(BlueprintCallable, Category = "Name Generator", meta = (WorldContext = "Context"))
	static FString GenerateUniqueSectorName(uint8 Theme, UObject* Context, int32 MaxAttempts = 100);

private:
	// Name component arrays for procedural generation
	static const TArray<FString> MilitarySectorPrefixes;
	static const TArray<FString> MilitarySectorSuffixes;
	static const TArray<FString> MiningSectorPrefixes;
	static const TArray<FString> MiningSectorSuffixes;
	static const TArray<FString> TradeSectorPrefixes;
	static const TArray<FString> TradeSectorSuffixes;
	static const TArray<FString> ScientificSectorPrefixes;
	static const TArray<FString> ScientificSectorSuffixes;
	static const TArray<FString> FrontierSectorPrefixes;
	static const TArray<FString> FrontierSectorSuffixes;
	static const TArray<FString> GreekLetters;
	static const TArray<FString> StationPrefixes;
	static const TArray<FString> StationSuffixes;
	static const TArray<FString> ShipPrefixes;
	static const TArray<FString> ShipSuffixes;

	/**
	 * Initialize random stream with seed
	 * @param Seed Seed value (0 = use random)
	 * @return Initialized random stream
	 */
	static FRandomStream InitializeRandomStream(int32 Seed);

	/**
	 * Generate military-themed sector name
	 */
	static FString GenerateMilitarySectorName(FRandomStream& RandomStream);

	/**
	 * Generate mining-themed sector name
	 */
	static FString GenerateMiningSectorName(FRandomStream& RandomStream);

	/**
	 * Generate trade-themed sector name
	 */
	static FString GenerateTradeSectorName(FRandomStream& RandomStream);

	/**
	 * Generate scientific-themed sector name
	 */
	static FString GenerateScientificSectorName(FRandomStream& RandomStream);

	/**
	 * Generate frontier-themed sector name
	 */
	static FString GenerateFrontierSectorName(FRandomStream& RandomStream);

	/**
	 * Generate Greek letter-based sector name
	 */
	static FString GenerateGreekSectorName(FRandomStream& RandomStream);

	/**
	 * Generate numeric sector name
	 */
	static FString GenerateNumericSectorName(FRandomStream& RandomStream);

	/**
	 * Generate descriptive sector name based on content
	 */
	static FString GenerateDescriptiveSectorName(FRandomStream& RandomStream);
};
