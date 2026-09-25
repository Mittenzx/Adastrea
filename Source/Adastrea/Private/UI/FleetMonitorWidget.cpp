// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FleetMonitorWidget.h"
#include "AI/AIMinerController.h"
#include "AI/AIPilotController.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/Border.h"
#include "Components/ButtonSlot.h"
#include "Components/BorderSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Mining/MiningLaserComponent.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Stations/DockingBayModule.h"
#include "Stations/MarketplaceModule.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "Styling/CoreStyle.h"
#include "Trading/CargoComponent.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"

namespace FleetMonitor
{
	const FLinearColor PanelColor(0.01f, 0.015f, 0.03f, 0.85f);
	const FLinearColor RowColor(0.05f, 0.07f, 0.12f, 0.9f);
	const FLinearColor RowHoverColor(0.09f, 0.14f, 0.24f, 0.95f);
	const FLinearColor TitleColor(0.45f, 0.85f, 1.0f);
	const FLinearColor HeadingColor(1.0f, 0.75f, 0.3f);
	const FLinearColor TextColor(0.9f, 0.92f, 0.95f);
	const FLinearColor DetailColor(0.7f, 0.78f, 0.85f);

	/** Walk up the child-actor/attachment chain to the station that owns Actor. */
	ASpaceStation* FindOwningStation(const AActor* Actor)
	{
		for (const AActor* Current = Actor; Current; )
		{
			if (const ASpaceStation* Station = Cast<ASpaceStation>(Current))
			{
				return const_cast<ASpaceStation*>(Station);
			}
			const AActor* Parent = Current->GetParentActor();
			Current = Parent ? Parent : Current->GetAttachParentActor();
		}
		return nullptr;
	}

	/** Station a ship is docked at, else null. */
	ASpaceStation* GetDockedStation(const ASpaceship* Ship)
	{
		return (Ship && Ship->IsDocked()) ? FindOwningStation(Ship->GetNearbyStation()) : nullptr;
	}

	FString ShipLabel(const ASpaceship* Ship)
	{
		const FString Name = Ship->GetShipName().ToString();
		const FString ActorName = Ship->GetActorNameOrLabel();
		return (Name.IsEmpty() || Name == ActorName) ? ActorName : FString::Printf(TEXT("%s [%s]"), *Name, *ActorName);
	}

	/** Station name, plus the actor name when several stations could share it. */
	FString StationLabel(const ASpaceStation* Station)
	{
		const FString Name = AAIPilotController::GetStationDisplayName(Station);
		const FString ActorName = Station->GetActorNameOrLabel();
		return Name == ActorName ? Name : FString::Printf(TEXT("%s [%s]"), *Name, *ActorName);
	}

	FString PilotLabel(const ASpaceship* Ship)
	{
		const AController* Controller = Ship->GetController();
		if (!Controller)
		{
			return TEXT("Unpiloted");
		}
		if (const APlayerController* PC = Cast<APlayerController>(Controller))
		{
			return PC->IsLocalController() ? TEXT("Player") : TEXT("Remote player");
		}
		if (Cast<AAIMinerController>(Controller))
		{
			return TEXT("AI Miner");
		}
		if (Cast<AAIPilotController>(Controller))
		{
			return TEXT("AI Trader");
		}
		return Controller->GetClass()->GetName();
	}

	FString Objective(const ASpaceship* Ship)
	{
		if (const AAIPilotController* Pilot = Cast<AAIPilotController>(Ship->GetController()))
		{
			return Pilot->GetObjectiveDescription();
		}
		if (const ASpaceStation* Docked = GetDockedStation(Ship))
		{
			return FString::Printf(TEXT("Docked at %s"), *AAIPilotController::GetStationDisplayName(Docked));
		}
		if (Ship->IsDocking())
		{
			return TEXT("Docking");
		}
		if (const UMiningLaserComponent* Laser = Ship->MiningLaser; Laser && Laser->IsMining() && Laser->GetTarget())
		{
			return FString::Printf(TEXT("Mining %s"), *Laser->GetTarget()->GetName());
		}
		if (Ship->IsOnAutopilot())
		{
			return TEXT("Autopilot");
		}
		return Ship->GetController() ? TEXT("Free flight") : TEXT("Idle");
	}

	FString FormatCredits(int32 Credits)
	{
		return FText::AsNumber(Credits).ToString() + TEXT(" cr");
	}

