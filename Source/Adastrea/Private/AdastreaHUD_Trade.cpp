// Copyright Epic Games, Inc. All Rights Reserved.

// Docked trade screen for AAdastreaHUD (canvas-drawn, laid out on a 1920-wide page
// scaled to the viewport by AdastreaHUDStyle::LayoutScale).
//
// Two views, Tab switches:
//   Market         category rail | commodity table (prices, best price elsewhere in the
//                  sector, supply / demand, stock, held) | selected item: where it trades,
//                  quantity, cost breakdown and the confirm button
//   Opportunities  the three most profitable buy-here / sell-there runs to other markets
//                  in the level, the highlighted run's item across those markets, and the
//                  hold as a grid with the run's cargo outlined
//
// "Known markets" are every marketplace in the loaded level; there is no price memory
// yet, so the screen reads their live prices.

#include "AdastreaHUD.h"
#include "AdastreaHUDStyle.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Stations/MarketplaceModule.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/TradeItemDataAsset.h"
#include "Player/AdastreaPlayerController.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "Misc/ScopeExit.h"

namespace
{
	// sRGB hex (as picked in the mock-ups) to the linear colour the canvas expects.
	FLinearColor Hex(uint32 RGB, float A = 1.0f)
	{
		FLinearColor C(FColor((RGB >> 16) & 0xFF, (RGB >> 8) & 0xFF, RGB & 0xFF));
		C.A = A;
		return C;
	}

	const FLinearColor kTGround  = Hex(0x060A10, 0.97f);
	const FLinearColor kTHeader  = Hex(0x080D15);
	const FLinearColor kTRail    = Hex(0x070C13);
	const FLinearColor kTPanel   = Hex(0x0A111A);
	const FLinearColor kTCard    = Hex(0x0C1520);
	const FLinearColor kTRaised  = Hex(0x0F1824);
	const FLinearColor kTLine    = Hex(0x1A2636);
	const FLinearColor kTRowLine = Hex(0x111B28);
	const FLinearColor kTTrack   = Hex(0x142030);
	const FLinearColor kTText    = Hex(0xDCE6F0);
	const FLinearColor kTBright  = Hex(0xF1F6FB);
	const FLinearColor kTMuted   = Hex(0x8C9CAF);
	const FLinearColor kTDim     = Hex(0x6F8095);
	const FLinearColor kTFaint   = Hex(0x4A586A);
	const FLinearColor kTAccent  = Hex(0x3FD0C2);
	const FLinearColor kTBuy     = Hex(0x4DA3FF);
	const FLinearColor kTBuySoft = Hex(0x8FC4FF);
	const FLinearColor kTSell    = Hex(0xFF9F43);
	const FLinearColor kTSellSoft= Hex(0xFFC58F);
	const FLinearColor kTGold    = Hex(0xF2C14E);
	const FLinearColor kTGood    = Hex(0x5BD69A);
	const FLinearColor kTBad     = Hex(0xFF6B5A);
	const FLinearColor kTSupply  = Hex(0x4F6A86);
	const FLinearColor kTDemand  = Hex(0xC98A4B);

	// Hold segments and grid cells, one colour per cargo stack.
	const FLinearColor kTStack[] = {
		Hex(0xC07A52), Hex(0x9FB4C8), Hex(0x7FD08A), Hex(0x8EA2FF),
		Hex(0xE6B85C), Hex(0x6CD0E0), Hex(0xE58FB0), Hex(0xB0A07A)
	};

	constexpr int32 kTCategoryCount = static_cast<int32>(ETradeItemCategory::Other) + 1;

	FString CategoryName(int32 Category)
	{
		return StaticEnum<ETradeItemCategory>()->GetDisplayNameTextByValue(Category).ToString();
	}

	FLinearColor CategoryTint(ETradeItemCategory Category)
	{
		switch (Category)
		{
		case ETradeItemCategory::RawMaterials: return Hex(0xE3B293);
		case ETradeItemCategory::RefinedGoods: return Hex(0xCFDBE7);
		case ETradeItemCategory::Components:   return Hex(0xA9C6EA);
		case ETradeItemCategory::Technology:   return Hex(0xB7C3FF);
		case ETradeItemCategory::Food:         return Hex(0xA8E3B0);
		case ETradeItemCategory::Luxury:       return Hex(0xF0D08A);
		case ETradeItemCategory::Contraband:   return Hex(0xF09A9A);
		case ETradeItemCategory::Military:     return Hex(0xD6C9A0);
		case ETradeItemCategory::Medical:      return Hex(0xF2B4CB);
		case ETradeItemCategory::Data:         return Hex(0x9FE3EE);
		default:                               return Hex(0xC4CAD3);
		}
	}

	// Short chip label: the element or formula for the raw goods, else initials.
	FString TradeSymbol(const FString& Name)
	{
		static const TPair<const TCHAR*, const TCHAR*> Known[] = {
			{TEXT("Iron"), TEXT("Fe")}, {TEXT("Copper"), TEXT("Cu")}, {TEXT("Titanium"), TEXT("Ti")},
			{TEXT("Nickel"), TEXT("Ni")}, {TEXT("Zinc"), TEXT("Zn")}, {TEXT("Aluminium"), TEXT("Al")},
			{TEXT("Aluminum"), TEXT("Al")}, {TEXT("Chromium"), TEXT("Cr")}, {TEXT("Manganese"), TEXT("Mn")},
			{TEXT("Gold"), TEXT("Au")}, {TEXT("Silver"), TEXT("Ag")}, {TEXT("Platinum"), TEXT("Pt")},
			{TEXT("Palladium"), TEXT("Pd")}, {TEXT("Cobalt"), TEXT("Co")}, {TEXT("Tungsten"), TEXT("W")},
			{TEXT("Uranium"), TEXT("U")}, {TEXT("Lithium"), TEXT("Li")}, {TEXT("Silicon"), TEXT("Si")},
			{TEXT("Helium"), TEXT("He")}, {TEXT("Hydrogen"), TEXT("H2")}, {TEXT("Water"), TEXT("H2O")},
			{TEXT("Methane"), TEXT("CH4")}, {TEXT("Nitrogen"), TEXT("N2")}, {TEXT("Carbon"), TEXT("C")},
			{TEXT("Noble Gas"), TEXT("Ne")}, {TEXT("Rare Earth"), TEXT("REE")}
		};
		for (const TPair<const TCHAR*, const TCHAR*>& K : Known)
		{
			if (Name.StartsWith(K.Key))
			{
				return K.Value;
			}
		}
		TArray<FString> Words;
		Name.ParseIntoArray(Words, TEXT(" "));
		FString Out;
		for (int32 i = 0; i < Words.Num() && Out.Len() < 2; ++i)
		{
			Out.AppendChar(FChar::ToUpper(Words[i][0]));
		}
		return Out.IsEmpty() ? FString(TEXT("?")) : Out;
	}

	FString Cr(int32 Value) { return FText::AsNumber(Value).ToString() + TEXT(" cr"); }
	FString Num(int32 Value) { return FText::AsNumber(Value).ToString(); }
	FString KmText(float Km) { return Km < 10.0f ? FString::Printf(TEXT("%.1f km"), Km) : FString::Printf(TEXT("%.0f km"), Km); }

	// The market entry for an item by ID (stations build their own item objects).
	const FMarketInventoryEntry* FindEntry(const UMarketDataAsset* Market, const UTradeItemDataAsset* Item)
	{
		if (!Market || !Item)
		{
			return nullptr;
		}
		for (const FMarketInventoryEntry& E : Market->Inventory)
		{
			if (E.TradeItem && (E.TradeItem == Item || E.TradeItem->ItemID == Item->ItemID))
			{
				return &E;
			}
		}
		return nullptr;
	}

	/** Drawing helpers bound to one HUD for the frame. */
	struct FTradePen
	{
		AAdastreaHUD* HUD;
		UFont* Font;

