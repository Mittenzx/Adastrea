// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Universe/PirateTypes.h"
#include "OrganisationSubsystem.generated.h"

class AActor;
class ASpaceship;
class UPlayerTraderComponent;

/**
 * What kind of owner an organisation is. Kept deliberately small-scale: the
 * biggest thing in the galaxy is an Authority running one to three sectors.
 */
UENUM(BlueprintType)
enum class EOrgKind : uint8
{
	/** A single captain (the player is one of these). Owns 1-2 ships. */
	Person,
	/** A family or small company. Owns 2-6 ships. */
	Outfit,
	/** Specialists sharing a trade (miners, haulers, salvagers). No territory; spans a few sectors. */
	Guild,
	/** Government, clan or council governing a sector from its main station or planet. */
	Authority,
};

/** What a roster ship does when it is spawned into a level. */
UENUM(BlueprintType)
enum class EShipRole : uint8
{
	Trader,
	Miner,
	/** No patrol AI yet: flies between stations without trading. */
	Patrol,
};

/** An owner of ships and stations, loaded from Organisations.json. */
USTRUCT(BlueprintType)
struct ADASTREA_API FOrgDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	EOrgKind Kind = EOrgKind::Person;

	/** Allegiance: captain -> outfit/guild, guild -> the authority that charters it. None = independent. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FName ParentId;

	/** Authority only: galaxy sector ids it governs (at most MaxGovernedSectors, all in one system). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	TArray<FName> Governs;

	/** Authority only: where it governs from ("Prime Station", "Kestrel IV"...). Display text. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FString Seat;

	/** Sector the organisation is based in. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FName HomeSectorId;

	/** Guild only: the trades it works ("Mining", "Transport"...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	TArray<FString> Industries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FText Description;

	/** Flag colour for UI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FLinearColor Colour = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

	/** Wallet: earns from its ships' trade (and taxes, for authorities), pays upkeep and buys ships. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	int32 Credits = 0;

	/** Authority only: share of the profit made by other owners' ships in its sectors it takes as tax (0..0.5). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	float TaxRate = 0.08f;

	/** Authority only: fleet size it keeps up by replacing lost/sold ships (it doesn't expand). Set from the roster on load. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	int32 TargetFleetSize = 0;

	/** A pirate gang (has a "pirate" block; see UPirateSubsystem). Pirates don't buy ships at lawful yards. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	bool bPirate = false;
};

/** Running totals for one organisation, shown by adastrea.OrgInfo. */
USTRUCT(BlueprintType)
struct ADASTREA_API FOrgLedger
{
	GENERATED_BODY()

	/** Profit from its ships (real trades plus off-screen income), after tax. */
	UPROPERTY(BlueprintReadOnly, Category="Organisation")
	int64 Earned = 0;

	UPROPERTY(BlueprintReadOnly, Category="Organisation")
	int64 TaxPaid = 0;

	/** Authorities: tax plus base income from governed sectors. */
	UPROPERTY(BlueprintReadOnly, Category="Organisation")
	int64 TaxCollected = 0;

	UPROPERTY(BlueprintReadOnly, Category="Organisation")
	int64 Upkeep = 0;

	UPROPERTY(BlueprintReadOnly, Category="Organisation")
	int32 ShipsBought = 0;

	UPROPERTY(BlueprintReadOnly, Category="Organisation")
	int32 ShipsSold = 0;
};

/** Tuning for the organisation economy, from the "economy" block of Organisations.json. Per-role arrays are indexed by EShipRole. */
struct FOrgEconomyConfig
{
	/** Game seconds between economy ticks. */
	float TickSeconds = 60.0f;
	int32 ShipPrice[3] = { 400000, 350000, 450000 };
	int32 Upkeep[3] = { 700, 600, 600 };
	/** Income per tick for a ship that isn't spawned (its sector isn't the level being played). */
	int32 OffscreenIncome[3] = { 3000, 3000, 0 };
	/** An authority's base income per governed sector per tick. */
	int32 SectorIncome = 2500;
	/** Share of the price got back when selling a ship. */
	float ResaleFraction = 0.6f;
	/** Buy only when this share of the price is left over afterwards. */
	float ReserveFraction = 0.5f;
};

/** One ship in the galaxy. It exists as data whether or not an actor is spawned for it. */
USTRUCT(BlueprintType)
struct ADASTREA_API FShipRecord
{
	GENERATED_BODY()

