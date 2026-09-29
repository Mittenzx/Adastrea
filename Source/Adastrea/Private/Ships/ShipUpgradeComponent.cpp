#include "Ships/ShipUpgradeComponent.h"
#include "Ships/Spaceship.h"
#include "Stations/OutfittingModule.h"
#include "Trading/PlayerTraderComponent.h"
#include "Player/PlayerProgressionComponent.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"

UShipUpgradeComponent::UShipUpgradeComponent()
	: ShipTypeID(NAME_None)
	, MaxUpgradeSlots(20)
	, SellBackFraction(0.5f)
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default hardpoints; ASpaceship sizes Weapons from its data asset's WeaponSlots.
	CategorySlots.Add(EShipUpgradeCategory::Engines, 2);
	CategorySlots.Add(EShipUpgradeCategory::Weapons, 2);
	CategorySlots.Add(EShipUpgradeCategory::Shields, 2);
	CategorySlots.Add(EShipUpgradeCategory::Hull, 2);
	CategorySlots.Add(EShipUpgradeCategory::Cargo, 3);
}

void UShipUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UShipUpgradeComponent::InstallUpgrade(UShipUpgradeDataAsset* Upgrade, bool bIgnoreRequirements)
{
	if (!Upgrade)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipUpgradeComponent: Cannot install null upgrade"));
		return false;
	}

	FText Reason;
	if (!bIgnoreRequirements && !CanInstallUpgrade(Upgrade, Reason))
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipUpgradeComponent: Cannot install %s: %s"), *Upgrade->UpgradeID.ToString(), *Reason.ToString());
		OnUpgradeInstallFailed.Broadcast(Upgrade, Reason);
		return false;
	}

	int32 NewStackCount = 1;
	if (FInstalledUpgrade* Existing = FindInstalledUpgrade(Upgrade->UpgradeID))
	{
		// CanInstallUpgrade already refused a stack past MaxStackCount; a forced install
		// (save load) still respects the cap.
		if (Existing->StackCount >= Upgrade->MaxStackCount)
		{
			OnUpgradeInstallFailed.Broadcast(Upgrade, FText::FromString(TEXT("Already installed")));
			return false;
		}
		NewStackCount = ++Existing->StackCount;
	}
	else
	{
		InstalledUpgrades.Add(FInstalledUpgrade(Upgrade, 1));
	}

	UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: Installed %s (%s) x%d"),
		*Upgrade->UpgradeID.ToString(), *Upgrade->DisplayName.ToString(), NewStackCount);
	OnUpgradeInstalled.Broadcast(Upgrade, NewStackCount);
	OnUpgradesChanged.Broadcast();
	return true;
}

bool UShipUpgradeComponent::UninstallUpgrade(FName UpgradeID)
{
	FInstalledUpgrade* Installed = FindInstalledUpgrade(UpgradeID);
	if (!Installed)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipUpgradeComponent: Upgrade not installed: %s"), *UpgradeID.ToString());
		return false;
	}

	if (--Installed->StackCount <= 0)
	{
		InstalledUpgrades.RemoveAll([UpgradeID](const FInstalledUpgrade& Entry)
		{
			return Entry.Upgrade && Entry.Upgrade->UpgradeID == UpgradeID;
		});
	}

	UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: Uninstalled one stack of %s"), *UpgradeID.ToString());
	OnUpgradeUninstalled.Broadcast(UpgradeID);
	OnUpgradesChanged.Broadcast();
	return true;
}

