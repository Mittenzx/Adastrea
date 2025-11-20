#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScannerDataAsset.generated.h"

/**
 * Scanner Type
 * Defines the type of scanner system
 */
UENUM(BlueprintType)
enum class EScannerType : uint8
{
	Basic			UMETA(DisplayName = "Basic Scanner"),
	Advanced		UMETA(DisplayName = "Advanced Scanner"),
	Military		UMETA(DisplayName = "Military Scanner"),
	Scientific		UMETA(DisplayName = "Scientific Scanner"),
	Exploration		UMETA(DisplayName = "Exploration Scanner")
};

/**
 * Scan Mode
 * Defines active scanning modes
 */
UENUM(BlueprintType)
enum class EScanMode : uint8
{
	Passive			UMETA(DisplayName = "Passive Scanning"),
	Active			UMETA(DisplayName = "Active Scanning"),
	DeepScan		UMETA(DisplayName = "Deep Scan"),
	Surface			UMETA(DisplayName = "Surface Scan")
};

/**
 * Signal Type
 * Types of signals that can be detected
 */
UENUM(BlueprintType)
enum class ESignalType : uint8
{
	Unknown			UMETA(DisplayName = "Unknown Signal"),
	Ship			UMETA(DisplayName = "Ship Signature"),
	Station			UMETA(DisplayName = "Station Signature"),
	Planet			UMETA(DisplayName = "Planetary Body"),
	Asteroid		UMETA(DisplayName = "Asteroid Field"),
	Anomaly			UMETA(DisplayName = "Spatial Anomaly"),
	Debris			UMETA(DisplayName = "Debris Field"),
	Beacon			UMETA(DisplayName = "Beacon Signal")
};

/**
 * Scan Result Detail Level
 * How much information is revealed by a scan
 */
UENUM(BlueprintType)
enum class EScanDetailLevel : uint8
{
	None			UMETA(DisplayName = "Not Detected"),
	Basic			UMETA(DisplayName = "Basic Info"),
	Detailed		UMETA(DisplayName = "Detailed Info"),
	Complete		UMETA(DisplayName = "Complete Analysis")
};

/**
 * Scanner Data Asset
 * 
 * Defines configuration for ship-mounted scanner systems including:
 * - Passive detection ranges and capabilities
 * - Active scanning power and resolution
 * - Signal identification accuracy
 * - Energy consumption and cooldown times
 * 
 * **Usage:**
 * 1. Create a Blueprint Data Asset based on this class
 * 2. Configure scanner properties for your ship type
 * 3. Assign to ScannerComponent in your spaceship Blueprint
 * 
 * **Scanner Types:**
 * - Basic: Short range, limited detail
 * - Advanced: Medium range, good detail
 * - Military: Long range combat focus, identifies threats
 * - Scientific: Deep scan capable, detailed analysis
 * - Exploration: Wide range, discovery focus
 */
UCLASS(BlueprintType)
class ADASTREA_API UScannerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UScannerDataAsset();

	// ====================
	// BASIC INFO
	// ====================

	/** Display name of this scanner */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText ScannerName;

	/** Description of scanner capabilities */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Type of scanner system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EScannerType ScannerType;

	// ====================
	// PASSIVE SCANNING
	// ====================

	/** Maximum range for passive detection in cm (1000000 = 10km) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Passive Scanning", meta=(ClampMin=10000, ClampMax=10000000))
	float PassiveRange;

	/** How often passive scan updates in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Passive Scanning", meta=(ClampMin=0.1, ClampMax=10.0))
	float PassiveScanInterval;

	/** Minimum signature strength to detect (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Passive Scanning", meta=(ClampMin=0, ClampMax=100))
	int32 PassiveDetectionThreshold;

	/** Can detect cloaked or stealthy objects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Passive Scanning")
	bool bCanDetectStealth;

	// ====================
	// ACTIVE SCANNING
	// ====================

	/** Maximum range for active scanning in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Active Scanning", meta=(ClampMin=50000, ClampMax=50000000))
	float ActiveRange;

	/** Time required to complete an active scan in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Active Scanning", meta=(ClampMin=1.0, ClampMax=60.0))
	float ScanDuration;

	/** Cooldown between active scans in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Active Scanning", meta=(ClampMin=0.0, ClampMax=120.0))
	float ScanCooldown;

	/** Energy cost per active scan */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Active Scanning", meta=(ClampMin=0, ClampMax=1000))
	int32 EnergyCost;

	/** Can perform deep scans for detailed analysis */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Active Scanning")
	bool bCanDeepScan;

	/** Deep scan duration multiplier (e.g., 3.0 = 3x longer) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Active Scanning", meta=(ClampMin=1.0, ClampMax=10.0, EditCondition="bCanDeepScan"))
	float DeepScanDurationMultiplier;

	// ====================
	// SIGNAL IDENTIFICATION
	// ====================

	/** Accuracy of signal type identification (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Signal Identification", meta=(ClampMin=0, ClampMax=100))
	int32 SignalIdentificationAccuracy;

	/** Distance at which full details are revealed in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Signal Identification", meta=(ClampMin=10000, ClampMax=5000000))
	float DetailRevealDistance;

	/** Supported signal types (empty = all types) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Signal Identification")
	TArray<ESignalType> SupportedSignalTypes;

	// ====================
	// SENSOR RESOLUTION
	// ====================

	/** Minimum object size detectable in cm (100 = 1m) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sensor Resolution", meta=(ClampMin=1, ClampMax=100000))
	float MinimumObjectSize;

	/** Maximum number of simultaneous targets to track */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sensor Resolution", meta=(ClampMin=1, ClampMax=1000))
	int32 MaxSimultaneousTargets;

	/** Angular resolution in degrees (smaller = better precision) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sensor Resolution", meta=(ClampMin=0.1, ClampMax=45.0))
	float AngularResolution;

	// ====================
	// BLUEPRINT FUNCTIONS
	// ====================

	/**
	 * Get detail level based on distance
	 * @param Distance Distance to target in cm
	 * @return Detail level available at this distance
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Scanner")
	EScanDetailLevel GetDetailLevelAtDistance(float Distance) const;

	/**
	 * Check if this scanner can detect a signal type
	 * @param SignalType Type of signal to check
	 * @return True if scanner supports this signal type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Scanner")
	bool CanDetectSignalType(ESignalType SignalType) const;

	/**
	 * Calculate scan time for a target
	 * @param bDeepScan Whether performing a deep scan
	 * @return Time in seconds to complete scan
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Scanner")
	float GetScanTime(bool bDeepScan = false) const;

	/**
	 * Get effective range based on scan mode
	 * @param ScanMode Mode of scanning
	 * @return Range in cm
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Scanner")
	float GetEffectiveRange(EScanMode ScanMode) const;
};
