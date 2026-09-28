// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "UI/StationEditorWidgetCpp.h"
#include "UI/StationEditorButton.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "AdastreaLog.h"
#include "StationBuildPreview.h"
#include "InputCoreTypes.h"
#include "Audio/AudioEventLibrary.h"

// StationEditor module includes
#include "StationEditorManager.h"
#include "StationModuleCatalog.h"

namespace StationEditorStyle
{
	static const FLinearColor Accent(0.25f, 0.75f, 1.0f);
	static const FLinearColor PanelFill(0.02f, 0.04f, 0.07f, 0.72f);

	static FSlateBrush RoundedBrush(const FLinearColor& Fill, const FLinearColor& Outline, float OutlineWidth = 1.0f)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(Fill);
		Brush.OutlineSettings.CornerRadii = FVector4(4.0f, 4.0f, 4.0f, 4.0f);
		Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Brush.OutlineSettings.Color = FSlateColor(Outline);
		Brush.OutlineSettings.Width = OutlineWidth;
		return Brush;
	}

	/** Dark translucent tile with an outline in the given color; brighter when hovered/pressed */
	static FButtonStyle TileStyle(const FLinearColor& Color)
	{
		FButtonStyle Style;
		Style.SetNormal(RoundedBrush(FLinearColor(0.03f, 0.06f, 0.1f, 0.85f), Color * FLinearColor(1, 1, 1, 0.55f)));
		Style.SetHovered(RoundedBrush(FLinearColor(0.06f, 0.12f, 0.18f, 0.92f), Color, 1.5f));
		Style.SetPressed(RoundedBrush(Color * FLinearColor(0.35f, 0.35f, 0.35f, 0.95f), Color, 2.0f));
		Style.SetDisabled(RoundedBrush(FLinearColor(0.03f, 0.03f, 0.03f, 0.6f), FLinearColor(0.3f, 0.3f, 0.3f, 0.5f)));
		Style.SetNormalPadding(FMargin(8.0f, 5.0f));
		Style.SetPressedPadding(FMargin(8.0f, 6.0f, 8.0f, 4.0f));
		return Style;
	}

	/** One color per module group, so the palette reads by function at a glance */
	static FLinearColor GroupColor(EStationModuleGroup Group)
	{
		switch (Group)
		{
		case EStationModuleGroup::Docking:    return FLinearColor(0.3f, 0.8f, 1.0f);
		case EStationModuleGroup::Power:      return FLinearColor(1.0f, 0.8f, 0.25f);
		case EStationModuleGroup::Storage:    return FLinearColor(0.75f, 0.6f, 0.4f);
		case EStationModuleGroup::Processing: return FLinearColor(1.0f, 0.5f, 0.25f);
		case EStationModuleGroup::Defence:    return FLinearColor(1.0f, 0.3f, 0.3f);
		case EStationModuleGroup::Habitation: return FLinearColor(0.45f, 0.95f, 0.5f);
		case EStationModuleGroup::Public:     return FLinearColor(0.85f, 0.5f, 1.0f);
		case EStationModuleGroup::Connection: return FLinearColor(0.7f, 0.75f, 0.8f);
		default:                              return FLinearColor(0.6f, 0.65f, 0.7f);
		}
	}

	static UTextBlock* MakeText(UObject* Outer, const FText& Text, int32 Size, const TCHAR* Typeface, const FLinearColor& Color)
	{
		UTextBlock* Block = NewObject<UTextBlock>(Outer);
		Block->SetText(Text);
		Block->SetFont(FCoreStyle::GetDefaultFontStyle(Typeface, Size));
		Block->SetColorAndOpacity(FSlateColor(Color));
		Block->SetShadowOffset(FVector2D(1.0f, 1.0f));
		Block->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));
		return Block;
	}

	static UBorder* MakePanel(UObject* Outer, UWidget* Content)
	{
		UBorder* Panel = NewObject<UBorder>(Outer);
		Panel->SetBrush(RoundedBrush(PanelFill, Accent * FLinearColor(1, 1, 1, 0.35f)));
		Panel->SetPadding(FMargin(12.0f, 10.0f));
		Panel->SetContent(Content);
		return Panel;
	}
}

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
	, StationNameText(nullptr)
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

	// Leave the ship's view for the 3D plan camera around the station.
	BeginPlanCamera();

	SetKeyboardFocus();

	UAudioEventLibrary::PlaySecondary2D(this, TEXT("UI.Open"), 0.1f);
}

