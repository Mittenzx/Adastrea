// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/ShipEngineVoiceSubsystem.h"
#include "Audio/ShipEngineAudioComponent.h"
#include "Ships/SpaceshipDataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarEngineVoiceDebug(
		TEXT("adastrea.EngineVoiceDebug"),
		0,
		TEXT("Ship engine voice readout. 0 = off, 1 = on-screen line per audible engine, 2 = on-screen + log twice a second."),
		ECVF_Default);

	FAutoConsoleCommand GEngineVoiceReportCommand(
		TEXT("adastrea.EngineVoiceReport"),
		TEXT("Log the derived engine voice (size, agility, family, base pitch, whine) for every ship data asset."),
		FConsoleCommandDelegate::CreateStatic(&UShipEngineVoiceSubsystem::LogVoiceReport));

	constexpr float ArbitrationInterval = 0.2f;
	constexpr float DebugLogInterval = 0.5f;
}

bool UShipEngineVoiceSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UShipEngineVoiceSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UShipEngineVoiceSubsystem, STATGROUP_Tickables);
}

void UShipEngineVoiceSubsystem::Register(UShipEngineAudioComponent* Engine)
{
	if (Engine)
	{
		Engines.AddUnique(Engine);
	}
}

void UShipEngineVoiceSubsystem::Unregister(UShipEngineAudioComponent* Engine)
{
	Engines.RemoveAll([Engine](const TWeakObjectPtr<UShipEngineAudioComponent>& Entry)
	{
		return !Entry.IsValid() || Entry.Get() == Engine;
	});
}

bool UShipEngineVoiceSubsystem::GetListener(FVector& OutLocation) const
{
	const UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return false;
	}
	FVector Front, Right;
	PC->GetAudioListenerPosition(OutLocation, Front, Right);
	return true;
}

void UShipEngineVoiceSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ArbitrationTimer -= DeltaTime;
	if (ArbitrationTimer <= 0.0f)
	{
		ArbitrationTimer = ArbitrationInterval;
		ArbitrateAIVoices();
	}

	if (CVarEngineVoiceDebug.GetValueOnGameThread() > 0)
	{
		DrawDebug(DeltaTime);
	}
}

void UShipEngineVoiceSubsystem::ArbitrateAIVoices()
{
	Engines.RemoveAll([](const TWeakObjectPtr<UShipEngineAudioComponent>& Entry) { return !Entry.IsValid(); });

	FVector Listener;
	const bool bHaveListener = GetListener(Listener);
	const UShipEngineAudioComponent* Tuning = GetDefault<UShipEngineAudioComponent>();

	struct FCandidate
	{
		UShipEngineAudioComponent* Engine;
		float Distance;
	};
	TArray<FCandidate> Candidates;
	for (const TWeakObjectPtr<UShipEngineAudioComponent>& Entry : Engines)
	{
		UShipEngineAudioComponent* Engine = Entry.Get();
		if (!Engine || Engine->GetMode() != EShipEngineVoiceMode::AI)
		{
			continue;
		}
		const float Distance = bHaveListener ? Engine->GetDistanceToHull(Listener) : TNumericLimits<float>::Max();
		// Only ships the listener could actually hear compete for a voice.
		if (Distance < Engine->AISilentRange)
		{
			Candidates.Add({ Engine, Distance });
		}
		else
		{
			Engine->SetAIVoiceGranted(false);
		}
	}

	Candidates.Sort([](const FCandidate& A, const FCandidate& B) { return A.Distance < B.Distance; });
	for (int32 Index = 0; Index < Candidates.Num(); ++Index)
	{
		Candidates[Index].Engine->SetAIVoiceGranted(Index < Tuning->MaxAIVoices);
	}
}

void UShipEngineVoiceSubsystem::DrawDebug(float DeltaTime)
{
	FVector Listener;
	const bool bHaveListener = GetListener(Listener);

	const bool bLog = CVarEngineVoiceDebug.GetValueOnGameThread() >= 2;
	DebugLogTimer -= DeltaTime;
	const bool bLogNow = bLog && DebugLogTimer <= 0.0f;
	if (bLogNow)
	{
		DebugLogTimer = DebugLogInterval;
	}

	float NearestAI = TNumericLimits<float>::Max();
	FString NearestAIName;
	int32 Line = 0;
	for (const TWeakObjectPtr<UShipEngineAudioComponent>& Entry : Engines)
	{
		const UShipEngineAudioComponent* Engine = Entry.Get();
		if (!Engine)
		{
			continue;
		}
		const float Distance = bHaveListener ? Engine->GetDistanceToHull(Listener) : -1.0f;
		const bool bAI = Engine->GetMode() == EShipEngineVoiceMode::AI;
		if (bAI && Distance >= 0.0f && Distance < NearestAI)
		{
			NearestAI = Distance;
			NearestAIName = GetNameSafe(Engine->GetOwner());
		}
		if (bAI && !Engine->IsAudible())
		{
			continue;
		}

		const FString Text = Engine->GetDebugLine(bAI ? Distance : -1.0f);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(static_cast<uint64>(0xE11E0000 + Line), 0.0f, bAI ? FColor::Orange : FColor::Cyan, Text);
		}
		if (bLogNow)
		{
			UE_LOG(LogAdastreaEngineVoice, Log, TEXT("[EngineVoice] %s"), *Text);
		}
		++Line;
	}

	const FString Summary = NearestAIName.IsEmpty()
		? FString(TEXT("[EngineVoice] no AI ships"))
		: FString::Printf(TEXT("[EngineVoice] nearest AI hull: %s at %.1f m (full < %.0f m, silent >= %.0f m)"),
			*NearestAIName, NearestAI / 100.0f,
			GetDefault<UShipEngineAudioComponent>()->AIFullVolumeRange / 100.0f,
			GetDefault<UShipEngineAudioComponent>()->AISilentRange / 100.0f);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(0xE11E0000 + Line), 0.0f, FColor::White, Summary);
	}
	if (bLogNow)
	{
		UE_LOG(LogAdastreaEngineVoice, Log, TEXT("%s"), *Summary);
	}
}

