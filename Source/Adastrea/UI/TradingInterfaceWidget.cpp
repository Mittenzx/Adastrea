// Copyright Mittenzx. All Rights Reserved.

#include "UI/TradingInterfaceWidget.h"
#include "Trading/TradeTransaction.h"
#include "Factions/FactionDataAsset.h"
#include "TimerManager.h"

UTradingInterfaceWidget::UTradingInterfaceWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TradePartnerFaction(nullptr)
	, SelectedCategory("")
	, SortMode("Name")
	, bShowBuyView(true)
{
}

void UTradingInterfaceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start periodic price update timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PriceUpdateTimer, this,
			&UTradingInterfaceWidget::RefreshAvailableItems, 5.0f, true);
	}
}

void UTradingInterfaceWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PriceUpdateTimer);
	}

	Super::NativeDestruct();
}

void UTradingInterfaceWidget::SetTradePartner(UFactionDataAsset* Faction)
{
	TradePartnerFaction = Faction;
	RefreshAvailableItems();
}

TArray<UTradeItemDataAsset*> UTradingInterfaceWidget::GetFilteredItems(const FString& Category) const
{
	TArray<UTradeItemDataAsset*> FilteredItems;

	for (UTradeItemDataAsset* Item : AvailableItems)
	{
		if (!Item)
		{
			continue;
		}

		// Apply category filter if specified
		if (Category.IsEmpty() || Item->Category.ToString() == Category)
		{
			FilteredItems.Add(Item);
		}
	}

	return FilteredItems;
}

TArray<UTradeItemDataAsset*> UTradingInterfaceWidget::GetSortedItems() const
{
	TArray<UTradeItemDataAsset*> Items = GetFilteredItems(SelectedCategory);

	// Sort based on current sort mode
	if (SortMode == "Price")
	{
		Items.Sort([this](const UTradeItemDataAsset& A, const UTradeItemDataAsset& B) {
			float PriceA = GetItemPrice(&A, 1, bShowBuyView);
			float PriceB = GetItemPrice(&B, 1, bShowBuyView);
			return PriceA < PriceB;
		});
	}
	else if (SortMode == "Name")
	{
		Items.Sort([](const UTradeItemDataAsset& A, const UTradeItemDataAsset& B) {
			return A.DisplayName.CompareTo(B.DisplayName) < 0;
		});
	}

	return Items;
}

float UTradingInterfaceWidget::GetItemPrice(const UTradeItemDataAsset* Item, int32 Quantity, bool bBuying) const
{
	if (!Item)
	{
		return 0.0f;
	}

	float BasePrice = Item->BasePrice * Quantity;
	float Modifier = GetFactionPriceModifier();

	// Buying costs more, selling gets less
	if (bBuying)
	{
		return BasePrice * (1.0f + (1.0f - Modifier) * 0.5f);
	}
	else
	{
		return BasePrice * Modifier * 0.8f; // Sell for less than buy
	}
}

int32 UTradingInterfaceWidget::GetPriceTrend(UTradeItemDataAsset* Item) const
{
	if (!Item)
	{
		return 0;
	}

	// Placeholder for price trend calculation
	// In full implementation, this would:
	// 1. Check historical price data
	// 2. Calculate trend based on supply/demand
	// 3. Return -1 (falling), 0 (stable), or 1 (rising)

	return 0; // Stable by default
}

void UTradingInterfaceWidget::AddToCart(UTradeItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return;
	}

	int32* ExistingQuantity = TransactionCart.Find(Item);
	if (ExistingQuantity)
	{
		*ExistingQuantity += Quantity;
	}
	else
	{
		TransactionCart.Add(Item, Quantity);
	}

	OnCartUpdated();
}

void UTradingInterfaceWidget::RemoveFromCart(UTradeItemDataAsset* Item)
{
	if (Item)
	{
		TransactionCart.Remove(Item);
		OnCartUpdated();
	}
}

void UTradingInterfaceWidget::ClearCart()
{
	TransactionCart.Empty();
	OnCartUpdated();
}

float UTradingInterfaceWidget::GetCartTotal() const
{
	float Total = 0.0f;

	for (const auto& CartItem : TransactionCart)
	{
		Total += GetItemPrice(CartItem.Key, CartItem.Value, bShowBuyView);
	}

	return Total;
}

bool UTradingInterfaceWidget::ExecuteTrade()
{
	if (TransactionCart.Num() == 0)
	{
		return false;
	}

	// Placeholder for trade execution logic
	// In full implementation:
	// 1. Check player has enough credits/items
	// 2. Validate transaction
	// 3. Transfer items and credits
	// 4. Update faction relations
	// 5. Clear cart

	bool bSuccess = true; // Placeholder
	OnTradeCompleted(bSuccess);

	if (bSuccess)
	{
		ClearCart();
	}

	return bSuccess;
}

void UTradingInterfaceWidget::SetCategoryFilter(const FString& Category)
{
	SelectedCategory = Category;
	OnTradeItemsUpdated();
}

void UTradingInterfaceWidget::SetSortMode(const FString& Mode)
{
	SortMode = Mode;
	OnTradeItemsUpdated();
}

void UTradingInterfaceWidget::ToggleBuySellView()
{
	bShowBuyView = !bShowBuyView;
	OnTradeItemsUpdated();
}

TArray<UFactionDataAsset*> UTradingInterfaceWidget::GetSuggestedTradeRoutes() const
{
	TArray<UFactionDataAsset*> SuggestedFactions;

	// Placeholder for trade route suggestions
	// In full implementation:
	// 1. Analyze items in cart
	// 2. Find factions with high demand for those items
	// 3. Calculate profitability
	// 4. Return sorted list of best trade destinations

	return SuggestedFactions;
}

void UTradingInterfaceWidget::CloseInterface()
{
	ClearCart();
	RemoveFromParent();
}

void UTradingInterfaceWidget::RefreshAvailableItems()
{
	if (!TradePartnerFaction)
	{
		return;
	}

	// Placeholder for item refresh logic
	// In full implementation:
	// 1. Query faction's available trade items
	// 2. Update quantities based on supply/demand
	// 3. Recalculate prices
	// 4. Fire update event

	OnTradeItemsUpdated();
}

float UTradingInterfaceWidget::GetFactionPriceModifier() const
{
	if (!TradePartnerFaction)
	{
		return 1.0f;
	}

	// Placeholder for faction relationship price modifier
	// In full implementation:
	// 1. Get player's reputation with faction
	// 2. Calculate modifier based on relationship
	// 3. Return multiplier (0.5 to 1.5 range typical)

	return 1.0f; // Neutral by default
}
