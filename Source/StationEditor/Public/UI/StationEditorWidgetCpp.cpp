// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "UI/StationEditorWidgetCpp.h"
#include "UI/ModuleListItemWidget.h"
#include "UI/ConstructionQueueItemWidget.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "AdastreaLog.h"
#include "StationBuildPreview.h"
#include "InputCoreTypes.h"

// StationEditor module includes
#include "StationEditorManager.h"
#include "StationModuleCatalog.h"

UStationEditorWidgetCpp::UStationEditorWidgetCpp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ModuleListScrollBox(nullptr)
	, PowerDisplayText(nullptr)
	, ModuleCountDisplay(nullptr)
	, PowerBalanceBar(nullptr)
	, CloseButton(nullptr)
	, QueueScrollBox(nullptr)
	, PlacementStatusText(nullptr)
	, CreditsText(nullptr)
	, ModuleCatalog(nullptr)
	, CurrentStation(nullptr)
	, EditorManager(nullptr)
	, bIsInPlacementMode(false)
	, PendingPlacementModule(nullptr)
	, bPreviewPositioned(false)
	, LastPlacementResult(EModulePlacementResult::Success)
{
	// Needed for the editor hotkeys (NativeOnKeyDown).
	SetIsFocusable(true);
}

void UStationEditorWidgetCpp::NativeConstruct()
{
	Super::NativeConstruct();

	// Ensure EditorManager exists
	EnsureEditorManager();

	// Bind close button
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UStationEditorWidgetCpp::OnCloseButtonClicked);
	}

	// Bind to EditorManager events
	if (EditorManager)
	{
		EditorManager->OnModulePlaced.AddUniqueDynamic(this, &UStationEditorWidgetCpp::OnManagerModulePlaced);
		EditorManager->OnModuleRemoved.AddUniqueDynamic(this, &UStationEditorWidgetCpp::OnManagerModuleRemoved);
		EditorManager->OnStatisticsUpdated.AddUniqueDynamic(this, &UStationEditorWidgetCpp::OnManagerStatisticsUpdated);
		EditorManager->OnConstructionQueueChanged.AddUniqueDynamic(this, &UStationEditorWidgetCpp::OnManagerQueueChanged);
		EditorManager->OnNotificationAdded.AddUniqueDynamic(this, &UStationEditorWidgetCpp::OnManagerNotificationAdded);
	}

	// Initial refresh
	RefreshModuleList();
	RefreshStatistics();
	UpdateConstructionQueue();
	UpdateStatusText();

	SetKeyboardFocus();
}

void UStationEditorWidgetCpp::NativeDestruct()
{
	// However the widget went away (Close button, the G toggle, a level change),
	// end the editing session: commit what was built and drop the preview actor.
	// Before this, closing with G left the session open and the preview alive.
	if (bIsInPlacementMode)
	{
		ExitPlacementMode();
	}
	if (EditorManager && EditorManager->bIsEditing)
	{
		EditorManager->Save();
	}

	// Unbind events
	if (EditorManager)
	{
		EditorManager->OnModulePlaced.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerModulePlaced);
		EditorManager->OnModuleRemoved.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerModuleRemoved);
		EditorManager->OnStatisticsUpdated.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerStatisticsUpdated);
		EditorManager->OnConstructionQueueChanged.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerQueueChanged);
		EditorManager->OnNotificationAdded.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerNotificationAdded);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UStationEditorWidgetCpp::OnCloseButtonClicked);
	}

	Super::NativeDestruct();
}

void UStationEditorWidgetCpp::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update construction progress
	if (EditorManager)
	{
		EditorManager->UpdateConstruction(InDeltaTime);
	}

	// Update preview position if in placement mode
	if (bIsInPlacementMode)
	{
		UpdatePreviewPosition();
	}

	UpdateStatusText();
}