	/** Distance to the local player's pawn, e.g. "3.2 km", or empty if there's no player. */
	FString DistanceFromPlayer(const UWorld* World, const AActor* Actor)
	{
		const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
		if (!PlayerPawn || PlayerPawn == Actor)
		{
			return FString();
		}
		const float Metres = FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation()) / 100.0f;
		return Metres >= 1000.0f ? FString::Printf(TEXT("%.1f km"), Metres / 1000.0f) : FString::Printf(TEXT("%.0f m"), Metres);
	}

	FString CargoLines(const UCargoComponent* Cargo)
	{
		if (!Cargo)
		{
			return TEXT("Cargo:     (no hold)\n");
		}
		const float Used = Cargo->CargoCapacity - Cargo->GetAvailableCargoSpace();
		FString Out = FString::Printf(TEXT("Cargo:     %.0f / %.0f m3\n"), Used, Cargo->CargoCapacity);
		const TArray<FCargoEntry> Contents = Cargo->GetCargoContents();
		if (Contents.IsEmpty())
		{
			Out += TEXT("           (empty)\n");
		}
		for (const FCargoEntry& Entry : Contents)
		{
			const FString ItemName = Entry.Item ? Entry.Item->ItemName.ToString() : TEXT("(missing item)");
			Out += FString::Printf(TEXT("           %4d x %s\n"), Entry.Quantity, *ItemName);
		}
		return Out;
	}
}

// ---------------------------------------------------------------------------
// Row button
// ---------------------------------------------------------------------------

UFleetMonitorRowButton::UFleetMonitorRowButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitIsFocusable(false);
	OnClicked.AddDynamic(this, &UFleetMonitorRowButton::HandleClicked);
}

void UFleetMonitorRowButton::HandleClicked()
{
	OnRowClicked.ExecuteIfBound(Target.Get());
}

// ---------------------------------------------------------------------------
// Widget
// ---------------------------------------------------------------------------

void UFleetMonitorWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildLayout();
	Refresh();
}

void UFleetMonitorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TimeSinceRefresh += InDeltaTime;
	if (TimeSinceRefresh >= RefreshInterval)
	{
		Refresh();
	}
}

UTextBlock* UFleetMonitorWidget::MakeText(const FString& Text, int32 Size, const FLinearColor& Color)
{
	UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>();
	Block->SetFont(FCoreStyle::GetDefaultFontStyle("Mono", Size));
	Block->SetColorAndOpacity(FSlateColor(Color));
	Block->SetText(FText::FromString(Text));
	return Block;
}

UFleetMonitorRowButton* UFleetMonitorWidget::MakeButton(UWidget* Content)
{
	UFleetMonitorRowButton* Button = WidgetTree->ConstructWidget<UFleetMonitorRowButton>();
	FButtonStyle Style;
	Style.SetNormal(FSlateColorBrush(FleetMonitor::RowColor));
	Style.SetHovered(FSlateColorBrush(FleetMonitor::RowHoverColor));
	Style.SetPressed(FSlateColorBrush(FleetMonitor::RowHoverColor));
	Style.SetNormalPadding(FMargin(6.0f, 3.0f));
	Style.SetPressedPadding(FMargin(6.0f, 3.0f));
	Button->SetStyle(Style);
	if (UButtonSlot* ContentSlot = Cast<UButtonSlot>(Button->SetContent(Content)))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Left);
	}
	return Button;
}

void UFleetMonitorWidget::BuildLayout()
{
	using namespace FleetMonitor;

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Root;

	// Right-hand panel, full height, clear of the HUD's left-side telemetry.
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetBrush(FSlateColorBrush(PanelColor));
	Panel->SetPadding(FMargin(10.0f));
	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 1.0f));
	PanelSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	PanelSlot->SetOffsets(FMargin(-20.0f, 60.0f, 560.0f, 60.0f));

	UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Layout);

	// Title bar: title + expand/collapse all.
	UHorizontalBox* TitleBar = WidgetTree->ConstructWidget<UHorizontalBox>();
	TitleText = MakeText(TEXT("FLEET MONITOR"), 13, TitleColor);
	UHorizontalBoxSlot* TitleSlot = TitleBar->AddChildToHorizontalBox(TitleText);
	TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	TitleSlot->SetVerticalAlignment(VAlign_Center);

	UFleetMonitorRowButton* ExpandAll = MakeButton(MakeText(TEXT("Expand all"), 9, TextColor));
	ExpandAll->OnClicked.AddDynamic(this, &UFleetMonitorWidget::HandleExpandAll);
	TitleBar->AddChildToHorizontalBox(ExpandAll)->SetPadding(FMargin(4.0f, 0.0f, 0.0f, 0.0f));

	UFleetMonitorRowButton* CollapseAll = MakeButton(MakeText(TEXT("Collapse all"), 9, TextColor));
	CollapseAll->OnClicked.AddDynamic(this, &UFleetMonitorWidget::HandleCollapseAll);
	TitleBar->AddChildToHorizontalBox(CollapseAll)->SetPadding(FMargin(4.0f, 0.0f, 0.0f, 0.0f));

	Layout->AddChildToVerticalBox(TitleBar);
	Layout->AddChildToVerticalBox(MakeText(TEXT("F10 or adastrea.FleetMonitor to close. Click a row for details."), 8, DetailColor))
		->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 6.0f));

	UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
	Layout->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	ShipsHeading = MakeText(TEXT("SHIPS"), 11, HeadingColor);
	Cast<UScrollBoxSlot>(Scroll->AddChild(ShipsHeading))->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 2.0f));
	ShipList = WidgetTree->ConstructWidget<UVerticalBox>();
	Scroll->AddChild(ShipList);

	StationsHeading = MakeText(TEXT("STATIONS"), 11, HeadingColor);
	Cast<UScrollBoxSlot>(Scroll->AddChild(StationsHeading))->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 2.0f));
	StationList = WidgetTree->ConstructWidget<UVerticalBox>();
	Scroll->AddChild(StationList);
}