		void Text(const FString& S, const FLinearColor& C, float X, float Y, float Scale) const
		{
			HUD->DrawText(S, C, X, Y, Font, Scale);
		}
		float Width(const FString& S, float Scale) const
		{
			float W = 0.0f, H = 0.0f;
			HUD->GetTextSize(S, W, H, Font, Scale);
			return W;
		}
		float Height(float Scale) const
		{
			float W = 0.0f, H = 0.0f;
			HUD->GetTextSize(TEXT("Ag"), W, H, Font, Scale);
			return H;
		}
		void Right(const FString& S, const FLinearColor& C, float RightX, float Y, float Scale) const
		{
			Text(S, C, RightX - Width(S, Scale), Y, Scale);
		}
		void Centre(const FString& S, const FLinearColor& C, float CX, float Y, float Scale) const
		{
			Text(S, C, CX - Width(S, Scale) * 0.5f, Y, Scale);
		}
		/** Text centred vertically in a band of height H starting at Y. */
		void Mid(const FString& S, const FLinearColor& C, float X, float Y, float H, float Scale) const
		{
			Text(S, C, X, Y + (H - Height(Scale)) * 0.5f, Scale);
		}
		void Rect(const FLinearColor& C, float X, float Y, float W, float H) const
		{
			if (W > 0.0f && H > 0.0f)
			{
				HUD->DrawRect(C, X, Y, W, H);
			}
		}
		void Frame(const FLinearColor& C, float X, float Y, float W, float H, float T = 1.0f) const
		{
			Rect(C, X, Y, W, T);
			Rect(C, X, Y + H - T, W, T);
			Rect(C, X, Y, T, H);
			Rect(C, X + W - T, Y, T, H);
		}
		/** A keycap: bordered key label. Returns its width. */
		float Key(const FString& K, float X, float Y) const
		{
			const float W = Width(K, HudType::Caption) + 16.0f;
			Frame(Hex(0x2A3A4E), X, Y, W, 24.0f);
			Rect(Hex(0x2A3A4E), X, Y + 22.0f, W, 2.0f);
			Mid(K, kTText, X + 8.0f, Y, 22.0f, HudType::Caption);
			return W;
		}
		/** Commodity chip with its symbol. */
		void Chip(const UTradeItemDataAsset* Item, float X, float Y, float Size, float Scale) const
		{
			const FLinearColor Tint = Item ? CategoryTint(Item->Category) : kTMuted;
			Rect(FLinearColor(Tint.R * 0.16f, Tint.G * 0.16f, Tint.B * 0.16f, 1.0f), X, Y, Size, Size);
			Frame(FLinearColor(Tint.R * 0.45f, Tint.G * 0.45f, Tint.B * 0.45f, 1.0f), X, Y, Size, Size);
			const FString Sym = Item ? TradeSymbol(Item->ItemName.ToString()) : FString(TEXT("?"));
			Text(Sym, Tint, X + (Size - Width(Sym, Scale)) * 0.5f, Y + (Size - Height(Scale)) * 0.5f, Scale);
		}
	};
}

// ---------------------------------------------------------------------------
// State and input
// ---------------------------------------------------------------------------

void AAdastreaHUD::ShowTradeScreen()
{
	bShowTradeScreen = true;
	SelectedTradeIndex = 0;
	TradeTab = 0;
	TradeCategory = 0;
	TradeQuantity = 1;
	TradeRunIndex = 0;
	TradeMessage.Reset();
	TradeCacheTime = -100.0;
	TradeSessionStartCredits = 0;
	if (const AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(GetOwningPlayerController()))
	{
		if (const ASpaceship* Ship = AdPC->GetControlledSpaceship())
		{
			TradeSessionStartCredits = Ship->PlayerTraderComponent ? Ship->PlayerTraderComponent->GetCredits() : 0;
		}
	}
	// Start on the first listed item, not a possibly-empty slot 0.
	if (const UMarketDataAsset* Market = GetDockedMarket())
	{
		for (int32 i = 0; i < Market->Inventory.Num(); ++i)
		{
			if (Market->Inventory[i].TradeItem)
			{
				SelectedTradeIndex = i;
				break;
			}
		}
	}
}

UMarketDataAsset* AAdastreaHUD::GetDockedMarket() const
{
	const AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(GetOwningPlayerController());
	ASpaceStation* Station = AdPC ? AdPC->GetNearestTradableStation() : nullptr;
	AMarketplaceModule* Module = Station ? Station->GetMarketplaceModule() : nullptr;
	return Module ? Module->GetMarketData() : nullptr;
}

bool AAdastreaHUD::IsTradeEntryVisible(const FMarketInventoryEntry& Entry) const
{
	return Entry.TradeItem && (TradeCategory == 0 || static_cast<int32>(Entry.TradeItem->Category) + 1 == TradeCategory);
}

void AAdastreaHUD::MoveTradeSelection(int32 Step)
{
	if (TradeTab == 1)
	{
		TradeRunIndex = FMath::Clamp(TradeRunIndex + Step, 0, FMath::Max(TradeRuns.Num() - 1, 0));
		return;
	}
	const UMarketDataAsset* Market = GetDockedMarket();
	if (!Market || Step == 0)
	{
		return;
	}
	// Step to the next item the category filter shows; stay put at either end.
	for (int32 i = SelectedTradeIndex + Step; Market->Inventory.IsValidIndex(i); i += FMath::Sign(Step))
	{
		if (IsTradeEntryVisible(Market->Inventory[i]))
		{
			SelectedTradeIndex = i;
			TradeQuantity = 1;
			return;
		}
	}
}

void AAdastreaHUD::ToggleTradeTab()
{
	TradeTab = TradeTab == 0 ? 1 : 0;
	TradeCacheTime = -100.0;
}

void AAdastreaHUD::CycleTradeCategory(int32 Step)
{
	const UMarketDataAsset* Market = GetDockedMarket();
	if (!Market || TradeTab != 0)
	{
		return;
	}
	// All, then each category this market actually lists.
	TArray<int32> Filters = { 0 };
	for (int32 c = 0; c < kTCategoryCount; ++c)
	{
		if (Market->Inventory.ContainsByPredicate([c](const FMarketInventoryEntry& E)
			{ return E.TradeItem && static_cast<int32>(E.TradeItem->Category) == c; }))
		{
			Filters.Add(c + 1);
		}
	}
	const int32 At = FMath::Max(Filters.IndexOfByKey(TradeCategory), 0);
	TradeCategory = Filters[(At + Step + Filters.Num()) % Filters.Num()];
	for (int32 i = 0; i < Market->Inventory.Num(); ++i)
	{
		if (IsTradeEntryVisible(Market->Inventory[i]))
		{
			SelectedTradeIndex = i;
			break;
		}
	}
	TradeQuantity = 1;
}

int32 AAdastreaHUD::GetTradeMaxQuantity() const
{
	const AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(GetOwningPlayerController());
	const ASpaceship* Ship = AdPC ? AdPC->GetControlledSpaceship() : nullptr;
	UMarketDataAsset* Market = GetDockedMarket();
	if (!Ship || !Ship->PlayerTraderComponent || !Ship->CargoComponent || !Market || !Market->Inventory.IsValidIndex(SelectedTradeIndex))
	{
		return 0;
	}
	const FMarketInventoryEntry& Entry = Market->Inventory[SelectedTradeIndex];
	return bBuyMode
		? GetMaxBuyQuantity(Ship->PlayerTraderComponent, Ship->CargoComponent, Market, Entry, MAX_int32)
		: UPlayerTraderComponent::GetSellableQuantity(Market, Entry.TradeItem, Ship->CargoComponent);
}

void AAdastreaHUD::AdjustTradeQuantity(int32 Step)
{
	TradeQuantity = FMath::Clamp(TradeQuantity + Step, 1, FMath::Max(GetTradeMaxQuantity(), 1));
}

void AAdastreaHUD::SetTradeQuantityMax()
{
	TradeQuantity = FMath::Max(GetTradeMaxQuantity(), 1);
}

void AAdastreaHUD::PlanSelectedTradeRun()
{
	if (!TradeRuns.IsValidIndex(TradeRunIndex))
	{
		return;
	}
	const FTradeRun& Run = TradeRuns[TradeRunIndex];
	TradeTab = 0;
	TradeCategory = 0;
	bBuyMode = true;
	SelectedTradeIndex = Run.InventoryIndex;
	SetTradeQuantityMax();
}

int32 AAdastreaHUD::GetMaxBuyQuantity(const UPlayerTraderComponent* Trader, const UCargoComponent* Cargo,
	UMarketDataAsset* Market, const FMarketInventoryEntry& Entry, int32 Wanted, FString* OutLimit)
{
	UTradeItemDataAsset* Item = Entry.TradeItem;
	if (!Trader || !Cargo || !Market || !Item || Wanted <= 0)
	{
		return 0;
	}
	int32 N = Wanted;
	auto Cap = [&N, OutLimit](int32 Max, const TCHAR* Why)
	{
		if (Max < N)
		{
			N = FMath::Max(Max, 0);
			if (OutLimit) { *OutLimit = Why; }
		}
	};
	Cap(Entry.CurrentStock, TEXT("stock"));
	const int32 UnitCost = Trader->GetBuyCost(Market, Item, 1);
	if (UnitCost > 0)
	{
		Cap(Trader->GetCredits() / UnitCost, TEXT("credits"));
	}
	if (Item->VolumePerUnit > KINDA_SMALL_NUMBER)
	{
		Cap(FMath::FloorToInt(Cargo->GetAvailableCargoSpace() / Item->VolumePerUnit + KINDA_SMALL_NUMBER), TEXT("hold space"));
	}
	// Rounding in the totals can tip the last unit over; settle against the real checks.
	while (N > 0 && !Trader->CanAfford(Market, Item, N))
	{
		--N;
		if (OutLimit) { *OutLimit = TEXT("credits"); }
	}
	while (N > 0 && !Cargo->HasSpaceFor(Item, N))
	{
		--N;
		if (OutLimit) { *OutLimit = TEXT("hold space"); }
	}
	return N;
}

