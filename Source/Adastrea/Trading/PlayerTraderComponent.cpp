#include "PlayerTraderComponent.h"
#include "TradeItemDataAsset.h"
#include "MarketDataAsset.h"
#include "CargoComponent.h"
#include "EconomyManager.h"
#include "Engine/GameInstance.h"

UPlayerTraderComponent::UPlayerTraderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Credits = 1000;  // Default starting credits
	StartingCredits = 1000;
	LastProfitMilestone = 0;
}

bool UPlayerTraderComponent::BuyItem(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity, UCargoComponent* CargoComponent)
{
	if (!Market || !Item || !CargoComponent || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTrader: Invalid buy parameters"));
		return false;
	}

	// Check if market has stock
	if (!Market->IsItemInStock(Item->ItemID, Quantity))
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTrader: Market doesn't have %d x %s"), Quantity, *Item->ItemName.ToString());
		return false;
	}

	// Check if can afford
	int32 TotalCost = GetBuyCost(Market, Item, Quantity);
	if (!CanAfford(Market, Item, Quantity))
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTrader: Can't afford %d x %s (need %d, have %d)"), 
			Quantity, *Item->ItemName.ToString(), TotalCost, Credits);
		return false;
	}

	// Check if has cargo space
	if (!CargoComponent->HasSpaceFor(Item, Quantity))
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTrader: Not enough cargo space for %d x %s"), Quantity, *Item->ItemName.ToString());
		return false;
	}

	// Perform transaction
	RemoveCredits(TotalCost);
	CargoComponent->AddCargo(Item, Quantity);

	// Record transaction with economy manager
	if (UEconomyManager* EconomyMgr = GetEconomyManager())
	{
		EconomyMgr->RecordTransaction(Market, Item, Quantity, true);
	}

	UE_LOG(LogTemp, Log, TEXT("PlayerTrader: Bought %d x %s for %d credits (remaining: %d)"), 
		Quantity, *Item->ItemName.ToString(), TotalCost, Credits);

	// Broadcast event
	OnItemBought.Broadcast(Item, Quantity, TotalCost);

	return true;
}

bool UPlayerTraderComponent::SellItem(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity, UCargoComponent* CargoComponent)
{
	if (!Market || !Item || !CargoComponent || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTrader: Invalid sell parameters"));
		return false;
	}

	// Check if has item in cargo
	if (!CargoComponent->HasItem(Item, Quantity))
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTrader: Don't have %d x %s in cargo"), Quantity, *Item->ItemName.ToString());
		return false;
	}

	// Calculate value
	int32 TotalValue = GetSellValue(Market, Item, Quantity);

	// Perform transaction
	CargoComponent->RemoveCargo(Item, Quantity);
	AddCredits(TotalValue);

	// Record transaction with economy manager
	if (UEconomyManager* EconomyMgr = GetEconomyManager())
	{
		EconomyMgr->RecordTransaction(Market, Item, Quantity, false);
	}

	UE_LOG(LogTemp, Log, TEXT("PlayerTrader: Sold %d x %s for %d credits (total: %d)"), 
		Quantity, *Item->ItemName.ToString(), TotalValue, Credits);

	// Broadcast event
	OnItemSold.Broadcast(Item, Quantity, TotalValue);

	// Check profit milestones
	CheckProfitMilestones();

	return true;
}

bool UPlayerTraderComponent::CanAfford(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity) const
{
	int32 TotalCost = GetBuyCost(Market, Item, Quantity);
	return Credits >= TotalCost;
}

int32 UPlayerTraderComponent::GetBuyCost(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity) const
{
	if (!Market || !Item || Quantity <= 0)
	{
		return 0;
	}

	UEconomyManager* EconomyMgr = GetEconomyManager();
	if (!EconomyMgr)
	{
		return 0;
	}

	float PricePerUnit = EconomyMgr->GetItemPrice(Market, Item, true);
	return FMath::RoundToInt(PricePerUnit * Quantity);
}

int32 UPlayerTraderComponent::GetSellValue(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity) const
{
	if (!Market || !Item || Quantity <= 0)
	{
		return 0;
	}

	UEconomyManager* EconomyMgr = GetEconomyManager();
	if (!EconomyMgr)
	{
		return 0;
	}

	float PricePerUnit = EconomyMgr->GetItemPrice(Market, Item, false);
	return FMath::RoundToInt(PricePerUnit * Quantity);
}

void UPlayerTraderComponent::AddCredits(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	int32 OldCredits = Credits;
	Credits += Amount;

	UE_LOG(LogTemp, Log, TEXT("PlayerTrader: Added %d credits (%d -> %d)"), Amount, OldCredits, Credits);

	OnCreditsChanged.Broadcast(Credits, Amount);
}

bool UPlayerTraderComponent::RemoveCredits(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	if (Credits < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTrader: Can't remove %d credits (only have %d)"), Amount, Credits);
		return false;
	}

	int32 OldCredits = Credits;
	Credits -= Amount;

	UE_LOG(LogTemp, Log, TEXT("PlayerTrader: Removed %d credits (%d -> %d)"), Amount, OldCredits, Credits);

	OnCreditsChanged.Broadcast(Credits, -Amount);

	return true;
}

float UPlayerTraderComponent::GetProfitPercentage() const
{
	if (StartingCredits <= 0)
	{
		return 0.0f;
	}

	return ((float)GetProfit() / (float)StartingCredits) * 100.0f;
}

void UPlayerTraderComponent::ResetFinances()
{
	Credits = StartingCredits;
	LastProfitMilestone = 0;
	
	UE_LOG(LogTemp, Log, TEXT("PlayerTrader: Reset finances to %d credits"), StartingCredits);
	
	OnCreditsChanged.Broadcast(Credits, 0);
}

UEconomyManager* UPlayerTraderComponent::GetEconomyManager() const
{
	if (AActor* Owner = GetOwner())
	{
		if (UWorld* World = Owner->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				return GameInstance->GetSubsystem<UEconomyManager>();
			}
		}
	}

	return nullptr;
}

void UPlayerTraderComponent::CheckProfitMilestones()
{
	int32 CurrentProfit = GetProfit();

	// Check for milestones at 5000, 10000, 25000, 50000, 100000, etc.
	const TArray<int32> Milestones = { 5000, 10000, 25000, 50000, 100000, 250000, 500000, 1000000 };

	for (int32 Milestone : Milestones)
	{
		if (CurrentProfit >= Milestone && LastProfitMilestone < Milestone)
		{
			LastProfitMilestone = Milestone;
			OnProfitMilestone.Broadcast(Milestone);
			
			UE_LOG(LogTemp, Log, TEXT("PlayerTrader: Reached profit milestone: %d credits!"), Milestone);
		}
	}
}
