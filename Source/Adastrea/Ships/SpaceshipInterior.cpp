#include "Ships/SpaceshipInterior.h" // Now resolves to Public/Ships/SpaceshipInterior.h

ASpaceshipInterior::ASpaceshipInterior()
{
    PrimaryActorTick.bCanEverTick = false;
    EntryLocation = FVector(0,0,100); // Example entry point
    EntryRotation = FRotator(0,0,0);
}

FVector ASpaceshipInterior::GetEntryLocation() const
{
    return EntryLocation;
}

FRotator ASpaceshipInterior::GetEntryRotation() const
{
    return EntryRotation;
}