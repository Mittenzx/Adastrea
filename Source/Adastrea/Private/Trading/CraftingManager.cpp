// Copyright Mittenzx. All Rights Reserved.

#include "Trading/CraftingManager.h"
#include "Trading/CargoComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "Ships/Spaceship.h"
#include "Universe/GalaxySubsystem.h"
#include "AdastreaLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

namespace CraftingConsole
{
	ASpaceship* GetPlayerShip(UWorld* World)
	{
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		return PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs InfoCmd(
		TEXT("adastrea.CraftInfo"),
		TEXT("List crafting jobs and the docked station's production facilities."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
		{
			UCraftingManager* Crafting = UCraftingManager::Get(World);
			if (!Crafting)
			{
				return;
			}
			ASpaceship* Ship = GetPlayerShip(World);
			if (ASpaceStation* Station = Ship ? Ship->GetDockedStation() : nullptr)
			{
				for (const TPair<FString, int32>& Facility : Crafting->GetStationFacilities(Station))
				{
					UE_LOG(LogAdastrea, Display, TEXT("Craft: %s has %d %s module(s), %d recipes"),
						*Station->GetDisplayNameString(), Facility.Value, *Facility.Key, Crafting->GetRecipesFor(Facility.Key).Num());
				}
			}
			for (const FCraftingJob& Job : Crafting->GetJobs())
			{
				UE_LOG(LogAdastrea, Display, TEXT("Craft job %d at %s (%s): %s %d/%d runs, %d ready, %.0fs left"),
					Job.JobId, *Job.StationName, *Job.Facility, *Job.OutputItem.ToString(), Job.CraftsDone, Job.Crafts,
					Job.UnitsReady, Job.GetSecondsLeft());
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs CraftCmd(
		TEXT("adastrea.Craft"),
		TEXT("Queue a crafting job at the docked station. Usage: adastrea.Craft OutputItem [Runs=1]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UCraftingManager* Crafting = UCraftingManager::Get(World);
			ASpaceship* Ship = GetPlayerShip(World);
			ASpaceStation* Station = Ship ? Ship->GetDockedStation() : nullptr;
			FCraftingRecipe Recipe;
			if (!Crafting || !Station || Args.IsEmpty() || !Crafting->GetLoader()->FindRecipe(FName(*Args[0]), Recipe))
			{
				UE_LOG(LogAdastrea, Warning, TEXT("adastrea.Craft: dock at a station and name a recipe output"));
				return;
			}
			const int32 Runs = Args.Num() > 1 ? FMath::Max(1, FCString::Atoi(*Args[1])) : 1;
			const ECraftingCheck Check = Crafting->CheckQueue(Station, Recipe, Runs, Ship->CargoComponent);
			const int32 JobId = Check == ECraftingCheck::Ok ? Crafting->QueueJob(Station, Recipe, Runs, Ship->CargoComponent) : 0;
			UE_LOG(LogAdastrea, Display, TEXT("adastrea.Craft %s x%d: %s (job %d)"),
				*Args[0], Runs, *UCraftingManager::CheckToText(Check).ToString(), JobId);
		}));

	FAutoConsoleCommandWithWorldAndArgs SpeedCmd(
		TEXT("adastrea.CraftSpeed"),
		TEXT("Scale crafting job speed (testing). Usage: adastrea.CraftSpeed Multiplier"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UCraftingManager* Crafting = UCraftingManager::Get(World))
			{
				Crafting->SpeedMultiplier = Args.IsEmpty() ? 1.0f : FMath::Clamp(FCString::Atof(*Args[0]), 0.01f, 1000.0f);
				UE_LOG(LogAdastrea, Display, TEXT("Crafting speed x%.2f"), Crafting->SpeedMultiplier);
			}
		}));
}

UCraftingManager* UCraftingManager::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UCraftingManager>() : nullptr;
}

void UCraftingManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Loader = NewObject<UCraftingTreeLoader>(this);
	Loader->LoadCraftingTree();
	Loader->LoadRecipes();
	LoadResearchRequirements();
	for (const FCraftingRecipe& Recipe : Loader->GetAllRecipes())
	{
		if (!Recipe.Ingredients.IsEmpty() && !Recipe.ProducedIn.StartsWith(TEXT("Contract:")))
		{
			CraftingFacilities.Add(Recipe.ProducedIn);
		}
	}
	UE_LOG(LogAdastrea, Log, TEXT("CraftingManager: %d items, %d recipes (%d need research)"),
		Loader->GetLoadedItemCount(), Loader->GetLoadedRecipeCount(), ResearchByRecipe.Num());
}

