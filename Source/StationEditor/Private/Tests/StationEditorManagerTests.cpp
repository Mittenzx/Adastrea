// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "StationEditorManager.h"
#include "StationGridSystem.h"
#include "Stations/SpaceStation.h"
#include "Stations/ReactorModule.h"
#include "Stations/CorridorModule.h"
#include "Stations/SolarArrayModule.h"
#include "Stations/StationCoreModule.h"
#include "Stations/OutfittingModule.h"
#include "StationModuleCatalog.h"
#include "Trading/PlayerTraderComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

namespace StationEditorTests
{
	/** A throwaway game world with one station in it, torn down on scope exit. */
	struct FTestWorld
	{
		UWorld* World = nullptr;
		ASpaceStation* Station = nullptr;

		explicit FTestWorld(const FVector& StationLocation)
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Station = World->SpawnActor<ASpaceStation>(ASpaceStation::StaticClass(), StationLocation, FRotator::ZeroRotator, Params);
		}

		~FTestWorld()
		{
			if (World)
			{
				GEngine->DestroyWorldContext(World);
				World->DestroyWorld(false);
			}
		}
	};

	/** Manager paying from its own trader wallet (the test world has no player pawn to resolve one from). */
	UStationEditorManager* MakeManager(int32 Credits = 1000000)
	{
		UStationEditorManager* Manager = NewObject<UStationEditorManager>();
		Manager->bRequireConstructionMaterials = false;
		Manager->bAutoResolvePlayerCargo = false;
		Manager->bAutoResolvePlayerTrader = false;
		Manager->PlayerTechLevel = 10;

		UPlayerTraderComponent* Trader = NewObject<UPlayerTraderComponent>(Manager);
		Trader->Credits = Credits;
		Trader->StartingCredits = Credits;
		Manager->PlayerTrader = Trader;
		return Manager;
	}

	int32 BuildCost(UStationEditorManager* Manager, TSubclassOf<ASpaceStationModule> ModuleClass)
	{
		FStationBuildCost Cost;
		return Manager->GetModuleBuildCost(ModuleClass, Cost) ? Cost.Credits : 0;
	}

	/** Reactor at the station origin, then corridors chained along +X. */
	TArray<ASpaceStationModule*> BuildChain(UStationEditorManager* Manager, ASpaceStation* Station, int32 NumCorridors)
	{
		TArray<ASpaceStationModule*> Built;
		ASpaceStationModule* Previous = Manager->PlaceModule(AReactorModule::StaticClass(), Station->GetActorLocation(), FRotator::ZeroRotator);
		Built.Add(Previous);
		for (int32 i = 0; i < NumCorridors && Previous; ++i)
		{
			FVector Position;
			Manager->FindAttachPosition(ACorridorModule::StaticClass(), Previous, FVector(1, 0, 0), Position);
			Previous = Manager->PlaceModule(ACorridorModule::StaticClass(), Position, FRotator::ZeroRotator);
			Built.Add(Previous);
		}
		return Built;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStationEditorPlaceUndoRedoTest, "Adastrea.StationEditor.PlaceUndoRedo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStationEditorPlaceUndoRedoTest::RunTest(const FString& Parameters)
{
	using namespace StationEditorTests;

	// Deliberately not on a multiple of the grid size: the grid must follow the station.
	FTestWorld TestWorld(FVector(1234.0f, 567.0f, 89.0f));
	UStationEditorManager* Manager = MakeManager();
	TestTrue(TEXT("BeginEditing"), Manager->BeginEditing(TestWorld.Station));
	TestEqual(TEXT("Grid origin follows the station"), Manager->GridSystem->GridOrigin, TestWorld.Station->GetActorLocation());

	const int32 StartCredits = Manager->GetPlayerCredits();
	const TArray<ASpaceStationModule*> Chain = BuildChain(Manager, TestWorld.Station, 1);
	if (!TestEqual(TEXT("Reactor + corridor placed"), TestWorld.Station->Modules.Num(), 2) || !Chain[1])
	{
		return false;
	}
	ASpaceStationModule* Corridor = Chain[1];
	TestTrue(TEXT("Reactor generates power"), Manager->GetTotalPowerGeneration() > 0.0f);
	TestEqual(TEXT("Credits charged"), Manager->GetPlayerCredits(),
		StartCredits - BuildCost(Manager, AReactorModule::StaticClass()) - BuildCost(Manager, ACorridorModule::StaticClass()));

	// Overlapping spot is rejected.
	TestEqual(TEXT("Overlap rejected"),
		Manager->CanPlaceModule(ACorridorModule::StaticClass(), Corridor->GetActorLocation(), FRotator::ZeroRotator),
		EModulePlacementResult::CollisionDetected);

	// Undo -> redo -> undo on the same placement (the second undo used to fail:
	// redo respawned a new actor and the undo record kept pointing at the old one).
	const int32 CreditsAfterBuild = Manager->GetPlayerCredits();
	TestTrue(TEXT("Undo place"), Manager->Undo());
	TestEqual(TEXT("Module gone after undo"), TestWorld.Station->Modules.Num(), 1);
	TestEqual(TEXT("Undo refunds"), Manager->GetPlayerCredits(), CreditsAfterBuild + BuildCost(Manager, ACorridorModule::StaticClass()));
	TestTrue(TEXT("Redo place"), Manager->Redo());
	TestTrue(TEXT("Redo brings back the same actor"), TestWorld.Station->Modules.Contains(Corridor));
	TestEqual(TEXT("Redo charges again"), Manager->GetPlayerCredits(), CreditsAfterBuild);
	TestTrue(TEXT("Second undo works"), Manager->Undo());
	TestEqual(TEXT("Module gone after second undo"), TestWorld.Station->Modules.Num(), 1);
	TestTrue(TEXT("Redo again"), Manager->Redo());

	// Rotation is normalised to 0..270 in the exported blueprint.
	TestTrue(TEXT("Rotate corridor"), Manager->RotateModule(Corridor, FRotator(0.0f, -90.0f, 0.0f)));
	TestTrue(TEXT("Export has 270 not -90"), Manager->ExportStationBlueprint().Contains(TEXT(":270:")));

	Manager->Save();
	TestEqual(TEXT("Save keeps modules"), TestWorld.Station->Modules.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStationEditorRemoveCancelTest, "Adastrea.StationEditor.RemoveAndCancel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStationEditorRemoveCancelTest::RunTest(const FString& Parameters)
{
	using namespace StationEditorTests;

	FTestWorld TestWorld(FVector::ZeroVector);
	UStationEditorManager* Manager = MakeManager();

	// Session 1: build reactor - corridor - corridor and commit it.
	Manager->BeginEditing(TestWorld.Station);
	const TArray<ASpaceStationModule*> Chain = BuildChain(Manager, TestWorld.Station, 2);
	if (!TestEqual(TEXT("Chain built"), TestWorld.Station->Modules.Num(), 3) || !Chain[1] || !Chain[2])
	{
		return false;
	}
	Manager->Save();

	// Session 2: removal rules, undo of a removal, and Cancel.
	Manager->BeginEditing(TestWorld.Station);
	const int32 StartCredits = Manager->GetPlayerCredits();
	TestFalse(TEXT("Middle module can't be removed (would split)"), Manager->CanRemoveModule(Chain[1]));
	TestFalse(TEXT("RemoveModule refuses the split"), Manager->RemoveModule(Chain[1]));
	TestTrue(TEXT("End module can be removed"), Manager->RemoveModule(Chain[2]));
	TestEqual(TEXT("Removed"), TestWorld.Station->Modules.Num(), 2);
	TestEqual(TEXT("Removing a committed module refunds nothing"), Manager->GetPlayerCredits(), StartCredits);
	TestTrue(TEXT("Undo removal"), Manager->Undo());
	TestTrue(TEXT("Same actor restored"), TestWorld.Station->Modules.Contains(Chain[2]));
	TestTrue(TEXT("Remove again"), Manager->RemoveModule(Chain[2]));

	// Build something new, then cancel the whole session.
	FVector Position;
	Manager->FindAttachPosition(ACorridorModule::StaticClass(), Chain[0], FVector(-1, 0, 0), Position);
	ASpaceStationModule* NewCorridor = Manager->PlaceModule(ACorridorModule::StaticClass(), Position, FRotator::ZeroRotator);
	TestNotNull(TEXT("New corridor placed"), NewCorridor);

	Manager->Cancel();
	TestEqual(TEXT("Cancel restores the original module count"), TestWorld.Station->Modules.Num(), 3);
	TestTrue(TEXT("Cancel brings the removed module back"), TestWorld.Station->Modules.Contains(Chain[2]));
	TestFalse(TEXT("Restored module is visible"), Chain[2]->IsHidden());
	TestFalse(TEXT("Cancel removes the new module"), TestWorld.Station->Modules.Contains(NewCorridor));
	TestEqual(TEXT("Cancel refunds the session's spend"), Manager->GetPlayerCredits(), StartCredits);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStationEditorQueueRefundTest, "Adastrea.StationEditor.QueueRefund",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStationEditorQueueRefundTest::RunTest(const FString& Parameters)
{
	using namespace StationEditorTests;

	FTestWorld TestWorld(FVector::ZeroVector);
	UStationEditorManager* Manager = MakeManager();
	Manager->BeginEditing(TestWorld.Station);
	ASpaceStationModule* Reactor = Manager->PlaceModule(AReactorModule::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Reactor"), Reactor))
	{
		return false;
	}

	const int32 CreditsBeforeQueue = Manager->GetPlayerCredits();
	FVector Position;
	Manager->FindAttachPosition(ACorridorModule::StaticClass(), Reactor, FVector(0, 1, 0), Position);
	const int32 QueueId = Manager->QueueConstruction(ACorridorModule::StaticClass(), Position, FRotator::ZeroRotator);
	TestTrue(TEXT("Queued"), QueueId > 0);
	TestEqual(TEXT("Queueing charges"), Manager->GetPlayerCredits(), CreditsBeforeQueue - BuildCost(Manager, ACorridorModule::StaticClass()));
	TestEqual(TEXT("Second build on the same queued spot is refused"),
		Manager->QueueConstruction(ACorridorModule::StaticClass(), Position, FRotator::ZeroRotator), -1);

	TestTrue(TEXT("Cancel queued build"), Manager->CancelConstruction(QueueId));
	TestEqual(TEXT("Cancelling refunds"), Manager->GetPlayerCredits(), CreditsBeforeQueue);

	// A build that finishes becomes a real module.
	Manager->QueueConstruction(ACorridorModule::StaticClass(), Position, FRotator::ZeroRotator);
	Manager->UpdateConstruction(100000.0f);
	TestEqual(TEXT("Queued build completed"), TestWorld.Station->Modules.Num(), 2);

	Manager->Save();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStationEditorTraderWalletTest, "Adastrea.StationEditor.TraderWallet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStationEditorTraderWalletTest::RunTest(const FString& Parameters)
{
	using namespace StationEditorTests;

	FTestWorld TestWorld(FVector::ZeroVector);
	UStationEditorManager* Manager = MakeManager(1000000);
	Manager->BeginEditing(TestWorld.Station);
	UPlayerTraderComponent* Trader = Manager->GetPlayerTrader();
	if (!TestNotNull(TEXT("Trader resolves"), Trader))
	{
		return false;
	}
	const int32 ReactorCost = BuildCost(Manager, AReactorModule::StaticClass());
	if (!TestTrue(TEXT("Reactor costs credits"), ReactorCost > 0))
	{
		return false;
	}

	// The editor reads and spends the trader's wallet, not a copy of it.
	TestEqual(TEXT("Editor credits are the trader's"), Manager->GetPlayerCredits(), Trader->GetCredits());
	TestNotNull(TEXT("Reactor placed"), Manager->PlaceModule(AReactorModule::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator));
	TestEqual(TEXT("Build charged the trader"), Trader->GetCredits(), 1000000 - ReactorCost);
	TestTrue(TEXT("Undo"), Manager->Undo());
	TestEqual(TEXT("Undo refunded the trader"), Trader->GetCredits(), 1000000);

	// Unaffordable: blocked, and nothing is taken.
	Trader->Credits = ReactorCost - 1;
	TestFalse(TEXT("Can't afford with too few credits"), Manager->CanAffordModule(AReactorModule::StaticClass()));
	TestNull(TEXT("Unaffordable placement blocked"),
		Manager->PlaceModule(AReactorModule::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator));
	TestEqual(TEXT("Blocked build takes nothing"), Trader->GetCredits(), ReactorCost - 1);

	// No wallet at all: costed modules are unaffordable instead of free.
	Manager->PlayerTrader = nullptr;
	TestNull(TEXT("No trader resolves"), Manager->GetPlayerTrader());
	TestEqual(TEXT("No trader reads as 0 credits"), Manager->GetPlayerCredits(), 0);
	TestFalse(TEXT("No trader: costed module unaffordable"), Manager->CanAffordModule(AReactorModule::StaticClass()));

	Manager->Cancel();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStationEditorCoreFootprintTest, "Adastrea.StationEditor.CoreFootprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStationEditorCoreFootprintTest::RunTest(const FString& Parameters)
{
	using namespace StationEditorTests;

	FTestWorld TestWorld(FVector::ZeroVector);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStationCoreModule* Core = TestWorld.World->SpawnActor<AStationCore_Research>(AStationCore_Research::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("Core spawned"), Core) || !TestEqual(TEXT("Research core is 3 cells"), Core->GetFootprintCells(), 3))
	{
		return false;
	}
	TestWorld.Station->AddModuleAtLocation(Core, FVector::ZeroVector);

	UStationEditorManager* Manager = MakeManager();
	TestTrue(TEXT("BeginEditing"), Manager->BeginEditing(TestWorld.Station));

	// 2x2 reactor against the 3x3 core's +Y face: 1.5 + 1 cells, rounded up to 3.
	FVector Position;
	TestTrue(TEXT("Attach to core"), Manager->FindAttachPosition(AReactorModule::StaticClass(), Core, FVector(0, 1, 0), Position));
	TestEqual(TEXT("Reactor clears the core footprint"), Position, FVector(0.0f, 1200.0f, 0.0f));
	ASpaceStationModule* Reactor = Manager->PlaceModule(AReactorModule::StaticClass(), Position, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Reactor placed against the core"), Reactor))
	{
		return false;
	}

	// A 3x1 solar array broadside to the reactor sits by its thin side (1 + 0.5
	// cells -> 2), not its long side as the old radius model did (1 + 1.5 -> 3).
	TestTrue(TEXT("Attach solar"), Manager->FindAttachPosition(ASolarArrayModule::StaticClass(), Reactor, FVector(0, 1, 0), Position));
	TestEqual(TEXT("Solar array sits flush"), Position, FVector(0.0f, 2000.0f, 0.0f));
	TestNotNull(TEXT("Solar array placed"), Manager->PlaceModule(ASolarArrayModule::StaticClass(), Position, FRotator::ZeroRotator));

	// Inside the core's footprint is a collision even though it is clear of the old radius.
	TestEqual(TEXT("Core footprint blocks"),
		Manager->CanPlaceModule(ACorridorModule::StaticClass(), FVector(400.0f, -400.0f, 0.0f), FRotator::ZeroRotator),
		EModulePlacementResult::CollisionDetected);

	// The core is fixed.
	TestFalse(TEXT("Core can't be removed"), Manager->CanRemoveModule(Core));
	TestFalse(TEXT("RemoveModule refuses the core"), Manager->RemoveModule(Core));
	TestFalse(TEXT("Core can't move"), Manager->MoveModule(Core, FVector(0.0f, -1200.0f, 0.0f)));
	TestFalse(TEXT("Core can't rotate"), Manager->RotateModule(Core, FRotator(0.0f, 90.0f, 0.0f)));
	TestTrue(TEXT("Export marks the core"), Manager->ExportStationBlueprint().Contains(TEXT(":StationCore_Research:0,0,0:0:1")));

	Manager->Cancel();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStationEditorOutfittingModulesTest, "Adastrea.StationEditor.OutfittingModules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStationEditorOutfittingModulesTest::RunTest(const FString& Parameters)
{
	using namespace StationEditorTests;

	// Every outfitting module is in the buildable catalog with a real footprint and cost.
	UStationModuleCatalog* Catalog = NewObject<UStationModuleCatalog>();
	Catalog->LoadCatalogFromJson();
	Catalog->LoadFootprintsFromJson();
	const TSubclassOf<ASpaceStationModule> OutfittingClasses[] = {
		AOutfittingModule::StaticClass(), AEngineWorkshopModule::StaticClass(), AArmouryModule::StaticClass(),
		AShieldWorkshopModule::StaticClass(), AHullWorksModule::StaticClass(), ACargoRefitModule::StaticClass(),
	};
	for (const TSubclassOf<ASpaceStationModule>& Class : OutfittingClasses)
	{
		FStationModuleEntry Entry;
		if (!TestTrue(*FString::Printf(TEXT("%s in catalog"), *Class->GetName()), Catalog->FindModuleByClass(Class, Entry)))
		{
			continue;
		}
		TestTrue(*FString::Printf(TEXT("%s costs credits"), *Class->GetName()), Entry.BuildCost.Credits > 0);
		TestTrue(*FString::Printf(TEXT("%s needs materials"), *Class->GetName()), Entry.BuildCost.Materials.Num() > 0);
		TestEqual(*FString::Printf(TEXT("%s footprint"), *Class->GetName()), Entry.GridFootprint, FIntVector(2, 2, 1));
		TestTrue(*FString::Printf(TEXT("%s tech level 2-3"), *Class->GetName()), Entry.RequiredTechLevel >= 2 && Entry.RequiredTechLevel <= 3);
	}

	// Build a workshop in the editor: the station gains that outfitting service.
	FTestWorld TestWorld(FVector::ZeroVector);
	UStationEditorManager* Manager = MakeManager();
	TestTrue(TEXT("BeginEditing"), Manager->BeginEditing(TestWorld.Station));
	ASpaceStationModule* Reactor = Manager->PlaceModule(AReactorModule::StaticClass(), TestWorld.Station->GetActorLocation(), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("reactor placed"), Reactor))
	{
		return false;
	}
	TestFalse(TEXT("no outfitting yet"), TestWorld.Station->HasOutfitting());

	FVector Position;
	TestTrue(TEXT("attach position"), Manager->FindAttachPosition(AEngineWorkshopModule::StaticClass(), Reactor, FVector(1, 0, 0), Position));
	const int32 CreditsBefore = Manager->GetPlayerCredits();
	AOutfittingModule* Workshop = Cast<AOutfittingModule>(
		Manager->PlaceModule(AEngineWorkshopModule::StaticClass(), Position, FRotator::ZeroRotator));
	if (!TestNotNull(TEXT("engine workshop placed"), Workshop))
	{
		return false;
	}
	TestEqual(TEXT("build cost charged"), Manager->GetPlayerCredits(), CreditsBefore - BuildCost(Manager, AEngineWorkshopModule::StaticClass()));
	TestTrue(TEXT("station has outfitting"), TestWorld.Station->HasOutfitting());
	TestTrue(TEXT("workshop services engines"), TestWorld.Station->GetOutfittingModuleFor(EShipUpgradeCategory::Engines) == Workshop);
	TestNull(TEXT("no weapons service"), TestWorld.Station->GetOutfittingModuleFor(EShipUpgradeCategory::Weapons));

	// Upgrading the module in the editor raises the tier it can fit.
	TestEqual(TEXT("workshop starts at tier 3"), Workshop->GetMaxTier(), EUpgradeTier::Tier3);
	TestTrue(TEXT("upgrade workshop"), Manager->UpgradeModule(Workshop));
	TestEqual(TEXT("upgraded workshop fits tier 4"), Workshop->GetMaxTier(), EUpgradeTier::Tier4);

	// The workshop round-trips through the station blueprint string (save games, sharing).
	TestTrue(TEXT("blueprint export names the workshop"), Manager->ExportStationBlueprint().Contains(TEXT("EngineWorkshopModule")));
	Manager->Save();
	TestTrue(TEXT("save keeps the workshop"), TestWorld.Station->Modules.Contains(Workshop));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