void UStationEditorWidgetCpp::NativeDestruct()
{
	// However the widget went away (Close button, the G toggle, a level change),
	// end the editing session: commit what was built and drop the preview actor.
	// Before this, closing with G left the session open and the preview alive.
	EndPlanCamera();

	if (bIsInPlacementMode)
	{
		ExitPlacementMode();
	}
	if (EditorManager && EditorManager->bIsEditing)
	{
		EditorManager->Save();
	}

	// Secondary: when Save() just played Editor.Save, that is the closing sound.
	UAudioEventLibrary::PlaySecondary2D(this, TEXT("UI.Close"), 0.1f);

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

	TickPlanCamera(InDeltaTime);
	UpdateQueueProgress();

	// Update preview position if in placement mode
	if (bIsInPlacementMode)
	{
		UpdatePreviewPosition();
	}

	UpdateStatusText();

	// The wallet is the trader's, which trading/saves can change while the
	// editor is open, so keep the readout live rather than only on stat updates.
	UpdateCreditsText();
}

void UStationEditorWidgetCpp::UpdateCreditsText()
{
	if (!CreditsText || !EditorManager)
	{
		return;
	}

	const int32 Credits = EditorManager->GetPlayerCredits();
	if (Credits != LastShownCredits)
	{
		LastShownCredits = Credits;
		CreditsText->SetText(FText::Format(NSLOCTEXT("StationEditor", "CreditsReadout", "Credits: {0}"), FText::AsNumber(Credits)));
	}
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
		// Credits aren't configured here: the manager pays from the player's
		// UPlayerTraderComponent (the HUD wallet), resolved from the piloted ship.

		// Begin editing the station
		if (Station)
		{
			EditorManager->BeginEditing(Station);
		}
	}

	PlacementRotation = FRotator::ZeroRotator;
	LastNotificationText = FText::GetEmpty();

	if (StationNameText && Station)
	{
		StationNameText->SetText(ITargetable::Execute_GetTargetDisplayName(Station));
	}

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

	PaletteButtons.Reset();

	// Get available modules, grouped by function so related modules sit together
	TArray<FStationModuleEntry> AvailableModules = EditorManager->GetAvailableModules();
	AvailableModules.StableSort([](const FStationModuleEntry& A, const FStationModuleEntry& B)
	{
		return A.ModuleGroup < B.ModuleGroup;
	});

	// One palette tile per module
	for (const FStationModuleEntry& Entry : AvailableModules)
	{
		if (!Entry.ModuleClass)
		{
			continue;
		}

		UStationEditorButton* Tile = NewObject<UStationEditorButton>(this);
		Tile->ModuleClass = Entry.ModuleClass;
		Tile->OnButtonClicked.BindUObject(this, &UStationEditorWidgetCpp::OnPaletteTileClicked);
		Tile->SetStyle(StationEditorStyle::TileStyle(StationEditorStyle::GroupColor(Entry.ModuleGroup)));
		Tile->SetToolTipText(FText::Format(NSLOCTEXT("StationEditor", "TileTooltip", "{0}\n{1}\nFootprint {2}x{3}"),
			Entry.DisplayName, Entry.Description, FText::AsNumber(Entry.GridFootprint.X), FText::AsNumber(Entry.GridFootprint.Y)));

		UVerticalBox* TileContent = NewObject<UVerticalBox>(this);
		UTextBlock* Name = StationEditorStyle::MakeText(this, Entry.DisplayName, 10, TEXT("Bold"), FLinearColor(0.92f, 0.96f, 1.0f));
		Name->SetAutoWrapText(true);
		TileContent->AddChildToVerticalBox(Name);
		TileContent->AddChildToVerticalBox(StationEditorStyle::MakeText(this,
			FText::Format(NSLOCTEXT("StationEditor", "TileCost", "{0} cr  \u00b7  {1}s"),
				FText::AsNumber(Entry.BuildCost.Credits), FText::AsNumber(FMath::RoundToInt(Entry.BuildCost.BuildTime))),
			8, TEXT("Regular"), FLinearColor(0.6f, 0.7f, 0.8f)));

		USizeBox* TileSize = NewObject<USizeBox>(this);
		TileSize->SetWidthOverride(150.0f);
		TileSize->SetHeightOverride(58.0f);
		TileSize->SetContent(TileContent);
		Tile->SetContent(TileSize);

		if (UScrollBoxSlot* TileSlot = Cast<UScrollBoxSlot>(ModuleListScrollBox->AddChild(Tile)))
		{
			TileSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
		}
		PaletteButtons.Add(Tile);
	}

	UpdatePaletteHighlight();
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

	UpdateCreditsText();

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

	QueueRowTexts.Reset();

	// One row per queued module: progress text plus a cancel button
	for (const FConstructionQueueItem& Item : Queue)
	{
		UHorizontalBox* Row = NewObject<UHorizontalBox>(this);

		UTextBlock* RowText = StationEditorStyle::MakeText(this, FText::GetEmpty(), 9, TEXT("Regular"), FLinearColor(0.85f, 0.9f, 1.0f));
		if (UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(RowText))
		{
			TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			TextSlot->SetVerticalAlignment(VAlign_Center);
		}
		QueueRowTexts.Add(RowText);

		UStationEditorButton* Cancel = NewObject<UStationEditorButton>(this);
		Cancel->QueueId = Item.QueueId;
		Cancel->OnButtonClicked.BindUObject(this, &UStationEditorWidgetCpp::OnQueueRowClicked);
		Cancel->SetStyle(StationEditorStyle::TileStyle(FLinearColor(0.8f, 0.3f, 0.25f)));
		Cancel->SetToolTipText(NSLOCTEXT("StationEditor", "CancelBuild", "Cancel and refund"));
		Cancel->SetContent(StationEditorStyle::MakeText(this, FText::FromString(TEXT("X")), 8, TEXT("Bold"), FLinearColor::White));
		Row->AddChildToHorizontalBox(Cancel);

		if (UScrollBoxSlot* RowSlot = Cast<UScrollBoxSlot>(QueueScrollBox->AddChild(Row)))
		{
			RowSlot->SetPadding(FMargin(0.0f, 2.0f));
		}
	}

	if (QueuePanel)
	{
		QueuePanel->SetVisibility(Queue.Num() > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	UpdateQueueProgress();
}

void UStationEditorWidgetCpp::OnModuleButtonClicked(TSubclassOf<ASpaceStationModule> ModuleClass)
{
	if (!ModuleClass)
	{
		return;
	}

	UAudioEventLibrary::PlayEvent2D(this, TEXT("UI.Click"), 0.08f);

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
	UpdatePaletteHighlight();

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
	UpdatePaletteHighlight();

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
	if (!GetWorld()->LineTraceSingleByChannel(Hit, WorldPosition, WorldPosition + WorldDirection * GetTraceDistance(), ECC_Visibility, QueryParams))
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
		return EditorManager->FindAttachPosition(PendingPlacementModule, HitModule, Hit.ImpactNormal, PlacementRotation, OutPosition);
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
	if (Distance <= 0.0f || Distance > GetTraceDistance())
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
	UAudioEventLibrary::PlayEvent2D(this, TEXT("Editor.Rotate"), 0.05f);
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
		Status = FString::Printf(TEXT("%s    [R] rotate  [Shift+Click] build more  [Esc/RMB click] cancel  [RMB drag] orbit"),
			*UStationEditorManager::GetPlacementResultText(LastPlacementResult).ToString());
	}
	else
	{
		Status = TEXT("Pick a module to build    [RMB drag/Q/E] orbit  [MMB drag/WASD] pan  [Wheel] zoom  [F] frame  [Del] remove hovered  [Ctrl+Z/Y] undo/redo  [Esc] close");
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
		UAudioEventLibrary::PlayEvent2D(this, TEXT("Editor.Invalid"), 0.25f);
		return;
	}

	if (LastPlacementResult != EModulePlacementResult::Success)
	{
		// The status line already says why; the buzz says "no".
		UAudioEventLibrary::PlayEvent2D(this, TEXT("Editor.Invalid"), 0.25f);
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

	// Camera drags: right button orbits (a click without a drag still cancels
	// placement, on release), middle button pans.
	if (PlanCamera && !Reply.IsEventHandled())
	{
		const FKey Button = InMouseEvent.GetEffectingButton();
		if (Button == EKeys::RightMouseButton || Button == EKeys::MiddleMouseButton)
		{
			if (Button == EKeys::RightMouseButton)
			{
				bOrbitDragging = true;
				RightDragPixels = 0.0f;
			}
			else
			{
				bPanDragging = true;
			}
			return FReply::Handled().CaptureMouse(TakeWidget()).SetUserFocus(TakeWidget(), EFocusCause::Mouse);
		}
	}

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
	// Camera keys, repeats included: anything left unhandled falls through to
	// the game viewport and would fly the ship.
	if (PlanCamera && !InKeyEvent.IsControlDown())
	{
		const FKey CameraKey = InKeyEvent.GetKey();
		if (CameraKey == EKeys::W || CameraKey == EKeys::A || CameraKey == EKeys::S || CameraKey == EKeys::D
			|| CameraKey == EKeys::Q || CameraKey == EKeys::E)
		{
			HeldCameraKeys.Add(CameraKey);
			return FReply::Handled();
		}
		if (CameraKey == EKeys::F)
		{
			if (!InKeyEvent.IsRepeat())
			{
				FrameStation();
				ApplyPlanCamera();
			}
			return FReply::Handled();
		}
	}

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

FReply UStationEditorWidgetCpp::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (HeldCameraKeys.Remove(InKeyEvent.GetKey()) > 0)
	{
		return FReply::Handled();
	}
	return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
}

void UStationEditorWidgetCpp::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
	HeldCameraKeys.Reset();
}