void UCraftingManager::LoadResearchRequirements()
{
	ResearchByRecipe.Reset();
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *(FPaths::ProjectContentDir() + UCraftingTreeLoader::GetCraftingTreePath())))
	{
		return;
	}
	TSharedPtr<FJsonObject> Root;
	const TArray<TSharedPtr<FJsonValue>>* Recipes = nullptr;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonStr), Root) || !Root.IsValid()
		|| !Root->TryGetArrayField(TEXT("Recipes"), Recipes))
	{
		return;
	}
	for (const TSharedPtr<FJsonValue>& Val : *Recipes)
	{
		const TSharedPtr<FJsonObject> Obj = Val->AsObject();
		FString Id, Research;
		if (Obj.IsValid() && Obj->TryGetStringField(TEXT("RecipeID"), Id) && Obj->TryGetStringField(TEXT("ResearchRequired"), Research)
			&& !Research.IsEmpty())
		{
			ResearchByRecipe.Add(Id, Research);
		}
	}
}

float UCraftingManager::GetBaseSecondsPerCraft(const FCraftingRecipe& Recipe)
{
	static const float SecondsByTier[] = { 6.0f, 10.0f, 16.0f, 25.0f, 40.0f, 60.0f, 90.0f };
	return SecondsByTier[FMath::Clamp(Recipe.Tier, 1, 7) - 1];
}

FString UCraftingManager::GetFacilityForModuleType(const FString& ModuleType)
{
	// "Kinetic Weapons Research Lab" -> "KineticWeaponsLab", then the few labs the tree names differently.
	FString Tag = ModuleType.Replace(TEXT(" "), TEXT("")).Replace(TEXT("Research"), TEXT(""));
	static const TMap<FString, FString> Aliases = {
		{ TEXT("BiotechLab"), TEXT("BiologyLab") },
		{ TEXT("ComputingLab"), TEXT("ElectronicsLab") },
		{ TEXT("KineticWeaponsLab"), TEXT("ProjectileWeaponsLab") },
		{ TEXT("PropulsionLab"), TEXT("PhysicsLab") },
	};
	if (const FString* Alias = Aliases.Find(Tag))
	{
		Tag = *Alias;
	}
	return Tag;
}

TArray<TPair<FString, int32>> UCraftingManager::GetStationFacilities(const ASpaceStation* Station) const
{
	TArray<TPair<FString, int32>> Out;
	if (!Station || !Loader)
	{
		return Out;
	}
	for (const ASpaceStationModule* Module : Station->GetModules())
	{
		if (!IsValid(Module) || Module->IsDestroyed_Implementation())
		{
			continue;
		}
		const FString Facility = GetFacilityForModuleType(Module->GetModuleType());
		if (!CraftingFacilities.Contains(Facility))
		{
			continue;
		}
		if (TPair<FString, int32>* Existing = Out.FindByPredicate([&Facility](const TPair<FString, int32>& P) { return P.Key == Facility; }))
		{
			++Existing->Value;
		}
		else
		{
			Out.Emplace(Facility, 1);
		}
	}
	// Processing (refining) first, then fabrication, then the rest by name.
	auto Rank = [](const FString& F) { return F == TEXT("Processing") ? 0 : F == TEXT("Fabrication") ? 1 : 2; };
	Out.Sort([&Rank](const TPair<FString, int32>& A, const TPair<FString, int32>& B)
	{
		return Rank(A.Key) != Rank(B.Key) ? Rank(A.Key) < Rank(B.Key) : A.Key < B.Key;
	});
	return Out;
}

