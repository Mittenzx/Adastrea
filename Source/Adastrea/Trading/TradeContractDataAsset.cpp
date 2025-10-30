#include "TradeContractDataAsset.h"
#include "TradeItemDataAsset.h"
#include "Public/Factions/FactionDataAsset.h"

UTradeContractDataAsset::UTradeContractDataAsset()
	: ContractName(FText::FromString(TEXT("Trade Contract")))
	, Description(FText::FromString(TEXT("A trading contract.")))
	, ContractID(NAME_None)
	, ContractType(ETradeContractType::Delivery)
	, Difficulty(EContractDifficulty::Medium)
	, ContractGiver(nullptr)
	, Distance(0.0f)
	, TimeLimit(0.0f)
	, MinReputationRequired(0)
	, Status(EContractStatus::Available)
	, AcceptedTime(0.0f)
	, ExpirationTime(0.0f)
	, AcceptedByPlayerID(NAME_None)
	, bRepeatable(false)
	, bRequiresStealth(false)
	, bCombatExpected(false)
{
}

bool UTradeContractDataAsset::CanPlayerAccept(int32 PlayerReputation, float PlayerCargoCapacity) const
{
	// Check reputation requirement
	if (PlayerReputation < MinReputationRequired)
	{
		return false;
	}

	// Check cargo capacity
	float RequiredVolume = GetTotalCargoVolume();
	if (RequiredVolume > PlayerCargoCapacity)
	{
		return false;
	}

	// Check if contract is available
	if (Status != EContractStatus::Available)
	{
		return false;
	}

	return true;
}

float UTradeContractDataAsset::GetTotalCargoVolume() const
{
	float TotalVolume = 0.0f;
	
	for (const FContractCargo& Cargo : RequiredCargo)
	{
		if (Cargo.TradeItem)
		{
			TotalVolume += Cargo.TradeItem->GetTotalVolume(Cargo.Quantity);
		}
	}
	
	return TotalVolume;
}

float UTradeContractDataAsset::GetTotalCargoMass() const
{
	float TotalMass = 0.0f;
	
	for (const FContractCargo& Cargo : RequiredCargo)
	{
		if (Cargo.TradeItem)
		{
			TotalMass += Cargo.TradeItem->GetTotalMass(Cargo.Quantity);
		}
	}
	
	return TotalMass;
}

float UTradeContractDataAsset::GetEstimatedCompletionTime(float AverageSpeed) const
{
	if (AverageSpeed <= 0.0f)
	{
		return 0.0f;
	}
	
	return Distance / AverageSpeed;
}

int32 UTradeContractDataAsset::CalculateProfitMargin(int32 CargoAcquisitionCost, int32 FuelCost) const
{
	int32 TotalCosts = CargoAcquisitionCost + FuelCost + Penalties.CollateralDeposit;
	int32 TotalRewards = Rewards.Credits;
	
	return TotalRewards - TotalCosts;
}

float UTradeContractDataAsset::GetRemainingTime(float CurrentGameTime) const
{
	if (TimeLimit <= 0.0f || Status != EContractStatus::Active)
	{
		return 0.0f;
	}
	
	float ElapsedTime = CurrentGameTime - AcceptedTime;
	return FMath::Max(0.0f, TimeLimit - ElapsedTime);
}

bool UTradeContractDataAsset::IsExpired(float CurrentGameTime) const
{
	if (TimeLimit <= 0.0f)
	{
		return false; // No time limit
	}
	
	if (Status != EContractStatus::Active)
	{
		return false;
	}
	
	return GetRemainingTime(CurrentGameTime) <= 0.0f;
}

bool UTradeContractDataAsset::AcceptContract(FName PlayerID, float CurrentGameTime)
{
	if (Status != EContractStatus::Available)
	{
		return false;
	}
	
	Status = EContractStatus::Active;
	AcceptedByPlayerID = PlayerID;
	AcceptedTime = CurrentGameTime;
	
	if (TimeLimit > 0.0f)
	{
		ExpirationTime = CurrentGameTime + TimeLimit;
	}
	
	OnContractAccepted(PlayerID);
	return true;
}

bool UTradeContractDataAsset::CompleteContract(float CurrentGameTime)
{
	if (Status != EContractStatus::Active)
	{
		return false;
	}
	
	// Check if expired
	if (IsExpired(CurrentGameTime))
	{
		Status = EContractStatus::Expired;
		OnContractExpired();
		return false;
	}
	
	Status = EContractStatus::Completed;
	OnContractCompleted(AcceptedByPlayerID);
	return true;
}

bool UTradeContractDataAsset::FailContract(const FText& Reason)
{
	if (Status != EContractStatus::Active)
	{
		return false;
	}
	
	Status = EContractStatus::Failed;
	OnContractFailed(AcceptedByPlayerID, Reason);
	return true;
}

bool UTradeContractDataAsset::HasTag(FName Tag) const
{
	return ContractTags.Contains(Tag);
}

// BlueprintNativeEvent implementations
void UTradeContractDataAsset::OnContractAccepted_Implementation(FName PlayerID)
{
	// Default implementation does nothing
	// Designers can override in Blueprint
}

void UTradeContractDataAsset::OnContractCompleted_Implementation(FName PlayerID)
{
	// Default implementation does nothing
	// Designers can override in Blueprint
}

void UTradeContractDataAsset::OnContractFailed_Implementation(FName PlayerID, const FText& Reason)
{
	// Default implementation does nothing
	// Designers can override in Blueprint
}

void UTradeContractDataAsset::OnContractExpired_Implementation()
{
	// Default implementation does nothing
	// Designers can override in Blueprint
}
