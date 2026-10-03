#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DroneBayComponent.generated.h"

class AAsteroid;
class AShipDrone;
class ACargoDrone;
class ASpaceship;
class UCargoComponent;
class UTradeItemDataAsset;

UENUM(BlueprintType)
enum class EDroneBayStatus : uint8
{
	Disabled   UMETA(DisplayName="No Drone Bay"),
	Stowed     UMETA(DisplayName="Stowed"),
	NoTarget   UMETA(DisplayName="No Target"),
	OutOfRange UMETA(DisplayName="Out Of Range"),
	HoldFull   UMETA(DisplayName="Hold Full"),
	Mining     UMETA(DisplayName="Mining"),
	Recalling  UMETA(DisplayName="Recalling"),
	/** Cargo drones moving goods to or from another ship. */
	Transferring UMETA(DisplayName="Transferring Cargo")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOreMined, UTradeItemDataAsset*, Ore, int32, Amount);

/**
 * The ship's drone bay. Ships have no beams: mining (and later gas extraction,
 * towing and cargo transfer) is done by drones that physically fly out, work,
 * and bring the goods home.
 *
 * Mining: lock an asteroid, LaunchDrones(). Drones fly out one after another,
 * cut ore into their hoppers, fly back and unload into the owner's
 * UCargoComponent, then go out again until the rock is spent, the hold is full,
 * or RecallDrones() is called. The component sits where the bay hatch is;
 * drones launch and land there.
 *
 * Cargo transfer: StartCargoTransfer() sends cargo drones (ACargoDrone) to another
 * ship to take goods out of its hold (a wreck, or a ship with the same owner) or to
 * give goods to it (same owner). A load at a time, until there's nothing left to
 * move or no room for it. Goods taken from someone else's wreck are stolen
 * (FCargoEntry::StolenFrom); pirate wrecks are fair salvage. Mining and transfers
 * share the drones: starting one stops the other.
 */