FReply UStationEditorWidgetCpp::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FKey Button = InMouseEvent.GetEffectingButton();
	const bool bWasOrbit = (Button == EKeys::RightMouseButton && bOrbitDragging);
	const bool bWasPan = (Button == EKeys::MiddleMouseButton && bPanDragging);
	if (!bWasOrbit && !bWasPan)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	if (bWasOrbit)
	{
		bOrbitDragging = false;
		// Barely moved: it was a click, which cancels placement as before.
		if (RightDragPixels < 5.0f && bIsInPlacementMode)
		{
			OnViewportRightClicked();
		}
	}
	else
	{
		bPanDragging = false;
	}

	FReply Reply = FReply::Handled();
	if (!bOrbitDragging && !bPanDragging)
	{
		Reply.ReleaseMouseCapture();
	}
	return Reply;
}

FReply UStationEditorWidgetCpp::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!PlanCamera || (!bOrbitDragging && !bPanDragging))
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	const FVector2D Delta = InMouseEvent.GetCursorDelta();

	if (bOrbitDragging)
	{
		RightDragPixels += Delta.Size();
		CameraYaw += Delta.X * OrbitSensitivity;
		CameraPitch -= Delta.Y * OrbitSensitivity;
	}
	else
	{
		// Grab-and-drag: the point under the cursor follows it across the screen.
		int32 ViewportX = 0, ViewportY = 0;
		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->GetViewportSize(ViewportX, ViewportY);
		}
		const float HalfFovRad = FMath::DegreesToRadians(PlanCamera->GetCameraComponent()->FieldOfView * 0.5f);
		const float UnitsPerPixel = 2.0f * CameraDistance * FMath::Tan(HalfFovRad) / FMath::Max(ViewportX, 1);

		const FRotationMatrix CameraAxes(FRotator(CameraPitch, CameraYaw, 0.0f));
		CameraFocus += (-CameraAxes.GetUnitAxis(EAxis::Y) * Delta.X + CameraAxes.GetUnitAxis(EAxis::Z) * Delta.Y) * UnitsPerPixel;
	}

	ApplyPlanCamera();
	return FReply::Handled();
}

