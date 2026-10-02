// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/ClaudeDriverSubsystem.h"
#include "AdastreaLog.h"
#include "AI/AIPilotController.h"
#include "Combat/CombatTeams.h"
#include "Combat/ShipHealthComponent.h"
#include "Combat/ShipWeaponComponent.h"
#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Stations/SpaceStation.h"
#include "Stations/DockingBayModule.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"
#include "Misc/OutputDevice.h"
#include "Misc/ScopeLock.h"
#include "UnrealClient.h"
#include "HighResScreenshot.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

namespace ClaudeDriver
{
	constexpr int32 MaxHistory = 20;
	/** Default per-command time limits (seconds). */
	constexpr float FlyTimeout = 300.0f;
	constexpr float FaceTimeout = 20.0f;
	constexpr float StopTimeout = 30.0f;
	constexpr float DockTimeout = 180.0f;
	constexpr float AttackTimeout = 60.0f;
	/** Fire only when the lead point is this close to the nose (degrees). */
	constexpr float FireConeDeg = 4.0f;

	static FAutoConsoleCommandWithWorldAndArgs RunCmd(
		TEXT("claude.Run"),
		TEXT("claude.Run <commands>: queue ';'-separated driver commands (see UClaudeDriverSubsystem)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UClaudeDriverSubsystem* Driver = World ? World->GetSubsystem<UClaudeDriverSubsystem>() : nullptr)
			{
				Driver->Run(FString::Join(Args, TEXT(" ")));
			}
		}));

	static FAutoConsoleCommandWithWorld CancelCmd(
		TEXT("claude.Cancel"),
		TEXT("Drop the driver's current command and queue."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UClaudeDriverSubsystem* Driver = World ? World->GetSubsystem<UClaudeDriverSubsystem>() : nullptr)
			{
				Driver->Cancel();
			}
		}));

	static FAutoConsoleCommandWithWorld StateCmd(
		TEXT("claude.State"),
		TEXT("Log the driver's state snapshot as one 'CLAUDE STATE {json}' line."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UClaudeDriverSubsystem* Driver = World ? World->GetSubsystem<UClaudeDriverSubsystem>() : nullptr)
			{
				UE_LOG(LogAdastrea, Log, TEXT("CLAUDE STATE %s"), *Driver->GetStateJson());
			}
		}));

	bool IsHostileShip(const ASpaceship* Ship)
	{
		return Ship && !Ship->IsWrecked() && Ship->HealthComponent && Ship->HealthComponent->Team == CombatTeam::Hostile;
	}

	FString DisplayName(const AActor* Actor)
	{
		if (const ASpaceStation* Station = Cast<ASpaceStation>(Actor))
		{
			return AAIPilotController::GetStationDisplayName(Station);
		}
		if (const ASpaceship* Ship = Cast<ASpaceship>(Actor))
		{
			if (Ship->ShipDataAsset && !Ship->ShipDataAsset->ShipName.IsEmpty())
			{
				return Ship->ShipDataAsset->ShipName.ToString();
			}
		}
		return Actor ? Actor->GetName() : FString();
	}

	/** Angle in degrees between the ship's nose and the direction to Location. */
	float AngleOffNose(const AActor* Ship, const FVector& Location)
	{
		const FVector To = (Location - Ship->GetActorLocation()).GetSafeNormal();
		if (To.IsNearlyZero())
		{
			return 0.0f;
		}
		return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Ship->GetActorForwardVector(), To), -1.0f, 1.0f)));
	}

	TArray<TSharedPtr<FJsonValue>> VectorJson(const FVector& V, float Scale)
	{
		return {
			MakeShared<FJsonValueNumber>(FMath::RoundToDouble(V.X * Scale)),
			MakeShared<FJsonValueNumber>(FMath::RoundToDouble(V.Y * Scale)),
			MakeShared<FJsonValueNumber>(FMath::RoundToDouble(V.Z * Scale))
		};
	}

	double Round1(double V)
	{
		return FMath::RoundToDouble(V * 10.0) / 10.0;
	}
}

