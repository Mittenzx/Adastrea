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
#include "Public/AdastreaLog.h"

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
	, ModuleCatalog(nullptr)
	, CurrentStation(nullptr)
	, EditorManager(nullptr)
	, bIsInPlacementMode(false)
	, PendingPlacementModule(nullptr)
{
}

void UStationEditorWidgetCpp::NativeConstruct()
{
	Super::NativeConstruct();

	// Ensure EditorManager exists
	EnsureEditorManager();

	// Bind close button
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UStationEditorWidgetCpp::OnCloseButtonClicked);
	}

	// Bind to EditorManager events
	if (EditorManager)
	{
		EditorManager->OnModulePlaced.AddDynamic(this, &UStationEditorWidgetCpp::OnManagerModulePlaced);
		EditorManager->OnModuleRemoved.AddDynamic(this, &UStationEditorWidgetCpp::OnManagerModuleRemoved);
		EditorManager->OnStatisticsUpdated.AddDynamic(this, &UStationEditorWidgetCpp::OnManagerStatisticsUpdated);
		EditorManager->OnConstructionQueueChanged.AddDynamic(this, &UStationEditorWidgetCpp::OnManagerQueueChanged);
	}

	// Initial refresh
	RefreshModuleList();
	RefreshStatistics();
	UpdateConstructionQueue();
}

void UStationEditorWidgetCpp::NativeDestruct()
{
	// Unbind events
	if (EditorManager)
	{
		EditorManager->OnModulePlaced.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerModulePlaced);
		EditorManager->OnModuleRemoved.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerModuleRemoved);
		EditorManager->OnStatisticsUpdated.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerStatisticsUpdated);
		EditorManager->OnConstructionQueueChanged.RemoveDynamic(this, &UStationEditorWidgetCpp::OnManagerQueueChanged);
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
		EditorManager->ModuleCatalog = Catalog;
		EditorManager->PlayerTechLevel = DefaultPlayerTechLevel;
		EditorManager->PlayerCredits = DefaultPlayerCredits;

		// Begin editing the station
		if (Station)
		{
			EditorManager->BeginEditing(Station);
		}
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
}

void UStationEditorWidgetCpp::OnCloseButtonClicked()
{
	// Exit placement mode if active
	if (bIsInPlacementMode)
	{
		ExitPlacementMode();
	}

	// Save and end editing
	if (EditorManager)
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
}

void UStationEditorWidgetCpp::OnQueueItemCancelled(int32 QueueId)
{
	if (EditorManager)
	{
		EditorManager->CancelConstruction(QueueId);
	}
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

	// Show preview with this module
	EditorManager->ShowPreview(ModuleClass);

	UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Entered placement mode for %s"), 
		*ModuleClass->GetName());
}

void UStationEditorWidgetCpp::ExitPlacementMode()
{
	if (!EditorManager)
	{
		return;
	}

	bIsInPlacementMode = false;
	PendingPlacementModule = nullptr;

	// Hide preview
	EditorManager->HidePreview();

	UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Exited placement mode"));
}

void UStationEditorWidgetCpp::UpdatePreviewPosition()
{
	if (!EditorManager || !CurrentStation || !PendingPlacementModule)
	{
		return;
	}

	// Get cursor world position
	FVector WorldPosition, WorldDirection;
	if (!GetCursorWorldPosition(WorldPosition, WorldDirection))
	{
		return;
	}

	// Line trace to find placement location
	FHitResult HitResult;
	FVector TraceStart = WorldPosition;
	FVector TraceEnd = WorldPosition + WorldDirection * MaxTraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwningPlayerPawn());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		// Update preview position
		EditorManager->UpdatePreview(HitResult.Location, FRotator::ZeroRotator);

		// Check if placement is valid at this location
		EModulePlacementResult ValidationResult = EditorManager->CanPlaceModule(
			PendingPlacementModule,
			HitResult.Location,
			FRotator::ZeroRotator
		);

		// Update preview validity color
		bool bIsValid = (ValidationResult == EModulePlacementResult::Success);
		if (EditorManager->PreviewActor)
		{
			EditorManager->PreviewActor->SetValid(bIsValid);
		}
	}
}

void UStationEditorWidgetCpp::OnViewportClicked()
{
	if (!bIsInPlacementMode || !EditorManager || !PendingPlacementModule || !CurrentStation)
	{
		return;
	}

	// Get preview position
	FVector PlacementPosition = FVector::ZeroVector;
	FRotator PlacementRotation = FRotator::ZeroRotator;

	if (EditorManager->PreviewActor)
	{
		PlacementPosition = EditorManager->PreviewActor->GetActorLocation();
		PlacementRotation = EditorManager->PreviewActor->GetActorRotation();
	}

	// Validate one more time before placement
	EModulePlacementResult ValidationResult = EditorManager->CanPlaceModule(
		PendingPlacementModule,
		PlacementPosition,
		PlacementRotation
	);

	if (ValidationResult != EModulePlacementResult::Success)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("Station Editor: Cannot place module: %d"), 
			static_cast<int32>(ValidationResult));
		return;
	}

	// Place the module
	ASpaceStationModule* PlacedModule = EditorManager->PlaceModule(
		PendingPlacementModule,
		PlacementPosition,
		PlacementRotation
	);

	if (PlacedModule)
	{
		UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Placed module %s at %s"),
			*PlacedModule->GetName(), *PlacementPosition.ToString());

		// Exit placement mode
		ExitPlacementMode();
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

	if (bIsInPlacementMode)
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			OnViewportClicked();
			return FReply::Handled();
		}
		else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			OnViewportRightClicked();
			return FReply::Handled();
		}
	}

	return Reply;
}

void UStationEditorWidgetCpp::EnsureEditorManager()
{
	if (!EditorManager)
	{
		EditorManager = NewObject<UStationEditorManager>(this);
	}
}
