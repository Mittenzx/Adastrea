// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/PlayerInteractableComponent.h"

UPlayerInteractableComponent::UPlayerInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector UPlayerInteractableComponent::GetInteractionWorldPoint() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}
	return Owner->GetActorLocation() + Owner->GetActorRotation().RotateVector(InteractionPointOffset);
}

FText UPlayerInteractableComponent::GetInteractPrompt_Implementation() const
{
	return InteractPrompt.IsEmpty() ? INVTEXT("Interact") : InteractPrompt;
}

void UPlayerInteractableComponent::Interact_Implementation(AAdastreaPlayerController* PC)
{
	OnInteracted.Broadcast(PC);
}