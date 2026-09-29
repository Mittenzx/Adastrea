// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Ships/ShipUpgradeCatalogSubsystem.h"
#include "Ships/ShipUpgradeComponent.h"
#include "Ships/ShipUpgradeDataAsset.h"
#include "Ships/Spaceship.h"
#include "Stations/OutfittingModule.h"
#include "Stations/SpaceStation.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace ShipOutfittingTests
{
	/** A throwaway game world, torn down on scope exit. */
	struct FTestWorld
	{
		UWorld* World = nullptr;

		FTestWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
		}

		~FTestWorld()
		{
			if (World)
			{
				GEngine->DestroyWorldContext(World);
				World->DestroyWorld(false);
			}
		}

		template <typename T>
		T* Spawn(UClass* Class = T::StaticClass())
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			return World->SpawnActor<T>(Class, FVector::ZeroVector, FRotator::ZeroRotator, Params);
		}
	};

	UShipUpgradeDataAsset* MakeUpgrade(FName Id, EShipUpgradeCategory Category, int32 Tier, int32 Cost, FName Stat,
		float Add, float Mult, int32 MaxStack = 1)
	{
		UShipUpgradeDataAsset* Upgrade = NewObject<UShipUpgradeDataAsset>(GetTransientPackage());
		Upgrade->UpgradeID = Id;
		Upgrade->DisplayName = FText::FromName(Id);
		Upgrade->Category = Category;
		Upgrade->Tier = static_cast<EUpgradeTier>(Tier - 1);
		Upgrade->Requirements.CreditCost = Cost;
		Upgrade->MaxStackCount = MaxStack;
		Upgrade->bIsUnique = MaxStack == 1;
		FShipUpgradeStatModifier& Modifier = Upgrade->StatModifiers.AddDefaulted_GetRef();
		Modifier.StatName = Stat;
		Modifier.AdditiveBonus = Add;
		Modifier.MultiplicativeBonus = Mult;
		return Upgrade;
	}

	UPlayerTraderComponent* MakeWallet(int32 Credits)
	{
		UPlayerTraderComponent* Wallet = NewObject<UPlayerTraderComponent>(GetTransientPackage());
		Wallet->Credits = Credits;
		Wallet->StartingCredits = Credits;
		return Wallet;
	}
}

