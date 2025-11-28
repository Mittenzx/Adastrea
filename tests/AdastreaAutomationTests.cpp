// Adastrea - C++ Automation Tests
// 
// This file contains automation tests for the Adastrea project using
// Unreal Engine's Automation Testing Framework.
//
// To run these tests:
// 1. Open the Unreal Editor
// 2. Go to Window > Developer Tools > Session Frontend
// 3. Navigate to Automation tab
// 4. Filter for "Adastrea" and run the tests
//
// For command-line execution:
// UnrealEditor-Cmd.exe "path/to/Adastrea.uproject" -ExecCmds="Automation RunTests Adastrea" -unattended

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// =============================================================================
// SPACESHIP DATA ASSET TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpaceshipDataAssetBasicTest, 
	"Adastrea.DataAssets.Spaceship.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSpaceshipDataAssetBasicTest::RunTest(const FString& Parameters)
{
	// Test that SpaceshipDataAsset class exists and can be loaded
	UClass* SpaceshipDataAssetClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.SpaceshipDataAsset"));
	TestNotNull(TEXT("SpaceshipDataAsset class should exist"), SpaceshipDataAssetClass);
	
	return true;
}

// =============================================================================
// FACTION DATA ASSET TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFactionDataAssetBasicTest,
	"Adastrea.DataAssets.Faction.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFactionDataAssetBasicTest::RunTest(const FString& Parameters)
{
	// Test that FactionDataAsset class exists
	UClass* FactionDataAssetClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.FactionDataAsset"));
	TestNotNull(TEXT("FactionDataAsset class should exist"), FactionDataAssetClass);
	
	return true;
}

// =============================================================================
// PERSONNEL DATA ASSET TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPersonnelDataAssetBasicTest,
	"Adastrea.DataAssets.Personnel.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPersonnelDataAssetBasicTest::RunTest(const FString& Parameters)
{
	// Test that PersonnelDataAsset class exists
	UClass* PersonnelDataAssetClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.PersonnelDataAsset"));
	TestNotNull(TEXT("PersonnelDataAsset class should exist"), PersonnelDataAssetClass);
	
	return true;
}

// =============================================================================
// ACHIEVEMENT SYSTEM TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementManagerBasicTest,
	"Adastrea.Systems.Achievement.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementManagerBasicTest::RunTest(const FString& Parameters)
{
	// Test that AchievementManagerSubsystem class exists
	UClass* AchievementManagerClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.AchievementManagerSubsystem"));
	TestNotNull(TEXT("AchievementManagerSubsystem class should exist"), AchievementManagerClass);
	
	return true;
}

// =============================================================================
// QUEST SYSTEM TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FQuestManagerBasicTest,
	"Adastrea.Systems.Quest.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FQuestManagerBasicTest::RunTest(const FString& Parameters)
{
	// Test that QuestManagerSubsystem class exists
	UClass* QuestManagerClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.QuestManagerSubsystem"));
	TestNotNull(TEXT("QuestManagerSubsystem class should exist"), QuestManagerClass);
	
	return true;
}

// =============================================================================
// AUDIO SYSTEM TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioComponentBasicTest,
	"Adastrea.Components.Audio.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAudioComponentBasicTest::RunTest(const FString& Parameters)
{
	// Test that AdastreaAudioComponent class exists
	UClass* AudioComponentClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.AdastreaAudioComponent"));
	TestNotNull(TEXT("AdastreaAudioComponent class should exist"), AudioComponentClass);
	
	return true;
}

// =============================================================================
// INVENTORY SYSTEM TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryComponentBasicTest,
	"Adastrea.Components.Inventory.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInventoryComponentBasicTest::RunTest(const FString& Parameters)
{
	// Test that InventoryComponent class exists
	UClass* InventoryComponentClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.InventoryComponent"));
	TestNotNull(TEXT("InventoryComponent class should exist"), InventoryComponentClass);
	
	return true;
}

// =============================================================================
// PLAYER UNLOCK SYSTEM TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerUnlockComponentBasicTest,
	"Adastrea.Components.PlayerUnlock.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlayerUnlockComponentBasicTest::RunTest(const FString& Parameters)
{
	// Test that PlayerUnlockComponent class exists
	UClass* UnlockComponentClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.PlayerUnlockComponent"));
	TestNotNull(TEXT("PlayerUnlockComponent class should exist"), UnlockComponentClass);
	
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
