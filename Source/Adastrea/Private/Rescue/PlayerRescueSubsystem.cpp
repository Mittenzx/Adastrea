#include "Rescue/PlayerRescueSubsystem.h"
#include "Rescue/EscapePod.h"
#include "Rescue/TowDrone.h"
#include "AI/AIMinerController.h"
#include "AI/AIPilotController.h"
#include "AI/CombatPilotController.h"
#include "Combat/ShipHealthComponent.h"
#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "Stations/DockingBayModule.h"
#include "Stations/SpaceStation.h"
#include "Stations/StationInterior.h"
#include "Engine/World.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "AdastreaLog.h"

namespace PlayerRescue
{
	/** Repair in place after this long when there's no friendly station at all (s). */
	constexpr float FallbackRepairSeconds = 5.0f;
	/** The tow drops the wreck this far out from the docking bay (cm), inside docking range. */
	constexpr float DropDistance = 3000.0f;
	/** The pod is taken aboard a station this close to its bay (cm). */
	constexpr float StationArrival = 600.0f;
	/** Extra margin beyond the pickup ship's hull for the pod to be taken aboard (cm). */
	constexpr float ShipArrivalMargin = 400.0f;
	/** Waking up: fade to black, hold, then fade in on the medical bay (s). */
	constexpr float FadeOutSeconds = 1.0f;
	constexpr float BlackSeconds = 0.8f;
	constexpr float FadeInSeconds = 3.0f;
	/** Ask the repaired ship to dock again this often until it's in (s). */
	constexpr float DockRetrySeconds = 3.0f;
}

UPlayerRescueSubsystem* UPlayerRescueSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UPlayerRescueSubsystem>() : nullptr;
}

bool UPlayerRescueSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UPlayerRescueSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPlayerRescueSubsystem, STATGROUP_Tickables);
}

void UPlayerRescueSubsystem::BeginRescue(ASpaceship* InWreck, APlayerController* PC)
{
	if (IsRescueUnderway() || !InWreck || !PC)
	{
		return;
	}
	// Disabling happens mid-damage (inside the projectile step); swap pawns next tick.
	Wreck = InWreck;
	PlayerController = PC;
	bPending = true;
}

void UPlayerRescueSubsystem::Message(const FString& Text, float Seconds, bool bWarning) const
{
	if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(PlayerController.Get()))
	{
		PC->ShowHUDMessage(Text, Seconds, bWarning);
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Rescue: %s"), *Text);
}

ASpaceStation* UPlayerRescueSubsystem::FindNearestFriendlyStation(const FVector& Location) const
{
	ASpaceStation* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (TActorIterator<ASpaceStation> It(GetWorld()); It; ++It)
	{
		if (const ADockingBayModule* Bay = It->GetDockingBayModule())
		{
			const float DistSq = FVector::DistSquared(Location, Bay->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				Best = *It;
			}
		}
	}
	return Best;
}

ASpaceship* UPlayerRescueSubsystem::FindPickupShip(const FVector& Location, float MaxDistance) const
{
	ASpaceship* Best = nullptr;
	float BestDistSq = FMath::Square(MaxDistance);
	for (TActorIterator<ASpaceship> It(GetWorld()); It; ++It)
	{
		ASpaceship* Ship = *It;
		const AAIPilotController* Pilot = Cast<AAIPilotController>(Ship->GetController());
		// Traders in flight only: miners are busy with rocks, fighters (hostiles and
		// patrols) are busy fighting, and a docked ship can't come and fetch anyone.
		if (!Pilot || Pilot->IsA<AAIMinerController>() || Pilot->IsA<ACombatPilotController>()
			|| Ship->IsWrecked() || Ship->IsDocked() || Ship->IsDocking())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Location, Ship->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Ship;
		}
	}
	return Best;
}

void UPlayerRescueSubsystem::Tick(float DeltaTime)
{
	if (bPending)
	{
		bPending = false;
		StartRescue();
		return;
	}
	if (IsRescueUnderway())
	{
		TickRescue(DeltaTime);
	}
}

