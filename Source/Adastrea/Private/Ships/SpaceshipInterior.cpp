#include "Ships/SpaceshipInterior.h"

ASpaceshipInterior::ASpaceshipInterior()
{
    PrimaryActorTick.bCanEverTick = false;
    EntryLocation = FVector(0,0,100); // Example entry point
    EntryRotation = FRotator(0,0,0);
    ExitLocation = FVector(0,0,100); // Default exit point (same as entry)
}

FVector ASpaceshipInterior::GetEntryLocation() const
{
    return EntryLocation;
}

FRotator ASpaceshipInterior::GetEntryRotation() const
{
    return EntryRotation;
}

FVector ASpaceshipInterior::GetExitLocation() const
{
    return ExitLocation;
}