// Copyright Epic Games, Inc. All Rights Reserved.

// Docked outfitting screen for AAdastreaHUD (canvas-drawn, same path as the trade screen).
//
// Layout:
//   top           title, station, credits, ship
//   tab row       Engines / Weapons / Shields / Hull / Cargo: slots used, and the tier the
//                 station's best module for that category can fit (or "no workshop")
//   left          upgrades for the selected category: what's for sale here plus anything
//                 already fitted (fitted-only rows can be sold if a module services them)
//   right         the selected upgrade: description, stat change if fitted/removed, and
//                 whether it can be bought or sold (with the reason when not); ship stats
//
// Only categories the docked station has an outfitting module for can be changed
// (AOutfittingModule / ASpaceStation::GetOutfittingModuleFor).

#include "AdastreaHUD.h"
#include "AdastreaHUDStyle.h"
#include "Ships/Spaceship.h"
#include "Ships/ShipUpgradeComponent.h"
#include "Ships/ShipUpgradeDataAsset.h"
#include "Stations/SpaceStation.h"
#include "Stations/OutfittingModule.h"
#include "Trading/PlayerTraderComponent.h"
#include "Player/AdastreaPlayerController.h"
#include "Audio/AudioEventLibrary.h"
#include "Engine/Canvas.h"

namespace
{
	const FLinearColor kOutfitAccent(0.15f, 0.9f, 0.6f, 1.0f);
	const FLinearColor kOutfitText(0.9f, 0.95f, 1.0f, 1.0f);
	const FLinearColor kOutfitLabel(0.6f, 0.7f, 0.8f, 1.0f);
	const FLinearColor kOutfitDim(0.42f, 0.47f, 0.52f, 1.0f);
	const FLinearColor kOutfitGold(0.95f, 0.78f, 0.30f, 1.0f);
	const FLinearColor kOutfitWarn(0.95f, 0.5f, 0.35f, 1.0f);
	const FLinearColor kOutfitGood(0.35f, 0.95f, 0.45f, 1.0f);
	const FLinearColor kOutfitPanel(0.05f, 0.08f, 0.11f, 0.9f);

	const EShipUpgradeCategory kOutfitCategories[] = {
		EShipUpgradeCategory::Engines,
		EShipUpgradeCategory::Weapons,
		EShipUpgradeCategory::Shields,
		EShipUpgradeCategory::Hull,
		EShipUpgradeCategory::Cargo,
	};
	constexpr int32 kOutfitCategoryCount = UE_ARRAY_COUNT(kOutfitCategories);

	/** How each upgradeable stat is labelled and printed. */
	struct FOutfitStatInfo
	{
		FName Stat;
		const TCHAR* Label;
		float DisplayScale;   // cm/s -> m/s for speeds
		int32 Decimals;
		const TCHAR* Prefix;
		const TCHAR* Suffix;
	};
	const FOutfitStatInfo kOutfitStats[] = {
		{ ShipUpgradeStats::MaxSpeed,        TEXT("Top speed"),     0.01f, 0, TEXT(""),  TEXT(" m/s") },
		{ ShipUpgradeStats::Acceleration,    TEXT("Thrust"),        0.01f, 0, TEXT(""),  TEXT(" m/s2") },
		{ ShipUpgradeStats::BoostMultiplier, TEXT("Boost"),         1.0f,  2, TEXT("x"), TEXT("") },
		{ ShipUpgradeStats::HullStrength,    TEXT("Hull"),          1.0f,  0, TEXT(""),  TEXT("") },
		{ ShipUpgradeStats::ShieldStrength,  TEXT("Shields"),       1.0f,  0, TEXT(""),  TEXT("") },
		{ ShipUpgradeStats::CargoCapacity,   TEXT("Cargo hold"),    1.0f,  0, TEXT(""),  TEXT(" units") },
		{ ShipUpgradeStats::WeaponDamage,    TEXT("Weapon damage"), 1.0f,  2, TEXT("x"), TEXT("") },
		{ ShipUpgradeStats::MiningPower,     TEXT("Mining power"),  1.0f,  1, TEXT(""),  TEXT("") },
	};

	const FOutfitStatInfo* FindOutfitStat(FName Stat)
	{
		for (const FOutfitStatInfo& Info : kOutfitStats)
		{
			if (Info.Stat == Stat)
			{
				return &Info;
			}
		}
		return nullptr;
	}

