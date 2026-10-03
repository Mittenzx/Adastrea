// Copyright Mittenzx. All Rights Reserved.

// Docked production screen for AAdastreaHUD (canvas-drawn, same path as the outfitting screen).
//
// Layout:
//   top           title, station, credits, cargo hold
//   tab row       one tab per facility the station has modules for (Processing, Fabrication,
//                 Reactor...): module count and jobs queued
//   left          that facility's recipes: tier, output, runs the hold has ingredients for
//   right         the selected recipe: ingredients held / needed, time per run, why it can't be
//                 queued; below it the station's job queue with progress
//
// UCraftingManager does the work; finished goods go into the hold while docked here.

#include "AdastreaHUD.h"
#include "AdastreaHUDStyle.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Trading/CargoComponent.h"
#include "Trading/CraftingManager.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "Audio/AudioEventLibrary.h"
#include "Engine/Canvas.h"

namespace
{
	const FLinearColor kCraftAccent(0.95f, 0.62f, 0.25f, 1.0f);
	const FLinearColor kCraftText(0.9f, 0.95f, 1.0f, 1.0f);
	const FLinearColor kCraftLabel(0.6f, 0.7f, 0.8f, 1.0f);
	const FLinearColor kCraftDim(0.42f, 0.47f, 0.52f, 1.0f);
	const FLinearColor kCraftGold(0.95f, 0.78f, 0.30f, 1.0f);
	const FLinearColor kCraftWarn(0.95f, 0.5f, 0.35f, 1.0f);
	const FLinearColor kCraftGood(0.35f, 0.95f, 0.45f, 1.0f);
	const FLinearColor kCraftPanel(0.05f, 0.08f, 0.11f, 0.9f);

	ASpaceStation* GetCraftingStation(const APlayerController* PC)
	{
		const ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
		return Ship && Ship->IsDocked() ? Ship->GetDockedStation() : nullptr;
	}

	FString ItemName(const UCraftingManager* Crafting, FName ItemID)
	{
		const UTradeItemDataAsset* Item = Crafting ? Crafting->GetLoader()->GetTradeItem(ItemID.ToString()) : nullptr;
		return Item && !Item->ItemName.IsEmpty() ? Item->ItemName.ToString() : FName::NameToDisplayString(ItemID.ToString(), false);
	}

	FString FormatSeconds(float Seconds)
	{
		const int32 S = FMath::CeilToInt(FMath::Max(0.0f, Seconds));
		return S >= 60 ? FString::Printf(TEXT("%dm %02ds"), S / 60, S % 60) : FString::Printf(TEXT("%ds"), S);
	}

	/** The facility tab currently shown (clamped), or empty when the station makes nothing. */
	FString FacilityAt(const TArray<TPair<FString, int32>>& Facilities, int32 Index)
	{
		return Facilities.IsEmpty() ? FString() : Facilities[FMath::Clamp(Index, 0, Facilities.Num() - 1)].Key;
	}
}

void AAdastreaHUD::ShowCrafting()
{
	bShowStationMenu = false;
	bShowTradeScreen = false;
	bShowOutfitting = false;
	bShowCrafting = true;
	bCraftingJobsFocus = false;
	CraftingFacilityIndex = 0;
	CraftingRowIndex = 0;
	CraftingJobIndex = 0;
}

bool AAdastreaHUD::IsCraftingAvailable(APlayerController* PC) const
{
	const UCraftingManager* Crafting = UCraftingManager::Get(this);
	return Crafting && !Crafting->GetStationFacilities(GetCraftingStation(PC)).IsEmpty();
}

void AAdastreaHUD::MoveCraftingSelection(int32 Step)
{
	const UCraftingManager* Crafting = UCraftingManager::Get(this);
	const ASpaceStation* Station = GetCraftingStation(GetOwningPlayerController());
	if (!Crafting)
	{
		return;
	}
	if (bCraftingJobsFocus)
	{
		CraftingJobIndex = FMath::Clamp(CraftingJobIndex + Step, 0, FMath::Max(Crafting->GetJobs(Station).Num() - 1, 0));
		return;
	}
	const FString Facility = FacilityAt(Crafting->GetStationFacilities(Station), CraftingFacilityIndex);
	CraftingRowIndex = FMath::Clamp(CraftingRowIndex + Step, 0, FMath::Max(Crafting->GetRecipesFor(Facility).Num() - 1, 0));
}

