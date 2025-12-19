// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/SectorMapWidget.h"
#include "SpaceSectorMap.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

USectorMapWidget::USectorMapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentSector(nullptr)
	, bIsSectorMapVisible(false)
	, Text_SectorName(nullptr)
	, Text_SectorDescription(nullptr)
	, Text_ObjectCount(nullptr)
	, ObjectListScrollBox(nullptr)
	, bAutoCreateMissingWidgets(true)
{
	// Initialize default sector info
	CurrentSectorInfo = FSectorDisplayInfo();
}

void USectorMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Create default UI widgets if they don't exist and auto-create is enabled
	if (bAutoCreateMissingWidgets)
	{
		CreateDefaultUIWidgets();
	}
	
	// Initialize the sector map when constructed
	InitializeSectorMap();
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Widget constructed"));
}

void USectorMapWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	// Clear references
	CurrentSector = nullptr;
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Widget destructed"));
}

void USectorMapWidget::InitializeSectorMap_Implementation()
{
	// Set widget as visible
	bIsSectorMapVisible = true;
	
	// Clear any existing sector info
	ClearSectorInfo();
	
	// Try to find a sector in the current level
	TArray<AActor*> SectorActors;
	UGameplayStatics::GetAllActorsOfClass(this, ASpaceSectorMap::StaticClass(), SectorActors);
	
	if (SectorActors.Num() > 0)
	{
		// Use the first sector found
		ASpaceSectorMap* FirstSector = Cast<ASpaceSectorMap>(SectorActors[0]);
		if (FirstSector)
		{
			SetTargetSector(FirstSector);
			UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Auto-selected sector '%s'"), *FirstSector->SectorName.ToString());
		}
	}
	else
	{
		UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: No sectors found in level"));
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Initialized"));
}

void USectorMapWidget::UpdateSectorInfo_Implementation(const FSectorDisplayInfo& SectorInfo)
{
	CurrentSectorInfo = SectorInfo;
	
	// Update UI widgets if they exist
	if (Text_SectorName)
	{
		Text_SectorName->SetText(SectorInfo.SectorName);
	}

	if (Text_SectorDescription)
	{
		Text_SectorDescription->SetText(SectorInfo.Description);
	}

	if (Text_ObjectCount)
	{
		FText CountText = FText::Format(
			FText::FromString("Objects: {0}"),
			SectorInfo.ObjectCount
		);
		Text_ObjectCount->SetText(CountText);
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Updated sector info for '%s'"), *SectorInfo.SectorName.ToString());
	
	// Blueprint implementation handles visual display
}

void USectorMapWidget::UpdateObjectList_Implementation(const TArray<FText>& ObjectNames, int32 ObjectCount)
{
	SectorObjectNames = ObjectNames;
	CurrentSectorInfo.ObjectCount = ObjectCount;
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Updated object list with %d objects"), ObjectCount);
	
	// Blueprint implementation handles visual display
}

void USectorMapWidget::SetTargetSector(ASpaceSectorMap* Sector)
{
	if (Sector == CurrentSector)
	{
		return; // No change needed
	}
	
	CurrentSector = Sector;
	
	if (Sector)
	{
		// Build and update sector info
		FSectorDisplayInfo Info = BuildSectorDisplayInfo(Sector);
		UpdateSectorInfo(Info);
		
		UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Set target sector to '%s'"), *Sector->SectorName.ToString());
	}
	else
	{
		ClearSectorInfo();
		UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Cleared target sector"));
	}
}

void USectorMapWidget::RefreshSectorData()
{
	if (CurrentSector)
	{
		FSectorDisplayInfo Info = BuildSectorDisplayInfo(CurrentSector);
		UpdateSectorInfo(Info);
		
		UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Refreshed sector data for '%s'"), *CurrentSector->SectorName.ToString());
	}
}

void USectorMapWidget::OnSectorSelected_Implementation(ASpaceSectorMap* SelectedSector)
{
	SetTargetSector(SelectedSector);
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Sector selected - '%s'"), 
		SelectedSector ? *SelectedSector->SectorName.ToString() : TEXT("None"));
	
	// Blueprint implementation can add custom selection behavior
}

void USectorMapWidget::ClearSectorInfo_Implementation()
{
	CurrentSectorInfo = FSectorDisplayInfo();
	SectorObjectNames.Empty();
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Cleared sector info"));
	
	// Blueprint implementation handles clearing visual display
}

void USectorMapWidget::ToggleSectorMapVisibility(bool bVisible)
{
	bIsSectorMapVisible = bVisible;
	
	if (bVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Visibility set to %s"), bVisible ? TEXT("Visible") : TEXT("Hidden"));
}

FVector USectorMapWidget::GetRandomSpawnPosition() const
{
	if (CurrentSector)
	{
		return CurrentSector->GetRandomPositionInSector();
	}
	
	UE_LOG(LogAdastrea, Warning, TEXT("SectorMapWidget: GetRandomSpawnPosition called with no current sector"));
	return FVector::ZeroVector;
}

bool USectorMapWidget::IsPositionInCurrentSector(const FVector& WorldPosition) const
{
	if (CurrentSector)
	{
		return CurrentSector->IsPositionInSector(WorldPosition);
	}
	
	return false;
}

FSectorDisplayInfo USectorMapWidget::BuildSectorDisplayInfo(ASpaceSectorMap* Sector) const
{
	FSectorDisplayInfo Info;
	
	if (Sector)
	{
		Info.SectorName = Sector->SectorName;
		Info.Description = Sector->Description;
		Info.SectorCenter = Sector->GetSectorCenter();
		// Use GetSectorBounds to get size rather than accessing static member
		FBox SectorBounds = Sector->GetSectorBounds();
		Info.SectorSize = SectorBounds.GetSize().X; // All dimensions are equal for cubic sectors
		Info.ObjectCount = 0; // Can be updated by Blueprint or game logic
	}
	
	return Info;
}

TArray<ASpaceSectorMap*> USectorMapWidget::GetAllSectors() const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, ASpaceSectorMap::StaticClass(), FoundActors);
	
	TArray<ASpaceSectorMap*> Sectors;
	for (AActor* Actor : FoundActors)
	{
		ASpaceSectorMap* Sector = Cast<ASpaceSectorMap>(Actor);
		if (Sector)
		{
			Sectors.Add(Sector);
		}
	}
	
	return Sectors;
}