void UStationEditorWidgetCpp::InitializeEditor(ASpaceStation* Station, UStationModuleCatalog* Catalog)
{
	CurrentStation = Station;
	ModuleCatalog = Catalog;

	// Ensure EditorManager exists
	EnsureEditorManager();

	// Configure EditorManager
	if (EditorManager)
	{
		// Keep the manager's own fallback (DA_StationModuleCatalog, loaded in
		// BeginEditing) when the caller has no catalog to give.
		if (Catalog)
		{
			EditorManager->ModuleCatalog = Catalog;
		}
		EditorManager->PlayerTechLevel = DefaultPlayerTechLevel;
		EditorManager->PlayerCredits = DefaultPlayerCredits;

		// Begin editing the station
		if (Station)
		{
			EditorManager->BeginEditing(Station);
		}
	}

	PlacementRotation = FRotator::ZeroRotator;
	LastNotificationText = FText::GetEmpty();

	// Refresh UI
	RefreshModuleList();
	RefreshStatistics();
	UpdateConstructionQueue();
}

void UStationEditorWidgetCpp::RefreshModuleList()
{
	if (!ModuleListScrollBox || !EditorManager)
	{
		return;
	}

	// Clear existing items
	ModuleListScrollBox->ClearChildren();

	// Get available modules
	TArray<FStationModuleEntry> AvailableModules = EditorManager->GetAvailableModules();

	// Create widget for each module
	for (const FStationModuleEntry& Entry : AvailableModules)
	{
		if (ModuleListItemClass)
		{
			UModuleListItemWidget* ItemWidget = CreateWidget<UModuleListItemWidget>(this, ModuleListItemClass);
			if (ItemWidget)
			{
				ItemWidget->SetModuleData(Entry);

				// Bind click event
				ItemWidget->OnModuleSelected.BindUObject(this, &UStationEditorWidgetCpp::OnModuleButtonClicked);

				ModuleListScrollBox->AddChild(ItemWidget);
			}
		}
	}
}

void UStationEditorWidgetCpp::RefreshStatistics()
{
	if (!EditorManager)
	{
		return;
	}

	FStationStatistics Stats = EditorManager->GetStationStatistics();

	// Update power display
	if (PowerDisplayText)
	{
		FText PowerText = FText::FromString(FString::Printf(TEXT("Power: %.0f / %.0f MW"),
			Stats.PowerGenerated, Stats.PowerConsumed));
		PowerDisplayText->SetText(PowerText);
	}

	// Update module count
	if (ModuleCountDisplay)
	{
		FText CountText = FText::FromString(FString::Printf(TEXT("Modules: %d / %d"),
			Stats.TotalModules, Stats.MaxModules));
		ModuleCountDisplay->SetText(CountText);
	}

	if (CreditsText)
	{
		CreditsText->SetText(FText::FromString(FString::Printf(TEXT("Credits: %d"), EditorManager->PlayerCredits)));
	}

	// Update power balance bar
	if (PowerBalanceBar)
	{
		// Calculate power balance as a percentage: 1.0 = sufficient power, <1.0 = deficit
		float BalancePercent = 0.5f; // Default to middle if no power consumption
		if (Stats.PowerConsumed > 0.0f)
		{
			BalancePercent = FMath::Clamp(Stats.PowerGenerated / Stats.PowerConsumed, 0.0f, 1.0f);
		}
		else if (Stats.PowerGenerated > 0.0f)
		{
			BalancePercent = 1.0f; // Surplus power with no consumption
		}

		PowerBalanceBar->SetPercent(BalancePercent);

		// Color based on balance
		if (Stats.PowerGenerated >= Stats.PowerConsumed)
		{
			PowerBalanceBar->SetFillColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			PowerBalanceBar->SetFillColorAndOpacity(FLinearColor::Red);
		}
	}
}

void UStationEditorWidgetCpp::UpdateConstructionQueue()
{
	if (!QueueScrollBox || !EditorManager)
	{
		return;
	}

	// Clear existing items
	QueueScrollBox->ClearChildren();

	// Get construction queue
	TArray<FConstructionQueueItem> Queue = EditorManager->GetConstructionQueue();

	// Create widget for each queue item
	for (const FConstructionQueueItem& Item : Queue)
	{
		if (QueueItemClass)
		{
			UConstructionQueueItemWidget* ItemWidget = CreateWidget<UConstructionQueueItemWidget>(this, QueueItemClass);
			if (ItemWidget)
			{
				ItemWidget->SetQueueData(Item);

				// Bind cancel event
				ItemWidget->OnCancelled.BindUObject(this, &UStationEditorWidgetCpp::OnQueueItemCancelled);

				QueueScrollBox->AddChild(ItemWidget);
			}
		}
	}
}

