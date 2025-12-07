// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/UniverseMapWidget.h"
#include "SpaceSectorMap.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

UUniverseMapWidget::UUniverseMapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SelectedSector(nullptr)
	, bIsUniverseMapVisible(false)
	, bAutoDiscoverVisitedSectors(true)
	, bShowUndiscoveredSectors(true)
{
}

void UUniverseMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
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
	// Blueprint implementation handles visual display of the grid
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
