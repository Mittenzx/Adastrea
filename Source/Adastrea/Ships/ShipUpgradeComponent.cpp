#include "Ships/ShipUpgradeComponent.h"
#include "Player/PlayerProgressionComponent.h"
#include "Player/PlayerReputationComponent.h"
#include "Player/AdastreaGameInstance.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"

UShipUpgradeComponent::UShipUpgradeComponent()
	: ShipTypeID(NAME_None)
	, MaxUpgradeSlots(20)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShipUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: Initialized with %d upgrade slots"), MaxUpgradeSlots);
}

bool UShipUpgradeComponent::InstallUpgrade(UShipUpgradeDataAsset* Upgrade, bool bIgnoreRequirements)
{
	if (!Upgrade)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipUpgradeComponent: Cannot install null upgrade"));
		return false;
	}

	// Check if can install
	FText Reason;
	if (!bIgnoreRequirements && !CanInstallUpgrade(Upgrade, Reason))
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipUpgradeComponent: Cannot install upgrade: %s"), *Reason.ToString());
		OnUpgradeInstallFailed.Broadcast(Upgrade, Reason);
		return false;
	}

	// Check if already installed
	FInstalledUpgrade* Existing = FindInstalledUpgrade(Upgrade->UpgradeID);
	if (Existing)
	{
		// If not unique, increase stack count
		if (!Upgrade->bIsUnique && Existing->StackCount < Upgrade->MaxStackCount)
		{
			Existing->StackCount++;
			UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: Increased upgrade stack: %s (x%d)"),
				*Upgrade->UpgradeID.ToString(), Existing->StackCount);
			OnUpgradeInstalled.Broadcast(Upgrade, Existing->StackCount);
			return true;
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("ShipUpgradeComponent: Upgrade already installed at max stacks: %s"),
				*Upgrade->UpgradeID.ToString());
			OnUpgradeInstallFailed.Broadcast(Upgrade, FText::FromString("Already installed"));
			return false;
		}
	}

	// Add new upgrade
	FInstalledUpgrade NewUpgrade(Upgrade, 1);
	InstalledUpgrades.Add(NewUpgrade);

	UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: Installed upgrade: %s (%s)"),
		*Upgrade->UpgradeID.ToString(), *Upgrade->DisplayName.ToString());

	OnUpgradeInstalled.Broadcast(Upgrade, 1);

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

	// Remove upgrade
	InstalledUpgrades.RemoveAll([UpgradeID](const FInstalledUpgrade& Upgrade)
	{
		return Upgrade.Upgrade && Upgrade.Upgrade->UpgradeID == UpgradeID;
	});

	UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: Uninstalled upgrade: %s"), *UpgradeID.ToString());
	OnUpgradeUninstalled.Broadcast(UpgradeID);

	return true;
}

bool UShipUpgradeComponent::CanInstallUpgrade(UShipUpgradeDataAsset* Upgrade, FText& OutReason) const
{
	if (!Upgrade)
	{
		OutReason = FText::FromString("Invalid upgrade");
		return false;
	}

	// Check slot availability
	if (InstalledUpgrades.Num() >= MaxUpgradeSlots)
	{
		// Check if we're stacking an existing upgrade
		const FInstalledUpgrade* Existing = FindInstalledUpgrade(Upgrade->UpgradeID);
		if (!Existing)
		{
			OutReason = FText::FromString("No upgrade slots available");
			return false;
		}
	}

	// Check ship compatibility
	if (!Upgrade->IsCompatibleWithShipType(ShipTypeID))
	{
		OutReason = FText::FromString("Incompatible with this ship type");
		return false;
	}

	// Check conflicts
	if (HasUpgradeConflicts(Upgrade))
	{
		OutReason = FText::FromString("Conflicts with installed upgrade");
		return false;
	}

	// Check requirements
	if (!CheckUpgradeRequirements(Upgrade, OutReason))
	{
		return false;
	}

	return true;
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
	InstalledUpgrades.Empty();
	UE_LOG(LogAdastrea, Log, TEXT("ShipUpgradeComponent: All upgrades uninstalled"));
}

float UShipUpgradeComponent::GetStatModifier(FName StatName, float BaseValue) const
{
	float ModifiedValue = BaseValue;

	// Apply all installed upgrades
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (Installed.Upgrade)
		{
			// Apply upgrade bonus for each stack
			for (int32 i = 0; i < Installed.StackCount; ++i)
			{
				ModifiedValue = Installed.Upgrade->CalculateStatBonus(StatName, ModifiedValue);
			}
		}
	}

	return ModifiedValue;
}

