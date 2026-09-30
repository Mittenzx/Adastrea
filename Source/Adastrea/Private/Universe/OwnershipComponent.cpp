// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/OwnershipComponent.h"
#include "Universe/OrganisationSubsystem.h"
#include "Trading/PlayerTraderComponent.h"
#include "GameFramework/Actor.h"

UOwnershipComponent::UOwnershipComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UOwnershipComponent* UOwnershipComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UOwnershipComponent>() : nullptr;
}

void UOwnershipComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ShipRecordId.IsNone())
	{
		return;
	}
	if (UPlayerTraderComponent* Trader = GetOwner()->FindComponentByClass<UPlayerTraderComponent>())
	{
		WorkingCapital = Trader->GetCredits();
		Trader->OnItemSold.AddDynamic(this, &UOwnershipComponent::HandleItemSold);
	}
}

void UOwnershipComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPlayerTraderComponent* Trader = GetOwner() ? GetOwner()->FindComponentByClass<UPlayerTraderComponent>() : nullptr)
	{
		Trader->OnItemSold.RemoveDynamic(this, &UOwnershipComponent::HandleItemSold);
	}
	Super::EndPlay(EndPlayReason);
}

void UOwnershipComponent::HandleItemSold(UTradeItemDataAsset* Item, int32 Quantity, int32 TotalValue)
{
	UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this);
	UPlayerTraderComponent* Trader = GetOwner()->FindComponentByClass<UPlayerTraderComponent>();
	if (Orgs && Trader)
	{
		Orgs->HandleShipSale(ShipRecordId, Trader, WorkingCapital);
	}
}
