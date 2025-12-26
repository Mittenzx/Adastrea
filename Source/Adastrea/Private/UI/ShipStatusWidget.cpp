#include "UI/ShipStatusWidget.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Player/AdastreaPlayerController.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

UShipStatusWidget::UShipStatusWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentSpaceship(nullptr)
	, ShipDataAsset(nullptr)
	, Text_ShipName(nullptr)
	, Text_ShipClass(nullptr)
	, Text_Description(nullptr)
	, Text_CombatRating(nullptr)
	, Text_MobilityRating(nullptr)
	, Text_UtilityRating(nullptr)
	, StatsScrollBox(nullptr)
	, Button_Close(nullptr)
	, bAutoCreateMissingWidgets(true)
	, DisplayCombatRating(0.0f)
	, DisplayMobilityRating(0.0f)
	, DisplayUtilityRating(0.0f)
{
}

void UShipStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Create default UI widgets if they don't exist and auto-create is enabled
	if (bAutoCreateMissingWidgets)
	{
		CreateDefaultUIWidgets();
	}

	// Setup close button if it exists
	if (Button_Close)
	{
		Button_Close->OnClicked.AddDynamic(this, &UShipStatusWidget::OnCloseButtonClicked);
	}

	// Auto-initialize with player's spaceship if not already set
	if (!CurrentSpaceship)
	{
		InitializeShipStatus();
	}
}

void UShipStatusWidget::InitializeShipStatus_Implementation()
{
	// Get the player's controlled spaceship
	ASpaceship* PlayerShip = GetPlayerSpaceship();
	
	if (PlayerShip)
	{
		InitializeWithSpaceship(PlayerShip);
	}
	else
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: No spaceship available to display"));
	}
}

void UShipStatusWidget::InitializeWithSpaceship_Implementation(ASpaceship* InSpaceship)
{
	if (!InSpaceship)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: Cannot initialize with null spaceship"));
		return;
	}

	CurrentSpaceship = InSpaceship;
	ShipDataAsset = InSpaceship->ShipDataAsset;

	if (!ShipDataAsset)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: Spaceship has no data asset assigned"));
		return;
	}

	// Refresh all displayed data
	RefreshShipStatus();
	
	UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Initialized with spaceship %s"), *InSpaceship->GetName());
}

void UShipStatusWidget::RefreshShipStatus_Implementation()
{
	if (!CurrentSpaceship || !ShipDataAsset)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: Cannot refresh - no spaceship or data asset"));
		return;
	}

	// Update basic info
	UpdateBasicInfo(
		ShipDataAsset->ShipName,
		ShipDataAsset->ShipClass,
		ShipDataAsset->Description
	);

	// Update core stats
	UpdateCoreStats(
		CurrentSpaceship->GetCurrentHullIntegrity(),
		CurrentSpaceship->GetMaxHullIntegrity(),
		ShipDataAsset->CargoCapacity,
		ShipDataAsset->CrewRequired,
		ShipDataAsset->MaxCrew,
		ShipDataAsset->ModularPoints
	);

	// Update combat stats
	UpdateCombatStats(
		ShipDataAsset->ArmorRating,
		ShipDataAsset->ShieldStrength,
		ShipDataAsset->ShieldRechargeRate,
		ShipDataAsset->WeaponSlots,
		ShipDataAsset->WeaponPowerCapacity,
		ShipDataAsset->PointDefenseRating
	);

	// Update mobility stats
	UpdateMobilityStats(
		ShipDataAsset->MaxSpeed,
		ShipDataAsset->Acceleration,
		ShipDataAsset->Maneuverability,
		ShipDataAsset->JumpRange,
		ShipDataAsset->FuelCapacity,
		ShipDataAsset->FuelConsumptionRate
	);

	// Update utility stats
	UpdateUtilityStats(
		ShipDataAsset->SensorRange,
		ShipDataAsset->SensorResolution,
		ShipDataAsset->StealthRating,
		ShipDataAsset->RepairSystemRating,
		ShipDataAsset->ScienceRating,
		ShipDataAsset->MedicalRating
	);

	// Update operational stats
	UpdateOperationalStats(
		ShipDataAsset->PowerCapacity,
		ShipDataAsset->LifeSupportRating,
		ShipDataAsset->MaintenanceLevel,
		ShipDataAsset->HangarCapacity,
		ShipDataAsset->DroneCapacity,
		ShipDataAsset->AISystemRating
	);

	// Update advanced stats
	UpdateAdvancedStats(
		ShipDataAsset->DiplomacyRating,
		ShipDataAsset->BoardingCapability,
		ShipDataAsset->CustomizableSlots,
		ShipDataAsset->EWarRating,
		ShipDataAsset->MiningRating
	);

	// Update lore info
	UpdateLoreInfo(
		ShipDataAsset->Manufacturer,
		ShipDataAsset->YearBuilt,
		ShipDataAsset->RarityTier,
		ShipDataAsset->LoreNotes
	);

	// Calculate and update ratings
	const float CombatRating = ShipDataAsset->GetCombatRating();
	const float MobilityRating = ShipDataAsset->GetMobilityRating();
	const float UtilityRating = ShipDataAsset->GetUtilityRating();
	
	UpdateRatings(CombatRating, MobilityRating, UtilityRating);

	UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Refreshed ship status for %s"), *ShipDataAsset->ShipName.ToString());
}