bool UShipUpgradeComponent::CanInstallUpgrade(UShipUpgradeDataAsset* Upgrade, FText& OutReason) const
{
	if (!Upgrade)
	{
		OutReason = FText::FromString(TEXT("Invalid upgrade"));
		return false;
	}

	const FInstalledUpgrade* Existing = FindInstalledUpgrade(Upgrade->UpgradeID);
	if (Existing && Existing->StackCount >= Upgrade->MaxStackCount)
	{
		OutReason = FText::FromString(Upgrade->MaxStackCount == 1
			? TEXT("Already installed")
			: FString::Printf(TEXT("Already fitted %d times (maximum)"), Upgrade->MaxStackCount));
		return false;
	}

	const int32 Slots = GetSlotCount(Upgrade->Category);
	if (Slots <= 0)
	{
		OutReason = FText::Format(FText::FromString(TEXT("This ship has no {0} slots")), Upgrade->GetCategoryDisplayName());
		return false;
	}
	if (GetUsedSlotCount(Upgrade->Category) >= Slots)
	{
		OutReason = FText::Format(FText::FromString(TEXT("All {0} {1} slots are full")),
			FText::AsNumber(Slots), Upgrade->GetCategoryDisplayName());
		return false;
	}
	if (GetRemainingUpgradeSlots() <= 0)
	{
		OutReason = FText::FromString(TEXT("No upgrade slots available"));
		return false;
	}

	if (!Upgrade->IsCompatibleWithShipType(ShipTypeID))
	{
		OutReason = FText::FromString(TEXT("Incompatible with this ship type"));
		return false;
	}

	if (HasUpgradeConflicts(Upgrade))
	{
		OutReason = FText::FromString(TEXT("Conflicts with an installed upgrade"));
		return false;
	}

	return CheckUpgradeRequirements(Upgrade, OutReason);
}

bool UShipUpgradeComponent::IsUpgradeInstalled(FName UpgradeID) const
{
	return FindInstalledUpgrade(UpgradeID) != nullptr;
}

int32 UShipUpgradeComponent::GetUpgradeStackCount(FName UpgradeID) const
{
	const FInstalledUpgrade* Installed = FindInstalledUpgrade(UpgradeID);
	return Installed ? Installed->StackCount : 0;
}

void UShipUpgradeComponent::UninstallAllUpgrades()
{
	if (InstalledUpgrades.Num() == 0)
	{
		return;
	}
	InstalledUpgrades.Empty();
	UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: All upgrades uninstalled"));
	OnUpgradesChanged.Broadcast();
}

// ====================
// Outfitting
// ====================

bool UShipUpgradeComponent::CanPurchaseUpgrade(UShipUpgradeDataAsset* Upgrade, const AOutfittingModule* Module, const UPlayerTraderComponent* Wallet, FText& OutReason) const
{
	if (!Upgrade)
	{
		OutReason = FText::FromString(TEXT("Invalid upgrade"));
		return false;
	}
	if (!Module || !Module->ServicesCategory(Upgrade->Category))
	{
		OutReason = FText::Format(FText::FromString(TEXT("No {0} workshop at this station")), Upgrade->GetCategoryDisplayName());
		return false;
	}
	if (!Module->CanFit(Upgrade))
	{
		OutReason = FText::Format(FText::FromString(TEXT("{0} can only fit up to tier {1}")),
			FText::FromString(Module->ModuleType), FText::AsNumber(static_cast<int32>(Module->GetMaxTier()) + 1));
		return false;
	}
	if (!CanInstallUpgrade(Upgrade, OutReason))
	{
		return false;
	}
	const int32 Price = Module->GetPrice(Upgrade);
	if (!Wallet || Wallet->GetCredits() < Price)
	{
		OutReason = FText::Format(FText::FromString(TEXT("Needs {0} credits")), FText::AsNumber(Price));
		return false;
	}
	return true;
}

bool UShipUpgradeComponent::PurchaseUpgrade(UShipUpgradeDataAsset* Upgrade, AOutfittingModule* Module, UPlayerTraderComponent* Wallet, FText& OutReason)
{
	if (!CanPurchaseUpgrade(Upgrade, Module, Wallet, OutReason))
	{
		return false;
	}

	const int32 Price = Module->GetPrice(Upgrade);
	if (!Wallet->RemoveCredits(Price))
	{
		OutReason = FText::Format(FText::FromString(TEXT("Needs {0} credits")), FText::AsNumber(Price));
		return false;
	}
	if (!InstallUpgrade(Upgrade, false))
	{
		// Checked above, so this is unexpected; don't keep the money for nothing.
		Wallet->AddCredits(Price);
		OutReason = FText::FromString(TEXT("Installation failed"));
		return false;
	}

	UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: Bought %s at %s for %d cr"),
		*Upgrade->UpgradeID.ToString(), *Module->GetName(), Price);
	return true;
}