void AAdastreaHUD::MoveCraftingFacility(int32 Step)
{
	const UCraftingManager* Crafting = UCraftingManager::Get(this);
	const int32 Count = Crafting ? Crafting->GetStationFacilities(GetCraftingStation(GetOwningPlayerController())).Num() : 0;
	if (Count > 0)
	{
		CraftingFacilityIndex = (FMath::Clamp(CraftingFacilityIndex, 0, Count - 1) + Step + Count) % Count;
		CraftingRowIndex = 0;
		bCraftingJobsFocus = false;
	}
}

void AAdastreaHUD::QueueCraftingSelection(APlayerController* PC, int32 Runs)
{
	UCraftingManager* Crafting = UCraftingManager::Get(this);
	ASpaceStation* Station = GetCraftingStation(PC);
	ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	if (!Crafting || !Station || !Ship || !Ship->CargoComponent || bCraftingJobsFocus)
	{
		return;
	}
	const TArray<FCraftingRecipe> Recipes = Crafting->GetRecipesFor(FacilityAt(Crafting->GetStationFacilities(Station), CraftingFacilityIndex));
	if (!Recipes.IsValidIndex(CraftingRowIndex))
	{
		return;
	}
	const FCraftingRecipe& Recipe = Recipes[CraftingRowIndex];
	const int32 Affordable = UCraftingManager::GetAffordableCrafts(Recipe, Ship->CargoComponent);
	// 0 = as many as the hold allows; asking for more than that queues what there is.
	const int32 Want = Runs <= 0 ? Affordable : FMath::Min(Runs, FMath::Max(Affordable, 1));
	const ECraftingCheck Check = Crafting->CheckQueue(Station, Recipe, FMath::Max(Want, 1), Ship->CargoComponent);
	if (Check == ECraftingCheck::Ok && Crafting->QueueJob(Station, Recipe, Want, Ship->CargoComponent) != 0)
	{
		UAudioEventLibrary::PlayEvent2D(this, TEXT("Trade.Buy"), 0.12f);
		ShowMessage(FString::Printf(TEXT("Queued %d x %s"), Want * FMath::Max(1, Recipe.OutputQuantity), *ItemName(Crafting, Recipe.OutputItem)), 3.0f, false);
		return;
	}
	UAudioEventLibrary::PlayEvent2D(this, TEXT("Trade.Denied"), 0.12f);
	ShowMessage(UCraftingManager::CheckToText(Check).ToString(), 3.0f, true);
}

void AAdastreaHUD::CancelCraftingSelection(APlayerController* PC)
{
	UCraftingManager* Crafting = UCraftingManager::Get(this);
	const ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	const TArray<FCraftingJob> Jobs = Crafting ? Crafting->GetJobs(GetCraftingStation(PC)) : TArray<FCraftingJob>();
	if (!Ship || !Jobs.IsValidIndex(CraftingJobIndex))
	{
		return;
	}
	if (Crafting->CancelJob(Jobs[CraftingJobIndex].JobId, Ship->CargoComponent))
	{
		UAudioEventLibrary::PlayEvent2D(this, TEXT("Trade.Sell"), 0.12f);
		ShowMessage(TEXT("Job cancelled: unused ingredients returned to the hold"), 3.0f, false);
		MoveCraftingSelection(0);
	}
}

