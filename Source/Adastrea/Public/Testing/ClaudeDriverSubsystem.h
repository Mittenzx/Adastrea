// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ClaudeDriverSubsystem.generated.h"

class AAdastreaPlayerController;
class ASpaceship;
class UInputAction;
class FClaudeLogCapture;

/**
 * Lets an AI agent (Claude) play the game in PIE through the player's own controller and ship.
 *
 * The agent queues short text commands, then polls a JSON snapshot of the game until the queue
 * is done. The commands run in C++ every tick, so a test script is a few one-shot calls:
 *
 *     claude.Run fly hostile 800; attack hostile 60; wait 2
 *     claude.State                     (logs "CLAUDE STATE {...}")
 *
 * or from editor Python: unreal.ClaudeDriverSubsystem.get(world).run("...").
 *
 * Nothing here replaces the player controller: the agent drives the possessed ship the way a
 * player does (throttle, the fire input action, lock-on, docking requests), so tests exercise
 * the real player path. Turning is applied straight to the ship (and the control rotation),
 * because synthetic mouse deltas are too coarse to aim with.
 *
 * Commands (separate with ';'). <ref> is "target" (the locked target), "hostile" (nearest
 * hostile ship), "station" (nearest station), "x,y,z" in metres, or a name fragment.
 *   fly <ref> [arrive_m=500]   turn toward and cruise to <ref>, stopping within arrive_m
 *   face <ref> [tol_deg=3]     turn the nose onto <ref>
 *   throttle <0-100>           set the throttle and move on
 *   stop                       throttle 0 and wait until the ship is (nearly) still
 *   lock <ref>                 lock <ref> as the target (as the targeting keys do)
 *   fire <seconds>             hold the fire input
 *   attack <ref> [timeout_s=60] lock, chase, lead and fire until <ref> is a wreck
 *   dock [ref=station]         fly to the station's docking bay and request docking
 *   undock                     leave the dock
 *   wait <seconds>             do nothing
 *   cmd <console command>      run a console command (e.g. "cmd SpawnHostiles 2")
 *   shot <name>                screenshot of the game viewport to Saved/Screenshots/<platform>/Claude/<name>.png
 * Each command logs "CLAUDE done <cmd> ok|FAIL <note>" and goes into the state's history.
 *
 * The state also counts the log's errors and warnings since PIE started (with the latest few
 * lines), so a test can fail on "something logged an error" without reading the log file.
 *
 * Only exists in Game/PIE worlds of non-shipping builds.
 */
UCLASS()
class ADASTREA_API UClaudeDriverSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Claude Driver", meta=(WorldContext="WorldContextObject"))
	static UClaudeDriverSubsystem* Get(const UObject* WorldContextObject);

	// USubsystem / UWorldSubsystem
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Append commands (';'-separated) to the queue. */
	UFUNCTION(BlueprintCallable, Category="Claude Driver")
	void Run(const FString& Script);

	/** Drop the current command and the queue, and release every held input. */
	UFUNCTION(BlueprintCallable, Category="Claude Driver")
	void Cancel();

	/** True while a command is running or queued. */
	UFUNCTION(BlueprintPure, Category="Claude Driver")
	bool IsBusy() const { return bHasCurrent || Queue.Num() > 0; }

	/** Snapshot of the player, ship, target, nearby contacts, queue and recent results, as JSON. */
	UFUNCTION(BlueprintCallable, Category="Claude Driver")
	FString GetStateJson() const;

	/** How many ships/stations GetStateJson lists, nearest first. */
	int32 MaxContacts = 12;

	/** Contacts further than this (cm) are left out of the state. */
	float ContactRange = 10000000.0f;

private:
	struct FCommand
	{
		FString Text;
		FString Verb;
		TArray<FString> Args;
		/** Everything after the verb, for "cmd". */
		FString Rest;
	};

	struct FResult
	{
		FString Text;
		bool bOk = false;
		FString Note;
		double Time = 0.0;
	};

	static bool ParseCommand(const FString& Text, FCommand& Out);

	void StartCurrent();
	/** One tick of the current command. Returns true when it has finished (and called Finish). */
	bool TickCurrent(float DeltaTime);
	void Finish(bool bOk, const FString& Note);

	/** Resolve a <ref> to an actor or a location; false if nothing matches. */
	bool ResolveRef(const FString& Ref, TWeakObjectPtr<AActor>& OutActor, FVector& OutLocation) const;
	/** Current location of the current command's ref (follows a moving actor). False if the actor is gone. */
	bool GetRefLocation(FVector& OutLocation) const;

	AAdastreaPlayerController* GetPC() const;
	ASpaceship* GetShip() const;

	/** Turn the ship toward Location at its turn rate. Returns the angle off the nose in degrees. */
	float FaceToward(ASpaceship* Ship, const FVector& Location, float DeltaTime) const;
	void SetFireHeld(bool bHeld);

	bool bHasCurrent = false;
	FCommand Current;
	float Elapsed = 0.0f;
	/** Per-command scratch: a phase flag and a seconds counter. */
	bool bPhase = false;
	float Counter = 0.0f;
	TWeakObjectPtr<AActor> RefActor;
	FVector RefLocation = FVector::ZeroVector;

	/** Counts and keeps the latest error/warning lines from the log while this world lives. */
	TSharedPtr<FClaudeLogCapture> LogCapture;

	TArray<FCommand> Queue;
	TArray<FResult> History;
	bool bFireHeld = false;
};
