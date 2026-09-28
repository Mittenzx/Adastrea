// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "UI/StationEditorButton.h"

UStationEditorButton::UStationEditorButton()
{
	// Clicking a tile mustn't pull keyboard focus off the editor (its hotkeys).
	InitIsFocusable(false);
}

TSharedRef<SWidget> UStationEditorButton::RebuildWidget()
{
	OnClicked.AddUniqueDynamic(this, &UStationEditorButton::HandleClicked);
	return Super::RebuildWidget();
}

void UStationEditorButton::HandleClicked()
{
	OnButtonClicked.ExecuteIfBound(this);
}
