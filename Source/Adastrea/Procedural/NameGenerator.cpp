// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "NameGenerator.h"
#include "SpaceSectorMap.h"
#include "Kismet/GameplayStatics.h"

// Name component getter functions to avoid static initialization order issues
TArray<FString> UNameGenerator::GetMilitarySectorPrefixes()
{
	return {
		TEXT("Tactical"), TEXT("Strategic"), TEXT("Defense"), TEXT("Guardian"), TEXT("Sentinel"),
		TEXT("Fortress"), TEXT("Bastion"), TEXT("Aegis"), TEXT("Bulwark"), TEXT("Vanguard"),
		TEXT("Patrol"), TEXT("Watch"), TEXT("Shield"), TEXT("Outpost"), TEXT("Command")
	};
}

TArray<FString> UNameGenerator::GetMilitarySectorSuffixes()
{
	return {
		TEXT("Zone"), TEXT("Sector"), TEXT("Perimeter"), TEXT("Point"), TEXT("Station"),
		TEXT("Line"), TEXT("Grid"), TEXT("Quadrant"), TEXT("Region"), TEXT("Territory")
	};
}

TArray<FString> UNameGenerator::GetMiningSectorPrefixes()
{
	return {
		TEXT("Mining"), TEXT("Extraction"), TEXT("Ore"), TEXT("Crystal"), TEXT("Resource"),
		TEXT("Industrial"), TEXT("Refinery"), TEXT("Processing"), TEXT("Harvest"), TEXT("Dig"),
		TEXT("Quarry"), TEXT("Mineral"), TEXT("Asteroid"), TEXT("Rich"), TEXT("Prospector")
	};
}

TArray<FString> UNameGenerator::GetMiningSectorSuffixes()
{
	return {
		TEXT("Field"), TEXT("Belt"), TEXT("Reach"), TEXT("Zone"), TEXT("Expanse"),
		TEXT("Region"), TEXT("Territory"), TEXT("Deposit"), TEXT("Vein"), TEXT("Cluster")
	};
}

TArray<FString> UNameGenerator::GetTradeSectorPrefixes()
{
	return {
		TEXT("Commerce"), TEXT("Trade"), TEXT("Market"), TEXT("Exchange"), TEXT("Merchant"),
		TEXT("Trading"), TEXT("Business"), TEXT("Economic"), TEXT("Port"), TEXT("Hub"),
		TEXT("Bazaar"), TEXT("Emporium"), TEXT("Plaza"), TEXT("Gateway"), TEXT("Transit")
	};
}

TArray<FString> UNameGenerator::GetTradeSectorSuffixes()
{
	return {
		TEXT("Hub"), TEXT("Center"), TEXT("Point"), TEXT("Junction"), TEXT("Nexus"),
		TEXT("Route"), TEXT("Way"), TEXT("Crossing"), TEXT("Station"), TEXT("Terminal")
	};
}

TArray<FString> UNameGenerator::GetScientificSectorPrefixes()
{
	return {
		TEXT("Research"), TEXT("Science"), TEXT("Discovery"), TEXT("Laboratory"), TEXT("Study"),
		TEXT("Analysis"), TEXT("Observation"), TEXT("Survey"), TEXT("Experimental"), TEXT("Academic"),
		TEXT("Investigation"), TEXT("Exploration"), TEXT("Pioneer"), TEXT("Quantum"), TEXT("Stellar")
	};
}

TArray<FString> UNameGenerator::GetScientificSectorSuffixes()
{
	return {
		TEXT("Station"), TEXT("Complex"), TEXT("Facility"), TEXT("Lab"), TEXT("Institute"),
		TEXT("Center"), TEXT("Site"), TEXT("Zone"), TEXT("Region"), TEXT("Sector")
	};
}

TArray<FString> UNameGenerator::GetFrontierSectorPrefixes()
{
	return {
		TEXT("Frontier"), TEXT("Colonial"), TEXT("New"), TEXT("Pioneer"), TEXT("Settler"),
		TEXT("Outpost"), TEXT("Remote"), TEXT("Edge"), TEXT("Border"), TEXT("Wild"),
		TEXT("Uncharted"), TEXT("Distant"), TEXT("Far"), TEXT("Outer"), TEXT("Rim")
	};
}

