// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/UniverseMapWidget.h"
#include "SpaceSectorMap.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

UUniverseMapWidget::UUniverseMapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SelectedSector(nullptr)
	, bIsUniverseMapVisible(false)
	, SectorGridPanel(nullptr)
	, Text_SectorName(nullptr)
	, Text_SectorDescription(nullptr)
	, ProgressBar_Exploration(nullptr)
	, Text_ExplorationPercent(nullptr)
	, Button_Close(nullptr)
	, bAutoDiscoverVisitedSectors(true)
	, bShowUndiscoveredSectors(true)
	, bAutoCreateMissingWidgets(true)
{
}

void UUniverseMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Create default UI widgets if they don't exist and auto-create is enabled
	if (bAutoCreateMissingWidgets)
	{
		CreateDefaultUIWidgets();
	}
	
	// Setup close button if it exists
	if (Button_Close)
	{
		Button_Close->OnClicked.AddDynamic(this, &UUniverseMapWidget::OnCloseButtonClicked);
	}
	
	// Initialize the universe map when constructed
	InitializeUniverseMap();
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Widget constructed"));
}

void UUniverseMapWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	// Clear references
	AllSectors.Empty();
	SelectedSector = nullptr;
	DiscoveredSectors.Empty();
	UniverseSectorInfo.Empty();
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Widget destructed"));
}

void UUniverseMapWidget::InitializeUniverseMap_Implementation()
{
	// Find all sectors in the world
	AllSectors = FindAllSectorsInWorld();
	
	// Build the sector grid
	BuildUniverseSectorGrid();
	
	// Auto-discover the player's current sector
	if (bAutoDiscoverVisitedSectors)
	{
		ASpaceSectorMap* PlayerSector = GetPlayerCurrentSector();
		if (PlayerSector)
		{
			MarkSectorDiscovered(PlayerSector);
		}
	}
	
	// Update the grid display
	UpdateUniverseGrid();
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Initialized with %d sectors"), AllSectors.Num());
}

void UUniverseMapWidget::RefreshUniverseMap()
{
	// Re-find all sectors (in case new ones were spawned)
	AllSectors = FindAllSectorsInWorld();
	
	// Rebuild the grid
	BuildUniverseSectorGrid();
	
	// Update the display
	UpdateUniverseGrid();
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Refreshed - now showing %d sectors"), AllSectors.Num());
}

void UUniverseMapWidget::UpdateUniverseGrid_Implementation()
{
	// Update exploration progress display if widgets exist
	if (ProgressBar_Exploration)
	{
		float Progress = GetExplorationProgress();
		ProgressBar_Exploration->SetPercent(Progress);
	}

	if (Text_ExplorationPercent)
	{
		float Progress = GetExplorationProgress();
		int32 ProgressPercent = FMath::RoundToInt(Progress * 100.0f);
		FText ProgressText = FText::Format(
			FText::FromString("Exploration: {0}% ({1}/{2})"),
			ProgressPercent,
			GetDiscoveredSectorCount(),
			GetTotalSectorCount()
		);
		Text_ExplorationPercent->SetText(ProgressText);
	}

	// Update selected sector info if a sector is selected
	if (SelectedSector)
	{
		if (Text_SectorName)
		{
			Text_SectorName->SetText(SelectedSector->SectorName);
		}

		if (Text_SectorDescription)
		{
			Text_SectorDescription->SetText(SelectedSector->Description);
		}
	}
	else
	{
		// No sector selected - show default text
		if (Text_SectorName)
		{
			Text_SectorName->SetText(FText::FromString("No Sector Selected"));
		}

		if (Text_SectorDescription)
		{
			Text_SectorDescription->SetText(FText::FromString("Select a sector to view details"));
		}
	}
	
	// Blueprint implementation can add additional visual display of the grid
	// This can draw sector icons, connections, labels, etc.
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Grid updated - %d sectors, %d discovered"), 
		AllSectors.Num(), GetDiscoveredSectorCount());
}

void UUniverseMapWidget::SetSelectedSector(ASpaceSectorMap* Sector)
{
	if (SelectedSector == Sector)
	{
		return; // No change needed
	}
	
	// Update selection in sector info
	for (FUniverseSectorInfo& Info : UniverseSectorInfo)
	{
		Info.bIsSelected = (Info.Sector == Sector);
	}
	
	SelectedSector = Sector;
	
	// Trigger selection event
	OnSectorSelected(Sector);
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Selected sector '%s'"), 
		Sector ? *Sector->SectorName.ToString() : TEXT("None"));
}

