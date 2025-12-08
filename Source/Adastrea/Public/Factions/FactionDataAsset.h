// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactionDataAsset.generated.h"

/**
 * Faction Data Asset
 * Data-driven configuration for political factions in Adastrea
 * 
 * Factions represent major political, economic, or social organizations
 * with distinct goals, relationships, and gameplay characteristics.
 * 
 * Usage:
 * 1. Create a Blueprint based on this class in Content/DataAssets/Factions/
 * 2. Configure faction properties (name, description, traits)
 * 3. Reference the Data Asset from stations, ships, NPCs, and quests
 * 4. Use reputation system to track player standing with faction
 * 
 * Integration:
 * - Space stations can be owned by factions
 * - Trading prices affected by faction relationships
 * - NPCs affiliated with factions behave accordingly
 * - Quest givers represent faction interests
 */
UCLASS(BlueprintType)
class ADASTREA_API UFactionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFactionDataAsset();

	/** Default technology level for new factions */
	static constexpr int32 DEFAULT_TECHNOLOGY_LEVEL = 5;

	// ====================
	// Basic Info
	// ====================

	/** Display name of the faction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
	FText FactionName;

	/** Brief description of the faction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info", meta=(MultiLine=true))
	FText Description;

	/** Unique identifier for the faction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
	FName FactionID;

	// ====================
	// Faction Properties
	// ====================

	/** Initial reputation with player (-100 to 100) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Config", meta=(ClampMin="-100", ClampMax="100"))
	int32 InitialReputation;

	/** Whether this faction is hostile by default */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Config")
	bool bIsHostileByDefault;

	/** Whether this is a major faction (affects gameplay systems) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Config")
	bool bIsMajorFaction;

	/** Technology level of this faction (1-10, affects available modules and equipment) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Config", meta=(ClampMin="1", ClampMax="10"))
	int32 TechnologyLevel;

	// ====================
	// Blueprint Functions
	// ====================

	/**
	 * Get the faction's unique identifier
	 * @return Faction ID as FName
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction")
	FName GetFactionID() const { return FactionID; }

	/**
	 * Get the faction's display name
	 * @return Display name as FText
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction")
	FText GetFactionName() const { return FactionName; }

	/**
	 * Check if this faction is hostile by default
	 * @return True if hostile by default
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction")
	bool IsHostileByDefault() const { return bIsHostileByDefault; }

#if WITH_EDITOR
	/**
	 * Validate faction data asset properties
	 * Checks for required fields and logical constraints
	 */
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};
