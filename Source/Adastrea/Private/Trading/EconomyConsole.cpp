// Copyright Mittenzx. All Rights Reserved.

// Console commands for testing the economy: market prices by region, cargo with stolen
// tags, cargo-transfer drones and wrecks to strip.
//
//   adastrea.MarketInfo [Filter]               the docked (or nearest) station's law and prices
//   adastrea.CargoInfo                         the player's hold, stolen goods marked
//   adastrea.AddCargo Item Units [StolenFrom]  put goods in the player's hold (crafting id, e.g. NickelOre)
//   adastrea.WreckShip [Name] [Item Units]     disable the nearest ship (matching Name), optionally loading its hold first
//   adastrea.CargoTransfer take|give [Name] [Item]   send cargo drones to the nearest ship (matching Name)

#include "AdastreaLog.h"
#include "AdastreaNames.h"
#include "Drones/DroneBayComponent.h"
#include "Ships/Spaceship.h"
#include "Stations/MarketplaceModule.h"
#include "Stations/SpaceStation.h"
#include "Trading/CargoComponent.h"
#include "Trading/CraftingManager.h"
#include "Trading/EconomyManager.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/TradeItemDataAsset.h"
#include "Universe/OrganisationSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

namespace EconomyConsole
{
	ASpaceship* PlayerShip(UWorld* World)
	{
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		return PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	}

	ASpaceStation* DockedOrNearestStation(UWorld* World)
	{
		ASpaceship* Ship = PlayerShip(World);
		if (ASpaceStation* Docked = Ship ? Ship->GetDockedStation() : nullptr)
		{
			return Docked;
		}
		ASpaceStation* Best = nullptr;
		double BestDist = TNumericLimits<double>::Max();
		for (TActorIterator<ASpaceStation> It(World); It; ++It)
		{
			const double Dist = Ship ? FVector::DistSquared(It->GetActorLocation(), Ship->GetActorLocation()) : 0.0;
			if (Dist < BestDist)
			{
				Best = *It;
				BestDist = Dist;
			}
		}
		return Best;
	}

	/** Nearest other ship whose object or display name contains Fragment (any ship if empty). */
	ASpaceship* NearestShip(UWorld* World, const FString& Fragment)
	{
		ASpaceship* Self = PlayerShip(World);
		ASpaceship* Best = nullptr;
		double BestDist = TNumericLimits<double>::Max();
		for (TActorIterator<ASpaceship> It(World); It; ++It)
		{
			if (*It == Self || (!Fragment.IsEmpty() && !It->GetName().Contains(Fragment) && !AdastreaNames::ForActor(*It).Contains(Fragment)))
			{
				continue;
			}
			const double Dist = Self ? FVector::DistSquared(It->GetActorLocation(), Self->GetActorLocation()) : 0.0;
			if (Dist < BestDist)
			{
				Best = *It;
				BestDist = Dist;
			}
		}
		return Best;
	}