ASpaceSectorMap* UUniverseMapWidget::GetSectorAtGridCoordinates(const FIntVector& GridCoords) const
{
	for (const FUniverseSectorInfo& Info : UniverseSectorInfo)
	{
		if (Info.GridCoordinates == GridCoords && Info.Sector)
		{
			return Info.Sector;
		}
	}
	return nullptr;
}

ASpaceSectorMap* UUniverseMapWidget::GetPlayerCurrentSector() const
{
	// Get player controller
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return nullptr;
	}
	
	// Get player pawn (should be a spaceship)
	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		return nullptr;
	}
	
	// Get player position
	FVector PlayerPosition = PlayerPawn->GetActorLocation();
	
	// Find which sector contains the player
	for (ASpaceSectorMap* Sector : AllSectors)
	{
		if (Sector && Sector->IsPositionInSector(PlayerPosition))
		{
			return Sector;
		}
	}
	
	return nullptr;
}

bool UUniverseMapWidget::IsSectorDiscovered(ASpaceSectorMap* Sector) const
{
	return DiscoveredSectors.Contains(Sector);
}

void UUniverseMapWidget::MarkSectorDiscovered(ASpaceSectorMap* Sector)
{
	if (!Sector)
	{
		return;
	}
	
	bool bWasAlreadyDiscovered = DiscoveredSectors.Contains(Sector);
	DiscoveredSectors.Add(Sector);
	
	// Update the sector info
	for (FUniverseSectorInfo& Info : UniverseSectorInfo)
	{
		if (Info.Sector == Sector)
		{
			Info.bIsDiscovered = true;
			break;
		}
	}
	
	if (!bWasAlreadyDiscovered)
	{
		UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Discovered sector '%s'"), *Sector->SectorName.ToString());
		
		// Update the grid to show the newly discovered sector
		UpdateUniverseGrid();
	}
}