void AAdastreaHUD::DrawCraftingScreen(APlayerController* PC, ASpaceship* Ship)
{
	UCraftingManager* Crafting = UCraftingManager::Get(this);
	ASpaceStation* Station = GetCraftingStation(PC);
	UCargoComponent* Cargo = Ship ? Ship->CargoComponent.Get() : nullptr;
	if (!PC || !Crafting || !Cargo)
	{
		return;
	}

	int32 VX = 0, VY = 0;
	PC->GetViewportSize(VX, VY);
	const float VW = (float)VX, VH = (float)VY;
	DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.94f), 0.0f, 0.0f, VW, VH);

	// ---- Header ----
	DrawCentredText(TEXT("PRODUCTION"), kCraftAccent, VW * 0.5f, 18.0f, HudType::Title);
	DrawCentredText(Station ? Station->GetDisplayNameString() : FString(TEXT("Not docked")), FLinearColor(0.8f, 0.9f, 1.0f, 1.0f), VW * 0.5f, 52.0f, HudType::Body);
	const int32 Credits = Ship->PlayerTraderComponent ? Ship->PlayerTraderComponent->GetCredits() : 0;
	DrawText(FString::Printf(TEXT("CREDITS: %s cr"), *FText::AsNumber(Credits).ToString()), kCraftGold, 40.0f, 40.0f, HudType::Font(), HudType::Body);
	DrawText(FString::Printf(TEXT("HOLD: %.0f / %.0f"), Cargo->CargoCapacity - Cargo->GetAvailableCargoSpace(), Cargo->CargoCapacity),
		kCraftLabel, 40.0f, 66.0f, HudType::Font(), HudType::Body);

	// ---- Facility tabs ----
	const TArray<TPair<FString, int32>> Facilities = Crafting->GetStationFacilities(Station);
	CraftingFacilityIndex = FMath::Clamp(CraftingFacilityIndex, 0, FMath::Max(Facilities.Num() - 1, 0));
	const TArray<FCraftingJob> Jobs = Crafting->GetJobs(Station);
	const float TabY = 104.0f, TabH = 54.0f, TabGap = 8.0f;
	const int32 TabCount = FMath::Max(Facilities.Num(), 1);
	const float TabW = FMath::Min(230.0f, (VW - 80.0f - TabGap * (TabCount - 1)) / TabCount);
	const float TabsX = (VW - (TabW * TabCount + TabGap * (TabCount - 1))) * 0.5f;
	for (int32 i = 0; i < Facilities.Num(); ++i)
	{
		const bool bSel = (i == CraftingFacilityIndex);
		const float X = TabsX + i * (TabW + TabGap);
		DrawRect(bSel ? FLinearColor(0.95f, 0.62f, 0.25f, 0.18f) : kCraftPanel, X, TabY, TabW, TabH);
		if (bSel)
		{
			DrawLine(X, TabY + TabH, X + TabW, TabY + TabH, kCraftAccent, 3.0f);
		}
		const int32 Running = Jobs.FilterByPredicate([&](const FCraftingJob& J) { return J.Facility == Facilities[i].Key && J.IsRunning(); }).Num();
		DrawText(FName::NameToDisplayString(Facilities[i].Key, false).ToUpper(), bSel ? kCraftAccent : kCraftText, X + 12.0f, TabY + 6.0f, HudType::Font(), HudType::Heading);
		DrawText(FString::Printf(TEXT("%d module%s   %d/%d jobs"), Facilities[i].Value, Facilities[i].Value == 1 ? TEXT("") : TEXT("s"),
			Running, UCraftingManager::MaxJobsPerFacility), kCraftLabel, X + 12.0f, TabY + 31.0f, HudType::Font(), HudType::Caption);
	}
	if (Facilities.IsEmpty())
	{
		DrawCentredText(TEXT("This station has no processing or fabrication modules."), kCraftWarn, VW * 0.5f, TabY + 14.0f, HudType::Body);
		DrawCentredText(TEXT("[ESC] back"), kCraftLabel, VW * 0.5f, VH - 40.0f, HudType::Label);
		return;
	}

	const FString Facility = FacilityAt(Facilities, CraftingFacilityIndex);
	const TArray<FCraftingRecipe> Recipes = Crafting->GetRecipesFor(Facility);
	CraftingRowIndex = FMath::Clamp(CraftingRowIndex, 0, FMath::Max(Recipes.Num() - 1, 0));

	// ---- Left: recipe list (scrolls to keep the selection in view) ----
	const float TopY = TabY + TabH + 24.0f;
	const float ListX = 40.0f;
	const float ListW = VW * 0.5f - ListX;
	const float TierX = ListX + ListW * 0.66f;
	const float RunsX = ListX + ListW * 0.80f;
	DrawText(TEXT("RECIPE"), kCraftLabel, ListX, TopY, HudType::Font(), HudType::Label);
	DrawText(TEXT("TIER"), kCraftLabel, TierX, TopY, HudType::Font(), HudType::Label);
	DrawText(TEXT("CAN MAKE"), kCraftLabel, RunsX, TopY, HudType::Font(), HudType::Label);
	DrawLine(ListX, TopY + 22.0f, ListX + ListW, TopY + 22.0f, FLinearColor(0.72f, 0.45f, 0.18f, 0.6f), 1.0f);

	const float RowH = 30.0f;
	const float ListTop = TopY + 30.0f;
	const int32 VisibleRows = FMath::Max(4, FMath::FloorToInt((VH - 80.0f - ListTop) / RowH));
	const int32 First = FMath::Clamp(CraftingRowIndex - VisibleRows / 2, 0, FMath::Max(0, Recipes.Num() - VisibleRows));
	for (int32 i = First; i < FMath::Min(Recipes.Num(), First + VisibleRows); ++i)
	{
		const FCraftingRecipe& Recipe = Recipes[i];
		const float RowY = ListTop + (i - First) * RowH;
		if (i == CraftingRowIndex && !bCraftingJobsFocus)
		{
			DrawRect(FLinearColor(0.35f, 0.24f, 0.12f, 0.5f), ListX - 10.0f, RowY - 2.0f, ListW + 20.0f, RowH - 4.0f);
			DrawLine(ListX - 10.0f, RowY - 2.0f, ListX - 10.0f, RowY + RowH - 6.0f, kCraftAccent, 3.0f);
		}
		const bool bLocked = !Crafting->GetResearchRequired(Recipe).IsEmpty();
		const int32 Runs = bLocked ? 0 : UCraftingManager::GetAffordableCrafts(Recipe, Cargo);
		const FString Name = ItemName(Crafting, Recipe.OutputItem) + (Recipe.OutputQuantity > 1 ? FString::Printf(TEXT(" x%d"), Recipe.OutputQuantity) : FString());
		DrawText(Name, bLocked ? kCraftDim : (Runs > 0 ? kCraftText : kCraftLabel), ListX, RowY + 2.0f, HudType::Font(), HudType::Body);
		DrawText(FString::Printf(TEXT("T%d"), Recipe.Tier), kCraftLabel, TierX, RowY + 2.0f, HudType::Font(), HudType::Body);
		DrawText(bLocked ? FString(TEXT("research")) : FString::FromInt(Runs), bLocked ? kCraftDim : (Runs > 0 ? kCraftGood : kCraftDim),
			RunsX, RowY + 2.0f, HudType::Font(), HudType::Body);
	}

	// ---- Right: selected recipe ----
	const float PX = VW * 0.54f;
	const float PW = VW - 40.0f - PX;
	DrawRect(kCraftPanel, PX - 16.0f, TopY - 10.0f, PW + 32.0f, VH - 70.0f - (TopY - 10.0f));
	float Y = TopY;
	if (Recipes.IsValidIndex(CraftingRowIndex))
	{
		const FCraftingRecipe& Recipe = Recipes[CraftingRowIndex];
		const int32 Modules = Facilities[CraftingFacilityIndex].Value;
		DrawText(ItemName(Crafting, Recipe.OutputItem), kCraftText, PX, Y, HudType::Font(), HudType::Heading);
		Y += 28.0f;
		DrawText(FString::Printf(TEXT("Tier %d  -  makes %d per run  -  %s per run with %d module%s"), Recipe.Tier,
			FMath::Max(1, Recipe.OutputQuantity), *FormatSeconds(UCraftingManager::GetBaseSecondsPerCraft(Recipe) / Modules),
			Modules, Modules == 1 ? TEXT("") : TEXT("s")), kCraftLabel, PX, Y, HudType::Font(), HudType::Label);
		Y += 30.0f;

		DrawText(TEXT("INGREDIENTS PER RUN"), kCraftLabel, PX, Y, HudType::Font(), HudType::Label);
		DrawText(TEXT("IN HOLD"), kCraftLabel, PX + PW * 0.72f, Y, HudType::Font(), HudType::Label);
		Y += 24.0f;
		for (const FCraftIngredient& Ing : Recipe.Ingredients)
		{
			const int32 Held = Cargo->GetItemQuantityByID(Ing.ItemID);
			DrawText(FString::Printf(TEXT("%d  %s"), Ing.Quantity, *ItemName(Crafting, Ing.ItemID)), kCraftText, PX, Y, HudType::Font(), HudType::Body);
			DrawText(FString::FromInt(Held), Held >= Ing.Quantity ? kCraftGood : kCraftWarn, PX + PW * 0.72f, Y, HudType::Font(), HudType::Body);
			Y += 24.0f;
		}
		Y += 8.0f;

		const ECraftingCheck Check = Crafting->CheckQueue(Station, Recipe, 1, Cargo);
		if (Check == ECraftingCheck::Ok)
		{
			DrawText(FString::Printf(TEXT("[SPACE/ENTER] queue 1   [Q] queue 5   [X] queue all %d"), UCraftingManager::GetAffordableCrafts(Recipe, Cargo)),
				kCraftGood, PX, Y, HudType::Font(), HudType::Body);
		}
		else
		{
			const FString Research = Crafting->GetResearchRequired(Recipe);
			const FString Why = Check == ECraftingCheck::NeedsResearch
				? FString::Printf(TEXT("Needs research: %s"), *FName::NameToDisplayString(Research, false))
				: UCraftingManager::CheckToText(Check).ToString();
			DrawText(Why, kCraftWarn, PX, Y, HudType::Font(), HudType::Body);
		}
		Y += 40.0f;
	}

	// ---- Right, lower: this station's job queue ----
	DrawLine(PX, Y, PX + PW, Y, FLinearColor(0.72f, 0.45f, 0.18f, 0.6f), 1.0f);
	Y += 10.0f;
	DrawText(bCraftingJobsFocus ? TEXT("JOBS  (UP/DOWN select, X cancel, B back to recipes)") : TEXT("JOBS  ([B] select a job)"),
		bCraftingJobsFocus ? kCraftAccent : kCraftLabel, PX, Y, HudType::Font(), HudType::Label);
	Y += 26.0f;
	CraftingJobIndex = FMath::Clamp(CraftingJobIndex, 0, FMath::Max(Jobs.Num() - 1, 0));
	if (Jobs.IsEmpty())
	{
		DrawText(TEXT("Nothing in production."), kCraftDim, PX, Y, HudType::Font(), HudType::Body);
	}
	for (int32 i = 0; i < Jobs.Num() && Y < VH - 110.0f; ++i)
	{
		const FCraftingJob& Job = Jobs[i];
		if (bCraftingJobsFocus && i == CraftingJobIndex)
		{
			DrawRect(FLinearColor(0.35f, 0.24f, 0.12f, 0.5f), PX - 8.0f, Y - 2.0f, PW + 16.0f, 48.0f);
		}
		DrawText(FString::Printf(TEXT("%s  x%d"), *ItemName(Crafting, Job.OutputItem), Job.Crafts * Job.OutputPerCraft), kCraftText, PX, Y, HudType::Font(), HudType::Body);
		const FString Status = Job.IsRunning()
			? FString::Printf(TEXT("%s   run %d/%d   %s left"), *FName::NameToDisplayString(Job.Facility, false), Job.CraftsDone + 1, Job.Crafts, *FormatSeconds(Job.GetSecondsLeft()))
			: FString::Printf(TEXT("done   %d waiting (hold full?)"), Job.UnitsReady);
		DrawText(Status, Job.IsRunning() ? kCraftLabel : kCraftGood, PX + PW * 0.45f, Y, HudType::Font(), HudType::Label);
		// Progress of the current run.
		const float BarY = Y + 28.0f;
		DrawRect(FLinearColor(0.15f, 0.15f, 0.18f, 1.0f), PX, BarY, PW, 6.0f);
		const float Fraction = Job.IsRunning() ? (Job.CraftsDone + Job.GetCurrentCraftFraction()) / FMath::Max(1, Job.Crafts) : 1.0f;
		DrawRect(Job.IsRunning() ? kCraftAccent : kCraftGood, PX, BarY, PW * Fraction, 6.0f);
		Y += 52.0f;
	}

	DrawCentredText(TEXT("[LEFT/RIGHT] facility    [UP/DOWN] select    [SPACE] queue 1    [Q] queue 5    [X] queue all    [B] jobs    [ESC] back"),
		kCraftLabel, VW * 0.5f, VH - 40.0f, HudType::Label);
}
