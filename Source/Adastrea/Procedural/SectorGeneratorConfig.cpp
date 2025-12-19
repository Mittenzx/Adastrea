// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "SectorGeneratorConfig.h"

USectorGeneratorConfig::USectorGeneratorConfig()
	: NamingTheme(ESectorNamingTheme::Greek)
	, RandomSeed(0)
	, DensityMultiplier(1.0f)
	, MinObjectSpacing(50000.0f)  // Min spacing: 50,000 UU = 50,000 cm = 500 m (1 UU = 1 cm)
	, MaxSpawnAttempts(100)
	, bCreateCentralHub(false)
	, CentralHubClass(nullptr)
	, bCreateZones(false)
	, NumZones(3)
{
	ConfigName = FText::FromString(TEXT("New Sector Config"));
	Description = FText::FromString(TEXT("Configure sector generation parameters"));
}

int32 USectorGeneratorConfig::GetTotalObjectCount() const
{
	int32 TotalCount = 0;
	
	for (const FSpaceObjectDefinition& Definition : ObjectDefinitions)
	{
		// Use max count for estimation
		TotalCount += FMath::RoundToInt(Definition.MaxCount * DensityMultiplier);
	}
	
	return FMath::Max(0, TotalCount);
}

bool USectorGeneratorConfig::ValidateConfiguration(TArray<FText>& OutErrors) const
{
	bool bIsValid = true;

	// Check if we have any object definitions
	if (ObjectDefinitions.Num() == 0)
	{
		OutErrors.Add(FText::FromString(TEXT("No object definitions specified. Add at least one object type to spawn.")));
		bIsValid = false;
	}

	// Validate each object definition
	for (int32 i = 0; i < ObjectDefinitions.Num(); ++i)
	{
		const FSpaceObjectDefinition& Definition = ObjectDefinitions[i];

		if (!Definition.ActorClass)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Object definition %d: No actor class specified."), i)));
			bIsValid = false;
		}

		if (Definition.MinCount > Definition.MaxCount)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Object definition %d: MinCount (%d) is greater than MaxCount (%d)."), 
				i, Definition.MinCount, Definition.MaxCount)));
			bIsValid = false;
		}

		if (Definition.MinCount < 0)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Object definition %d: MinCount cannot be negative."), i)));
			bIsValid = false;
		}

		if (Definition.ScaleVariation <= 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Object definition %d: ScaleVariation must be positive."), i)));
			bIsValid = false;
		}

		if (Definition.MinDistanceFromCenter < 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Object definition %d: MinDistanceFromCenter cannot be negative."), i)));
			bIsValid = false;
		}

		if (Definition.MaxDistanceFromCenter > 0.0f && Definition.MaxDistanceFromCenter < Definition.MinDistanceFromCenter)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Object definition %d: MaxDistanceFromCenter is less than MinDistanceFromCenter."), i)));
			bIsValid = false;
		}

		// Validate ship-specific requirements
		if (Definition.ObjectType == ESpaceObjectType::Ship && !Definition.ShipData)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Object definition %d: Ship type requires ShipData to be set."), i)));
			bIsValid = false;
		}
	}

	// Validate spacing
	if (MinObjectSpacing < 0.0f)
	{
		OutErrors.Add(FText::FromString(TEXT("MinObjectSpacing cannot be negative.")));
		bIsValid = false;
	}

	// Validate density multiplier
	if (DensityMultiplier <= 0.0f)
	{
		OutErrors.Add(FText::FromString(TEXT("DensityMultiplier must be positive.")));
		bIsValid = false;
	}

	// Validate central hub settings
	if (bCreateCentralHub && !CentralHubClass)
	{
		OutErrors.Add(FText::FromString(TEXT("bCreateCentralHub is true but CentralHubClass is not set.")));
		bIsValid = false;
	}

	// Validate zone settings
	if (bCreateZones && NumZones <= 0)
	{
		OutErrors.Add(FText::FromString(TEXT("bCreateZones is true but NumZones is not positive.")));
		bIsValid = false;
	}

	return bIsValid;
}

TArray<FSpaceObjectDefinition> USectorGeneratorConfig::GetObjectDefinitionsByType(ESpaceObjectType ObjectType) const
{
	TArray<FSpaceObjectDefinition> FilteredDefinitions;
	
	for (const FSpaceObjectDefinition& Definition : ObjectDefinitions)
	{
		if (Definition.ObjectType == ObjectType)
		{
			FilteredDefinitions.Add(Definition);
		}
	}
	
	return FilteredDefinitions;
}

#if WITH_EDITOR
EDataValidationResult USectorGeneratorConfig::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	// Use our validation function
	TArray<FText> ConfigErrors;
	if (!ValidateConfiguration(ConfigErrors))
	{
		ValidationErrors.Append(ConfigErrors);
		Result = EDataValidationResult::Invalid;
	}

	// Validate basic info
	if (ConfigName.IsEmpty())
	{
		ValidationErrors.Add(FText::FromString(TEXT("ConfigName is required.")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