void UUniverseMapWidget::ToggleUniverseMapVisibility(bool bVisible)
{
	bIsUniverseMapVisible = bVisible;
	
	if (bVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
		OnMapOpened();
		
		// Refresh data when opening
		RefreshUniverseMap();
		
		// Auto-select player's current sector
		ASpaceSectorMap* PlayerSector = GetPlayerCurrentSector();
		if (PlayerSector)
		{
			SetSelectedSector(PlayerSector);
			
			// Auto-discover if configured
			if (bAutoDiscoverVisitedSectors)
			{
				MarkSectorDiscovered(PlayerSector);
			}
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
		OnMapClosed();
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Visibility set to %s"), bVisible ? TEXT("Visible") : TEXT("Hidden"));
}

void UUniverseMapWidget::OnSectorSelected_Implementation(ASpaceSectorMap* Sector)
{
	// Blueprint implementation can add custom selection behavior
	// E.g., show sector details panel, allow navigation, etc.
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Sector selected event - '%s'"), 
		Sector ? *Sector->SectorName.ToString() : TEXT("None"));
}

void UUniverseMapWidget::OnMapOpened_Implementation()
{
	// Blueprint implementation can add custom behavior when map opens
	// E.g., pause game, show tutorial, play sound effect, etc.
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Map opened"));
}

void UUniverseMapWidget::OnMapClosed_Implementation()
{
	// Blueprint implementation can add custom behavior when map closes
	// E.g., resume game, save settings, etc.
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Map closed"));
}

int32 UUniverseMapWidget::GetDiscoveredSectorCount() const
{
	return DiscoveredSectors.Num();
}

float UUniverseMapWidget::GetExplorationProgress() const
{
	int32 TotalSectors = GetTotalSectorCount();
	if (TotalSectors == 0)
	{
		return 0.0f;
	}
	
	int32 DiscoveredCount = GetDiscoveredSectorCount();
	return static_cast<float>(DiscoveredCount) / static_cast<float>(TotalSectors);
}

void UUniverseMapWidget::BuildUniverseSectorGrid()
{
	UniverseSectorInfo.Empty();
	
	// Calculate grid coordinates for each sector
	for (ASpaceSectorMap* Sector : AllSectors)
	{
		if (!Sector)
		{
			continue;
		}
		
		FUniverseSectorInfo Info;
		Info.Sector = Sector;
		Info.GridCoordinates = CalculateGridCoordinates(Sector);
		Info.bIsDiscovered = IsSectorDiscovered(Sector);
		Info.bIsSelected = (Sector == SelectedSector);
		
		UniverseSectorInfo.Add(Info);
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Built sector grid with %d sectors"), UniverseSectorInfo.Num());
}

FIntVector UUniverseMapWidget::CalculateGridCoordinates(ASpaceSectorMap* Sector) const
{
	if (!Sector)
	{
		return FIntVector::ZeroValue;
	}
	
	// Get sector center position
	FVector SectorCenter = Sector->GetSectorCenter();
	
	// Convert world position to grid coordinates
	// Each grid cell is one sector size (20,000,000 units = 200km)
	// Use the sector's bounds to determine size rather than directly accessing static member
	FBox SectorBounds = Sector->GetSectorBounds();
	float SectorSize = SectorBounds.GetSize().X; // All dimensions are equal for cubic sectors
	
	int32 GridX = FMath::RoundToInt(SectorCenter.X / SectorSize);
	int32 GridY = FMath::RoundToInt(SectorCenter.Y / SectorSize);
	int32 GridZ = FMath::RoundToInt(SectorCenter.Z / SectorSize);
	
	return FIntVector(GridX, GridY, GridZ);
}

TArray<ASpaceSectorMap*> UUniverseMapWidget::FindAllSectorsInWorld() const
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

void UUniverseMapWidget::CreateDefaultUIWidgets()
{
	if (!WidgetTree)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("UniverseMapWidget: Cannot create default widgets - WidgetTree is null"));
		return;
	}

	// Get or create root canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		if (!RootCanvas)
		{
			UE_LOG(LogAdastrea, Error, TEXT("UniverseMapWidget: Failed to create root canvas panel"));
			return;
		}
		WidgetTree->RootWidget = RootCanvas;
		UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created root canvas panel"));
	}

	// Create background image if not exists
	if (!WidgetTree->FindWidget(TEXT("Background")))
	{
		UImage* Background = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Background"));
		if (Background && RootCanvas)
		{
			Background->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.05f, 0.9f));
			RootCanvas->AddChild(Background);
			UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(Background->Slot);
			if (BgSlot)
			{
				BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				BgSlot->SetOffsets(FMargin(0.0f));
			}
			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created background image"));
		}
	}

	// Create map container border if not exists
	UBorder* MapContainer = Cast<UBorder>(WidgetTree->FindWidget(TEXT("MapContainer")));
	if (!MapContainer)
	{
		MapContainer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MapContainer"));
		if (MapContainer)
		{
			MapContainer->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.15f, 0.8f));
			RootCanvas->AddChild(MapContainer);
			UCanvasPanelSlot* ContainerSlot = Cast<UCanvasPanelSlot>(MapContainer->Slot);
			if (ContainerSlot)
			{
				ContainerSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				ContainerSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				ContainerSlot->SetPosition(FVector2D(0.0f, 0.0f));
				ContainerSlot->SetSize(FVector2D(1520.0f, 880.0f));
			}
			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created map container border"));
		}
	}

	// Create or find sector grid panel
	if (!SectorGridPanel)
	{
		SectorGridPanel = Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("SectorGridPanel")));
		if (!SectorGridPanel && MapContainer)
		{
			SectorGridPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SectorGridPanel"));
			if (SectorGridPanel)
			{
				MapContainer->AddChild(SectorGridPanel);
				UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created sector grid panel"));
			}
		}
	}

	// Create info panel if not exists
	UVerticalBox* InfoPanel = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("InfoPanel")));
	if (!InfoPanel)
	{
		InfoPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoPanel"));
		if (InfoPanel)
		{
			RootCanvas->AddChild(InfoPanel);
			UCanvasPanelSlot* InfoSlot = Cast<UCanvasPanelSlot>(InfoPanel->Slot);
			if (InfoSlot)
			{
				InfoSlot->SetAnchors(FAnchors(1.0f, 0.5f));
				InfoSlot->SetAlignment(FVector2D(1.0f, 0.5f));
				InfoSlot->SetPosition(FVector2D(-20.0f, 0.0f));
				InfoSlot->SetSize(FVector2D(300.0f, 880.0f));
			}
			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created info panel"));
		}
	}

	// Create sector name text if not exists
	if (!Text_SectorName && InfoPanel)
	{
		Text_SectorName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_SectorName"));
		if (Text_SectorName)
		{
			Text_SectorName->SetText(FText::FromString("Sector Name"));
			Text_SectorName->SetJustification(ETextJustify::Center);
			FSlateFontInfo FontInfo = Text_SectorName->GetFont();
			FontInfo.Size = 24;
			Text_SectorName->SetFont(FontInfo);
			InfoPanel->AddChild(Text_SectorName);
			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created sector name text"));
		}
	}

	// Create sector description text if not exists
	if (!Text_SectorDescription && InfoPanel)
	{
		Text_SectorDescription = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_SectorDescription"));
		if (Text_SectorDescription)
		{
			Text_SectorDescription->SetText(FText::FromString("Sector description will appear here..."));
			Text_SectorDescription->SetAutoWrapText(true);
			FSlateFontInfo FontInfo = Text_SectorDescription->GetFont();
			FontInfo.Size = 14;
			Text_SectorDescription->SetFont(FontInfo);
			InfoPanel->AddChild(Text_SectorDescription);
			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created sector description text"));
		}
	}

	// Create exploration progress bar if not exists
	if (!ProgressBar_Exploration && InfoPanel)
	{
		ProgressBar_Exploration = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ProgressBar_Exploration"));
		if (ProgressBar_Exploration)
		{
			ProgressBar_Exploration->SetPercent(0.0f);
			ProgressBar_Exploration->SetFillColorAndOpacity(FLinearColor(0.2f, 0.5f, 1.0f, 1.0f));
			InfoPanel->AddChild(ProgressBar_Exploration);
			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created exploration progress bar"));
		}
	}

	// Create exploration percentage text if not exists
	if (!Text_ExplorationPercent && InfoPanel)
	{
		Text_ExplorationPercent = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_ExplorationPercent"));
		if (Text_ExplorationPercent)
		{
			Text_ExplorationPercent->SetText(FText::FromString("Exploration: 0%"));
			Text_ExplorationPercent->SetJustification(ETextJustify::Center);
			FSlateFontInfo FontInfo = Text_ExplorationPercent->GetFont();
			FontInfo.Size = 16;
			Text_ExplorationPercent->SetFont(FontInfo);
			InfoPanel->AddChild(Text_ExplorationPercent);
			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created exploration percent text"));
		}
	}

	// Create close button if not exists
	if (!Button_Close)
	{
		Button_Close = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_Close"));
		if (Button_Close)
		{
			RootCanvas->AddChild(Button_Close);
			UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(Button_Close->Slot);
			if (ButtonSlot)
			{
				ButtonSlot->SetAnchors(FAnchors(1.0f, 0.0f));
				ButtonSlot->SetAlignment(FVector2D(1.0f, 0.0f));
				ButtonSlot->SetPosition(FVector2D(-70.0f, 50.0f));
				ButtonSlot->SetSize(FVector2D(50.0f, 50.0f));
			}

			// Add "X" text to button
			UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Button_Close_Text"));
			if (ButtonText)
			{
				ButtonText->SetText(FText::FromString("X"));
				ButtonText->SetJustification(ETextJustify::Center);
				FSlateFontInfo FontInfo = ButtonText->GetFont();
				FontInfo.Size = 28;
				ButtonText->SetFont(FontInfo);
				Button_Close->AddChild(ButtonText);
			}

			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Created close button"));
		}
	}
}

