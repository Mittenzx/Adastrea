#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShipStatusWidget.generated.h"

// Forward declarations
class ASpaceship;
class USpaceshipDataAsset;

/**
 * Widget for displaying comprehensive ship status and statistics
 * 
 * This widget provides a detailed view of the current spaceship including:
 * - Basic ship information (name, class, description)
 * - Core stats (hull, cargo, crew, modular points)
 * - Combat stats (armor, shields, weapons, point defense)
 * - Mobility stats (speed, acceleration, maneuverability, fuel)
 * - Utility stats (sensors, stealth, repair, science, medical)
 * - Operational stats (power, life support, maintenance, hangars, drones, AI)
 * - Advanced stats (diplomacy, boarding, customization, EW, mining)
 * - Lore information (manufacturer, year, rarity)
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class
 * - Design the ship status screen layout
 * - Call InitializeShipStatus() to populate with current ship data
 * - Use RefreshShipStatus() to update displayed values
 * - Toggle visibility with Show/Hide or through PlayerController
 * 
 * Integration:
 * - Automatically gets ship data from controlled spaceship
 * - Updates display values from SpaceshipDataAsset
 * - Managed by AdastreaPlayerController for show/hide
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UShipStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UShipStatusWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the ship status widget with current spaceship data
	 * Automatically retrieves the player's controlled spaceship
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status")
	void InitializeShipStatus();
	virtual void InitializeShipStatus_Implementation();

	/**
	 * Initialize the ship status widget with a specific spaceship
	 * @param InSpaceship The spaceship to display data for
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status")
	void InitializeWithSpaceship(ASpaceship* InSpaceship);
	virtual void InitializeWithSpaceship_Implementation(ASpaceship* InSpaceship);

	/**
	 * Refresh all displayed ship data
	 * Call this to update the display with current values
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status")
	void RefreshShipStatus();
	virtual void RefreshShipStatus_Implementation();

	/**
	 * Update basic ship information display
	 * @param ShipName The ship's display name
	 * @param ShipClass The ship's class/type
	 * @param Description The ship's description
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Basic Info")
	void UpdateBasicInfo(const FText& ShipName, const FText& ShipClass, const FText& Description);
	virtual void UpdateBasicInfo_Implementation(const FText& ShipName, const FText& ShipClass, const FText& Description);

	/**
	 * Update core stats display
	 * @param CurrentHull Current hull integrity
	 * @param MaxHull Maximum hull strength
	 * @param CargoCapacity Total cargo capacity
	 * @param CrewRequired Minimum crew required
	 * @param MaxCrew Maximum crew capacity
	 * @param ModularPoints Available modular points
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Core Stats")
	void UpdateCoreStats(float CurrentHull, float MaxHull, float CargoCapacity, int32 CrewRequired, int32 MaxCrew, int32 ModularPoints);
	virtual void UpdateCoreStats_Implementation(float CurrentHull, float MaxHull, float CargoCapacity, int32 CrewRequired, int32 MaxCrew, int32 ModularPoints);

	/**
	 * Update combat stats display
	 * @param ArmorRating Armor rating
	 * @param ShieldStrength Shield strength
	 * @param ShieldRechargeRate Shield recharge rate
	 * @param WeaponSlots Number of weapon slots
	 * @param WeaponPower Weapon power capacity
	 * @param PointDefense Point defense rating
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Combat Stats")
	void UpdateCombatStats(float ArmorRating, float ShieldStrength, float ShieldRechargeRate, int32 WeaponSlots, float WeaponPower, float PointDefense);
	virtual void UpdateCombatStats_Implementation(float ArmorRating, float ShieldStrength, float ShieldRechargeRate, int32 WeaponSlots, float WeaponPower, float PointDefense);

	/**
	 * Update mobility stats display
	 * @param MaxSpeed Maximum speed
	 * @param Acceleration Acceleration rate
	 * @param Maneuverability Maneuverability rating
	 * @param JumpRange Jump drive range
	 * @param FuelCapacity Fuel capacity
	 * @param FuelConsumption Fuel consumption rate
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Mobility Stats")
	void UpdateMobilityStats(float MaxSpeed, float Acceleration, int32 Maneuverability, float JumpRange, float FuelCapacity, float FuelConsumption);
	virtual void UpdateMobilityStats_Implementation(float MaxSpeed, float Acceleration, int32 Maneuverability, float JumpRange, float FuelCapacity, float FuelConsumption);

	/**
	 * Update utility stats display
	 * @param SensorRange Sensor range
	 * @param SensorResolution Sensor resolution rating
	 * @param StealthRating Stealth rating
	 * @param RepairRating Repair system rating
	 * @param ScienceRating Science rating
	 * @param MedicalRating Medical rating
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Utility Stats")
	void UpdateUtilityStats(float SensorRange, int32 SensorResolution, int32 StealthRating, int32 RepairRating, int32 ScienceRating, int32 MedicalRating);
	virtual void UpdateUtilityStats_Implementation(float SensorRange, int32 SensorResolution, int32 StealthRating, int32 RepairRating, int32 ScienceRating, int32 MedicalRating);

	/**
	 * Update operational stats display
	 * @param PowerCapacity Power capacity
	 * @param LifeSupportRating Life support rating
	 * @param MaintenanceLevel Maintenance level
	 * @param HangarCapacity Hangar capacity
	 * @param DroneCapacity Drone capacity
	 * @param AIRating AI system rating
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Operational Stats")
	void UpdateOperationalStats(float PowerCapacity, int32 LifeSupportRating, int32 MaintenanceLevel, int32 HangarCapacity, int32 DroneCapacity, int32 AIRating);
	virtual void UpdateOperationalStats_Implementation(float PowerCapacity, int32 LifeSupportRating, int32 MaintenanceLevel, int32 HangarCapacity, int32 DroneCapacity, int32 AIRating);

	/**
	 * Update advanced stats display
	 * @param DiplomacyRating Diplomacy rating
	 * @param BoardingCapability Boarding capability
	 * @param CustomizableSlots Customizable slots
	 * @param EWarRating Electronic warfare rating
	 * @param MiningRating Mining rating
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Advanced Stats")
	void UpdateAdvancedStats(int32 DiplomacyRating, int32 BoardingCapability, int32 CustomizableSlots, int32 EWarRating, int32 MiningRating);
	virtual void UpdateAdvancedStats_Implementation(int32 DiplomacyRating, int32 BoardingCapability, int32 CustomizableSlots, int32 EWarRating, int32 MiningRating);

	/**
	 * Update lore information display
	 * @param Manufacturer Ship manufacturer
	 * @param YearBuilt Year built
	 * @param RarityTier Rarity tier
	 * @param LoreNotes Lore notes
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Lore")
	void UpdateLoreInfo(const FText& Manufacturer, int32 YearBuilt, const FText& RarityTier, const FText& LoreNotes);
	virtual void UpdateLoreInfo_Implementation(const FText& Manufacturer, int32 YearBuilt, const FText& RarityTier, const FText& LoreNotes);

	/**
	 * Update ship rating displays (calculated values)
	 * @param CombatRating Combat effectiveness rating (0-100)
	 * @param MobilityRating Mobility rating (0-100)
	 * @param UtilityRating Utility rating (0-100)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Status|Ratings")
	void UpdateRatings(float CombatRating, float MobilityRating, float UtilityRating);
	virtual void UpdateRatings_Implementation(float CombatRating, float MobilityRating, float UtilityRating);

	/**
	 * Get the currently displayed spaceship
	 * @return The spaceship being displayed, or nullptr if none
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Status")
	ASpaceship* GetDisplayedSpaceship() const { return CurrentSpaceship; }

	/**
	 * Get the ship data asset being displayed
	 * @return The ship data asset, or nullptr if none
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Status")
	USpaceshipDataAsset* GetShipDataAsset() const { return ShipDataAsset; }

protected:
	/** Native construction */
	virtual void NativeConstruct() override;

	/** Reference to the spaceship being displayed */
	UPROPERTY(BlueprintReadOnly, Category="Ship Status")
	ASpaceship* CurrentSpaceship;

	/** Cached reference to the ship's data asset */
	UPROPERTY(BlueprintReadOnly, Category="Ship Status")
	USpaceshipDataAsset* ShipDataAsset;

	// Cached display values for Blueprint access
	UPROPERTY(BlueprintReadOnly, Category="Ship Status|Display")
	FText DisplayShipName;

	UPROPERTY(BlueprintReadOnly, Category="Ship Status|Display")
	FText DisplayShipClass;

	UPROPERTY(BlueprintReadOnly, Category="Ship Status|Display")
	FText DisplayDescription;

	UPROPERTY(BlueprintReadOnly, Category="Ship Status|Display")
	float DisplayCombatRating;

	UPROPERTY(BlueprintReadOnly, Category="Ship Status|Display")
	float DisplayMobilityRating;

	UPROPERTY(BlueprintReadOnly, Category="Ship Status|Display")
	float DisplayUtilityRating;

	/**
	 * Get the player's controlled spaceship
	 * @return The controlled spaceship, or nullptr if not controlling a ship
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Status")
	ASpaceship* GetPlayerSpaceship() const;
};