TArray<FCraftingRecipe> UCraftingManager::GetRecipesFor(const FString& Facility) const
{
	TArray<FCraftingRecipe> Out = Loader ? Loader->GetRecipesForFacility(Facility) : TArray<FCraftingRecipe>();
	Out.RemoveAll([](const FCraftingRecipe& R) { return R.Ingredients.IsEmpty(); });
	Out.StableSort([](const FCraftingRecipe& A, const FCraftingRecipe& B) { return A.Tier < B.Tier; });
	return Out;
}

FString UCraftingManager::GetResearchRequired(const FCraftingRecipe& Recipe) const
{
	return ResearchByRecipe.FindRef(Recipe.RecipeID);
}

int32 UCraftingManager::GetAffordableCrafts(const FCraftingRecipe& Recipe, const UCargoComponent* Cargo)
{
	if (!Cargo || Recipe.Ingredients.IsEmpty())
	{
		return 0;
	}
	int32 Runs = 999;
	for (const FCraftIngredient& Ing : Recipe.Ingredients)
	{
		if (Ing.Quantity > 0)
		{
			Runs = FMath::Min(Runs, Cargo->GetItemQuantityByID(Ing.ItemID) / Ing.Quantity);
		}
	}
	return Runs;
}

ECraftingCheck UCraftingManager::CheckQueue(const ASpaceStation* Station, const FCraftingRecipe& Recipe, int32 Crafts, const UCargoComponent* Cargo) const
{
	if (Recipe.OutputItem.IsNone() || Recipe.Ingredients.IsEmpty())
	{
		return ECraftingCheck::NoRecipe;
	}
	if (!GetStationFacilities(Station).ContainsByPredicate([&Recipe](const TPair<FString, int32>& P) { return P.Key == Recipe.ProducedIn; }))
	{
		return ECraftingCheck::NoFacility;
	}
	if (!GetResearchRequired(Recipe).IsEmpty())
	{
		return ECraftingCheck::NeedsResearch;
	}
	if (GetAffordableCrafts(Recipe, Cargo) < FMath::Max(1, Crafts))
	{
		return ECraftingCheck::MissingIngredients;
	}
	const FName Key = GetStationKey(Station);
	int32 Queued = 0;
	for (const FCraftingJob& Job : Jobs)
	{
		Queued += (Job.StationKey == Key && Job.Facility == Recipe.ProducedIn && Job.IsRunning()) ? 1 : 0;
	}
	return Queued >= MaxJobsPerFacility ? ECraftingCheck::QueueFull : ECraftingCheck::Ok;
}

FText UCraftingManager::CheckToText(ECraftingCheck Check)
{
	switch (Check)
	{
	case ECraftingCheck::Ok:                 return NSLOCTEXT("Crafting", "Ok", "Ready to queue");
	case ECraftingCheck::NoRecipe:           return NSLOCTEXT("Crafting", "NoRecipe", "Nothing to make");
	case ECraftingCheck::NoFacility:         return NSLOCTEXT("Crafting", "NoFacility", "No module here makes this");
	case ECraftingCheck::NeedsResearch:      return NSLOCTEXT("Crafting", "NeedsResearch", "Needs research");
	case ECraftingCheck::MissingIngredients: return NSLOCTEXT("Crafting", "Missing", "Missing ingredients");
	case ECraftingCheck::QueueFull:          return NSLOCTEXT("Crafting", "QueueFull", "Queue full");
	}
	return FText::GetEmpty();
}