FReply UStationEditorWidgetCpp::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!PlanCamera)
	{
		return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
	}

	CameraDistance *= FMath::Pow(1.0f - ZoomStep, InMouseEvent.GetWheelDelta());
	ApplyPlanCamera();
	return FReply::Handled();
}

float UStationEditorWidgetCpp::GetTraceDistance() const
{
	// From a zoomed-out plan camera the station can be further away than MaxTraceDistance.
	return PlanCamera ? FMath::Max(MaxTraceDistance, CameraDistance + 2.0f * MaxPlacementDistance) : MaxTraceDistance;
}

void UStationEditorWidgetCpp::BeginPlanCamera()
{
	APlayerController* PC = GetOwningPlayer();
	UWorld* World = GetWorld();
	if (!PC || !World || !CurrentStation)
	{
		return;
	}

	if (!PlanCamera)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;
		PlanCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform::Identity, SpawnParams);
		if (!PlanCamera)
		{
			UE_LOG(LogAdastreaStations, Warning, TEXT("Station Editor: failed to spawn the plan camera - staying on the ship view"));
			return;
		}
		UCameraComponent* Camera = PlanCamera->GetCameraComponent();
		Camera->bConstrainAspectRatio = false;
		Camera->SetFieldOfView(60.0f);
	}

	bOrbitDragging = false;
	bPanDragging = false;
	HeldCameraKeys.Reset();

	// Start looking at the station from roughly where the ship is.
	if (APawn* Pawn = PC->GetPawn())
	{
		const FVector ToStation = CurrentStation->GetActorLocation() - Pawn->GetActorLocation();
		if (!ToStation.IsNearlyZero())
		{
			CameraYaw = ToStation.Rotation().Yaw;
		}
	}
	CameraPitch = -35.0f;
	FrameStation();
	ApplyPlanCamera();

	PC->SetViewTargetWithBlend(PlanCamera, CameraBlendTime, VTBlend_Cubic);
}