void UUniverseMapWidget::OnCloseButtonClicked()
{
	ToggleUniverseMapVisibility(false);
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Close button clicked"));
}

TArray<ASpaceSectorMap*> UUniverseMapWidget::FindPathBetweenSectors(ASpaceSectorMap* StartSector, ASpaceSectorMap* EndSector)
{
	TArray<ASpaceSectorMap*> Path;
	
	if (!StartSector || !EndSector || StartSector == EndSector)
	{
		return Path;
	}
	
	// Simple A* pathfinding through grid
	TSet<ASpaceSectorMap*> ClosedSet;
	TSet<ASpaceSectorMap*> OpenSet;
	TMap<ASpaceSectorMap*, ASpaceSectorMap*> CameFrom;
	TMap<ASpaceSectorMap*, int32> GScore;
	TMap<ASpaceSectorMap*, int32> FScore;
	
	OpenSet.Add(StartSector);
	GScore.Add(StartSector, 0);
	FScore.Add(StartSector, GetGridDistanceBetweenSectors(StartSector, EndSector));
	
	while (OpenSet.Num() > 0)
	{
		// Find sector with lowest FScore in OpenSet
		ASpaceSectorMap* Current = nullptr;
		int32 LowestFScore = MAX_int32;
		for (ASpaceSectorMap* Sector : OpenSet)
		{
			int32 Score = FScore.FindRef(Sector);
			if (Score < LowestFScore)
			{
				LowestFScore = Score;
				Current = Sector;
			}
		}
		
		if (Current == EndSector)
		{
			// Reconstruct path
			while (Current != nullptr)
			{
				Path.Insert(Current, 0);
				Current = CameFrom.FindRef(Current);
			}
			
			UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Found path with %d sectors"), Path.Num());
			return Path;
		}
		
		OpenSet.Remove(Current);
		ClosedSet.Add(Current);
		
		// Check neighbors
		TArray<ASpaceSectorMap*> Neighbors = Current->GetNeighboringSectors();
		for (ASpaceSectorMap* Neighbor : Neighbors)
		{
			if (ClosedSet.Contains(Neighbor))
			{
				continue;
			}
			
			int32 TentativeGScore = GScore.FindRef(Current) + 1; // Cost of 1 per sector
			
			if (!OpenSet.Contains(Neighbor))
			{
				OpenSet.Add(Neighbor);
			}
			else if (TentativeGScore >= GScore.FindRef(Neighbor))
			{
				continue;
			}
			
			CameFrom.Add(Neighbor, Current);
			GScore.Add(Neighbor, TentativeGScore);
			FScore.Add(Neighbor, TentativeGScore + GetGridDistanceBetweenSectors(Neighbor, EndSector));
		}
	}
	
	UE_LOG(LogAdastrea, Warning, TEXT("UniverseMapWidget: No path found between sectors"));
	return Path;
}