	/** Stable id (e.g. "juno_marr_1"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Record")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Record")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Record")
	FName OwnerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Record")
	EShipRole Role = EShipRole::Trader;

	/** Sector the ship is in; it is spawned when that sector's level is played. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Record")
	FName SectorId;

	/** Optional ship Blueprint; when unset the level's AAIShipPopulator picks one for the role. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Record")
	TSoftClassPtr<ASpaceship> ShipClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Record")
	bool bDestroyed = false;

	/** Profit this ship has passed to its owner (after tax). */
	UPROPERTY(BlueprintReadOnly, Category="Ship Record")
	int32 Earnings = 0;

	/** The actor currently standing in for this record, if any (cleared when the level unloads). */
	TWeakObjectPtr<ASpaceship> SpawnedShip;
};

/** Standing between two organisations, -100 (war) .. 100 (allies). Symmetric. */
USTRUCT(BlueprintType)
struct ADASTREA_API FOrgRelation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FName A;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FName B;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	int32 Standing = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organisation")
	FString Note;
};

/** What UAdastreaSaveGame keeps of the organisations: wallets, ledgers and the roster. */
USTRUCT(BlueprintType)
struct ADASTREA_API FSavedOrganisations
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Save")
	bool bValid = false;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TMap<FName, int32> Wallets;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TMap<FName, FOrgLedger> Ledgers;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TArray<FShipRecord> Ships;

	/** Game seconds into the current economy tick. */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	float TickProgress = 0.0f;

	/** Pirate gangs' need levels (UPirateSubsystem). */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	TMap<FName, FPirateNeeds> PirateNeeds;
};

/**
 * Who owns what: organisations (people, outfits, guilds, authorities), the ship
 * roster, the standings between them, and the money moving between them.
 *
 * Loaded from Content/Data/Universe/Organisations.json. Lives on the
 * GameInstance, so the roster survives sector jumps: the same ships come back
 * when the player returns to a sector. AAIShipPopulator spawns the roster ships
 * of the level's sector.
 *
 * Economy: when a spawned roster ship sells, its profit above its trading float
 * goes to its owner, less the sector authority's tax (see HandleShipSale).
 * Every TickSeconds of game time: ships that aren't spawned earn a flat income
 * for their role (taxed the same way), every ship costs its owner upkeep, and
 * authorities get a base income per governed sector. Then owners in debt sell a
 * ship that isn't in view, and businesses with spare credits buy one (up to
 * their cap; authorities only back up to TargetFleetSize).
 *
 * Small-scale rules (warned about on load, not enforced): an authority governs
 * at most MaxGovernedSectors sectors, all in one system; each kind has a ship cap.
 *
 * Console: adastrea.ReloadOrgs, adastrea.OrgInfo [SectorId], adastrea.OrgStanding A B,
 * adastrea.EconomyTick [Count], adastrea.OrgGrant OrgId Amount.
 */