	UTradeItemDataAsset* FindItem(UWorld* World, const FString& Id)
	{
		const UCraftingManager* Crafting = UCraftingManager::Get(World);
		return Crafting ? Crafting->GetLoader()->GetTradeItem(Id) : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs MarketInfoCmd(
		TEXT("adastrea.MarketInfo"),
		TEXT("Log the docked (or nearest) station's law and market prices. Usage: adastrea.MarketInfo [Filter=raw]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ASpaceStation* Station = DockedOrNearestStation(World);
			AMarketplaceModule* Marketplace = Station ? Station->GetMarketplaceModule() : nullptr;
			UMarketDataAsset* Market = Marketplace ? Marketplace->GetMarketData() : nullptr;
			UEconomyManager* Economy = World && World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UEconomyManager>() : nullptr;
			if (!Market || !Economy)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("adastrea.MarketInfo: no station market"));
				return;
			}
			const EStationLaw Law = Station->GetEffectiveLaw();
			UE_LOG(LogAdastrea, Display, TEXT("Market at %s: %s%s, %d entries"), *Station->GetDisplayNameString(),
				Law == EStationLaw::Fence ? TEXT("FENCE") : TEXT("LAWFUL"),
				Market->BuysStolenGoods() ? *FString::Printf(TEXT(" (buys stolen goods at %.0f%%)"), Market->StolenGoodsRate * 100.0f) : TEXT(""),
				Market->Inventory.Num());
			const FString Filter = Args.IsEmpty() ? FString(TEXT("raw")) : Args[0];
			for (const FMarketInventoryEntry& Entry : Market->Inventory)
			{
				UTradeItemDataAsset* Item = Entry.TradeItem;
				const bool bShow = Item && (Filter == TEXT("all")
					|| (Filter == TEXT("raw") && Item->Category == ETradeItemCategory::RawMaterials)
					|| Item->ItemID.ToString().Contains(Filter));
				if (bShow)
				{
					UE_LOG(LogAdastrea, Display, TEXT("  %-24s base %6.0f  buy %7.1f  sell %7.1f  stock %6d  supply %.2f demand %.2f"),
						*Item->ItemName.ToString(), Item->BasePrice, Economy->GetItemPrice(Market, Item, true),
						Economy->GetItemPrice(Market, Item, false), Entry.CurrentStock, Entry.SupplyLevel, Entry.DemandLevel);
				}
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs CargoInfoCmd(
		TEXT("adastrea.CargoInfo"),
		TEXT("Log the player's cargo hold, stolen goods marked."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
		{
			ASpaceship* Ship = PlayerShip(World);
			UCargoComponent* Cargo = Ship ? Ship->CargoComponent.Get() : nullptr;
			if (!Cargo)
			{
				return;
			}
			UE_LOG(LogAdastrea, Display, TEXT("Hold: %.1f / %.1f used, %d units (%d stolen)"), Cargo->CargoCapacity - Cargo->GetAvailableCargoSpace(),
				Cargo->CargoCapacity, Cargo->GetTotalUnits(), Cargo->GetTotalStolenUnits());
			for (const FCargoEntry& Entry : Cargo->CargoInventory)
			{
				UE_LOG(LogAdastrea, Display, TEXT("  %4d x %s%s"), Entry.Quantity, Entry.Item ? *Entry.Item->ItemName.ToString() : TEXT("?"),
					Entry.IsStolen() ? *FString::Printf(TEXT("  [stolen from %s]"), *Entry.StolenFrom.ToString()) : TEXT(""));
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs AddCargoCmd(
		TEXT("adastrea.AddCargo"),
		TEXT("Put goods in the player's hold. Usage: adastrea.AddCargo Item Units [StolenFrom]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ASpaceship* Ship = PlayerShip(World);
			UTradeItemDataAsset* Item = Args.Num() >= 1 ? FindItem(World, Args[0]) : nullptr;
			const int32 Units = Args.Num() >= 2 ? FCString::Atoi(*Args[1]) : 1;
			const FName StolenFrom = Args.Num() >= 3 ? FName(*Args[2]) : NAME_None;
			const bool bOk = Ship && Ship->CargoComponent && Item && Ship->CargoComponent->AddStolenCargo(Item, Units, StolenFrom);
			UE_LOG(LogAdastrea, Display, TEXT("adastrea.AddCargo %s x%d %s: %s"), Args.Num() ? *Args[0] : TEXT("?"), Units,
				*StolenFrom.ToString(), bOk ? TEXT("ok") : TEXT("failed (unknown item or no room)"));
		}));

	FAutoConsoleCommandWithWorldAndArgs WreckShipCmd(
		TEXT("adastrea.WreckShip"),
		TEXT("Disable the nearest ship (matching Name), optionally loading its hold first. Usage: adastrea.WreckShip [Name] [Item Units]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ASpaceship* Ship = NearestShip(World, Args.Num() >= 1 ? Args[0] : FString());
			if (!Ship)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("adastrea.WreckShip: no ship found"));
				return;
			}
			if (Args.Num() >= 3 && Ship->CargoComponent)
			{
				if (UTradeItemDataAsset* Item = FindItem(World, Args[1]))
				{
					Ship->CargoComponent->CargoCapacity = FMath::Max(Ship->CargoComponent->CargoCapacity,
						Ship->CargoComponent->CargoCapacity - Ship->CargoComponent->GetAvailableCargoSpace() + Item->GetTotalVolume(FCString::Atoi(*Args[2])));
					Ship->CargoComponent->AddCargo(Item, FCString::Atoi(*Args[2]));
				}
			}
			Ship->SetWrecked(true);
			const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(World);
			UE_LOG(LogAdastrea, Display, TEXT("adastrea.WreckShip: %s (%s, owner %s) is a wreck with %d units aboard"), *Ship->GetName(),
				*AdastreaNames::ForActor(Ship), Orgs ? *Orgs->GetOwnerIdOf(Ship).ToString() : TEXT("?"),
				Ship->CargoComponent ? Ship->CargoComponent->GetTotalUnits() : 0);
		}));

	FAutoConsoleCommandWithWorldAndArgs CargoTransferCmd(
		TEXT("adastrea.CargoTransfer"),
		TEXT("Send the player's cargo drones to the nearest ship (matching Name). Usage: adastrea.CargoTransfer take|give [Name] [Item]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ASpaceship* Self = PlayerShip(World);
			const bool bTake = Args.IsEmpty() || Args[0] != TEXT("give");
			ASpaceship* Other = NearestShip(World, Args.Num() >= 2 ? Args[1] : FString());
			if (!Self || !Self->DroneBay || !Other)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("adastrea.CargoTransfer: no drone bay or no other ship"));
				return;
			}
			FText Why;
			const bool bOk = Self->DroneBay->CanTransferWith(Other, bTake, &Why)
				&& Self->DroneBay->StartCargoTransfer(Other, bTake, Args.Num() >= 3 ? FName(*Args[2]) : NAME_None);
			UE_LOG(LogAdastrea, Display, TEXT("adastrea.CargoTransfer %s %s: %s"), bTake ? TEXT("take from") : TEXT("give to"),
				*Other->GetName(), bOk ? TEXT("drones away") : *Why.ToString());
		}));
}
