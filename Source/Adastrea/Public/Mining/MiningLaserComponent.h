#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MiningLaserComponent.generated.h"

class AAsteroid;
class UCargoComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UNiagaraSystem;
class UNiagaraComponent;
class UTradeItemDataAsset;

UENUM(BlueprintType)
enum class EMiningStatus : uint8
{
	Disabled   UMETA(DisplayName="No Mining Laser"),
	Idle       UMETA(DisplayName="Idle"),
	NoTarget   UMETA(DisplayName="No Target"),
	OutOfRange UMETA(DisplayName="Out Of Range"),
	OffAim     UMETA(DisplayName="Off Aim"),
	CargoFull  UMETA(DisplayName="Cargo Full"),
	Mining     UMETA(DisplayName="Mining")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOreMined, UTradeItemDataAsset*, Ore, int32, Amount);

/**
 * Ship-mounted mining laser. Needs a locked asteroid (ITargetable), in range and
 * roughly in front of the muzzle. While the trigger is held it drains ore from
 * the asteroid and moves whole units into the owner's UCargoComponent, stopping
 * when the hold is full. Attach it to a hardpoint (socket / relative offset).
 */
UCLASS(ClassGroup=(Mining), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UMiningLaserComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UMiningLaserComponent();

	/** Master switch; ships without a mining laser leave this off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mining")
	bool bMiningEnabled;

	/** Maximum distance (cm) from the muzzle to the asteroid surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mining", meta=(ClampMin="100.0"))
	float Range;

	/** How far away (cm) an asteroid can be locked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mining", meta=(ClampMin="100.0"))
	float LockRange;

	/** Mining power multiplier (1 = reference; divided by rock hardness). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mining", meta=(ClampMin="0.0"))
	float MiningPower;

	/** Beam only works while the target is within this cone (degrees) of the ship's nose. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mining", meta=(ClampMin="1.0", ClampMax="180.0"))
	float MaxAimAngle;

	/** Beam thickness in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mining|Visual", meta=(ClampMin="0.1"))
	float BeamWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mining|Visual")
	TSoftObjectPtr<UMaterialInterface> BeamMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mining|Visual")
	TSoftObjectPtr<UNiagaraSystem> ImpactDust;

	/** Ore units delivered to cargo since the ship spawned (for the HUD). */
	UPROPERTY(BlueprintReadOnly, Category="Mining")
	int32 SessionOreMined;

	UPROPERTY(BlueprintAssignable, Category="Mining")
	FOnOreMined OnOreMined;

	/** Lock a specific asteroid. Returns false if it cannot be targeted. */
	UFUNCTION(BlueprintCallable, Category="Mining")
	bool SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category="Mining")
	void ClearTarget();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Mining")
	AActor* GetTarget() const;

	/** Lock the asteroid closest to the given aim ray (within LockRange). */
	UFUNCTION(BlueprintCallable, Category="Mining")
	bool LockBestTarget(FVector Origin, FVector AimDirection);

	/** Lock the asteroid nearest to the ship's nose within the aim cone; used by the fire input if nothing is locked. */
	UFUNCTION(BlueprintCallable, Category="Mining")
	bool LockNearestAhead();

	UFUNCTION(BlueprintCallable, Category="Mining")
	void StartMining();

	UFUNCTION(BlueprintCallable, Category="Mining")
	void StopMining();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Mining")
	bool IsMining() const { return Status == EMiningStatus::Mining; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Mining")
	bool IsTriggerHeld() const { return bTriggerHeld; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Mining")
	EMiningStatus GetStatus() const { return Status; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Mining")
	static FText StatusToText(EMiningStatus InStatus);

	/** Distance (cm) from the muzzle to the target's surface, or -1 with no target. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Mining")
	float GetTargetSurfaceDistance() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> Target;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> BeamMesh;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> DustFX;

	EMiningStatus Status;
	bool bTriggerHeld;

	/** Fractional ore not yet delivered as a whole unit. */
	float CarriedOre;

	UCargoComponent* GetCargo() const;
	FVector GetMuzzleLocation() const { return GetComponentLocation(); }
	FVector GetAimForward() const;
	void SetBeamActive(bool bActive, const FVector& HitPoint = FVector::ZeroVector);
	void SetStatus(EMiningStatus NewStatus);
};