TArray<FString> UNameGenerator::GetFrontierSectorSuffixes()
{
	return {
		TEXT("Territory"), TEXT("Region"), TEXT("Zone"), TEXT("Expanse"), TEXT("Reach"),
		TEXT("Space"), TEXT("Frontier"), TEXT("Edge"), TEXT("Bounds"), TEXT("Limit")
	};
}

TArray<FString> UNameGenerator::GetGreekLetters()
{
	return {
		TEXT("Alpha"), TEXT("Beta"), TEXT("Gamma"), TEXT("Delta"), TEXT("Epsilon"),
		TEXT("Zeta"), TEXT("Eta"), TEXT("Theta"), TEXT("Iota"), TEXT("Kappa"),
		TEXT("Lambda"), TEXT("Mu"), TEXT("Nu"), TEXT("Xi"), TEXT("Omicron"),
		TEXT("Pi"), TEXT("Rho"), TEXT("Sigma"), TEXT("Tau"), TEXT("Upsilon"),
		TEXT("Phi"), TEXT("Chi"), TEXT("Psi"), TEXT("Omega")
	};
}

TArray<FString> UNameGenerator::GetStationPrefixes()
{
	return {
		TEXT("Station"), TEXT("Outpost"), TEXT("Hub"), TEXT("Port"), TEXT("Base"),
		TEXT("Depot"), TEXT("Terminal"), TEXT("Platform"), TEXT("Facility"), TEXT("Complex")
	};
}

TArray<FString> UNameGenerator::GetStationSuffixes()
{
	return {
		TEXT("Prime"), TEXT("Central"), TEXT("One"), TEXT("Alpha"), TEXT("Major"),
		TEXT("North"), TEXT("South"), TEXT("East"), TEXT("West"), TEXT("Haven")
	};
}

TArray<FString> UNameGenerator::GetShipPrefixes()
{
	return {
		TEXT("HSS"), TEXT("ISS"), TEXT("USS"), TEXT("MSS"), TEXT("CSS"),
		TEXT("FSS"), TEXT("TSS"), TEXT("ESS"), TEXT("DSS"), TEXT("ASS")
	};
}

TArray<FString> UNameGenerator::GetShipSuffixes()
{
	return {
		TEXT("Endeavor"), TEXT("Discovery"), TEXT("Pathfinder"), TEXT("Explorer"), TEXT("Voyager"),
		TEXT("Pioneer"), TEXT("Wanderer"), TEXT("Seeker"), TEXT("Navigator"), TEXT("Ranger")
	};
}

FRandomStream UNameGenerator::InitializeRandomStream(int32 Seed)
{
	if (Seed == 0)
	{
		// Use time-based seed if not specified
		Seed = FDateTime::Now().GetTicks();
	}
	return FRandomStream(Seed);
}

FString UNameGenerator::GenerateSectorName(uint8 Theme, int32 Seed)
{
	FRandomStream RandomStream = InitializeRandomStream(Seed);

	switch (Theme)
	{
	case 0: // Military
		return GenerateMilitarySectorName(RandomStream);
	case 1: // Mining
		return GenerateMiningSectorName(RandomStream);
	case 2: // Trade
		return GenerateTradeSectorName(RandomStream);
	case 3: // Scientific
		return GenerateScientificSectorName(RandomStream);
	case 4: // Frontier
		return GenerateFrontierSectorName(RandomStream);
	case 5: // Greek
		return GenerateGreekSectorName(RandomStream);
	case 6: // Numeric
		return GenerateNumericSectorName(RandomStream);
	case 7: // Descriptive
		return GenerateDescriptiveSectorName(RandomStream);
	default:
		return GenerateGreekSectorName(RandomStream);
	}
}

