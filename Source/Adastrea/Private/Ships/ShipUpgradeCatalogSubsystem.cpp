#include "Ships/ShipUpgradeCatalogSubsystem.h"
#include "AdastreaLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

namespace
{
	bool ParseCategory(const FString& Name, EShipUpgradeCategory& Out)
	{
		const UEnum* Enum = StaticEnum<EShipUpgradeCategory>();
		const int64 Value = Enum ? Enum->GetValueByNameString(Name) : INDEX_NONE;
		if (Value == INDEX_NONE)
		{
			return false;
		}
		Out = static_cast<EShipUpgradeCategory>(Value);
		return true;
	}

	FString ReadString(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
	{
		FString Value;
		Object->TryGetStringField(Field, Value);
		return Value;
	}

	int32 ReadInt(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, int32 Default)
	{
		int32 Value = Default;
		Object->TryGetNumberField(Field, Value);
		return Value;
	}

	TArray<FName> ReadIdList(const TSharedPtr<FJsonObject>& Entry, const TCHAR* Field)
	{
		TArray<FName> Ids;
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (Entry->TryGetArrayField(Field, Values))
		{
			for (const TSharedPtr<FJsonValue>& Value : *Values)
			{
				Ids.Add(FName(*Value->AsString()));
			}
		}
		return Ids;
	}
}

void UShipUpgradeCatalogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FString FullPath = FPaths::ProjectContentDir() + GetCatalogPath();
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *FullPath))
	{
		UE_LOG(LogAdastreaShips, Warning, TEXT("ShipUpgradeCatalog: could not read %s; no upgrades for sale"), *FullPath);
		return;
	}

	TArray<UShipUpgradeDataAsset*> Parsed;
	TArray<FString> Errors;
	ParseCatalog(Json, this, Parsed, &Errors);
	for (const FString& Error : Errors)
	{
		UE_LOG(LogAdastreaShips, Warning, TEXT("ShipUpgradeCatalog: %s"), *Error);
	}
	SetUpgrades(Parsed);
	UE_LOG(LogAdastreaShips, Log, TEXT("ShipUpgradeCatalog: loaded %d upgrade(s) from %s"), Upgrades.Num(), GetCatalogPath());
}

UShipUpgradeCatalogSubsystem* UShipUpgradeCatalogSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UShipUpgradeCatalogSubsystem>() : nullptr;
}