	FString FormatOutfitStat(const FOutfitStatInfo& Info, float Value)
	{
		FNumberFormattingOptions Options;
		Options.SetUseGrouping(false).SetMinimumFractionalDigits(Info.Decimals).SetMaximumFractionalDigits(Info.Decimals);
		return FString(Info.Prefix) + FText::AsNumber(Value * Info.DisplayScale, &Options).ToString() + Info.Suffix;
	}

	FString FormatCredits(int32 Credits)
	{
		return FText::AsNumber(Credits).ToString() + TEXT(" cr");
	}

	/** One line of the upgrade list. */
	struct FOutfitRow
	{
		UShipUpgradeDataAsset* Upgrade = nullptr;
		int32 Fitted = 0;
		bool bForSale = false;
	};

	/** The station whose modules do the outfitting: where the ship is docked. */
	ASpaceStation* GetOutfittingStation(APlayerController* PC, ASpaceship* Ship)
	{
		if (Ship)
		{
			if (ASpaceStation* Docked = Ship->GetDockedStation())
			{
				return Docked;
			}
		}
		const AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC);
		return AdPC ? AdPC->GetNearestTradableStation() : nullptr;
	}

	/** For-sale upgrades of Category in stock order, then fitted ones the station doesn't sell. */
	TArray<FOutfitRow> BuildOutfitRows(const ASpaceship* Ship, const ASpaceStation* Station, EShipUpgradeCategory Category)
	{
		TArray<FOutfitRow> Rows;
		const UShipUpgradeComponent* Upgrades = Ship ? Ship->UpgradeComponent.Get() : nullptr;
		if (const AOutfittingModule* Module = Station ? Station->GetOutfittingModuleFor(Category) : nullptr)
		{
			for (UShipUpgradeDataAsset* Upgrade : Module->GetStock(Category))
			{
				FOutfitRow& Row = Rows.AddDefaulted_GetRef();
				Row.Upgrade = Upgrade;
				Row.bForSale = true;
				Row.Fitted = Upgrades ? Upgrades->GetUpgradeStackCount(Upgrade->UpgradeID) : 0;
			}
		}
		if (Upgrades)
		{
			for (const FInstalledUpgrade& Installed : Upgrades->GetUpgradesByCategory(Category))
			{
				if (!Rows.ContainsByPredicate([&](const FOutfitRow& Row) { return Row.Upgrade == Installed.Upgrade; }))
				{
					FOutfitRow& Row = Rows.AddDefaulted_GetRef();
					Row.Upgrade = Installed.Upgrade;
					Row.Fitted = Installed.StackCount;
				}
			}
		}
		return Rows;
	}

	EShipUpgradeCategory OutfitCategoryAt(int32 Index)
	{
		return kOutfitCategories[FMath::Clamp(Index, 0, kOutfitCategoryCount - 1)];
	}
}

void AAdastreaHUD::ShowOutfitting()
{
	bShowStationMenu = false;
	bShowTradeScreen = false;
	bShowOutfitting = true;
	OutfittingRowIndex = 0;

	// Open on the first category this station can actually work on.
	APlayerController* PC = GetOwningPlayerController();
	ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	if (const ASpaceStation* Station = GetOutfittingStation(PC, Ship))
	{
		for (int32 i = 0; i < kOutfitCategoryCount; ++i)
		{
			if (Station->GetOutfittingModuleFor(kOutfitCategories[i]))
			{
				OutfittingCategoryIndex = i;
				break;
			}
		}
	}
}

bool AAdastreaHUD::IsOutfittingAvailable(APlayerController* PC) const
{
	ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	const ASpaceStation* Station = GetOutfittingStation(PC, Ship);
	return Station && Station->HasOutfitting();
}

void AAdastreaHUD::MoveOutfittingSelection(int32 Step)
{
	APlayerController* PC = GetOwningPlayerController();
	ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	const int32 Count = BuildOutfitRows(Ship, GetOutfittingStation(PC, Ship), OutfitCategoryAt(OutfittingCategoryIndex)).Num();
	OutfittingRowIndex = FMath::Clamp(OutfittingRowIndex + Step, 0, FMath::Max(Count - 1, 0));
}

void AAdastreaHUD::MoveOutfittingCategory(int32 Step)
{
	OutfittingCategoryIndex = (OutfittingCategoryIndex + Step + kOutfitCategoryCount) % kOutfitCategoryCount;
	OutfittingRowIndex = 0;
}

