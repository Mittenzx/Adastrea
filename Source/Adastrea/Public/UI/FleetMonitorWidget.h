// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "FleetMonitorWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UVerticalBox;
class ASpaceship;
class ASpaceStation;

DECLARE_DELEGATE_OneParam(FOnFleetMonitorRowClicked, AActor* /*Target*/);

/**
 * Header button for one row of the fleet monitor. Not focusable, so clicking a row
 * doesn't steal keyboard focus from ship flight (Space/Enter would re-press it).
 */
UCLASS()
class ADASTREA_API UFleetMonitorRowButton : public UButton
{
	GENERATED_BODY()

public:
	UFleetMonitorRowButton(const FObjectInitializer& ObjectInitializer);

	/** Actor this row describes. */
	TWeakObjectPtr<AActor> Target;

	FOnFleetMonitorRowClicked OnRowClicked;

private:
	UFUNCTION()
	void HandleClicked();
};

/**
 * Debug screen listing every ship and station in the level. Each row shows a one-line
 * summary; click it to expand pilot, credits, cargo, current objective and so on.
 * Built entirely in C++ (no Blueprint asset needed) and refreshed a few times a second.
 *
 * Toggle in PIE with F10 or the console command `adastrea.FleetMonitor`
 * (see UFleetMonitorSubsystem).
 */
UCLASS()
class ADASTREA_API UFleetMonitorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Seconds between refreshes of the listed data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fleet Monitor", meta=(ClampMin="0.05"))
	float RefreshInterval = 0.25f;

	/** Rebuild the list from the world right now. */
	UFUNCTION(BlueprintCallable, Category="Fleet Monitor")
	void Refresh();

	/** Expand (true) or collapse (false) every row. */
	UFUNCTION(BlueprintCallable, Category="Fleet Monitor")
	void SetAllExpanded(bool bExpanded);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	struct FRow
	{
		TWeakObjectPtr<UFleetMonitorRowButton> Header;
		TWeakObjectPtr<UTextBlock> HeaderText;
		TWeakObjectPtr<UTextBlock> Details;
		TWeakObjectPtr<UVerticalBox> Container;
		bool bExpanded = false;
	};

	void BuildLayout();

	/** Find or create the row for Actor under Section. */
	FRow& GetOrAddRow(AActor* Actor, UVerticalBox* Section);

	void UpdateRow(FRow& Row, const FString& Summary, const FString& Details);

	void HandleRowClicked(AActor* Target);

	UFUNCTION()
	void HandleExpandAll();

	UFUNCTION()
	void HandleCollapseAll();

	FString DescribeShipSummary(const ASpaceship* Ship) const;
	FString DescribeShipDetails(const ASpaceship* Ship) const;
	FString DescribeStationSummary(const ASpaceStation* Station, const TArray<ASpaceship*>& Ships) const;
	FString DescribeStationDetails(const ASpaceStation* Station, const TArray<ASpaceship*>& Ships) const;

	UTextBlock* MakeText(const FString& Text, int32 Size, const FLinearColor& Color);
	UFleetMonitorRowButton* MakeButton(UWidget* Content);

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ShipsHeading;

	UPROPERTY()
	TObjectPtr<UTextBlock> StationsHeading;

	UPROPERTY()
	TObjectPtr<UVerticalBox> ShipList;

	UPROPERTY()
	TObjectPtr<UVerticalBox> StationList;

	TMap<TWeakObjectPtr<AActor>, FRow> Rows;

	float TimeSinceRefresh = 0.0f;
};
