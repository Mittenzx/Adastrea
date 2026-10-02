// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Player/AdastreaSaveGame.h"
#include "GalaxySubsystem.generated.h"

class APlayerController;
class AJumpGate;

/**
 * How well a sector is policed. Decides whether patrols answer a distress call, how
 * many come and how long they take (see UDistressSubsystem).
 */
UENUM(BlueprintType)
enum class ESectorSecurity : uint8
{
	/** Nobody answers. */
	None,
	Low,
	Medium,
	High
};

/**
 * One sector inside a star system.
 *
 * A sector is the playable unit: at most one level (umap) per sector. Sectors
 * without a level are "planned" - they show on the system map so the layout
 * can be designed before the level exists.
 */
USTRUCT(BlueprintType)
struct ADASTREA_API FGalaxySectorDef
{
	GENERATED_BODY()

	/** Unique ID across the whole galaxy (e.g. "adastrea_prime"). Matched by ASpaceSectorMap::SectorId. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	FText Name;

	/** Free-form role shown on the map ("Trade Hub", "Mining", "Frontier"...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	FString Type;

	/** "High", "Medium", "Low" or "None". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	FString Security;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	FString Faction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	FText Description;

	/** Long package name of the sector's level (e.g. "/Game/Maps/TestLevel"); empty = planned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	FString Level;

	/** Distance from the star on the system map, 0 (star) .. 1 (system edge). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	float OrbitRadius = 0.5f;

	/** Angle around the star on the system map, degrees (0 = right, counter-clockwise). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	float OrbitAngle = 0.0f;

	/** Sectors in the same system reachable by gate. Links are made two-way on load. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	TArray<FName> Gates;

	/**
	 * Sectors in OTHER systems reachable by a jump-lane gate from this sector.
	 * The two systems must be joined by a jump lane. Made two-way on load.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|Sector")
	TArray<FName> LaneGates;

	/** Owning system (filled in on load). */
	UPROPERTY(BlueprintReadOnly, Category="Galaxy|Sector")
	FName SystemId;

	bool HasLevel() const { return !Level.IsEmpty(); }
};

/** A star system: a star plus the sectors around it, placed on the universe map. */
USTRUCT(BlueprintType)
struct ADASTREA_API FStarSystemDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	FText Name;

	/** Spectral class shown on the map ("G2V", "M4", "Neutron"...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	FString StarClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	FLinearColor StarColor = FLinearColor(1.0f, 0.85f, 0.55f, 1.0f);

	/** Position on the universe map in light-years. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	FString Faction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	FText Description;

	/** Systems reachable by jump lane. Links are made two-way on load. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	TArray<FName> JumpLinks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Galaxy|System")
	TArray<FGalaxySectorDef> Sectors;

	int32 NumBuiltSectors() const
	{
		int32 N = 0;
		for (const FGalaxySectorDef& S : Sectors) { N += S.HasLevel() ? 1 : 0; }
		return N;
	}
};

/**
 * The galaxy layout: star systems, their sectors, gates and jump lanes.
 *
 * Loaded from Content/Data/Universe/Galaxy.json so sectors can be added and
 * wired up without touching C++ or uassets. If the file is missing or invalid,
 * a small built-in galaxy is used so the map always has something to show.
 *
 * A level joins the galaxy either by placing an ASpaceSectorMap with its
 * SectorId set, or by being listed as a sector's "level" in the JSON.
 *
 * Console: adastrea.ReloadGalaxy (re-reads the JSON), adastrea.GalaxyInfo.
 */
