#include "Trading/AITraderComponent.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/TradeItemDataAsset.h"
#include "Trading/TradeContractDataAsset.h"
// REMOVED: #include "Factions/FactionDataAsset.h" - faction system removed per Trade Simulator MVP

UAITraderComponent::UAITraderComponent()
	// REMOVED: TraderFaction - faction system removed per Trade Simulator MVP
	: Strategy(EAITraderStrategy::Balanced)
	, TradingCapital(10000)
	, CargoCapacity(1000.0f)
	, TravelSpeed(100.0f)
	, TradingSkill(5)
	, RiskTolerance(0.5f)
	, MinProfitMargin(0.1f)
	, bCanManipulatePrices(false)
	, bOperatesInBlackMarkets(false)
	, bAcceptsContracts(true)
	, TradeFrequency(5)
	, CurrentLocation(nullptr)
	, TotalProfit(0)
	, SuccessfulTrades(0)
	, UpdateTimer(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // Update every second
}

void UAITraderComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAITraderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTimer += DeltaTime;
	
	// Update trader logic every few seconds based on trade frequency
	float UpdateInterval = 3600.0f / FMath::Max(1, TradeFrequency); // Convert frequency to seconds
	if (UpdateTimer >= UpdateInterval)
	{
		UpdateTrader(UpdateTimer);
		UpdateTimer = 0.0f;
	}
}

void UAITraderComponent::Initialize(int32 StartingCapital, UMarketDataAsset* StartingMarket)
{
	TradingCapital = StartingCapital;
	CurrentLocation = StartingMarket;
	
	if (StartingMarket)
	{
		DiscoverMarket(StartingMarket);
	}
}

void UAITraderComponent::UpdateTrader(float DeltaTime)
{
	if (!CurrentLocation)
	{
		return;
	}

	// Execute different AI behaviors based on enabled behaviors
	MakeTradeDecisions();
	OptimizeTradeRoutes();
	ManageInventory();
}

TArray<FTradeRoute> UAITraderComponent::FindBestTradeRoutes(int32 MaxRoutes)
{
	TArray<FTradeRoute> BestRoutes;
	
	// Early exit if no markets known
	if (KnownMarkets.Num() == 0)
	{
		return BestRoutes;
	}
	
	// Reserve space to avoid reallocations
	// Estimate: each market might have profitable routes to 25% of other markets
	const int32 EstimatedRoutes = FMath::Max(1, (KnownMarkets.Num() * KnownMarkets.Num()) / 4);
	BestRoutes.Reserve(EstimatedRoutes);
	
	// Cache current location to optimize the most common case
	if (CurrentLocation)
	{
		// Prioritize routes from current location first (most relevant)
		for (const FMarketInventoryEntry& Entry : CurrentLocation->Inventory)
		{
			if (!Entry.TradeItem || !Entry.bInStock)
			{
				continue;
			}

			FTradeRoute Route = CalculateArbitrageOpportunity(Entry.TradeItem);
			if (Route.ProfitabilityScore > 0.0f)
			{
				BestRoutes.Add(Route);
			}
		}
	}
	
	// For each known market combination (excluding current location already processed)
	for (UMarketDataAsset* Origin : KnownMarkets)
	{
		if (!Origin || Origin == CurrentLocation)
		{
			continue;
		}

		// Process only in-stock items to reduce unnecessary calculations
		for (const FMarketInventoryEntry& Entry : Origin->Inventory)
		{
			if (!Entry.TradeItem || !Entry.bInStock)
			{
				continue;
			}

			// Calculate arbitrage only if we have enough capital for at least one unit
			float BuyPrice = Origin->GetItemPrice(Entry.TradeItem, true);
			if (BuyPrice > TradingCapital)
			{
				continue;
			}

			// Calculate best destination for this item from this origin
			float BestProfitability = 0.0f;
			UMarketDataAsset* BestDestination = nullptr;
			
			for (UMarketDataAsset* Destination : KnownMarkets)
			{
				if (!Destination || Destination == Origin)
				{
					continue;
				}
				
				float SellPrice = Destination->GetItemPrice(Entry.TradeItem, false);
				float ProfitPerUnit = SellPrice - BuyPrice;
				
				// Skip if not profitable
				if (ProfitPerUnit <= 0.0f)
				{
					continue;
				}
				
				// Check profit margin threshold
				float ProfitMargin = BuyPrice > 0.0f ? (ProfitPerUnit / BuyPrice) : 0.0f;
				if (ProfitMargin < MinProfitMargin)
				{
					continue;
				}
				
				// Simplified distance calculation using name hash as placeholder
				// TODO: Replace with actual market world positions when available
				// For now, this provides consistent relative distances for route prioritization
				float Distance = FVector::Dist(
					FVector(static_cast<float>(Origin->GetFName().GetNumber()), 0, 0),
					FVector(static_cast<float>(Destination->GetFName().GetNumber()), 0, 0)
				);
				float TravelTime = TravelSpeed > 0.0f ? Distance / TravelSpeed : 0.0f;
				float ProfitabilityScore = TravelTime > 0.0f ? ProfitPerUnit / TravelTime : ProfitPerUnit;
				
				// Track best destination for this item-origin pair
				if (ProfitabilityScore > BestProfitability)
				{
					BestProfitability = ProfitabilityScore;
					BestDestination = Destination;
				}
			}
			
			// Add only the best route for this item-origin combination
			if (BestDestination && BestProfitability > 0.0f)
			{
				FTradeRoute Route;
				Route.OriginMarket = Origin;
				Route.DestinationMarket = BestDestination;
				Route.TradeItem = Entry.TradeItem;
				
				float RouteBuyPrice = Origin->GetItemPrice(Entry.TradeItem, true);
				float RouteSellPrice = BestDestination->GetItemPrice(Entry.TradeItem, false);
				Route.ProfitPerUnit = RouteSellPrice - RouteBuyPrice;
				
				Route.Distance = FVector::Dist(
					FVector(static_cast<float>(Origin->GetFName().GetNumber()), 0, 0),
					FVector(static_cast<float>(BestDestination->GetFName().GetNumber()), 0, 0)
				);
				Route.TravelTime = TravelSpeed > 0.0f ? Route.Distance / TravelSpeed : 0.0f;
				Route.ProfitabilityScore = BestProfitability;
				
				BestRoutes.Add(Route);
			}
		}
	}

	// Sort by profitability score (only if we need to limit results)
	if (BestRoutes.Num() > MaxRoutes)
	{
		BestRoutes.Sort([](const FTradeRoute& A, const FTradeRoute& B) {
			return A.ProfitabilityScore > B.ProfitabilityScore;
		});
		BestRoutes.SetNum(MaxRoutes);
	}

	return BestRoutes;
}

