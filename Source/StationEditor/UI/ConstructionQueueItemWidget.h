// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "ConstructionQueueItemWidget.generated.h"

// Forward declarations
struct FConstructionQueueItem;

/**
 * Delegate for queue item cancellation
 */
DECLARE_DELEGATE_OneParam(FOnQueueItemCancelled, int32);

/**
 * Construction Queue Item Widget
 * 
 * Displays a single item in the construction queue.
 * Shows module name, build progress, time remaining, and cancel button.
 * 
 * Usage:
 * 1. Create Blueprint widget based on this class
 * 2. Layout UI with named widgets (must match BindWidget properties)
 * 3. Set queue data via SetQueueData()
 * 4. Bind OnCancelled delegate to handle cancellation
 */
UCLASS()
class STATIONEDITOR_API UConstructionQueueItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UConstructionQueueItemWidget(const FObjectInitializer& ObjectInitializer);

	// =====================
	// UI Widget References (BindWidget)
	// =====================

	/** Text block displaying item name */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ItemNameText;

	/** Progress bar showing build progress */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* BuildProgressBar;

	/** Text block displaying time remaining */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TimeRemainingText;

	/** Button to cancel this queue item */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* CancelButton;

	// =====================
	// Data
	// =====================

	/** The queue ID for this item */
	UPROPERTY()
	int32 QueueId;

	/** Delegate called when item is cancelled */
	FOnQueueItemCancelled OnCancelled;

	// =====================
	// Public Functions
	// =====================

	/**
	 * Set the queue data for this item
	 * @param Item The construction queue item data
	 */
	void SetQueueData(const FConstructionQueueItem& Item);

	/**
	 * Update the progress display
	 * @param Progress Progress value (0.0 to 1.0)
	 * @param TimeRemaining Time remaining in seconds
	 */
	void UpdateProgress(float Progress, float TimeRemaining);

protected:
	/** Called when the widget is constructed */
	virtual void NativeConstruct() override;

	/** Called when the widget is destroyed */
	virtual void NativeDestruct() override;

	/**
	 * Handle cancel button clicked
	 */
	UFUNCTION()
	void OnCancelButtonClicked();
};
