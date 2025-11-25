#include "UI/SectorMapWidget.h"
#include "SpaceSectorMap.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"

USectorMapWidget::USectorMapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentSector(nullptr)
	, bIsSectorMapVisible(false)
{
	// Initialize default sector info
	CurrentSectorInfo = FSectorDisplayInfo();
}

void USectorMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
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
		Info.SectorSize = ASpaceSectorMap::SectorSize;
		Info.ObjectCount = 0; // Can be updated by Blueprint or game logic
	}
	
	return Info;
}