UFleetMonitorWidget::FRow& UFleetMonitorWidget::GetOrAddRow(AActor* Actor, UVerticalBox* Section)
{
	if (FRow* Existing = Rows.Find(Actor))
	{
		return *Existing;
	}

	FRow Row;
	UVerticalBox* Container = WidgetTree->ConstructWidget<UVerticalBox>();
	UTextBlock* HeaderText = MakeText(FString(), 10, FleetMonitor::TextColor);
	HeaderText->SetAutoWrapText(true);
	UFleetMonitorRowButton* Header = MakeButton(HeaderText);
	Header->Target = Actor;
	Header->OnRowClicked.BindUObject(this, &UFleetMonitorWidget::HandleRowClicked);
	Container->AddChildToVerticalBox(Header);

	UTextBlock* Details = MakeText(FString(), 9, FleetMonitor::DetailColor);
	Details->SetVisibility(ESlateVisibility::Collapsed);
	Container->AddChildToVerticalBox(Details)->SetPadding(FMargin(18.0f, 2.0f, 4.0f, 6.0f));

	Section->AddChildToVerticalBox(Container)->SetPadding(FMargin(0.0f, 1.0f));

	Row.Header = Header;
	Row.HeaderText = HeaderText;
	Row.Details = Details;
	Row.Container = Container;
	return Rows.Add(Actor, Row);
}

void UFleetMonitorWidget::UpdateRow(FRow& Row, const FString& Summary, const FString& Details)
{
	if (UTextBlock* HeaderText = Row.HeaderText.Get())
	{
		HeaderText->SetText(FText::FromString(FString(Row.bExpanded ? TEXT("- ") : TEXT("+ ")) + Summary));
	}
	if (UTextBlock* DetailsText = Row.Details.Get())
	{
		DetailsText->SetVisibility(Row.bExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (Row.bExpanded)
		{
			DetailsText->SetText(FText::FromString(Details.TrimEnd()));
		}
	}
}

void UFleetMonitorWidget::HandleRowClicked(AActor* Target)
{
	if (FRow* Row = Rows.Find(Target))
	{
		Row->bExpanded = !Row->bExpanded;
		Refresh();
	}
}

void UFleetMonitorWidget::HandleExpandAll()
{
	SetAllExpanded(true);
}

void UFleetMonitorWidget::HandleCollapseAll()
{
	SetAllExpanded(false);
}

void UFleetMonitorWidget::SetAllExpanded(bool bExpanded)
{
	for (TPair<TWeakObjectPtr<AActor>, FRow>& Pair : Rows)
	{
		Pair.Value.bExpanded = bExpanded;
	}
	Refresh();
}

void UFleetMonitorWidget::Refresh()
{
	TimeSinceRefresh = 0.0f;
	UWorld* World = GetWorld();
	if (!World || !ShipList || !StationList)
	{
		return;
	}

	// Drop rows whose actor is gone.
	for (auto It = Rows.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			if (UVerticalBox* Container = It.Value().Container.Get())
			{
				Container->RemoveFromParent();
			}
			It.RemoveCurrent();
		}
	}

	// Player's ship first, then by name, so new rows land in a stable order.
	TArray<ASpaceship*> Ships;
	for (TActorIterator<ASpaceship> It(World); It; ++It)
	{
		if (!It->IsActorBeingDestroyed())
		{
			Ships.Add(*It);
		}
	}
	Ships.Sort([](const ASpaceship& A, const ASpaceship& B)
	{
		const bool bAPlayer = A.IsPlayerControlled();
		const bool bBPlayer = B.IsPlayerControlled();
		return bAPlayer != bBPlayer ? bAPlayer : A.GetActorNameOrLabel() < B.GetActorNameOrLabel();
	});

	TArray<ASpaceStation*> Stations;
	for (TActorIterator<ASpaceStation> It(World); It; ++It)
	{
		if (!It->IsActorBeingDestroyed())
		{
			Stations.Add(*It);
		}
	}
	Stations.Sort([](const ASpaceStation& A, const ASpaceStation& B)
	{
		return AAIPilotController::GetStationDisplayName(&A) < AAIPilotController::GetStationDisplayName(&B);
	});

	for (ASpaceship* Ship : Ships)
	{
		FRow& Row = GetOrAddRow(Ship, ShipList);
		UpdateRow(Row, DescribeShipSummary(Ship), Row.bExpanded ? DescribeShipDetails(Ship) : FString());
	}
	for (ASpaceStation* Station : Stations)
	{
		FRow& Row = GetOrAddRow(Station, StationList);
		UpdateRow(Row, DescribeStationSummary(Station, Ships), Row.bExpanded ? DescribeStationDetails(Station, Ships) : FString());
	}

	ShipsHeading->SetText(FText::FromString(FString::Printf(TEXT("SHIPS (%d)"), Ships.Num())));
	StationsHeading->SetText(FText::FromString(FString::Printf(TEXT("STATIONS (%d)"), Stations.Num())));
}