void AAdastreaHUD::ConfirmOutfittingPurchase(APlayerController* PC)
{
	ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	ASpaceStation* Station = GetOutfittingStation(PC, Ship);
	if (!Ship || !Ship->UpgradeComponent)
	{
		return;
	}

	const EShipUpgradeCategory Category = OutfitCategoryAt(OutfittingCategoryIndex);
	const TArray<FOutfitRow> Rows = BuildOutfitRows(Ship, Station, Category);
	if (!Rows.IsValidIndex(OutfittingRowIndex))
	{
		return;
	}
	const FOutfitRow& Row = Rows[OutfittingRowIndex];

	FText Reason;
	AOutfittingModule* Module = Station ? Station->GetOutfittingModuleFor(Category) : nullptr;
	if (!Row.bForSale)
	{
		Reason = FText::FromString(TEXT("Not sold at this station"));
	}
	else if (Ship->UpgradeComponent->PurchaseUpgrade(Row.Upgrade, Module, Ship->PlayerTraderComponent, Reason))
	{
		UAudioEventLibrary::PlayEvent2D(this, TEXT("Trade.Buy"), 0.12f);
		ShowMessage(FString::Printf(TEXT("Fitted %s  (-%s)"), *Row.Upgrade->DisplayName.ToString(),
			*FormatCredits(Module->GetPrice(Row.Upgrade))), 3.0f, false);
		return;
	}

	UAudioEventLibrary::PlayEvent2D(this, TEXT("Trade.Denied"), 0.12f);
	ShowMessage(Reason.ToString(), 3.0f, true);
}

void AAdastreaHUD::SellOutfittingSelection(APlayerController* PC)
{
	ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	ASpaceStation* Station = GetOutfittingStation(PC, Ship);
	if (!Ship || !Ship->UpgradeComponent)
	{
		return;
	}

	const EShipUpgradeCategory Category = OutfitCategoryAt(OutfittingCategoryIndex);
	const TArray<FOutfitRow> Rows = BuildOutfitRows(Ship, Station, Category);
	if (!Rows.IsValidIndex(OutfittingRowIndex))
	{
		return;
	}
	const FOutfitRow& Row = Rows[OutfittingRowIndex];

	FText Reason;
	int32 Refund = 0;
	if (Row.Fitted <= 0)
	{
		Reason = FText::FromString(TEXT("Not fitted to this ship"));
	}
	else if (Ship->UpgradeComponent->SellUpgrade(Row.Upgrade->UpgradeID, Station ? Station->GetOutfittingModuleFor(Category) : nullptr,
		Ship->PlayerTraderComponent, Refund, Reason))
	{
		UAudioEventLibrary::PlayEvent2D(this, TEXT("Trade.Sell"), 0.12f);
		ShowMessage(FString::Printf(TEXT("Removed %s  (+%s)"), *Row.Upgrade->DisplayName.ToString(), *FormatCredits(Refund)), 3.0f, false);
		// A fitted-only row disappears once its last stack is sold.
		MoveOutfittingSelection(0);
		return;
	}

	UAudioEventLibrary::PlayEvent2D(this, TEXT("Trade.Denied"), 0.12f);
	ShowMessage(Reason.ToString(), 3.0f, true);
}