void UPlayerRescueSubsystem::StartRescue()
{
	ASpaceship* Ship = Wreck.Get();
	APlayerController* PC = PlayerController.Get();
	if (!Ship || !PC)
	{
		Reset();
		return;
	}

	ASpaceStation* Home = FindNearestFriendlyStation(Ship->GetActorLocation());
	ADockingBayModule* Bay = Home ? Home->GetDockingBayModule() : nullptr;
	if (!Bay)
	{
		FallbackClock = 0.0f;
		Message(FString::Printf(TEXT("SHIP DISABLED - no friendly station in range. Emergency repair in %.0f s"), PlayerRescue::FallbackRepairSeconds), PlayerRescue::FallbackRepairSeconds, true);
		return;
	}
	Station = Home;
	const FString StationName = AAIPilotController::GetStationDisplayName(Home);

	UWorld* World = GetWorld();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Tow drone out of the bay; it leaves the wreck just outside, on the side it comes from.
	const FVector BayLoc = Bay->GetActorLocation();
	const FVector Out = (Ship->GetActorLocation() - BayLoc).GetSafeNormal();
	const FVector DropPoint = BayLoc + Out * PlayerRescue::DropDistance;
	if (ATowDrone* Drone = World->SpawnActor<ATowDrone>(ATowDrone::StaticClass(), BayLoc + Out * 600.0f, Out.Rotation(), Params))
	{
		Drone->StartTow(Ship, DropPoint, BayLoc);
		Tow = Drone;
	}

	// Eject: the pilot takes the pod (and the camera with it).
	const FBox Hull = Ship->GetHullLocalBounds();
	const float HullHeight = Hull.IsValid ? Hull.GetExtent().Z : 200.0f;
	const FVector Up = Ship->GetActorUpVector();
	AEscapePod* EscapePod = World->SpawnActor<AEscapePod>(AEscapePod::StaticClass(),
		Ship->GetActorLocation() + Up * (HullHeight + 250.0f), Ship->GetActorRotation(), Params);
	if (!EscapePod)
	{
		Reset();
		return;
	}
	Pod = EscapePod;
	PC->Possess(EscapePod);
	EscapePod->Launch(Ship->GetVelocity(), Up);

	// Pickup: a trader if one is nearer than the station, else the station itself.
	const float StationDist = FVector::Dist(Ship->GetActorLocation(), BayLoc);
	if (ASpaceship* Trader = FindPickupShip(Ship->GetActorLocation(), StationDist))
	{
		PickupShip = Trader;
		EscapePod->SetDestination(Trader, Trader->GetHullHalfLength() + PlayerRescue::ShipArrivalMargin);
		Message(FString::Printf(TEXT("SHIP DISABLED - ejecting. %s is picking you up; tow drone dispatched from %s"),
			*Trader->GetShipName().ToString(), *StationName), 6.0f, true);
	}
	else
	{
		EscapePod->SetDestination(Bay, PlayerRescue::StationArrival);
		Message(FString::Printf(TEXT("SHIP DISABLED - ejecting to %s; tow drone on its way"), *StationName), 6.0f, true);
	}
}

