#include "UI/AdastreaHUDWidget.h"
#include "Ships/Spaceship.h"
#include "GameFramework/PlayerController.h"
#include "Player/AdastreaGameInstance.h"
#include "Kismet/GameplayStatics.h"

UAdastreaHUDWidget::UAdastreaHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HealthPercent(1.0f)
	, ShieldPercent(1.0f)
	, CurrentSpeedValue(0.0f)
	, bHasTarget(false)
	, ControlledSpaceship(nullptr)
	, CurrentPlayerName(FText::FromString("Player"))
	, CurrentPlayerLevel(1)
	, CurrentPlayerCredits(0)
	, CurrentShipName(FText::FromString("Ship"))
	, CurrentShipClass(FText::FromString("Unknown"))
	, ShipIntegrityPercent(1.0f)
	, WeaponAimPosition(FVector2D(0.5f, 0.5f))
	, bAimCrosshairVisible(true)
{
}

void UAdastreaHUDWidget::InitializeHUD_Implementation()
{
	// Default implementation - override in Blueprint
	HealthPercent = 1.0f;
	ShieldPercent = 1.0f;
	CurrentSpeedValue = 0.0f;
	bHasTarget = false;
}

void UAdastreaHUDWidget::UpdateHealth_Implementation(float CurrentHealth, float MaxHealth)
{
	if (MaxHealth > 0.0f)
	{
		HealthPercent = FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
	}
	else
	{
		HealthPercent = 0.0f;
	}
}

void UAdastreaHUDWidget::UpdateShield_Implementation(float CurrentShield, float MaxShield)
{
	if (MaxShield > 0.0f)
	{
		ShieldPercent = FMath::Clamp(CurrentShield / MaxShield, 0.0f, 1.0f);
	}
	else
	{
		ShieldPercent = 0.0f;
	}
}

void UAdastreaHUDWidget::UpdateSpeed_Implementation(float CurrentSpeed, float MaxSpeed)
{
	CurrentSpeedValue = CurrentSpeed;
}

void UAdastreaHUDWidget::UpdateTargetInfo_Implementation(const FText& TargetName, float TargetDistance, bool bIsHostile)
{
	bHasTarget = true;
	// Blueprint implementation handles display
}

void UAdastreaHUDWidget::ClearTargetInfo_Implementation()
{
	bHasTarget = false;
	// Blueprint implementation handles clearing display
}

void UAdastreaHUDWidget::ShowAlert_Implementation(const FText& Message, float Duration, bool bIsWarning)
{
	// Blueprint implementation handles message display
}

void UAdastreaHUDWidget::UpdateWeaponStatus_Implementation(int32 WeaponIndex, float CurrentAmmo, float MaxAmmo)
{
	// Blueprint implementation handles weapon status display
}

void UAdastreaHUDWidget::ToggleHUDVisibility(bool bVisible)
{
	if (bVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAdastreaHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Update HUD based on current game state
	UpdateHUDFromGameState_Implementation(InDeltaTime);
	
	// Update weapon aim crosshair position based on mouse location
	if (bAimCrosshairVisible)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			float MouseX, MouseY;
			if (PC->GetMousePosition(MouseX, MouseY))
			{
				// Get viewport size
				int32 ViewportSizeX, ViewportSizeY;
				PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
				
				if (ViewportSizeX > 0 && ViewportSizeY > 0)
				{
					// Convert mouse position to normalized screen coordinates (0-1)
					FVector2D NormalizedPosition;
					NormalizedPosition.X = MouseX / static_cast<float>(ViewportSizeX);
					NormalizedPosition.Y = MouseY / static_cast<float>(ViewportSizeY);
					
					// Only update if position changed (avoid unnecessary function calls)
					if (!NormalizedPosition.Equals(WeaponAimPosition, 0.001f))
					{
						UpdateAimCrosshair(NormalizedPosition);
					}
				}
			}
		}
	}
}

void UAdastreaHUDWidget::UpdateHUDFromGameState_Implementation(float DeltaTime)
{
	// Get current spaceship
	ControlledSpaceship = GetControlledSpaceship();
	
	// Blueprint can override this to implement custom auto-update logic
}

