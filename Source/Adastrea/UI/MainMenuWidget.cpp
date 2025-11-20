#include "UI/MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AdastreaLog.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, GameplayLevelName(TEXT("TestLevel"))
	, bCheckForExistingSave(true)
	, VersionText(FText::FromString(TEXT("v0.1.0 Alpha")))
	, bIsMenuVisible(false)
{
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Initialize the menu when constructed
	InitializeMenu();
	
	UE_LOG(LogAdastrea, Log, TEXT("MainMenuWidget: Menu constructed"));
}

void UMainMenuWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	UE_LOG(LogAdastrea, Log, TEXT("MainMenuWidget: Menu destructed"));
}

void UMainMenuWidget::InitializeMenu_Implementation()
{
	// Set menu as visible
	bIsMenuVisible = true;
	
	// Show mouse cursor for menu interaction
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		
		// Set input mode to UI only
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("MainMenuWidget: Menu initialized"));
}

void UMainMenuWidget::OnStartGameClicked_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("MainMenuWidget: Start Game clicked"));
	
	// Check for existing save if enabled
	if (bCheckForExistingSave)
	{
		// This can be extended in Blueprint to check for saves
		// For now, just start a new game
	}
	
	// Transition to gameplay level
	TransitionToGameplay(GameplayLevelName);
}

void UMainMenuWidget::OnSettingsClicked_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("MainMenuWidget: Settings clicked"));
	
	// Blueprint implementation can override this to show settings menu
	// Default implementation does nothing - settings menu not yet implemented
}

void UMainMenuWidget::OnQuitClicked_Implementation()
{
	UE_LOG(LogAdastrea, Log, TEXT("MainMenuWidget: Quit clicked"));
	
	// Show confirmation dialog (can be overridden in Blueprint)
	ShowQuitConfirmation();
}

void UMainMenuWidget::UpdateVersionText(const FText& NewVersionText)
{
	VersionText = NewVersionText;
	UE_LOG(LogAdastrea, Log, TEXT("MainMenuWidget: Version text updated to: %s"), *VersionText.ToString());
}

void UMainMenuWidget::SetMenuVisibility(bool bVisible)
{
	bIsMenuVisible = bVisible;
	
	if (bVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}

FString UMainMenuWidget::GetGameplayLevelName() const
{
	return GameplayLevelName;
}

void UMainMenuWidget::TransitionToGameplay_Implementation(const FString& LevelName)
{
	UE_LOG(LogAdastrea, Log, TEXT("MainMenuWidget: Transitioning to level: %s"), *LevelName);
	
	// Remove this widget from viewport before loading new level
	RemoveFromParent();
	
	// Reset input mode before transitioning
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
	
	// Load the gameplay level
	if (!LevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*LevelName));
	}
	else
	{
		UE_LOG(LogAdastrea, Warning, TEXT("MainMenuWidget: Cannot transition - level name is empty"));
	}
}

void UMainMenuWidget::ShowQuitConfirmation_Implementation()
{
	// Default implementation quits immediately
	// Override in Blueprint to show confirmation dialog
	
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}
