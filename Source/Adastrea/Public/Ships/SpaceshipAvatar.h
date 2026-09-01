// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceshipAvatar.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * Third-person avatar the player walks ship interiors with.
 *
 * The player leaves the cockpit (from the ship) and possesses this character to
 * walk the interior. WASD moves, mouse looks, an "interact/enter cockpit" action
 * returns possession to the ship.
 */
UCLASS()
class ADASTREA_API ASpaceshipAvatar : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceshipAvatar();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** The ship we left, which owns the interior we're standing in. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	TObjectPtr<class ASpaceship> SourceShip;

	/** The interior scene we are currently inside. */
	UPROPERTY(BlueprintReadWrite, Category="Avatar")
	TObjectPtr<class ASpaceshipInterior> CurrentInterior;

	/** Return possession to the ship at its saved cockpit transform. */
	UFUNCTION(BlueprintCallable, Category="Avatar")
	void SitDown();

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void MoveForward();
	void MoveBack();
	void MoveLeft();
	void MoveRight();
	void Turn(float Value);
	void LookUp(float Value);

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- Input ---
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SitDownAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> AvatarMappingContext;

private:
	bool bIsOnFoot = false;
};