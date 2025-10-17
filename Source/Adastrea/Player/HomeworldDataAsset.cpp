#include "Player/HomeworldDataAsset.h"

UHomeworldDataAsset::UHomeworldDataAsset()
{
	// Set default values
	HomeworldName = FText::FromString(TEXT("Unknown Homeworld"));
	Description = FText::FromString(TEXT("A homeworld in the galaxy."));
	HomeworldID = FName(TEXT("UnknownHomeworld"));
}

bool UHomeworldDataAsset::GetStartingReputation(const UFactionDataAsset* Faction, int32& OutReputation) const
{
	if (!Faction)
	{
		return false;
	}

	for (const FFactionRelationEntry& Entry : FactionRelations)
	{
		if (Entry.Faction == Faction)
		{
			OutReputation = Entry.StartingReputation;
			return true;
		}
	}

	return false;
}

TArray<FFactionRelationEntry> UHomeworldDataAsset::GetFactionRelations() const
{
	return FactionRelations;
}

bool UHomeworldDataAsset::HasRelationshipWith(const UFactionDataAsset* Faction) const
{
	if (!Faction)
	{
		return false;
	}

	for (const FFactionRelationEntry& Entry : FactionRelations)
	{
		if (Entry.Faction == Faction)
		{
			return true;
		}
	}

	return false;
}