void UShipStatusWidget::UpdateBasicInfo_Implementation(const FText& ShipName, const FText& ShipClass, const FText& Description)
{
	// Store for Blueprint access
	DisplayShipName = ShipName;
	DisplayShipClass = ShipClass;
	DisplayDescription = Description;

	// Update UI widgets if they exist
	if (Text_ShipName)
	{
		Text_ShipName->SetText(ShipName);
	}

	if (Text_ShipClass)
	{
		Text_ShipClass->SetText(ShipClass);
	}

	if (Text_Description)
	{
		Text_Description->SetText(Description);
	}

	// Blueprint can override to update UI elements
}

void UShipStatusWidget::UpdateCoreStats_Implementation(float CurrentHull, float MaxHull, float CargoCapacity, int32 CrewRequired, int32 MaxCrew, int32 ModularPoints)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateCombatStats_Implementation(float ArmorRating, float ShieldStrength, float ShieldRechargeRate, int32 WeaponSlots, float WeaponPower, float PointDefense)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateMobilityStats_Implementation(float MaxSpeed, float Acceleration, int32 Maneuverability, float JumpRange, float FuelCapacity, float FuelConsumption)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateUtilityStats_Implementation(float SensorRange, int32 SensorResolution, int32 StealthRating, int32 RepairRating, int32 ScienceRating, int32 MedicalRating)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateOperationalStats_Implementation(float PowerCapacity, int32 LifeSupportRating, int32 MaintenanceLevel, int32 HangarCapacity, int32 DroneCapacity, int32 AIRating)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateAdvancedStats_Implementation(int32 DiplomacyRating, int32 BoardingCapability, int32 CustomizableSlots, int32 EWarRating, int32 MiningRating)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateLoreInfo_Implementation(const FText& Manufacturer, int32 YearBuilt, const FText& RarityTier, const FText& LoreNotes)
{
	// Default implementation - Blueprint can override to update UI
}