FString UNameGenerator::GenerateMilitarySectorName(FRandomStream& RandomStream)
{
	TArray<FString> Prefixes = GetMilitarySectorPrefixes();
	TArray<FString> Suffixes = GetMilitarySectorSuffixes();
	const FString& Prefix = Prefixes[RandomStream.RandRange(0, Prefixes.Num() - 1)];
	const FString& Suffix = Suffixes[RandomStream.RandRange(0, Suffixes.Num() - 1)];
	int32 Number = RandomStream.RandRange(1, 99);
	return FString::Printf(TEXT("%s %s %d"), *Prefix, *Suffix, Number);
}

FString UNameGenerator::GenerateMiningSectorName(FRandomStream& RandomStream)
{
	TArray<FString> Prefixes = GetMiningSectorPrefixes();
	TArray<FString> Suffixes = GetMiningSectorSuffixes();
	const FString& Prefix = Prefixes[RandomStream.RandRange(0, Prefixes.Num() - 1)];
	const FString& Suffix = Suffixes[RandomStream.RandRange(0, Suffixes.Num() - 1)];
	
	// 50% chance to include a letter designation
	if (RandomStream.FRand() > 0.5f)
	{
		TCHAR Letter = 'A' + RandomStream.RandRange(0, 25);
		return FString::Printf(TEXT("%s %s %c"), *Prefix, *Suffix, Letter);
	}
	else
	{
		int32 Number = RandomStream.RandRange(1, 99);
		return FString::Printf(TEXT("%s %s %d"), *Prefix, *Suffix, Number);
	}
}

FString UNameGenerator::GenerateTradeSectorName(FRandomStream& RandomStream)
{
	TArray<FString> Prefixes = GetTradeSectorPrefixes();
	TArray<FString> Suffixes = GetTradeSectorSuffixes();
	const FString& Prefix = Prefixes[RandomStream.RandRange(0, Prefixes.Num() - 1)];
	const FString& Suffix = Suffixes[RandomStream.RandRange(0, Suffixes.Num() - 1)];
	int32 Number = RandomStream.RandRange(1, 99);
	return FString::Printf(TEXT("%s %s %d"), *Prefix, *Suffix, Number);
}

FString UNameGenerator::GenerateScientificSectorName(FRandomStream& RandomStream)
{
	TArray<FString> Prefixes = GetScientificSectorPrefixes();
	TArray<FString> Suffixes = GetScientificSectorSuffixes();
	const FString& Prefix = Prefixes[RandomStream.RandRange(0, Prefixes.Num() - 1)];
	const FString& Suffix = Suffixes[RandomStream.RandRange(0, Suffixes.Num() - 1)];
	
	// Scientific sectors often have designation codes
	TCHAR Letter1 = 'A' + RandomStream.RandRange(0, 25);
	TCHAR Letter2 = 'A' + RandomStream.RandRange(0, 25);
	int32 Number = RandomStream.RandRange(1, 999);
	return FString::Printf(TEXT("%s %s %c%c-%d"), *Prefix, *Suffix, Letter1, Letter2, Number);
}

FString UNameGenerator::GenerateFrontierSectorName(FRandomStream& RandomStream)
{
	TArray<FString> Prefixes = GetFrontierSectorPrefixes();
	TArray<FString> Suffixes = GetFrontierSectorSuffixes();
	const FString& Prefix = Prefixes[RandomStream.RandRange(0, Prefixes.Num() - 1)];
	const FString& Suffix = Suffixes[RandomStream.RandRange(0, Suffixes.Num() - 1)];
	int32 Number = RandomStream.RandRange(1, 99);
	return FString::Printf(TEXT("%s %s %d"), *Prefix, *Suffix, Number);
}

FString UNameGenerator::GenerateGreekSectorName(FRandomStream& RandomStream)
{
	TArray<FString> Letters = GetGreekLetters();
	const FString& Letter = Letters[RandomStream.RandRange(0, Letters.Num() - 1)];
	int32 Number = RandomStream.RandRange(1, 99);
	return FString::Printf(TEXT("%s Sector %d"), *Letter, Number);
}

FString UNameGenerator::GenerateNumericSectorName(FRandomStream& RandomStream)
{
	int32 Number = RandomStream.RandRange(1, 9999);
	return FString::Printf(TEXT("Sector-%04d"), Number);
}

