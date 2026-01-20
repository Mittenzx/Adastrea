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
	, CachedPlayerController(nullptr)
	, CurrentPlayerName(FText::FromString("Player"))
	, CurrentPlayerLevel(1)
	, CurrentPlayerCredits(0)
	, CurrentShipName(FText::FromString("Ship"))
	, CurrentShipClass(FText::FromString("Unknown"))
	, ShipIntegrityPercent(1.0f)
	, WeaponAimPosition(FVector2D(0.5f, 0.5f))
	, bAimCrosshairVisible(true)
	, FlightMousePosition(FVector2D(0.5f, 0.5f))
	, ShipRotationIndicator(FVector2D(0.5f, 0.5f))
	, CurrentRotationSpeed(0.0f)
	, bMouseInDeadzone(true)
	, bFlightCrosshairVisible(true)
	, DeadzoneRadiusPercent(0.05f)
	, MaxRadiusPercent(0.4f)
	, bShowDeadzoneCircle(false)
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
	
	// Cache player controller reference for performance
	if (!CachedPlayerController)
	{
		CachedPlayerController = GetOwningPlayer();
	}
	
	if (CachedPlayerController)
	{
		float MouseX, MouseY;
		if (CachedPlayerController->GetMousePosition(MouseX, MouseY))
		{
			// Get viewport size
			int32 ViewportSizeX, ViewportSizeY;
			CachedPlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
			
			if (ViewportSizeX > 0 && ViewportSizeY > 0)
			{
				// Convert mouse position to normalized screen coordinates (0-1)
				FVector2D NormalizedPosition;
				NormalizedPosition.X = MouseX / static_cast<float>(ViewportSizeX);
				NormalizedPosition.Y = MouseY / static_cast<float>(ViewportSizeY);
				
				// Update weapon aim crosshair position
				if (bAimCrosshairVisible)
				{
					// Only update if position changed (avoid unnecessary function calls)
					if (!NormalizedPosition.Equals(WeaponAimPosition, 0.001f))
					{
						UpdateAimCrosshair(NormalizedPosition);
					}
				}
				
				// Update X4-style flight crosshair system
				if (bFlightCrosshairVisible)
				{
					// Use cached controlled spaceship where possible to avoid repeated Cast operations
					ASpaceship* Ship = ControlledSpaceship;
					if (!IsValid(Ship))
					{
						Ship = GetControlledSpaceship();
						ControlledSpaceship = Ship;
					}
					
					if (Ship && Ship->bUseMousePositionFlight)
					{
						// Calculate center of screen
						float CenterX = ViewportSizeX * 0.5f;
						float CenterY = ViewportSizeY * 0.5f;
						
						// Calculate distance from center in pixels
						float DeltaX = MouseX - CenterX;
						float DeltaY = MouseY - CenterY;
						float DistanceFromCenter = FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
						
						// Get deadzone and max radius from ship
						float DeadzoneRadius = Ship->MouseDeadzoneRadius;
						float MaxRadius = Ship->MouseMaxRadius;
						
						// Calculate deadzone state
						bool bInDeadzone = (DistanceFromCenter < DeadzoneRadius);
						
						// Calculate rotation speed (0-1 normalized)
						float RotationSpeed = 0.0f;
						if (!bInDeadzone && MaxRadius > DeadzoneRadius)
						{
							float EffectiveDistance = DistanceFromCenter - DeadzoneRadius;
							float MaxEffectiveDistance = MaxRadius - DeadzoneRadius;
							
							// Safety check to prevent division by zero or very small numbers
							const float SafeMaxEffectiveDistance = FMath::Max(MaxEffectiveDistance, KINDA_SMALL_NUMBER);
							RotationSpeed = FMath::Clamp(EffectiveDistance / SafeMaxEffectiveDistance, 0.0f, 1.0f);
						}
						
						// Calculate ship rotation direction indicator
						// This shows where the ship is actually turning towards
						// For now, we'll use the direction from center to mouse as a simple approximation
						// In a more advanced implementation, this could show the ship's actual rotation vector
						FVector2D RotationDirection = NormalizedPosition;
						
						// If in deadzone, center the rotation indicator
						if (bInDeadzone)
						{
							RotationDirection = FVector2D(0.5f, 0.5f);
						}
						
						// Update the flight crosshair
						UpdateFlightCrosshair(NormalizedPosition, RotationDirection, RotationSpeed, bInDeadzone);
						
						// Update deadzone visualization with ship's parameters
						if (ViewportSizeX > 0)
						{
							float DeadzonePercent = DeadzoneRadius / (ViewportSizeX * 0.5f);
							float LocalMaxRadiusPercent = MaxRadius / (ViewportSizeX * 0.5f);
							UpdateDeadzoneVisualization(DeadzonePercent, LocalMaxRadiusPercent);
						}
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

// ====================
// X4-STYLE FLIGHT CROSSHAIR IMPLEMENTATIONS
// ====================

void UAdastreaHUDWidget::UpdateFlightCrosshair_Implementation(FVector2D MouseScreenPosition, FVector2D ShipRotationDirection, float RotationSpeed, bool bInDeadzone)
{
	// Update mouse position (clamped to 0-1 range)
	FlightMousePosition.X = FMath::Clamp(MouseScreenPosition.X, 0.0f, 1.0f);
	FlightMousePosition.Y = FMath::Clamp(MouseScreenPosition.Y, 0.0f, 1.0f);
	
	// Update ship rotation direction indicator (clamped to 0-1 range)
	ShipRotationIndicator.X = FMath::Clamp(ShipRotationDirection.X, 0.0f, 1.0f);
	ShipRotationIndicator.Y = FMath::Clamp(ShipRotationDirection.Y, 0.0f, 1.0f);
	
	// Update rotation speed (clamped to 0-1 range)
	CurrentRotationSpeed = FMath::Clamp(RotationSpeed, 0.0f, 1.0f);
	
	// Update deadzone state
	bMouseInDeadzone = bInDeadzone;
	
	// Blueprint implementation handles visual rendering:
	// - Draw mouse cursor indicator (where player wants to turn)
	// - Draw ship direction indicator (where ship is actually turning)
	// - Draw line/arrow connecting them (showing rotation intent)
	// - Color/size based on rotation speed
	// - Different appearance when in deadzone
}

void UAdastreaHUDWidget::SetFlightCrosshairVisible_Implementation(bool bVisible)
{
	bFlightCrosshairVisible = bVisible;
	
	// Blueprint implementation handles visibility change
}

void UAdastreaHUDWidget::UpdateDeadzoneVisualization_Implementation(float DeadzoneRadiusPercentParam, float MaxRadiusPercentParam)
{
	// Update deadzone radius (clamped to valid range)
	DeadzoneRadiusPercent = FMath::Clamp(DeadzoneRadiusPercentParam, 0.0f, 1.0f);
	
	// Update max effective radius (clamped to valid range)
	MaxRadiusPercent = FMath::Clamp(MaxRadiusPercentParam, 0.0f, 1.0f);
	
	// Blueprint implementation handles visual rendering:
	// - Draw inner circle at DeadzoneRadiusPercent (no rotation area)
	// - Draw outer circle at MaxRadiusPercent (100% rotation speed)
	// - Visual gradient between them showing rotation speed zones
}

void UAdastreaHUDWidget::ShowDeadzoneVisualization(bool bShow)
{
	bShowDeadzoneCircle = bShow;
	
	// Blueprint implementation handles visibility toggle
}
