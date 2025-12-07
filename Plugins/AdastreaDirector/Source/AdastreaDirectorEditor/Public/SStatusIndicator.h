// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class STextBlock;

/**
 * Status indicator widget that displays a colored status light with label.
 * Used to visualize the health of plugin components.
 */
class SStatusIndicator : public SCompoundWidget
{
public:
	/** Status states that can be displayed */
	enum class EStatus : uint8
	{
		/** Component is working correctly */
		Good,
		/** Component has warnings but is functional */
		Warning,
		/** Component has errors or is not functional */
		Error,
		/** Component status is unknown or checking */
		Unknown
	};

	SLATE_BEGIN_ARGS(SStatusIndicator)
		: _StatusText(FText::FromString(TEXT("Status")))
		, _InitialStatus(EStatus::Unknown)
		{}
		/** The label text to display next to the status light */
		SLATE_ATTRIBUTE(FText, StatusText)
		/** Initial status state */
		SLATE_ARGUMENT(EStatus, InitialStatus)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** Updates the status of this indicator */
	void SetStatus(EStatus NewStatus);

	/** Updates the status and text */
	void SetStatus(EStatus NewStatus, const FText& NewText);

	/** Gets the current status */
	EStatus GetStatus() const { return CurrentStatus; }

private:
	/** Current status state */
	EStatus CurrentStatus;

	/** Status text block */
	TSharedPtr<STextBlock> StatusTextBlock;

	/** Status indicator text (the colored circle) */
	TSharedPtr<STextBlock> StatusLightBlock;

	/** Get the color for the current status */
	FSlateColor GetStatusColor() const;

	/** Get the status indicator character */
	FText GetStatusIndicator() const;
};