FString UNameGenerator::GenerateDescriptiveSectorName(FRandomStream& RandomStream)
{
	// This would ideally be based on content, but for now generate generic descriptive names
	TArray<FString> Descriptors = {
		TEXT("Blue"), TEXT("Red"), TEXT("Crimson"), TEXT("Azure"), TEXT("Golden"),
		TEXT("Silver"), TEXT("Dark"), TEXT("Bright"), TEXT("Distant"), TEXT("Near"),
		TEXT("Hidden"), TEXT("Open"), TEXT("Dense"), TEXT("Sparse"), TEXT("Rich")
	};
	
	TArray<FString> Objects = {
		TEXT("Nebula"), TEXT("Cluster"), TEXT("Field"), TEXT("Expanse"), TEXT("Region"),
		TEXT("Zone"), TEXT("Space"), TEXT("Void"), TEXT("Belt"), TEXT("Ring")
	};
	
	const FString& Descriptor = Descriptors[RandomStream.RandRange(0, Descriptors.Num() - 1)];
	const FString& Object = Objects[RandomStream.RandRange(0, Objects.Num() - 1)];
	return FString::Printf(TEXT("%s %s"), *Descriptor, *Object);
}

FString UNameGenerator::GenerateStationName(const FString& StationType, int32 Seed)
{
	FRandomStream RandomStream = InitializeRandomStream(Seed);
	TArray<FString> Prefixes = GetStationPrefixes();
	TArray<FString> Suffixes = GetStationSuffixes();
	const FString& Prefix = Prefixes[RandomStream.RandRange(0, Prefixes.Num() - 1)];
	const FString& Suffix = Suffixes[RandomStream.RandRange(0, Suffixes.Num() - 1)];
	
	if (StationType.IsEmpty())
	{
		return FString::Printf(TEXT("%s %s"), *Prefix, *Suffix);
	}
	else
	{
		return FString::Printf(TEXT("%s %s %s"), *StationType, *Prefix, *Suffix);
	}
}

FString UNameGenerator::GenerateShipName(const FString& ShipClass, int32 Seed)
{
	FRandomStream RandomStream = InitializeRandomStream(Seed);
	TArray<FString> Prefixes = GetShipPrefixes();
	TArray<FString> Suffixes = GetShipSuffixes();
	const FString& Prefix = Prefixes[RandomStream.RandRange(0, Prefixes.Num() - 1)];
	const FString& Suffix = Suffixes[RandomStream.RandRange(0, Suffixes.Num() - 1)];
	
	if (ShipClass.IsEmpty())
	{
		return FString::Printf(TEXT("%s %s"), *Prefix, *Suffix);
	}
	else
	{
		return FString::Printf(TEXT("%s %s %s"), *Prefix, *ShipClass, *Suffix);
	}
}

bool UNameGenerator::IsNameUnique(const FString& Name, UObject* Context)
{
	if (!Context)
	{
		return false;
	}

	UWorld* World = Context->GetWorld();
	if (!World)
	{
		return false;
	}

	// Check all SpaceSectorMap actors for name collision
	TArray<AActor*> SectorActors;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceSectorMap::StaticClass(), SectorActors);
	
	for (AActor* Actor : SectorActors)
	{
		if (ASpaceSectorMap* Sector = Cast<ASpaceSectorMap>(Actor))
		{
			if (Sector->SectorName.ToString().Equals(Name, ESearchCase::IgnoreCase))
			{
				return false;
			}
		}
	}

	return true;
}

FString UNameGenerator::GenerateUniqueSectorName(uint8 Theme, UObject* Context, int32 MaxAttempts)
{
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Use attempt number as part of seed for variety
		FString Name = GenerateSectorName(Theme, Attempt);
		
		if (IsNameUnique(Name, Context))
		{
			return Name;
		}
	}

	// If we couldn't find a unique name, append a timestamp
	FString BaseName = GenerateSectorName(Theme, 0);
	int32 Timestamp = FDateTime::Now().GetTicks() % 10000;
	return FString::Printf(TEXT("%s-%d"), *BaseName, Timestamp);
}