// ---------------------------------------------------------------------------
// Descriptions
// ---------------------------------------------------------------------------

FString UFleetMonitorWidget::DescribeShipSummary(const ASpaceship* Ship) const
{
	using namespace FleetMonitor;
	return FString::Printf(TEXT("%s | %s | %s"), *ShipLabel(Ship), *PilotLabel(Ship), *Objective(Ship));
}

FString UFleetMonitorWidget::DescribeShipDetails(const ASpaceship* Ship) const
{
	using namespace FleetMonitor;

	FString Out;
	Out += FString::Printf(TEXT("Class:     %s (%s)\n"), *Ship->GetShipClass().ToString(), *Ship->GetClass()->GetName());
	Out += FString::Printf(TEXT("Pilot:     %s"), *PilotLabel(Ship));
	if (const AController* Controller = Ship->GetController())
	{
		Out += FString::Printf(TEXT(" (%s)"), *Controller->GetName());
	}
	Out += TEXT("\n");
	Out += FString::Printf(TEXT("Objective: %s\n"), *Objective(Ship));

	if (const UPlayerTraderComponent* Trader = Ship->PlayerTraderComponent)
	{
		const int32 Profit = Trader->GetProfit();
		Out += FString::Printf(TEXT("Credits:   %s (profit %s%s)\n"), *FormatCredits(Trader->GetCredits()),
			Profit >= 0 ? TEXT("+") : TEXT(""), *FormatCredits(Profit));
	}
	else
	{
		Out += TEXT("Credits:   (no trader component)\n");
	}

	Out += CargoLines(Ship->CargoComponent);

	Out += FString::Printf(TEXT("Hull:      %.0f / %.0f\n"), Ship->GetCurrentHullIntegrity(), Ship->GetMaxHullIntegrity());
	Out += FString::Printf(TEXT("Speed:     %.0f m/s  (throttle %.0f%%)\n"), Ship->GetCurrentSpeed() / 100.0f, Ship->ThrottlePercentage);

	FString Dock = TEXT("Undocked");
	if (const ASpaceStation* Docked = GetDockedStation(Ship))
	{
		Dock = FString::Printf(TEXT("Docked at %s"), *AAIPilotController::GetStationDisplayName(Docked));
	}
	else if (Ship->IsDocking())
	{
		Dock = TEXT("Docking in progress");
	}
	Out += FString::Printf(TEXT("Docking:   %s\n"), *Dock);

	if (const UMiningLaserComponent* Laser = Ship->MiningLaser; Laser && Laser->bMiningEnabled)
	{
		const AActor* Target = Laser->GetTarget();
		Out += FString::Printf(TEXT("Mining:    %s%s\n"), *UMiningLaserComponent::StatusToText(Laser->GetStatus()).ToString(),
			Target ? *FString::Printf(TEXT(" on %s (%.0f m)"), *Target->GetName(), Laser->GetTargetSurfaceDistance() / 100.0f) : TEXT(""));
	}

	const FString Distance = DistanceFromPlayer(GetWorld(), Ship);
	if (!Distance.IsEmpty())
	{
		Out += FString::Printf(TEXT("Distance:  %s from player\n"), *Distance);
	}
	return Out;
}