/** Log listener: counts errors and warnings and keeps the latest few lines. Called from any thread. */
class FClaudeLogCapture : public FOutputDevice
{
public:
	static constexpr int32 MaxRecent = 8;

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
	{
		const ELogVerbosity::Type Level = ELogVerbosity::Type(Verbosity & ELogVerbosity::VerbosityMask);
		if (Level > ELogVerbosity::Warning)
		{
			return;
		}
		FScopeLock Lock(&Mutex);
		(Level == ELogVerbosity::Warning ? Warnings : Errors)++;
		Recent.Add(FString::Printf(TEXT("%s %s: %s"), Level == ELogVerbosity::Warning ? TEXT("W") : TEXT("E"), *Category.ToString(), V).Left(300));
		if (Recent.Num() > MaxRecent)
		{
			Recent.RemoveAt(0);
		}
	}

	virtual bool CanBeUsedOnAnyThread() const override { return true; }
	virtual bool CanBeUsedOnMultipleThreads() const override { return true; }

	void Read(int32& OutErrors, int32& OutWarnings, TArray<FString>& OutRecent) const
	{
		FScopeLock Lock(&Mutex);
		OutErrors = Errors;
		OutWarnings = Warnings;
		OutRecent = Recent;
	}

private:
	mutable FCriticalSection Mutex;
	int32 Errors = 0;
	int32 Warnings = 0;
	TArray<FString> Recent;
};

// ---------------------------------------------------------------------------
// Subsystem
// ---------------------------------------------------------------------------

UClaudeDriverSubsystem* UClaudeDriverSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UClaudeDriverSubsystem>() : nullptr;
}

bool UClaudeDriverSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return Super::ShouldCreateSubsystem(Outer);
#endif
}

bool UClaudeDriverSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UClaudeDriverSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LogCapture = MakeShared<FClaudeLogCapture>();
	GLog->AddOutputDevice(LogCapture.Get());
}

void UClaudeDriverSubsystem::Deinitialize()
{
	Cancel();
	if (LogCapture)
	{
		GLog->RemoveOutputDevice(LogCapture.Get());
		LogCapture.Reset();
	}
	Super::Deinitialize();
}

TStatId UClaudeDriverSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UClaudeDriverSubsystem, STATGROUP_Tickables);
}

AAdastreaPlayerController* UClaudeDriverSubsystem::GetPC() const
{
	UWorld* World = GetWorld();
	return World ? Cast<AAdastreaPlayerController>(World->GetFirstPlayerController()) : nullptr;
}

ASpaceship* UClaudeDriverSubsystem::GetShip() const
{
	const AAdastreaPlayerController* PC = GetPC();
	return PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
}

void UClaudeDriverSubsystem::Run(const FString& Script)
{
	TArray<FString> Parts;
	Script.ParseIntoArray(Parts, TEXT(";"), true);
	for (const FString& Part : Parts)
	{
		FCommand Command;
		if (ParseCommand(Part.TrimStartAndEnd(), Command))
		{
			Queue.Add(MoveTemp(Command));
		}
	}
	UE_LOG(LogAdastrea, Log, TEXT("CLAUDE queued '%s' (%d waiting)"), *Script, Queue.Num());
}

void UClaudeDriverSubsystem::Cancel()
{
	if (bHasCurrent)
	{
		Finish(false, TEXT("cancelled"));
	}
	Queue.Reset();
	SetFireHeld(false);
}

bool UClaudeDriverSubsystem::ParseCommand(const FString& Text, FCommand& Out)
{
	if (Text.IsEmpty())
	{
		return false;
	}
	Out.Text = Text;
	Text.ParseIntoArrayWS(Out.Args);
	Out.Verb = Out.Args[0].ToLower();
	Out.Args.RemoveAt(0);
	Out.Rest = Text.Mid(Out.Verb.Len()).TrimStartAndEnd();
	return true;
}

