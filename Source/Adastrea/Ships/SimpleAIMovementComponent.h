#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimpleAIMovementComponent.generated.h"

/**
 * Simple AI Movement Component for autonomous ship navigation
 * 
 * This component provides basic AI movement behavior:
 * - Generates random target locations
 * - Moves ship toward target
 * - Generates new target when arrived
 * 
 * Usage:
 * - Add this component to any Pawn/Ship Blueprint
 * - Configure movement parameters in editor
 * - Component handles movement automatically when ship is possessed by AI
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class ADASTREA_API USimpleAIMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USimpleAIMovementComponent();

	// Minimum distance for random location generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Movement", meta=(ClampMin="1000.0"))
	float MinDistance;

	// Maximum distance for random location generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Movement", meta=(ClampMin="1000.0"))
	float MaxDistance;

	// Speed at which ship moves toward target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Movement", meta=(ClampMin="100.0"))
	float MoveSpeed;

	// Distance at which we consider we've arrived at target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Movement", meta=(ClampMin="100.0"))
	float ArrivalThreshold;

	// Turn rate for rotating toward target (degrees per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Movement", meta=(ClampMin="10.0"))
	float TurnRate;

	// Whether to constrain rotation to horizontal plane (ignore Z-axis when looking at target)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Movement")
	bool bConstrainToHorizontalPlane;

	// Whether to enable debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Movement|Debug")
	bool bShowDebug;

	/**
	 * Generate a new random target location
	 */
	UFUNCTION(BlueprintCallable, Category="AI Movement")
	void GenerateNewTarget();

	/**
	 * Get the current target location
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AI Movement")
	FVector GetTargetLocation() const { return TargetLocation; }

	/**
	 * Get distance to current target
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AI Movement")
	float GetDistanceToTarget() const;

	/**
	 * Check if we've arrived at the target
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AI Movement")
	bool HasArrivedAtTarget() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Current target location
	UPROPERTY()
	FVector TargetLocation;

	/**
	 * Move toward the current target
	 */
	void MoveTowardTarget(float DeltaTime);

	/**
	 * Rotate toward the target
	 */
	void RotateTowardTarget(float DeltaTime);
};