void UStationEditorWidgetCpp::OnModuleButtonClicked(TSubclassOf<ASpaceStationModule> ModuleClass)
{
	if (!ModuleClass)
	{
		return;
	}

	// Enter placement mode instead of placing immediately
	EnterPlacementMode(ModuleClass);

	// The list button took focus; take it back so the hotkeys work.
	SetKeyboardFocus();
}

void UStationEditorWidgetCpp::OnCloseButtonClicked()
{
	// Exit placement mode if active
	if (bIsInPlacementMode)
	{
		ExitPlacementMode();
	}

	// Save and end editing
	if (EditorManager && EditorManager->bIsEditing)
	{
		EditorManager->Save();
	}

	// Remove from parent and restore input
	RemoveFromParent();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

void UStationEditorWidgetCpp::OnManagerModulePlaced(ASpaceStationModule* Module)
{
	RefreshStatistics();
}

void UStationEditorWidgetCpp::OnManagerModuleRemoved(ASpaceStationModule* Module)
{
	RefreshStatistics();
}

void UStationEditorWidgetCpp::OnManagerStatisticsUpdated(const FStationStatistics& Statistics)
{
	RefreshStatistics();
}

void UStationEditorWidgetCpp::OnManagerQueueChanged()
{
	UpdateConstructionQueue();
	RefreshStatistics();
}

void UStationEditorWidgetCpp::OnQueueItemCancelled(int32 QueueId)
{
	if (EditorManager)
	{
		EditorManager->CancelConstruction(QueueId);
	}
}

void UStationEditorWidgetCpp::OnManagerNotificationAdded(const FStationNotification& Notification)
{
	LastNotificationText = Notification.Message;
	LastNotificationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	UpdateStatusText();
}

// =====================
// Placement Mode Functions
// =====================

void UStationEditorWidgetCpp::EnterPlacementMode(TSubclassOf<ASpaceStationModule> ModuleClass)
{
	if (!EditorManager || !ModuleClass)
	{
		return;
	}

	// Exit any existing placement mode
	if (bIsInPlacementMode)
	{
		ExitPlacementMode();
	}

	// Store selected module
	PendingPlacementModule = ModuleClass;
	bIsInPlacementMode = true;
	bPreviewPositioned = false;  // Reset positioning flag

	// Show preview with this module
	EditorManager->ShowPreview(ModuleClass);

	// Log with user-friendly module name instead of internal class name
	const ASpaceStationModule* ModuleCDO = ModuleClass->GetDefaultObject<ASpaceStationModule>();
	const FString ModuleDisplayName = (ModuleCDO && !ModuleCDO->ModuleType.IsEmpty())
		? ModuleCDO->ModuleType
		: ModuleClass->GetName();
	UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Entered placement mode for %s"),
		*ModuleDisplayName);
}

void UStationEditorWidgetCpp::ExitPlacementMode()
{
	if (!EditorManager)
	{
		return;
	}

	bIsInPlacementMode = false;
	PendingPlacementModule = nullptr;
	bPreviewPositioned = false;

	// Hide preview
	EditorManager->HidePreview();

	UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Exited placement mode"));
}

ASpaceStationModule* UStationEditorWidgetCpp::GetModuleUnderCursor(FHitResult* OutHit)
{
	if (!CurrentStation || !GetWorld())
	{
		return nullptr;
	}

	FVector WorldPosition, WorldDirection;
	if (!GetCursorWorldPosition(WorldPosition, WorldDirection))
	{
		return nullptr;
	}

	FCollisionQueryParams QueryParams;
	if (AActor* PlayerPawn = GetOwningPlayerPawn())
	{
		QueryParams.AddIgnoredActor(PlayerPawn);
	}

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, WorldPosition, WorldPosition + WorldDirection * MaxTraceDistance, ECC_Visibility, QueryParams))
	{
		return nullptr;
	}

	if (OutHit)
	{
		*OutHit = Hit;
	}

	ASpaceStationModule* Module = Cast<ASpaceStationModule>(Hit.GetActor());
	return (Module && CurrentStation->Modules.Contains(Module)) ? Module : nullptr;
}

