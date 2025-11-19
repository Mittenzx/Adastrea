// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Factions/FactionDataAsset.h"

UFactionDataAsset::UFactionDataAsset()
{
	FactionName = FText::FromString(TEXT("New Faction"));
	Description = FText::FromString(TEXT("A faction in the Adastrea universe"));
	FactionID = NAME_None;
	InitialReputation = 0;
	bIsHostileByDefault = false;
	bIsMajorFaction = false;
}