UCLASS()
class ADASTREA_API UOrganisationSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	/** Id of the organisation standing for the player. Always present. */
	static const FName PlayerOrgId;

	static constexpr int32 MaxGovernedSectors = 3;

	/** Fired when ships are added to or removed from the roster (bought, sold, loaded). */
	DECLARE_MULTICAST_DELEGATE(FOnRosterChanged);
	FOnRosterChanged OnRosterChanged;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// FTickableGameObject: advances the economy with game time (not while paused).
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

	static UOrganisationSubsystem* Get(const UObject* WorldContextObject);

	static FString GetOrganisationsFilePath();

	/** Most ships an owner of this kind should have. */
	static int32 GetShipCap(EOrgKind Kind);

	static FString KindToString(EOrgKind Kind);
	static FString RoleToString(EShipRole Role);

	/** Re-read the JSON. Forgets which actors stand in for which records. Returns true if the file loaded. */
	UFUNCTION(BlueprintCallable, Category="Organisations")
	bool ReloadOrganisations();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Organisations")
	TArray<FOrgDef> GetOrganisations() const { return Orgs; }

	UFUNCTION(BlueprintCallable, Category="Organisations")
	bool GetOrganisation(FName OrgId, FOrgDef& OutOrg) const;

	/** Display name of an organisation, or "Unowned" for None / unknown ids. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Organisations")
	FText GetOrgDisplayName(FName OrgId) const;

	/** The authority governing SectorId, or None (independent / unclaimed space). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Organisations")
	FName GetSectorAuthorityId(FName SectorId) const;

	/** OrgId followed by its parents up to the top (cycles are cut). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Organisations")
	TArray<FName> GetAllegianceChain(FName OrgId) const;

	/**
	 * Standing between two organisations. An explicit relation between the two
	 * (or the nearest pair of their parents) wins; organisations sharing an
	 * allegiance default to 50; otherwise 0.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Organisations")
	int32 GetStanding(FName A, FName B) const;

	/**
	 * Who owns Actor: its UOwnershipComponent, else PlayerOrgId for a
	 * player-controlled pawn, else (for a station) the sector's authority.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Organisations")
	FName GetOwnerIdOf(const AActor* Actor) const;

	/** "Juno Marr (Person) > Haven Freight Union (Guild) > Haven Moot (Authority)". */
	FString DescribeAllegiance(FName OrgId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Organisations")
	TArray<FShipRecord> GetShipRecords() const { return Ships; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Organisations")
	FOrgLedger GetLedger(FName OrgId) const { return Ledgers.FindRef(OrgId); }

	// Native access (pointers stay valid until the roster changes).
	const FOrgDef* FindOrg(FName OrgId) const;
	const TArray<FShipRecord>& GetShips() const { return Ships; }
	FShipRecord* FindShip(FName ShipId);
	const FShipRecord* FindShip(FName ShipId) const;
	const FOrgEconomyConfig& GetEconomyConfig() const { return Economy; }

	/** Records in SectorId that aren't destroyed. */
	TArray<FShipRecord*> GetShipsInSector(FName SectorId);

	/** Ships owned directly by OrgId (not counting its members'). */
	int32 CountShipsOwnedBy(FName OrgId) const;

	/** Remember that Ship stands in for record ShipId. */
	void BindSpawnedShip(FName ShipId, ASpaceship* Ship);

	/**
	 * A spawned roster ship sold cargo. Whatever Trader now holds above
	 * WorkingCapital is profit: it leaves the ship, the sector's authority takes
	 * its tax rate of it and the rest goes to the ship's owner.
	 */
	void HandleShipSale(FName ShipId, UPlayerTraderComponent* Trader, int32 WorkingCapital);

	/** Run Count economy ticks now (also the console command adastrea.EconomyTick). */
	UFUNCTION(BlueprintCallable, Category="Organisations")
	void RunEconomyTicks(int32 Count = 1);

	/** Add Amount (may be negative) to an organisation's wallet. False if OrgId is unknown. */
	UFUNCTION(BlueprintCallable, Category="Organisations")
	bool GrantCredits(FName OrgId, int32 Amount);

	/** Game seconds until the next economy tick. */
	float GetSecondsToNextTick() const { return FMath::Max(0.0f, Economy.TickSeconds - TickAccumulator); }

	/** Wallets, ledgers and roster, for UAdastreaSaveGame. */
	void ExportState(FSavedOrganisations& Out) const;

	/**
	 * Restore a saved state: wallets and ledgers of known orgs, and the roster.
	 * Saved records keep the actor of a live record with the same id. Live
	 * spawned ships the save doesn't know are kept (actors can't be removed
	 * cleanly yet). Records missing an actor are left for the level's
	 * AAIShipPopulator (OnRosterChanged).
	 */
	void ImportState(const FSavedOrganisations& In);

	/** True if the organisations came from the JSON file (false = only the player). */
	bool IsLoadedFromFile() const { return bLoadedFromFile; }

	int32 GetRevision() const { return Revision; }

private:
	bool LoadFromFile(const FString& Path, FString& OutError);
	void LoadEconomy(const TSharedPtr<class FJsonObject>& Root);
	/** Adds the player org, indexes ids, logs rule violations. */
	void Finalize();
	void Validate() const;
	void RebuildShipIndex();

	FOrgDef* FindOrgMutable(FName OrgId);

	/** Authority tax on Amount earned by OwnerId in SectorId; credits the authority and returns the tax. */
	int32 CollectTax(FName OwnerId, FName SectorId, int32 Amount);

	void RunEconomyTick();
	/** Sell one of Org's ships that isn't spawned. */
	bool SellShip(FOrgDef& Org);
	bool BuyShip(FOrgDef& Org);
	EShipRole ChooseRoleToBuy(const FOrgDef& Org) const;
	FName ChooseSectorForNewShip(const FOrgDef& Org) const;

	UPROPERTY()
	TArray<FOrgDef> Orgs;

	UPROPERTY()
	TArray<FShipRecord> Ships;

	UPROPERTY()
	TArray<FOrgRelation> Relations;

	UPROPERTY()
	TMap<FName, FOrgLedger> Ledgers;

	FOrgEconomyConfig Economy;
	float TickAccumulator = 0.0f;

	TMap<FName, int32> OrgIndex;
	TMap<FName, int32> ShipIndex;
	TMap<FName, FName> SectorAuthority;

	bool bLoadedFromFile = false;
	int32 Revision = 0;
};
