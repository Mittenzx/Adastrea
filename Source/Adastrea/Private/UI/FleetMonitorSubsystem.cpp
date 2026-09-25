// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FleetMonitorSubsystem.h"
#include "AdastreaLog.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Player/AdastreaPlayerController.h"
#include "UI/FleetMonitorWidget.h"

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GFleetMonitorCommand(
		TEXT("adastrea.FleetMonitor"),
		TEXT("Toggle the fleet monitor: every ship and station in the level with pilot, credits, cargo and current objective (also F10). "
			"Usage: adastrea.FleetMonitor [expand|collapse]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UFleetMonitorSubsystem* Monitor = World ? World->GetSubsystem<UFleetMonitorSubsystem>() : nullptr;
			if (!Monitor)
			{
				return;
			}
			const FString Mode = Args.IsEmpty() ? FString() : Args[0];
			if (Mode.Equals(TEXT("expand"), ESearchCase::IgnoreCase) || Mode.Equals(TEXT("collapse"), ESearchCase::IgnoreCase))
			{
				Monitor->SetMonitorVisible(true);
				Monitor->SetAllExpanded(Mode.Equals(TEXT("expand"), ESearchCase::IgnoreCase));
			}
			else
			{
				Monitor->ToggleMonitor();
			}
		}));
}

bool UFleetMonitorSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UFleetMonitorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFleetMonitorSubsystem, STATGROUP_Tickables);
}

void UFleetMonitorSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PC && PC->WasInputKeyJustPressed(ToggleKey))
	{
		ToggleMonitor();
	}
}

void UFleetMonitorSubsystem::Deinitialize()
{
	if (Widget)
	{
		Widget->RemoveFromParent();
		Widget = nullptr;
	}
	Super::Deinitialize();
}

bool UFleetMonitorSubsystem::IsMonitorVisible() const
{
	return Widget && Widget->IsInViewport();
}

void UFleetMonitorSubsystem::SetAllExpanded(bool bExpanded)
{
	if (Widget)
	{
		Widget->SetAllExpanded(bExpanded);
	}
}

void UFleetMonitorSubsystem::ToggleMonitor()
{
	SetMonitorVisible(!IsMonitorVisible());
}

void UFleetMonitorSubsystem::SetMonitorVisible(bool bVisible)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC || bVisible == IsMonitorVisible())
	{
		return;
	}
	AAdastreaPlayerController* AdastreaPC = Cast<AAdastreaPlayerController>(PC);

	if (bVisible)
	{
		if (!Widget)
		{
			Widget = CreateWidget<UFleetMonitorWidget>(PC, UFleetMonitorWidget::StaticClass());
		}
		if (!Widget)
		{
			return;
		}
		Widget->AddToViewport(50);
		Widget->Refresh();

		// Cursor on so rows can be clicked; leave things alone if another screen already turned it on.
		bChangedInputMode = !PC->bShowMouseCursor;
		if (bChangedInputMode)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
			if (AdastreaPC)
			{
				AdastreaPC->bLockMouseLook = true;
			}
		}
		UE_LOG(LogAdastrea, Log, TEXT("Fleet monitor opened"));
	}
	else
	{
		Widget->RemoveFromParent();
		if (bChangedInputMode)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
			if (AdastreaPC)
			{
				AdastreaPC->bLockMouseLook = false;
			}
			bChangedInputMode = false;
		}
		UE_LOG(LogAdastrea, Log, TEXT("Fleet monitor closed"));
	}
}
