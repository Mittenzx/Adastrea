#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestingDashboardWidget.generated.h"

/**
 * Represents a single testable game system/feature
 */
USTRUCT(BlueprintType)
struct FTestableFeature
{
	GENERATED_BODY()

	/** Display name of the feature */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Testing")
	FText FeatureName;

	/** Detailed description of what to test */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Testing")
	FText Description;

	/** Whether this feature has been tested */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Testing")
	bool bIsTested;

	/** Category of the feature (e.g., "Combat", "Navigation", "UI") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Testing")
	FString Category;

	/** Optional notes from testing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Testing")
	FString TestNotes;

	FTestableFeature()
		: FeatureName(FText::FromString(TEXT("Unnamed Feature")))
		, Description(FText::FromString(TEXT("No description")))
		, bIsTested(false)
		, Category(TEXT("General"))
		, TestNotes(TEXT(""))
	{
	}
};

/**
 * Testing Dashboard Widget - Interactive testing checklist accessible during gameplay
 * 
 * This widget provides an in-game testing dashboard that allows QA and developers to:
 * - Check off game systems as they test them
 * - Configure test scenarios on-the-fly
 * - Spawn ships and change settings during gameplay
 * - Track testing progress with checkboxes
 * - Add notes for each tested feature
 * - Toggle visibility with a hotkey (e.g., F12)
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class (e.g., WBP_TestingDashboard)
 * - Add to viewport during gameplay
 * - Toggle visibility with a dedicated input action
 * - Configure testable features in Blueprint or C++
 * 
 * Integration:
 * - Can be toggled on/off during gameplay
 * - Persists across level transitions
 * - Saves testing progress
 * - Provides quick access to debug commands
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UTestingDashboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTestingDashboardWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the testing dashboard
	 * Called when the widget is first created
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Testing Dashboard")
	void InitializeDashboard();
	virtual void InitializeDashboard_Implementation();

	/**
	 * Toggle the dashboard visibility
	 * @param bVisible Whether to show or hide the dashboard
	 */
	UFUNCTION(BlueprintCallable, Category="Testing Dashboard")
	void ToggleDashboard(bool bVisible);

	/**
	 * Mark a feature as tested
	 * @param FeatureIndex Index of the feature in the testable features array
	 * @param bTested Whether the feature is tested
	 */
	UFUNCTION(BlueprintCallable, Category="Testing Dashboard")
	void SetFeatureTested(int32 FeatureIndex, bool bTested);

	/**
	 * Add notes to a tested feature
	 * @param FeatureIndex Index of the feature
	 * @param Notes Testing notes
	 */
	UFUNCTION(BlueprintCallable, Category="Testing Dashboard")
	void AddFeatureNotes(int32 FeatureIndex, const FString& Notes);

	/**
	 * Get all testable features
	 * @return Array of testable features
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Testing Dashboard")
	TArray<FTestableFeature> GetTestableFeatures() const { return TestableFeatures; }

	/**
	 * Get features by category
	 * @param Category The category to filter by
	 * @return Array of features in that category
	 */
	UFUNCTION(BlueprintCallable, Category="Testing Dashboard")
	TArray<FTestableFeature> GetFeaturesByCategory(const FString& Category) const;

	/**
	 * Get testing progress percentage
	 * @return Percentage of features tested (0-100)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Testing Dashboard")
	float GetTestingProgress() const;

	/**
	 * Reset all testing progress
	 */
	UFUNCTION(BlueprintCallable, Category="Testing Dashboard")
	void ResetAllProgress();

	/**
	 * Spawn a ship for testing
	 * @param ShipType The type of ship to spawn
	 * @param SpawnDistance Distance from player to spawn
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Testing Dashboard|Actions")
	void SpawnTestShip(const FString& ShipType, float SpawnDistance);
	virtual void SpawnTestShip_Implementation(const FString& ShipType, float SpawnDistance);

	/**
	 * Enable a debug visualization mode
	 * @param DebugMode The debug mode to enable
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Testing Dashboard|Actions")
	void EnableDebugMode(const FString& DebugMode);
	virtual void EnableDebugMode_Implementation(const FString& DebugMode);

	/**
	 * Change difficulty level on-the-fly
	 * @param DifficultyLevel The difficulty level to set
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Testing Dashboard|Actions")
	void SetDifficulty(const FString& DifficultyLevel);
	virtual void SetDifficulty_Implementation(const FString& DifficultyLevel);

	/**
	 * Execute a custom test command
	 * @param Command The command to execute
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Testing Dashboard|Actions")
	void ExecuteTestCommand(const FString& Command);
	virtual void ExecuteTestCommand_Implementation(const FString& Command);

	/**
	 * Export testing results to a file
	 * @param FilePath Path to save the results
	 */
	UFUNCTION(BlueprintCallable, Category="Testing Dashboard")
	void ExportTestResults(const FString& FilePath);

	/**
	 * Get list of available ship types for spawning
	 * @return Array of ship type names
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Testing Dashboard")
	TArray<FString> GetAvailableShipTypes() const { return AvailableShipTypes; }

	/**
	 * Get list of available debug modes
	 * @return Array of debug mode names
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Testing Dashboard")
	TArray<FString> GetAvailableDebugModes() const { return AvailableDebugModes; }

protected:
	/** Native construction */
	virtual void NativeConstruct() override;

	/** Native destruction */
	virtual void NativeDestruct() override;

	/** Native tick for updating dashboard */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Configuration Properties

	/**
	 * List of all testable features/systems
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Testing Dashboard|Configuration")
	TArray<FTestableFeature> TestableFeatures;

	/**
	 * Available ship types for spawning during tests
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Testing Dashboard|Configuration")
	TArray<FString> AvailableShipTypes;

	/**
	 * Available debug visualization modes
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Testing Dashboard|Configuration")
	TArray<FString> AvailableDebugModes;

	/**
	 * Whether the dashboard is currently visible
	 */
	UPROPERTY(BlueprintReadOnly, Category="Testing Dashboard|State")
	bool bIsDashboardVisible;

	/**
	 * Whether to pause the game when dashboard is open
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Testing Dashboard|Configuration")
	bool bPauseGameWhenOpen;

	/**
	 * Whether to show only uncompleted features
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Testing Dashboard|Configuration")
	bool bShowOnlyUncompleted;

	/**
	 * Current filter category ("" for all)
	 */
	UPROPERTY(BlueprintReadWrite, Category="Testing Dashboard|State")
	FString CurrentFilterCategory;

	/**
	 * Path to save testing results
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Testing Dashboard|Configuration")
	FString TestResultsPath;

	/**
	 * Initialize default testable features
	 * Override in Blueprint to customize the feature list
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Testing Dashboard")
	void LoadDefaultFeatures();
	virtual void LoadDefaultFeatures_Implementation();

	/**
	 * Apply current filter to feature list
	 * Called when filter settings change
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Testing Dashboard")
	void ApplyFilters();
	virtual void ApplyFilters_Implementation();

	/**
	 * Save testing progress
	 * Override to implement custom save logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Testing Dashboard")
	void SaveProgress();
	virtual void SaveProgress_Implementation();

	/**
	 * Load testing progress
	 * Override to implement custom load logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Testing Dashboard")
	void LoadProgress();
	virtual void LoadProgress_Implementation();
};
