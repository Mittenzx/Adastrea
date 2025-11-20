#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Exploration/ScannerDataAsset.h"
#include "ScannableObjectComponent.generated.h"

/**
 * Scan Data Result
 * Information revealed by a scan
 */
USTRUCT(BlueprintType)
struct FScanData
{
	GENERATED_BODY()

	/** Name of the scanned object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	FText ObjectName;

	/** Type of object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	ESignalType SignalType;

	/** Detail level of the scan */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	EScanDetailLevel DetailLevel;

	/** Mass in kg */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	float Mass;

	/** Composition description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	FText Composition;

	/** Additional details (mission relevant info, resources, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	FText AdditionalDetails;

	/** Is this object hostile? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	bool bIsHostile;

	/** Faction ID if applicable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	FString FactionID;

	FScanData()
		: SignalType(ESignalType::Unknown)
		, DetailLevel(EScanDetailLevel::None)
		, Mass(0.0f)
		, bIsHostile(false)
	{
	}
};

/**
 * Scannable Object Component
 * 
 * Makes an object detectable by scanner systems. Add this component to any actor
 * that should be discoverable through ship scanners including:
 * - Ships (friendly and hostile)
 * - Space stations
 * - Planets and asteroids
 * - Anomalies and points of interest
 * - Debris fields
 * - Beacons
 * 
 * **Detection Mechanics:**
 * - Signature Strength: How easily the object is detected (0-100)
 * - Stealth: Can be hidden from basic scanners
 * - Size: Larger objects detected at greater ranges
 * - Detail Levels: Information revealed based on scan quality
 * 
 * **Usage:**
 * 1. Add component to actor Blueprint
 * 2. Set SignalType and SignatureStrength
 * 3. Configure scan data for each detail level
 * 4. Optionally enable stealth or masking
 * 
 * **Integration:**
 * - Automatically detected by ScannerComponent
 * - Responds to active scan requests
 * - Provides appropriate data based on scan quality
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UScannableObjectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScannableObjectComponent();

	virtual void BeginPlay() override;

	// ====================
	// BASIC PROPERTIES
	// ====================

	/** Display name of this object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scannable Object")
	FText DisplayName;

	/** Type of signal this object emits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scannable Object")
	ESignalType SignalType;

	/** How strong this object's signature is (0-100, higher = easier to detect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scannable Object", meta=(ClampMin=0, ClampMax=100))
	int32 SignatureStrength;

	/** Size of the object in cm (used for detection range calculations) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scannable Object", meta=(ClampMin=1, ClampMax=10000000))
	float ObjectSize;

	/** Can this object be detected by passive scanners? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scannable Object")
	bool bPassivelyDetectable;

	/** Is this object using stealth technology? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scannable Object")
	bool bStealthEnabled;

	/** Is this object masking its signature as a different type? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scannable Object")
	bool bMaskedSignature;

	/** Apparent signal type when masked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scannable Object", meta=(EditCondition="bMaskedSignature"))
	ESignalType MaskedSignalType;

	// ====================
	// SCAN DATA
	// ====================

	/** Data revealed at basic scan level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	FScanData BasicScanData;

	/** Data revealed at detailed scan level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	FScanData DetailedScanData;

	/** Data revealed at complete scan level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scan Data")
	FScanData CompleteScanData;

	// ====================
	// SCAN STATE
	// ====================

	/** Has this object been scanned before? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scan State")
	bool bHasBeenScanned;

	/** Highest detail level achieved */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scan State")
	EScanDetailLevel HighestScanLevel;

	/** When was this object first discovered? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scan State")
	float DiscoveryTime;

	// ====================
	// BLUEPRINT FUNCTIONS
	// ====================

	/**
	 * Get scan data for the specified detail level
	 * @param DetailLevel Level of detail to retrieve
	 * @return Scan data at this detail level
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Scannable Object")
	FScanData GetScanData(EScanDetailLevel DetailLevel) const;

	/**
	 * Check if this object can be detected by a scanner
	 * @param ScannerData Scanner configuration
	 * @param Distance Distance from scanner in cm
	 * @param bIsActiveScan Is this an active scan?
	 * @return True if detectable
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Scannable Object")
	bool IsDetectable(const UScannerDataAsset* ScannerData, float Distance, bool bIsActiveScan) const;

	/**
	 * Get the signal type seen by scanners (real or masked)
	 * @param ScannerData Scanner configuration
	 * @return Signal type visible to this scanner
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Scannable Object")
	ESignalType GetVisibleSignalType(const UScannerDataAsset* ScannerData) const;

	/**
	 * Mark this object as scanned
	 * @param DetailLevel Level of detail achieved
	 */
	UFUNCTION(BlueprintCallable, Category="Scannable Object")
	void MarkAsScanned(EScanDetailLevel DetailLevel);

	/**
	 * Reset scan state (for testing or respawn)
	 */
	UFUNCTION(BlueprintCallable, Category="Scannable Object")
	void ResetScanState();

	// ====================
	// BLUEPRINT EVENTS
	// ====================

	/**
	 * Called when this object is first detected
	 * Override in Blueprint to add custom behavior
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Scannable Object")
	void OnFirstDetected();

	/**
	 * Called when this object is scanned
	 * @param DetailLevel Level of detail achieved
	 * Override in Blueprint to add custom behavior
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Scannable Object")
	void OnScanned(EScanDetailLevel DetailLevel);

protected:
	/**
	 * Initialize scan data if not set
	 */
	void InitializeScanData();
};
