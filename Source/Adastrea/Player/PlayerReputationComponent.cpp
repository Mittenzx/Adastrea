// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Player/PlayerReputationComponent.h"
#include "Factions/FactionDataAsset.h"

UPlayerReputationComponent::UPlayerReputationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerReputationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerReputationComponent::InitializeFactionReputation(FName FactionID, int32 InitialReputation)
{
	if (FactionID == NAME_None)
	{
		return;
	}

	FPlayerFactionReputation NewReputation(FactionID, FMath::Clamp(InitialReputation, -100, 100));
	FactionReputations.Add(FactionID, NewReputation);
}

void UPlayerReputationComponent::InitializeFromFactionData(UFactionDataAsset* FactionData)
{
	if (!FactionData)
	{
		return;
	}

	InitializeFactionReputation(FactionData->GetFactionID(), FactionData->InitialReputation);
}

void UPlayerReputationComponent::ModifyReputation(FName FactionID, int32 Amount)
{
	if (FactionID == NAME_None || Amount == 0)
	{
		return;
	}

	FPlayerFactionReputation* Reputation = FactionReputations.Find(FactionID);
	if (!Reputation)
	{
		// Initialize if doesn't exist, clamping the amount to valid range
		InitializeFactionReputation(FactionID, FMath::Clamp(Amount, -100, 100));
		return;
	}

	// Clamp to -100 to 100 range
	Reputation->ReputationValue = FMath::Clamp(Reputation->ReputationValue + Amount, -100, 100);
	Reputation->Tier = FPlayerFactionReputation::CalculateTier(Reputation->ReputationValue);
}

int32 UPlayerReputationComponent::GetReputation(FName FactionID) const
{
	const FPlayerFactionReputation* Reputation = FactionReputations.Find(FactionID);
	return Reputation ? Reputation->ReputationValue : 0;
}

EReputationTier UPlayerReputationComponent::GetReputationTier(FName FactionID) const
{
	const FPlayerFactionReputation* Reputation = FactionReputations.Find(FactionID);
	return Reputation ? Reputation->Tier : EReputationTier::Neutral;
}

bool UPlayerReputationComponent::ShouldBeHostile(FName FactionID) const
{
	return GetReputationTier(FactionID) == EReputationTier::Hostile;
}

float UPlayerReputationComponent::GetTradePriceModifier(FName FactionID) const
{
	int32 Reputation = GetReputation(FactionID);
	
	// Map reputation (-100 to 100) to price modifier (1.3 to 0.7)
	// Higher reputation = lower prices (better deals)
	// Formula: 1.0 - (reputation * 0.003)
	// At -100: 1.0 - (-0.3) = 1.3 (30% more expensive)
	// At 0: 1.0
	// At 100: 1.0 - 0.3 = 0.7 (30% cheaper)
	
	return FMath::Clamp(1.0f - (Reputation * 0.003f), 0.7f, 1.3f);
}

TArray<FPlayerFactionReputation> UPlayerReputationComponent::GetAllReputations() const
{
	TArray<FPlayerFactionReputation> Result;
	FactionReputations.GenerateValueArray(Result);
	return Result;
}
