#include "Player/AdastreaSaveGame.h"

UAdastreaSaveGame::UAdastreaSaveGame()
	: SaveSlotName("DefaultSlot")
	, SaveVersion(CURRENT_SAVE_VERSION)
	, SaveTimestamp(FDateTime::Now())
	, PlayerName("Player")
	, CurrentLevelName("Unknown")
	, TotalPlaytimeSeconds(0.0f)
	, PlayerCredits(10000)
	, PlayerLocation(FVector::ZeroVector)
	, PlayerRotation(FRotator::ZeroRotator)
	, CurrentShipID(NAME_None)
	, DifficultyLevel(1)
	, bAutoSaveEnabled(true)
	, AutoSaveIntervalMinutes(10)
{
}

FText UAdastreaSaveGame::GetDisplayName() const
{
	return FText::Format(
		FText::FromString("{0} - Level {1}"),
		FText::FromString(PlayerName),
		FText::AsNumber(PlayerProgression.PlayerLevel)
	);
}

FText UAdastreaSaveGame::GetDescription() const
{
	FDateTime Timestamp = SaveTimestamp;
	return FText::Format(
		FText::FromString("{0} | {1} | {2}"),
		FText::FromString(CurrentLevelName),
		GetFormattedPlaytime(),
		FText::AsDateTime(Timestamp)
	);
}

FText UAdastreaSaveGame::GetFormattedPlaytime() const
{
	int32 TotalSeconds = FMath::RoundToInt(TotalPlaytimeSeconds);
	int32 Hours = TotalSeconds / 3600;
	int32 Minutes = (TotalSeconds % 3600) / 60;
	int32 Seconds = TotalSeconds % 60;

	return FText::Format(
		FText::FromString("{0}:{1}:{2}"),
		FText::AsNumber(Hours),
		FText::AsNumber(Minutes),
		FText::AsNumber(Seconds)
	);
}

bool UAdastreaSaveGame::IsCompatibleVersion() const
{
	// For now, only accept exact version match
	// In future, could implement migration logic
	return SaveVersion == CURRENT_SAVE_VERSION;
}
