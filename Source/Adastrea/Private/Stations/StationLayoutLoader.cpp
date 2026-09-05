// Copyright Mittenzx. All Rights Reserved.

#include "Stations/StationLayoutLoader.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

int32 UStationLayoutLoader::LoadLayout(UStationLayoutDataAsset* Layout)
{
	if (!Layout)
	{
		return 0;
	}
	Layout->Modules.Empty();

	const FString FullPath = FPaths::ProjectContentDir() + GetLayoutPath();
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FullPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("StationLayoutLoader: could not read %s"), *FullPath);
		return 0;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("StationLayoutLoader: invalid JSON in %s"), *FullPath);
		return 0;
	}

	Root->TryGetStringField(TEXT("StationName"), Layout->StationName);

	double Spacing = 100.0;
	Root->TryGetNumberField(TEXT("GridSpacing"), Spacing);
	Layout->GridSpacing = (float)Spacing;

	// PlotSize: [x, y, z] array of numbers.
	const TArray<TSharedPtr<FJsonValue>>* PlotArr = nullptr;
	if (Root->TryGetArrayField(TEXT("PlotSize"), PlotArr) && PlotArr->Num() >= 3)
	{
		double PX = 0.0, PY = 0.0, PZ = 0.0;
		(*PlotArr)[0]->TryGetNumber(PX);
		(*PlotArr)[1]->TryGetNumber(PY);
		(*PlotArr)[2]->TryGetNumber(PZ);
		Layout->PlotSize = FVector((float)PX, (float)PY, (float)PZ);
	}

	// Modules: array of objects.
	const TArray<TSharedPtr<FJsonValue>>* Mods = nullptr;
	if (!Root->TryGetArrayField(TEXT("Modules"), Mods))
	{
		UE_LOG(LogTemp, Warning, TEXT("StationLayoutLoader: no 'Modules' array"));
		return 0;
	}

	for (const TSharedPtr<FJsonValue>& Mv : *Mods)
	{
		const TSharedPtr<FJsonObject>& Mo = Mv->AsObject();
		if (!Mo.IsValid())
		{
			continue;
		}
		FStationLayoutModule Mod;
		FString ItemIDStr;
		Mo->TryGetStringField(TEXT("ModuleID"), Mod.ModuleID);
		Mo->TryGetStringField(TEXT("ItemID"), ItemIDStr);
		Mod.ItemID = FName(*ItemIDStr);

		// GridPos: [x, y, z] array of numbers.
		const TArray<TSharedPtr<FJsonValue>>* Gp = nullptr;
		if (Mo->TryGetArrayField(TEXT("GridPos"), Gp) && Gp->Num() >= 3)
		{
			int32 GX = 0, GY = 0, GZ = 0;
			(*Gp)[0]->TryGetNumber(GX);
			(*Gp)[1]->TryGetNumber(GY);
			(*Gp)[2]->TryGetNumber(GZ);
			Mod.GridPos = FIntVector(GX, GY, GZ);
		}

		Mo->TryGetNumberField(TEXT("Rotation"), Mod.Rotation);
		Mo->TryGetBoolField(TEXT("IsCore"), Mod.IsCore);
		Layout->Modules.Add(Mod);
	}

	return Layout->Modules.Num();
}

void UStationLayoutLoader::PlaceModules(UStationLayoutDataAsset* Layout, AActor* SpawnedStationModules, float SpacingNull)
{
	// Placement of real module actors is delegated to ASpaceStation::BuildFromLayout()
	// (re-enabled module-management API in SpaceStation.h). This entry point exists
	// so builders/blueprints can trigger a spawn uniformly.
	UE_LOG(LogTemp, Warning, TEXT("StationLayoutLoader: PlaceModules is a stub — use ASpaceStation::BuildFromLayout."));
}