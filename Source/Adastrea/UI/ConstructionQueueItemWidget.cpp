// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "ConstructionQueueItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"

// StationEditor module includes
#include "StationEditor/StationEditorManager.h"

UConstructionQueueItemWidget::UConstructionQueueItemWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ItemNameText(nullptr)
	, BuildProgressBar(nullptr)
	, TimeRemainingText(nullptr)
	, CancelButton(nullptr)
	, QueueId(-1)
{
}

void UConstructionQueueItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind cancel button
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UConstructionQueueItemWidget::OnCancelButtonClicked);
	}
}

void UConstructionQueueItemWidget::NativeDestruct()
{
	// Unbind cancel button
	if (CancelButton)
	{
		CancelButton->OnClicked.RemoveDynamic(this, &UConstructionQueueItemWidget::OnCancelButtonClicked);
	}

	Super::NativeDestruct();
}

void UConstructionQueueItemWidget::SetQueueData(const FConstructionQueueItem& Item)
{
	QueueId = Item.QueueId;

	// Get module name from class default object
	if (ItemNameText)
	{
		FText ModuleName = FText::FromString(TEXT("Module")); // Default fallback
		
		if (Item.ModuleClass)
		{
			ASpaceStationModule* CDO = Item.ModuleClass->GetDefaultObject<ASpaceStationModule>();
			// Null check CDO before accessing its properties
			if (CDO)
			{
				if (!CDO->ModuleType.IsEmpty())
				{
					ModuleName = FText::FromString(CDO->ModuleType);
				}
			}
		}
		
		ItemNameText->SetText(ModuleName);
	}

	// Update progress
	UpdateProgress(Item.GetProgress(), Item.TimeRemaining);
}

void UConstructionQueueItemWidget::UpdateProgress(float Progress, float TimeRemaining)
{
	// Update progress bar
	if (BuildProgressBar)
	{
		BuildProgressBar->SetPercent(Progress);
	}

	// Update time remaining text
	if (TimeRemainingText)
	{
		int32 Minutes = FMath::FloorToInt(TimeRemaining / 60.0f);
		int32 Seconds = FMath::FloorToInt(TimeRemaining) % 60;
		
		FText TimeText = FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
		TimeRemainingText->SetText(TimeText);
	}
}

void UConstructionQueueItemWidget::OnCancelButtonClicked()
{
	if (OnCancelled.IsBound() && QueueId >= 0)
	{
		OnCancelled.Execute(QueueId);
	}
}
