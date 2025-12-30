#include "Exploration/ScannerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "AdastreaLog.h"

UScannerComponent::UScannerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;  // Update 10 times per second

	// Default values
	ScannerData = nullptr;
	bPassiveScanningEnabled = true;
	bDebugShowDetections = false;

	// State
	bScanInProgress = false;
	ScanCooldownRemaining = 0.0f;
	TimeSinceLastPassiveScan = 0.0f;
}

void UScannerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ScannerData)
	{
		UE_LOG(LogAdastreaExploration, Warning, TEXT("ScannerComponent: No ScannerDataAsset assigned!"));
	}
}

void UScannerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ScannerData)
	{
		return;
	}

	// Update passive scanning
	if (bPassiveScanningEnabled)
	{
		UpdatePassiveScanning(DeltaTime);
	}

	// Update active scan
	if (bScanInProgress)
	{
		UpdateActiveScan(DeltaTime);
	}

	// Update cooldown
	if (ScanCooldownRemaining > 0.0f)
	{
		ScanCooldownRemaining -= DeltaTime;
	}
}

void UScannerComponent::UpdatePassiveScanning(float DeltaTime)
{
	TimeSinceLastPassiveScan += DeltaTime;

	// Check if time to scan
	if (TimeSinceLastPassiveScan < ScannerData->PassiveScanInterval)
	{
		return;
	}

	TimeSinceLastPassiveScan = 0.0f;

	// Find scannable objects in passive range
	TArray<UScannableObjectComponent*> ScannableObjects = FindScannableObjectsInRange(ScannerData->PassiveRange);

	// Track which objects are still in range
	TArray<AActor*> CurrentlyDetected;

	for (UScannableObjectComponent* Scannable : ScannableObjects)
	{
		if (!Scannable || !Scannable->GetOwner())
		{
			continue;
		}

		AActor* Actor = Scannable->GetOwner();
		AActor* OwnerActor = GetOwner();
		if (!OwnerActor)
		{
			continue;
		}

		float Distance = FVector::Dist(OwnerActor->GetActorLocation(), Actor->GetActorLocation());

		// Check if detectable
		if (!Scannable->IsDetectable(ScannerData, Distance, false))
		{
			continue;
		}

		CurrentlyDetected.Add(Actor);

		// Check if already in detected list
		bool bAlreadyDetected = false;
		for (FDetectedObject& Detected : DetectedObjects)
		{
			if (Detected.DetectedActor == Actor)
			{
				// Update existing detection
				Detected.Distance = Distance;
				Detected.Direction = (Actor->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal();
				Detected.DetailLevel = ScannerData->GetDetailLevelAtDistance(Distance);
				bAlreadyDetected = true;
				break;
			}
		}

		if (!bAlreadyDetected)
		{
			// New detection
			FDetectedObject NewDetection;
			NewDetection.DetectedActor = Actor;
			NewDetection.ScannableComponent = Scannable;
			NewDetection.Distance = Distance;
			NewDetection.Direction = (Actor->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal();
			NewDetection.DetailLevel = ScannerData->GetDetailLevelAtDistance(Distance);
			NewDetection.DetectionTime = GetWorld()->GetTimeSeconds();

			DetectedObjects.Add(NewDetection);

			if (bDebugShowDetections)
			{
				UE_LOG(LogAdastreaExploration, Log, TEXT("Scanner: Detected new object '%s' at distance %.0f cm"), 
					*Scannable->DisplayName.ToString(), Distance);
			}

			OnObjectDetected(NewDetection);
		}
	}

	// Remove objects that are no longer in range
	for (int32 i = DetectedObjects.Num() - 1; i >= 0; --i)
	{
		if (!DetectedObjects[i].DetectedActor || !CurrentlyDetected.Contains(DetectedObjects[i].DetectedActor))
		{
			FDetectedObject LostObject = DetectedObjects[i];
			DetectedObjects.RemoveAt(i);

			if (bDebugShowDetections && LostObject.ScannableComponent)
			{
				UE_LOG(LogAdastreaExploration, Log, TEXT("Scanner: Lost object '%s'"), 
					*LostObject.ScannableComponent->DisplayName.ToString());
			}

			OnObjectLost(LostObject);
		}
	}

	// Enforce max simultaneous targets limit
	if (DetectedObjects.Num() > ScannerData->MaxSimultaneousTargets)
	{
		// Sort by distance and keep closest
		DetectedObjects.Sort([](const FDetectedObject& A, const FDetectedObject& B) {
			return A.Distance < B.Distance;
		});

		int32 NumToRemove = DetectedObjects.Num() - ScannerData->MaxSimultaneousTargets;
		DetectedObjects.RemoveAt(ScannerData->MaxSimultaneousTargets, NumToRemove);
	}
}

void UScannerComponent::UpdateActiveScan(float DeltaTime)
{
	if (!CurrentScan.TargetActor)
	{
		CancelActiveScan();
		return;
	}

	// Update progress
	float ElapsedTime = GetWorld()->GetTimeSeconds() - CurrentScan.StartTime;
	CurrentScan.Progress = FMath::Clamp(ElapsedTime / CurrentScan.Duration, 0.0f, 1.0f);

	// Check if complete
	if (CurrentScan.Progress >= 1.0f)
	{
		CompleteActiveScan();
	}
}

TArray<UScannableObjectComponent*> UScannerComponent::FindScannableObjectsInRange(float Range)
{
	TArray<UScannableObjectComponent*> Result;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !GetWorld())
	{
		return Result;
	}

	FVector ScanOrigin = OwnerActor->GetActorLocation();

	// Use spatial query to find actors within range
	// This is much more efficient than GetAllActorsOfClass
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Range);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ScanOrigin,
		FQuat::Identity,
		ECC_WorldDynamic,  // Consider using a custom channel for scannable objects
		SphereShape,
		QueryParams
	);

	// Extract scannable components from overlapped actors
	for (const FOverlapResult& Overlap : OverlapResults)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || Actor == OwnerActor)
		{
			continue;
		}

		UScannableObjectComponent* Scannable = Actor->FindComponentByClass<UScannableObjectComponent>();
		if (Scannable)
		{
			Result.Add(Scannable);
		}
	}

	return Result;
}