float UShipUpgradeComponent::GetStatBonusPercentage(FName StatName) const
{
	// Calculate percentage difference from base value of 100
	float BaseValue = 100.0f;
	float ModifiedValue = GetStatModifier(StatName, BaseValue);
	
	return (ModifiedValue - BaseValue) / BaseValue;
}

TMap<FName, float> UShipUpgradeComponent::GetAllStatModifiers() const
{
	TMap<FName, float> Modifiers;

	// Collect all unique stat names
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (Installed.Upgrade)
		{
			for (const FShipUpgradeStatModifier& StatMod : Installed.Upgrade->StatModifiers)
			{
				if (!Modifiers.Contains(StatMod.StatName))
				{
					// Calculate cumulative modifier for this stat
					float BonusPercentage = GetStatBonusPercentage(StatMod.StatName);
					Modifiers.Add(StatMod.StatName, BonusPercentage);
				}
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

int32 UShipUpgradeComponent::GetRemainingUpgradeSlots() const
{
	return FMath::Max(0, MaxUpgradeSlots - InstalledUpgrades.Num());
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
	for (FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (Installed.Upgrade && Installed.Upgrade->UpgradeID == UpgradeID)
		{
			return &Installed;
		}
	}
	return nullptr;
}

const FInstalledUpgrade* UShipUpgradeComponent::FindInstalledUpgrade(FName UpgradeID) const
{
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (Installed.Upgrade && Installed.Upgrade->UpgradeID == UpgradeID)
		{
			return &Installed;
		}
	}
	return nullptr;
}

bool UShipUpgradeComponent::HasUpgradeConflicts(UShipUpgradeDataAsset* Upgrade) const
{
	if (!Upgrade)
	{
		return false;
	}

	// Check mutually exclusive upgrades
	for (const FInstalledUpgrade& Installed : InstalledUpgrades)
	{
		if (!Installed.Upgrade)
		{
			continue;
		}

		// Check if new upgrade conflicts with installed
		if (Upgrade->MutuallyExclusiveWith.Contains(Installed.Upgrade))
		{
			return true;
		}

		// Check if installed conflicts with new
		if (Installed.Upgrade->MutuallyExclusiveWith.Contains(Upgrade))
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

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FShipUpgradeRequirement& Req = Upgrade->Requirements;

	// Check prerequisites
	for (UShipUpgradeDataAsset* PrereqUpgrade : Req.PrerequisiteUpgrades)
	{
		if (PrereqUpgrade && !IsUpgradeInstalled(PrereqUpgrade->UpgradeID))
		{
			OutReason = FText::Format(
				FText::FromString("Requires {0}"),
				PrereqUpgrade->DisplayName
			);
			return false;
		}
	}

	// Check credits
	UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(UGameplayStatics::GetGameInstance(World));
	if (GameInstance && Req.CreditCost > 0)
	{
		if (GameInstance->GetPlayerCredits() < Req.CreditCost)
		{
			OutReason = FText::Format(
				FText::FromString("Requires {0} credits"),
				FText::AsNumber(Req.CreditCost)
			);
			return false;
		}
	}

	// Check player level
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC)
	{
		APawn* PlayerPawn = PC->GetPawn();
		if (PlayerPawn)
		{
			UPlayerProgressionComponent* ProgressionComp = PlayerPawn->FindComponentByClass<UPlayerProgressionComponent>();
			if (ProgressionComp && Req.RequiredPlayerLevel > 1)
			{
				if (!ProgressionComp->MeetsLevelRequirement(Req.RequiredPlayerLevel))
				{
					OutReason = FText::Format(
						FText::FromString("Requires level {0}"),
						FText::AsNumber(Req.RequiredPlayerLevel)
					);
					return false;
				}
			}

			// Check reputation
			if (Req.RequiredFactionID != NAME_None)
			{
				UPlayerReputationComponent* ReputationComp = PlayerPawn->FindComponentByClass<UPlayerReputationComponent>();
				if (ReputationComp)
				{
					if (!ReputationComp->IsReputationAtLeast(Req.RequiredFactionID, Req.MinimumReputation))
					{
						OutReason = FText::Format(
							FText::FromString("Requires {0} reputation with {1}"),
							FText::AsNumber(Req.MinimumReputation),
							FText::FromName(Req.RequiredFactionID)
						);
						return false;
					}
				}
			}
		}
	}

	// TODO: Check materials via inventory system

	return true;
}
