// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "StationEditorButton.generated.h"

class ASpaceStationModule;
class UStationEditorButton;

DECLARE_DELEGATE_OneParam(FOnStationEditorButtonClicked, UStationEditorButton*);

/**
 * A button that remembers what it stands for (a palette module or a queue entry)
 * and says which button it is when clicked. The Station Editor builds its
 * overlay from these in C++; UButton::OnClicked carries no payload.
 */
UCLASS()
class STATIONEDITOR_API UStationEditorButton : public UButton
{
	GENERATED_BODY()

public:
	UStationEditorButton();

	/** Palette tiles: the module this tile builds */
	UPROPERTY()
	TSubclassOf<ASpaceStationModule> ModuleClass;

	/** Queue rows: the construction queue entry this row cancels */
	int32 QueueId = INDEX_NONE;

	FOnStationEditorButtonClicked OnButtonClicked;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void HandleClicked();
};
