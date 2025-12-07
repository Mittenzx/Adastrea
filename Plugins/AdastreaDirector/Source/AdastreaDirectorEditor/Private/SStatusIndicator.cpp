// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "SStatusIndicator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "StatusIndicator"

void SStatusIndicator::Construct(const FArguments& InArgs)
{
	CurrentStatus = InArgs._InitialStatus;

	ChildSlot
	[
		SNew(SHorizontalBox)
		
		// Status light (colored circle)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(12.0f)
			.HeightOverride(12.0f)
			[
				SAssignNew(StatusLightBlock, STextBlock)
				.Text(this, &SStatusIndicator::GetStatusIndicator)
				.ColorAndOpacity(this, &SStatusIndicator::GetStatusColor)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
		]
		
		// Status text label
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SAssignNew(StatusTextBlock, STextBlock)
			.Text(InArgs._StatusText)
		]
	];
}

void SStatusIndicator::SetStatus(EStatus NewStatus)
{
	CurrentStatus = NewStatus;
}

void SStatusIndicator::SetStatus(EStatus NewStatus, const FText& NewText)
{
	CurrentStatus = NewStatus;
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(NewText);
	}
}

FSlateColor SStatusIndicator::GetStatusColor() const
{
	switch (CurrentStatus)
	{
		case EStatus::Good:
			return FLinearColor(0.0f, 0.8f, 0.0f); // Green
		case EStatus::Warning:
			return FLinearColor(1.0f, 0.8f, 0.0f); // Yellow/Orange
		case EStatus::Error:
			return FLinearColor(1.0f, 0.0f, 0.0f); // Red
		case EStatus::Unknown:
		default:
			return FLinearColor(0.5f, 0.5f, 0.5f); // Gray
	}
}

FText SStatusIndicator::GetStatusIndicator() const
{
	// Use a filled circle character (●) as the indicator
	// Unicode U+25CF is widely supported on modern systems (Windows, Mac, Linux)
	// and provides a clear visual indicator for status lights
	return FText::FromString(TEXT("●"));
}

#undef LOCTEXT_NAMESPACE
