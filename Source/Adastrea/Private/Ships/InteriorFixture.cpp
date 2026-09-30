#include "Ships/InteriorFixture.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipAvatar.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Player/AdastreaPlayerController.h"
#include "Player/PlayerInteractableComponent.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "Stations/SpaceStation.h"
#include "Interfaces/ITargetable.h"
#include "Audio/AudioEventLibrary.h"
#include "AdastreaHUD.h"
#include "AdastreaLog.h"
#include "CanvasItem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TextureResource.h"

namespace
{
	const TCHAR* const FeedNames[] = { TEXT("Bow"), TEXT("Stern"), TEXT("Chase"), TEXT("Dorsal"), TEXT("Ventral"), TEXT("Port"), TEXT("Starboard") };
	const TCHAR* const ReadoutNames[] = { TEXT("Status"), TEXT("Nav"), TEXT("Engineering"), TEXT("Cargo"), TEXT("Comms") };
	constexpr int32 NumFeeds = UE_ARRAY_COUNT(FeedNames);

	const TCHAR* const CrewChatter[] = {
		TEXT("Deck 2, galley reports coffee is ready."),
		TEXT("Engineering: coolant loop 3 back in the green."),
		TEXT("Cargo bay, clamps secure. Manifest checks out."),
		TEXT("Medbay: routine checkups at 0900."),
		TEXT("Bridge copies. Holding course."),
		TEXT("Maintenance crew to the aft hatch, please."),
		TEXT("Reminder: log your EVA hours."),
	};

	FLinearColor ReadoutColour(EConsoleReadout R)
	{
		switch (R)
		{
		case EConsoleReadout::Navigation:  return FLinearColor(0.35f, 0.75f, 1.0f);
		case EConsoleReadout::Engineering: return FLinearColor(1.0f, 0.6f, 0.2f);
		case EConsoleReadout::Cargo:       return FLinearColor(1.0f, 0.85f, 0.35f);
		case EConsoleReadout::Comms:       return FLinearColor(0.7f, 0.5f, 1.0f);
		default:                           return FLinearColor(0.3f, 1.0f, 0.5f);
		}
	}
}

AInteriorFixture::AInteriorFixture()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickInterval = 0.25f;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	// The deck's walk-collision mesh already blocks the avatar around every fixture.
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(true);

	Interactable = CreateDefaultSubobject<UPlayerInteractableComponent>(TEXT("Interactable"));
	Interactable->InteractionRadius = 180.0f;
	Interactable->OnInteracted.AddDynamic(this, &AInteriorFixture::OnUsed);
}

bool AInteriorFixture::ParseKind(const FString& MeshName, EInteriorFixtureKind& OutKind)
{
	if (MeshName.StartsWith(TEXT("Monitor"))) { OutKind = EInteriorFixtureKind::Monitor; return true; }
	if (MeshName == TEXT("Terminal") || MeshName == TEXT("WallPanel")) { OutKind = EInteriorFixtureKind::Console; return true; }
	if (MeshName == TEXT("AlertButton")) { OutKind = EInteriorFixtureKind::AlertButton; return true; }
	if (MeshName == TEXT("LightSwitch")) { OutKind = EInteriorFixtureKind::LightSwitch; return true; }
	if (MeshName == TEXT("Intercom")) { OutKind = EInteriorFixtureKind::Intercom; return true; }
	if (MeshName == TEXT("Coffee")) { OutKind = EInteriorFixtureKind::Dispenser; return true; }
	if (MeshName == TEXT("MapTable")) { OutKind = EInteriorFixtureKind::MapTerminal; return true; }
	return false;
}

bool AInteriorFixture::ParseFeed(const FString& Name, EExteriorFeed& OutFeed)
{
	for (int32 i = 0; i < NumFeeds; ++i)
	{
		if (Name.Equals(FeedNames[i], ESearchCase::IgnoreCase))
		{
			OutFeed = static_cast<EExteriorFeed>(i);
			return true;
		}
	}
	return false;
}