FString UFleetMonitorWidget::DescribeStationSummary(const ASpaceStation* Station, const TArray<ASpaceship*>& Ships) const
{
	int32 Docked = 0;
	int32 Inbound = 0;
	for (const ASpaceship* Ship : Ships)
	{
		if (FleetMonitor::GetDockedStation(Ship) == Station)
		{
			++Docked;
		}
		else if (const AAIPilotController* Pilot = Cast<AAIPilotController>(Ship->GetController()); Pilot && Pilot->TargetStation == Station)
		{
			++Inbound;
		}
	}
	return FString::Printf(TEXT("%s | docked %d, inbound %d"), *FleetMonitor::StationLabel(Station), Docked, Inbound);
}

FString UFleetMonitorWidget::DescribeStationDetails(const ASpaceStation* Station, const TArray<ASpaceship*>& Ships) const
{
	using namespace FleetMonitor;

	FString Out;
	Out += FString::Printf(TEXT("Actor:     %s\n"), *Station->GetActorNameOrLabel());
	Out += FString::Printf(TEXT("Modules:   %d\n"), Station->GetModuleCount());
	Out += FString::Printf(TEXT("Integrity: %.0f / %.0f%s\n"), Station->GetCurrentHealth_Implementation(), Station->GetMaxHealth_Implementation(),
		Station->IsDestroyed_Implementation() ?TEXT("  DESTROYED") : TEXT(""));
	Out += FString::Printf(TEXT("Power:     %.0f gen / %.0f use (balance %+.0f)\n"),
		Station->GetTotalPowerGeneration(), Station->GetTotalPowerConsumption(), Station->GetPowerBalance());
	Out += FString::Printf(TEXT("Storage:   %d / %d    Residents: %d / %d\n"),
		Station->GetTotalCargoStored(), Station->GetTotalStorageCapacity(), Station->GetTotalResidents(), Station->GetTotalCrewCapacity());

	// Docking bays and who is where.
	int32 Berths = 0;
	int32 Taken = 0;
	for (const ADockingBayModule* Bay : Station->GetDockingBayModules())
	{
		if (Bay)
		{
			Berths += Bay->MaxDockedShips;
			Taken += Bay->CurrentDockedShips;
		}
	}
	Out += FString::Printf(TEXT("Berths:    %d / %d taken\n"), Taken, Berths);

	FString DockedList;
	FString InboundList;
	for (const ASpaceship* Ship : Ships)
	{
		if (GetDockedStation(Ship) == Station)
		{
			DockedList += FString::Printf(TEXT("           %s (%s)\n"), *ShipLabel(Ship), *PilotLabel(Ship));
		}
		else if (const AAIPilotController* Pilot = Cast<AAIPilotController>(Ship->GetController()); Pilot && Pilot->TargetStation == Station)
		{
			InboundList += FString::Printf(TEXT("           %s (%s, %.1f km out)\n"), *ShipLabel(Ship), *PilotLabel(Ship),
				FVector::Dist(Ship->GetActorLocation(), Station->GetActorLocation()) / 100000.0f);
		}
	}
	Out += TEXT("Docked:\n") + (DockedList.IsEmpty() ? FString(TEXT("           (none)\n")) : DockedList);
	Out += TEXT("Inbound:\n") + (InboundList.IsEmpty() ? FString(TEXT("           (none)\n")) : InboundList);

	// Market stock and prices.
	const AMarketplaceModule* Marketplace = Station->GetMarketplaceModule();
	UMarketDataAsset* Market = Marketplace ? Marketplace->GetMarketData() : nullptr;
	if (!Market)
	{
		Out += TEXT("Market:    (none)\n");
		return Out;
	}
	Out += FString::Printf(TEXT("Market:    %s%s\n"), *Market->MarketName.ToString(), Market->bAllowAITraders ? TEXT("") : TEXT("  (no AI traders)"));
	Out += TEXT("           item                  stock      buy     sell\n");
	for (const FMarketInventoryEntry& Entry : Market->Inventory)
	{
		if (!Entry.TradeItem)
		{
			continue;
		}
		Out += FString::Printf(TEXT("           %-20s %4d/%-4d %7.0f  %7.0f\n"),
			*Entry.TradeItem->ItemName.ToString().Left(20), Entry.CurrentStock, Entry.MaxStock,
			Market->GetItemPrice(Entry.TradeItem, true), Market->GetItemPrice(Entry.TradeItem, false));
	}
	return Out;
}