int32 UShipUpgradeCatalogSubsystem::ParseCatalog(const FString& Json, UObject* Outer, TArray<UShipUpgradeDataAsset*>& OutUpgrades, TArray<FString>* OutErrors)
{
	auto Error = [OutErrors](const FString& Message)
	{
		if (OutErrors)
		{
			OutErrors->Add(Message);
		}
	};

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		Error(TEXT("invalid JSON"));
		return 0;
	}

	const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
	if (!Root->TryGetArrayField(TEXT("Upgrades"), Entries))
	{
		Error(TEXT("no 'Upgrades' array"));
		return 0;
	}

	// Prerequisites and exclusions name other entries, so resolve them after every entry exists.
	struct FPendingLinks
	{
		UShipUpgradeDataAsset* Upgrade;
		TArray<FName> Requires;
		TArray<FName> ExclusiveWith;
	};
	TArray<FPendingLinks> Links;
	TMap<FName, UShipUpgradeDataAsset*> ById;
	const int32 FirstNew = OutUpgrades.Num();

	for (const TSharedPtr<FJsonValue>& Value : *Entries)
	{
		const TSharedPtr<FJsonObject> Entry = Value.IsValid() ? Value->AsObject() : nullptr;
		if (!Entry.IsValid())
		{
			Error(TEXT("entry is not an object"));
			continue;
		}

		const FName Id(*ReadString(Entry, TEXT("id")));
		EShipUpgradeCategory Category = EShipUpgradeCategory::Systems;
		if (Id.IsNone() || !ParseCategory(ReadString(Entry, TEXT("category")), Category))
		{
			Error(FString::Printf(TEXT("entry '%s' needs an id and a valid category"), *Id.ToString()));
			continue;
		}
		if (ById.Contains(Id))
		{
			Error(FString::Printf(TEXT("duplicate id '%s'"), *Id.ToString()));
			continue;
		}

		UShipUpgradeDataAsset* Upgrade = NewObject<UShipUpgradeDataAsset>(Outer, NAME_None, RF_Transient);
		Upgrade->UpgradeID = Id;
		Upgrade->DisplayName = FText::FromString(ReadString(Entry, TEXT("name")));
		Upgrade->Description = FText::FromString(ReadString(Entry, TEXT("description")));
		Upgrade->Category = Category;
		Upgrade->Tier = static_cast<EUpgradeTier>(FMath::Clamp(ReadInt(Entry, TEXT("tier"), 1), 1, 5) - 1);
		Upgrade->Requirements.CreditCost = FMath::Max(0, ReadInt(Entry, TEXT("cost"), 0));

		Upgrade->MaxStackCount = FMath::Clamp(ReadInt(Entry, TEXT("maxStack"), 1), 1, 10);
		Upgrade->bIsUnique = Upgrade->MaxStackCount == 1;

		const TArray<TSharedPtr<FJsonValue>>* Stats = nullptr;
		if (Entry->TryGetArrayField(TEXT("stats"), Stats))
		{
			for (const TSharedPtr<FJsonValue>& StatValue : *Stats)
			{
				const TSharedPtr<FJsonObject> Stat = StatValue->AsObject();
				if (!Stat.IsValid())
				{
					continue;
				}
				FShipUpgradeStatModifier& Modifier = Upgrade->StatModifiers.AddDefaulted_GetRef();
				Modifier.StatName = FName(*ReadString(Stat, TEXT("stat")));
				double Add = 0.0;
				double Mult = 1.0;
				Stat->TryGetNumberField(TEXT("add"), Add);
				Stat->TryGetNumberField(TEXT("mult"), Mult);
				Modifier.AdditiveBonus = static_cast<float>(Add);
				Modifier.MultiplicativeBonus = static_cast<float>(Mult);
			}
		}
		if (Upgrade->StatModifiers.Num() == 0)
		{
			Error(FString::Printf(TEXT("'%s' modifies no stats"), *Id.ToString()));
		}

		ById.Add(Id, Upgrade);
		OutUpgrades.Add(Upgrade);
		Links.Add({ Upgrade, ReadIdList(Entry, TEXT("requires")), ReadIdList(Entry, TEXT("exclusiveWith")) });
	}

	for (const FPendingLinks& Link : Links)
	{
		for (const FName& Id : Link.Requires)
		{
			if (UShipUpgradeDataAsset* const* Found = ById.Find(Id))
			{
				Link.Upgrade->Requirements.PrerequisiteUpgrades.Add(*Found);
			}
			else
			{
				Error(FString::Printf(TEXT("'%s' requires unknown upgrade '%s'"), *Link.Upgrade->UpgradeID.ToString(), *Id.ToString()));
			}
		}
		for (const FName& Id : Link.ExclusiveWith)
		{
			if (UShipUpgradeDataAsset* const* Found = ById.Find(Id))
			{
				Link.Upgrade->MutuallyExclusiveWith.AddUnique(*Found);
			}
			else
			{
				Error(FString::Printf(TEXT("'%s' excludes unknown upgrade '%s'"), *Link.Upgrade->UpgradeID.ToString(), *Id.ToString()));
			}
		}
	}

	return OutUpgrades.Num() - FirstNew;
}

UShipUpgradeDataAsset* UShipUpgradeCatalogSubsystem::FindUpgrade(FName UpgradeID) const
{
	for (UShipUpgradeDataAsset* Upgrade : Upgrades)
	{
		if (Upgrade && Upgrade->UpgradeID == UpgradeID)
		{
			return Upgrade;
		}
	}
	return nullptr;
}

TArray<UShipUpgradeDataAsset*> UShipUpgradeCatalogSubsystem::GetUpgrades(EShipUpgradeCategory Category, EUpgradeTier MaxTier) const
{
	TArray<UShipUpgradeDataAsset*> Result;
	for (UShipUpgradeDataAsset* Upgrade : Upgrades)
	{
		if (Upgrade && Upgrade->Category == Category && Upgrade->Tier <= MaxTier)
		{
			Result.Add(Upgrade);
		}
	}
	Result.StableSort([](const UShipUpgradeDataAsset& A, const UShipUpgradeDataAsset& B)
	{
		return A.Tier != B.Tier ? A.Tier < B.Tier : A.Requirements.CreditCost < B.Requirements.CreditCost;
	});
	return Result;
}

void UShipUpgradeCatalogSubsystem::SetUpgrades(const TArray<UShipUpgradeDataAsset*>& InUpgrades)
{
	Upgrades.Reset();
	Upgrades.Append(InUpgrades);
}