void UClaudeDriverSubsystem::Tick(float DeltaTime)
{
	// Held fire is re-injected every frame; the action completes the frame we stop.
	if (bFireHeld)
	{
		ASpaceship* Ship = GetShip();
		AAdastreaPlayerController* PC = GetPC();
		UEnhancedInputLocalPlayerSubsystem* Input = PC ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()) : nullptr;
		if (Ship && Ship->FireAction && Input)
		{
			Input->InjectInputForAction(Ship->FireAction, FInputActionValue(true), {}, {});
		}
		else if (Ship)
		{
			Ship->FireStarted();
		}
	}

	if (!bHasCurrent)
	{
		if (Queue.Num() == 0)
		{
			return;
		}
		Current = Queue[0];
		Queue.RemoveAt(0);
		StartCurrent();
		if (!bHasCurrent)
		{
			return; // finished on start (instant command or bad arguments)
		}
	}

	Elapsed += DeltaTime;
	TickCurrent(DeltaTime);
}

void UClaudeDriverSubsystem::Finish(bool bOk, const FString& Note)
{
	FResult Result;
	Result.Text = Current.Text;
	Result.bOk = bOk;
	Result.Note = Note;
	Result.Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	History.Add(Result);
	if (History.Num() > ClaudeDriver::MaxHistory)
	{
		History.RemoveAt(0);
	}

	UE_LOG(LogAdastrea, Log, TEXT("CLAUDE done %s %s %s"), *Current.Text, bOk ? TEXT("ok") : TEXT("FAIL"), *Note);

	// Commands that hold inputs let go when they end.
	if (Current.Verb == TEXT("fire") || Current.Verb == TEXT("attack"))
	{
		SetFireHeld(false);
	}
	bHasCurrent = false;
}

void UClaudeDriverSubsystem::SetFireHeld(bool bHeld)
{
	if (bFireHeld == bHeld)
	{
		return;
	}
	bFireHeld = bHeld;
	if (!bHeld)
	{
		// Injected fire completes by itself next frame; the direct fallback needs releasing.
		if (ASpaceship* Ship = GetShip())
		{
			Ship->FireStopped();
		}
	}
}

bool UClaudeDriverSubsystem::ResolveRef(const FString& Ref, TWeakObjectPtr<AActor>& OutActor, FVector& OutLocation) const
{
	OutActor = nullptr;
	UWorld* World = GetWorld();
	const ASpaceship* Own = GetShip();
	const FVector From = Own ? Own->GetActorLocation() : FVector::ZeroVector;
	if (!World || Ref.IsEmpty())
	{
		return false;
	}

	// "x,y,z" in metres
	TArray<FString> Coords;
	if (Ref.ParseIntoArray(Coords, TEXT(","), true) == 3 && Coords[0].IsNumeric() && Coords[1].IsNumeric() && Coords[2].IsNumeric())
	{
		OutLocation = FVector(FCString::Atod(*Coords[0]), FCString::Atod(*Coords[1]), FCString::Atod(*Coords[2])) * 100.0;
		return true;
	}

	AActor* Best = nullptr;
	double BestDistSq = TNumericLimits<double>::Max();
	auto Consider = [&](AActor* Candidate)
	{
		const double DistSq = FVector::DistSquared(From, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			Best = Candidate;
			BestDistSq = DistSq;
		}
	};

	const FString Lower = Ref.ToLower();
	if (Lower == TEXT("target"))
	{
		const AAdastreaPlayerController* PC = GetPC();
		Best = PC ? PC->GetLockedTarget() : nullptr;
	}
	else if (Lower == TEXT("hostile"))
	{
		for (TActorIterator<ASpaceship> It(World); It; ++It)
		{
			if (*It != Own && ClaudeDriver::IsHostileShip(*It))
			{
				Consider(*It);
			}
		}
	}
	else if (Lower == TEXT("station"))
	{
		for (TActorIterator<ASpaceStation> It(World); It; ++It)
		{
			Consider(*It);
		}
	}
	else
	{
		for (TActorIterator<ASpaceship> It(World); It; ++It)
		{
			if (*It != Own && (It->GetName().Contains(Ref) || ClaudeDriver::DisplayName(*It).Contains(Ref)))
			{
				Consider(*It);
			}
		}
		for (TActorIterator<ASpaceStation> It(World); It; ++It)
		{
			if (It->GetName().Contains(Ref) || ClaudeDriver::DisplayName(*It).Contains(Ref))
			{
				Consider(*It);
			}
		}
		if (!Best)
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (*It != Own && It->GetName().Contains(Ref))
				{
					Consider(*It);
				}
			}
		}
	}

	if (!Best)
	{
		return false;
	}
	OutActor = Best;
	OutLocation = Best->GetActorLocation();
	return true;
}