bool UShipUpgradeComponent::CanSellUpgrade(FName UpgradeID, const AOutfittingModule* Module, FText& OutReason) const
{
	const FInstalledUpgrade* Installed = FindInstalledUpgrade(UpgradeID);
	if (!Installed || !Installed->Upgrade)
	{
		OutReason = FText::FromString(TEXT("Not installed"));
		return false;
	}
	const UShipUpgradeDataAsset* Upgrade = Installed->Upgrade;
	if (!Module || !Module->ServicesCategory(Upgrade->Category))
	{
		OutReason = FText::Format(FText::FromString(TEXT("No {0} workshop at this station")), Upgrade->GetCategoryDisplayName());
		return false;
	}
	for (const FInstalledUpgrade& Other : InstalledUpgrades)
	{
		if (Other.Upgrade && Other.Upgrade != Upgrade && Installed->StackCount == 1
			&& Other.Upgrade->Requirements.PrerequisiteUpgrades.Contains(Upgrade))
		{
			OutReason = FText::Format(FText::FromString(TEXT("{0} depends on it")), Other.Upgrade->DisplayName);
			return false;
		}
	}
	if (const ASpaceship* Ship = Cast<ASpaceship>(GetOwner()))
	{
		return Ship->CanRemoveUpgrade(Upgrade, OutReason);
	}
	return true;
}

bool UShipUpgradeComponent::SellUpgrade(FName UpgradeID, AOutfittingModule* Module, UPlayerTraderComponent* Wallet, int32& OutRefund, FText& OutReason)
{
	OutRefund = 0;
	if (!CanSellUpgrade(UpgradeID, Module, OutReason))
	{
		return false;
	}

	const int32 Refund = GetSellPrice(FindInstalledUpgrade(UpgradeID)->Upgrade);
	if (!UninstallUpgrade(UpgradeID))
	{
		OutReason = FText::FromString(TEXT("Removal failed"));
		return false;
	}
	if (Wallet && Refund > 0)
	{
		Wallet->AddCredits(Refund);
	}
	OutRefund = Refund;
	return true;
}

int32 UShipUpgradeComponent::GetSellPrice(const UShipUpgradeDataAsset* Upgrade) const
{
	return Upgrade ? FMath::FloorToInt(Upgrade->Requirements.CreditCost * SellBackFraction) : 0;
}

// ====================
// Slots
// ====================

int32 UShipUpgradeComponent::GetSlotCount(EShipUpgradeCategory Category) const
{
	const int32* Slots = CategorySlots.Find(Category);
	return Slots ? FMath::Max(*Slots, 0) : 0;
}

int32 UShipUpgradeComponent::GetUsedSlotCount(EShipUpgradeCategory Category) const
{
	int32 Used = 0;
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (Installed.Upgrade && Installed.Upgrade->Category == Category)
		{
			Used += Installed.StackCount;
		}
	}
	return Used;
}

int32 UShipUpgradeComponent::GetRemainingUpgradeSlots() const
{
	int32 Used = 0;
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		Used += Installed.StackCount;
	}
	return FMath::Max(0, MaxUpgradeSlots - Used);
}

// ====================
// Stats
// ====================

namespace
{
	/** Accumulate Upgrade's modifiers for StatName, Stacks times. */
	void AccumulateStat(const UShipUpgradeDataAsset* Upgrade, FName StatName, int32 Stacks, float& InOutAdd, float& InOutMult)
	{
		if (!Upgrade || Stacks <= 0)
		{
			return;
		}
		for (const FShipUpgradeStatModifier& Modifier : Upgrade->StatModifiers)
		{
			if (Modifier.StatName == StatName)
			{
				InOutAdd += Modifier.AdditiveBonus * Stacks;
				InOutMult *= FMath::Pow(Modifier.MultiplicativeBonus, static_cast<float>(Stacks));
			}
		}
	}
}

float UShipUpgradeComponent::GetStatModifier(FName StatName, float BaseValue) const
{
	return PreviewStat(StatName, BaseValue, nullptr, NAME_None);
}

float UShipUpgradeComponent::PreviewStat(FName StatName, float BaseValue, const UShipUpgradeDataAsset* AddedUpgrade, FName RemovedUpgradeID) const
{
	float Add = 0.0f;
	float Mult = 1.0f;
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (!Installed.Upgrade)
		{
			continue;
		}
		const int32 Stacks = Installed.StackCount - (Installed.Upgrade->UpgradeID == RemovedUpgradeID && !RemovedUpgradeID.IsNone() ? 1 : 0);
		AccumulateStat(Installed.Upgrade, StatName, Stacks, Add, Mult);
	}
	AccumulateStat(AddedUpgrade, StatName, 1, Add, Mult);
	return (BaseValue + Add) * Mult;
}

