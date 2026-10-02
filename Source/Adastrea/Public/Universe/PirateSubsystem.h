#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Universe/PirateTypes.h"
#include "PirateSubsystem.generated.h"

class UTradeItemDataAsset;

/** One pirate gang: an independent organisation (no parent) whose "pirate" block in Organisations.json gives its needs. */
struct FPirateGang
{
	FName OrgId;
	FPirateNeeds Needs;
	/** Level lost per upkeep tick, for each need. */
	float Upkeep[FPirateNeeds::Num] = { 0.5f, 0.3f, 2.0f, 4.0f, 1.5f, 1.0f };
	/** Game seconds its biggest raidable shortage has been severe (see GetDesperation). */
	float ShortageSeconds = 0.0f;
	int32 Raids = 0;
	int32 RaidsWon = 0;
	int32 ShipsLost = 0;
};

/**
 * Pirate gangs and what they need (PIRACY_AND_LAW.md, section 2a).
 *
 * Each gang keeps a level of 0..100 for each need: people, access, ships and parts,
 * supplies, information, cash. Levels fall every upkeep tick (the organisation
 * economy's tickSeconds) and when the gang loses ships, and rise from what its raids
 * bring home. What a gang is short of decides what it raids (URaidSubsystem).
 *
 * People and access are tracked but nothing fills them yet: captives and a captured
 * player come with boarding and capture (build-order steps 5 and 6). Raids fill parts,
 * supplies, information and cash.
 *
 * Desperation: while a gang's biggest raidable shortage stays below SevereLevel it
 * grows from 0 to 1 over DesperationSeconds; a desperate gang raids further from home,
 * into safer space, takes on better-armed targets and demands more.
 *
 * Console: adastrea.PirateInfo, adastrea.PirateNeed Gang Need Level, adastrea.PirateUpkeep [Count].
 */
UCLASS()
class ADASTREA_API UPirateSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	/** A need below this level is a severe shortage: desperation builds while it lasts. */
	static constexpr float SevereLevel = 30.0f;
	/** Game seconds of severe shortage to reach full desperation. */
	static constexpr float DesperationSeconds = 600.0f;

	static UPirateSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

	/** Re-read the "pirate" blocks of Organisations.json. Levels go back to the file's. */
	void ReloadGangs();

	const TArray<FPirateGang>& GetGangs() const { return Gangs; }
	FPirateGang* FindGang(FName OrgId);
	const FPirateGang* FindGang(FName OrgId) const;
	bool IsPirate(FName OrgId) const { return FindGang(OrgId) != nullptr; }

	/** A gang's level for one need (0 for unknown gangs). */
	UFUNCTION(BlueprintCallable, Category="Pirates")
	float GetNeedLevel(FName OrgId, EPirateNeed Need) const
	{
		const FPirateGang* Gang = FindGang(OrgId);
		return Gang ? Gang->Needs.Get(Need) : 0.0f;
	}

	static FString NeedToString(EPirateNeed Need);
	static bool ParseNeed(const FString& Text, EPirateNeed& OutNeed);

	/** How much a gang values filling this need, relative to the others (people highest, cash lowest). */
	static float GetImportance(EPirateNeed Need);

	/** Whether raids can fill this need yet (parts, supplies, information, cash). */
	static bool IsRaidable(EPirateNeed Need);

	/** The need a cargo item fills: fuel, food and medicine are supplies, components are parts, data is information, the rest is fenced for cash. */
	static EPirateNeed NeedForItem(const UTradeItemDataAsset* Item);

	/** Need points Units of Item add to the need it fills. */
	static float PointsForGoods(const UTradeItemDataAsset* Item, int32 Units);

	/** Cash points for Credits taken. */
	static float PointsForCredits(int32 Credits);

	/** Need points from stripping salvage off a wreck. */
	static constexpr float WreckSalvagePoints = 12.0f;

	/** 0 (stocked) .. 1 (has none). */
	static float GetShortage(const FPirateGang& Gang, EPirateNeed Need) { return 1.0f - Gang.Needs.Get(Need) / 100.0f; }

	/** The raidable need the gang most wants filled: its shortage weighed by importance. */
	static EPirateNeed GetBiggestShortage(const FPirateGang& Gang);

	/** 0 (calm) .. 1 (desperate). */
	static float GetDesperation(const FPirateGang& Gang) { return FMath::Clamp(Gang.ShortageSeconds / DesperationSeconds, 0.0f, 1.0f); }

	/** Ships the gang can send on a raid: 0 when it's out of parts (it fades out), up to 3. */
	static int32 GetRaidStrength(const FPirateGang& Gang);

	/** Raid loot arrives at the gang. */
	void DeliverGoods(FName OrgId, UTradeItemDataAsset* Item, int32 Units);
	void DeliverCredits(FName OrgId, int32 Credits);
	void DeliverSalvage(FName OrgId, float Points);

	/** A raid ended. */
	void RecordRaid(FName OrgId, bool bBroughtSomethingHome);

	/** One of the gang's ships was disabled: the hull is lost, the pilot has to get home in a pod. */
	void RecordShipLost(FName OrgId);

	/** Apply Count upkeep ticks now (also adastrea.PirateUpkeep). */
	void RunUpkeep(int32 Count = 1);

	/** Log every gang's levels (adastrea.PirateInfo). */
	void LogGangs() const;

	/** Need levels, for UAdastreaSaveGame. */
	void ExportState(TMap<FName, FPirateNeeds>& Out) const;
	void ImportState(const TMap<FName, FPirateNeeds>& In);

private:
	void RunUpkeepTick();

	TArray<FPirateGang> Gangs;
	float UpkeepSeconds = 60.0f;
	float UpkeepAccumulator = 0.0f;
};
