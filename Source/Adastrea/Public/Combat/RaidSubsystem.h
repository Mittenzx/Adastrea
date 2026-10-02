#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Universe/PirateTypes.h"
#include "RaidSubsystem.generated.h"

class ALootDrone;
class ARaiderController;
class ASpaceship;
class UTradeItemDataAsset;
struct FPirateGang;

UENUM(BlueprintType)
enum class ERaidPhase : uint8
{
	/** Flying in on the chosen target. */
	Approach,
	/** Holding off the target, demand sent, waiting for an answer. */
	Hail,
	/** The target complied: drones are taking what was demanded. */
	Collect,
	/** The target refused: disable it. */
	Attack,
	/** The target is a wreck: drones strip its cargo and salvage. */
	Strip,
	/** Going home, with or without loot. */
	Retreat
};

/** One line of a demand: Units of Item. */
struct FRaidGoods
{
	TWeakObjectPtr<UTradeItemDataAsset> Item;
	int32 Units = 0;
};

/** What a raid asks a ship for. It matches the gang's shortage (PIRACY_AND_LAW.md 2a). */
struct FRaidDemand
{
	EPirateNeed Need = EPirateNeed::Cash;
	TArray<FRaidGoods> Goods;
	int32 Credits = 0;
	/** "We're short of fuel and food. Hand over 20 Helium-3 and we let you go." */
	FString Line;

	bool IsEmpty() const { return Goods.IsEmpty() && Credits <= 0; }
};

/** One raid in the level. */
struct FRaid
{
	int32 Id = INDEX_NONE;
	FName GangId;
	ERaidPhase Phase = ERaidPhase::Approach;
	float PhaseSeconds = 0.0f;
	TArray<TWeakObjectPtr<ARaiderController>> Raiders;
	TWeakObjectPtr<ASpaceship> Target;
	bool bTargetIsPlayer = false;
	FRaidDemand Demand;
	/** Desperation when the raid set out (0..1). */
	float Desperation = 0.0f;
	TArray<TWeakObjectPtr<ALootDrone>> Drones;
	bool bDronesLaunched = false;
	/** Drones sent in the last launch; each takes every DroneCount-th line of the demand. */
	int32 DroneCount = 0;
	bool bBroughtSomething = false;
	/** Where the target was when it agreed to comply. */
	FVector ComplyLocation = FVector::ZeroVector;
	/** Seconds the target has been out of reach (docked, hidden). */
	float LostTargetSeconds = 0.0f;
	FString Outcome;
};

/**
 * Pirate raids in the current level (PIRACY_AND_LAW.md, section 2, build-order step 2).
 *
 * A raid belongs to a gang (UPirateSubsystem) and runs:
 *
 *   Approach -> Hail -> Collect -> Retreat         the target complied
 *                    -> Attack  -> Strip -> Retreat the target refused, and was disabled
 *
 * Target choice: every ship in the level that isn't a raider or a patrol is scored by
 * how well its cargo (and credits) would fill the gang's shortages, traders first,
 * against the risk: sector security, patrols near it, an armed target, and distance
 * from the gang's home. Desperate gangs discount the risk. Wrecks are stripped without
 * a hail (parts from salvage).
 *
 * Raiders lie in wait ahead of a moving target (or of where an AI trader is heading).
 * Hail: the demand asks for what the gang lacks. The player answers in the comms panel
 * (AAdastreaHUD, J complies, K refuses, the countdown running out or shooting the
 * raiders refuses). AI traders decide in a few seconds. Comply: the target stops and
 * the raiders' drones (ALootDrone) fly over and take the goods. Refuse: the raiders
 * disable the target (never destroy), then strip its hold with drones.
 *
 * Each raider breaks off and runs home when its own shields fail. A raider leaves the
 * level once it is clear of the fight. Loot reaches the gang when a drone lands.
 *
 * Auto-raids: every so often a gang with ships to spare raids the level if it is within
 * reach of home and a target is worth the risk.
 *
 * Console: adastrea.Raid [GangId] [TargetName], adastrea.AutoRaids 0/1, adastrea.RaidAIComply -1/0/1,
 * adastrea.RaidTrader ItemName Units (a test trader with cargo), RaidComply / RaidRefuse (J / K).
 */