TArray<ASpaceSectorMap*> USectorMapWidget::GetNeighboringSectors() const
{
	TArray<ASpaceSectorMap*> Neighbors;
	
	if (!CurrentSector)
	{
		return Neighbors;
	}
	
	// Get all sectors
	TArray<ASpaceSectorMap*> AllSectors = GetAllSectors();
	
	// Current sector center and size
	FVector CurrentCenter = CurrentSector->GetSectorCenter();
	FBox CurrentBounds = CurrentSector->GetSectorBounds();
	float SectorSize = CurrentBounds.GetSize().X; // All dimensions are equal for cubic sectors
	
	// Check each sector to see if it's adjacent (within 1.5 sector sizes)
	float MaxDistance = SectorSize * 1.5f;
	
	for (ASpaceSectorMap* Sector : AllSectors)
	{
		if (Sector == CurrentSector)
		{
			continue; // Skip self
		}
		
		FVector OtherCenter = Sector->GetSectorCenter();
		float Distance = FVector::Dist(CurrentCenter, OtherCenter);
		
		if (Distance <= MaxDistance)
		{
			Neighbors.Add(Sector);
		}
	}
	
	return Neighbors;
}

float USectorMapWidget::GetDistanceToSector(ASpaceSectorMap* OtherSector) const
{
	if (!CurrentSector || !OtherSector)
	{
		return -1.0f;
	}
	
	FVector CurrentCenter = CurrentSector->GetSectorCenter();
	FVector OtherCenter = OtherSector->GetSectorCenter();
	
	return FVector::Dist(CurrentCenter, OtherCenter);
}