void UShipStatusWidget::UpdateRatings_Implementation(float CombatRating, float MobilityRating, float UtilityRating)
{
	// Store for Blueprint access
	DisplayCombatRating = CombatRating;
	DisplayMobilityRating = MobilityRating;
	DisplayUtilityRating = UtilityRating;

	// Update UI widgets if they exist
	if (Text_CombatRating)
	{
		FText RatingText = FText::Format(
			FText::FromString("Combat Rating: {0}"),
			FMath::RoundToInt(CombatRating)
		);
		Text_CombatRating->SetText(RatingText);
	}

	if (Text_MobilityRating)
	{
		FText RatingText = FText::Format(
			FText::FromString("Mobility Rating: {0}"),
			FMath::RoundToInt(MobilityRating)
		);
		Text_MobilityRating->SetText(RatingText);
	}

	if (Text_UtilityRating)
	{
		FText RatingText = FText::Format(
			FText::FromString("Utility Rating: {0}"),
			FMath::RoundToInt(UtilityRating)
		);
		Text_UtilityRating->SetText(RatingText);
	}
}

ASpaceship* UShipStatusWidget::GetPlayerSpaceship() const
{
	// Get the world context
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Get the player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return nullptr;
	}

	// Try to cast to AdastreaPlayerController for GetControlledSpaceship
	AAdastreaPlayerController* AdastreaPC = Cast<AAdastreaPlayerController>(PC);
	if (AdastreaPC)
	{
		return AdastreaPC->GetControlledSpaceship();
	}

	// Fallback: Try to cast pawn directly
	return Cast<ASpaceship>(PC->GetPawn());
}