bool AInteriorFixture::ParseReadout(const FString& Name, EConsoleReadout& OutReadout)
{
	for (int32 i = 0; i < UE_ARRAY_COUNT(ReadoutNames); ++i)
	{
		if (Name.Equals(ReadoutNames[i], ESearchCase::IgnoreCase))
		{
			OutReadout = static_cast<EConsoleReadout>(i);
			return true;
		}
	}
	return false;
}

FString AInteriorFixture::FeedName(EExteriorFeed InFeed)
{
	const int32 i = static_cast<int32>(InFeed);
	return i >= 0 && i < NumFeeds ? FString(FeedNames[i]) : FString(TEXT("?"));
}

void AInteriorFixture::Setup(ASpaceshipInterior* InInterior, EInteriorFixtureKind InKind, FName Arg, FName MeshName)
{
	Interior = InInterior;
	Kind = InKind;
	if (Kind == EInteriorFixtureKind::Monitor)
	{
		ParseFeed(Arg.ToString(), Feed);
	}
	else if (Kind == EInteriorFixtureKind::Console)
	{
		ParseReadout(Arg.ToString(), Readout);
	}

	if (!Mesh->GetStaticMesh() && !MeshName.IsNone())
	{
		const FString Path = FString::Printf(TEXT("/AdastreaShips/Meshes/Props/SM_Prop_Fx_%s.SM_Prop_Fx_%s"),
			*MeshName.ToString(), *MeshName.ToString());
		if (UStaticMesh* SM = LoadObject<UStaticMesh>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			Mesh->SetStaticMesh(SM);
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("InteriorFixture: mesh %s missing (run Tools/import_interior_props.py)."), *Path);
		}
	}

	// Screen material: a dynamic instance on the render-target slot, if the mesh has one.
	ScreenMID = nullptr;
	for (const TCHAR* Slot : { TEXT("M_Prop_Live"), TEXT("M_Prop_Term") })
	{
		const int32 Index = Mesh->GetMaterialIndex(FName(Slot));
		if (Index != INDEX_NONE)
		{
			ScreenMID = Mesh->CreateDynamicMaterialInstance(Index);
			break;
		}
	}
	if (ScreenMID && (Kind != EInteriorFixtureKind::Monitor))
	{
		ScreenCanvas = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 512, 320);
		if (ScreenCanvas)
		{
			ScreenCanvas->ClearColor = FLinearColor(0.0f, 0.004f, 0.002f, 1.0f);
			ScreenCanvas->OnCanvasRenderTargetUpdate.AddDynamic(this, &AInteriorFixture::DrawScreen);
			ScreenMID->SetTextureParameterValue(TEXT("Screen"), ScreenCanvas);
			ScreenMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
		}
	}

	// Stand-in interaction point: the face of the fixture at chest height.
	const FBoxSphereBounds B = Mesh->GetStaticMesh() ? Mesh->GetStaticMesh()->GetBounds() : FBoxSphereBounds(FVector::ZeroVector, FVector(30.0f), 30.0f);
	Interactable->InteractionPointOffset = FVector(B.Origin.X + B.BoxExtent.X, B.Origin.Y, FMath::Clamp(B.Origin.Z, 0.0f, 150.0f));
	RefreshPrompt();
	SetActorHiddenInGame(true);
}

void AInteriorFixture::SetFixtureActive(bool bInActive)
{
	bActive = bInActive;
	SetActorHiddenInGame(!bInActive);
	SetActorTickEnabled(bInActive && ScreenCanvas != nullptr);
	if (bInActive && Kind == EInteriorFixtureKind::Monitor)
	{
		BindMonitorFeed();
	}
	if (bInActive && ScreenCanvas)
	{
		ScreenCanvas->UpdateResource();
	}
}

void AInteriorFixture::BindMonitorFeed()
{
	if (!ScreenMID)
	{
		return;
	}
	if (ASpaceshipInterior* I = Interior.Get())
	{
		if (UTextureRenderTarget2D* RT = I->AcquireFeed(Feed))
		{
			ScreenMID->SetTextureParameterValue(TEXT("Screen"), RT);
			ScreenMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
		}
	}
}

