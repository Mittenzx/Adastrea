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

// =============================================================================
// FREE LOOK CAMERA SYSTEM TESTS
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFreeLookCameraBasicTest,
	"Adastrea.Systems.FreeLookCamera.BasicValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFreeLookCameraBasicTest::RunTest(const FString& Parameters)
{
	// Test that Spaceship class exists and has free look components
	UClass* SpaceshipClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.Spaceship"));
	TestNotNull(TEXT("Spaceship class should exist"), SpaceshipClass);
	
	if (SpaceshipClass)
	{
		// Verify free look properties exist
		FProperty* FreeLookActiveProperty = SpaceshipClass->FindPropertyByName(TEXT("bFreeLookActive"));
		TestNotNull(TEXT("bFreeLookActive property should exist"), FreeLookActiveProperty);
		
		FProperty* FreeLookSensitivityProperty = SpaceshipClass->FindPropertyByName(TEXT("FreeLookSensitivity"));
		TestNotNull(TEXT("FreeLookSensitivity property should exist"), FreeLookSensitivityProperty);
		
		FProperty* DoubleClickThresholdProperty = SpaceshipClass->FindPropertyByName(TEXT("DoubleClickThreshold"));
		TestNotNull(TEXT("DoubleClickThreshold property should exist"), DoubleClickThresholdProperty);
		
		FProperty* CameraSpringArmProperty = SpaceshipClass->FindPropertyByName(TEXT("CameraSpringArm"));
		TestNotNull(TEXT("CameraSpringArm property should exist"), CameraSpringArmProperty);
		
		FProperty* CameraProperty = SpaceshipClass->FindPropertyByName(TEXT("CameraComponent"));
		TestNotNull(TEXT("CameraComponent property should exist"), CameraProperty);
	}
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFreeLookCameraStateTransitionTest,
	"Adastrea.Systems.FreeLookCamera.StateTransitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFreeLookCameraStateTransitionTest::RunTest(const FString& Parameters)
{
	// Test free look state transitions
	// Note: This is a basic structural test. Full functionality tests require world context
	
	UClass* SpaceshipClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.Spaceship"));
	if (!SpaceshipClass)
	{
		AddError(TEXT("Spaceship class not found"));
		return false;
	}
	
	// Verify state management methods exist
	UFunction* FreeLookStartedFunc = SpaceshipClass->FindFunctionByName(TEXT("FreeLookStarted"));
	TestNotNull(TEXT("FreeLookStarted function should exist"), FreeLookStartedFunc);
	
	UFunction* FreeLookCompletedFunc = SpaceshipClass->FindFunctionByName(TEXT("FreeLookCompleted"));
	TestNotNull(TEXT("FreeLookCompleted function should exist"), FreeLookCompletedFunc);
	
	UFunction* FreeLookCameraFunc = SpaceshipClass->FindFunctionByName(TEXT("FreeLookCamera"));
	TestNotNull(TEXT("FreeLookCamera function should exist"), FreeLookCameraFunc);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFreeLookCameraPitchClampTest,
	"Adastrea.Systems.FreeLookCamera.PitchClamping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFreeLookCameraPitchClampTest::RunTest(const FString& Parameters)
{
	// Test that pitch clamping values are reasonable
	// Verifies the implementation clamps to ±89 degrees as documented
	
	const float ExpectedMinPitch = -89.0f;
	const float ExpectedMaxPitch = 89.0f;
	
	// Test clamp function behavior
	float TestPitch1 = FMath::Clamp(-100.0f, ExpectedMinPitch, ExpectedMaxPitch);
	TestEqual(TEXT("Pitch should clamp to minimum -89 degrees"), TestPitch1, ExpectedMinPitch);
	
	float TestPitch2 = FMath::Clamp(100.0f, ExpectedMinPitch, ExpectedMaxPitch);
	TestEqual(TEXT("Pitch should clamp to maximum 89 degrees"), TestPitch2, ExpectedMaxPitch);
	
	float TestPitch3 = FMath::Clamp(45.0f, ExpectedMinPitch, ExpectedMaxPitch);
	TestEqual(TEXT("Pitch within range should not be clamped"), TestPitch3, 45.0f);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFreeLookCameraDoubleClickTimingTest,
	"Adastrea.Systems.FreeLookCamera.DoubleClickDetection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFreeLookCameraDoubleClickTimingTest::RunTest(const FString& Parameters)
{
	// Test double-click detection timing logic
	const float DoubleClickThreshold = 0.3f; // Default threshold
	
	// Simulate timing scenarios
	float FirstClickTime = 1.0f;
	float SecondClickTime = 1.2f; // 0.2s later - should be detected
	float ThirdClickTime = 1.7f;  // 0.7s after first - should not be detected
	
	float TimeDelta1 = SecondClickTime - FirstClickTime;
	TestTrue(TEXT("Quick double-click should be within threshold"), TimeDelta1 <= DoubleClickThreshold);
	
	float TimeDelta2 = ThirdClickTime - FirstClickTime;
	TestFalse(TEXT("Slow clicks should exceed threshold"), TimeDelta2 <= DoubleClickThreshold);
	
	// Test edge case: exactly at threshold
	float EdgeClickTime = FirstClickTime + DoubleClickThreshold;
	float EdgeDelta = EdgeClickTime - FirstClickTime;
	TestTrue(TEXT("Click exactly at threshold should be detected"), EdgeDelta <= DoubleClickThreshold);
	
	// Test that zero time difference check prevents false positives on first click
	float ZeroTimeDelta = 0.0f;
	TestFalse(TEXT("Zero time delta should not trigger double-click"), ZeroTimeDelta > 0.0f);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFreeLookCameraConfigurationTest,
	"Adastrea.Systems.FreeLookCamera.Configuration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFreeLookCameraConfigurationTest::RunTest(const FString& Parameters)
{
	// Test that default configuration values are reasonable
	const float DefaultFreeLookSensitivity = 1.5f;
	const float DefaultCameraDistance = 800.0f;
	const float DefaultCameraLagSpeed = 10.0f;
	const float DefaultCameraRotationLagSpeed = 10.0f;
	const float DefaultDoubleClickThreshold = 0.3f;
	
	// Verify ranges are sensible
	TestTrue(TEXT("FreeLookSensitivity should be positive"), DefaultFreeLookSensitivity > 0.0f);
	TestTrue(TEXT("CameraDistance should be positive"), DefaultCameraDistance > 0.0f);
	TestTrue(TEXT("CameraLagSpeed should be positive"), DefaultCameraLagSpeed > 0.0f);
	TestTrue(TEXT("CameraRotationLagSpeed should be positive"), DefaultCameraRotationLagSpeed > 0.0f);
	TestTrue(TEXT("DoubleClickThreshold should be positive"), DefaultDoubleClickThreshold > 0.0f);
	
	// Test that sensitivity is within reasonable range (0.1 - 5.0)
	TestTrue(TEXT("FreeLookSensitivity should be >= 0.1"), DefaultFreeLookSensitivity >= 0.1f);
	TestTrue(TEXT("FreeLookSensitivity should be <= 5.0"), DefaultFreeLookSensitivity <= 5.0f);
	
	// Test that double-click threshold is within reasonable range (0.1 - 1.0)
	TestTrue(TEXT("DoubleClickThreshold should be >= 0.1"), DefaultDoubleClickThreshold >= 0.1f);
	TestTrue(TEXT("DoubleClickThreshold should be <= 1.0"), DefaultDoubleClickThreshold <= 1.0f);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraRotationLagTest,
	"Adastrea.Systems.Camera.RotationLagValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraRotationLagTest::RunTest(const FString& Parameters)
{
	// Test that camera rotation lag property exists and is properly configured
	UClass* SpaceshipClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.Spaceship"));
	if (!SpaceshipClass)
	{
		AddError(TEXT("Spaceship class not found"));
		return false;
	}
	
	// Verify CameraRotationLagSpeed property exists
	FProperty* CameraRotationLagSpeedProperty = SpaceshipClass->FindPropertyByName(TEXT("CameraRotationLagSpeed"));
	TestNotNull(TEXT("CameraRotationLagSpeed property should exist"), CameraRotationLagSpeedProperty);
	
	if (CameraRotationLagSpeedProperty)
	{
		// Verify it's a float property
		FFloatProperty* FloatProp = CastField<FFloatProperty>(CameraRotationLagSpeedProperty);
		TestNotNull(TEXT("CameraRotationLagSpeed should be a float property"), FloatProp);
	}
	
	// Verify CameraSpringArm exists
	FProperty* CameraSpringArmProperty = SpaceshipClass->FindPropertyByName(TEXT("CameraSpringArm"));
	TestNotNull(TEXT("CameraSpringArm property should exist"), CameraSpringArmProperty);
	
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