bool UClaudeDriverSubsystem::GetRefLocation(FVector& OutLocation) const
{
	if (RefActor.IsExplicitlyNull())
	{
		OutLocation = RefLocation;
		return true;
	}
	if (const AActor* Actor = RefActor.Get())
	{
		OutLocation = Actor->GetActorLocation();
		return true;
	}
	return false;
}

float UClaudeDriverSubsystem::FaceToward(ASpaceship* Ship, const FVector& Location, float DeltaTime) const
{
	const FVector ToTarget = Location - Ship->GetActorLocation();
	if (ToTarget.IsNearlyZero())
	{
		return 0.0f;
	}
	FRotator Desired = ToTarget.Rotation();
	Desired.Roll = 0.0f;
	const float Rate = Ship->TurnRate * (Ship->ShipDataAsset ? Ship->ShipDataAsset->RotationRateMultiplier : 1.0f);
	const FRotator NewRotation = FMath::RInterpConstantTo(Ship->GetActorRotation(), Desired, DeltaTime, Rate);
	Ship->SetActorRotation(NewRotation);
	// Keep the control rotation in step, so nothing that follows the controller turns the ship back.
	if (AController* Controller = Ship->GetController())
	{
		Controller->SetControlRotation(NewRotation);
	}
	return ClaudeDriver::AngleOffNose(Ship, Location);
}

// ---------------------------------------------------------------------------
// Commands
// ---------------------------------------------------------------------------