void AInteriorFixture::RefreshPrompt()
{
	FText Prompt;
	switch (Kind)
	{
	case EInteriorFixtureKind::Monitor:
		Prompt = FText::Format(NSLOCTEXT("Adastrea", "FixtureMonitor", "Camera: {0} (switch)"), FText::FromString(FeedName(Feed).ToUpper()));
		break;
	case EInteriorFixtureKind::Console:
		switch (Readout)
		{
		case EConsoleReadout::Navigation:  Prompt = NSLOCTEXT("Adastrea", "FixtureNav", "Navigation Terminal"); break;
		case EConsoleReadout::Engineering: Prompt = NSLOCTEXT("Adastrea", "FixtureEng", "Engineering: Reroute Power"); break;
		case EConsoleReadout::Cargo:       Prompt = NSLOCTEXT("Adastrea", "FixtureCargo", "Cargo Manifest"); break;
		case EConsoleReadout::Comms:       Prompt = NSLOCTEXT("Adastrea", "FixtureComms", "Comms: Hail"); break;
		default:                           Prompt = NSLOCTEXT("Adastrea", "FixtureStatus", "Ship Status"); break;
		}
		break;
	case EInteriorFixtureKind::AlertButton:
		Prompt = (Interior.IsValid() && Interior->IsRedAlert())
			? NSLOCTEXT("Adastrea", "FixtureAlertOff", "Stand Down Red Alert")
			: NSLOCTEXT("Adastrea", "FixtureAlertOn", "Sound Red Alert");
		break;
	case EInteriorFixtureKind::LightSwitch: Prompt = NSLOCTEXT("Adastrea", "FixtureLights", "Lighting"); break;
	case EInteriorFixtureKind::Intercom:    Prompt = NSLOCTEXT("Adastrea", "FixtureIntercom", "Intercom"); break;
	case EInteriorFixtureKind::Dispenser:   Prompt = NSLOCTEXT("Adastrea", "FixtureCoffee", "Pour Coffee"); break;
	case EInteriorFixtureKind::MapTerminal: Prompt = NSLOCTEXT("Adastrea", "FixtureMap", "Star Map"); break;
	}
	Interactable->InteractPrompt = Prompt;
}

bool AInteriorFixture::IsAvatarNear(float Radius) const
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	return Pawn && FVector::DistSquared(Pawn->GetActorLocation(), GetActorLocation()) < FMath::Square(Radius);
}

void AInteriorFixture::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ScreenClock += DeltaSeconds;
	// Redraw only when someone could be reading it.
	if (ScreenCanvas && IsAvatarNear(1500.0f))
	{
		ScreenCanvas->UpdateResource();
	}
}