bool UStationEditorWidgetCpp::GetPlacementTarget(FVector& OutPosition)
{
	if (!EditorManager || !CurrentStation || !PendingPlacementModule)
	{
		return false;
	}

	// First module: it becomes the station's core, so put it on the station origin
	// (which is also the build grid's origin).
	if (CurrentStation->Modules.Num() == 0)
	{
		OutPosition = CurrentStation->GetActorLocation();
		return true;
	}

	FHitResult Hit;
	ASpaceStationModule* HitModule = GetModuleUnderCursor(&Hit);
	if (HitModule)
	{
		// Build against the face under the cursor. (Snapping the raw surface hit
		// point usually landed inside the module that was clicked.)
		return EditorManager->FindAttachPosition(PendingPlacementModule, HitModule, Hit.ImpactNormal, OutPosition);
	}

	if (Hit.bBlockingHit)
	{
		// Something that isn't one of this station's modules - use the hit point
		// and let validation decide.
		OutPosition = Hit.Location;
		return true;
	}

	// Open space: where the cursor ray crosses the station's horizontal build plane.
	FVector WorldPosition, WorldDirection;
	if (!GetCursorWorldPosition(WorldPosition, WorldDirection) || FMath::IsNearlyZero(WorldDirection.Z, 1.e-3f))
	{
		return false;
	}

	const float Distance = (CurrentStation->GetActorLocation().Z - WorldPosition.Z) / WorldDirection.Z;
	if (Distance <= 0.0f || Distance > MaxTraceDistance)
	{
		return false;
	}

	OutPosition = WorldPosition + WorldDirection * Distance;
	return FVector::Dist(OutPosition, CurrentStation->GetActorLocation()) <= MaxPlacementDistance;
}

void UStationEditorWidgetCpp::UpdatePreviewPosition()
{
	if (!EditorManager || !CurrentStation || !PendingPlacementModule || !EditorManager->PreviewActor)
	{
		return;
	}

	FVector TargetPosition;
	if (!GetPlacementTarget(TargetPosition))
	{
		// Nowhere to put it in this direction: hide the preview.
		EditorManager->PreviewActor->Hide();
		bPreviewPositioned = false;
		LastPlacementResult = EModulePlacementResult::InvalidPosition;
		return;
	}

	// Snap exactly as PlaceModule() will, so the preview sits where the module will.
	FVector SnappedPosition;
	FRotator SnappedRotation;
	EditorManager->SnapPlacement(TargetPosition, PlacementRotation, SnappedPosition, SnappedRotation);

	EditorManager->PreviewActor->UpdatePosition(SnappedPosition, SnappedRotation);
	EditorManager->PreviewActor->Show();
	bPreviewPositioned = true;

	// Full validation (tech level, funds, materials, collision, connectivity, power)
	LastPlacementResult = EditorManager->CanPlaceModule(PendingPlacementModule, SnappedPosition, SnappedRotation);
	EditorManager->PreviewActor->SetValid(LastPlacementResult == EModulePlacementResult::Success);
}

void UStationEditorWidgetCpp::RotatePlacement(bool bClockwise)
{
	PlacementRotation.Yaw = FRotator::NormalizeAxis(PlacementRotation.Yaw + (bClockwise ? 90.0f : -90.0f));
	if (bIsInPlacementMode)
	{
		UpdatePreviewPosition();
	}
}

void UStationEditorWidgetCpp::UpdateStatusText()
{
	if (!PlacementStatusText)
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const bool bShowNotification = !LastNotificationText.IsEmpty() && (Now - LastNotificationTime) <= NotificationDisplayTime;

	FString Status;
	if (bShowNotification)
	{
		Status = LastNotificationText.ToString();
	}
	else if (bIsInPlacementMode)
	{
		Status = FString::Printf(TEXT("%s    [R] rotate  [Shift+Click] build more  [Esc/RMB] cancel"),
			*UStationEditorManager::GetPlacementResultText(LastPlacementResult).ToString());
	}
	else
	{
		Status = TEXT("Pick a module to build    [Del] remove hovered module  [Ctrl+Z/Y] undo/redo  [Esc] close");
	}

	PlacementStatusText->SetText(FText::FromString(Status));
}

