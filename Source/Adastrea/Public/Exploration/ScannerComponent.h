#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Exploration/ScannerDataAsset.h"
#include "Exploration/ScannableObjectComponent.h"
#include "ScannerComponent.generated.h"

/**
 * Detected Object Info
 * Information about a detected object
 */
USTRUCT(BlueprintType)
struct FDetectedObject
{
	GENERATED_BODY()

	/** The detected actor */
	UPROPERTY(BlueprintReadWrite, Category="Detection")
	AActor* DetectedActor;

	/** Scannable component */
	UPROPERTY(BlueprintReadWrite, Category="Detection")
	UScannableObjectComponent* ScannableComponent;

	/** Distance to object in cm */
	UPROPERTY(BlueprintReadWrite, Category="Detection")
	float Distance;

	/** Direction to object (normalized) */
	UPROPERTY(BlueprintReadWrite, Category="Detection")
	FVector Direction;

	/** Current detail level */
	UPROPERTY(BlueprintReadWrite, Category="Detection")
	EScanDetailLevel DetailLevel;

	/** Time when detected */
	UPROPERTY(BlueprintReadWrite, Category="Detection")
	float DetectionTime;

	FDetectedObject()
		: DetectedActor(nullptr)
		, ScannableComponent(nullptr)
		, Distance(0.0f)
		, Direction(FVector::ZeroVector)
		, DetailLevel(EScanDetailLevel::None)
		, DetectionTime(0.0f)
	{
	}
};

/**
 * Active Scan Info
 * Information about an in-progress active scan
 */
USTRUCT(BlueprintType)
struct FActiveScanInfo
{
	GENERATED_BODY()

	/** Target being scanned */
	UPROPERTY(BlueprintReadWrite, Category="Active Scan")
	AActor* TargetActor;

	/** Scan mode */
	UPROPERTY(BlueprintReadWrite, Category="Active Scan")
	EScanMode ScanMode;

	/** Start time */
	UPROPERTY(BlueprintReadWrite, Category="Active Scan")
	float StartTime;

	/** Duration */
	UPROPERTY(BlueprintReadWrite, Category="Active Scan")
	float Duration;

	/** Progress (0-1) */
	UPROPERTY(BlueprintReadWrite, Category="Active Scan")
	float Progress;

	FActiveScanInfo()
		: TargetActor(nullptr)
		, ScanMode(EScanMode::Active)
		, StartTime(0.0f)
		, Duration(0.0f)
		, Progress(0.0f)
	{
	}
};

