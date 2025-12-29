// Copyright Mittenzx. All Rights Reserved.

#include "UI/TradingInterfaceWidget.h"
#include "Trading/TradeItemDataAsset.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/CargoComponent.h"
#include "Trading/EconomyManager.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UTradingInterfaceWidget::UTradingInterfaceWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentMarket(nullptr)
	, PlayerTrader(nullptr)
	, PlayerCargo(nullptr)
	, EconomyManager(nullptr)
	, SelectedCategory("")
	, SortMode("Name")
	, bShowBuyView(true)
	, SelectedItem(nullptr)
{
}

void UTradingInterfaceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize component references
	InitializeComponents();

	// Bind to component events
	BindComponentEvents();

	// Start periodic update timer for market prices
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this,
			&UTradingInterfaceWidget::RefreshMarketDisplay, 5.0f, true);
	}
}

void UTradingInterfaceWidget::NativeDestruct()
{
	// Unbind from component events
	UnbindComponentEvents();

	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimer);
	}

	Super::NativeDestruct();
}

// ========================================================================
// Market Setup
// ========================================================================

bool UTradingInterfaceWidget::OpenMarket(UMarketDataAsset* Market)
{
	if (!Market)
	{
		return false;
	}

	CurrentMarket = Market;

	// Initialize components if not already done
	if (!PlayerTrader || !PlayerCargo)
	{
		InitializeComponents();
	}

	// Validate we have required components
	if (!PlayerTrader || !PlayerCargo || !EconomyManager)
	{
		return false;
	}

	// Clear shopping cart
	ClearCart();

	// Refresh display
	RefreshMarketDisplay();
	UpdatePlayerState();

	return true;
}

void UTradingInterfaceWidget::CloseMarket()
{
	// Clear cart before closing
	ClearCart();

	// Clear current market
	CurrentMarket = nullptr;

	// Remove widget from viewport
	RemoveFromParent();
}

// ========================================================================
// Market Information
// ========================================================================

TArray<FMarketInventoryEntry> UTradingInterfaceWidget::GetAvailableItems() const
{
	if (!CurrentMarket)
	{
		return TArray<FMarketInventoryEntry>();
	}

	return CurrentMarket->Inventory;
}

TArray<FMarketInventoryEntry> UTradingInterfaceWidget::GetFilteredItems(ETradeItemCategory Category) const
{
	if (!CurrentMarket)
	{
		return TArray<FMarketInventoryEntry>();
	}

	// If no category filter, return all items
	if (SelectedCategory.IsEmpty())
	{
		return CurrentMarket->Inventory;
	}

	// Filter by category
	TArray<FMarketInventoryEntry> FilteredItems;
	for (const FMarketInventoryEntry& Entry : CurrentMarket->Inventory)
	{
		if (Entry.TradeItem && Entry.TradeItem->Category == Category)
		{
			FilteredItems.Add(Entry);
		}
	}

	return FilteredItems;
}

int32 UTradingInterfaceWidget::GetItemPrice(UTradeItemDataAsset* Item, int32 Quantity) const
{
	if (!Item || !PlayerTrader || !CurrentMarket || Quantity <= 0)
	{
		return 0;
	}

	if (bShowBuyView)
	{
		return PlayerTrader->GetBuyCost(CurrentMarket, Item, Quantity);
	}
	else
	{
		return PlayerTrader->GetSellValue(CurrentMarket, Item, Quantity);
	}
}

int32 UTradingInterfaceWidget::GetPlayerCredits() const
{
	if (!PlayerTrader)
	{
		return 0;
	}

	return PlayerTrader->GetCredits();
}

void UTradingInterfaceWidget::GetCargoSpaceInfo(float& OutUsedSpace, float& OutTotalSpace, float& OutAvailableSpace) const
{
	if (!PlayerCargo)
	{
		OutUsedSpace = 0.0f;
		OutTotalSpace = 0.0f;
		OutAvailableSpace = 0.0f;
		return;
	}

	OutTotalSpace = PlayerCargo->CargoCapacity;
	OutAvailableSpace = PlayerCargo->GetAvailableCargoSpace();
	OutUsedSpace = OutTotalSpace - OutAvailableSpace;
}

int32 UTradingInterfaceWidget::GetPlayerProfit() const
{
	if (!PlayerTrader)
	{
		return 0;
	}

	return PlayerTrader->GetProfit();
}

// ========================================================================
// Shopping Cart
// ========================================================================