void AAdastreaHUD::SetTradeMessage(const FString& Message, bool bSuccess)
{
	TradeMessage = Message;
	bTradeMessageOK = bSuccess;
	TradeMessageTime = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
	TradeCacheTime = -100.0; // prices, stock and the hold just changed
	TradeQuantity = FMath::Clamp(TradeQuantity, 1, FMath::Max(GetTradeMaxQuantity(), 1));
}

void AAdastreaHUD::RefreshTradeCache(ASpaceship* Ship, ASpaceStation* Station, UMarketDataAsset* Here, bool bForce)
{
	const double Now = GetWorld()->GetRealTimeSeconds();
	if (!bForce && TradeCacheMarket.Get() == Here && Now - TradeCacheTime < 1.0)
	{
		return;
	}
	TradeCacheTime = Now;
	TradeCacheMarket = Here;
	TradeMarkets.Reset();
	TradeRuns.Reset();

	// Every marketplace in the level, once per market asset (a hub can run two modules on one).
	TradeMarkets.Add({ Here, Here->MarketName.IsEmpty() ? Here->GetName() : Here->MarketName.ToString(), 0.0f, true });
	for (TActorIterator<ASpaceStation> It(GetWorld()); It; ++It)
	{
		for (AMarketplaceModule* Module : It->GetMarketplaceModules())
		{
			UMarketDataAsset* M = Module ? Module->GetMarketData() : nullptr;
			if (!M || TradeMarkets.ContainsByPredicate([M](const FTradeMarketRef& R) { return R.Market.Get() == M; }))
			{
				continue;
			}
			const float Km = Ship ? FVector::Dist(Ship->GetActorLocation(), It->GetActorLocation()) / 100000.0f : 0.0f;
			TradeMarkets.Add({ M, M->MarketName.IsEmpty() ? M->GetName() : M->MarketName.ToString(), Km, false });
		}
	}
	TradeMarkets.Sort([](const FTradeMarketRef& A, const FTradeMarketRef& B)
		{ return A.bHere != B.bHere ? A.bHere : A.DistanceKm < B.DistanceKm; });

	// Best destination per item, then the three that earn most for what you can buy and carry.
	UPlayerTraderComponent* Trader = Ship ? Ship->PlayerTraderComponent.Get() : nullptr;
	UCargoComponent* Cargo = Ship ? Ship->CargoComponent.Get() : nullptr;
	if (!Trader || !Cargo)
	{
		return;
	}
	for (int32 i = 0; i < Here->Inventory.Num(); ++i)
	{
		const FMarketInventoryEntry& Entry = Here->Inventory[i];
		if (!Entry.TradeItem || Entry.CurrentStock <= 0)
		{
			continue;
		}
		const int32 BuyHere = Trader->GetBuyCost(Here, Entry.TradeItem, 1);
		if (BuyHere <= 0)
		{
			continue;
		}
		FTradeRun Best;
		for (int32 m = 0; m < TradeMarkets.Num(); ++m)
		{
			UMarketDataAsset* Other = TradeMarkets[m].Market.Get();
			if (TradeMarkets[m].bHere || !FindEntry(Other, Entry.TradeItem))
			{
				continue;
			}
			const int32 SellThere = Trader->GetSellValue(Other, Entry.TradeItem, 1);
			if (SellThere - BuyHere > Best.SellThere - Best.BuyHere)
			{
				Best = { i, m, BuyHere, SellThere, 0 };
			}
		}
		if (Best.MarketRef == INDEX_NONE)
		{
			continue;
		}
		Best.Units = GetMaxBuyQuantity(Trader, Cargo, Here, Entry, MAX_int32);
		if (Best.Units > 0)
		{
			TradeRuns.Add(Best);
		}
	}
	TradeRuns.Sort([](const FTradeRun& A, const FTradeRun& B)
		{
			const int64 PA = int64(A.SellThere - A.BuyHere) * A.Units, PB = int64(B.SellThere - B.BuyHere) * B.Units;
			return PA != PB ? PA > PB : (A.SellThere - A.BuyHere) > (B.SellThere - B.BuyHere);
		});
	if (TradeRuns.Num() > 3)
	{
		TradeRuns.SetNum(3);
	}
	TradeRunIndex = FMath::Clamp(TradeRunIndex, 0, FMath::Max(TradeRuns.Num() - 1, 0));
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

void AAdastreaHUD::DrawTradeScreen(APlayerController* PC, AAdastreaPlayerController* AdController, ASpaceship* Ship)
{
	if (!Ship || !AdController)
	{
		return;
	}
	ASpaceStation* Station = AdController->GetNearestTradableStation();
	UMarketDataAsset* Market = (Station && Station->GetMarketplaceModule()) ? Station->GetMarketplaceModule()->GetMarketData() : nullptr;
	if (!Market || !Ship->PlayerTraderComponent || !Ship->CargoComponent)
	{
		DrawText(!Market ? TEXT("No active market at this dock.") : TEXT("This ship has no cargo hold to trade with."),
			kTSell, 200.0f, 200.0f, HudType::Font(), HudType::Body);
		return;
	}
	if (!Market->Inventory.IsValidIndex(SelectedTradeIndex) || !IsTradeEntryVisible(Market->Inventory[SelectedTradeIndex]))
	{
		TradeCategory = 0;
		SelectedTradeIndex = FMath::Clamp(SelectedTradeIndex, 0, FMath::Max(Market->Inventory.Num() - 1, 0));
	}
	RefreshTradeCache(Ship, Station, Market);

	// A 1920-wide page scaled to the viewport (adastrea.HUDScale included).
	const float S = HudType::LayoutScale(Canvas->SizeX, Canvas->SizeY);
	const float VW = Canvas->SizeX / S;
	const float VH = Canvas->SizeY / S;
	Canvas->Canvas->PushRelativeTransform(FScaleMatrix(FVector(S, S, 1.0f)));
	ON_SCOPE_EXIT { Canvas->Canvas->PopTransform(); };

	const FTradePen P{ this, HudType::Font() };
	UPlayerTraderComponent* Trader = Ship->PlayerTraderComponent;
	UCargoComponent* Cargo = Ship->CargoComponent;
	constexpr float M = 40.0f;
	constexpr float HeaderH = 116.0f;
	constexpr float FooterH = 56.0f;

	P.Rect(kTGround, 0.0f, 0.0f, VW, VH);

	// ---- Header: market, view tabs, credits, hold.
	P.Rect(kTHeader, 0.0f, 0.0f, VW, HeaderH);
	P.Rect(kTLine, 0.0f, HeaderH - 1.0f, VW, 1.0f);
	P.Text(TEXT("DOCKED  ·  TRADING DEPARTMENT"), kTAccent, M, 20.0f, HudType::Caption);
	const FString MarketName = Market->MarketName.IsEmpty() ? Market->GetName() : Market->MarketName.ToString();
	P.Text(MarketName, kTBright, M, 38.0f, 1.7f);
	int32 Listed = 0;
	for (const FMarketInventoryEntry& E : Market->Inventory) { Listed += E.TradeItem ? 1 : 0; }
	P.Text(FString::Printf(TEXT("%d commodities listed  ·  %d other markets in this sector"), Listed, FMath::Max(TradeMarkets.Num() - 1, 0)),
		kTMuted, M, 80.0f, HudType::Label);

	// View tabs.
	{
		const float TabX = FMath::Max(M + P.Width(MarketName, 1.7f) + 48.0f, 600.0f);
		const float TabY = 36.0f, TabH = 44.0f;
		const TCHAR* Tabs[] = { TEXT("MARKET"), TEXT("OPPORTUNITIES") };
		float X = TabX + 5.0f;
		const float Total = P.Width(Tabs[0], HudType::Heading) + P.Width(Tabs[1], HudType::Heading) + 4 * 22.0f + 14.0f;
		P.Rect(Hex(0x0D1520), TabX, TabY, Total, TabH);
		P.Frame(kTLine, TabX, TabY, Total, TabH);
		for (int32 t = 0; t < 2; ++t)
		{
			const float W = P.Width(Tabs[t], HudType::Heading) + 44.0f;
			if (t == TradeTab)
			{
				P.Rect(Hex(0x16273A), X, TabY + 5.0f, W, TabH - 10.0f);
			}
			P.Mid(Tabs[t], t == TradeTab ? kTBright : kTMuted, X + 22.0f, TabY + 5.0f, TabH - 10.0f, HudType::Heading);
			X += W + 4.0f;
		}
		P.Key(TEXT("TAB"), TabX + Total + 12.0f, TabY + 10.0f);
	}

	// Hold block (right) and credits (left of it).
	{
		const float HoldW = 380.0f;
		const float HX = VW - M - HoldW;
		const float Cap = FMath::Max(Cargo->CargoCapacity, 0.01f);
		const float Free = FMath::Max(Cargo->GetAvailableCargoSpace(), 0.0f);
		P.Text(TEXT("CARGO HOLD"), kTMuted, HX, 22.0f, HudType::Caption);
		P.Right(FString::Printf(TEXT("%.1f / %.1f"), Cap - Free, Cap), kTText, HX + HoldW, 20.0f, HudType::Label);
		P.Rect(kTTrack, HX, 44.0f, HoldW, 12.0f);
		float SegX = HX;
		int32 StackIdx = 0;
		float LegendX = HX;
		for (const FCargoEntry& C : Cargo->CargoInventory)
		{
			if (!C.Item || C.Quantity <= 0)
			{
				continue;
			}
			const FLinearColor Col = kTStack[StackIdx % UE_ARRAY_COUNT(kTStack)];
			const float W = HoldW * FMath::Clamp(C.Item->GetTotalVolume(C.Quantity) / Cap, 0.0f, 1.0f);
			P.Rect(Col, SegX, 44.0f, FMath::Max(W - 2.0f, 2.0f), 12.0f);
			SegX += W;
			if (StackIdx < 3)
			{
				const FString L = FString::Printf(TEXT("%s %d"), *C.Item->ItemName.ToString(), C.Quantity);
				P.Rect(Col, LegendX, 72.0f, 9.0f, 9.0f);
				P.Text(L, kTMuted, LegendX + 14.0f, 66.0f, HudType::Caption);
				LegendX += P.Width(L, HudType::Caption) + 30.0f;
			}
			++StackIdx;
		}
		if (StackIdx > 3)
		{
			P.Text(FString::Printf(TEXT("+%d more"), StackIdx - 3), kTDim, LegendX, 66.0f, HudType::Caption);
		}
		P.Right(StackIdx == 0 ? FString(TEXT("empty")) : FString::Printf(TEXT("%.1f free"), Free), kTMuted, HX + HoldW, 86.0f, HudType::Caption);

		const float CR = HX - 40.0f;
		P.Rect(kTLine, CR + 20.0f, 22.0f, 1.0f, 72.0f);
		P.Right(TEXT("CREDITS"), kTMuted, CR, 22.0f, HudType::Caption);
		P.Right(Num(Trader->GetCredits()), kTGold, CR - 28.0f, 36.0f, 1.8f);
		P.Right(TEXT("cr"), Hex(0xB8964A), CR, 50.0f, HudType::Body);
		const int32 Delta = Trader->GetCredits() - TradeSessionStartCredits;
		if (Delta != 0)
		{
			P.Right(FString::Printf(TEXT("this visit %s%s"), Delta > 0 ? TEXT("+") : TEXT("-"), *Num(FMath::Abs(Delta))),
				Delta > 0 ? kTGood : kTSellSoft, CR, 84.0f, HudType::Caption);
		}
	}

	if (TradeTab == 0)
	{
		DrawTradeMarketView(Ship, Station, Market, VW, VH);
	}
	else
	{
		DrawTradeOpportunitiesView(Ship, Station, Market, VW, VH);
	}

	// ---- Footer: keys and the last trade result.
	const float FY = VH - FooterH;
	P.Rect(kTHeader, 0.0f, FY, VW, FooterH);
	P.Rect(kTLine, 0.0f, FY, VW, 1.0f);
	{
		TArray<TPair<FString, FString>> Keys;
		if (TradeTab == 0)
		{
			Keys = { {TEXT("UP/DOWN"), TEXT("Select")}, {TEXT("[ ]"), TEXT("Category")}, {TEXT("B/S"), TEXT("Buy / Sell")},
				{TEXT("LEFT/RIGHT"), TEXT("Quantity")}, {TEXT("X"), TEXT("Max")}, {TEXT("ENTER"), TEXT("Confirm")}, {TEXT("ESC"), TEXT("Back")} };
		}
		else
		{
			Keys = { {TEXT("UP/DOWN"), TEXT("Select run")}, {TEXT("ENTER"), TEXT("Plan run")}, {TEXT("TAB"), TEXT("Market")}, {TEXT("ESC"), TEXT("Back")} };
		}
		float X = M;
		for (const TPair<FString, FString>& K : Keys)
		{
			X += P.Key(K.Key, X, FY + 16.0f) + 8.0f;
			P.Mid(K.Value, kTMuted, X, FY + 16.0f, 24.0f, HudType::Label);
			X += P.Width(K.Value, HudType::Label) + 28.0f;
		}
	}
	const double Age = GetWorld()->GetRealTimeSeconds() - TradeMessageTime;
	if (!TradeMessage.IsEmpty() && Age < 5.0)
	{
		FLinearColor Col = bTradeMessageOK ? kTGood : kTBad;
		Col.A = Age < 4.0 ? 1.0f : static_cast<float>(5.0 - Age);
		const float W = P.Width(TradeMessage, HudType::Body);
		P.Rect(Col, VW - M - W - 18.0f, FY + 22.0f, 8.0f, 8.0f);
		P.Mid(TradeMessage, Col, VW - M - W, FY, FooterH, HudType::Body);
	}
}

void AAdastreaHUD::DrawTradeMarketView(ASpaceship* Ship, ASpaceStation* Station, UMarketDataAsset* Market, float VW, float VH)
{
	const FTradePen P{ this, HudType::Font() };
	UPlayerTraderComponent* Trader = Ship->PlayerTraderComponent;
	UCargoComponent* Cargo = Ship->CargoComponent;
	constexpr float Top = 116.0f;
	const float Bottom = VH - 56.0f;
	constexpr float RailW = 270.0f;
	constexpr float SideW = 520.0f;
	const FLinearColor ModeCol = bBuyMode ? kTBuy : kTSell;
	const FLinearColor ModeSoft = bBuyMode ? kTBuySoft : kTSellSoft;

	// ---- Category rail.
	P.Rect(kTRail, 0.0f, Top, RailW, Bottom - Top);
	P.Rect(kTLine, RailW - 1.0f, Top, 1.0f, Bottom - Top);
	{
		int32 Counts[kTCategoryCount] = {};
		int32 All = 0;
		for (const FMarketInventoryEntry& E : Market->Inventory)
		{
			if (E.TradeItem)
			{
				++Counts[static_cast<int32>(E.TradeItem->Category)];
				++All;
			}
		}
		float Y = Top + 24.0f;
		P.Text(TEXT("CATEGORIES"), kTDim, 28.0f, Y, HudType::Caption);
		P.Key(TEXT("["), RailW - 76.0f, Y - 4.0f);
		P.Key(TEXT("]"), RailW - 46.0f, Y - 4.0f);
		Y += 34.0f;
		auto Row = [&](const FString& Name, int32 Count, bool bSel)
		{
			if (bSel)
			{
				P.Rect(Hex(0x12202F), 14.0f, Y, RailW - 28.0f, 40.0f);
				P.Rect(kTAccent, 14.0f, Y, 3.0f, 40.0f);
			}
			P.Mid(Name, bSel ? kTBright : Hex(0x9AAABD), 28.0f, Y, 40.0f, HudType::Body);
			P.Right(Num(Count), kTDim, RailW - 26.0f, Y + 12.0f, HudType::Label);
			Y += 42.0f;
		};
		Row(TEXT("All commodities"), All, TradeCategory == 0);
		for (int32 c = 0; c < kTCategoryCount; ++c)
		{
			if (Counts[c] > 0)
			{
				Row(CategoryName(c), Counts[c], TradeCategory == c + 1);
			}
		}
		// About this market.
		if (!Market->Description.IsEmpty() && Y < Bottom - 190.0f)
		{
			const float BY = Bottom - 180.0f;
			P.Rect(kTCard, 14.0f, BY, RailW - 28.0f, 164.0f);
			P.Frame(kTLine, 14.0f, BY, RailW - 28.0f, 164.0f);
			P.Text(TEXT("ABOUT THIS MARKET"), kTDim, 28.0f, BY + 14.0f, HudType::Caption);
			FString Desc = Market->Description.ToString();
			if (Desc.Len() > 150) { Desc = Desc.Left(147).TrimEnd() + TEXT("..."); }
			DrawWrappedText(Desc, Hex(0x9AAABD), 28.0f, BY + 38.0f, RailW - 56.0f, HudType::Font(), HudType::Label);
		}
	}

	// ---- Commodity table.
	const float TX = RailW + 28.0f;
	const float TW = VW - SideW - TX - 28.0f;
	const float TR = TX + TW;
	{
		// Toolbar: buy / sell switch and the filter summary.
		const float BarY = Top + 20.0f;
		P.Rect(Hex(0x0D1520), TX, BarY, 196.0f, 42.0f);
		P.Frame(kTLine, TX, BarY, 196.0f, 42.0f);
		P.Rect(bBuyMode ? Hex(0x12304F) : Hex(0x3A240F), TX + 4.0f + (bBuyMode ? 0.0f : 94.0f), BarY + 4.0f, 94.0f, 34.0f);
		P.Centre(TEXT("BUY"), bBuyMode ? kTBuySoft : kTMuted, TX + 51.0f, BarY + 11.0f, HudType::Heading);
		P.Centre(TEXT("SELL"), bBuyMode ? kTMuted : kTSellSoft, TX + 145.0f, BarY + 11.0f, HudType::Heading);
		P.Key(TEXT("B/S"), TX + 208.0f, BarY + 9.0f);
		int32 Shown = 0;
		for (const FMarketInventoryEntry& E : Market->Inventory) { Shown += IsTradeEntryVisible(E) ? 1 : 0; }
		P.Mid(FString::Printf(TEXT("%s  ·  %d items"), TradeCategory == 0 ? TEXT("All commodities") : *CategoryName(TradeCategory - 1), Shown),
			kTMuted, TX + 268.0f, BarY, 42.0f, HudType::Body);
		P.Right(bBuyMode ? TEXT("Buying from the station") : TEXT("Selling from your hold"), ModeSoft, TR, BarY + 12.0f, HudType::Label);
	}

	// Columns, right edges from the table's right side.
	const float CHold = TR - 16.0f;
	const float CStock = CHold - 78.0f;
	const float CDemand = CStock - 196.0f;   // bar starts here (96 wide)
	const float CSupply = CDemand - 116.0f;
	const float CElse = CSupply - 28.0f;     // right edge
	const float CSell = CElse - 150.0f;
	const float CBuy = CSell - 96.0f;
	const float HeadY = Top + 84.0f;
	P.Text(TEXT("COMMODITY"), kTDim, TX + 16.0f, HeadY, HudType::Caption);
	P.Right(TEXT("BUY"), bBuyMode ? kTBuySoft : kTDim, CBuy, HeadY, HudType::Caption);
	P.Right(TEXT("SELL"), bBuyMode ? kTDim : kTSellSoft, CSell, HeadY, HudType::Caption);
	P.Right(TEXT("BEST ELSEWHERE"), kTDim, CElse, HeadY, HudType::Caption);
	P.Text(TEXT("SUPPLY"), kTDim, CSupply, HeadY, HudType::Caption);
	P.Text(TEXT("DEMAND"), kTDim, CDemand, HeadY, HudType::Caption);
	P.Right(TEXT("STOCK"), kTDim, CStock, HeadY, HudType::Caption);
	P.Right(TEXT("HOLD"), kTDim, CHold, HeadY, HudType::Caption);
	P.Rect(kTLine, TX, HeadY + 24.0f, TW, 1.0f);

	TArray<int32> Visible;
	for (int32 i = 0; i < Market->Inventory.Num(); ++i)
	{
		if (IsTradeEntryVisible(Market->Inventory[i]))
		{
			Visible.Add(i);
		}
	}
	const float RowH = 54.0f;
	const float RowsTop = HeadY + 26.0f;
	const int32 VisibleRows = FMath::Max(1, FMath::FloorToInt((Bottom - 34.0f - RowsTop) / RowH));
	const int32 SelPos = FMath::Max(Visible.IndexOfByKey(SelectedTradeIndex), 0);
	const int32 First = FMath::Clamp(SelPos - VisibleRows / 2, 0, FMath::Max(Visible.Num() - VisibleRows, 0));
	const int32 Last = FMath::Min(First + VisibleRows, Visible.Num());

	for (int32 v = First; v < Last; ++v)
	{
		const int32 i = Visible[v];
		const FMarketInventoryEntry& E = Market->Inventory[i];
		UTradeItemDataAsset* Item = E.TradeItem;
		const float Y = RowsTop + (v - First) * RowH;
		const bool bSel = i == SelectedTradeIndex;
		const int32 Held = Cargo->GetItemQuantity(Item);
		const int32 Buy = Trader->GetBuyCost(Market, Item, 1);
		const int32 Sell = Trader->GetSellValue(Market, Item, 1);
		const bool bAvailable = bBuyMode ? E.CurrentStock > 0 : UPlayerTraderComponent::GetSellableQuantity(Market, Item, Cargo) > 0;
		if (bSel)
		{
			P.Rect(FLinearColor(ModeCol.R, ModeCol.G, ModeCol.B, 0.11f), TX, Y, TW, RowH);
			P.Rect(ModeCol, TX, Y, 3.0f, RowH);
		}
		P.Rect(kTRowLine, TX, Y + RowH - 1.0f, TW, 1.0f);
		P.Chip(Item, TX + 16.0f, Y + 10.0f, 34.0f, HudType::Label);
		P.Text(Item->ItemName.ToString(), bSel ? FLinearColor::White : (bAvailable ? kTText : kTFaint), TX + 64.0f, Y + 8.0f, HudType::Body);
		P.Text(CategoryName(static_cast<int32>(Item->Category)), kTDim, TX + 64.0f, Y + 30.0f, HudType::Caption);
		P.Right(Cr(Buy), bBuyMode ? kTBuySoft : kTDim, CBuy, Y + 17.0f, HudType::Body);
		P.Right(Cr(Sell), bBuyMode ? kTDim : kTSellSoft, CSell, Y + 17.0f, HudType::Body);

		// Best sell price at another market, and what it means for this mode.
		int32 BestElse = 0;
		for (const FTradeMarketRef& R : TradeMarkets)
		{
			if (!R.bHere && FindEntry(R.Market.Get(), Item))
			{
				BestElse = FMath::Max(BestElse, Trader->GetSellValue(R.Market.Get(), Item, 1));
			}
		}
		if (BestElse > 0)
		{
			const int32 Edge = BestElse - (bBuyMode ? Buy : Sell);
			const FString Tag = FString::Printf(TEXT("%d  %s%d"), BestElse, Edge >= 0 ? TEXT("+") : TEXT("-"), FMath::Abs(Edge));
			P.Right(Tag, bBuyMode ? (Edge > 0 ? kTGood : kTDim) : (Edge > 0 ? kTSellSoft : kTDim), CElse, Y + 17.0f, HudType::Body);
		}
		else
		{
			P.Right(TEXT("—"), kTFaint, CElse, Y + 17.0f, HudType::Body);
		}
		P.Rect(kTTrack, CSupply, Y + 24.0f, 96.0f, 6.0f);
		P.Rect(kTSupply, CSupply, Y + 24.0f, 96.0f * FMath::Clamp(E.SupplyLevel / 2.0f, 0.03f, 1.0f), 6.0f);
		P.Rect(kTTrack, CDemand, Y + 24.0f, 96.0f, 6.0f);
		P.Rect(kTDemand, CDemand, Y + 24.0f, 96.0f * FMath::Clamp(E.DemandLevel / 2.0f, 0.03f, 1.0f), 6.0f);
		P.Right(Num(E.CurrentStock), E.CurrentStock > 0 ? Hex(0xB4C2D2) : kTFaint, CStock, Y + 17.0f, HudType::Body);
		P.Right(Held > 0 ? Num(Held) : FString(TEXT("—")), Held > 0 ? kTAccent : kTFaint, CHold, Y + 17.0f, HudType::Body);
	}
	if (Visible.Num() > VisibleRows)
	{
		P.Text(FString::Printf(TEXT("%d-%d of %d%s%s"), First + 1, Last, Visible.Num(),
			First > 0 ? TEXT("   ·   more above") : TEXT(""), Last < Visible.Num() ? TEXT("   ·   more below") : TEXT("")),
			kTDim, TX + 16.0f, Bottom - 28.0f, HudType::Caption);
	}

	// ---- Selected item panel.
	const float SX = VW - SideW;
	P.Rect(kTPanel, SX, Top, SideW, Bottom - Top);
	P.Rect(kTLine, SX, Top, 1.0f, Bottom - Top);
	if (!Market->Inventory.IsValidIndex(SelectedTradeIndex) || !Market->Inventory[SelectedTradeIndex].TradeItem)
	{
		return;
	}
	const FMarketInventoryEntry& Entry = Market->Inventory[SelectedTradeIndex];
	UTradeItemDataAsset* Item = Entry.TradeItem;
	const float PX = SX + 32.0f;
	const float PR = VW - 32.0f;
	const float PW = PR - PX;
	float Y = Top + 26.0f;

	P.Chip(Item, PX, Y, 56.0f, HudType::Heading);
	P.Text(Item->ItemName.ToString(), kTBright, PX + 72.0f, Y + 2.0f, 1.45f);
	P.Text(FString::Printf(TEXT("%s  ·  %.1f hold per unit  ·  %s"), *CategoryName(static_cast<int32>(Item->Category)), Item->VolumePerUnit,
		*StaticEnum<ELegalityStatus>()->GetDisplayNameTextByValue(static_cast<int64>(Item->LegalityStatus)).ToString()),
		kTMuted, PX + 72.0f, Y + 34.0f, HudType::Label);
	Y += 80.0f;

	// Where it trades (this market and every other one that lists it).
	P.Rect(kTCard, PX, Y, PW, 30.0f + 46.0f * FMath::Min(TradeMarkets.Num(), 5) + 12.0f);
	P.Frame(kTLine, PX, Y, PW, 30.0f + 46.0f * FMath::Min(TradeMarkets.Num(), 5) + 12.0f);
	P.Text(TEXT("WHERE IT TRADES"), kTDim, PX + 16.0f, Y + 12.0f, HudType::Caption);
	P.Rect(kTGold, PR - 190.0f, Y + 16.0f, 8.0f, 8.0f);
	P.Text(TEXT("pays you"), kTMuted, PR - 176.0f, Y + 10.0f, HudType::Caption);
	P.Rect(kTBuy, PR - 96.0f, Y + 16.0f, 8.0f, 8.0f);
	P.Text(TEXT("charges"), kTMuted, PR - 82.0f, Y + 10.0f, HudType::Caption);
	DrawTradeAcrossMarkets(Ship, Item, PX + 16.0f, Y + 36.0f, PW - 32.0f, 46.0f, 5);
	Y += 30.0f + 46.0f * FMath::Min(TradeMarkets.Num(), 5) + 26.0f;

	// Best place to sell, relative to this trade.
	{
		int32 Best = 0;
		const FTradeMarketRef* BestRef = nullptr;
		for (const FTradeMarketRef& R : TradeMarkets)
		{
			if (!R.bHere && FindEntry(R.Market.Get(), Item))
			{
				const int32 V = Trader->GetSellValue(R.Market.Get(), Item, 1);
				if (V > Best) { Best = V; BestRef = &R; }
			}
		}
		const int32 Ref = bBuyMode ? Trader->GetBuyCost(Market, Item, 1) : Trader->GetSellValue(Market, Item, 1);
		const bool bGood = BestRef && Best > Ref;
		const FLinearColor Edge = bGood ? (bBuyMode ? kTGood : kTSell) : kTLine;
		P.Rect(bGood ? (bBuyMode ? Hex(0x0F1E17) : Hex(0x1F150B)) : kTCard, PX, Y, PW, 64.0f);
		P.Frame(bGood ? FLinearColor(Edge.R * 0.45f, Edge.G * 0.45f, Edge.B * 0.45f, 1.0f) : kTLine, PX, Y, PW, 64.0f);
		if (bGood)
		{
			P.Text(FString::Printf(TEXT("%s pays %s"), *BestRef->Name, *Cr(Best)), kTBright, PX + 18.0f, Y + 11.0f, HudType::Body);
			P.Text(bBuyMode
				? FString::Printf(TEXT("+%d per unit over buying here  ·  %s"), Best - Ref, *KmText(BestRef->DistanceKm))
				: FString::Printf(TEXT("%d more per unit than here  ·  %s  ·  consider holding"), Best - Ref, *KmText(BestRef->DistanceKm)),
				bBuyMode ? Hex(0x8DB8A2) : kTSellSoft, PX + 18.0f, Y + 36.0f, HudType::Label);
		}
		else
		{
			P.Mid(BestRef ? (bBuyMode ? FString(TEXT("No market in this sector pays more than this costs."))
				: FString(TEXT("This is the best price in the sector.")))
				: FString(TEXT("No other market in this sector trades it.")),
				kTMuted, PX + 18.0f, Y, 64.0f, HudType::Body);
		}
		Y += 84.0f;
	}

	// Quantity.
	FString Limit;
	const int32 Held = Cargo->GetItemQuantity(Item);
	const int32 Sellable = UPlayerTraderComponent::GetSellableQuantity(Market, Item, Cargo);
	const int32 MaxQty = bBuyMode ? GetMaxBuyQuantity(Trader, Cargo, Market, Entry, MAX_int32, &Limit) : Sellable;
	const int32 Qty = FMath::Clamp(TradeQuantity, 1, FMath::Max(MaxQty, 1));
	P.Text(TEXT("QUANTITY"), kTDim, PX, Y, HudType::Caption);
	P.Right(MaxQty > 0 ? (bBuyMode ? FString::Printf(TEXT("MAX %d  ·  limited by %s"), MaxQty, *Limit) : FString::Printf(TEXT("YOU CAN SELL %d"), Sellable))
		: FString(bBuyMode ? (Limit == TEXT("stock") ? TEXT("OUT OF STOCK") : Limit == TEXT("credits") ? TEXT("NOT ENOUGH CREDITS") : TEXT("HOLD FULL"))
			: (Held > 0 ? TEXT("STOLEN - SELLS ONLY AT A FENCE") : TEXT("NONE IN YOUR HOLD"))),
		MaxQty > 0 ? kTDim : kTBad, PR, Y, HudType::Caption);
	Y += 24.0f;
	P.Rect(kTRaised, PX, Y, 52.0f, 52.0f);
	P.Frame(Hex(0x1F2E41), PX, Y, 52.0f, 52.0f);
	P.Centre(TEXT("-"), kTText, PX + 26.0f, Y + 12.0f, 1.4f);
	P.Rect(kTCard, PX + 62.0f, Y, PW - 124.0f, 52.0f);
	P.Frame(MaxQty > 0 ? FLinearColor(ModeCol.R * 0.55f, ModeCol.G * 0.55f, ModeCol.B * 0.55f, 1.0f) : Hex(0x1F2E41), PX + 62.0f, Y, PW - 124.0f, 52.0f);
	P.Centre(MaxQty > 0 ? Num(Qty) : FString(TEXT("0")), MaxQty > 0 ? kTBright : kTFaint, PX + PW * 0.5f, Y + 11.0f, 1.55f);
	P.Rect(kTRaised, PR - 52.0f, Y, 52.0f, 52.0f);
	P.Frame(Hex(0x1F2E41), PR - 52.0f, Y, 52.0f, 52.0f);
	P.Centre(TEXT("+"), kTText, PR - 26.0f, Y + 12.0f, 1.4f);
	Y += 66.0f;
	P.Rect(kTTrack, PX, Y, PW, 6.0f);
	if (MaxQty > 0)
	{
		const float F = FMath::Clamp(static_cast<float>(Qty) / MaxQty, 0.0f, 1.0f);
		P.Rect(ModeCol, PX, Y, PW * F, 6.0f);
		P.Rect(Hex(0xDCEBFF), PX + PW * F - 7.0f, Y - 5.0f, 14.0f, 16.0f);
		P.Frame(ModeCol, PX + PW * F - 7.0f, Y - 5.0f, 14.0f, 16.0f, 2.0f);
	}
	Y += 30.0f;

	// Breakdown.
	const int32 Unit = bBuyMode ? Trader->GetBuyCost(Market, Item, 1) : Trader->GetSellValue(Market, Item, 1);
	const int32 Total = MaxQty > 0 ? (bBuyMode ? Trader->GetBuyCost(Market, Item, Qty) : Trader->GetSaleValueFromHold(Market, Item, Qty, Cargo)) : 0;
	P.Rect(kTLine, PX, Y, PW, 1.0f);
	Y += 14.0f;
	P.Text(FString::Printf(TEXT("%s x %s"), *Num(MaxQty > 0 ? Qty : 0), *Cr(Unit)), Hex(0x9AAABD), PX, Y, HudType::Body);
	P.Right(FString::Printf(TEXT("%s%s"), bBuyMode ? TEXT("-") : TEXT("+"), *Cr(Total)), kTGold, PR, Y, HudType::Body);
	Y += 28.0f;
	const int32 CreditsAfter = Trader->GetCredits() + (bBuyMode ? -Total : Total);
	const float VolDelta = Item->GetTotalVolume(MaxQty > 0 ? Qty : 0) * (bBuyMode ? 1.0f : -1.0f);
	const float Cap = FMath::Max(Cargo->CargoCapacity, 0.01f);
	P.Text(TEXT("After trade"), kTMuted, PX, Y, HudType::Label);
	P.Right(FString::Printf(TEXT("%s  ·  hold %.1f / %.1f"), *Cr(CreditsAfter), Cap - Cargo->GetAvailableCargoSpace() + VolDelta, Cap), kTMuted, PR, Y, HudType::Label);
	Y += 24.0f;
	P.Rect(kTLine, PX, Y, PW, 1.0f);

	// Confirm button.
	const float BY = Bottom - 32.0f - 66.0f;
	const bool bCan = MaxQty > 0;
	P.Rect(bCan ? (bBuyMode ? Hex(0x2F7FDB) : Hex(0xD9792A)) : Hex(0x1A2433), PX, BY, PW, 66.0f);
	const FString Label = FString::Printf(TEXT("%s %s %s"), bBuyMode ? TEXT("BUY") : TEXT("SELL"), *Num(bCan ? Qty : 0), *Item->ItemName.ToString().ToUpper());
	const float LW = P.Width(Label, 1.2f) + 16.0f + P.Width(TEXT("ENTER"), HudType::Caption) + 16.0f;
	P.Mid(Label, bCan ? FLinearColor::White : kTFaint, PX + (PW - LW) * 0.5f, BY, 66.0f, 1.2f);
	if (bCan)
	{
		const float KX = PX + (PW - LW) * 0.5f + P.Width(Label, 1.2f) + 16.0f;
		const float KW = P.Width(TEXT("ENTER"), HudType::Caption) + 16.0f;
		P.Rect(bBuyMode ? Hex(0x2266B5) : Hex(0xA85A1C), KX, BY + 21.0f, KW, 24.0f);
		P.Mid(TEXT("ENTER"), FLinearColor::White, KX + 8.0f, BY + 21.0f, 24.0f, HudType::Caption);
	}
}

void AAdastreaHUD::DrawTradeAcrossMarkets(ASpaceship* Ship, UTradeItemDataAsset* Item, float X, float Y, float W, float RowH, int32 MaxRows)
{
	const FTradePen P{ this, HudType::Font() };
	UPlayerTraderComponent* Trader = Ship ? Ship->PlayerTraderComponent.Get() : nullptr;
	if (!Trader || !Item)
	{
		return;
	}
	struct FRow { const FTradeMarketRef* Ref; int32 Buy; int32 Sell; bool bListed; };
	TArray<FRow> Rows;
	int32 Lo = MAX_int32, Hi = 0;
	for (const FTradeMarketRef& R : TradeMarkets)
	{
		UMarketDataAsset* M = R.Market.Get();
		const bool bListed = FindEntry(M, Item) != nullptr;
		const int32 B = bListed ? Trader->GetBuyCost(M, Item, 1) : 0;
		const int32 S = bListed ? Trader->GetSellValue(M, Item, 1) : 0;
		Rows.Add({ &R, B, S, bListed });
		if (bListed)
		{
			Lo = FMath::Min(Lo, FMath::Min(B, S));
			Hi = FMath::Max(Hi, FMath::Max(B, S));
		}
	}
	// Here on top, then the markets that pay most, then ones that don't trade it; cut to MaxRows.
	Rows.StableSort([](const FRow& A, const FRow& B)
		{
			if (A.Ref->bHere != B.Ref->bHere) { return A.Ref->bHere; }
			if (A.bListed != B.bListed) { return A.bListed; }
			return A.Sell > B.Sell;
		});
	if (Rows.Num() > MaxRows)
	{
		Rows.SetNum(MaxRows);
	}
	if (Hi <= 0)
	{
		return;
	}
	const float Pad = FMath::Max((Hi - Lo) * 0.15f, 4.0f);
	const float Min = Lo - Pad, Span = (Hi - Lo) + Pad * 2.0f;
	const float NameW = FMath::Min(W * 0.42f, 230.0f);
	const float BX = X + NameW + 12.0f;
	const float BW = W - NameW - 12.0f;
	auto At = [&](int32 V) { return BX + BW * (V - Min) / Span; };

	for (int32 r = 0; r < Rows.Num(); ++r)
	{
		const FRow& Row = Rows[r];
		const float RY = Y + r * RowH;
		if (Row.Ref->bHere)
		{
			P.Rect(Hex(0x12202F), X - 8.0f, RY, W + 16.0f, RowH - 4.0f);
		}
		FString Name = Row.Ref->Name;
		if (P.Width(Name, HudType::Label) > NameW)
		{
			while (Name.Len() > 4 && P.Width(Name + TEXT("..."), HudType::Label) > NameW) { Name.LeftChopInline(1); }
			Name += TEXT("...");
		}
		P.Text(Name, Row.Ref->bHere ? kTBright : kTText, X, RY + 4.0f, HudType::Label);
		P.Text(Row.Ref->bHere ? FString(TEXT("you are here")) : KmText(Row.Ref->DistanceKm), kTDim, X, RY + 22.0f, HudType::Caption);
		if (!Row.bListed)
		{
			P.Mid(TEXT("doesn't trade it"), kTFaint, BX, RY, RowH - 4.0f, HudType::Caption);
			continue;
		}
		const float MidY = RY + (RowH - 4.0f) * 0.5f;
		P.Rect(Hex(0x1E2A3A), BX, MidY, BW, 1.0f);
		const float XS = At(Row.Sell), XB = At(Row.Buy);
		P.Rect(Hex(0x34465C), FMath::Min(XS, XB), MidY - 2.0f, FMath::Abs(XB - XS), 4.0f);
		P.Rect(kTGold, XS - 6.0f, MidY - 6.0f, 12.0f, 12.0f);
		P.Rect(kTBuy, XB - 5.0f, MidY - 5.0f, 10.0f, 10.0f);
		P.Right(Num(Row.Sell), kTGold, XS - 10.0f, MidY - 9.0f, HudType::Caption);
		P.Text(Num(Row.Buy), kTBuySoft, XB + 10.0f, MidY - 9.0f, HudType::Caption);
	}
}

void AAdastreaHUD::DrawTradeHoldGrid(ASpaceship* Ship, UTradeItemDataAsset* PlannedItem, int32 PlannedUnits, float X, float Y, float W, int32 Columns, int32 Rows)
{
	const FTradePen P{ this, HudType::Font() };
	UCargoComponent* Cargo = Ship ? Ship->CargoComponent.Get() : nullptr;
	if (!Cargo || Columns <= 0 || Rows <= 0)
	{
		return;
	}
	const int32 Cells = Columns * Rows;
	const float CellVol = FMath::Max(Cargo->CargoCapacity, 0.01f) / Cells;
	const float Gap = 4.0f;
	const float Size = (W - Gap * (Columns - 1)) / Columns;

	// CellOwner of each cell: stack index, -2 planned, -1 free.
	TArray<int32> CellOwner;
	CellOwner.Init(-1, Cells);
	int32 Next = 0;
	int32 Stack = 0;
	for (const FCargoEntry& C : Cargo->CargoInventory)
	{
		if (!C.Item || C.Quantity <= 0)
		{
			continue;
		}
		const int32 N = FMath::Max(1, FMath::RoundToInt(C.Item->GetTotalVolume(C.Quantity) / CellVol));
		for (int32 k = 0; k < N && Next < Cells; ++k) { CellOwner[Next++] = Stack; }
		++Stack;
	}
	if (PlannedItem && PlannedUnits > 0)
	{
		const int32 N = FMath::Max(1, FMath::RoundToInt(PlannedItem->GetTotalVolume(PlannedUnits) / CellVol));
		for (int32 k = 0; k < N && Next < Cells; ++k) { CellOwner[Next++] = -2; }
	}
	for (int32 c = 0; c < Cells; ++c)
	{
		const float CX = X + (c % Columns) * (Size + Gap);
		const float CY = Y + (c / Columns) * (Size + Gap);
		if (CellOwner[c] >= 0)
		{
			P.Rect(kTStack[CellOwner[c] % UE_ARRAY_COUNT(kTStack)], CX, CY, Size, Size);
		}
		else if (CellOwner[c] == -2)
		{
			P.Rect(FLinearColor(kTBuy.R, kTBuy.G, kTBuy.B, 0.14f), CX, CY, Size, Size);
			P.Frame(kTBuySoft, CX, CY, Size, Size, 2.0f);
		}
		else
		{
			P.Rect(Hex(0x111B28), CX, CY, Size, Size);
			P.Frame(Hex(0x1C2A3B), CX, CY, Size, Size);
		}
	}
}

void AAdastreaHUD::DrawTradeOpportunitiesView(ASpaceship* Ship, ASpaceStation* Station, UMarketDataAsset* Market, float VW, float VH)
{
	const FTradePen P{ this, HudType::Font() };
	UCargoComponent* Cargo = Ship->CargoComponent;
	constexpr float M = 40.0f;
	constexpr float Top = 116.0f;
	const float Bottom = VH - 56.0f;
	float Y = Top + 28.0f;

	P.Text(TEXT("Best runs from here"), kTBright, M, Y, 1.3f);
	P.Text(TEXT("Buy here, sell at another market in this sector  ·  as many units as your credits and hold allow"),
		kTMuted, M + P.Width(TEXT("Best runs from here"), 1.3f) + 20.0f, Y + 6.0f, HudType::Label);
	Y += 44.0f;

	const float CardGap = 22.0f;
	const float CardW = (VW - 2.0f * M - 2.0f * CardGap) / 3.0f;
	const float CardH = 236.0f;
	if (TradeRuns.Num() == 0)
	{
		P.Rect(kTCard, M, Y, VW - 2.0f * M, CardH);
		P.Frame(kTLine, M, Y, VW - 2.0f * M, CardH);
		P.Centre(Cargo->GetAvailableCargoSpace() < 0.05f ? TEXT("Your hold is full. Sell something first to plan a run.")
			: TEXT("Nothing sold here fetches more at the other markets in this sector right now."),
			kTMuted, VW * 0.5f, Y + CardH * 0.5f - 10.0f, HudType::Body);
	}
	for (int32 r = 0; r < TradeRuns.Num(); ++r)
	{
		const FTradeRun& Run = TradeRuns[r];
		if (!Market->Inventory.IsValidIndex(Run.InventoryIndex) || !TradeMarkets.IsValidIndex(Run.MarketRef))
		{
			continue;
		}
		UTradeItemDataAsset* Item = Market->Inventory[Run.InventoryIndex].TradeItem;
		const FTradeMarketRef& Dest = TradeMarkets[Run.MarketRef];
		const bool bSel = r == TradeRunIndex;
		const float CX = M + r * (CardW + CardGap);
		const int32 Margin = Run.SellThere - Run.BuyHere;
		P.Rect(bSel ? Hex(0x0F1C2B) : kTCard, CX, Y, CardW, CardH);
		P.Frame(bSel ? kTAccent : kTLine, CX, Y, CardW, CardH, bSel ? 2.0f : 1.0f);
		const float IX = CX + 24.0f, IR = CX + CardW - 24.0f;
		P.Chip(Item, IX, Y + 22.0f, 40.0f, HudType::Label);
		P.Text(Item ? Item->ItemName.ToString() : FString(), kTBright, IX + 54.0f, Y + 26.0f, 1.25f);
		P.Right(KmText(Dest.DistanceKm), kTMuted, IR, Y + 32.0f, HudType::Label);

		P.Text(TEXT("BUY HERE"), kTDim, IX, Y + 82.0f, HudType::Caption);
		P.Text(Cr(Run.BuyHere), kTBuySoft, IX, Y + 100.0f, 1.25f);
		const float AX = IX + 150.0f;
		P.Rect(Hex(0x3A4B60), AX, Y + 112.0f, 60.0f, 2.0f);
		P.Text(TEXT(">"), Hex(0x5E7088), AX + 60.0f, Y + 102.0f, HudType::Body);
		FString DestName = Dest.Name;
		while (DestName.Len() > 4 && P.Width(DestName, HudType::Caption) > IR - (AX + 96.0f)) { DestName.LeftChopInline(1); }
		P.Text(FString::Printf(TEXT("SELL AT %s"), *DestName.ToUpper()), kTDim, AX + 96.0f - 60.0f + 60.0f, Y + 82.0f, HudType::Caption);
		P.Text(Cr(Run.SellThere), kTGold, AX + 96.0f, Y + 100.0f, 1.25f);

		P.Rect(Hex(0x1C2738), IX, Y + 150.0f, IR - IX, 1.0f);
		P.Text(FString::Printf(TEXT("+%d per unit  ·  %s units"), Margin, *Num(Run.Units)), kTMuted, IX, Y + 166.0f, HudType::Label);
		P.Text(FString::Printf(TEXT("costs %s"), *Cr(Run.BuyHere * Run.Units)), kTMuted, IX, Y + 190.0f, HudType::Label);
		P.Right(TEXT("EST. PROFIT"), kTDim, IR, Y + 162.0f, HudType::Caption);
		P.Right(FString::Printf(TEXT("+%s"), *Num(Margin * Run.Units)), kTGood, IR, Y + 180.0f, 1.6f);
	}
	Y += CardH + 28.0f;

	// Lower row: the highlighted run's item across markets, and the hold.
	const FTradeRun* Sel = TradeRuns.IsValidIndex(TradeRunIndex) ? &TradeRuns[TradeRunIndex] : nullptr;
	UTradeItemDataAsset* Item = (Sel && Market->Inventory.IsValidIndex(Sel->InventoryIndex)) ? Market->Inventory[Sel->InventoryIndex].TradeItem
		: (Market->Inventory.IsValidIndex(SelectedTradeIndex) ? Market->Inventory[SelectedTradeIndex].TradeItem : nullptr);
	const float GridW = 640.0f;
	const float LW = VW - 2.0f * M - GridW - 24.0f;
	const float LH = Bottom - 28.0f - Y;
	P.Rect(kTCard, M, Y, LW, LH);
	P.Frame(kTLine, M, Y, LW, LH);
	if (Item)
	{
		P.Text(FString::Printf(TEXT("%s across known markets"), *Item->ItemName.ToString()), kTBright, M + 24.0f, Y + 20.0f, HudType::Heading);
		P.Rect(kTGold, M + LW - 250.0f, Y + 26.0f, 10.0f, 10.0f);
		P.Text(TEXT("pays you"), kTMuted, M + LW - 234.0f, Y + 21.0f, HudType::Label);
		P.Rect(kTBuy, M + LW - 140.0f, Y + 26.0f, 10.0f, 10.0f);
		P.Text(TEXT("charges you"), kTMuted, M + LW - 124.0f, Y + 21.0f, HudType::Label);
		const int32 Rows = FMath::Max(1, FMath::FloorToInt((LH - 70.0f) / 52.0f));
		DrawTradeAcrossMarkets(Ship, Item, M + 24.0f, Y + 62.0f, LW - 48.0f, 52.0f, Rows);
	}

	const float GX = VW - M - GridW;
	P.Rect(kTCard, GX, Y, GridW, LH);
	P.Frame(kTLine, GX, Y, GridW, LH);
	P.Text(TEXT("Your hold"), kTBright, GX + 24.0f, Y + 20.0f, HudType::Heading);
	P.Right(FString::Printf(TEXT("1 cell = %.2f"), FMath::Max(Cargo->CargoCapacity, 0.01f) / 100.0f), kTDim, GX + GridW - 24.0f, Y + 24.0f, HudType::Caption);
	const int32 Planned = Sel ? Sel->Units : 0;
	DrawTradeHoldGrid(Ship, Sel ? Item : nullptr, Planned, GX + 24.0f, Y + 60.0f, GridW - 48.0f, 20, 5);
	float LY = Y + 60.0f + 5.0f * ((GridW - 48.0f - 4.0f * 19.0f) / 20.0f + 4.0f) + 16.0f;
	{
		float LX = GX + 24.0f;
		int32 StackIdx = 0;
		for (const FCargoEntry& C : Cargo->CargoInventory)
		{
			if (!C.Item || C.Quantity <= 0 || StackIdx >= 4)
			{
				StackIdx += (C.Item && C.Quantity > 0) ? 1 : 0;
				continue;
			}
			const FString L = FString::Printf(TEXT("%s %d"), *C.Item->ItemName.ToString(), C.Quantity);
			P.Rect(kTStack[StackIdx % UE_ARRAY_COUNT(kTStack)], LX, LY + 5.0f, 10.0f, 10.0f);
			P.Text(L, kTMuted, LX + 16.0f, LY, HudType::Label);
			LX += P.Width(L, HudType::Label) + 34.0f;
			++StackIdx;
		}
		if (Planned > 0 && Item)
		{
			P.Frame(kTBuySoft, LX, LY + 3.0f, 12.0f, 12.0f, 2.0f);
			P.Text(FString::Printf(TEXT("Planned +%d %s"), Planned, *Item->ItemName.ToString()), kTBuySoft, LX + 18.0f, LY, HudType::Label);
		}
	}
	if (Sel && Item)
	{
		const float BY = Y + LH - 24.0f - 60.0f;
		P.Rect(Hex(0x2F7FDB), GX + 24.0f, BY, GridW - 48.0f, 60.0f);
		const FString Label = FString::Printf(TEXT("PLAN: BUY %s %s"), *Num(Sel->Units), *Item->ItemName.ToString().ToUpper());
		P.Mid(Label, FLinearColor::White, GX + 44.0f, BY, 60.0f, HudType::Heading);
		const float KW = P.Width(TEXT("ENTER"), HudType::Caption) + 16.0f;
		P.Rect(Hex(0x2266B5), GX + GridW - 44.0f - KW, BY + 18.0f, KW, 24.0f);
		P.Mid(TEXT("ENTER"), FLinearColor::White, GX + GridW - 36.0f - KW, BY + 18.0f, 24.0f, HudType::Caption);
	}
}