UCLASS()
class ADASTREA_API UGalaxySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Convenience accessor from any world context. May return nullptr. */
	static UGalaxySubsystem* Get(const UObject* WorldContextObject);

	/** Absolute path of the galaxy JSON. */
	static FString GetGalaxyFilePath();

	/** Re-read the galaxy JSON (falls back to the built-in galaxy on failure). Returns true if the file loaded. */
	UFUNCTION(BlueprintCallable, Category="Galaxy")
	bool ReloadGalaxy();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Galaxy")
	TArray<FStarSystemDef> GetStarSystems() const { return Systems; }

	/** Native, no-copy access to all systems. */
	const TArray<FStarSystemDef>& GetSystems() const { return Systems; }

	/** System the player starts in (and the map's default when the level isn't in the galaxy). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Galaxy")
	FName GetStartSystemId() const { return StartSystemId; }

	UFUNCTION(BlueprintCallable, Category="Galaxy")
	bool GetStarSystem(FName SystemId, FStarSystemDef& OutSystem) const;

	UFUNCTION(BlueprintCallable, Category="Galaxy")
	bool GetSector(FName SectorId, FGalaxySectorDef& OutSector) const;

	/**
	 * Which galaxy sector the given world is.
	 * 1. An ASpaceSectorMap with a known SectorId (the one containing the player, else the first).
	 * 2. A sector whose "level" is this world's map.
	 * Returns NAME_None if the level isn't part of the galaxy yet.
	 */
	UFUNCTION(BlueprintCallable, Category="Galaxy", meta=(WorldContext="WorldContextObject"))
	FName ResolveCurrentSectorId(const UObject* WorldContextObject) const;

	/** A sector's "security" text ("High", "Medium", "Low"); anything else is None. */
	static ESectorSecurity ParseSecurity(const FString& Text);
	static FString SecurityToString(ESectorSecurity Level);

	/**
	 * Security of the sector the world is (None for levels outside the galaxy, such as
	 * CombatTest). The console variable adastrea.Security 0-3 overrides it for testing.
	 */
	UFUNCTION(BlueprintCallable, Category="Galaxy", meta=(WorldContext="WorldContextObject"))
	ESectorSecurity GetCurrentSecurity(const UObject* WorldContextObject) const;

	/** True if the galaxy came from the JSON file (false = built-in fallback). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Galaxy")
	bool IsLoadedFromFile() const { return bLoadedFromFile; }

	// Native lookups (pointers stay valid until the next reload).
	const FStarSystemDef* FindSystem(FName SystemId) const;
	const FGalaxySectorDef* FindSector(FName SectorId) const;

	/** Bumped on every (re)load so cached UI can notice. */
	int32 GetRevision() const { return Revision; }

	// ---- Jump-gate travel ----

	/** Every sector a jump gate in SectorId leads to (Gates + LaneGates). */
	TArray<FName> GetGateDestinations(FName SectorId) const;

	/** Shortest gate route From -> To, excluding From (empty if unreachable or From == To). */
	UFUNCTION(BlueprintCallable, Category="Galaxy|Travel")
	TArray<FName> FindRoute(FName FromSectorId, FName ToSectorId) const;

	/**
	 * Jump the player to another sector: snapshots the ship, credits and cargo
	 * (via USaveGameSubsystem), remembers this sector's player-built stations,
	 * fades out and loads the target sector's level. On arrival the player comes
	 * out of the gate that leads back. Returns false (with a reason) if the jump
	 * can't start: target has no level, not flying a ship, docked, already jumping...
	 */
	bool BeginJump(APlayerController* PC, FName TargetSectorId, FString& OutReason);

	/** True between BeginJump and the arrival being applied in the new level. */
	bool IsJumpInProgress() const { return bJumpPending; }

	/** Sector the pending jump came from (the arrival gate is the one leading back there). */
	FName GetPendingJumpFrom() const { return bJumpPending ? PendingFromSectorId : NAME_None; }

	/**
	 * Called by UJumpGateWorldSubsystem once the new level is playing and the
	 * gates exist: moves the ship out of ArrivalGate (may be null) and applies
	 * the snapshot. Returns true if a pending jump was completed.
	 */
	bool CompletePendingJump(UWorld* World, AJumpGate* ArrivalGate);

private:
	bool LoadFromFile(const FString& Path, FString& OutError);
	void BuildDefaultGalaxy();
	/** Fill SystemId, make links two-way, drop dangling links, index IDs. */
	void FinalizeGalaxy();

	UPROPERTY()
	TArray<FStarSystemDef> Systems;

	FName StartSystemId;
	bool bLoadedFromFile = false;
	int32 Revision = 0;

	// Jump in flight (survives the level change: this subsystem lives on the GameInstance).
	bool bJumpPending = false;
	FName PendingFromSectorId;
	FName PendingToSectorId;

	/** Ship/credits/cargo/progression captured when the jump started. */
	UPROPERTY()
	TObjectPtr<UAdastreaSaveGame> PendingSnapshot;

	/** Player-built stations per sector, so they are still there when the player comes back (this session). */
	TMap<FName, TArray<FSavedStation>> SectorStations;

	FTimerHandle JumpTimer;

	TMap<FName, int32> SystemIndex;
	TMap<FName, TPair<int32, int32>> SectorIndex; // sector -> (system, sector)
};
