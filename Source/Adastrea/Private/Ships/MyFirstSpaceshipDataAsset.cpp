#include "MyFirstSpaceshipDataAsset.h"
#include "AdastreaLog.h"

UMyFirstSpaceshipDataAsset::UMyFirstSpaceshipDataAsset()
{
	// Set example values for basic info - representing a light scout ship
	ShipName = FText::FromString(TEXT("Pathfinder MK-I"));
	Description = FText::FromString(TEXT("A nimble exploration vessel designed for solo pilots. Perfect for surveying new systems and avoiding trouble."));
	ShipClass = FText::FromString(TEXT("Scout"));
	ShipID = FName(TEXT("PathfinderMK1"));

	// Core stats - lightweight and efficient
	HullStrength = 750.0f;
	CargoCapacity = 50.0f;
	CrewRequired = 1;
	MaxCrew = 3;
	ModularPoints = 8;

	// Combat stats - minimal but present for self-defense
	ArmorRating = 25.0f;
	ShieldStrength = 400.0f;
	ShieldRechargeRate = 15.0f;
	WeaponSlots = 1;
	WeaponPowerCapacity = 500.0f;
	PointDefenseRating = 3.0f;

	// Mobility stats - excellent for a scout ship
	MaxSpeed = 850.0f;
	Acceleration = 120.0f;
	Maneuverability = 8;
	JumpRange = 25.0f;
	FuelCapacity = 1500.0f;
	FuelConsumptionRate = 50.0f;

	// Utility stats - high sensor capabilities for exploration
	SensorRange = 15000.0f;
	SensorResolution = 8;
	StealthRating = 7;
	RepairSystemRating = 4;
	ScienceRating = 6;
	MedicalRating = 2;

	// Operational stats - efficient systems
	PowerCapacity = 4000.0f;
	LifeSupportRating = 6;
	MaintenanceLevel = 3;
	HangarCapacity = 0;
	DroneCapacity = 10;
	AISystemRating = 7;

	// Advanced stats - specialized for reconnaissance
	DiplomacyRating = 2;
	BoardingCapability = 1;
	CustomizableSlots = 5;
	EWarRating = 4;
	MiningRating = 2;

	// Lore - an appealing starter ship
	Manufacturer = FText::FromString(TEXT("Horizon Explorers Inc."));
	YearBuilt = 2547;
	RarityTier = FText::FromString(TEXT("Common"));
	LoreNotes = FText::FromString(TEXT("The Pathfinder MK-I is the most popular choice for new explorers. Its reliable systems and exceptional sensor suite make it ideal for solo reconnaissance missions. While lightly armed, its speed and maneuverability allow pilots to escape dangerous situations."));
	PrimaryColor = FLinearColor(0.2f, 0.5f, 0.8f, 1.0f);  // Blue
	SecondaryColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);  // Light gray
}