FTradeRoute UAITraderComponent::CalculateArbitrageOpportunity(UTradeItemDataAsset* TradeItem)
{
	FTradeRoute BestRoute;
	
	if (!TradeItem || !CurrentLocation)
	{
		return BestRoute;
	}

	float BestProfit = 0.0f;
	
	// Check each destination market
	for (UMarketDataAsset* DestMarket : KnownMarkets)
	{
		if (!DestMarket || DestMarket == CurrentLocation)
		{
			continue;
		}

		// Get buy price at current location
		float BuyPrice = CurrentLocation->GetItemPrice(TradeItem, true);
		
		// Get sell price at destination
		float SellPrice = DestMarket->GetItemPrice(TradeItem, false);
		
		// Calculate profit per unit
		float ProfitPerUnit = SellPrice - BuyPrice;
		
		// Calculate profit margin
		float ProfitMargin = BuyPrice > 0.0f ? (ProfitPerUnit / BuyPrice) : 0.0f;
		
		// Check if meets minimum profit margin
		if (ProfitMargin >= MinProfitMargin)
		{
			// Calculate travel time (simplified)
			float Distance = FVector::Dist(
				FVector(CurrentLocation->GetFName().GetNumber(), 0, 0),
				FVector(DestMarket->GetFName().GetNumber(), 0, 0)
			);
			float TravelTime = TravelSpeed > 0.0f ? Distance / TravelSpeed : 0.0f;
			
			// Calculate profitability score (profit per unit / travel time)
			float ProfitabilityScore = TravelTime > 0.0f ? ProfitPerUnit / TravelTime : ProfitPerUnit;
			
			if (ProfitabilityScore > BestProfit)
			{
				BestProfit = ProfitabilityScore;
				BestRoute.OriginMarket = CurrentLocation;
				BestRoute.DestinationMarket = DestMarket;
				BestRoute.TradeItem = TradeItem;
				BestRoute.ProfitPerUnit = ProfitPerUnit;
				BestRoute.Distance = Distance;
				BestRoute.TravelTime = TravelTime;
				BestRoute.ProfitabilityScore = ProfitabilityScore;
			}
		}
	}
	
	return BestRoute;
}

