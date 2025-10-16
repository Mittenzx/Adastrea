#include "Factions/FactionDataAsset.h"

UFactionDataAsset::UFactionDataAsset()
{
    // Set default values
    FactionName = FText::FromString(TEXT("Unknown Faction"));
    Description = FText::FromString(TEXT("A mysterious faction operating in the galaxy."));
    PrimaryColor = FLinearColor::White;
    SecondaryColor = FLinearColor::Gray;
    InitialReputation = 0;
    bHostileByDefault = false;
    TechnologyLevel = 5;
    MilitaryStrength = 5;
    EconomicPower = 5;
    FactionID = FName(TEXT("UnknownFaction"));
}
