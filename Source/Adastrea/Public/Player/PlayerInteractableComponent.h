// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/WorldInteractable.h"
#include "PlayerInteractableComponent.generated.h"

class AAdastreaPlayerController;

/** Broadcast when the avatar interacts with this object. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableInteracted, AAdastreaPlayerController*, PC);

/**
 * A reusable, drop-in "interactable" component.
 *
 * Add this to any actor you want the worldwide on-foot avatar to be able to interact
 * with (ship cockpit seat, station terminal, airlock control, planet door/NPC). It
 * implements IWorldInteractable so the avatar's E key finds and activates it. The
 * delegate fires on Interact, and the prompt is configurable.
 */
UCLASS(ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UPlayerInteractableComponent : public UActorComponent, public IWorldInteractable
{
	GENERATED_BODY()

public:
	UPlayerInteractableComponent();

	/** The prompt text shown to the player ("Board Cockpit", "Open Terminal", ...). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText InteractPrompt;

	/** Broadcast when the avatar interacts with this object. */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractableInteracted OnInteracted;

	/** Interaction radius (world units) the avatar must be within to prompt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractionRadius = 250.0f;

	/** Optional offset from the owner actor's origin used for distance checks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FVector InteractionPointOffset = FVector::ZeroVector;

	/** World location the avatar distances against for this interactable. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Interaction")
	FVector GetInteractionWorldPoint() const;

	// ~ IWorldInteractable
	virtual FText GetInteractPrompt_Implementation() const override;
	virtual void Interact_Implementation(AAdastreaPlayerController* PC) override;
};