void UShipEngineVoiceSubsystem::LogVoiceReport()
{
	IAssetRegistry& Registry = FAssetRegistryModule::GetRegistry();
	TArray<FAssetData> Assets;
	Registry.GetAssetsByClass(USpaceshipDataAsset::StaticClass()->GetClassPathName(), Assets, /*bSearchSubClasses*/ true);

	struct FRow
	{
		FString Name;
		const USpaceshipDataAsset* Data;
		FShipEngineVoice Voice;
		float SizeScore;
		float AgilityScore;
	};
	TArray<FRow> Rows;
	for (const FAssetData& Asset : Assets)
	{
		const USpaceshipDataAsset* Data = Cast<USpaceshipDataAsset>(Asset.GetAsset());
		if (!Data)
		{
			continue;
		}
		Rows.Add({ Asset.AssetName.ToString(), Data, UShipEngineAudioComponent::DeriveVoice(Data),
			UShipEngineAudioComponent::ComputeSizeScore(Data->HullStrength, Data->CargoCapacity),
			UShipEngineAudioComponent::ComputeAgilityScore(Data->Acceleration, static_cast<float>(Data->Maneuverability)) });
	}
	Rows.Sort([](const FRow& A, const FRow& B) { return A.SizeScore < B.SizeScore; });

	const UShipEngineAudioComponent* Tuning = GetDefault<UShipEngineAudioComponent>();
	UE_LOG(LogAdastreaEngineVoice, Display, TEXT("EngineVoiceReport: %d ship data assets (size bounds %.2f..%.2f, agility bounds %.2f..%.2f)"),
		Rows.Num(), Tuning->SizeScoreMin, Tuning->SizeScoreMax, Tuning->AgilityScoreMin, Tuning->AgilityScoreMax);
	UE_LOG(LogAdastreaEngineVoice, Display, TEXT("| %-32s | %7s | %6s | %5s | %3s | %5s | %5s | %5s | %5s | %-7s | %5s | %5s | %-3s |"),
		TEXT("Ship"), TEXT("Hull"), TEXT("Cargo"), TEXT("Accel"), TEXT("Man"), TEXT("SzRaw"), TEXT("Size"), TEXT("AgRaw"), TEXT("Agil"),
		TEXT("Family"), TEXT("Pitch"), TEXT("Whine"), TEXT("Ovr"));

	float MinSize = TNumericLimits<float>::Max(), MaxSize = -TNumericLimits<float>::Max();
	float MinAgility = TNumericLimits<float>::Max(), MaxAgility = -TNumericLimits<float>::Max();
	for (const FRow& Row : Rows)
	{
		MinSize = FMath::Min(MinSize, Row.SizeScore);
		MaxSize = FMath::Max(MaxSize, Row.SizeScore);
		MinAgility = FMath::Min(MinAgility, Row.AgilityScore);
		MaxAgility = FMath::Max(MaxAgility, Row.AgilityScore);
		UE_LOG(LogAdastreaEngineVoice, Display, TEXT("| %-32s | %7.0f | %6.0f | %5.0f | %3d | %5.2f | %5.2f | %5.2f | %5.2f | %-7s | %5.2f | %5.2f | %-3s |"),
			*Row.Name, Row.Data->HullStrength, Row.Data->CargoCapacity, Row.Data->Acceleration, Row.Data->Maneuverability,
			Row.SizeScore, Row.Voice.Size, Row.AgilityScore, Row.Voice.Agility,
			UShipEngineAudioComponent::FamilyName(Row.Voice.Family), Row.Voice.BasePitch, Row.Voice.WhineAmount,
			Row.Voice.bFromOverride ? TEXT("yes") : TEXT("-"));
	}
	if (Rows.Num() > 0)
	{
		UE_LOG(LogAdastreaEngineVoice, Display, TEXT("EngineVoiceReport: roster size score %.2f..%.2f, agility score %.2f..%.2f"),
			MinSize, MaxSize, MinAgility, MaxAgility);
	}
}
