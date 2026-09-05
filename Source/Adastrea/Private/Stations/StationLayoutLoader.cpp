// Copyright Mittenzx. All Rights Reserved.

#include "Stations/StationLayoutLoader.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

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

	Layout->StationName = Root->GetStringField(TEXT("StationName"));
	Layout->GridSpacing = (float)Root->GetNumberField(TEXT("GridSpacing"), 100.0);

	const TArray<FJsonValue>& PlotArr = Root->GetArrayField(TEXT("PlotSize"));
	if (PlotArr.Num() >= 3)
	{
		Layout->PlotSize = FVector((float)PlotArr[0].GetNumber(),
			(float)PlotArr[1].GetNumber(), (float)PlotArr[2].GetNumber());
	}

	const TArray<FJsonValue>& Mods = Root->GetArrayField(TEXT("Modules"));
	for (const FJsonValue& Mv : Mods)
	{
		const TSharedPtr<FJsonObject>& Mo = Mv.GetObject();
		if (!Mo.IsValid())
		{
			continue;
		}
		FStationLayoutModule Mod;
		Mod.ModuleID = Mo->GetStringField(TEXT("ModuleID"));
		Mod.ItemID = FName(*Mo->GetStringField(TEXT("ItemID")));
		const TArray<FJsonValue>& Gp = Mo->GetArrayField(TEXT("GridPos"));
		if (Gp.Num() >= 3)
		{
			Mod.GridPos = FIntVector((int32)Gp[0].GetNumber(),
				(int32)Gp[1].GetNumber(), (int32)Gp[2].GetNumber());
		}
		Mod.Rotation = (int32)Mo->GetNumberField(TEXT("Rotation"), 0.0);
		Mod.IsCore = Mo->GetBoolField(TEXT("IsCore"));
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