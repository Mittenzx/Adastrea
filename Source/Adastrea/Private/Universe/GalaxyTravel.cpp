// Copyright Epic Games, Inc. All Rights Reserved.

// Jump-gate travel for UGalaxySubsystem: routes, starting a jump (snapshot +
// level load) and completing it in the destination level.

#include "Universe/GalaxySubsystem.h"
#include "Universe/JumpGate.h"
#include "AdastreaHUD.h"
#include "AdastreaLog.h"
#include "Player/AdastreaPlayerController.h"
#include "Player/SaveGameSubsystem.h"
#include "Ships/Spaceship.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
	constexpr float JumpFadeSeconds = 0.7f;

	// adastrea.JumpTo <sectorId>: jump without flying to the gate (dev / testing).
	static FAutoConsoleCommandWithWorldAndArgs GJumpToCmd(
		TEXT("adastrea.JumpTo"),
		TEXT("Jump the player to a galaxy sector (id from Galaxy.json) as if through a gate: adastrea.JumpTo adastrea_alpha"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(World);
			APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
			if (!Galaxy || !PC || Args.Num() == 0)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Usage: adastrea.JumpTo <sectorId>"));
				return;
			}
			FString Reason;
			if (!Galaxy->BeginJump(PC, FName(*Args[0]), Reason))
			{
				UE_LOG(LogAdastrea, Warning, TEXT("adastrea.JumpTo %s: %s"), *Args[0], *Reason);
			}
		}));
}

TArray<FName> UGalaxySubsystem::GetGateDestinations(FName SectorId) const
{
	TArray<FName> Out;
	if (const FGalaxySectorDef* Sector = FindSector(SectorId))
	{
		Out = Sector->Gates;
		for (const FName Lane : Sector->LaneGates)
		{
			Out.AddUnique(Lane);
		}
	}
	return Out;
}

TArray<FName> UGalaxySubsystem::FindRoute(FName FromSectorId, FName ToSectorId) const
{
	TArray<FName> Route;
	if (FromSectorId == ToSectorId || !FindSector(FromSectorId) || !FindSector(ToSectorId))
	{
		return Route;
	}

	// Breadth-first over gates + lane gates (every jump costs the same).
	TMap<FName, FName> CameFrom;
	TArray<FName> Queue = { FromSectorId };
	CameFrom.Add(FromSectorId, NAME_None);
	for (int32 Head = 0; Head < Queue.Num(); ++Head)
	{
		const FName Cur = Queue[Head];
		if (Cur == ToSectorId)
		{
			break;
		}
		for (const FName Next : GetGateDestinations(Cur))
		{
			if (!CameFrom.Contains(Next))
			{
				CameFrom.Add(Next, Cur);
				Queue.Add(Next);
			}
		}
	}
	if (!CameFrom.Contains(ToSectorId))
	{
		return Route;
	}
	for (FName Step = ToSectorId; Step != FromSectorId; Step = CameFrom[Step])
	{
		Route.Insert(Step, 0);
	}
	return Route;
}

bool UGalaxySubsystem::BeginJump(APlayerController* PC, FName TargetSectorId, FString& OutReason)
{
	UWorld* World = PC ? PC->GetWorld() : nullptr;
	if (!World)
	{
		OutReason = TEXT("no player");
		return false;
	}
	if (bJumpPending)
	{
		OutReason = TEXT("a jump is already in progress");
		return false;
	}
	const FGalaxySectorDef* Target = FindSector(TargetSectorId);
	if (!Target)
	{
		OutReason = FString::Printf(TEXT("unknown sector '%s'"), *TargetSectorId.ToString());
		return false;
	}
	if (!Target->HasLevel())
	{
		OutReason = FString::Printf(TEXT("%s has no level yet"), *Target->Name.ToString());
		return false;
	}
	ASpaceship* Ship = Cast<ASpaceship>(PC->GetPawn());
	if (!Ship)
	{
		OutReason = TEXT("not flying a ship");
		return false;
	}
	if (Ship->IsDocked())
	{
		OutReason = TEXT("undock first");
		return false;
	}
	if (const AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC))
	{
		if (AdPC->IsStationEditorOpen())
		{
			OutReason = TEXT("close the Station Editor first");
			return false;
		}
	}
	USaveGameSubsystem* Save = GetGameInstance()->GetSubsystem<USaveGameSubsystem>();
	if (!Save)
	{
		OutReason = TEXT("save system unavailable");
		return false;
	}

	// Snapshot what travels with the player. Stations belong to the sector: remember
	// this sector's player-built ones and hand the target its own (empty on a first visit).
	UAdastreaSaveGame* Snapshot = NewObject<UAdastreaSaveGame>(this);
	Save->CollectGameState(Snapshot);
	Snapshot->SaveVersion = UAdastreaSaveGame::CURRENT_SAVE_VERSION;
	const FName FromSectorId = ResolveCurrentSectorId(World);
	if (!FromSectorId.IsNone())
	{
		SectorStations.Add(FromSectorId, Snapshot->Stations);
	}
	Snapshot->Stations = SectorStations.FindRef(TargetSectorId);
	Snapshot->PlayerShip.bDocked = false;

	bJumpPending = true;
	PendingFromSectorId = FromSectorId;
	PendingToSectorId = TargetSectorId;
	PendingSnapshot = Snapshot;

	UE_LOG(LogAdastrea, Log, TEXT("Galaxy: jump %s -> %s (%s)"),
		*FromSectorId.ToString(), *TargetSectorId.ToString(), *Target->Level);

	if (PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, JumpFadeSeconds, FLinearColor::Black, false, true);
	}
	const FString Level = Target->Level;
	TWeakObjectPtr<UWorld> WeakWorld(World);
	World->GetTimerManager().SetTimer(JumpTimer, FTimerDelegate::CreateWeakLambda(this, [this, Level, WeakWorld]()
	{
		if (UWorld* W = WeakWorld.Get())
		{
			UGameplayStatics::OpenLevel(W, FName(*Level));
		}
		else
		{
			bJumpPending = false;
			PendingSnapshot = nullptr;
		}
	}), JumpFadeSeconds, false);
	return true;
}