void USectorMapWidget::CreateDefaultUIWidgets()
{
	if (!WidgetTree)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SectorMapWidget: Cannot create default widgets - WidgetTree is null"));
		return;
	}

	// Get or create root canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		if (!RootCanvas)
		{
			UE_LOG(LogAdastrea, Error, TEXT("SectorMapWidget: Failed to create root canvas panel"));
			return;
		}
		WidgetTree->RootWidget = RootCanvas;
		UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Created root canvas panel"));
	}

	// Create background border if not exists
	UBorder* Background = Cast<UBorder>(WidgetTree->FindWidget(TEXT("Background")));
	if (!Background)
	{
		Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
		if (Background && RootCanvas)
		{
			Background->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.1f, 0.9f));
			RootCanvas->AddChild(Background);
			UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(Background->Slot);
			if (BgSlot)
			{
				BgSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				BgSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				BgSlot->SetPosition(FVector2D(0.0f, 0.0f));
				BgSlot->SetSize(FVector2D(600.0f, 700.0f));
			}
			UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Created background border"));
		}
	}

	// Create main content vertical box
	UVerticalBox* MainContent = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("MainContent")));
	if (!MainContent && Background)
	{
		MainContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainContent"));
		if (MainContent)
		{
			Background->AddChild(MainContent);
			UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Created main content vertical box"));
		}
	}

	// Create sector name text if not exists
	if (!Text_SectorName && MainContent)
	{
		Text_SectorName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_SectorName"));
		if (Text_SectorName)
		{
			Text_SectorName->SetText(FText::FromString("Sector Name"));
			Text_SectorName->SetJustification(ETextJustify::Center);
			FSlateFontInfo FontInfo = Text_SectorName->GetFont();
			FontInfo.Size = 28;
			Text_SectorName->SetFont(FontInfo);
			MainContent->AddChild(Text_SectorName);
			UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Created sector name text"));
		}
	}

	// Create sector description text if not exists
	if (!Text_SectorDescription && MainContent)
	{
		Text_SectorDescription = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_SectorDescription"));
		if (Text_SectorDescription)
		{
			Text_SectorDescription->SetText(FText::FromString("Sector description will appear here"));
			Text_SectorDescription->SetAutoWrapText(true);
			FSlateFontInfo FontInfo = Text_SectorDescription->GetFont();
			FontInfo.Size = 14;
			Text_SectorDescription->SetFont(FontInfo);
			MainContent->AddChild(Text_SectorDescription);
			UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Created sector description text"));
		}
	}

	// Create object count text if not exists
	if (!Text_ObjectCount && MainContent)
	{
		Text_ObjectCount = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_ObjectCount"));
		if (Text_ObjectCount)
		{
			Text_ObjectCount->SetText(FText::FromString("Objects: 0"));
			FSlateFontInfo FontInfo = Text_ObjectCount->GetFont();
			FontInfo.Size = 16;
			Text_ObjectCount->SetFont(FontInfo);
			MainContent->AddChild(Text_ObjectCount);
			UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Created object count text"));
		}
	}

	// Create object list scroll box if not exists
	if (!ObjectListScrollBox && MainContent)
	{
		ObjectListScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ObjectListScrollBox"));
		if (ObjectListScrollBox)
		{
			MainContent->AddChild(ObjectListScrollBox);
			UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Created object list scroll box"));
		}
	}
}

FString USectorMapWidget::GetSectorStatistics() const
{
	if (!CurrentSector)
	{
		return TEXT("No sector selected");
	}
	
	return CurrentSector->GetDebugInfo();
}

void USectorMapWidget::UpdateObjectTracking()
{
	if (!CurrentSector)
	{
		return;
	}
	
	// Get actors in sector
	TArray<AActor*> ActorsInSector = CurrentSector->GetActorsInSector();
	
	// Update object list
	TArray<FText> ObjectNames;
	for (AActor* Actor : ActorsInSector)
	{
		if (Actor)
		{
			ObjectNames.Add(FText::FromString(Actor->GetName()));
		}
	}
	
	UpdateObjectList(ObjectNames, ObjectNames.Num());
	
	UE_LOG(LogAdastrea, Log, TEXT("SectorMapWidget: Updated object tracking - %d objects found"), ObjectNames.Num());
}

float USectorMapWidget::GetPlayerDistanceToSectorCenter() const
{
	if (!CurrentSector)
	{
		return -1.0f;
	}
	
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->GetPawn())
	{
		return -1.0f;
	}
	
	FVector PlayerPosition = PC->GetPawn()->GetActorLocation();
	FVector SectorCenter = CurrentSector->GetSectorCenter();
	
	return FVector::Dist(PlayerPosition, SectorCenter);
}

bool USectorMapWidget::IsPlayerInSector() const
{
	if (!CurrentSector)
	{
		return false;
	}
	
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->GetPawn())
	{
		return false;
	}
	
	FVector PlayerPosition = PC->GetPawn()->GetActorLocation();
	return CurrentSector->IsPositionInSector(PlayerPosition);
}

FVector USectorMapWidget::GetNavigationDirectionToCenter() const
{
	if (!CurrentSector)
	{
		return FVector::ZeroVector;
	}
	
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->GetPawn())
	{
		return FVector::ZeroVector;
	}
	
	FVector PlayerPosition = PC->GetPawn()->GetActorLocation();
	FVector SectorCenter = CurrentSector->GetSectorCenter();
	
	FVector Direction = (SectorCenter - PlayerPosition);
	Direction.Normalize();
	
	return Direction;
}

float USectorMapWidget::CalculateTravelTimeToSector(float TravelSpeed) const
{
	if (TravelSpeed <= 0.0f)
	{
		return -1.0f;
	}
	
	float Distance = GetPlayerDistanceToSectorCenter();
	if (Distance < 0.0f)
	{
		return -1.0f;
	}
	
	return Distance / TravelSpeed;
}