TArray<FDetectedObject> UScannerComponent::GetDetectedObjectsByType(ESignalType SignalType) const
{
	TArray<FDetectedObject> Result;

	for (const FDetectedObject& Detected : DetectedObjects)
	{
		if (Detected.ScannableComponent)
		{
			ESignalType VisibleType = Detected.ScannableComponent->GetVisibleSignalType(ScannerData);
			if (VisibleType == SignalType)
			{
				Result.Add(Detected);
			}
		}
	}

	return Result;
}

FDetectedObject UScannerComponent::GetNearestDetectedObject() const
{
	if (DetectedObjects.Num() == 0)
	{
		return FDetectedObject();
	}

	FDetectedObject Nearest = DetectedObjects[0];
	for (const FDetectedObject& Detected : DetectedObjects)
	{
		if (Detected.Distance < Nearest.Distance)
		{
			Nearest = Detected;
		}
	}

	return Nearest;
}

bool UScannerComponent::IsObjectDetected(AActor* Actor) const
{
	for (const FDetectedObject& Detected : DetectedObjects)
	{
		if (Detected.DetectedActor == Actor)
		{
			return true;
		}
	}

	return false;
}

bool UScannerComponent::StartActiveScan(AActor* Target, EScanMode ScanMode)
{
	FText Reason;
	if (!CanStartActiveScan(Target, Reason))
	{
		OnScanFailed(Reason);
		return false;
	}

	// Check if deep scan is requested but not supported
	if (ScanMode == EScanMode::DeepScan && !ScannerData->bCanDeepScan)
	{
		ScanMode = EScanMode::Active;
	}

	// Setup scan
	CurrentScan.TargetActor = Target;
	CurrentScan.ScanMode = ScanMode;
	CurrentScan.StartTime = GetWorld()->GetTimeSeconds();
	CurrentScan.Duration = ScannerData->GetScanTime(ScanMode == EScanMode::DeepScan);
	CurrentScan.Progress = 0.0f;

	bScanInProgress = true;

	// Start cooldown immediately to prevent scan spam
	// Note: Cooldown applies even if scan is cancelled to prevent abuse
	ScanCooldownRemaining = ScannerData->ScanCooldown;

	if (bDebugShowDetections)
	{
		UE_LOG(LogAdastreaExploration, Log, TEXT("Scanner: Started %s scan of '%s' (%.1f seconds)"), 
			ScanMode == EScanMode::DeepScan ? TEXT("deep") : TEXT("active"),
			*Target->GetName(), CurrentScan.Duration);
	}

	return true;
}