ASpaceship* UAdastreaHUDWidget::GetControlledSpaceship() const
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		return Cast<ASpaceship>(PC->GetPawn());
	}
	return nullptr;
}

// ====================
// PLAYER INFO DISPLAY IMPLEMENTATIONS
// ====================

void UAdastreaHUDWidget::UpdatePlayerName_Implementation(const FText& PlayerName)
{
	CurrentPlayerName = PlayerName;
	// Blueprint implementation handles visual display
}

void UAdastreaHUDWidget::UpdatePlayerLevel_Implementation(int32 Level)
{
	CurrentPlayerLevel = Level;
	// Blueprint implementation handles visual display
}

void UAdastreaHUDWidget::UpdatePlayerCredits_Implementation(int32 Credits)
{
	CurrentPlayerCredits = Credits;
	// Blueprint implementation handles visual display
}

void UAdastreaHUDWidget::UpdatePlayerInfo(const FText& PlayerName, int32 Level, int32 Credits)
{
	UpdatePlayerName(PlayerName);
	UpdatePlayerLevel(Level);
	UpdatePlayerCredits(Credits);
}

// ====================
// SHIP INFO DISPLAY IMPLEMENTATIONS
// ====================

void UAdastreaHUDWidget::UpdateShipName_Implementation(const FText& ShipName)
{
	CurrentShipName = ShipName;
	// Blueprint implementation handles visual display
}

void UAdastreaHUDWidget::UpdateShipClass_Implementation(const FText& ShipClass)
{
	CurrentShipClass = ShipClass;
	// Blueprint implementation handles visual display
}

void UAdastreaHUDWidget::UpdateShipIntegrity_Implementation(float CurrentIntegrity, float MaxIntegrity)
{
	if (MaxIntegrity > 0.0f)
	{
		ShipIntegrityPercent = FMath::Clamp(CurrentIntegrity / MaxIntegrity, 0.0f, 1.0f);
	}
	else
	{
		ShipIntegrityPercent = 0.0f;
	}
	// Blueprint implementation handles visual display
}

void UAdastreaHUDWidget::UpdateShipInfo(const FText& ShipName, const FText& ShipClass, float CurrentIntegrity, float MaxIntegrity)
{
	UpdateShipName(ShipName);
	UpdateShipClass(ShipClass);
	UpdateShipIntegrity(CurrentIntegrity, MaxIntegrity);
}

void UAdastreaHUDWidget::RefreshPlayerInfo()
{
	// Get game instance to access player data
	UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GameInstance)
	{
		// Get player credits from game instance
		int32 Credits = GameInstance->GetPlayerCredits();
		
		// Try to get more detailed player info from save game if available
		// For now, use defaults for name and level
		FText PlayerName = FText::FromString("Captain");
		int32 PlayerLevel = 1;
		
		// Update the displays
		UpdatePlayerInfo(PlayerName, PlayerLevel, Credits);
	}
}

void UAdastreaHUDWidget::RefreshShipInfo()
{
	// Get the currently controlled spaceship
	ASpaceship* Ship = GetControlledSpaceship();
	if (Ship)
	{
		// Get ship name and class from the ship
		FText ShipName = Ship->GetShipName();
		FText ShipClass = Ship->GetShipClass();
		
		// Get hull integrity values
		float CurrentIntegrity = Ship->GetCurrentHullIntegrity();
		float MaxIntegrity = Ship->GetMaxHullIntegrity();
		
		// Update the displays
		UpdateShipInfo(ShipName, ShipClass, CurrentIntegrity, MaxIntegrity);
	}
}

// ====================
// WEAPON AIM CROSSHAIR IMPLEMENTATIONS
// ====================

void UAdastreaHUDWidget::UpdateAimCrosshair_Implementation(FVector2D ScreenPosition)
{
	// Clamp screen position to valid range (0-1)
	WeaponAimPosition.X = FMath::Clamp(ScreenPosition.X, 0.0f, 1.0f);
	WeaponAimPosition.Y = FMath::Clamp(ScreenPosition.Y, 0.0f, 1.0f);
	
	// Blueprint implementation handles visual update
}

void UAdastreaHUDWidget::SetAimCrosshairVisible_Implementation(bool bVisible)
{
	bAimCrosshairVisible = bVisible;
	
	// Blueprint implementation handles visibility change
}