void UPlayerRescueSubsystem::TickRescue(float DeltaTime)
{
	ASpaceship* Ship = Wreck.Get();
	APlayerController* PC = PlayerController.Get();

	// No station anywhere: patch the ship up where it is.
	if (FallbackClock >= 0.0f)
	{
		FallbackClock += DeltaTime;
		if (FallbackClock >= PlayerRescue::FallbackRepairSeconds)
		{
			if (Ship && Ship->HealthComponent)
			{
				Ship->HealthComponent->Restore();
			}
			Message(TEXT("Emergency repairs complete"), 2.0f);
			Reset();
		}
		return;
	}

	ASpaceStation* Home = Station.Get();
	ADockingBayModule* Bay = Home ? Home->GetDockingBayModule() : nullptr;
	AEscapePod* EscapePod = Pod.Get();
	const bool bInPod = !bAwake && WakeClock < 0.0f;
	if (!Ship || !PC || !Bay || (bInPod && !EscapePod))
	{
		UE_LOG(LogAdastreaCombat, Warning, TEXT("Rescue: lost track (%s); giving up"), *GetStatus());
		Reset();
		return;
	}
	const FString StationName = AAIPilotController::GetStationDisplayName(Home);

	if (bInPod)
	{
		// Heading for a trader that was disabled first: make for the station instead.
		ASpaceship* Trader = PickupShip.Get();
		if (EscapePod->GetDestination() != Bay && (!Trader || Trader->IsWrecked() || !Trader->GetController()))
		{
			PickupShip = nullptr;
			EscapePod->SetDestination(Bay, PlayerRescue::StationArrival);
			Message(TEXT("Your ride was disabled - the pod is heading for the station"), 4.0f, true);
		}
		else if (EscapePod->HasArrived())
		{
			if (Trader && EscapePod->GetDestination() == Trader)
			{
				if (AAIPilotController* Pilot = Cast<AAIPilotController>(Trader->GetController()))
				{
					bFerryOrdered = Pilot->FerryTo(Home);
				}
			}
			else
			{
				bPilotAtStation = true;
			}
			BeginWake();
		}
	}
	else if (WakeClock >= 0.0f)
	{
		WakeClock += DeltaTime;
		if (WakeClock >= PlayerRescue::FadeOutSeconds + PlayerRescue::BlackSeconds)
		{
			WakeClock = -1.0f;
			WakeInMedicalBay();
		}
	}
	else if (!bPilotAtStation)
	{
		// Awake in a trader's sick bay: ashore once it docks at the tow's station.
		ASpaceship* Trader = PickupShip.Get();
		if (!Trader || Trader->IsWrecked() || !Trader->GetController())
		{
			bPilotAtStation = true;
			Message(FString::Printf(TEXT("Your ride was disabled - a station shuttle brought you to %s"), *StationName), 5.0f, true);
		}
		else if (Trader->IsDocked() && Trader->GetDockedStation() == Home)
		{
			bPilotAtStation = true;
			Message(FString::Printf(TEXT("%s has docked at %s - you can go ashore"), *Trader->GetShipName().ToString(), *StationName), 5.0f);
		}
		else if (!bFerryOrdered)
		{
			if (AAIPilotController* Pilot = Cast<AAIPilotController>(Trader->GetController()))
			{
				bFerryOrdered = Pilot->FerryTo(Home);
			}
		}
	}

	// The ship: towed in, repaired, docked with nobody aboard.
	if (!bShipSentToDock)
	{
		ATowDrone* Drone = Tow.Get();
		if (!Drone)
		{
			// Lost the tow: the station brings it in another way.
			UE_LOG(LogAdastreaCombat, Warning, TEXT("Rescue: tow drone lost; moving the wreck to the bay"));
			const FVector Out = (Ship->GetActorLocation() - Bay->GetActorLocation()).GetSafeNormal();
			Ship->SetActorLocation(Bay->GetActorLocation() + Out * PlayerRescue::DropDistance);
			DockTowedShip();
		}
		else if (Drone->HasDelivered())
		{
			DockTowedShip();
		}
	}
	else if (!Ship->IsDocked() && !Ship->IsDocking())
	{
		DockRetryClock += DeltaTime;
		if (DockRetryClock >= PlayerRescue::DockRetrySeconds)
		{
			DockRetryClock = 0.0f;
			Ship->SetNearbyStation(Bay);
			Ship->RequestDocking();
		}
	}

	// Done once the pilot is at the station and the ship is docked there.
	if (bAwake && bPilotAtStation && bShipSentToDock && Ship->IsDocked())
	{
		const AAdastreaPlayerController* APC = Cast<AAdastreaPlayerController>(PC);
		if (APC && APC->IsWalkingStation())
		{
			Message(FString::Printf(TEXT("Your ship is docked at %s, repaired free of charge. Board it at the airlock."), *StationName), 6.0f);
			Reset();
		}
		else
		{
			ReuniteInShip();
		}
	}
}

FString UPlayerRescueSubsystem::GetTransitBlockReason() const
{
	if (!IsRescueUnderway() || !bAwake || bPilotAtStation)
	{
		return FString();
	}
	const ASpaceship* Trader = PickupShip.Get();
	return FString::Printf(TEXT("Still in flight aboard %s - wait until it docks"),
		Trader ? *Trader->GetShipName().ToString() : TEXT("the trader"));
}

void UPlayerRescueSubsystem::BeginWake()
{
	WakeClock = 0.0f;
	if (APlayerController* PC = PlayerController.Get(); PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, PlayerRescue::FadeOutSeconds, FLinearColor::Black, false, true);
	}
}