int32 UCraftingManager::QueueJob(ASpaceStation* Station, const FCraftingRecipe& Recipe, int32 Crafts, UCargoComponent* Cargo)
{
	Crafts = FMath::Max(1, Crafts);
	if (CheckQueue(Station, Recipe, Crafts, Cargo) != ECraftingCheck::Ok)
	{
		return 0;
	}
	for (const FCraftIngredient& Ing : Recipe.Ingredients)
	{
		if (Ing.Quantity > 0 && !Cargo->RemoveCargoByID(Ing.ItemID, Ing.Quantity * Crafts))
		{
			UE_LOG(LogAdastrea, Error, TEXT("CraftingManager: lost track of %s while queueing %s"), *Ing.ItemID.ToString(), *Recipe.RecipeID);
			return 0;
		}
	}

	int32 Modules = 1;
	for (const TPair<FString, int32>& Facility : GetStationFacilities(Station))
	{
		Modules = Facility.Key == Recipe.ProducedIn ? Facility.Value : Modules;
	}

	FCraftingJob& Job = Jobs.AddDefaulted_GetRef();
	Job.JobId = NextJobId++;
	Job.StationKey = GetStationKey(Station);
	Job.StationName = Station->GetDisplayNameString();
	Job.Facility = Recipe.ProducedIn;
	Job.OutputItem = Recipe.OutputItem;
	Job.OutputPerCraft = FMath::Max(1, Recipe.OutputQuantity);
	Job.Crafts = Crafts;
	Job.SecondsPerCraft = GetBaseSecondsPerCraft(Recipe) / FMath::Max(1, Modules);
	UE_LOG(LogAdastrea, Log, TEXT("CraftingManager: job %d queued at %s: %d x %s (%s, %.1fs per run)"),
		Job.JobId, *Job.StationName, Crafts, *Recipe.OutputItem.ToString(), *Recipe.ProducedIn, Job.SecondsPerCraft);
	return Job.JobId;
}

bool UCraftingManager::CancelJob(int32 JobId, UCargoComponent* Cargo)
{
	FCraftingJob* Job = FindJob(JobId);
	FCraftingRecipe Recipe;
	if (!Job || !Loader->FindRecipe(Job->OutputItem, Recipe))
	{
		return false;
	}
	// The run in progress is lost with its ingredients; runs not started are refunded.
	const int32 NotStarted = FMath::Max(0, Job->Crafts - Job->CraftsDone - (Job->Progress > 0.0f ? 1 : 0));
	if (Cargo)
	{
		for (const FCraftIngredient& Ing : Recipe.Ingredients)
		{
			UTradeItemDataAsset* Item = Loader->GetTradeItem(Ing.ItemID.ToString());
			if (Item && NotStarted > 0)
			{
				const int32 Units = Ing.Quantity * NotStarted;
				const int32 Fits = FMath::Min(Units, Item->VolumePerUnit > 0.0f
					? FMath::FloorToInt(Cargo->GetAvailableCargoSpace() / Item->VolumePerUnit) : Units);
				if (Fits > 0)
				{
					Cargo->AddCargo(Item, Fits);
				}
			}
		}
		if (Job->UnitsReady > 0)
		{
			if (UTradeItemDataAsset* Out = Loader->GetTradeItem(Job->OutputItem.ToString()))
			{
				Cargo->AddCargo(Out, Job->UnitsReady);
			}
		}
	}
	UE_LOG(LogAdastrea, Log, TEXT("CraftingManager: job %d cancelled (%d runs refunded)"), JobId, NotStarted);
	Jobs.RemoveAll([JobId](const FCraftingJob& J) { return J.JobId == JobId; });
	return true;
}

int32 UCraftingManager::CollectAtStation(const ASpaceStation* Station, UCargoComponent* Cargo)
{
	if (!Station || !Cargo)
	{
		return 0;
	}
	const FName Key = GetStationKey(Station);
	int32 Moved = 0;
	for (FCraftingJob& Job : Jobs)
	{
		UTradeItemDataAsset* Out = Job.StationKey == Key && Job.UnitsReady > 0 ? Loader->GetTradeItem(Job.OutputItem.ToString()) : nullptr;
		if (!Out)
		{
			continue;
		}
		int32 Units = Job.UnitsReady;
		if (Out->VolumePerUnit > 0.0f)
		{
			Units = FMath::Min(Units, FMath::FloorToInt(Cargo->GetAvailableCargoSpace() / Out->VolumePerUnit + KINDA_SMALL_NUMBER));
		}
		if (Units > 0 && Cargo->AddCargo(Out, Units))
		{
			Job.UnitsReady -= Units;
			Moved += Units;
		}
	}
	Jobs.RemoveAll([](const FCraftingJob& J) { return J.IsFinished(); });
	if (Moved > 0)
	{
		UE_LOG(LogAdastrea, Log, TEXT("CraftingManager: collected %d crafted units at %s"), Moved, *Station->GetDisplayNameString());
	}
	return Moved;
}

