// Copyright Mittenzx. All Rights Reserved.

#include "AdastreaNames.h"
#include "Interfaces/ITargetable.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Universe/JumpGate.h"
#include "Universe/OwnershipComponent.h"

FString AdastreaNames::Readable(const FString& ObjectOrClassName)
{
	FString Name = ObjectOrClassName;

	// Instance numbers and the blueprint class suffix: "_C_2", "_C", "_2".
	int32 Underscore = INDEX_NONE;
	if (Name.FindLastChar(TEXT('_'), Underscore) && Underscore + 1 < Name.Len()
		&& Name.RightChop(Underscore + 1).IsNumeric())
	{
		Name.LeftInline(Underscore);
	}
	if (Name.EndsWith(TEXT("_C")))
	{
		Name.LeftChopInline(2);
	}
	// (UClass names carry no C++ "A" prefix, so only Blueprint prefixes need stripping.)
	if (!Name.RemoveFromStart(TEXT("BP_"), ESearchCase::CaseSensitive))
	{
		Name.RemoveFromStart(TEXT("B_"), ESearchCase::CaseSensitive);
	}
	Name.ReplaceInline(TEXT("_"), TEXT(" "));
	return FName::NameToDisplayString(Name, false);
}

FString AdastreaNames::ForActor(const AActor* Actor)
{
	if (!Actor)
	{
		return FString();
	}
	if (const AJumpGate* Gate = Cast<AJumpGate>(Actor))
	{
		return Gate->GetDisplayName();
	}
	if (const ASpaceStation* Station = Cast<ASpaceStation>(Actor))
	{
		return Station->GetDisplayNameString();
	}
	if (const ASpaceship* Ship = Cast<ASpaceship>(Actor))
	{
		if (const UOwnershipComponent* Owner = UOwnershipComponent::Find(Ship))
		{
			if (!Owner->RecordName.IsEmpty())
			{
				return Owner->RecordName.ToString();
			}
		}
		return Ship->GetShipName().ToString();
	}
	if (Actor->Implements<UTargetable>())
	{
		const FText Name = ITargetable::Execute_GetTargetDisplayName(Actor);
		if (!Name.IsEmpty())
		{
			return Name.ToString();
		}
	}
	return Readable(Actor->GetClass()->GetName());
}