void UPlayerRescueSubsystem::WakeInMedicalBay()
{
	ASpaceship* Ship = Wreck.Get();
	APlayerController* PC = PlayerController.Get();
	AAdastreaPlayerController* APC = Cast<AAdastreaPlayerController>(PC);
	bAwake = true;
	if (APC && Ship)
	{
		APC->EnterStationRoom(Ship, EStationRoom::Medical);
	}
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, PlayerRescue::FadeInSeconds, FLinearColor::Black);
	}
	if (!APC || !APC->IsWalkingStation())
	{
		// No interior to wake in: stay with the pod; the pilot re-boards the ship when it docks.
		UE_LOG(LogAdastreaCombat, Warning, TEXT("Rescue: couldn't open the medical bay; the pilot waits in the pod"));
		return;
	}
	if (AEscapePod* EscapePod = Pod.Get())
	{
		EscapePod->Destroy();
	}

	const FString StationName = AAIPilotController::GetStationDisplayName(Station.Get());
	if (const ASpaceship* Trader = PickupShip.Get(); Trader && !bPilotAtStation)
	{
		Message(FString::Printf(TEXT("You wake in the sick bay aboard %s. The crew have checked you over; they're taking you to %s."),
			*Trader->GetShipName().ToString(), *StationName), 7.0f);
	}
	else
	{
		Message(FString::Printf(TEXT("You wake in the medical bay of %s. The medics have checked you over - your ship is being towed in."),
			*StationName), 7.0f);
	}
}

void UPlayerRescueSubsystem::DockTowedShip()
{
	ASpaceship* Ship = Wreck.Get();
	ASpaceStation* Home = Station.Get();
	ADockingBayModule* Bay = Home ? Home->GetDockingBayModule() : nullptr;
	if (ATowDrone* Drone = Tow.Get())
	{
		Drone->Release();
	}
	if (Ship->HealthComponent)
	{
		Ship->HealthComponent->Restore();   // free repair; clears the wreck state
	}
	bShipSentToDock = true;
	DockRetryClock = 0.0f;
	if (Bay)
	{
		Ship->SetNearbyStation(Bay);
		Ship->RequestDocking();
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Rescue: %s towed in and repaired; docking"), *Ship->GetName());
}

void UPlayerRescueSubsystem::ReuniteInShip()
{
	ASpaceship* Ship = Wreck.Get();
	APlayerController* PC = PlayerController.Get();
	PC->Possess(Ship);
	if (AEscapePod* EscapePod = Pod.Get())
	{
		EscapePod->Destroy();
	}
	Message(FString::Printf(TEXT("%s repaired your ship free of charge"), *AAIPilotController::GetStationDisplayName(Station.Get())), 5.0f);
	Reset();
}

void UPlayerRescueSubsystem::Reset()
{
	Wreck = nullptr;
	PlayerController = nullptr;
	Station = nullptr;
	Tow = nullptr;
	Pod = nullptr;
	PickupShip = nullptr;
	bPending = false;
	bFerryOrdered = false;
	bPilotAtStation = false;
	WakeClock = -1.0f;
	bAwake = false;
	bShipSentToDock = false;
	DockRetryClock = 0.0f;
	FallbackClock = -1.0f;
}

FString UPlayerRescueSubsystem::GetStatus() const
{
	if (!IsRescueUnderway())
	{
		return TEXT("idle");
	}
	if (FallbackClock >= 0.0f)
	{
		return FString::Printf(TEXT("emergency repair %.1fs"), FallbackClock);
	}
	const ATowDrone* Drone = Tow.Get();
	const AEscapePod* EscapePod = Pod.Get();
	return FString::Printf(TEXT("tow=%s pod=%s pickup=%s ferry=%d awake=%d atStation=%d shipSent=%d"),
		Drone ? *UEnum::GetValueAsString(Drone->GetTowState()) : TEXT("none"),
		EscapePod ? *UEnum::GetValueAsString(EscapePod->GetPodState()) : TEXT("none"),
		*GetNameSafe(PickupShip.Get()), bFerryOrdered, bAwake, bPilotAtStation, bShipSentToDock);
}
