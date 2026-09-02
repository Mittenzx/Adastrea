// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/WorldInteractable.h"
#include "Player/AdastreaPlayerController.h"

FText IWorldInteractable::GetInteractPrompt_Implementation() const
{
	return FText::FromString(TEXT("Interact"));
}

void IWorldInteractable::Interact_Implementation(AAdastreaPlayerController* PC)
{
	// Default: nothing. Implementors override.
}