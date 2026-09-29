// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Ships/ShipUpgradeDataAsset.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipDataAssetMobilityTest, "Adastrea.Ships.Mobility.DataAssetDrivesSpeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FShipDataAssetMobilityTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	ON_SCOPE_EXIT
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	USpaceshipDataAsset* Hauler = NewObject<USpaceshipDataAsset>(GetTransientPackage());
	Hauler->MaxSpeed = 350.0f;
	Hauler->Acceleration = 30.0f;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bDeferConstruction = true;
	ASpaceship* Ship = World->SpawnActor<ASpaceship>(ASpaceship::StaticClass(), FTransform::Identity, Params);
	if (!TestNotNull(TEXT("ship"), Ship))
	{
		return false;
	}
	Ship->ShipDataAsset = Hauler;
	Ship->FinishSpawning(FTransform::Identity);
	// The bare test world has no game mode, so actors don't get BeginPlay on their own.
	if (!Ship->HasActorBegunPlay())
	{
		Ship->DispatchBeginPlay();
	}

	const float ExpectedSpeed = 350.0f * Ship->DataAssetSpeedScale;
	const float ExpectedAccel = 30.0f * Ship->DataAssetAccelerationScale;
	TestEqual(TEXT("top speed from data asset"), Ship->DefaultMaxSpeed, ExpectedSpeed, 0.01f);
	TestEqual(TEXT("acceleration from data asset"), Ship->DefaultAcceleration, ExpectedAccel, 0.01f);
	TestEqual(TEXT("deceleration matches acceleration"), Ship->DefaultDeceleration, ExpectedAccel, 0.01f);
	if (TestNotNull(TEXT("movement"), Ship->MovementComponent.Get()))
	{
		TestEqual(TEXT("movement max speed"), Ship->MovementComponent->MaxSpeed, ExpectedSpeed, 0.01f);
		TestEqual(TEXT("movement acceleration"), Ship->MovementComponent->Acceleration, ExpectedAccel, 0.01f);
	}
	// Outfitting must scale the data-asset speed, not the old class default.
	TestEqual(TEXT("upgrade base speed"), Ship->GetBaseStat(ShipUpgradeStats::MaxSpeed), ExpectedSpeed, 0.01f);

	// The starter Viper keeps the speed it flew at before the wiring.
	TestEqual(TEXT("Viper speed unchanged"), 1200.0f * Ship->DataAssetSpeedScale, 3000.0f, 0.01f);
	TestEqual(TEXT("Viper accel unchanged"), 180.0f * Ship->DataAssetAccelerationScale, 1000.0f, 0.01f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
