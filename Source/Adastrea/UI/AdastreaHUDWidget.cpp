#include "UI/AdastreaHUDWidget.h"
#include "Ships/Spaceship.h"
#include "GameFramework/PlayerController.h"

UAdastreaHUDWidget::UAdastreaHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HealthPercent(1.0f)
	, ShieldPercent(1.0f)
	, CurrentSpeedValue(0.0f)
	, bHasTarget(false)
	, ControlledSpaceship(nullptr)
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
