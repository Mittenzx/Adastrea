// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WorldInteractable.generated.h"

class AAdastreaPlayerController;

/**
 * Interface for anything the on-foot avatar can interact with using the E key.
 *
 * This is the WORLDWIDE interaction seam: it makes no assumption about whether the
 * interactable is a ship seat, a station terminal, an airlock, a planet NPC or a door.
 * Any actor implementing this interface gets a context prompt the avatar shows and can
 * be activated by the E key. Placing an interactable = implement this interface (or add
 * UPlayerInteractableComponent), place the actor in a level, and it just works in ships,
 * on stations and on planets alike.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UWorldInteractable : public UInterface
{
	GENERATED_BODY()
};

class ADASTREA_API IWorldInteractable
{
	GENERATED_BODY()

public:
	/** Short prompt shown on the HUD near the avatar, e.g. "Board Cockpit" / "Open Terminal". */
	UFUNCTION(BlueprintNativeEvent, Category="Interaction")
	FText GetInteractPrompt() const;
	virtual FText GetInteractPrompt_Implementation() const;

	/** Perform the interaction (context-dependent: sit down, open a menu, open a door). */
	UFUNCTION(BlueprintNativeEvent, Category="Interaction")
	void Interact(AAdastreaPlayerController* PC);
	virtual void Interact_Implementation(AAdastreaPlayerController* PC);
};