void UStationEditorWidgetCpp::OnViewportClicked()
{
	if (!bIsInPlacementMode || !EditorManager || !PendingPlacementModule || !CurrentStation)
	{
		return;
	}

	// Check if preview actor exists and has been positioned
	if (!EditorManager->PreviewActor || !bPreviewPositioned)
	{
		UE_LOG(LogAdastreaStations, Verbose, TEXT("Station Editor: Cannot place module - preview not positioned"));
		return;
	}

	if (LastPlacementResult != EModulePlacementResult::Success)
	{
		// The status line already says why; nothing else to do.
		return;
	}

	// Get preview position
	const FVector PlacementPosition = EditorManager->PreviewActor->GetActorLocation();
	const FRotator PreviewRotation = EditorManager->PreviewActor->GetActorRotation();

	// PlaceModule re-validates the same snapped transform.
	ASpaceStationModule* PlacedModule = EditorManager->PlaceModule(
		PendingPlacementModule,
		PlacementPosition,
		PreviewRotation
	);

	if (PlacedModule)
	{
		UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Placed module %s at %s"),
			*PlacedModule->GetName(), *PlacementPosition.ToString());

		// Shift+Click keeps building the same module (X4-style repeat placement).
		if (!FSlateApplication::Get().GetModifierKeys().IsShiftDown())
		{
			ExitPlacementMode();
		}
	}
	else
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("Station Editor: Failed to place module at %s"),
			*PlacementPosition.ToString());
	}
}

void UStationEditorWidgetCpp::OnViewportRightClicked()
{
	// Cancel placement on right-click
	if (bIsInPlacementMode)
	{
		ExitPlacementMode();
	}
}

bool UStationEditorWidgetCpp::GetCursorWorldPosition(FVector& OutWorldPosition, FVector& OutWorldDirection)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return false;
	}

	float MouseX, MouseY;
	if (PC->GetMousePosition(MouseX, MouseY))
	{
		return PC->DeprojectScreenPositionToWorld(MouseX, MouseY, OutWorldPosition, OutWorldDirection);
	}

	return false;
}

FReply UStationEditorWidgetCpp::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// Only handle placement clicks if in placement mode and not clicking on UI widgets
	// Check if the reply was already handled by a child widget (button, list, etc.)
	if (bIsInPlacementMode && !Reply.IsEventHandled())
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			OnViewportClicked();
			return FReply::Handled().SetUserFocus(TakeWidget(), EFocusCause::Mouse);
		}
		else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			OnViewportRightClicked();
			return FReply::Handled().SetUserFocus(TakeWidget(), EFocusCause::Mouse);
		}
	}

	return Reply;
}

FReply UStationEditorWidgetCpp::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!EditorManager || InKeyEvent.IsRepeat())
	{
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}

	const FKey Key = InKeyEvent.GetKey();
	const bool bCtrl = InKeyEvent.IsControlDown();
	const bool bShift = InKeyEvent.IsShiftDown();

	if (Key == EKeys::R && !bCtrl)
	{
		RotatePlacement(!bShift);
		return FReply::Handled();
	}

	if (Key == EKeys::Escape)
	{
		if (bIsInPlacementMode)
		{
			ExitPlacementMode();
		}
		else
		{
			OnCloseButtonClicked();
		}
		return FReply::Handled();
	}

	if (bCtrl && (Key == EKeys::Y || (Key == EKeys::Z && bShift)))
	{
		EditorManager->Redo();
		return FReply::Handled();
	}

	if (bCtrl && Key == EKeys::Z)
	{
		EditorManager->Undo();
		return FReply::Handled();
	}

	if (Key == EKeys::Delete && !bIsInPlacementMode)
	{
		if (ASpaceStationModule* Module = GetModuleUnderCursor())
		{
			EditorManager->RemoveModule(Module);
		}
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UStationEditorWidgetCpp::EnsureEditorManager()
{
	if (!EditorManager)
	{
		EditorManager = NewObject<UStationEditorManager>(this);
	}
}
