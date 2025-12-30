#include "Exploration/ScannableObjectComponent.h"
#include "GameFramework/Actor.h"

UScannableObjectComponent::UScannableObjectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default values
	DisplayName = FText::FromString("Unknown Object");
	SignalType = ESignalType::Unknown;
	SignatureStrength = 50;
	ObjectSize = 1000.0f;  // 10 meters
	bPassivelyDetectable = true;
	bStealthEnabled = false;
	bMaskedSignature = false;
	MaskedSignalType = ESignalType::Unknown;

	// Scan state
	bHasBeenScanned = false;
	HighestScanLevel = EScanDetailLevel::None;
	DiscoveryTime = 0.0f;
}

void UScannableObjectComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeScanData();
}

void UScannableObjectComponent::InitializeScanData()
{
	// Initialize basic scan data if not set
	if (BasicScanData.ObjectName.IsEmpty())
	{
		BasicScanData.ObjectName = DisplayName;
		BasicScanData.SignalType = SignalType;
		BasicScanData.DetailLevel = EScanDetailLevel::Basic;
	}

	// Initialize detailed scan data if not set
	if (DetailedScanData.ObjectName.IsEmpty())
	{
		DetailedScanData.ObjectName = DisplayName;
		DetailedScanData.SignalType = SignalType;
		DetailedScanData.DetailLevel = EScanDetailLevel::Detailed;
	}

	// Initialize complete scan data if not set
	if (CompleteScanData.ObjectName.IsEmpty())
	{
		CompleteScanData.ObjectName = DisplayName;
		CompleteScanData.SignalType = SignalType;
		CompleteScanData.DetailLevel = EScanDetailLevel::Complete;
	}
}

FScanData UScannableObjectComponent::GetScanData(EScanDetailLevel DetailLevel) const
{
	switch (DetailLevel)
	{
		case EScanDetailLevel::Basic:
			return BasicScanData;

		case EScanDetailLevel::Detailed:
			return DetailedScanData;

		case EScanDetailLevel::Complete:
			return CompleteScanData;

		default:
			return FScanData();
	}
}

bool UScannableObjectComponent::IsDetectable(const UScannerDataAsset* ScannerData, float Distance, bool bIsActiveScan) const
{
	if (!ScannerData)
	{
		return false;
	}

	// Check if object is within range
	float EffectiveRange = bIsActiveScan ? ScannerData->ActiveRange : ScannerData->PassiveRange;
	if (Distance > EffectiveRange)
	{
		return false;
	}

	// Passive detection checks
	if (!bIsActiveScan)
	{
		// Check if passively detectable
		if (!bPassivelyDetectable)
		{
			return false;
		}

		// Check stealth
		if (bStealthEnabled && !ScannerData->bCanDetectStealth)
		{
			return false;
		}

		// Check signature strength threshold
		if (SignatureStrength < ScannerData->PassiveDetectionThreshold)
		{
			return false;
		}
	}

	// Check if scanner supports this signal type
	ESignalType VisibleType = GetVisibleSignalType(ScannerData);
	if (!ScannerData->CanDetectSignalType(VisibleType))
	{
		return false;
	}

	// Check minimum object size
	if (ObjectSize < ScannerData->MinimumObjectSize)
	{
		return false;
	}

	return true;
}

ESignalType UScannableObjectComponent::GetVisibleSignalType(const UScannerDataAsset* ScannerData) const
{
	// If masking and scanner accuracy is below threshold, show masked type
	if (bMaskedSignature && ScannerData)
	{
		// Higher accuracy scanners can see through masking
		if (ScannerData->SignalIdentificationAccuracy < 75)
		{
			return MaskedSignalType;
		}
	}

	return SignalType;
}

void UScannableObjectComponent::MarkAsScanned(EScanDetailLevel DetailLevel)
{
	bool bFirstScan = !bHasBeenScanned;

	bHasBeenScanned = true;

	// Only set discovery time on first scan
	if (bFirstScan)
	{
		DiscoveryTime = GetWorld()->GetTimeSeconds();
	}

	// Update highest scan level if this is better
	if (DetailLevel > HighestScanLevel)
	{
		HighestScanLevel = DetailLevel;
	}

	// Fire events
	if (bFirstScan)
	{
		OnFirstDetected();
	}

	OnScanned(DetailLevel);
}

void UScannableObjectComponent::ResetScanState()
{
	bHasBeenScanned = false;
	HighestScanLevel = EScanDetailLevel::None;
	DiscoveryTime = 0.0f;
}