bool UAITraderComponent::ExecuteTrade(UTradeItemDataAsset* TradeItem, int32 Quantity, bool bIsBuying)
{
	if (!TradeItem || !CurrentLocation || Quantity <= 0)
	{
		return false;
	}

	// Check if we should execute this trade
	if (!OnEvaluateTrade(TradeItem, CurrentLocation, bIsBuying))
	{
		return false;
	}

	float Price = CurrentLocation->GetItemPrice(TradeItem, bIsBuying);
	int32 TotalCost = FMath::RoundToInt(Price * Quantity);

	if (bIsBuying)
	{
		// Check if we have enough capital
		if (TotalCost > TradingCapital)
		{
			return false;
		}

		// Check cargo space
		float RequiredSpace = TradeItem->GetTotalVolume(Quantity);
		if (RequiredSpace > GetAvailableCargoSpace())
		{
			return false;
		}

		// Execute buy
		TradingCapital -= TotalCost;
		
		// Add to inventory
		FAITraderInventory NewInventory;
		NewInventory.TradeItem = TradeItem;
		NewInventory.Quantity = Quantity;
		NewInventory.PurchasePrice = Price;
		NewInventory.PurchaseMarket = CurrentLocation;
		Inventory.Add(NewInventory);
	}
	else
	{
		// Find item in inventory
		int32 FoundIndex = -1;
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			if (Inventory[i].TradeItem == TradeItem)
			{
				FoundIndex = i;
				break;
			}
		}

		if (FoundIndex == -1 || Inventory[FoundIndex].Quantity < Quantity)
		{
			return false;
		}

		// Execute sell
		TradingCapital += TotalCost;
		
		// Calculate profit
		int32 PurchaseCost = FMath::RoundToInt(Inventory[FoundIndex].PurchasePrice * Quantity);
		int32 Profit = TotalCost - PurchaseCost;
		TotalProfit += Profit;
		
		// Remove from inventory
		Inventory[FoundIndex].Quantity -= Quantity;
		if (Inventory[FoundIndex].Quantity <= 0)
		{
			Inventory.RemoveAt(FoundIndex);
		}

		SuccessfulTrades++;
	}

	OnTradeExecuted(TradeItem, Quantity, Price, bIsBuying);
	return true;
}

bool UAITraderComponent::EvaluateContract(UTradeContractDataAsset* Contract)
{
	if (!Contract)
	{
		return false;
	}

	// Check if we accept contracts
	if (!bAcceptsContracts)
	{
		return false;
	}

	// Check cargo capacity
	float RequiredVolume = Contract->GetTotalCargoVolume();
	if (RequiredVolume > CargoCapacity)
	{
		return false;
	}

	// Check capital for cargo acquisition
	// (Simplified - would need to calculate actual cargo costs)
	
	// Evaluate profitability
	int32 EstimatedProfit = Contract->CalculateProfitMargin(0, 0);
	
	// Risk-adjusted profit threshold
	float ProfitThreshold = 1000.0f * (1.0f - RiskTolerance);
	
	return EstimatedProfit >= ProfitThreshold;
}

bool UAITraderComponent::AcceptContract(UTradeContractDataAsset* Contract)
{
	if (!EvaluateContract(Contract))
	{
		return false;
	}

	// Accept the contract
	if (Contract->AcceptContract(FName(*GetOwner()->GetName()), 0.0f))
	{
		ActiveContracts.Add(Contract);
		return true;
	}

	return false;
}

bool UAITraderComponent::AttemptMarketManipulation(UMarketDataAsset* TargetMarket, UTradeItemDataAsset* TradeItem, float DesiredPriceChange)
{
	if (!bCanManipulatePrices || !TargetMarket || !TradeItem)
	{
		return false;
	}

	// Calculate quantity needed to influence price
	// (Simplified - actual implementation would be more complex)
	int32 ManipulationQuantity = FMath::RoundToInt(TradeItem->TypicalMarketStock * 0.1f);

	if (DesiredPriceChange > 0)
	{
		// Buy to increase price
		return ExecuteTrade(TradeItem, ManipulationQuantity, true);
	}
	else
	{
		// Sell to decrease price
		return ExecuteTrade(TradeItem, ManipulationQuantity, false);
	}
}

void UAITraderComponent::ReactToMarketEvent(UMarketDataAsset* Market, FName EventID)
{
	if (!Market || !IsBehaviorEnabled(EAITradeBehavior::EventReaction))
	{
		return;
	}

	// Get events affecting items we trade
	// React accordingly (buy low, sell high based on event)
	// This is a simplified implementation
}