void AInteriorFixture::OnUsed(AAdastreaPlayerController* PC)
{
	ASpaceshipInterior* I = Interior.Get();
	++UseCount;
	UAudioEventLibrary::PlayEventAtLocation(this, TEXT("Interior.ConsoleChirp"), GetActorLocation());

	switch (Kind)
	{
	case EInteriorFixtureKind::Monitor:
		Feed = static_cast<EExteriorFeed>((static_cast<int32>(Feed) + 1) % NumFeeds);
		BindMonitorFeed();
		break;

	case EInteriorFixtureKind::Console:
		if (PC)
		{
			switch (Readout)
			{
			case EConsoleReadout::Navigation:
				if (AAdastreaHUD* HUD = PC->GetHUD<AAdastreaHUD>()) { HUD->ToggleMap(); }
				break;
			case EConsoleReadout::Cargo:
				PC->ToggleInventory();
				break;
			case EConsoleReadout::Engineering:
				PC->ShowHUDMessage(TEXT("Engineering: power rerouted to engines."), 2.5f, false);
				break;
			case EConsoleReadout::Comms:
				CommsLog.Add(FString::Printf(TEXT("> Hailing frequencies open (%d)."), UseCount));
				PC->ShowHUDMessage(TEXT("Comms: hailing frequencies open."), 2.5f, false);
				break;
			default:
				PC->ToggleShipStatus();
				break;
			}
		}
		break;

	case EInteriorFixtureKind::AlertButton:
		if (I)
		{
			I->SetRedAlert(!I->IsRedAlert());
			if (PC) { PC->ShowHUDMessage(I->IsRedAlert() ? TEXT("RED ALERT - all hands to stations") : TEXT("Red alert cancelled"), 3.0f, I->IsRedAlert()); }
		}
		break;

	case EInteriorFixtureKind::LightSwitch:
		if (I)
		{
			const int32 Mode = I->CycleLighting();
			static const TCHAR* ModeNames[] = { TEXT("Lighting: normal"), TEXT("Lighting: dimmed"), TEXT("Lighting: emergency") };
			if (PC) { PC->ShowHUDMessage(ModeNames[FMath::Clamp(Mode, 0, 2)], 2.0f, false); }
		}
		break;

	case EInteriorFixtureKind::Intercom:
		if (PC)
		{
			PC->ShowHUDMessage(FString::Printf(TEXT("Intercom: %s"), CrewChatter[UseCount % UE_ARRAY_COUNT(CrewChatter)]), 3.5f, false);
		}
		break;

	case EInteriorFixtureKind::Dispenser:
	{
		// Pour a cup onto the counter (a few at most, oldest recycled).
		static const TCHAR* MugPath = TEXT("/AdastreaShips/Meshes/Props/SM_Prop_Mug.SM_Prop_Mug");
		if (UStaticMesh* MugMesh = LoadObject<UStaticMesh>(nullptr, MugPath, nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			UStaticMeshComponent* Cup = nullptr;
			if (Cups.Num() >= 4)
			{
				Cup = Cups[0];
				Cups.RemoveAt(0);
			}
			else
			{
				Cup = NewObject<UStaticMeshComponent>(this);
				Cup->SetupAttachment(Root);
				Cup->SetStaticMesh(MugMesh);
				Cup->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Cup->RegisterComponent();
			}
			const int32 Slot = UseCount % 4;
			Cup->SetRelativeLocation(FVector(-8.0f - (Slot % 2) * 10.0f, 4.0f + (Slot / 2) * 10.0f, 92.0f));
			Cups.Add(Cup);
		}
		if (PC) { PC->ShowHUDMessage(TEXT("Fresh coffee. Mind, it's hot."), 2.0f, false); }
		break;
	}

	case EInteriorFixtureKind::MapTerminal:
		if (PC)
		{
			if (AAdastreaHUD* HUD = PC->GetHUD<AAdastreaHUD>()) { HUD->ToggleMap(); }
		}
		break;
	}
	RefreshPrompt();
	if (ScreenCanvas)
	{
		ScreenCanvas->UpdateResource();
	}
}

void AInteriorFixture::DrawScreen(UCanvas* Canvas, int32 Width, int32 Height)
{
	if (!Canvas)
	{
		return;
	}
	DrawReadout(Canvas, static_cast<float>(Width), static_cast<float>(Height));
}

namespace
{
	void Tile(UCanvas* C, float X, float Y, float W, float H, const FLinearColor& Col)
	{
		FCanvasTileItem Item(FVector2D(X, Y), GWhiteTexture, FVector2D(W, H), Col);
		Item.BlendMode = SE_BLEND_Translucent;
		C->DrawItem(Item);
	}

	void Text(UCanvas* C, const FString& S, float X, float Y, const FLinearColor& Col, float Scale = 1.0f)
	{
		UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
		C->SetDrawColor(Col.ToFColor(true));
		C->DrawText(Font, S, X, Y, Scale, Scale);
	}

	void Bar(UCanvas* C, const FString& Label, float Frac, float Y, float W, const FLinearColor& Col)
	{
		Text(C, Label, 24.0f, Y, Col * 0.85f);
		Tile(C, 200.0f, Y + 4.0f, W - 230.0f, 16.0f, Col * 0.18f);
		Tile(C, 200.0f, Y + 4.0f, (W - 230.0f) * FMath::Clamp(Frac, 0.0f, 1.0f), 16.0f, Col);
	}
}

void AInteriorFixture::DrawReadout(UCanvas* C, float W, float H)
{
	const bool bAlert = Interior.IsValid() && Interior->IsRedAlert();
	FLinearColor Col = bAlert ? FLinearColor(1.0f, 0.2f, 0.12f) : ReadoutColour(Readout);
	ASpaceship* Ship = Interior.IsValid() ? Interior->GetOwningShip() : nullptr;
	const float T = ScreenClock;

	// frame + header
	Tile(C, 0, 0, W, H, FLinearColor(0.0f, 0.01f, 0.008f, 1.0f));
	Tile(C, 0, 0, W, 44.0f, Col * 0.35f);
	Tile(C, 0, H - 4.0f, W, 4.0f, Col * 0.6f);
	FString Title;
	switch (Kind)
	{
	case EInteriorFixtureKind::Intercom:    Title = TEXT("INTERCOM"); break;
	case EInteriorFixtureKind::MapTerminal: Title = TEXT("ASTROGATION"); break;
	default:
		switch (Readout)
		{
		case EConsoleReadout::Navigation:  Title = TEXT("NAVIGATION"); break;
		case EConsoleReadout::Engineering: Title = TEXT("ENGINEERING"); break;
		case EConsoleReadout::Cargo:       Title = TEXT("CARGO MANIFEST"); break;
		case EConsoleReadout::Comms:       Title = TEXT("COMMUNICATIONS"); break;
		default:                           Title = TEXT("SHIP STATUS"); break;
		}
	}
	Text(C, Title, 20.0f, 8.0f, FLinearColor::White, 1.35f);
	const bool bBlink = FMath::Fmod(T, 1.0f) < 0.5f;
	if (bAlert)
	{
		Text(C, bBlink ? TEXT("RED ALERT") : TEXT(""), W - 170.0f, 10.0f, FLinearColor(1, 0.25f, 0.2f), 1.2f);
	}
	else
	{
		Tile(C, W - 34.0f, 14.0f, 14.0f, 14.0f, bBlink ? Col : Col * 0.3f);
	}

	float Y = 60.0f;
	const float Line = 30.0f;
	auto Row = [&](const FString& L, const FString& V)
	{
		Text(C, L, 24.0f, Y, Col * 0.8f);
		Text(C, V, 230.0f, Y, FLinearColor::White);
		Y += Line;
	};

	if (Kind == EInteriorFixtureKind::Intercom)
	{
		Row(TEXT("CHANNEL"), TEXT("ALL DECKS"));
		Row(TEXT("STATUS"), bBlink ? TEXT("OPEN") : TEXT(""));
		return;
	}
	if (!Ship)
	{
		Row(TEXT("LINK"), TEXT("NO SHIP DATA"));
		return;
	}

	switch (Readout)
	{
	case EConsoleReadout::Navigation:
	{
		const FVector P = Ship->GetActorLocation() / 100000.0f;
		Row(TEXT("POSITION km"), FString::Printf(TEXT("%.1f  %.1f  %.1f"), P.X, P.Y, P.Z));
		Row(TEXT("HEADING"), FString::Printf(TEXT("%03.0f"), FRotator::ClampAxis(Ship->GetActorRotation().Yaw)));
		Row(TEXT("SPEED m/s"), FString::Printf(TEXT("%.0f"), Ship->GetCurrentSpeed() / 100.0f));
		// nearest station
		const ASpaceStation* Best = nullptr;
		double BestD = TNumericLimits<double>::Max();
		for (TActorIterator<ASpaceStation> It(GetWorld()); It; ++It)
		{
			const double D = FVector::DistSquared(It->GetActorLocation(), Ship->GetActorLocation());
			if (D < BestD) { BestD = D; Best = *It; }
		}
		if (Best)
		{
			const FText Display = ITargetable::Execute_GetTargetDisplayName(const_cast<ASpaceStation*>(Best));
			const FString Name = Display.IsEmpty() ? Best->GetName() : Display.ToString();
			Row(TEXT("NEAREST"), Name.Left(22));
			Row(TEXT("RANGE km"), FString::Printf(TEXT("%.1f"), FMath::Sqrt(BestD) / 100000.0f));
		}
		// radar sweep
		const FVector2D Ctr(W - 90.0f, H - 90.0f);
		for (int32 k = 0; k < 24; ++k)
		{
			const float A = T * 2.0f - k * 0.05f;
			Tile(C, Ctr.X + FMath::Cos(A) * 60.0f * (k / 24.0f), Ctr.Y + FMath::Sin(A) * 60.0f * (k / 24.0f), 4, 4, Col * (1.0f - k / 24.0f));
		}
		break;
	}
	case EConsoleReadout::Engineering:
	{
		const float Pulse = 0.5f + 0.5f * FMath::Sin(T * 1.7f);
		Bar(C, TEXT("REACTOR"), 0.82f + 0.06f * Pulse, Y, W, Col); Y += Line;
		Bar(C, TEXT("ENGINES"), 0.55f + 0.2f * FMath::Clamp(Ship->GetCurrentSpeed() / 3000.0f, 0.0f, 1.0f), Y, W, Col); Y += Line;
		Bar(C, TEXT("SHIELDS"), 0.7f, Y, W, Col); Y += Line;
		Bar(C, TEXT("LIFE SUPPORT"), 0.96f, Y, W, Col); Y += Line;
		Row(TEXT("CORE TEMP"), FString::Printf(TEXT("%.0f K"), 1480.0f + 35.0f * Pulse));
		Row(TEXT("STATUS"), bAlert ? TEXT("COMBAT POWER") : TEXT("NOMINAL"));
		break;
	}
	case EConsoleReadout::Cargo:
	{
		if (UCargoComponent* Cargo = Ship->CargoComponent)
		{
			const float Cap = FMath::Max(1.0f, Cargo->CargoCapacity);
			Bar(C, TEXT("HOLD"), 1.0f - Cargo->GetAvailableCargoSpace() / Cap, Y, W, Col); Y += Line + 4.0f;
			int32 Shown = 0;
			for (const FCargoEntry& E : Cargo->GetCargoContents())
			{
				if (!E.Item || Shown >= 6) { continue; }
				Row(E.Item->ItemName.ToString().Left(20), FString::Printf(TEXT("x %d"), E.Quantity));
				++Shown;
			}
			if (Shown == 0) { Row(TEXT("MANIFEST"), TEXT("HOLD EMPTY")); }
		}
		else
		{
			Row(TEXT("MANIFEST"), TEXT("NO CARGO SYSTEM"));
		}
		break;
	}
	case EConsoleReadout::Comms:
	{
		const int32 N = CommsLog.Num();
		for (int32 k = 0; k < 6; ++k)
		{
			const int32 Idx = (static_cast<int32>(T / 3.0f) + k) % UE_ARRAY_COUNT(CrewChatter);
			const FString S = (N > 0 && k == 5) ? CommsLog.Last() : FString::Printf(TEXT("%02d:%02d %s"), (Idx * 7) % 24, (Idx * 13) % 60, CrewChatter[Idx]);
			Text(C, S.Left(40), 24.0f, Y, k == 5 ? FLinearColor::White : Col * 0.75f, 0.9f);
			Y += Line - 4.0f;
		}
		break;
	}
	default:
	{
		const FString Name = Ship->GetShipName().IsEmpty() ? Ship->GetName() : Ship->GetShipName().ToString();
		Row(TEXT("VESSEL"), Name.Left(22));
		if (Ship->ShipDataAsset)
		{
			Row(TEXT("CLASS"), Ship->ShipDataAsset->ShipClass.ToString().Left(22));
		}
		const float Hull = Ship->GetMaxHullIntegrity() > 0.0f ? Ship->GetCurrentHullIntegrity() / Ship->GetMaxHullIntegrity() : 1.0f;
		Bar(C, TEXT("HULL"), Hull, Y, W, Hull > 0.4f ? Col : FLinearColor(1, 0.2f, 0.1f)); Y += Line;
		Row(TEXT("SPEED m/s"), FString::Printf(TEXT("%.0f"), Ship->GetCurrentSpeed() / 100.0f));
		if (UPlayerTraderComponent* Trader = Ship->PlayerTraderComponent)
		{
			Row(TEXT("CREDITS"), FString::Printf(TEXT("%d cr"), Trader->GetCredits()));
		}
		break;
	}
	}
}