bool UGalaxySubsystem::CompletePendingJump(UWorld* World, AJumpGate* ArrivalGate)
{
	if (!bJumpPending || !World)
	{
		return false;
	}
	const FName FromSectorId = PendingFromSectorId;
	const FName ToSectorId = PendingToSectorId;
	UAdastreaSaveGame* Snapshot = PendingSnapshot;
	bJumpPending = false;
	PendingSnapshot = nullptr;

	const FName Here = ResolveCurrentSectorId(World);
	if (Here != ToSectorId)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: jump to '%s' ended in '%s' (level %s); snapshot dropped"),
			*ToSectorId.ToString(), *Here.ToString(), *World->GetMapName());
		return false;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	const FVector ArrivalLoc = ArrivalGate ? ArrivalGate->GetArrivalLocation() : FVector::ZeroVector;
	const FRotator ArrivalRot = ArrivalGate ? ArrivalGate->GetArrivalRotation() : FRotator::ZeroRotator;

	// Some levels' game modes don't give the player a ship (or fail to spawn one):
	// bring the ship we left in, so a jump never strands the player.
	if (PC && Snapshot && !Cast<ASpaceship>(PC->GetPawn()) && Snapshot->PlayerShip.bValid)
	{
		if (UClass* ShipClass = Snapshot->PlayerShip.ShipClass.TryLoadClass<ASpaceship>())
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			const FVector SpawnLoc = ArrivalGate ? ArrivalLoc : (PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector);
			if (ASpaceship* NewShip = World->SpawnActor<ASpaceship>(ShipClass, SpawnLoc, ArrivalRot, Params))
			{
				PC->Possess(NewShip);
				UE_LOG(LogAdastrea, Log, TEXT("Galaxy: level gave the player no ship; spawned %s at the arrival point"), *ShipClass->GetName());
			}
		}
	}

	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	USaveGameSubsystem* Save = GetGameInstance()->GetSubsystem<USaveGameSubsystem>();
	if (!PC || !Pawn || !Save || !Snapshot)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: arrived in '%s' but no player ship to restore onto"), *Here.ToString());
		return false;
	}

	// Come out of the gate that leads back; without one, keep the level's spawn point.
	Snapshot->PlayerLocation = ArrivalGate ? ArrivalLoc : Pawn->GetActorLocation();
	Snapshot->PlayerRotation = ArrivalGate ? ArrivalRot : Pawn->GetActorRotation();
	Save->ApplyGameState(Snapshot);

	if (PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 1.0f, FLinearColor::Black, false, false);
	}
	const FGalaxySectorDef* Sector = FindSector(Here);
	const FStarSystemDef* System = Sector ? FindSystem(Sector->SystemId) : nullptr;
	if (AAdastreaHUD* HUD = Cast<AAdastreaHUD>(PC->GetHUD()))
	{
		HUD->ShowMessage(FString::Printf(TEXT("Arrived: %s - %s system"),
			Sector ? *Sector->Name.ToString() : *Here.ToString(), System ? *System->Name.ToString() : TEXT("?")), 4.0f, false);
	}
	UE_LOG(LogAdastrea, Log, TEXT("Galaxy: jump %s -> %s complete (arrival gate: %s)"),
		*FromSectorId.ToString(), *Here.ToString(), ArrivalGate ? *ArrivalGate->GetName() : TEXT("none"));
	return true;
}