using namespace ShipOutfittingTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipUpgradeCatalogParseTest, "Adastrea.Ships.Outfitting.CatalogParses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FShipUpgradeCatalogParseTest::RunTest(const FString& Parameters)
{
	FString Json;
	const FString Path = FPaths::ProjectContentDir() + UShipUpgradeCatalogSubsystem::GetCatalogPath();
	if (!TestTrue(TEXT("catalog file readable"), FFileHelper::LoadFileToString(Json, *Path)))
	{
		return false;
	}

	TArray<UShipUpgradeDataAsset*> Upgrades;
	TArray<FString> Errors;
	const int32 Count = UShipUpgradeCatalogSubsystem::ParseCatalog(Json, GetTransientPackage(), Upgrades, &Errors);
	TestEqual(TEXT("no parse errors"), Errors.Num(), 0);
	for (const FString& Error : Errors)
	{
		AddError(Error);
	}
	TestTrue(TEXT("catalog has upgrades"), Count > 0);

	// Every category a workshop services has something at tier 1, so any outfitting module sells something.
	for (EShipUpgradeCategory Category : { EShipUpgradeCategory::Engines, EShipUpgradeCategory::Weapons,
		EShipUpgradeCategory::Shields, EShipUpgradeCategory::Hull, EShipUpgradeCategory::Cargo })
	{
		TestTrue(*FString::Printf(TEXT("tier-1 %s upgrade exists"), *UShipUpgradeDataAsset::GetCategoryName(Category).ToString()),
			Upgrades.ContainsByPredicate([Category](const UShipUpgradeDataAsset* U)
			{
				return U->Category == Category && U->Tier == EUpgradeTier::Tier1;
			}));
	}

	// Only stats ASpaceship applies are allowed.
	const TSet<FName> Known = { ShipUpgradeStats::MaxSpeed, ShipUpgradeStats::Acceleration, ShipUpgradeStats::BoostMultiplier,
		ShipUpgradeStats::HullStrength, ShipUpgradeStats::ShieldStrength, ShipUpgradeStats::CargoCapacity,
		ShipUpgradeStats::WeaponDamage, ShipUpgradeStats::MiningPower };
	for (const UShipUpgradeDataAsset* Upgrade : Upgrades)
	{
		TestTrue(*FString::Printf(TEXT("%s has a cost"), *Upgrade->UpgradeID.ToString()), Upgrade->Requirements.CreditCost > 0);
		for (const FShipUpgradeStatModifier& Modifier : Upgrade->StatModifiers)
		{
			TestTrue(*FString::Printf(TEXT("%s stat %s is applied by ASpaceship"), *Upgrade->UpgradeID.ToString(), *Modifier.StatName.ToString()),
				Known.Contains(Modifier.StatName));
		}
	}

	// Exclusions resolve both ways.
	const UShipUpgradeDataAsset* const* Overdrive = Upgrades.FindByPredicate([](const UShipUpgradeDataAsset* U) { return U->UpgradeID == TEXT("ENG_OverdriveManifold"); });
	if (TestNotNull(TEXT("overdrive present"), Overdrive))
	{
		TestEqual(TEXT("overdrive excludes the afterburner coil"), (*Overdrive)->MutuallyExclusiveWith.Num(), 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipUpgradeStatMathTest, "Adastrea.Ships.Outfitting.StatMathAndSlots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FShipUpgradeStatMathTest::RunTest(const FString& Parameters)
{
	UShipUpgradeComponent* Upgrades = NewObject<UShipUpgradeComponent>(GetTransientPackage());
	Upgrades->CategorySlots.Add(EShipUpgradeCategory::Cargo, 2);

	UShipUpgradeDataAsset* Racks = MakeUpgrade(TEXT("Racks"), EShipUpgradeCategory::Cargo, 1, 100, ShipUpgradeStats::CargoCapacity, 10.0f, 1.0f, 3);
	UShipUpgradeDataAsset* Expand = MakeUpgrade(TEXT("Expand"), EShipUpgradeCategory::Cargo, 2, 200, ShipUpgradeStats::CargoCapacity, 0.0f, 1.5f);

	// (Base + adds) * mults, whatever the fitting order.
	TestTrue(TEXT("expand fits"), Upgrades->InstallUpgrade(Expand));
	TestTrue(TEXT("racks fit"), Upgrades->InstallUpgrade(Racks));
	TestEqual(TEXT("(50 + 10) * 1.5"), Upgrades->GetStatModifier(ShipUpgradeStats::CargoCapacity, 50.0f), 90.0f);
	TestEqual(TEXT("unrelated stat untouched"), Upgrades->GetStatModifier(ShipUpgradeStats::MaxSpeed, 3000.0f), 3000.0f);

	// Two cargo slots, both used: a second rack stack is refused.
	FText Reason;
	TestEqual(TEXT("cargo slots used"), Upgrades->GetUsedSlotCount(EShipUpgradeCategory::Cargo), 2);
	TestFalse(TEXT("third cargo upgrade refused"), Upgrades->CanInstallUpgrade(Racks, Reason));

	// Previews: one more rack / one fewer expansion.
	Upgrades->CategorySlots.Add(EShipUpgradeCategory::Cargo, 3);
	TestEqual(TEXT("preview add a rack"), Upgrades->PreviewStat(ShipUpgradeStats::CargoCapacity, 50.0f, Racks, NAME_None), 105.0f);
	TestEqual(TEXT("preview remove expansion"), Upgrades->PreviewStat(ShipUpgradeStats::CargoCapacity, 50.0f, nullptr, TEXT("Expand")), 60.0f);

	// Stacks come off one at a time.
	TestTrue(TEXT("second rack"), Upgrades->InstallUpgrade(Racks));
	TestEqual(TEXT("two rack stacks"), Upgrades->GetUpgradeStackCount(TEXT("Racks")), 2);
	TestTrue(TEXT("remove a rack"), Upgrades->UninstallUpgrade(TEXT("Racks")));
	TestEqual(TEXT("one rack stack left"), Upgrades->GetUpgradeStackCount(TEXT("Racks")), 1);

	// A unique upgrade can't be fitted twice.
	TestFalse(TEXT("expansion is unique"), Upgrades->CanInstallUpgrade(Expand, Reason));

	// Mutually exclusive upgrades.
	UShipUpgradeDataAsset* Heavy = MakeUpgrade(TEXT("Heavy"), EShipUpgradeCategory::Cargo, 1, 100, ShipUpgradeStats::CargoCapacity, 5.0f, 1.0f);
	Heavy->MutuallyExclusiveWith.Add(Expand);
	TestFalse(TEXT("exclusive upgrade refused"), Upgrades->CanInstallUpgrade(Heavy, Reason));

	// No slots for a category the ship doesn't have.
	Upgrades->CategorySlots.Remove(EShipUpgradeCategory::Engines);
	UShipUpgradeDataAsset* Engine = MakeUpgrade(TEXT("Eng"), EShipUpgradeCategory::Engines, 1, 100, ShipUpgradeStats::MaxSpeed, 0.0f, 1.1f);
	TestFalse(TEXT("no engine slots"), Upgrades->CanInstallUpgrade(Engine, Reason));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipOutfittingPurchaseTest, "Adastrea.Ships.Outfitting.StationModulesGateBuyAndSell",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FShipOutfittingPurchaseTest::RunTest(const FString& Parameters)
{
	FTestWorld TestWorld;
	ASpaceStation* Station = TestWorld.Spawn<ASpaceStation>();
	AOutfittingModule* Bay = TestWorld.Spawn<AOutfittingModule>();
	AEngineWorkshopModule* Workshop = TestWorld.Spawn<AEngineWorkshopModule>();
	if (!TestNotNull(TEXT("station"), Station) || !TestNotNull(TEXT("bay"), Bay) || !TestNotNull(TEXT("workshop"), Workshop))
	{
		return false;
	}

	UShipUpgradeDataAsset* Tune = MakeUpgrade(TEXT("Tune"), EShipUpgradeCategory::Engines, 1, 1000, ShipUpgradeStats::MaxSpeed, 0.0f, 1.1f);
	UShipUpgradeDataAsset* Drive = MakeUpgrade(TEXT("Drive"), EShipUpgradeCategory::Engines, 3, 5000, ShipUpgradeStats::MaxSpeed, 0.0f, 1.2f);
	UShipUpgradeDataAsset* Plating = MakeUpgrade(TEXT("Plating"), EShipUpgradeCategory::Hull, 1, 800, ShipUpgradeStats::HullStrength, 100.0f, 1.0f);

	// Station capability comes from its modules.
	TestFalse(TEXT("no outfitting before modules"), Station->HasOutfitting());
	Station->AddModule(Bay);
	TestTrue(TEXT("outfitting with a bay"), Station->HasOutfitting());
	TestTrue(TEXT("general bay services engines"), Station->GetOutfittingModuleFor(EShipUpgradeCategory::Engines) == Bay);
	Station->AddModule(Workshop);
	TestTrue(TEXT("specialist (higher tier) wins"), Station->GetOutfittingModuleFor(EShipUpgradeCategory::Engines) == Workshop);
	TestTrue(TEXT("hull still from the bay"), Station->GetOutfittingModuleFor(EShipUpgradeCategory::Hull) == Bay);

	// Tiers: bay fits T1-2, workshop T1-3, and each module upgrade level adds one.
	TestFalse(TEXT("bay can't fit tier 3"), Bay->CanFit(Drive));
	TestTrue(TEXT("workshop fits tier 3"), Workshop->CanFit(Drive));
	TestFalse(TEXT("workshop doesn't do hull"), Workshop->CanFit(Plating));
	Bay->UpgradeLevel = 1;
	TestTrue(TEXT("upgraded bay fits tier 3"), Bay->CanFit(Drive));
	Bay->UpgradeLevel = 0;

	// Stock is limited to serviced categories and tiers.
	Workshop->AdditionalStock = { Tune, Drive, Plating };
	TestEqual(TEXT("workshop stocks its engine upgrades"), Workshop->GetStock(EShipUpgradeCategory::Engines).Num(), 2);
	TestEqual(TEXT("workshop stocks no hull"), Workshop->GetStock(EShipUpgradeCategory::Hull).Num(), 0);

	UShipUpgradeComponent* Upgrades = NewObject<UShipUpgradeComponent>(GetTransientPackage());
	UPlayerTraderComponent* Wallet = MakeWallet(5500);
	FText Reason;

	TestFalse(TEXT("bay refuses tier 3"), Upgrades->PurchaseUpgrade(Drive, Bay, Wallet, Reason));
	TestFalse(TEXT("no module, no sale"), Upgrades->PurchaseUpgrade(Tune, nullptr, Wallet, Reason));
	TestTrue(TEXT("buy tune at workshop"), Upgrades->PurchaseUpgrade(Tune, Workshop, Wallet, Reason));
	TestEqual(TEXT("charged list price"), Wallet->GetCredits(), 4500);
	TestFalse(TEXT("can't afford drive"), Upgrades->PurchaseUpgrade(Drive, Workshop, Wallet, Reason));
	TestTrue(TEXT("installed"), Upgrades->IsUpgradeInstalled(TEXT("Tune")));

	// Price multiplier.
	Workshop->PriceMultiplier = 0.5f;
	TestTrue(TEXT("discounted drive affordable"), Upgrades->PurchaseUpgrade(Drive, Workshop, Wallet, Reason));
	TestEqual(TEXT("charged half price"), Wallet->GetCredits(), 2000);

	// Selling needs a module for the category and refunds half the list price.
	int32 Refund = 0;
	AOutfittingModule* HullOnly = TestWorld.Spawn<AHullWorksModule>();
	TestFalse(TEXT("hull works can't remove engines"), Upgrades->SellUpgrade(TEXT("Tune"), HullOnly, Wallet, Refund, Reason));
	TestTrue(TEXT("sell tune"), Upgrades->SellUpgrade(TEXT("Tune"), Workshop, Wallet, Refund, Reason));
	TestEqual(TEXT("refund is half list"), Refund, 500);
	TestEqual(TEXT("credits after refund"), Wallet->GetCredits(), 2500);
	TestFalse(TEXT("tune removed"), Upgrades->IsUpgradeInstalled(TEXT("Tune")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipUpgradeAppliesToShipTest, "Adastrea.Ships.Outfitting.UpgradesChangeShipStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FShipUpgradeAppliesToShipTest::RunTest(const FString& Parameters)
{
	FTestWorld TestWorld;
	ASpaceship* Ship = TestWorld.Spawn<ASpaceship>();
	if (!TestNotNull(TEXT("ship"), Ship) || !TestNotNull(TEXT("upgrade component"), Ship->UpgradeComponent.Get())
		|| !TestNotNull(TEXT("cargo"), Ship->CargoComponent.Get()))
	{
		return false;
	}
	// The bare test world has no game mode, so actors don't get BeginPlay on their own;
	// the ship captures its base stats and binds to its upgrades there.
	if (!Ship->HasActorBegunPlay())
	{
		Ship->DispatchBeginPlay();
	}

	const float BaseSpeed = Ship->DefaultMaxSpeed;
	const float BaseCargo = Ship->CargoComponent->CargoCapacity;
	const float BaseHull = Ship->MaxHullIntegrity;
	UShipUpgradeDataAsset* Drive = MakeUpgrade(TEXT("Drive"), EShipUpgradeCategory::Engines, 1, 100, ShipUpgradeStats::MaxSpeed, 0.0f, 1.2f);
	UShipUpgradeDataAsset* Hold = MakeUpgrade(TEXT("Hold"), EShipUpgradeCategory::Cargo, 1, 100, ShipUpgradeStats::CargoCapacity, 20.0f, 1.0f);
	UShipUpgradeDataAsset* Armour = MakeUpgrade(TEXT("Armour"), EShipUpgradeCategory::Hull, 1, 100, ShipUpgradeStats::HullStrength, 0.0f, 2.0f);
	UShipUpgradeDataAsset* Shield = MakeUpgrade(TEXT("Shield"), EShipUpgradeCategory::Shields, 1, 100, ShipUpgradeStats::ShieldStrength, 150.0f, 1.0f);

	TestTrue(TEXT("fit drive"), Ship->UpgradeComponent->InstallUpgrade(Drive));
	TestTrue(TEXT("fit hold"), Ship->UpgradeComponent->InstallUpgrade(Hold));
	TestTrue(TEXT("fit armour"), Ship->UpgradeComponent->InstallUpgrade(Armour));
	TestTrue(TEXT("fit shield"), Ship->UpgradeComponent->InstallUpgrade(Shield));

	TestEqual(TEXT("top speed +20%"), Ship->DefaultMaxSpeed, BaseSpeed * 1.2f, 0.01f);
	TestEqual(TEXT("movement max speed follows"), Ship->MovementComponent->MaxSpeed, BaseSpeed * 1.2f, 0.01f);
	TestEqual(TEXT("hold +20"), Ship->CargoComponent->CargoCapacity, BaseCargo + 20.0f, 0.01f);
	TestEqual(TEXT("hull doubled"), Ship->MaxHullIntegrity, BaseHull * 2.0f, 0.01f);
	TestEqual(TEXT("hull stays full"), Ship->CurrentHullIntegrity, Ship->MaxHullIntegrity, 0.01f);
	TestEqual(TEXT("shields from zero base"), Ship->GetMaxShieldStrength(), Ship->GetBaseStat(ShipUpgradeStats::ShieldStrength) + 150.0f, 0.01f);

	// The hold can't shrink below what it carries.
	UTradeItemDataAsset* Ore = NewObject<UTradeItemDataAsset>(GetTransientPackage());
	Ore->ItemID = TEXT("TestOre");
	Ore->VolumePerUnit = 1.0f;
	TestTrue(TEXT("load cargo past stock capacity"), Ship->CargoComponent->AddCargo(Ore, FMath::FloorToInt(BaseCargo) + 5));
	FText Reason;
	TestFalse(TEXT("can't remove hold while it's full"), Ship->CanRemoveUpgrade(Hold, Reason));
	Ship->CargoComponent->ClearCargo();
	TestTrue(TEXT("can remove hold once empty"), Ship->CanRemoveUpgrade(Hold, Reason));

	// Removing everything restores stock stats.
	Ship->UpgradeComponent->UninstallAllUpgrades();
	TestEqual(TEXT("speed restored"), Ship->DefaultMaxSpeed, BaseSpeed, 0.01f);
	TestEqual(TEXT("hold restored"), Ship->CargoComponent->CargoCapacity, BaseCargo, 0.01f);
	TestEqual(TEXT("hull restored"), Ship->MaxHullIntegrity, BaseHull, 0.01f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