void UScannerComponent::CancelActiveScan()
{
	if (bScanInProgress)
	{
		if (bDebugShowDetections)
		{
			UE_LOG(LogAdastreaExploration, Log, TEXT("Scanner: Active scan cancelled"));
		}

		OnScanFailed(FText::FromString("Scan cancelled"));
	}

	bScanInProgress = false;
	CurrentScan = FActiveScanInfo();
}

bool UScannerComponent::CanStartActiveScan(AActor* Target, FText& OutReason) const
{
	if (!ScannerData)
	{
		OutReason = FText::FromString("No scanner data configured");
		return false;
	}

	if (!Target)
	{
		OutReason = FText::FromString("No target specified");
		return false;
	}

	if (bScanInProgress)
	{
		OutReason = FText::FromString("Scan already in progress");
		return false;
	}

	if (ScanCooldownRemaining > 0.0f)
	{
		OutReason = FText::FromString("Scanner on cooldown");
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		OutReason = FText::FromString("Scanner has no owner");
		return false;
	}

	float Distance = FVector::Dist(OwnerActor->GetActorLocation(), Target->GetActorLocation());
	if (Distance > ScannerData->ActiveRange)
	{
		OutReason = FText::FromString("Target out of range");
		return false;
	}

	UScannableObjectComponent* Scannable = Target->FindComponentByClass<UScannableObjectComponent>();
	if (!Scannable)
	{
		OutReason = FText::FromString("Target is not scannable");
		return false;
	}

	return true;
}

float UScannerComponent::GetScanProgress() const
{
	return bScanInProgress ? CurrentScan.Progress : 0.0f;
}

void UScannerComponent::CompleteActiveScan()
{
	if (!CurrentScan.TargetActor)
	{
		CancelActiveScan();
		return;
	}

	UScannableObjectComponent* Scannable = CurrentScan.TargetActor->FindComponentByClass<UScannableObjectComponent>();
	if (!Scannable)
	{
		CancelActiveScan();
		return;
	}

	// Determine detail level based on scan mode
	EScanDetailLevel DetailLevel = EScanDetailLevel::Detailed;
	if (CurrentScan.ScanMode == EScanMode::DeepScan)
	{
		DetailLevel = EScanDetailLevel::Complete;
	}

	// Mark as scanned
	Scannable->MarkAsScanned(DetailLevel);

	// Get scan data
	FScanData ScanData = Scannable->GetScanData(DetailLevel);

	// Find or create detected object entry
	FDetectedObject* DetectedObj = nullptr;
	for (FDetectedObject& Detected : DetectedObjects)
	{
		if (Detected.DetectedActor == CurrentScan.TargetActor)
		{
			Detected.DetailLevel = DetailLevel;
			DetectedObj = &Detected;
			break;
		}
	}

	if (!DetectedObj)
	{
		// Create new entry
		FDetectedObject NewDetection;
		NewDetection.DetectedActor = CurrentScan.TargetActor;
		NewDetection.ScannableComponent = Scannable;
		NewDetection.Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentScan.TargetActor->GetActorLocation());
		NewDetection.Direction = (CurrentScan.TargetActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
		NewDetection.DetailLevel = DetailLevel;
		NewDetection.DetectionTime = GetWorld()->GetTimeSeconds();
		DetectedObjects.Add(NewDetection);
		DetectedObj = &DetectedObjects.Last();
	}

	if (bDebugShowDetections)
	{
		UE_LOG(LogAdastreaExploration, Log, TEXT("Scanner: Scan complete for '%s' - Detail Level: %d"), 
			*ScanData.ObjectName.ToString(), (int32)DetailLevel);
	}

	OnScanComplete(*DetectedObj, ScanData);

	bScanInProgress = false;
	CurrentScan = FActiveScanInfo();
}

ESignalType UScannerComponent::IdentifySignal(const FDetectedObject& DetectedObject) const
{
	if (!DetectedObject.ScannableComponent)
	{
		return ESignalType::Unknown;
	}

	return DetectedObject.ScannableComponent->GetVisibleSignalType(ScannerData);
}

FScanData UScannerComponent::GetScanDataForObject(const FDetectedObject& DetectedObject) const
{
	if (!DetectedObject.ScannableComponent)
	{
		return FScanData();
	}

	return DetectedObject.ScannableComponent->GetScanData(DetectedObject.DetailLevel);
}

void UScannerComponent::RemoveDetectedObject(AActor* Actor)
{
	for (int32 i = DetectedObjects.Num() - 1; i >= 0; --i)
	{
		if (DetectedObjects[i].DetectedActor == Actor)
		{
			DetectedObjects.RemoveAt(i);
			break;
		}
	}
}