bool UTradingInterfaceWidget::AddToCart(UTradeItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return false;
	}

	// Check if we're buying or selling
	if (bShowBuyView)
	{
		// Buying: Check market has stock
		if (!CurrentMarket || !CurrentMarket->IsItemInStock(Item->ItemID, Quantity))
		{
			return false;
		}
	}
	else
	{
		// Selling: Check player has item in cargo
		if (!PlayerCargo)
		{
			return false;
		}

		int32 PlayerQuantity = PlayerCargo->GetItemQuantity(Item);
		if (PlayerQuantity < Quantity)
		{
			return false;
		}
	}

	// Add or update quantity in cart
	int32* ExistingQuantity = ShoppingCart.Find(Item);
	if (ExistingQuantity)
	{
		*ExistingQuantity += Quantity;
	}
	else
	{
		ShoppingCart.Add(Item, Quantity);
	}

	// Fire update event
	OnCartUpdated();

	return true;
}

void UTradingInterfaceWidget::RemoveFromCart(UTradeItemDataAsset* Item)
{
	if (Item)
	{
		ShoppingCart.Remove(Item);
		OnCartUpdated();
	}
}

void UTradingInterfaceWidget::UpdateCartQuantity(UTradeItemDataAsset* Item, int32 NewQuantity)
{
	if (!Item)
	{
		return;
	}

	if (NewQuantity <= 0)
	{
		RemoveFromCart(Item);
		return;
	}

	ShoppingCart.Add(Item, NewQuantity);
	OnCartUpdated();
}

void UTradingInterfaceWidget::ClearCart()
{
	ShoppingCart.Empty();
	OnCartUpdated();
}

int32 UTradingInterfaceWidget::GetCartTotal() const
{
	int32 Total = 0;

	for (const auto& CartItem : ShoppingCart)
	{
		Total += GetItemPrice(CartItem.Key, CartItem.Value);
	}

	return Total;
}

int32 UTradingInterfaceWidget::GetCartItemCount() const
{
	int32 Count = 0;
	for (const auto& CartItem : ShoppingCart)
	{
		Count += CartItem.Value;
	}
	return Count;
}

// ========================================================================
// Transaction Validation
// ========================================================================

bool UTradingInterfaceWidget::CanAffordCart() const
{
	if (!PlayerTrader)
	{
		return false;
	}

	// If selling, player always "can afford"
	if (!bShowBuyView)
	{
		return true;
	}

	// If buying, check credits
	int32 CartTotal = GetCartTotal();
	return PlayerTrader->GetCredits() >= CartTotal;
}

bool UTradingInterfaceWidget::HasCargoSpaceForCart() const
{
	if (!PlayerCargo)
	{
		return false;
	}

	// If selling, we're removing from cargo (always has "space")
	if (!bShowBuyView)
	{
		return true;
	}

	// If buying, check cargo space
	float RequiredSpace = 0.0f;
	for (const auto& CartItem : ShoppingCart)
	{
		if (CartItem.Key)
		{
			RequiredSpace += CartItem.Key->GetTotalVolume(CartItem.Value);
		}
	}

	return PlayerCargo->GetAvailableCargoSpace() >= RequiredSpace;
}

bool UTradingInterfaceWidget::ValidateTransaction(FText& OutErrorMessage) const
{
	if (ShoppingCart.Num() == 0)
	{
		OutErrorMessage = FText::FromString(TEXT("Shopping cart is empty"));
		return false;
	}

	if (!CurrentMarket)
	{
		OutErrorMessage = FText::FromString(TEXT("No market selected"));
		return false;
	}

	if (!PlayerTrader || !PlayerCargo)
	{
		OutErrorMessage = FText::FromString(TEXT("Player components not initialized"));
		return false;
	}

	if (bShowBuyView)
	{
		// Buying validation
		if (!CanAffordCart())
		{
			OutErrorMessage = FText::FromString(TEXT("Insufficient credits"));
			return false;
		}

		if (!HasCargoSpaceForCart())
		{
			OutErrorMessage = FText::FromString(TEXT("Insufficient cargo space"));
			return false;
		}
	}
	else
	{
		// Selling validation - check player actually has items
		for (const auto& CartItem : ShoppingCart)
		{
			int32 PlayerQuantity = PlayerCargo->GetItemQuantity(CartItem.Key);
			if (PlayerQuantity < CartItem.Value)
			{
				OutErrorMessage = FText::Format(
					FText::FromString(TEXT("Insufficient {0} in cargo")),
					CartItem.Key->ItemName
				);
				return false;
			}
		}
	}

	OutErrorMessage = FText::GetEmpty();
	return true;
}

// ========================================================================
// Execute Trade
// ========================================================================