void UAITraderComponent::ReactToPlayerAction(UMarketDataAsset* Market, UTradeItemDataAsset* TradeItem, int32 Quantity, bool bPlayerBought)
{
	if (!Market || !TradeItem)
	{
		return;
	}

	// Competitive behavior - do opposite of player if aggressive
	if (Strategy == EAITraderStrategy::Aggressive && IsBehaviorEnabled(EAITradeBehavior::MarketMaking))
	{
		// If player bought, consider selling (and vice versa)
		ExecuteTrade(TradeItem, FMath::Max(1, Quantity / 2), !bPlayerBought);
	}
}

float UAITraderComponent::TravelToMarket(UMarketDataAsset* DestinationMarket)
{
	if (!DestinationMarket || !CurrentLocation)
	{
		return 0.0f;
	}

	// Calculate travel time using placeholder distance (name hash)
	// TODO: Replace with actual market world positions when available
	float Distance = FVector::Dist(
		FVector(static_cast<float>(CurrentLocation->GetFName().GetNumber()), 0, 0),
		FVector(static_cast<float>(DestinationMarket->GetFName().GetNumber()), 0, 0)
	);
	
	float TravelTime = TravelSpeed > 0.0f ? Distance / TravelSpeed : 0.0f;
	
	CurrentLocation = DestinationMarket;
	
	return TravelTime;
}

float UAITraderComponent::GetCargoUsage() const
{
	float UsedSpace = 0.0f;
	
	for (const FAITraderInventory& Item : Inventory)
	{
		if (Item.TradeItem)
		{
			UsedSpace += Item.TradeItem->GetTotalVolume(Item.Quantity);
		}
	}
	
	return CargoCapacity > 0.0f ? UsedSpace / CargoCapacity : 0.0f;
}

float UAITraderComponent::GetAvailableCargoSpace() const
{
	float UsedSpace = 0.0f;
	
	for (const FAITraderInventory& Item : Inventory)
	{
		if (Item.TradeItem)
		{
			UsedSpace += Item.TradeItem->GetTotalVolume(Item.Quantity);
		}
	}
	
	return FMath::Max(0.0f, CargoCapacity - UsedSpace);
}

bool UAITraderComponent::IsBehaviorEnabled(EAITradeBehavior Behavior) const
{
	return EnabledBehaviors.Contains(Behavior);
}

void UAITraderComponent::DiscoverMarket(UMarketDataAsset* Market)
{
	if (Market && !IsMarketKnown(Market))
	{
		KnownMarkets.Add(Market);
	}
}

bool UAITraderComponent::IsMarketKnown(UMarketDataAsset* Market) const
{
	return KnownMarkets.Contains(Market);
}

// BlueprintNativeEvent implementations
bool UAITraderComponent::OnEvaluateTrade_Implementation(UTradeItemDataAsset* TradeItem, UMarketDataAsset* Market, bool bIsBuying)
{
	// Default implementation allows trade
	return true;
}

void UAITraderComponent::OnTradeExecuted_Implementation(UTradeItemDataAsset* TradeItem, int32 Quantity, float Price, bool bWasBuying)
{
	// Default implementation does nothing
}

void UAITraderComponent::OnContractCompleted_Implementation(UTradeContractDataAsset* Contract, int32 Profit)
{
	// Default implementation does nothing
}

// Private helper functions
void UAITraderComponent::MakeTradeDecisions()
{
	if (!CurrentLocation)
	{
		return;
	}

	// Based on strategy, make trading decisions
	switch (Strategy)
	{
		case EAITraderStrategy::Aggressive:
			// Look for high-risk, high-reward opportunities
			if (IsBehaviorEnabled(EAITradeBehavior::Arbitrage))
			{
				TArray<FTradeRoute> Routes = FindBestTradeRoutes(3);
				// Execute top routes
			}
			break;

		case EAITraderStrategy::Conservative:
			// Focus on safe, steady profits
			if (IsBehaviorEnabled(EAITradeBehavior::MarketMaking))
			{
				// Buy low, sell high with low-risk items
			}
			break;

		case EAITraderStrategy::Opportunistic:
			// React to events and opportunities
			if (IsBehaviorEnabled(EAITradeBehavior::EventReaction))
			{
				// Check for market events and react
			}
			break;

		default:
			break;
	}
}

void UAITraderComponent::OptimizeTradeRoutes()
{
	if (!IsBehaviorEnabled(EAITradeBehavior::RoutePlanning))
	{
		return;
	}

	// Find and update best trade routes
	ActiveRoutes = FindBestTradeRoutes(10);
}

void UAITraderComponent::ManageInventory()
{
	// Sell old inventory, manage cargo space
	// Remove items that have been held too long
}