void UClaudeDriverSubsystem::StartCurrent()
{
	bHasCurrent = true;
	Elapsed = 0.0f;
	bPhase = false;
	Counter = 0.0f;
	RefActor = nullptr;
	RefLocation = FVector::ZeroVector;

	const FString& Verb = Current.Verb;
	const TArray<FString>& Args = Current.Args;
	UE_LOG(LogAdastrea, Log, TEXT("CLAUDE start %s"), *Current.Text);

	auto ArgFloat = [&Args](int32 Index, float Default)
	{
		return Args.IsValidIndex(Index) ? FCString::Atof(*Args[Index]) : Default;
	};

	if (Verb == TEXT("wait"))
	{
		Counter = ArgFloat(0, 1.0f);
		return;
	}
	if (Verb == TEXT("cmd"))
	{
		AAdastreaPlayerController* PC = GetPC();
		if (!PC || Current.Rest.IsEmpty())
		{
			Finish(false, TEXT("no player controller or empty command"));
			return;
		}
		PC->ConsoleCommand(Current.Rest);
		Finish(true, TEXT(""));
		return;
	}
	if (Verb == TEXT("shot"))
	{
		const FString Name = Args.Num() > 0 ? Args[0] : FString::Printf(TEXT("shot_%.0f"), GetWorld()->GetTimeSeconds());
		const FString Path = FPaths::ConvertRelativePathToFull(FPaths::ScreenShotDir() / TEXT("Claude") / Name + TEXT(".png"));
		// Shoot the game's own viewport: a plain screenshot request is taken by whichever
		// viewport draws next, which in PIE is often the level editor's.
		UGameViewportClient* GameViewport = GetWorld()->GetGameViewport();
		FViewport* Viewport = GameViewport ? GameViewport->Viewport : nullptr;
		if (!Viewport)
		{
			Finish(false, TEXT("no game viewport"));
			return;
		}
		FHighResScreenshotConfig& Config = GetHighResScreenshotConfig();
		Config.SetResolution(Viewport->GetSizeXY().X, Viewport->GetSizeXY().Y);
		Config.SetFilename(Path);
		Config.bMaskEnabled = false;
		Config.SetHDRCapture(false);
		Viewport->TakeHighResScreenShot();
		Finish(true, Path);
		return;
	}

	// Everything else flies the ship.
	ASpaceship* Ship = GetShip();
	if (!Ship)
	{
		Finish(false, TEXT("not flying a ship"));
		return;
	}

	if (Verb == TEXT("throttle"))
	{
		Ship->SetThrottle(FMath::Clamp(ArgFloat(0, 0.0f), 0.0f, 100.0f));
		Finish(true, FString::Printf(TEXT("throttle %.0f"), Ship->ThrottlePercentage));
		return;
	}
	if (Verb == TEXT("stop"))
	{
		Ship->SetThrottle(0.0f);
		return;
	}
	if (Verb == TEXT("undock"))
	{
		if (!Ship->IsDocked())
		{
			Finish(false, TEXT("not docked"));
			return;
		}
		Ship->Undock();
		return;
	}
	if (Verb == TEXT("fire"))
	{
		Counter = ArgFloat(0, 1.0f);
		SetFireHeld(true);
		return;
	}

	// The rest take a <ref>.
	const FString Ref = Args.Num() > 0 ? Args[0] : (Verb == TEXT("dock") ? FString(TEXT("station")) : FString());
	if (!ResolveRef(Ref, RefActor, RefLocation))
	{
		Finish(false, FString::Printf(TEXT("nothing matches '%s'"), *Ref));
		return;
	}

	if (Verb == TEXT("lock") || Verb == TEXT("attack"))
	{
		AAdastreaPlayerController* PC = GetPC();
		AActor* Target = RefActor.Get();
		if (!Target || !PC)
		{
			Finish(false, TEXT("needs an actor"));
			return;
		}
		PC->LockTarget(Target->GetName());
		const bool bLocked = PC->GetLockedTarget() == Target;
		if (Verb == TEXT("lock"))
		{
			Finish(bLocked, bLocked ? Target->GetName() : FString::Printf(TEXT("%s is not a lockable target in range"), *Target->GetName()));
		}
		else if (!bLocked)
		{
			UE_LOG(LogAdastrea, Log, TEXT("CLAUDE attack: %s isn't locked (out of lock range?); chasing it anyway"), *Target->GetName());
		}
		return;
	}
	if (Verb == TEXT("dock"))
	{
		ASpaceStation* Station = Cast<ASpaceStation>(RefActor.Get());
		ADockingBayModule* Bay = Station ? Station->GetDockingBayModule() : nullptr;
		if (!Bay)
		{
			Finish(false, TEXT("not a station with a docking bay"));
			return;
		}
		RefActor = Bay;
		return;
	}
	if (Verb != TEXT("fly") && Verb != TEXT("face"))
	{
		Finish(false, FString::Printf(TEXT("unknown command '%s'"), *Verb));
	}
}