/**
 * Scanner Component
 * 
 * Ship-mounted scanner system providing:
 * - **Passive Scanning**: Continuous detection of nearby objects
 * - **Active Scanning**: Targeted analysis of specific objects
 * - **Deep Scanning**: Detailed analysis revealing all information
 * - **Signal Tracking**: Monitor multiple targets simultaneously
 * 
 * **Passive Scanning:**
 * - Runs continuously in background
 * - Limited range and detail
 * - No energy cost
 * - Updates at configured interval
 * 
 * **Active Scanning:**
 * - Targeted scan of specific object
 * - Longer range and more detail
 * - Energy cost and cooldown
 * - Takes time to complete
 * 
 * **Deep Scanning:**
 * - Reveals complete information
 * - Requires scanner capability
 * - Longer scan time
 * - Used for planets, anomalies
 * 
 * **Usage:**
 * 1. Add component to spaceship Blueprint
 * 2. Assign ScannerDataAsset
 * 3. Enable passive scanning
 * 4. Use Blueprint functions for active scans
 * 
 * **Integration:**
 * - Automatically detects ScannableObjectComponent
 * - Fires events for discoveries
 * - Respects scanner capabilities
 * - LOD-aware (disables at distance)
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UScannerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScannerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================
	// CONFIGURATION
	// ====================

	/** Scanner data asset defining capabilities */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scanner")
	UScannerDataAsset* ScannerData;

	/** Is passive scanning enabled? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scanner")
	bool bPassiveScanningEnabled;

	/** Should we show detected objects in log? (debug) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scanner|Debug")
	bool bDebugShowDetections;

	// ====================
	// PASSIVE SCANNING
	// ====================

	/** Currently detected objects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Passive Scanning")
	TArray<FDetectedObject> DetectedObjects;

	/** Time since last passive scan */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Passive Scanning")
	float TimeSinceLastPassiveScan;

	/**
	 * Get all detected objects of a specific type
	 * @param SignalType Type of signal to filter
	 * @return Array of detected objects
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Passive Scanning")
	TArray<FDetectedObject> GetDetectedObjectsByType(ESignalType SignalType) const;

	/**
	 * Get nearest detected object
	 * @return Nearest detected object, or invalid if none
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Passive Scanning")
	FDetectedObject GetNearestDetectedObject() const;

	/**
	 * Check if an object is currently detected
	 * @param Actor Actor to check
	 * @return True if detected
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Passive Scanning")
	bool IsObjectDetected(AActor* Actor) const;

	// ====================
	// ACTIVE SCANNING
	// ====================

	/** Current active scan info */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Active Scanning")
	FActiveScanInfo CurrentScan;

	/** Is an active scan in progress? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Active Scanning")
	bool bScanInProgress;

	/** Time until next scan can be initiated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Active Scanning")
	float ScanCooldownRemaining;

	/**
	 * Start an active scan of a target
	 * @param Target Actor to scan
	 * @param ScanMode Mode of scan (Active or DeepScan)
	 * @return True if scan started successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Active Scanning")
	bool StartActiveScan(AActor* Target, EScanMode ScanMode = EScanMode::Active);

	/**
	 * Cancel the current active scan
	 */
	UFUNCTION(BlueprintCallable, Category="Active Scanning")
	void CancelActiveScan();

	/**
	 * Check if can start an active scan
	 * @param Target Target to scan
	 * @param OutReason Reason why scan cannot start (if false)
	 * @return True if can start scan
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Active Scanning")
	bool CanStartActiveScan(AActor* Target, FText& OutReason) const;

	/**
	 * Get scan progress (0-1)
	 * @return Progress of current scan, 0 if no scan in progress
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Active Scanning")
	float GetScanProgress() const;

	// ====================
	// SIGNAL IDENTIFICATION
	// ====================

	/**
	 * Identify signal type of a detected object
	 * @param DetectedObject Object to identify
	 * @return Identified signal type (may be masked)
	 */
	UFUNCTION(BlueprintCallable, Category="Signal Identification")
	ESignalType IdentifySignal(const FDetectedObject& DetectedObject) const;

	/**
	 * Get scan data for a detected object
	 * @param DetectedObject Object to get data for
	 * @return Scan data at current detail level
	 */
	UFUNCTION(BlueprintCallable, Category="Signal Identification")
	FScanData GetScanDataForObject(const FDetectedObject& DetectedObject) const;

	// ====================
	// BLUEPRINT EVENTS
	// ====================

	/**
	 * Called when a new object is detected
	 * @param DetectedObject Newly detected object
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Scanner")
	void OnObjectDetected(const FDetectedObject& DetectedObject);

	/**
	 * Called when an object is lost (out of range)
	 * @param LostObject Object that was lost
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Scanner")
	void OnObjectLost(const FDetectedObject& LostObject);

	/**
	 * Called when active scan completes
	 * @param ScannedObject Object that was scanned
	 * @param ScanData Data revealed by scan
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Scanner")
	void OnScanComplete(const FDetectedObject& ScannedObject, const FScanData& ScanData);

	/**
	 * Called when scan fails or is interrupted
	 * @param Reason Why scan failed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Scanner")
	void OnScanFailed(const FText& Reason);

protected:
	/**
	 * Perform passive scan update
	 */
	void UpdatePassiveScanning(float DeltaTime);

	/**
	 * Update active scan progress
	 */
	void UpdateActiveScan(float DeltaTime);

	/**
	 * Find all scannable objects in range
	 */
	TArray<UScannableObjectComponent*> FindScannableObjectsInRange(float Range);

	/**
	 * Complete the current active scan
	 */
	void CompleteActiveScan();

	/**
	 * Remove object from detected list
	 */
	void RemoveDetectedObject(AActor* Actor);
};
