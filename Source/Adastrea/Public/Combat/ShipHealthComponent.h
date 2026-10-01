#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShipHealthComponent.generated.h"

class ASpaceship;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShipDamaged, float, ShieldDamage, float, HullDamage, AActor*, DamageInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShipDestroyed, AActor*, DamageInstigator);

/**
 * A ship's shields and the damage it takes. Shields soak damage first and recharge
 * after a quiet spell; what gets through hits the hull (ASpaceship::CurrentHullIntegrity),
 * reduced by the data asset's ArmorRating.
 *
 * Ship hulls fly with collision off, so hits are tested against the hull's bounding
 * box (SegmentHit) rather than physics. Training targets (bTrainingTarget, used for
 * the CombatTest dummies) vanish when destroyed and come back at full strength.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UShipHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShipHealthComponent();

	/** Seconds without a hit before the shields start recharging. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(ClampMin="0.0"))
	float ShieldRechargeDelay;

	/** Shield points recharged per second; the data asset's ShieldRechargeRate replaces it when set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(ClampMin="0.0"))
	float ShieldRechargeRate;

	/** Respawn at full strength a few seconds after being destroyed (combat test dummies). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	bool bTrainingTarget;

	/** Seconds a destroyed training target stays gone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(ClampMin="0.0"))
	float TrainingRespawnDelay;

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnShipDamaged OnDamaged;

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnShipDestroyed OnDestroyed;

	/** Deal damage: shields first, the rest to the hull after armor. Returns the damage taken. */
	UFUNCTION(BlueprintCallable, Category="Combat")
	float ApplyDamage(float Amount, AActor* DamageInstigator);

	/** Full shields and hull; clears the destroyed state. */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void Restore();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat")
	float GetShield() const { return CurrentShield; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat")
	float GetMaxShield() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat")
	bool IsDestroyed() const { return bDestroyed; }

	/** Seconds since the last hit (large if never hit). */
	float GetTimeSinceHit() const;

	/** Whether the segment A->B enters the hull box; OutHit is the entry point. */
	bool SegmentHit(const FVector& A, const FVector& B, FVector& OutHit, float& OutTime) const;

	ASpaceship* GetShip() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Hull box in the ShipRoot frame (unscaled), cached on first use. */
	const FBox& GetHitBox() const;

	float CurrentShield;
	float LastMaxShield;
	double LastHitTime;
	double DestroyedTime;
	bool bDestroyed;

	mutable FBox CachedHitBox;
	mutable bool bHitBoxCached;
};