void AAdastreaHUD::DrawOutfittingScreen(APlayerController* PC, ASpaceship* Ship)
{
	if (!PC || !Ship || !Ship->UpgradeComponent)
	{
		return;
	}
	UShipUpgradeComponent* Upgrades = Ship->UpgradeComponent;
	ASpaceStation* Station = GetOutfittingStation(PC, Ship);

	int32 VX = 0, VY = 0;
	PC->GetViewportSize(VX, VY);
	const float VW = (float)VX, VH = (float)VY;

	DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.94f), 0.0f, 0.0f, VW, VH);

	// ---- Header ----
	DrawCentredText(TEXT("OUTFITTING"), kOutfitAccent, VW * 0.5f, 18.0f, HudType::Title);
	DrawCentredText(Station ? Station->GetTargetDisplayName_Implementation().ToString() : FString(TEXT("Docked")),
		FLinearColor(0.8f, 0.9f, 1.0f, 1.0f), VW * 0.5f, 52.0f, HudType::Body);
	const int32 Credits = Ship->PlayerTraderComponent ? Ship->PlayerTraderComponent->GetCredits() : 0;
	DrawText(FString::Printf(TEXT("CREDITS: %s"), *FormatCredits(Credits)), kOutfitGold, 40.0f, 40.0f, HudType::Font(), HudType::Body);
	DrawText(FString::Printf(TEXT("SHIP: %s"), *Ship->GetShipName().ToString()), kOutfitLabel, 40.0f, 66.0f, HudType::Font(), HudType::Body);

	// ---- Category tabs ----
	const float TabY = 104.0f;
	const float TabH = 54.0f;
	const float TabGap = 8.0f;
	const float TabW = FMath::Min(230.0f, (VW - 80.0f - TabGap * (kOutfitCategoryCount - 1)) / kOutfitCategoryCount);
	const float TabsX = (VW - (TabW * kOutfitCategoryCount + TabGap * (kOutfitCategoryCount - 1))) * 0.5f;
	for (int32 i = 0; i < kOutfitCategoryCount; ++i)
	{
		const EShipUpgradeCategory Cat = kOutfitCategories[i];
		const AOutfittingModule* CatModule = Station ? Station->GetOutfittingModuleFor(Cat) : nullptr;
		const bool bSel = (i == OutfittingCategoryIndex);
		const float X = TabsX + i * (TabW + TabGap);

		DrawRect(bSel ? FLinearColor(0.15f, 0.9f, 0.6f, 0.18f) : kOutfitPanel, X, TabY, TabW, TabH);
		if (bSel)
		{
			DrawLine(X, TabY + TabH, X + TabW, TabY + TabH, kOutfitAccent, 3.0f);
		}
		const FLinearColor NameCol = bSel ? kOutfitAccent : (CatModule ? kOutfitText : kOutfitDim);
		DrawText(UShipUpgradeDataAsset::GetCategoryName(Cat).ToString().ToUpper(), NameCol, X + 12.0f, TabY + 6.0f, HudType::Font(), HudType::Heading);
		const FString Sub = FString::Printf(TEXT("%d/%d slots   %s"),
			Upgrades->GetUsedSlotCount(Cat), Upgrades->GetSlotCount(Cat),
			CatModule ? *FString::Printf(TEXT("up to T%d"), static_cast<int32>(CatModule->GetMaxTier()) + 1) : TEXT("no workshop"));
		DrawText(Sub, CatModule ? kOutfitLabel : kOutfitDim, X + 12.0f, TabY + 31.0f, HudType::Font(), HudType::Caption);
	}

	const EShipUpgradeCategory Category = OutfitCategoryAt(OutfittingCategoryIndex);
	AOutfittingModule* Module = Station ? Station->GetOutfittingModuleFor(Category) : nullptr;
	const TArray<FOutfitRow> Rows = BuildOutfitRows(Ship, Station, Category);
	OutfittingRowIndex = FMath::Clamp(OutfittingRowIndex, 0, FMath::Max(Rows.Num() - 1, 0));

	// ---- Left: upgrade list ----
	const float TopY = TabY + TabH + 24.0f;
	const float ListX = 40.0f;
	const float ListW = VW * 0.54f - ListX;
	const FString CatName = UShipUpgradeDataAsset::GetCategoryName(Category).ToString();
	if (Module)
	{
		DrawText(FString::Printf(TEXT("%s  -  fits %s upgrades up to tier %d"), *Module->ModuleType, *CatName.ToLower(),
			static_cast<int32>(Module->GetMaxTier()) + 1), kOutfitAccent, ListX, TopY, HudType::Font(), HudType::Body);
	}
	else
	{
		DrawText(FString::Printf(TEXT("No %s workshop at this station: fitted %s upgrades can't be changed here"),
			*CatName.ToLower(), *CatName.ToLower()), kOutfitWarn, ListX, TopY, HudType::Font(), HudType::Body);
	}

	const float HeadY = TopY + 34.0f;
	const float TierX = ListX + ListW * 0.58f;
	const float PriceX = ListX + ListW * 0.68f;
	const float FittedX = ListX + ListW * 0.87f;
	DrawText(TEXT("UPGRADE"), kOutfitLabel, ListX, HeadY, HudType::Font(), HudType::Label);
	DrawText(TEXT("TIER"), kOutfitLabel, TierX, HeadY, HudType::Font(), HudType::Label);
	DrawText(TEXT("PRICE"), kOutfitLabel, PriceX, HeadY, HudType::Font(), HudType::Label);
	DrawText(TEXT("FITTED"), kOutfitLabel, FittedX, HeadY, HudType::Font(), HudType::Label);
	DrawLine(ListX, HeadY + 22.0f, ListX + ListW, HeadY + 22.0f, FLinearColor(0.10f, 0.65f, 0.72f, 0.6f), 1.0f);

	const float RowH = 32.0f;
	float RowY = HeadY + 30.0f;
	if (Rows.Num() == 0)
	{
		DrawText(Module ? TEXT("Nothing in stock.") : TEXT("Nothing fitted."), kOutfitDim, ListX, RowY + 4.0f, HudType::Font(), HudType::Body);
	}
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		const FOutfitRow& Row = Rows[i];
		if (i == OutfittingRowIndex)
		{
			DrawRect(FLinearColor(0.15f, 0.32f, 0.35f, 0.5f), ListX - 10.0f, RowY - 2.0f, ListW + 20.0f, RowH - 4.0f);
			DrawLine(ListX - 10.0f, RowY - 2.0f, ListX - 10.0f, RowY + RowH - 6.0f, kOutfitAccent, 3.0f);
		}
		const bool bMaxed = Row.Fitted >= Row.Upgrade->MaxStackCount;
		const FLinearColor NameCol = Row.bForSale ? kOutfitText : kOutfitDim;
		DrawText(Row.Upgrade->DisplayName.ToString(), NameCol, ListX, RowY + 2.0f, HudType::Font(), HudType::Body);
		DrawText(FString::Printf(TEXT("T%d"), Row.Upgrade->GetTierNumber()), kOutfitLabel, TierX, RowY + 2.0f, HudType::Font(), HudType::Body);
		DrawText(Row.bForSale ? FormatCredits(Module->GetPrice(Row.Upgrade)) : FString(TEXT("-")),
			Row.bForSale ? kOutfitGold : kOutfitDim, PriceX, RowY + 2.0f, HudType::Font(), HudType::Body);
		if (Row.Fitted > 0)
		{
			const FString Fitted = Row.Upgrade->MaxStackCount > 1
				? FString::Printf(TEXT("%d/%d"), Row.Fitted, Row.Upgrade->MaxStackCount)
				: FString(TEXT("YES"));
			DrawText(Fitted, bMaxed ? kOutfitGood : kOutfitAccent, FittedX, RowY + 2.0f, HudType::Font(), HudType::Body);
		}
		RowY += RowH;
	}

	// ---- Right: selected upgrade + ship stats ----
	const float PX = VW * 0.57f;
	const float PW = VW - 40.0f - PX;
	const float PY = TopY;
	DrawRect(kOutfitPanel, PX - 16.0f, PY - 10.0f, PW + 32.0f, VH - 70.0f - (PY - 10.0f));

	float Y = PY;
	if (Rows.IsValidIndex(OutfittingRowIndex))
	{
		const FOutfitRow& Row = Rows[OutfittingRowIndex];
		const UShipUpgradeDataAsset* Upgrade = Row.Upgrade;
		DrawText(Upgrade->DisplayName.ToString(), kOutfitText, PX, Y, HudType::Font(), HudType::Heading);
		Y += 28.0f;
		DrawText(FString::Printf(TEXT("Tier %d %s upgrade%s"), Upgrade->GetTierNumber(), *CatName.ToLower(),
			Upgrade->MaxStackCount > 1 ? *FString::Printf(TEXT("  -  can fit %d"), Upgrade->MaxStackCount) : TEXT("")),
			kOutfitLabel, PX, Y, HudType::Font(), HudType::Label);
		Y += 26.0f;
		if (!Upgrade->Description.IsEmpty())
		{
			Y = DrawWrappedText(Upgrade->Description.ToString(), kOutfitText, PX, Y, PW, HudType::Font(), HudType::Body) + 10.0f;
		}

		// Stat change: what one more would do, or (when it can't be fitted again) what removing one would do.
		const bool bPreviewAdd = Row.Fitted < Upgrade->MaxStackCount;
		DrawText(bPreviewAdd ? TEXT("IF FITTED") : TEXT("IF REMOVED"), kOutfitLabel, PX, Y, HudType::Font(), HudType::Label);
		Y += 22.0f;
		TArray<FName> ShownStats;
		for (const FShipUpgradeStatModifier& Modifier : Upgrade->StatModifiers)
		{
			if (ShownStats.Contains(Modifier.StatName))
			{
				continue;
			}
			ShownStats.Add(Modifier.StatName);
			const FOutfitStatInfo* Info = FindOutfitStat(Modifier.StatName);
			if (!Info)
			{
				continue;
			}
			const float Base = Ship->GetBaseStat(Modifier.StatName);
			const float Now = Upgrades->GetStatModifier(Modifier.StatName, Base);
			const float After = Upgrades->PreviewStat(Modifier.StatName, Base,
				bPreviewAdd ? Upgrade : nullptr, bPreviewAdd ? NAME_None : Upgrade->UpgradeID);
			const FLinearColor DeltaCol = After > Now + KINDA_SMALL_NUMBER ? kOutfitGood
				: (After < Now - KINDA_SMALL_NUMBER ? kOutfitWarn : kOutfitLabel);
			DrawText(Info->Label, kOutfitLabel, PX, Y, HudType::Font(), HudType::Body);
			DrawText(FString::Printf(TEXT("%s  ->  %s"), *FormatOutfitStat(*Info, Now), *FormatOutfitStat(*Info, After)),
				DeltaCol, PX + PW * 0.38f, Y, HudType::Font(), HudType::Body);
			Y += 24.0f;
		}
		Y += 10.0f;

		// Buy / sell status.
		FText Reason;
		if (Row.bForSale && bPreviewAdd)
		{
			if (Upgrades->CanPurchaseUpgrade(Row.Upgrade, Module, Ship->PlayerTraderComponent, Reason))
			{
				DrawText(FString::Printf(TEXT("[ENTER] Buy & fit  -%s"), *FormatCredits(Module->GetPrice(Upgrade))), kOutfitGood, PX, Y, HudType::Font(), HudType::Body);
			}
			else
			{
				DrawText(FString::Printf(TEXT("Can't fit: %s"), *Reason.ToString()), kOutfitWarn, PX, Y, HudType::Font(), HudType::Body);
			}
			Y += 26.0f;
		}
		if (Row.Fitted > 0)
		{
			if (Upgrades->CanSellUpgrade(Upgrade->UpgradeID, Module, Reason))
			{
				DrawText(FString::Printf(TEXT("[X] Remove & sell  +%s"), *FormatCredits(Upgrades->GetSellPrice(Upgrade))), kOutfitAccent, PX, Y, HudType::Font(), HudType::Body);
			}
			else
			{
				DrawText(FString::Printf(TEXT("Can't remove: %s"), *Reason.ToString()), kOutfitWarn, PX, Y, HudType::Font(), HudType::Body);
			}
			Y += 26.0f;
		}
	}

	// Ship stats, upgrades included; the change from stock in brackets.
	Y = FMath::Max(Y + 16.0f, VH - 70.0f - 26.0f * UE_ARRAY_COUNT(kOutfitStats) - 44.0f);
	DrawLine(PX, Y, PX + PW, Y, FLinearColor(0.10f, 0.65f, 0.72f, 0.6f), 1.0f);
	Y += 10.0f;
	DrawText(TEXT("SHIP"), kOutfitLabel, PX, Y, HudType::Font(), HudType::Label);
	Y += 24.0f;
	for (const FOutfitStatInfo& Info : kOutfitStats)
	{
		const float Base = Ship->GetBaseStat(Info.Stat);
		const float Now = Ship->GetUpgradedStat(Info.Stat);
		DrawText(Info.Label, kOutfitLabel, PX, Y, HudType::Font(), HudType::Label);
		DrawText(FormatOutfitStat(Info, Now), kOutfitText, PX + PW * 0.38f, Y, HudType::Font(), HudType::Label);
		if (Base > KINDA_SMALL_NUMBER && !FMath::IsNearlyEqual(Now, Base, 0.001f * Base))
		{
			const float Pct = (Now / Base - 1.0f) * 100.0f;
			DrawText(FString::Printf(TEXT("(%+.0f%%)"), Pct), Pct > 0.0f ? kOutfitGood : kOutfitWarn,
				PX + PW * 0.72f, Y, HudType::Font(), HudType::Label);
		}
		else if (Base <= KINDA_SMALL_NUMBER && Now > KINDA_SMALL_NUMBER)
		{
			DrawText(TEXT("(new)"), kOutfitGood, PX + PW * 0.72f, Y, HudType::Font(), HudType::Label);
		}
		Y += 26.0f;
	}

	DrawCentredText(TEXT("[LEFT/RIGHT] category    [UP/DOWN] select    [ENTER] buy & fit    [X] remove & sell    [ESC] back"),
		kOutfitLabel, VW * 0.5f, VH - 40.0f, HudType::Label);
}