void UShipStatusWidget::CreateDefaultUIWidgets()
{
	if (!WidgetTree)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShipStatusWidget: Cannot create default widgets - WidgetTree is null"));
		return;
	}

	// Get or create root canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		if (!RootCanvas)
		{
			UE_LOG(LogAdastrea, Error, TEXT("ShipStatusWidget: Failed to create root canvas panel"));
			return;
		}
		WidgetTree->RootWidget = RootCanvas;
		UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created root canvas panel"));
	}

	// Create background border if not exists
	UBorder* Background = Cast<UBorder>(WidgetTree->FindWidget(TEXT("Background")));
	if (!Background)
	{
		Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
		if (Background && RootCanvas)
		{
			Background->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.08f, 0.95f));
			RootCanvas->AddChild(Background);
			UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(Background->Slot);
			if (BgSlot)
			{
				BgSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				BgSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				BgSlot->SetPosition(FVector2D(0.0f, 0.0f));
				BgSlot->SetSize(FVector2D(800.0f, 900.0f));
			}
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created background border"));
		}
	}

	// Create main vertical box for content layout
	UVerticalBox* MainContent = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("MainContent")));
	if (!MainContent && Background)
	{
		MainContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainContent"));
		if (MainContent)
		{
			Background->AddChild(MainContent);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created main content vertical box"));
		}
	}

	// Create ship name text if not exists
	if (!Text_ShipName && MainContent)
	{
		Text_ShipName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_ShipName"));
		if (Text_ShipName)
		{
			Text_ShipName->SetText(FText::FromString("Ship Name"));
			Text_ShipName->SetJustification(ETextJustify::Center);
			FSlateFontInfo FontInfo = Text_ShipName->GetFont();
			FontInfo.Size = 32;
			Text_ShipName->SetFont(FontInfo);
			MainContent->AddChild(Text_ShipName);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created ship name text"));
		}
	}

	// Create ship class text if not exists
	if (!Text_ShipClass && MainContent)
	{
		Text_ShipClass = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_ShipClass"));
		if (Text_ShipClass)
		{
			Text_ShipClass->SetText(FText::FromString("Ship Class"));
			Text_ShipClass->SetJustification(ETextJustify::Center);
			FSlateFontInfo FontInfo = Text_ShipClass->GetFont();
			FontInfo.Size = 18;
			Text_ShipClass->SetFont(FontInfo);
			Text_ShipClass->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
			MainContent->AddChild(Text_ShipClass);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created ship class text"));
		}
	}

	// Create description text if not exists
	if (!Text_Description && MainContent)
	{
		Text_Description = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Description"));
		if (Text_Description)
		{
			Text_Description->SetText(FText::FromString("Ship description will appear here"));
			Text_Description->SetAutoWrapText(true);
			FSlateFontInfo FontInfo = Text_Description->GetFont();
			FontInfo.Size = 14;
			Text_Description->SetFont(FontInfo);
			MainContent->AddChild(Text_Description);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created description text"));
		}
	}

	// Create ratings section
	UVerticalBox* RatingsBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("RatingsBox")));
	if (!RatingsBox && MainContent)
	{
		RatingsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RatingsBox"));
		if (RatingsBox)
		{
			MainContent->AddChild(RatingsBox);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created ratings box"));
		}
	}

	// Create combat rating text if not exists
	if (!Text_CombatRating && RatingsBox)
	{
		Text_CombatRating = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_CombatRating"));
		if (Text_CombatRating)
		{
			Text_CombatRating->SetText(FText::FromString("Combat Rating: 0"));
			FSlateFontInfo FontInfo = Text_CombatRating->GetFont();
			FontInfo.Size = 16;
			Text_CombatRating->SetFont(FontInfo);
			RatingsBox->AddChild(Text_CombatRating);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created combat rating text"));
		}
	}

	// Create mobility rating text if not exists
	if (!Text_MobilityRating && RatingsBox)
	{
		Text_MobilityRating = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_MobilityRating"));
		if (Text_MobilityRating)
		{
			Text_MobilityRating->SetText(FText::FromString("Mobility Rating: 0"));
			FSlateFontInfo FontInfo = Text_MobilityRating->GetFont();
			FontInfo.Size = 16;
			Text_MobilityRating->SetFont(FontInfo);
			RatingsBox->AddChild(Text_MobilityRating);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created mobility rating text"));
		}
	}

	// Create utility rating text if not exists
	if (!Text_UtilityRating && RatingsBox)
	{
		Text_UtilityRating = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_UtilityRating"));
		if (Text_UtilityRating)
		{
			Text_UtilityRating->SetText(FText::FromString("Utility Rating: 0"));
			FSlateFontInfo FontInfo = Text_UtilityRating->GetFont();
			FontInfo.Size = 16;
			Text_UtilityRating->SetFont(FontInfo);
			RatingsBox->AddChild(Text_UtilityRating);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created utility rating text"));
		}
	}

	// Create stats scroll box if not exists
	if (!StatsScrollBox && MainContent)
	{
		StatsScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("StatsScrollBox"));
		if (StatsScrollBox)
		{
			MainContent->AddChild(StatsScrollBox);
			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created stats scroll box"));
		}
	}

	// Create close button if not exists
	if (!Button_Close)
	{
		Button_Close = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_Close"));
		if (Button_Close)
		{
			RootCanvas->AddChild(Button_Close);
			UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(Button_Close->Slot);
			if (ButtonSlot)
			{
				ButtonSlot->SetAnchors(FAnchors(1.0f, 0.0f));
				ButtonSlot->SetAlignment(FVector2D(1.0f, 0.0f));
				ButtonSlot->SetPosition(FVector2D(-50.0f, 10.0f));
				ButtonSlot->SetSize(FVector2D(40.0f, 40.0f));
			}

			// Add "X" text to button
			UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Button_Close_Text"));
			if (ButtonText)
			{
				ButtonText->SetText(FText::FromString("X"));
				ButtonText->SetJustification(ETextJustify::Center);
				FSlateFontInfo FontInfo = ButtonText->GetFont();
				FontInfo.Size = 24;
				ButtonText->SetFont(FontInfo);
				Button_Close->AddChild(ButtonText);
			}

			UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Created close button"));
		}
	}
}

void UShipStatusWidget::OnCloseButtonClicked()
{
	// Hide the widget
	SetVisibility(ESlateVisibility::Hidden);
	UE_LOG(LogAdastrea, Log, TEXT("ShipStatusWidget: Close button clicked, hiding widget"));
}
