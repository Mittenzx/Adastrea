#include "Combat/CombatTestDirector.h"
#include "AI/HostileFighterController.h"
#include "Ships/Spaceship.h"
#include "Player/AdastreaPlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "AdastreaLog.h"

namespace CombatTest
{
	const TCHAR* DefaultHostileShip = TEXT("/Game/Blueprints/Ships/BP_Ship_Fighter.BP_Ship_Fighter_C");
	/** Waves arrive within this cone (degrees) of the player's nose, so they're seen coming. */
	constexpr float ArrivalYawSpread = 50.0f;
	constexpr float ArrivalPitchSpread = 15.0f;
}

ACombatTestDirector::ACombatTestDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.25f;

	bAutoWaves = true;
	WaveSize = 2;
	FirstWaveDelay = 8.0f;
	WaveDelay = 6.0f;
	SpawnDistance = 25000.0f;

	WaveClock = 0.0f;
	WavesSent = 0;
}

void ACombatTestDirector::BeginPlay()
{
	Super::BeginPlay();
	WaveClock = -FirstWaveDelay + WaveDelay;
}

ACombatTestDirector* ACombatTestDirector::Find(UWorld* World)
{
	if (World)
	{
		for (TActorIterator<ACombatTestDirector> It(World); It; ++It)
		{
			return *It;
		}
	}
	return nullptr;
}

int32 ACombatTestDirector::CountHostiles(UWorld* World)
{
	int32 Count = 0;
	if (World)
	{
		for (TActorIterator<ASpaceship> It(World); It; ++It)
		{
			Count += AHostileFighterController::IsHostileShip(*It) ? 1 : 0;
		}
	}
	return Count;
}

int32 ACombatTestDirector::ClearHostiles(UWorld* World)
{
	TArray<ASpaceship*> Hostiles;
	if (World)
	{
		for (TActorIterator<ASpaceship> It(World); It; ++It)
		{
			if (AHostileFighterController::IsHostileShip(*It))
			{
				Hostiles.Add(*It);
			}
		}
	}
	for (ASpaceship* Ship : Hostiles)
	{
		if (AController* Pilot = Ship->GetController())
		{
			Pilot->UnPossess();
			Pilot->Destroy();
		}
		Ship->Destroy();
	}
	return Hostiles.Num();
}

int32 ACombatTestDirector::SpawnHostilesNearPlayer(UWorld* World, int32 Count, float Distance, TSubclassOf<ASpaceship> ShipClass)
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!World || !Player)
	{
		return 0;
	}
	if (!ShipClass)
	{
		ShipClass = StaticLoadClass(ASpaceship::StaticClass(), nullptr, CombatTest::DefaultHostileShip);
	}

	int32 Spawned = 0;
	const FRotator Nose = Player->GetActorRotation();
	for (int32 i = 0; i < Count; ++i)
	{
		const FRotator Bearing(
			Nose.Pitch + FMath::FRandRange(-CombatTest::ArrivalPitchSpread, CombatTest::ArrivalPitchSpread),
			Nose.Yaw + FMath::FRandRange(-CombatTest::ArrivalYawSpread, CombatTest::ArrivalYawSpread), 0.0f);
		const FVector Location = Player->GetActorLocation() + Bearing.Vector() * Distance;
		const FRotator Facing = (Player->GetActorLocation() - Location).Rotation();
		if (AHostileFighterController::SpawnHostile(World, ShipClass, Location, Facing))
		{
			++Spawned;
		}
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Spawned %d hostile(s) %.0f cm from the player"), Spawned, Distance);
	return Spawned;
}

void ACombatTestDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bAutoWaves || !Cast<ASpaceship>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		return;
	}
	if (CountHostiles(GetWorld()) > 0)
	{
		WaveClock = 0.0f;
		return;
	}
	WaveClock += DeltaSeconds;
	if (WaveClock >= WaveDelay)
	{
		WaveClock = 0.0f;
		if (SpawnHostilesNearPlayer(GetWorld(), WaveSize, SpawnDistance, HostileShipClass) > 0)
		{
			++WavesSent;
			if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
			{
				PC->ShowHUDMessage(FString::Printf(TEXT("Hostiles inbound: wave %d (%d)"), WavesSent, WaveSize), 3.0f, true);
			}
		}
	}
}