void UStationEditorWidgetCpp::EndPlanCamera()
{
	bOrbitDragging = false;
	bPanDragging = false;
	HeldCameraKeys.Reset();

	if (!PlanCamera)
	{
		return;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		AActor* ViewBack = PC->GetPawn() ? static_cast<AActor*>(PC->GetPawn()) : static_cast<AActor*>(PC);
		PC->SetViewTargetWithBlend(ViewBack, CameraBlendTime, VTBlend_Cubic);
	}

	// Outlive the blend back, which reads the camera's view until it finishes.
	PlanCamera->SetLifeSpan(CameraBlendTime + 0.1f);
	PlanCamera = nullptr;
}

void UStationEditorWidgetCpp::FrameStation()
{
	if (!CurrentStation)
	{
		return;
	}

	// Empty stations still get some room: that's the area being built in.
	float Radius = 2000.0f;
	const FBox Bounds = CurrentStation->GetStationBounds();
	if (Bounds.IsValid)
	{
		CameraFocus = Bounds.GetCenter();
		Radius = FMath::Max(Radius, static_cast<float>(Bounds.GetExtent().Size()));
	}
	else
	{
		CameraFocus = CurrentStation->GetActorLocation();
	}

	const float FieldOfView = PlanCamera ? PlanCamera->GetCameraComponent()->FieldOfView : 60.0f;
	CameraDistance = 1.3f * Radius / FMath::Sin(FMath::DegreesToRadians(FieldOfView * 0.5f));
}

void UStationEditorWidgetCpp::ApplyPlanCamera()
{
	if (!PlanCamera)
	{
		return;
	}

	CameraPitch = FMath::Clamp(CameraPitch, -85.0f, 85.0f);
	CameraYaw = FRotator::NormalizeAxis(CameraYaw);
	CameraDistance = FMath::Clamp(CameraDistance, MinCameraDistance, FMath::Max(MinCameraDistance, MaxCameraDistance));

	const FRotator ViewRotation(CameraPitch, CameraYaw, 0.0f);
	PlanCamera->SetActorLocationAndRotation(CameraFocus - ViewRotation.Vector() * CameraDistance, ViewRotation);
}

void UStationEditorWidgetCpp::TickPlanCamera(float DeltaTime)
{
	if (!PlanCamera || HeldCameraKeys.Num() == 0)
	{
		return;
	}

	auto Axis = [this](const FKey& Positive, const FKey& Negative)
	{
		return (HeldCameraKeys.Contains(Positive) ? 1.0f : 0.0f) - (HeldCameraKeys.Contains(Negative) ? 1.0f : 0.0f);
	};

	CameraYaw += Axis(EKeys::E, EKeys::Q) * KeyOrbitSpeed * DeltaTime;

	// Pan on the horizontal plane, relative to where the camera faces.
	const FRotationMatrix YawAxes(FRotator(0.0f, CameraYaw, 0.0f));
	const FVector Pan = YawAxes.GetUnitAxis(EAxis::X) * Axis(EKeys::W, EKeys::S) + YawAxes.GetUnitAxis(EAxis::Y) * Axis(EKeys::D, EKeys::A);
	CameraFocus += Pan * KeyPanSpeed * CameraDistance * DeltaTime;

	ApplyPlanCamera();
}

