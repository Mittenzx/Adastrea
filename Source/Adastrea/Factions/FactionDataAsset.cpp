// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Factions/FactionDataAsset.h"
#include "AdastreaLog.h"

UFactionDataAsset::UFactionDataAsset()
{
	FactionName = INVTEXT("New Faction");
	Description = INVTEXT("A faction in the Adastrea universe");
	FactionID = NAME_None;
	InitialReputation = 0;
	bIsHostileByDefault = false;
	bIsMajorFaction = false;
	TechnologyLevel = DEFAULT_TECHNOLOGY_LEVEL;
}

#if WITH_EDITOR
EDataValidationResult UFactionDataAsset::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	// Validate basic info
	if (FactionName.IsEmpty())
	{
		ValidationErrors.Add(FText::FromString(TEXT("Faction Name is empty")));
		Result = EDataValidationResult::Invalid;
	}

	if (Description.IsEmpty())
	{
		ValidationErrors.Add(FText::FromString(TEXT("Faction Description is empty")));
		Result = EDataValidationResult::Invalid;
	}

	if (FactionID.IsNone())
	{
		ValidationErrors.Add(FText::FromString(TEXT("Faction ID is not set")));
		Result = EDataValidationResult::Invalid;
	}

	// Validate initial reputation range
	if (InitialReputation < -100 || InitialReputation > 100)
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Initial Reputation ({0}) must be between -100 and 100")),
			FText::AsNumber(InitialReputation)
		));
		Result = EDataValidationResult::Invalid;
	}

	// Validate technology level
	if (TechnologyLevel < 1 || TechnologyLevel > 10)
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Technology Level ({0}) must be between 1 and 10")),
			FText::AsNumber(TechnologyLevel)
		));
		Result = EDataValidationResult::Invalid;
	}

	// Warn about inconsistent settings
	if (bIsHostileByDefault && InitialReputation > 0)
	{
		ValidationErrors.Add(FText::FromString(TEXT("Warning: Faction is hostile by default but has positive initial reputation")));
		// Just a warning, not invalid
	}

	// Log validation result
	if (Result == EDataValidationResult::Valid)
	{
		UE_LOG(LogAdastrea, Log, TEXT("FactionDataAsset %s passed validation"), *FactionName.ToString());
	}
	else
	{
		UE_LOG(LogAdastrea, Warning, TEXT("FactionDataAsset %s failed validation with %d errors"), 
			*FactionName.ToString(), ValidationErrors.Num());
	}

	return Result;
}
#endif
