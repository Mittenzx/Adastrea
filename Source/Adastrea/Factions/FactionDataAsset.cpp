// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Factions/FactionDataAsset.h"

UFactionDataAsset::UFactionDataAsset()
{
	FactionName = INVTEXT("New Faction");
	Description = INVTEXT("A faction in the Adastrea universe");
	FactionID = NAME_None;
	InitialReputation = 0;
	bIsHostileByDefault = false;
	bIsMajorFaction = false;
	TechnologyLevel = 5;  // Default mid-level technology
}
