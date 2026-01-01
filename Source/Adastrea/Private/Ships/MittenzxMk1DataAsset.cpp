#include "Ships/MittenzxMk1DataAsset.h"
#include "AdastreaLog.h"

UMittenzxMk1DataAsset::UMittenzxMk1DataAsset()
{
	// Set example values for basic info - representing a balanced multi-role ship
	ShipName = FText::FromString(TEXT("Mittenzx Mk1"));
	Description = FText::FromString(TEXT("A versatile multi-role spacecraft designed for balanced performance. Ideal for pilots who need a reliable all-rounder for exploration, light combat, and trade missions."));
	ShipClass = FText::FromString(TEXT("Multi-Role"));
	ShipID = FName(TEXT("MittenzxMk1"));

	// Core stats - balanced and reliable
	HullStrength = 1000.0f;
	CargoCapacity = 75.0f;
	CrewRequired = 2;
	MaxCrew = 4;
	ModularPoints = 10;

	// Combat stats - decent for self-defense and light combat
	ArmorRating = 35.0f;
	ShieldStrength = 500.0f;
	ShieldRechargeRate = 18.0f;
	WeaponSlots = 2;
	WeaponPowerCapacity = 650.0f;
	PointDefenseRating = 4.0f;

	// Mobility stats - good balance of speed and maneuverability
	MaxSpeed = 750.0f;
	Acceleration = 110.0f;
	Maneuverability = 7;
	JumpRange = 22.0f;
	FuelCapacity = 1800.0f;
	FuelConsumptionRate = 55.0f;

	// Utility stats - balanced capabilities
	SensorRange = 12000.0f;
	SensorResolution = 7;
	StealthRating = 5;
	RepairSystemRating = 5;
	ScienceRating = 5;
	MedicalRating = 4;

	// Operational stats - efficient and reliable
	PowerCapacity = 4500.0f;
	LifeSupportRating = 7;
	MaintenanceLevel = 4;
	HangarCapacity = 1;
	DroneCapacity = 12;
	AISystemRating = 6;

	// Advanced stats - versatile capabilities
	DiplomacyRating = 3;
	BoardingCapability = 2;
	CustomizableSlots = 6;
	EWarRating = 5;
	MiningRating = 3;

	// Lore - a solid choice for experienced pilots
	Manufacturer = FText::FromString(TEXT("Mittenzx Industries"));
	YearBuilt = 2550;
	RarityTier = FText::FromString(TEXT("Uncommon"));
	LoreNotes = FText::FromString(TEXT("The Mittenzx Mk1 represents the next evolution in versatile spacecraft design. Built by the renowned Mittenzx Industries, this ship combines reliable performance across multiple roles without sacrificing capability. Popular among independent pilots and small trade fleets alike, the Mk1 has earned a reputation for dependability in diverse situations."));
	PrimaryColor = FLinearColor(0.6f, 0.2f, 0.8f, 1.0f);  // Purple
	SecondaryColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);  // Dark gray
}
