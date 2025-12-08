#include "UI/ShipStatusWidget.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Player/AdastreaPlayerController.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"

UShipStatusWidget::UShipStatusWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentSpaceship = nullptr;
	ShipDataAsset = nullptr;
	DisplayCombatRating = 0.0f;
	DisplayMobilityRating = 0.0f;
	DisplayUtilityRating = 0.0f;
}

void UShipStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Auto-initialize with player's spaceship if not already set
	if (!CurrentSpaceship)
	{
		InitializeShipStatus();
	}
}

void UShipStatusWidget::InitializeShipStatus_Implementation()
{
	// Get the player's controlled spaceship
	ASpaceship* PlayerShip = GetPlayerSpaceship();
	
	if (PlayerShip)
	{
		InitializeWithSpaceship(PlayerShip);
	}
	else
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: No spaceship available to display"));
	}
}

void UShipStatusWidget::InitializeWithSpaceship_Implementation(ASpaceship* InSpaceship)
{
	if (!InSpaceship)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: Cannot initialize with null spaceship"));
		return;
	}

	CurrentSpaceship = InSpaceship;
	ShipDataAsset = InSpaceship->ShipDataAsset;

	if (!ShipDataAsset)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: Spaceship has no data asset assigned"));
		return;
	}

	// Refresh all displayed data
	RefreshShipStatus();
	
	UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Initialized with spaceship %s"), *InSpaceship->GetName());
}

void UShipStatusWidget::RefreshShipStatus_Implementation()
{
	if (!CurrentSpaceship || !ShipDataAsset)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: Cannot refresh - no spaceship or data asset"));
		return;
	}

	// Update basic info
	UpdateBasicInfo(
		ShipDataAsset->ShipName,
		ShipDataAsset->ShipClass,
		ShipDataAsset->Description
	);

	// Update core stats
	UpdateCoreStats(
		CurrentSpaceship->GetCurrentHullIntegrity(),
		CurrentSpaceship->GetMaxHullIntegrity(),
		ShipDataAsset->CargoCapacity,
		ShipDataAsset->CrewRequired,
		ShipDataAsset->MaxCrew,
		ShipDataAsset->ModularPoints
	);

	// Update combat stats
	UpdateCombatStats(
		ShipDataAsset->ArmorRating,
		ShipDataAsset->ShieldStrength,
		ShipDataAsset->ShieldRechargeRate,
		ShipDataAsset->WeaponSlots,
		ShipDataAsset->WeaponPowerCapacity,
		ShipDataAsset->PointDefenseRating
	);

	// Update mobility stats
	UpdateMobilityStats(
		ShipDataAsset->MaxSpeed,
		ShipDataAsset->Acceleration,
		ShipDataAsset->Maneuverability,
		ShipDataAsset->JumpRange,
		ShipDataAsset->FuelCapacity,
		ShipDataAsset->FuelConsumptionRate
	);

	// Update utility stats
	UpdateUtilityStats(
		ShipDataAsset->SensorRange,
		ShipDataAsset->SensorResolution,
		ShipDataAsset->StealthRating,
		ShipDataAsset->RepairSystemRating,
		ShipDataAsset->ScienceRating,
		ShipDataAsset->MedicalRating
	);

	// Update operational stats
	UpdateOperationalStats(
		ShipDataAsset->PowerCapacity,
		ShipDataAsset->LifeSupportRating,
		ShipDataAsset->MaintenanceLevel,
		ShipDataAsset->HangarCapacity,
		ShipDataAsset->DroneCapacity,
		ShipDataAsset->AISystemRating
	);

	// Update advanced stats
	UpdateAdvancedStats(
		ShipDataAsset->DiplomacyRating,
		ShipDataAsset->BoardingCapability,
		ShipDataAsset->CustomizableSlots,
		ShipDataAsset->EWarRating,
		ShipDataAsset->MiningRating
	);

	// Update lore info
	UpdateLoreInfo(
		ShipDataAsset->Manufacturer,
		ShipDataAsset->YearBuilt,
		ShipDataAsset->RarityTier,
		ShipDataAsset->LoreNotes
	);

	// Calculate and update ratings
	const float CombatRating = ShipDataAsset->GetCombatRating();
	const float MobilityRating = ShipDataAsset->GetMobilityRating();
	const float UtilityRating = ShipDataAsset->GetUtilityRating();
	
	UpdateRatings(CombatRating, MobilityRating, UtilityRating);

	UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Refreshed ship status for %s"), *ShipDataAsset->ShipName.ToString());
}

void UShipStatusWidget::UpdateBasicInfo_Implementation(const FText& ShipName, const FText& ShipClass, const FText& Description)
{
	// Store for Blueprint access
	DisplayShipName = ShipName;
	DisplayShipClass = ShipClass;
	DisplayDescription = Description;

	// Blueprint can override to update UI elements
}

void UShipStatusWidget::UpdateCoreStats_Implementation(float CurrentHull, float MaxHull, float CargoCapacity, int32 CrewRequired, int32 MaxCrew, int32 ModularPoints)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateCombatStats_Implementation(float ArmorRating, float ShieldStrength, float ShieldRechargeRate, int32 WeaponSlots, float WeaponPower, float PointDefense)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateMobilityStats_Implementation(float MaxSpeed, float Acceleration, int32 Maneuverability, float JumpRange, float FuelCapacity, float FuelConsumption)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateUtilityStats_Implementation(float SensorRange, int32 SensorResolution, int32 StealthRating, int32 RepairRating, int32 ScienceRating, int32 MedicalRating)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateOperationalStats_Implementation(float PowerCapacity, int32 LifeSupportRating, int32 MaintenanceLevel, int32 HangarCapacity, int32 DroneCapacity, int32 AIRating)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateAdvancedStats_Implementation(int32 DiplomacyRating, int32 BoardingCapability, int32 CustomizableSlots, int32 EWarRating, int32 MiningRating)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateLoreInfo_Implementation(const FText& Manufacturer, int32 YearBuilt, const FText& RarityTier, const FText& LoreNotes)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateRatings_Implementation(float CombatRating, float MobilityRating, float UtilityRating)
{
	// Store for Blueprint access
	DisplayCombatRating = CombatRating;
	DisplayMobilityRating = MobilityRating;
	DisplayUtilityRating = UtilityRating;

	// Blueprint can override to update UI elements
}

ASpaceship* UShipStatusWidget::GetPlayerSpaceship() const
{
	// Get the world context
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Get the player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return nullptr;
	}

	// Try to cast to AdastreaPlayerController for GetControlledSpaceship
	AAdastreaPlayerController* AdastreaPC = Cast<AAdastreaPlayerController>(PC);
	if (AdastreaPC)
	{
		return AdastreaPC->GetControlledSpaceship();
	}

	// Fallback: Try to cast pawn directly
	return Cast<ASpaceship>(PC->GetPawn());
}