TArray<FCraftingJob> UCraftingManager::GetJobs(const ASpaceStation* Station) const
{
	if (!Station)
	{
		return Jobs;
	}
	const FName Key = GetStationKey(Station);
	return Jobs.FilterByPredicate([Key](const FCraftingJob& J) { return J.StationKey == Key; });
}

FName UCraftingManager::GetStationKey(const ASpaceStation* Station)
{
	if (!Station)
	{
		return NAME_None;
	}
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(Station);
	const FName Sector = Galaxy ? Galaxy->ResolveCurrentSectorId(Station) : NAME_None;
	const FString Level = Sector.IsNone() ? Station->GetWorld()->GetMapName() : Sector.ToString();
	return FName(*FString::Printf(TEXT("%s/%s"), *Level, *Station->GetDisplayNameString()));
}

void UCraftingManager::ImportJobs(const TArray<FCraftingJob>& In)
{
	Jobs = In;
	NextJobId = 1;
	for (const FCraftingJob& Job : Jobs)
	{
		NextJobId = FMath::Max(NextJobId, Job.JobId + 1);
	}
}

FCraftingJob* UCraftingManager::FindJob(int32 JobId)
{
	return Jobs.FindByPredicate([JobId](const FCraftingJob& J) { return J.JobId == JobId; });
}

UCargoComponent* UCraftingManager::GetDockedPlayerCargo(FName StationKey) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const ASpaceship* Ship = It->IsValid() ? Cast<ASpaceship>((*It)->GetPawn()) : nullptr;
		const ASpaceStation* Docked = Ship && Ship->IsDocked() ? Ship->GetDockedStation() : nullptr;
		if (Docked && GetStationKey(Docked) == StationKey)
		{
			return Ship->CargoComponent;
		}
	}
	return nullptr;
}

void UCraftingManager::Tick(float DeltaTime)
{
	// Each station works one job per facility at a time, oldest first.
	TSet<FString> Busy;
	for (FCraftingJob& Job : Jobs)
	{
		if (!Job.IsRunning())
		{
			continue;
		}
		const FString Line = Job.StationKey.ToString() + TEXT("|") + Job.Facility;
		if (Busy.Contains(Line))
		{
			continue;
		}
		Busy.Add(Line);

		Job.Progress += DeltaTime * SpeedMultiplier;
		int32 Made = 0;
		while (Job.IsRunning() && Job.Progress >= Job.SecondsPerCraft)
		{
			Job.Progress -= Job.SecondsPerCraft;
			++Job.CraftsDone;
			Made += Job.OutputPerCraft;
		}
		if (!Job.IsRunning())
		{
			Job.Progress = 0.0f;
		}
		if (Made > 0)
		{
			Job.UnitsReady += Made;
			UE_LOG(LogAdastrea, Log, TEXT("CraftingManager: job %d made %d x %s at %s (%d/%d runs)"),
				Job.JobId, Made, *Job.OutputItem.ToString(), *Job.StationName, Job.CraftsDone, Job.Crafts);
			OnCraftCompleted.Broadcast(Job.JobId, Made);
		}
	}

	// Docked at a station with finished goods: they go straight into the hold.
	for (const FCraftingJob& Job : Jobs)
	{
		if (Job.UnitsReady > 0)
		{
			if (UCargoComponent* Cargo = GetDockedPlayerCargo(Job.StationKey))
			{
				if (ASpaceship* Ship = Cast<ASpaceship>(Cargo->GetOwner()))
				{
					CollectAtStation(Ship->GetDockedStation(), Cargo);
				}
				break; // CollectAtStation may have removed jobs
			}
		}
	}
}

TStatId UCraftingManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCraftingManager, STATGROUP_Tickables);
}

ETickableTickType UCraftingManager::GetTickableTickType() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Conditional;
}

bool UCraftingManager::IsTickable() const
{
	const UWorld* World = GetWorld();
	return World && World->IsGameWorld() && !Jobs.IsEmpty();
}
