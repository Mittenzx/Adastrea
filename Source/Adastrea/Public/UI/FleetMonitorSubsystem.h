// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "FleetMonitorSubsystem.generated.h"

class UFleetMonitorWidget;

/**
 * Owns the fleet monitor debug screen (UFleetMonitorWidget) for game/PIE worlds.
 * Press ToggleKey (F10) or run `adastrea.FleetMonitor` in the console to show/hide it.
 * While open the mouse cursor is shown and ship mouse-look is paused so rows can be
 * clicked; keyboard flight keeps working.
 */
UCLASS()
class ADASTREA_API UFleetMonitorSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Key that toggles the monitor. */
	FKey ToggleKey = EKeys::F10;

	UFUNCTION(BlueprintCallable, Category="Fleet Monitor")
	void ToggleMonitor();

	UFUNCTION(BlueprintCallable, Category="Fleet Monitor")
	void SetMonitorVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category="Fleet Monitor")
	bool IsMonitorVisible() const;

	/** Expand (true) or collapse (false) every row of the open monitor. */
	UFUNCTION(BlueprintCallable, Category="Fleet Monitor")
	void SetAllExpanded(bool bExpanded);

	// UTickableWorldSubsystem
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Deinitialize() override;

private:
	UPROPERTY()
	TObjectPtr<UFleetMonitorWidget> Widget;

	/** True when opening the monitor switched the cursor on (so closing it switches it back off). */
	bool bChangedInputMode = false;
};