bool UClaudeDriverSubsystem::TickCurrent(float DeltaTime)
{
	const FString& Verb = Current.Verb;
	const TArray<FString>& Args = Current.Args;
	auto ArgFloat = [&Args](int32 Index, float Default)
	{
		return Args.IsValidIndex(Index) ? FCString::Atof(*Args[Index]) : Default;
	};
	auto Done = [this](bool bOk, const FString& Note)
	{
		Finish(bOk, Note);
		return true;
	};

	if (Verb == TEXT("wait"))
	{
		return Elapsed >= Counter ? Done(true, TEXT("")) : false;
	}

	ASpaceship* Ship = GetShip();
	if (!Ship)
	{
		return Done(false, TEXT("no longer flying a ship"));
	}
	if (Ship->IsWrecked())
	{
		return Done(false, TEXT("our ship is a wreck"));
	}

	if (Verb == TEXT("fire"))
	{
		return Elapsed >= Counter ? Done(true, TEXT("")) : false;
	}
	if (Verb == TEXT("stop"))
	{
		if (Ship->IsStationary(100.0f))
		{
			return Done(true, TEXT(""));
		}
		return Elapsed > ClaudeDriver::StopTimeout ? Done(false, FString::Printf(TEXT("still at %.0f m/s"), Ship->GetCurrentSpeed() / 100.0f)) : false;
	}
	if (Verb == TEXT("undock"))
	{
		return !Ship->IsDocked() ? Done(true, TEXT("")) : (Elapsed > 10.0f ? Done(false, TEXT("still docked")) : false);
	}

	FVector Location;
	if (!GetRefLocation(Location))
	{
		return Done(false, TEXT("target is gone"));
	}
	const float Distance = FVector::Dist(Ship->GetActorLocation(), Location);

	if (Verb == TEXT("face"))
	{
		const float Tolerance = ArgFloat(1, 3.0f);
		const float Angle = FaceToward(Ship, Location, DeltaTime);
		if (Angle <= Tolerance)
		{
			return Done(true, FString::Printf(TEXT("%.1f deg off"), Angle));
		}
		return Elapsed > ClaudeDriver::FaceTimeout ? Done(false, FString::Printf(TEXT("still %.0f deg off"), Angle)) : false;
	}

	if (Verb == TEXT("fly"))
	{
		const float Arrive = ArgFloat(1, 500.0f) * 100.0f;
		if (Distance <= Arrive)
		{
			Ship->SetThrottle(0.0f);
			return Done(true, FString::Printf(TEXT("arrived, %.0f m out"), Distance / 100.0f));
		}
		const float Angle = FaceToward(Ship, Location, DeltaTime);
		const float TurnFactor = Angle < 20.0f ? 1.0f : 0.3f;
		const float ApproachFactor = FMath::Clamp((Distance - Arrive) / FMath::Max(Arrive * 4.0f, 50000.0f), 0.15f, 1.0f);
		Ship->SetThrottle(100.0f * TurnFactor * ApproachFactor);
		return Elapsed > ClaudeDriver::FlyTimeout ? Done(false, FString::Printf(TEXT("timed out %.0f m away"), Distance / 100.0f)) : false;
	}

	if (Verb == TEXT("dock"))
	{
		if (Ship->IsDocked())
		{
			return Done(true, TEXT("docked"));
		}
		if (!bPhase)
		{
			float DockDistance = 0.0f;
			FString StationName;
			if (Ship->CanRequestDocking(DockDistance, StationName))
			{
				Ship->SetThrottle(0.0f);
				Ship->RequestDocking();
				bPhase = true;
				UE_LOG(LogAdastrea, Log, TEXT("CLAUDE dock: requested docking at %s (%.0f m)"), *StationName, DockDistance / 100.0f);
			}
			else
			{
				const float Angle = FaceToward(Ship, Location, DeltaTime);
				const float TurnFactor = Angle < 20.0f ? 1.0f : 0.3f;
				Ship->SetThrottle(100.0f * TurnFactor * FMath::Clamp(Distance / 200000.0f, 0.15f, 1.0f));
			}
		}
		return Elapsed > ClaudeDriver::DockTimeout ? Done(false, bPhase ? TEXT("requested but never docked") : TEXT("never got in docking range")) : false;
	}

	if (Verb == TEXT("attack"))
	{
		const ASpaceship* TargetShip = Cast<ASpaceship>(RefActor.Get());
		if (TargetShip && TargetShip->IsWrecked())
		{
			Ship->SetThrottle(0.0f);
			return Done(true, FString::Printf(TEXT("%s disabled after %.1f s"), *TargetShip->GetName(), Elapsed));
		}
		if (Elapsed > ArgFloat(1, ClaudeDriver::AttackTimeout))
		{
			return Done(false, FString::Printf(TEXT("timed out, %.0f m away"), Distance / 100.0f));
		}

		FVector AimPoint = Location;
		const UShipWeaponComponent* Weapons = Ship->WeaponComponent;
		if (Weapons && !Weapons->GetLeadPoint(RefActor.Get(), AimPoint))
		{
			AimPoint = Location;
		}
		const float Range = Weapons ? Weapons->Range : 100000.0f;
		const float Angle = FaceToward(Ship, AimPoint, DeltaTime);

		// Close to the middle of weapon range, then hold off.
		float Throttle = 40.0f;
		if (Distance > Range * 0.6f)
		{
			Throttle = Angle < 20.0f ? 100.0f : 30.0f;
		}
		else if (Distance < Range * 0.25f)
		{
			Throttle = 0.0f;
		}
		Ship->SetThrottle(Throttle);
		SetFireHeld(Angle <= ClaudeDriver::FireConeDeg && Distance <= Range);
		return false;
	}

	return Done(false, FString::Printf(TEXT("unknown command '%s'"), *Verb));
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

FString UClaudeDriverSubsystem::GetStateJson() const
{
	using namespace ClaudeDriver;
	UWorld* World = GetWorld();
	const AAdastreaPlayerController* PC = GetPC();
	const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	const ASpaceship* Ship = Cast<ASpaceship>(Pawn);

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("time"), Round1(World ? World->GetTimeSeconds() : 0.0));
	Root->SetStringField(TEXT("map"), World ? World->GetMapName() : FString());
	Root->SetStringField(TEXT("pawn"), Pawn ? Pawn->GetClass()->GetName() : FString());
	Root->SetStringField(TEXT("mode"), Ship ? TEXT("ship") : (Pawn ? TEXT("on_foot") : TEXT("none")));

	auto HullFields = [](TSharedRef<FJsonObject> Obj, const ASpaceship* S)
	{
		Obj->SetNumberField(TEXT("hull"), FMath::RoundToDouble(S->GetCurrentHullIntegrity()));
		Obj->SetNumberField(TEXT("hull_max"), FMath::RoundToDouble(S->GetMaxHullIntegrity()));
		if (S->HealthComponent)
		{
			Obj->SetNumberField(TEXT("shield"), FMath::RoundToDouble(S->HealthComponent->GetShield()));
			Obj->SetNumberField(TEXT("shield_max"), FMath::RoundToDouble(S->HealthComponent->GetMaxShield()));
			Obj->SetNumberField(TEXT("team"), S->HealthComponent->Team);
		}
		Obj->SetBoolField(TEXT("wrecked"), S->IsWrecked());
	};

	if (Ship)
	{
		TSharedRef<FJsonObject> ShipObj = MakeShared<FJsonObject>();
		ShipObj->SetStringField(TEXT("name"), Ship->GetName());
		ShipObj->SetStringField(TEXT("display"), DisplayName(Ship));
		ShipObj->SetArrayField(TEXT("loc_m"), VectorJson(Ship->GetActorLocation(), 0.01f));
		ShipObj->SetArrayField(TEXT("rot"), VectorJson(FVector(Ship->GetActorRotation().Pitch, Ship->GetActorRotation().Yaw, Ship->GetActorRotation().Roll), 1.0f));
		ShipObj->SetNumberField(TEXT("speed_mps"), Round1(Ship->GetCurrentSpeed() / 100.0f));
		ShipObj->SetNumberField(TEXT("throttle"), FMath::RoundToDouble(Ship->ThrottlePercentage));
		ShipObj->SetBoolField(TEXT("docked"), Ship->IsDocked());
		ShipObj->SetBoolField(TEXT("docking"), Ship->IsDocking());
		ShipObj->SetBoolField(TEXT("firing"), Ship->WeaponComponent && Ship->WeaponComponent->IsTriggerHeld());
		HullFields(ShipObj, Ship);
		Root->SetObjectField(TEXT("ship"), ShipObj);
	}
	else if (Pawn)
	{
		Root->SetArrayField(TEXT("pawn_loc_m"), VectorJson(Pawn->GetActorLocation(), 0.01f));
	}

	const AActor* Eye = Ship ? static_cast<const AActor*>(Ship) : Pawn;
	auto ContactJson = [&](const AActor* Actor) -> TSharedRef<FJsonObject>
	{
		TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
		Obj->SetStringField(TEXT("name"), Actor->GetName());
		Obj->SetStringField(TEXT("display"), DisplayName(Actor));
		Obj->SetStringField(TEXT("kind"), Actor->IsA<ASpaceStation>() ? TEXT("station") : (Actor->IsA<ASpaceship>() ? TEXT("ship") : TEXT("other")));
		if (Eye)
		{
			Obj->SetNumberField(TEXT("dist_m"), FMath::RoundToDouble(FVector::Dist(Eye->GetActorLocation(), Actor->GetActorLocation()) / 100.0));
			Obj->SetNumberField(TEXT("off_nose_deg"), FMath::RoundToDouble(AngleOffNose(Eye, Actor->GetActorLocation())));
		}
		if (const ASpaceship* Other = Cast<ASpaceship>(Actor))
		{
			HullFields(Obj, Other);
			const AController* Controller = Other->GetController();
			Obj->SetStringField(TEXT("pilot"), Controller ? Controller->GetClass()->GetName() : TEXT("none"));
		}
		return Obj;
	};

	const AActor* Locked = PC ? PC->GetLockedTarget() : nullptr;
	if (Locked)
	{
		Root->SetObjectField(TEXT("target"), ContactJson(Locked));
	}

	if (World && Eye)
	{
		TArray<AActor*> Contacts;
		for (TActorIterator<ASpaceship> It(World); It; ++It)
		{
			if (*It != Ship)
			{
				Contacts.Add(*It);
			}
		}
		for (TActorIterator<ASpaceStation> It(World); It; ++It)
		{
			Contacts.Add(*It);
		}
		const FVector From = Eye->GetActorLocation();
		Contacts.RemoveAll([&](const AActor* A) { return FVector::DistSquared(From, A->GetActorLocation()) > FMath::Square(ContactRange); });
		Contacts.Sort([&](const AActor& A, const AActor& B)
		{
			return FVector::DistSquared(From, A.GetActorLocation()) < FVector::DistSquared(From, B.GetActorLocation());
		});
		TArray<TSharedPtr<FJsonValue>> ContactValues;
		for (int32 i = 0; i < FMath::Min(Contacts.Num(), MaxContacts); ++i)
		{
			ContactValues.Add(MakeShared<FJsonValueObject>(ContactJson(Contacts[i])));
		}
		Root->SetArrayField(TEXT("contacts"), ContactValues);
	}

	if (LogCapture)
	{
		int32 Errors = 0;
		int32 Warnings = 0;
		TArray<FString> Recent;
		LogCapture->Read(Errors, Warnings, Recent);
		TSharedRef<FJsonObject> LogObj = MakeShared<FJsonObject>();
		LogObj->SetNumberField(TEXT("errors"), Errors);
		LogObj->SetNumberField(TEXT("warnings"), Warnings);
		TArray<TSharedPtr<FJsonValue>> RecentValues;
		for (const FString& Line : Recent)
		{
			RecentValues.Add(MakeShared<FJsonValueString>(Line));
		}
		LogObj->SetArrayField(TEXT("recent"), RecentValues);
		Root->SetObjectField(TEXT("log"), LogObj);
	}

	Root->SetBoolField(TEXT("busy"), IsBusy());
	if (bHasCurrent)
	{
		TSharedRef<FJsonObject> CurrentObj = MakeShared<FJsonObject>();
		CurrentObj->SetStringField(TEXT("cmd"), Current.Text);
		CurrentObj->SetNumberField(TEXT("elapsed"), Round1(Elapsed));
		Root->SetObjectField(TEXT("current"), CurrentObj);
	}
	TArray<TSharedPtr<FJsonValue>> QueueValues;
	for (const FCommand& Command : Queue)
	{
		QueueValues.Add(MakeShared<FJsonValueString>(Command.Text));
	}
	Root->SetArrayField(TEXT("queue"), QueueValues);

	TArray<TSharedPtr<FJsonValue>> HistoryValues;
	for (const FResult& Result : History)
	{
		TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
		Obj->SetStringField(TEXT("cmd"), Result.Text);
		Obj->SetBoolField(TEXT("ok"), Result.bOk);
		Obj->SetStringField(TEXT("note"), Result.Note);
		Obj->SetNumberField(TEXT("t"), Round1(Result.Time));
		HistoryValues.Add(MakeShared<FJsonValueObject>(Obj));
	}
	Root->SetArrayField(TEXT("history"), HistoryValues);

	FString Out;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Out);
	FJsonSerializer::Serialize(Root, Writer);
	return Out;
}