float UShipUpgradeComponent::GetStatBonusPercentage(FName StatName) const
{
	constexpr float BaseValue = 100.0f;
	return (GetStatModifier(StatName, BaseValue) - BaseValue) / BaseValue;
}

TMap<FName, float> UShipUpgradeComponent::GetAllStatModifiers() const
{
	TMap<FName, float> Modifiers;
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (!Installed.Upgrade)
		{
			continue;
		}
		for (const FShipUpgradeStatModifier& StatMod : Installed.Upgrade->StatModifiers)
		{
			if (!Modifiers.Contains(StatMod.StatName))
			{
				Modifiers.Add(StatMod.StatName, GetStatBonusPercentage(StatMod.StatName));
			}
		}
	}
	return Modifiers;
}

TArray<FInstalledUpgrade> UShipUpgradeComponent::GetUpgradesByCategory(EShipUpgradeCategory Category) const
{
	TArray<FInstalledUpgrade> Result;
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (Installed.Upgrade && Installed.Upgrade->Category == Category)
		{
			Result.Add(Installed);
		}
	}
	return Result;
}

int32 UShipUpgradeComponent::GetTotalUpgradeValue() const
{
	int32 TotalValue = 0;
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (Installed.Upgrade)
		{
			TotalValue += Installed.Upgrade->Requirements.CreditCost * Installed.StackCount;
		}
	}
	return TotalValue;
}

FInstalledUpgrade* UShipUpgradeComponent::FindInstalledUpgrade(FName UpgradeID)
{
	return InstalledUpgrades.FindByPredicate([UpgradeID](const FInstalledUpgrade& Entry)
	{
		return Entry.Upgrade && Entry.Upgrade->UpgradeID == UpgradeID;
	});
}

const FInstalledUpgrade* UShipUpgradeComponent::FindInstalledUpgrade(FName UpgradeID) const
{
	return InstalledUpgrades.FindByPredicate([UpgradeID](const FInstalledUpgrade& Entry)
	{
		return Entry.Upgrade && Entry.Upgrade->UpgradeID == UpgradeID;
	});
}

bool UShipUpgradeComponent::HasUpgradeConflicts(UShipUpgradeDataAsset* Upgrade) const
{
	if (!Upgrade)
	{
		return false;
	}
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (Installed.Upgrade && Installed.Upgrade != Upgrade
			&& (Upgrade->MutuallyExclusiveWith.Contains(Installed.Upgrade) || Installed.Upgrade->MutuallyExclusiveWith.Contains(Upgrade)))
		{
			return true;
		}
	}
	return false;
}

bool UShipUpgradeComponent::CheckUpgradeRequirements(UShipUpgradeDataAsset* Upgrade, FText& OutReason) const
{
	if (!Upgrade)
	{
		return false;
	}

	const FShipUpgradeRequirement& Req = Upgrade->Requirements;

	for (UShipUpgradeDataAsset* PrereqUpgrade : Req.PrerequisiteUpgrades)
	{
		if (PrereqUpgrade && !IsUpgradeInstalled(PrereqUpgrade->UpgradeID))
		{
			OutReason = FText::Format(FText::FromString(TEXT("Requires {0}")), PrereqUpgrade->DisplayName);
			return false;
		}
	}

	// Credits are not checked here: they come out of the ship's UPlayerTraderComponent
	// wallet when the upgrade is bought (CanPurchaseUpgrade/PurchaseUpgrade).

	if (Req.RequiredPlayerLevel > 1)
	{
		const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
		const UPlayerProgressionComponent* Progression = PlayerPawn ? PlayerPawn->FindComponentByClass<UPlayerProgressionComponent>() : nullptr;
		if (Progression && !Progression->MeetsLevelRequirement(Req.RequiredPlayerLevel))
		{
			OutReason = FText::Format(FText::FromString(TEXT("Requires level {0}")), FText::AsNumber(Req.RequiredPlayerLevel));
			return false;
		}
	}

	// RequiredMaterials are not enforced yet (no inventory hook for upgrades).
	return true;
}