UCLASS(ClassGroup=(Drones), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UDroneBayComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDroneBayComponent();

	/** Master switch; ships without mining drones leave this off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones")
	bool bMiningEnabled;

	/** Mining drones carried. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="1", ClampMax="12"))
	int32 DroneCount;

	/** Drones only work rocks whose surface is within this distance (cm) of the bay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="100.0"))
	float Range;

	/** How far away (cm) an asteroid can be locked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="100.0"))
	float LockRange;

	/** Cutting power of each drone (1 = reference; divided by rock hardness). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="0.0"))
	float MiningPower;

	/** Ore units a drone carries per trip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="1.0"))
	float HopperCapacity;

	/** Cruise speed (cm/s). Drones always fly at least a bit faster than the ship so they can catch up. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="100.0"))
	float DroneSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="100.0"))
	float DroneAcceleration;

	/** Seconds between drones leaving the bay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="0.0"))
	float LaunchInterval;

	/** Lock brackets only pick rocks within this cone (degrees) of the ship's nose. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones", meta=(ClampMin="1.0", ClampMax="180.0"))
	float MaxAimAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones")
	TSubclassOf<AShipDrone> DroneClass;

	/** At BeginPlay, move the hatch to the underside of the hull (centre of the ship's bounds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones")
	bool bAutoPlaceHatch;

	/** Ore units delivered to cargo since the ship spawned (for the HUD). */
	UPROPERTY(BlueprintReadOnly, Category="Drones")
	int32 SessionOreMined;

	UPROPERTY(BlueprintAssignable, Category="Drones")
	FOnOreMined OnOreMined;

	/** Lock a specific asteroid. Returns false if it cannot be targeted. */
	UFUNCTION(BlueprintCallable, Category="Drones")
	bool SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category="Drones")
	void ClearTarget();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	AActor* GetTarget() const;

	/** Lock the asteroid closest to the given aim ray (within LockRange). */
	UFUNCTION(BlueprintCallable, Category="Drones")
	bool LockBestTarget(FVector Origin, FVector AimDirection);

	/** Lock the asteroid nearest to the ship's nose within the aim cone. */
	UFUNCTION(BlueprintCallable, Category="Drones")
	bool LockNearestAhead();

	/** Send the drones to work the locked rock (locks the rock ahead if nothing is locked). */
	UFUNCTION(BlueprintCallable, Category="Drones")
	void LaunchDrones();

	/** Call every drone home. They unload what they carry and stay in the bay. */
	UFUNCTION(BlueprintCallable, Category="Drones")
	void RecallDrones();

	/** True while the drones are ordered out (they keep cycling until recalled or the job ends). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	bool IsDeployed() const { return bDeployed; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	bool IsMining() const { return Status == EDroneBayStatus::Mining; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	EDroneBayStatus GetStatus() const { return Status; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	static FText StatusToText(EDroneBayStatus InStatus);

	/** StatusToText, with the work named for the target: MINING, SKIMMING GAS or SALVAGING. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	FText GetStatusText() const;

	/** Drones outside the bay (flying, cutting or landing). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	int32 GetDronesOut() const;

	/** Drones currently cutting the rock. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	int32 GetDronesCutting() const;

	/** Ore aboard drones that hasn't reached the hold yet, plus any part-unit waiting in the bay. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	float GetOreInTransit() const;

	/** Distance (cm) from the bay to the target's surface, or -1 with no target. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	float GetTargetSurfaceDistance() const;

	/** True when a target is locked and its surface is within Range. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	bool IsTargetInRange() const;

	/** Ore item the locked asteroid yields, or null. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	UTradeItemDataAsset* GetTargetOre() const;

	/** Ore units per second one drone cuts from the locked asteroid (0 with no target). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	float GetDroneCutRate() const;

	/** Ore units per second all cutting drones are pulling from the locked asteroid right now. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	float GetExtractionRate() const { return GetDroneCutRate() * GetDronesCutting(); }

	/** True when the hold has no room for another unit of the target's ore (or of anything, with no target). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	bool IsHoldFull() const;

	/** Ore item most recently delivered to the hold, or null. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	UTradeItemDataAsset* GetLastMinedOre() const { return LastMinedOre.Get(); }

	/** Units delivered to the hold by the most recent delivery. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	int32 GetLastMinedAmount() const { return LastMinedAmount; }

	/** Seconds since ore was last delivered to the hold (large if never). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones")
	float GetSecondsSinceLastMined() const;

	// ---- Cargo transfer ----

	/** Cargo volume one cargo drone carries per trip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones|Cargo", meta=(ClampMin="1.0"))
	float CargoDroneLoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drones|Cargo")
	TSubclassOf<ACargoDrone> CargoDroneClass;

	/** Whether cargo can be moved with Other now; OutReason says why not. */
	bool CanTransferWith(const ASpaceship* Other, bool bTake, FText* OutReason = nullptr) const;

	/**
	 * Start moving cargo with Other: bTake pulls its hold into ours, otherwise ours goes
	 * to it (only OnlyItem, if set). Recalls mining drones. Returns false (see
	 * CanTransferWith) if the transfer isn't allowed.
	 */
	bool StartCargoTransfer(ASpaceship* Other, bool bTake, FName OnlyItem = NAME_None);

	/** Stop sending drones; the ones out bring their loads home. */
	UFUNCTION(BlueprintCallable, Category="Drones|Cargo")
	void StopCargoTransfer();

	/** A transfer job is on, or cargo drones are still out. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drones|Cargo")
	bool IsTransferring() const;

	bool IsTransferTarget(const ASpaceship* Ship) const;
	ASpaceship* GetTransferTarget() const;
	bool IsTransferTake() const { return bTransferTake; }

	/** Units moved by the current (or last) transfer. */
	int32 GetTransferredUnits() const { return TransferredUnits; }

	/** Organisation that goods taken from Other would be stolen from (None if they're fair to take). */
	FName GetStolenTagFor(const ASpaceship* Other) const;

	/** Fired when a transfer ends and every drone is home (units moved). */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCargoTransferFinished, int32);
	FOnCargoTransferFinished OnCargoTransferFinished;

	// ---- Used by ACargoDrone ----
	void NotifyCargoDroneAtTarget(ACargoDrone* Drone, ASpaceship* Other);
	void NotifyCargoDroneHome(ACargoDrone* Drone);
	/** Load a landed drone for its next trip; false if the job is over. */
	bool ReloadCargoDrone(ACargoDrone* Drone);
	void NotifyCargoDroneStowed(ACargoDrone* Drone);

	// ---- Used by AShipDrone ----

	/** Rock the drones should be working, or null when they should come home. */
	AAsteroid* GetWorkTarget() const;

	/** Where drones leave and land (the hatch). */
	FVector GetHatchLocation() const { return GetComponentLocation(); }

	/** Point outside the hatch drones pass through on the way in and out. */
	FVector GetHatchApproach() const;

	/** Owner's velocity, so drones can match it while landing. */
	FVector GetShipVelocity() const;

	/** A landed drone hands over its hopper; whole units go to the hold, the rest waits in the bay. */
	void UnloadOre(UTradeItemDataAsset* Ore, float Amount);

	/** A landed drone is stowed (destroyed); forget it. */
	void NotifyDroneStowed(AShipDrone* Drone);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> Target;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AShipDrone>> Drones;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACargoDrone>> CargoDrones;

	TWeakObjectPtr<ASpaceship> TransferTarget;
	bool bTransferActive = false;
	bool bTransferTake = true;
	FName TransferItem;
	FName TransferStolenFrom;
	int32 TransferredUnits = 0;
	float CargoLaunchCooldown = 0.0f;

	/** Ore landed but not yet in the hold (part units, or no room). */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UTradeItemDataAsset>, float> BayOre;

	EDroneBayStatus Status;
	bool bDeployed;
	/** Distance (cm) below the hatch that drones pass through, clear of the hull. */
	float HatchClearance;
	float LaunchCooldown;

	TWeakObjectPtr<UTradeItemDataAsset> LastMinedOre;
	int32 LastMinedAmount;
	double LastMinedTime;

	UCargoComponent* GetCargo() const;
	FVector GetAimForward() const;
	void SetStatus(EDroneBayStatus NewStatus);
	void FlushBayOre();
	void LaunchOneDrone();

	void TickCargoTransfer(float DeltaTime);
	/** Something is left to move and there's room for it (counting loads in flight). */
	bool HasTransferWorkLeft() const;
	/** Volume aboard drones heading for the receiving hold. */
	float GetInboundVolume() const;
	/** Fill Drone's pod from the giving hold (the other ship's when taking, ours when giving). */
	void FillCargoDrone(ACargoDrone* Drone, UCargoComponent* From, UCargoComponent* To);
	/** Empty Drone's pod into a hold; what doesn't fit stays in the pod. Returns units moved. */
	int32 EmptyCargoDrone(ACargoDrone* Drone, UCargoComponent* Into);
	void LaunchCargoDrone();
};