int32 UUniverseMapWidget::GetGridDistanceBetweenSectors(ASpaceSectorMap* SectorA, ASpaceSectorMap* SectorB) const
{
	if (!SectorA || !SectorB)
	{
		return MAX_int32;
	}
	
	FIntVector GridA = SectorA->GetGridCoordinates();
	FIntVector GridB = SectorB->GetGridCoordinates();
	
	// Manhattan distance
	return FMath::Abs(GridA.X - GridB.X) + FMath::Abs(GridA.Y - GridB.Y) + FMath::Abs(GridA.Z - GridB.Z);
}

void UUniverseMapWidget::AddSectorBookmark(ASpaceSectorMap* Sector, const FText& BookmarkName)
{
	if (!Sector)
	{
		return;
	}
	
	BookmarkedSectors.Add(Sector, BookmarkName);
	
	UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Added bookmark '%s' for sector '%s'"),
		*BookmarkName.ToString(), *Sector->SectorName.ToString());
	
	// Update grid display to show bookmark
	UpdateUniverseGrid();
}

void UUniverseMapWidget::RemoveSectorBookmark(ASpaceSectorMap* Sector)
{
	if (!Sector)
	{
		return;
	}
	
	if (BookmarkedSectors.Remove(Sector) > 0)
	{
		UE_LOG(LogAdastrea, Log, TEXT("UniverseMapWidget: Removed bookmark from sector '%s'"),
			*Sector->SectorName.ToString());
		
		// Update grid display
		UpdateUniverseGrid();
	}
}

bool UUniverseMapWidget::IsSectorBookmarked(ASpaceSectorMap* Sector) const
{
	return BookmarkedSectors.Contains(Sector);
}

TArray<ASpaceSectorMap*> UUniverseMapWidget::GetBookmarkedSectors() const
{
	TArray<ASpaceSectorMap*> Bookmarks;
	BookmarkedSectors.GetKeys(Bookmarks);
	return Bookmarks;
}

TArray<ASpaceSectorMap*> UUniverseMapWidget::FilterSectorsByName(const FString& SearchText) const
{
	TArray<ASpaceSectorMap*> FilteredSectors;
	
	if (SearchText.IsEmpty())
	{
		return AllSectors;
	}
	
	for (ASpaceSectorMap* Sector : AllSectors)
	{
		if (Sector && Sector->SectorName.ToString().Contains(SearchText))
		{
			FilteredSectors.Add(Sector);
		}
	}
	
	return FilteredSectors;
}