void UStationEditorWidgetCpp::OnPaletteTileClicked(UStationEditorButton* Tile)
{
	if (Tile)
	{
		OnModuleButtonClicked(Tile->ModuleClass);
	}
}

void UStationEditorWidgetCpp::OnQueueRowClicked(UStationEditorButton* Row)
{
	if (Row)
	{
		UAudioEventLibrary::PlayEvent2D(this, TEXT("UI.Click"), 0.08f);
		OnQueueItemCancelled(Row->QueueId);
	}
}

bool UStationEditorWidgetCpp::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (!IsDesignTime() && WidgetTree)
	{
		BuildPlanLayout();
	}
	return true;
}

void UStationEditorWidgetCpp::BuildPlanLayout()
{
	using namespace StationEditorStyle;

	// Full-screen, invisible, but hit-testable: clicks on empty space reach
	// NativeOnMouseButtonDown (placement) and the plan camera's drags.
	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PlanRoot"));
	Root->SetBrushColor(FLinearColor::Transparent);
	Root->SetPadding(FMargin(20.0f));
	Root->SetVisibility(ESlateVisibility::Visible);

	UOverlay* Layer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PlanLayer"));
	Root->SetContent(Layer);

	// ---- Top-left: station readout ----
	UVerticalBox* Stats = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatsBox"));
	Stats->AddChildToVerticalBox(MakeText(this, NSLOCTEXT("StationEditor", "Title", "STATION BUILDER"), 13, TEXT("Bold"), Accent));
	StationNameText = MakeText(this, FText::GetEmpty(), 10, TEXT("Regular"), FLinearColor(0.8f, 0.85f, 0.9f));
	Stats->AddChildToVerticalBox(StationNameText)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	PowerDisplayText = MakeText(this, FText::GetEmpty(), 10, TEXT("Regular"), FLinearColor::White);
	Stats->AddChildToVerticalBox(PowerDisplayText);
	PowerBalanceBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("PowerBalanceBar"));
	USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PowerBarSize"));
	BarSize->SetWidthOverride(220.0f);
	BarSize->SetHeightOverride(5.0f);
	BarSize->SetContent(PowerBalanceBar);
	Stats->AddChildToVerticalBox(BarSize)->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 6.0f));
	ModuleCountDisplay = MakeText(this, FText::GetEmpty(), 10, TEXT("Regular"), FLinearColor::White);
	Stats->AddChildToVerticalBox(ModuleCountDisplay);
	CreditsText = MakeText(this, FText::GetEmpty(), 10, TEXT("Regular"), FLinearColor(1.0f, 0.85f, 0.4f));
	Stats->AddChildToVerticalBox(CreditsText);
	LastShownCredits = INDEX_NONE;

	UBorder* StatsPanel = MakePanel(this, Stats);
	StatsPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UOverlaySlot* StatsSlot = Layer->AddChildToOverlay(StatsPanel))
	{
		StatsSlot->SetHorizontalAlignment(HAlign_Left);
		StatsSlot->SetVerticalAlignment(VAlign_Top);
	}

	// ---- Top-right: close ----
	UStationEditorButton* Close = WidgetTree->ConstructWidget<UStationEditorButton>(UStationEditorButton::StaticClass(), TEXT("CloseButton"));
	Close->SetStyle(TileStyle(Accent));
	Close->SetContent(MakeText(this, NSLOCTEXT("StationEditor", "Close", "CLOSE  [Esc]"), 10, TEXT("Bold"), FLinearColor::White));
	CloseButton = Close;
	if (UOverlaySlot* CloseSlot = Layer->AddChildToOverlay(Close))
	{
		CloseSlot->SetHorizontalAlignment(HAlign_Right);
		CloseSlot->SetVerticalAlignment(VAlign_Top);
	}

	// ---- Bottom-right, above the palette: construction queue (hidden when empty) ----
	UVerticalBox* QueueBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("QueueBox"));
	QueueBox->AddChildToVerticalBox(MakeText(this, NSLOCTEXT("StationEditor", "Queue", "CONSTRUCTION"), 10, TEXT("Bold"), Accent))
		->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	QueueScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("QueueScrollBox"));
	USizeBox* QueueSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("QueueSize"));
	QueueSize->SetWidthOverride(260.0f);
	QueueSize->SetMaxDesiredHeight(180.0f);
	QueueSize->SetContent(QueueScrollBox);
	QueueBox->AddChildToVerticalBox(QueueSize);
	QueuePanel = MakePanel(this, QueueBox);
	QueuePanel->SetVisibility(ESlateVisibility::Collapsed);
	if (UOverlaySlot* QueueSlot = Layer->AddChildToOverlay(QueuePanel))
	{
		QueueSlot->SetHorizontalAlignment(HAlign_Right);
		QueueSlot->SetVerticalAlignment(VAlign_Bottom);
		QueueSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 110.0f));
	}

	// ---- Bottom: status line above the module palette strip ----
	UVerticalBox* Bottom = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BottomBox"));
	PlacementStatusText = MakeText(this, FText::GetEmpty(), 10, TEXT("Regular"), FLinearColor(0.85f, 0.92f, 1.0f));
	PlacementStatusText->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* StatusSlot = Bottom->AddChildToVerticalBox(PlacementStatusText))
	{
		StatusSlot->SetHorizontalAlignment(HAlign_Center);
		StatusSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	ModuleListScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ModuleListScrollBox"));
	ModuleListScrollBox->SetOrientation(Orient_Horizontal);
	ModuleListScrollBox->SetScrollBarVisibility(ESlateVisibility::Collapsed);
	UBorder* PalettePanel = MakePanel(this, ModuleListScrollBox);
	PalettePanel->SetPadding(FMargin(8.0f));
	if (UVerticalBoxSlot* PaletteSlot = Bottom->AddChildToVerticalBox(PalettePanel))
	{
		PaletteSlot->SetHorizontalAlignment(HAlign_Center);
	}

	Bottom->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UOverlaySlot* BottomSlot = Layer->AddChildToOverlay(Bottom))
	{
		BottomSlot->SetHorizontalAlignment(HAlign_Fill);
		BottomSlot->SetVerticalAlignment(VAlign_Bottom);
	}

	WidgetTree->RootWidget = Root;
}