UCLASS()
class ADASTREA_API URaidSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static URaidSubsystem* Get(const UObject* WorldContext);

	/** Hold off the target this far while hailing (cm). */
	static constexpr float HoldDistance = 30000.0f;
	/** Hail (over comms) once the lead raider is this close (cm). */
	static constexpr float HailRange = 250000.0f;
	/** Launch drones once the lead raider is this close (cm). */
	static constexpr float DroneRange = 60000.0f;
	/** Seconds the player has to answer. */
	static constexpr float PlayerAnswerSeconds = 20.0f;

	/**
	 * Start a raid by GangId on Target (or the gang's best target when null), spawning
	 * its raiders SpawnDistance away. Returns the raid id, or INDEX_NONE.
	 */
	int32 StartRaid(FName GangId, ASpaceship* Target = nullptr, float SpawnDistance = 250000.0f);

	/** End every raid and remove its raiders and drones. */
	void ClearRaids();

	/** Best target for a gang in this level, or null; OutScore is its value less its risk. */
	ASpaceship* ChooseTarget(const FPirateGang& Gang, float& OutScore) const;

	/** What Ship is worth to Gang (need points weighed by shortage) and how risky it is. */
	void ScoreTarget(const FPirateGang& Gang, const ASpaceship* Ship, float& OutValue, float& OutRisk) const;

	/** How much Gang wants to raid Ship: its value less its risk, the risk discounted by desperation. */
	float ComputeScore(const FPirateGang& Gang, const ASpaceship* Ship) const;

	/** ComputeScore by gang id (for tests and debug). */
	UFUNCTION(BlueprintCallable, Category="Raids")
	float GetTargetScore(FName GangId, ASpaceship* Ship) const;

	/** StartRaid for scripts: returns the raid id or -1. */
	UFUNCTION(BlueprintCallable, Category="Raids")
	int32 StartRaidOn(FName GangId, ASpaceship* Target, float SpawnDistance = 250000.0f) { return StartRaid(GangId, Target, SpawnDistance); }

	/** One line per raid: "Id Gang phase Target raiders=N drones=N". */
	UFUNCTION(BlueprintCallable, Category="Raids")
	TArray<FString> DescribeRaids() const;

	UFUNCTION(BlueprintCallable, Category="Raids")
	int32 GetRaidCount() const { return Raids.Num(); }

	/** Whether a pirate hail (or collection) is aimed at the player right now. */
	UFUNCTION(BlueprintCallable, Category="Raids")
	bool IsPlayerHailed() const { return GetPlayerHail() != nullptr; }

	/** What Gang would demand of Ship. */
	FRaidDemand BuildDemand(const FPirateGang& Gang, const ASpaceship* Ship, float Desperation) const;

	const TArray<FRaid>& GetRaids() const { return Raids; }
	const FRaid* FindRaid(int32 RaidId) const;

	/** The raid hailing (or collecting from) the player's ship, if any: what the comms panel shows. */
	const FRaid* GetPlayerHail() const;

	/** Seconds left to answer a player hail. */
	float GetPlayerAnswerSecondsLeft() const;

	/** The player answers the hail (J / K, RaidComply / RaidRefuse). */
	void AnswerPlayerHail(bool bComply);

	/** A ship was hit (UShipHealthComponent reports every hit): a target shooting its raiders refuses. */
	void ReportAttack(ASpaceship* Victim, AActor* Attacker);

	// ---- Used by ARaiderController ----
	ERaidPhase GetPhase(int32 RaidId) const;
	ASpaceship* GetRaidTarget(int32 RaidId) const;
	void NotifyRaiderWrecked(ARaiderController* Raider);
	void NotifyRaiderLeft(ARaiderController* Raider);

	// ---- Used by ALootDrone ----
	/** A drone reached the target: take its share from the target's hold. */
	void NotifyDroneAtTarget(ALootDrone* Drone);
	/** A drone landed on a raider: its load reaches the gang. */
	void NotifyDroneHome(ALootDrone* Drone);
	/** The ship a drone of raid RaidId should land on, or null if every raider is gone. */
	ASpaceship* GetDroneHome(int32 RaidId, ASpaceship* Preferred) const;

	/** Whether Actor is a ship flown in (or left wrecked by) a raid. */
	static bool IsRaiderShip(const AActor* Actor);

	static FString PhaseToString(ERaidPhase Phase);

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Deinitialize() override;

private:
	FRaid* FindRaidMutable(int32 RaidId);
	void SetPhase(FRaid& Raid, ERaidPhase Phase);
	void TickRaid(FRaid& Raid, float DeltaTime);
	void TickAutoRaids(float DeltaTime);
	void BeginHail(FRaid& Raid);
	void Answer(FRaid& Raid, bool bComply, const TCHAR* Why);
	void LaunchDrones(FRaid& Raid, bool bStrip);
	void EndRaid(FRaid& Raid);
	void ReleaseTarget(FRaid& Raid);
	void TellPlayer(const FRaid& Raid, const FString& Text, float Seconds, bool bWarning) const;
	ASpaceship* GetLeadShip(const FRaid& Raid) const;
	bool AllRaidersFleeing(const FRaid& Raid) const;
	/** Whether the AI target of a hail complies (adastrea.RaidAIComply, else a weighed coin flip). */
	bool DecideAIComply(const FRaid& Raid) const;
	int32 CountPatrolsNear(const FVector& Location, float Radius) const;
	/** Gate hops from the gang's home to this level's sector (1 for levels outside the galaxy). */
	int32 GetHopsFromHome(const FPirateGang& Gang) const;

	TArray<FRaid> Raids;
	int32 NextRaidId = 1;
	float AutoRaidClock = 0.0f;
};