bool UTradingInterfaceWidget::ExecuteTrade()
{
	// Validate transaction
	FText ErrorMessage;
	if (!ValidateTransaction(ErrorMessage))
	{
		OnTradeCompleted(false, ErrorMessage);
		return false;
	}

	// Execute each item in cart
	bool bAllSuccess = true;
	for (const auto& CartItem : ShoppingCart)
	{
		bool bSuccess = false;

		if (bShowBuyView)
		{
			// Buy item
			bSuccess = PlayerTrader->BuyItem(CurrentMarket, CartItem.Key, CartItem.Value, PlayerCargo);
		}
		else
		{
			// Sell item
			bSuccess = PlayerTrader->SellItem(CurrentMarket, CartItem.Key, CartItem.Value, PlayerCargo);
		}

		if (!bSuccess)
		{
			bAllSuccess = false;
			break;
		}
	}

	if (bAllSuccess)
	{
		// Clear cart on success
		ClearCart();

		// Update displays
		RefreshMarketDisplay();
		UpdatePlayerState();

		OnTradeCompleted(true, FText::GetEmpty());
	}
	else
	{
		OnTradeCompleted(false, FText::FromString(TEXT("Trade execution failed")));
	}

	return bAllSuccess;
}

// ========================================================================
// UI Controls
// ========================================================================

void UTradingInterfaceWidget::SetCategoryFilter(ETradeItemCategory Category)
{
	SelectedCategory = UEnum::GetValueAsString(Category);
	RefreshMarketDisplay();
}

void UTradingInterfaceWidget::SetSortMode(const FString& Mode)
{
	SortMode = Mode;
	RefreshMarketDisplay();
}

void UTradingInterfaceWidget::ToggleBuySellView()
{
	bShowBuyView = !bShowBuyView;
	ClearCart();
	RefreshMarketDisplay();
}

void UTradingInterfaceWidget::SetSelectedItem(UTradeItemDataAsset* Item)
{
	SelectedItem = Item;
}

// ========================================================================
// Private Helper Functions
// ========================================================================

void UTradingInterfaceWidget::InitializeComponents()
{
	// Get player controller and pawn
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	// Find player trader component
	PlayerTrader = PlayerPawn->FindComponentByClass<UPlayerTraderComponent>();

	// Find cargo component
	PlayerCargo = PlayerPawn->FindComponentByClass<UCargoComponent>();

	// Get economy manager subsystem
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		EconomyManager = GetWorld()->GetGameInstance()->GetSubsystem<UEconomyManager>();
	}
}

void UTradingInterfaceWidget::RefreshMarketDisplay()
{
	if (!CurrentMarket)
	{
		return;
	}

	// Fire update event for Blueprint to rebuild UI
	OnMarketInventoryUpdated();
}

void UTradingInterfaceWidget::UpdatePlayerState()
{
	if (!PlayerTrader || !PlayerCargo)
	{
		return;
	}

	// Update credits display
	OnCreditsUpdated(PlayerTrader->GetCredits());

	// Update cargo space display
	float UsedSpace, TotalSpace, AvailableSpace;
	GetCargoSpaceInfo(UsedSpace, TotalSpace, AvailableSpace);
	OnCargoSpaceUpdated(UsedSpace, TotalSpace);
}

void UTradingInterfaceWidget::BindComponentEvents()
{
	if (!PlayerTrader || !PlayerCargo)
	{
		return;
	}

	// Bind to trader events
	PlayerTrader->OnCreditsChanged.AddDynamic(this, &UTradingInterfaceWidget::OnPlayerCreditsChanged);

	// Bind to cargo events
	PlayerCargo->OnCargoAdded.AddDynamic(this, &UTradingInterfaceWidget::OnPlayerCargoChanged);
	PlayerCargo->OnCargoRemoved.AddDynamic(this, &UTradingInterfaceWidget::OnPlayerCargoChanged);
}

void UTradingInterfaceWidget::UnbindComponentEvents()
{
	if (PlayerTrader)
	{
		PlayerTrader->OnCreditsChanged.RemoveDynamic(this, &UTradingInterfaceWidget::OnPlayerCreditsChanged);
	}

	if (PlayerCargo)
	{
		PlayerCargo->OnCargoAdded.RemoveDynamic(this, &UTradingInterfaceWidget::OnPlayerCargoChanged);
		PlayerCargo->OnCargoRemoved.RemoveDynamic(this, &UTradingInterfaceWidget::OnPlayerCargoChanged);
	}
}

void UTradingInterfaceWidget::OnPlayerCreditsChanged(int32 NewCredits, int32 ChangeAmount)
{
	OnCreditsUpdated(NewCredits);
}

void UTradingInterfaceWidget::OnPlayerCargoChanged(UTradeItemDataAsset* Item, int32 Quantity)
{
	float UsedSpace, TotalSpace, AvailableSpace;
	GetCargoSpaceInfo(UsedSpace, TotalSpace, AvailableSpace);
	OnCargoSpaceUpdated(UsedSpace, TotalSpace);
}
