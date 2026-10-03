// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Trading/CraftingTreeLoader.h"
#include "CraftingManager.generated.h"

class ASpaceStation;
class UCargoComponent;

/** One production order at a station: Crafts runs of a recipe, made one after another. */
USTRUCT(BlueprintType)
struct ADASTREA_API FCraftingJob
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	int32 JobId = 0;

	/** UCraftingManager::GetStationKey of the station running the job ("sector/Station Name"). */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	FName StationKey;

	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	FString StationName;

	/** Recipe ProducedIn tag ("Fabrication", "Processing"...). */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	FString Facility;

	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	FName OutputItem;

	/** Units one run makes. */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	int32 OutputPerCraft = 1;

	/** Runs ordered (ingredients for all of them were taken when the job was queued). */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	int32 Crafts = 1;

	/** Runs finished. */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	int32 CraftsDone = 0;

	/** Finished units waiting at the station to be collected. */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	int32 UnitsReady = 0;

	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	float SecondsPerCraft = 10.0f;

	/** Seconds spent on the current run. */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	float Progress = 0.0f;

	bool IsRunning() const { return CraftsDone < Crafts; }
	bool IsFinished() const { return !IsRunning() && UnitsReady <= 0; }
	float GetCurrentCraftFraction() const { return SecondsPerCraft > 0.0f ? FMath::Clamp(Progress / SecondsPerCraft, 0.0f, 1.0f) : 1.0f; }
	float GetSecondsLeft() const { return IsRunning() ? (Crafts - CraftsDone) * SecondsPerCraft - Progress : 0.0f; }
};

/** Why a recipe can or can't be queued at a station right now. */
UENUM(BlueprintType)
enum class ECraftingCheck : uint8
{
	Ok,
	NoRecipe,
	NoFacility,
	NeedsResearch,
	MissingIngredients,
	QueueFull,
};

/**
 * Runs crafting-tree recipes at station production modules.
 *
 * Owns the one copy of the crafting tree (items and recipes) everything else
 * shares. The player queues orders at the station they're docked at: the
 * ingredients for every run leave the hold at once, the station's modules of
 * the recipe's facility work through the runs (more modules, faster runs; jobs
 * of one facility run one after another), and finished units wait at that
 * station. They go straight into the hold while the player is docked there,
 * and otherwise when they next dock. Jobs keep running in other sectors and are
 * saved with the game.
 *
 * Recipes that need research ("ResearchRequired" in CraftingTree.json) are
 * listed but locked: there is no research yet.
 *
 * Console: adastrea.CraftInfo, adastrea.Craft OutputItem [Runs], adastrea.CraftSpeed Multiplier.
 */
UCLASS()
class ADASTREA_API UCraftingManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static UCraftingManager* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** The shared crafting tree, loaded once (items and recipes). */
	UCraftingTreeLoader* GetLoader() const { return Loader; }

	/** Seconds one run of a recipe takes with a single module (by tier). */
	static float GetBaseSecondsPerCraft(const FCraftingRecipe& Recipe);

	/** ProducedIn tag a station module produces for ("Science Lab" -> "ScienceLab"), or empty. */
	static FString GetFacilityForModuleType(const FString& ModuleType);

	/** Facilities the station has modules for, in a stable order, each with its module count. */
	TArray<TPair<FString, int32>> GetStationFacilities(const ASpaceStation* Station) const;

	/** Recipes a facility makes, cheapest tiers first (no gathering recipes). */
	TArray<FCraftingRecipe> GetRecipesFor(const FString& Facility) const;

	/** Research a recipe needs, or empty. */
	FString GetResearchRequired(const FCraftingRecipe& Recipe) const;

	/** How many runs of Recipe the hold has ingredients for (capped at 999). */
	static int32 GetAffordableCrafts(const FCraftingRecipe& Recipe, const UCargoComponent* Cargo);

	ECraftingCheck CheckQueue(const ASpaceStation* Station, const FCraftingRecipe& Recipe, int32 Crafts, const UCargoComponent* Cargo) const;
	static FText CheckToText(ECraftingCheck Check);

	/** Take the ingredients for Crafts runs from Cargo and queue the job at Station. Returns the job id, or 0. */
	int32 QueueJob(ASpaceStation* Station, const FCraftingRecipe& Recipe, int32 Crafts, UCargoComponent* Cargo);

	/** Cancel a job: runs not started give their ingredients back to Cargo (as space allows; the rest is lost). */
	bool CancelJob(int32 JobId, UCargoComponent* Cargo);

	/** Move finished units of every job at Station into Cargo. Returns units moved. */
	int32 CollectAtStation(const ASpaceStation* Station, UCargoComponent* Cargo);

	/** Jobs at a station (or all jobs with no station), oldest first. */
	TArray<FCraftingJob> GetJobs(const ASpaceStation* Station = nullptr) const;

	/** Max jobs waiting or running per facility at one station. */
	static constexpr int32 MaxJobsPerFacility = 4;

	/** "adastrea_prime/Prime Station": identifies a station across levels and saves. */
	static FName GetStationKey(const ASpaceStation* Station);

	// Save support.
	void ExportJobs(TArray<FCraftingJob>& Out) const { Out = Jobs; }
	void ImportJobs(const TArray<FCraftingJob>& In);

	/** Fired when a run completes (JobId, units made). */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCraftCompleted, int32, int32);
	FOnCraftCompleted OnCraftCompleted;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

	/** Scales job speed (console adastrea.CraftSpeed, for testing). */
	float SpeedMultiplier = 1.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UCraftingTreeLoader> Loader;

	UPROPERTY()
	TArray<FCraftingJob> Jobs;

	/** Recipe id -> research it needs (from the JSON; FCraftingRecipe doesn't carry it). */
	TMap<FString, FString> ResearchByRecipe;

	/** ProducedIn tags that have at least one recipe with ingredients (what a module can make). */
	TSet<FString> CraftingFacilities;

	int32 NextJobId = 1;

	void LoadResearchRequirements();
	FCraftingJob* FindJob(int32 JobId);

	/** The player's ship docked at the station with this key, if any. */
	UCargoComponent* GetDockedPlayerCargo(FName StationKey) const;
};
