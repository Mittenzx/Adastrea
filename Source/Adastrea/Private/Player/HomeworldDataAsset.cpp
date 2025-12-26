#include "Player/HomeworldDataAsset.h"

UHomeworldDataAsset::UHomeworldDataAsset()
{
	// Set default values
	HomeworldName = FText::FromString(TEXT("Unknown Homeworld"));
	Description = FText::FromString(TEXT("A homeworld in the galaxy."));
	HomeworldID = FName(TEXT("UnknownHomeworld"));
	StartingLocation = FVector::ZeroVector;
}