void UStationEditorWidgetCpp::UpdatePaletteHighlight()
{
	for (UStationEditorButton* Tile : PaletteButtons)
	{
		if (Tile)
		{
			const bool bSelected = bIsInPlacementMode && Tile->ModuleClass == PendingPlacementModule;
			Tile->SetBackgroundColor(bSelected ? FLinearColor(2.2f, 2.2f, 2.2f) : FLinearColor::White);
		}
	}
}

void UStationEditorWidgetCpp::UpdateQueueProgress()
{
	if (!EditorManager || QueueRowTexts.Num() == 0)
	{
		return;
	}

	const TArray<FConstructionQueueItem> Queue = EditorManager->GetConstructionQueue();
	for (int32 Index = 0; Index < QueueRowTexts.Num() && Index < Queue.Num(); ++Index)
	{
		const FConstructionQueueItem& Item = Queue[Index];
		const ASpaceStationModule* CDO = Item.ModuleClass ? Item.ModuleClass->GetDefaultObject<ASpaceStationModule>() : nullptr;
		const FString Name = (CDO && !CDO->ModuleType.IsEmpty()) ? CDO->ModuleType : FString(TEXT("Module"));
		const FString Line = Item.bIsBuilding
			? FString::Printf(TEXT("%s  %3.0f%%  %.0fs"), *Name, Item.GetProgress() * 100.0f, Item.TimeRemaining)
			: FString::Printf(TEXT("%s  queued"), *Name);
		if (QueueRowTexts[Index])
		{
			QueueRowTexts[Index]->SetText(FText::FromString(Line));
		}
	}
}

void UStationEditorWidgetCpp::EnsureEditorManager()
{
	if (!EditorManager)
	{
		EditorManager = NewObject<UStationEditorManager>(this);
	}
}
