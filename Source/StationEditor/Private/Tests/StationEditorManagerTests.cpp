// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "StationEditorManager.h"
#include "StationGridSystem.h"
#include "Stations/SpaceStation.h"
#include "Stations/ReactorModule.h"
#include "Stations/CorridorModule.h"
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

	UStationEditorManager* MakeManager()
	{
		UStationEditorManager* Manager = NewObject<UStationEditorManager>();
		Manager->bRequireConstructionMaterials = false;
		Manager->bAutoResolvePlayerCargo = false;
		Manager->PlayerTechLevel = 10;
		Manager->PlayerCredits = 1000000;
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

	const int32 StartCredits = Manager->PlayerCredits;
	const TArray<ASpaceStationModule*> Chain = BuildChain(Manager, TestWorld.Station, 1);
	if (!TestEqual(TEXT("Reactor + corridor placed"), TestWorld.Station->Modules.Num(), 2) || !Chain[1])
	{
		return false;
	}
	ASpaceStationModule* Corridor = Chain[1];
	TestTrue(TEXT("Reactor generates power"), Manager->GetTotalPowerGeneration() > 0.0f);
	TestEqual(TEXT("Credits charged"), Manager->PlayerCredits,
		StartCredits - BuildCost(Manager, AReactorModule::StaticClass()) - BuildCost(Manager, ACorridorModule::StaticClass()));

	// Overlapping spot is rejected.
	TestEqual(TEXT("Overlap rejected"),
		Manager->CanPlaceModule(ACorridorModule::StaticClass(), Corridor->GetActorLocation(), FRotator::ZeroRotator),
		EModulePlacementResult::CollisionDetected);

	// Undo -> redo -> undo on the same placement (the second undo used to fail:
	// redo respawned a new actor and the undo record kept pointing at the old one).
	const int32 CreditsAfterBuild = Manager->PlayerCredits;
	TestTrue(TEXT("Undo place"), Manager->Undo());
	TestEqual(TEXT("Module gone after undo"), TestWorld.Station->Modules.Num(), 1);
	TestEqual(TEXT("Undo refunds"), Manager->PlayerCredits, CreditsAfterBuild + BuildCost(Manager, ACorridorModule::StaticClass()));
	TestTrue(TEXT("Redo place"), Manager->Redo());
	TestTrue(TEXT("Redo brings back the same actor"), TestWorld.Station->Modules.Contains(Corridor));
	TestEqual(TEXT("Redo charges again"), Manager->PlayerCredits, CreditsAfterBuild);
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
	const int32 StartCredits = Manager->PlayerCredits;
	TestFalse(TEXT("Middle module can't be removed (would split)"), Manager->CanRemoveModule(Chain[1]));
	TestFalse(TEXT("RemoveModule refuses the split"), Manager->RemoveModule(Chain[1]));
	TestTrue(TEXT("End module can be removed"), Manager->RemoveModule(Chain[2]));
	TestEqual(TEXT("Removed"), TestWorld.Station->Modules.Num(), 2);
	TestEqual(TEXT("Removing a committed module refunds nothing"), Manager->PlayerCredits, StartCredits);
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
	TestEqual(TEXT("Cancel refunds the session's spend"), Manager->PlayerCredits, StartCredits);
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

	const int32 CreditsBeforeQueue = Manager->PlayerCredits;
	FVector Position;
	Manager->FindAttachPosition(ACorridorModule::StaticClass(), Reactor, FVector(0, 1, 0), Position);
	const int32 QueueId = Manager->QueueConstruction(ACorridorModule::StaticClass(), Position, FRotator::ZeroRotator);
	TestTrue(TEXT("Queued"), QueueId > 0);
	TestEqual(TEXT("Queueing charges"), Manager->PlayerCredits, CreditsBeforeQueue - BuildCost(Manager, ACorridorModule::StaticClass()));
	TestEqual(TEXT("Second build on the same queued spot is refused"),
		Manager->QueueConstruction(ACorridorModule::StaticClass(), Position, FRotator::ZeroRotator), -1);

	TestTrue(TEXT("Cancel queued build"), Manager->CancelConstruction(QueueId));
	TestEqual(TEXT("Cancelling refunds"), Manager->PlayerCredits, CreditsBeforeQueue);

	// A build that finishes becomes a real module.
	Manager->QueueConstruction(ACorridorModule::StaticClass(), Position, FRotator::ZeroRotator);
	Manager->UpdateConstruction(100000.0f);
	TestEqual(TEXT("Queued build completed"), TestWorld.Station->Modules.Num(), 2);

	Manager->Save();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
