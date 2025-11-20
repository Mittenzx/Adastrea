#include "Exploration/ScannerDataAsset.h"

UScannerDataAsset::UScannerDataAsset()
{
	// Default values for a basic scanner
	ScannerName = FText::FromString("Basic Scanner");
	Description = FText::FromString("Standard ship scanner with basic detection capabilities.");
	ScannerType = EScannerType::Basic;

	// Passive scanning defaults
	PassiveRange = 1000000.0f;  // 10km
	PassiveScanInterval = 2.0f;
	PassiveDetectionThreshold = 50;
	bCanDetectStealth = false;

	// Active scanning defaults
	ActiveRange = 5000000.0f;  // 50km
	ScanDuration = 5.0f;
	ScanCooldown = 10.0f;
	EnergyCost = 50;
	bCanDeepScan = false;
	DeepScanDurationMultiplier = 3.0f;

	// Signal identification defaults
	SignalIdentificationAccuracy = 60;
	DetailRevealDistance = 500000.0f;  // 5km
	// Empty array = supports all signal types

	// Sensor resolution defaults
	MinimumObjectSize = 100.0f;  // 1 meter
	MaxSimultaneousTargets = 50;
	AngularResolution = 5.0f;  // 5 degrees
}

EScanDetailLevel UScannerDataAsset::GetDetailLevelAtDistance(float Distance) const
{
	// Note: Logic assumes ActiveRange > PassiveRange > DetailRevealDistance (e.g., 50km > 10km > 5km)
	if (Distance > ActiveRange)
	{
		return EScanDetailLevel::None;
	}
	else if (Distance > PassiveRange)
	{
		return EScanDetailLevel::Basic;
	}
	else if (Distance > DetailRevealDistance)
	{
		return EScanDetailLevel::Detailed;
	}
	else
	{
		return EScanDetailLevel::Complete;
	}
}

bool UScannerDataAsset::CanDetectSignalType(ESignalType SignalType) const
{
	// Empty array means supports all types
	if (SupportedSignalTypes.Num() == 0)
	{
		return true;
	}

	return SupportedSignalTypes.Contains(SignalType);
}

float UScannerDataAsset::GetScanTime(bool bDeepScan) const
{
	float Time = ScanDuration;

	if (bDeepScan && bCanDeepScan)
	{
		Time *= DeepScanDurationMultiplier;
	}

	return Time;
}

float UScannerDataAsset::GetEffectiveRange(EScanMode ScanMode) const
{
	switch (ScanMode)
	{
		case EScanMode::Passive:
			return PassiveRange;

		case EScanMode::Active:
		case EScanMode::Surface:
			return ActiveRange;

		case EScanMode::DeepScan:
			return bCanDeepScan ? ActiveRange * 0.75f : ActiveRange;

		default:
			return PassiveRange;
	}
}
