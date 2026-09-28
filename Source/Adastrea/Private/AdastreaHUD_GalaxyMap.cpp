// Copyright Epic Games, Inc. All Rights Reserved.

// Galaxy layers of the full-screen map (M): the System and Universe views,
// the Sector/System/Universe tabs, and map click handling. The Sector view
// (the loaded level in 3D) is AAdastreaHUD::DrawSectorMap in AdastreaHUD.cpp.

#include "AdastreaHUD.h"
#include "AdastreaHUDStyle.h"
#include "Universe/GalaxySubsystem.h"
#include "Universe/JumpGate.h"
#include "Ships/Spaceship.h"
#include "Player/AdastreaPlayerController.h"
#include "UI/AdastreaHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "EngineUtils.h"
#include "Audio/AudioEventLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Misc/Paths.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"

namespace GalaxyMap
{
	// Shared layout (matches the Sector view's box in DrawSectorMap).
	constexpr float Margin = 40.0f;
	constexpr float HeaderH = 30.0f;
	constexpr float FooterH = 70.0f;
	constexpr float PanelW = 340.0f;
	constexpr float PanelGap = 12.0f;

	const FLinearColor Backdrop(0.008f, 0.012f, 0.02f, 1.0f);
	const FLinearColor BoxFill(0.03f, 0.05f, 0.07f, 0.9f);
	const FLinearColor BoxEdge(0.10f, 0.55f, 0.60f, 0.9f);
	const FLinearColor Title(0.6f, 0.9f, 1.0f, 1.0f);
	const FLinearColor Body(0.78f, 0.86f, 0.92f, 1.0f);
	const FLinearColor Dim(0.5f, 0.6f, 0.68f, 0.9f);
	const FLinearColor Help(0.6f, 0.7f, 0.8f, 0.9f);
	const FLinearColor You(0.15f, 0.95f, 0.6f, 1.0f);
	const FLinearColor Built(0.25f, 0.88f, 0.82f, 1.0f);
	const FLinearColor Planned(0.5f, 0.56f, 0.62f, 1.0f);
	const FLinearColor Gate(0.95f, 0.72f, 0.3f, 0.6f);
	const FLinearColor Lane(0.35f, 0.6f, 0.72f, 0.55f);
	const FLinearColor LaneHot(0.55f, 0.9f, 1.0f, 0.95f);
	const FLinearColor Select(1.0f, 1.0f, 1.0f, 0.95f);

	struct FLayout
	{
		float VW = 0, VH = 0;
		float BoxX = 0, BoxY = 0, BoxW = 0, BoxH = 0;
		float AreaX = 0, AreaW = 0;              // map drawing area (left of the panel)
		float PanelX = 0, PanelY = 0, PanelH = 0, PanelWidth = 0;
	};

	FLayout MakeLayout(const UCanvas* Canvas, APlayerController* PC)
	{
		FLayout L;
		int32 VX = 0, VY = 0;
		PC->GetViewportSize(VX, VY);
		// Canvas size, not viewport size: they differ for high-res screenshots.
		L.VW = Canvas ? Canvas->ClipX : (float)VX;
		L.VH = Canvas ? Canvas->ClipY : (float)VY;
		L.BoxX = Margin;
		L.BoxY = Margin + HeaderH;
		L.BoxW = FMath::Max(L.VW - Margin * 2.0f, 200.0f);
		L.BoxH = FMath::Max(L.VH - Margin * 2.0f - FooterH, 200.0f);
		L.PanelWidth = FMath::Min(PanelW, L.BoxW * 0.4f);
		L.PanelX = L.BoxX + L.BoxW - L.PanelWidth - PanelGap;
		L.PanelY = L.BoxY + PanelGap;
		L.PanelH = L.BoxH - PanelGap * 2.0f;
		L.AreaX = L.BoxX;
		L.AreaW = L.PanelX - L.BoxX - PanelGap;
		return L;
	}

	bool GetMouse(APlayerController* PC, FVector2D& Out)
	{
		float X = 0, Y = 0;
		if (PC && PC->GetMousePosition(X, Y))
		{
			Out = FVector2D(X, Y);
			return true;
		}
		return false;
	}

	bool Near(const FVector2D& A, const FVector2D& B, float R)
	{
		return FVector2D::DistSquared(A, B) <= R * R;
	}

	FString Upper(const FText& T) { return T.ToString().ToUpper(); }

	FString JoinSystemNames(const UGalaxySubsystem* Galaxy, const TArray<FName>& Ids)
	{
		TArray<FString> Names;
		for (const FName Id : Ids)
		{
			const FStarSystemDef* S = Galaxy->FindSystem(Id);
			Names.Add(S ? S->Name.ToString() : Id.ToString());
		}
		return Names.Num() ? FString::Join(Names, TEXT(", ")) : FString(TEXT("none"));
	}

	FString JoinSectorNames(const UGalaxySubsystem* Galaxy, const TArray<FName>& Ids)
	{
		TArray<FString> Names;
		for (const FName Id : Ids)
		{
			const FGalaxySectorDef* S = Galaxy->FindSector(Id);
			Names.Add(S ? S->Name.ToString() : Id.ToString());
		}
		return Names.Num() ? FString::Join(Names, TEXT(", ")) : FString(TEXT("none"));
	}

	// adastrea.Map [sector|system|universe|off]: open the map on a layer (for scripted checks / screenshots).
	static FAutoConsoleCommandWithWorldAndArgs GMapViewCmd(
		TEXT("adastrea.Map"),
		TEXT("Open the full-screen map on a layer: adastrea.Map sector|system|universe|off"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
			AAdastreaHUD* HUD = PC ? Cast<AAdastreaHUD>(PC->GetHUD()) : nullptr;
			if (!HUD)
			{
				return;
			}
			const FString Arg = Args.Num() ? Args[0].ToLower() : FString(TEXT("universe"));
			if (Arg == TEXT("off"))
			{
				HUD->SetMapVisible(false);
				return;
			}
			HUD->SetMapVisible(true);
			HUD->SetMapView(Arg == TEXT("sector") ? EAdastreaMapView::Sector
				: Arg == TEXT("system") ? EAdastreaMapView::System : EAdastreaMapView::Universe);
		}));

	/** "/Game/Maps/TestLevel" -> "TestLevel". */
	FString LevelShortName(const FString& Level)
	{
		return FPaths::GetBaseFilename(Level);
	}
}

// ---------------------------------------------------------------------------
// State / navigation
// ---------------------------------------------------------------------------

FName AAdastreaHUD::GetMapCurrentSectorId() const
{
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	return Galaxy ? Galaxy->ResolveCurrentSectorId(this) : NAME_None;
}

FName AAdastreaHUD::GetMapCurrentSystemId() const
{
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	if (!Galaxy)
	{
		return NAME_None;
	}
	if (const FGalaxySectorDef* Sector = Galaxy->FindSector(Galaxy->ResolveCurrentSectorId(this)))
	{
		return Sector->SystemId;
	}
	return Galaxy->GetStartSystemId();
}

void AAdastreaHUD::SetMapView(EAdastreaMapView NewView)
{
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	if (NewView != EAdastreaMapView::Sector)
	{
		if (!Galaxy || !Galaxy->FindSystem(MapSystemId))
		{
			MapSystemId = GetMapCurrentSystemId();
		}
		if (NewView == EAdastreaMapView::Universe && (!Galaxy || !Galaxy->FindSystem(MapSelectedSystemId)))
		{
			MapSelectedSystemId = MapSystemId;
		}
	}
	MapView = NewView;
}

void AAdastreaHUD::MapViewUp()
{
	if (MapView == EAdastreaMapView::Sector)
	{
		// Coming up from the level: show its system with the current sector picked.
		MapSystemId = GetMapCurrentSystemId();
		MapSelectedSectorId = GetMapCurrentSectorId();
		SetMapView(EAdastreaMapView::System);
	}
	else if (MapView == EAdastreaMapView::System)
	{
		MapSelectedSystemId = MapSystemId;
		SetMapView(EAdastreaMapView::Universe);
	}
}

void AAdastreaHUD::MapDrillDown()
{
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	if (!Galaxy)
	{
		return;
	}
	if (MapView == EAdastreaMapView::Universe && Galaxy->FindSystem(MapSelectedSystemId))
	{
		MapSystemId = MapSelectedSystemId;
		MapSelectedSectorId = NAME_None;
		SetMapView(EAdastreaMapView::System);
	}
	else if (MapView == EAdastreaMapView::System && !MapSelectedSectorId.IsNone()
		&& MapSelectedSectorId == GetMapCurrentSectorId())
	{
		SetMapView(EAdastreaMapView::Sector);
	}
}

void AAdastreaHUD::AddMapHit(float X, float Y, float W, float H, EMapHitKind Kind, FName Id, EAdastreaMapView View)
{
	FMapHitRect& Hit = MapHitRects.AddDefaulted_GetRef();
	Hit.Min = FVector2D(X, Y);
	Hit.Max = FVector2D(X + W, Y + H);
	Hit.Kind = Kind;
	Hit.Id = Id;
	Hit.View = View;
}

const AAdastreaHUD::FMapHitRect* AAdastreaHUD::FindMapHit(const FVector2D& P) const
{
	for (int32 i = MapHitRects.Num() - 1; i >= 0; --i)
	{
		const FMapHitRect& H = MapHitRects[i];
		if (P.X >= H.Min.X && P.X <= H.Max.X && P.Y >= H.Min.Y && P.Y <= H.Max.Y)
		{
			return &H;
		}
	}
	return nullptr;
}

bool AAdastreaHUD::HandleMapClick(const FVector2D& ScreenPos)
{
	if (!bShowMap)
	{
		return false;
	}

	const FMapHitRect* Hit = FindMapHit(ScreenPos);
	if (!Hit)
	{
		// Empty space clears the selection on the galaxy layers; the Sector
		// view falls through to the controller's station targeting.
		if (MapView == EAdastreaMapView::System) { MapSelectedSectorId = NAME_None; return true; }
		if (MapView == EAdastreaMapView::Universe) { MapSelectedSystemId = NAME_None; return true; }
		return false;
	}

	const FMapHitRect Clicked = *Hit; // SetMapView etc. don't touch MapHitRects, but copy anyway
	switch (Clicked.Kind)
	{
	case EMapHitKind::Tab:
		if (Clicked.View == EAdastreaMapView::System && MapView == EAdastreaMapView::Sector)
		{
			MapViewUp(); // picks the current sector
		}
		else
		{
			SetMapView(Clicked.View);
		}
		break;

	case EMapHitKind::System:
		if (MapSelectedSystemId == Clicked.Id)
		{
			MapSelectedSystemId = Clicked.Id;
			MapDrillDown(); // second click opens it
		}
		else
		{
			MapSelectedSystemId = Clicked.Id;
		}
		break;

	case EMapHitKind::Sector:
		if (MapSelectedSectorId == Clicked.Id)
		{
			MapDrillDown(); // second click on the loaded sector opens the Sector view
		}
		else
		{
			MapSelectedSectorId = Clicked.Id;
		}
		break;

	case EMapHitKind::JumpExit:
		MapSystemId = Clicked.Id;
		MapSelectedSystemId = Clicked.Id;
		MapSelectedSectorId = NAME_None;
		break;

	case EMapHitKind::OpenSystem:
		MapSelectedSystemId = Clicked.Id;
		MapView = EAdastreaMapView::Universe;
		MapDrillDown();
		break;

	case EMapHitKind::OpenSector:
		SetMapView(EAdastreaMapView::Sector);
		break;
	}

	UAudioEventLibrary::PlayEvent2D(this, TEXT("UI.Click"), 0.08f);
	return true;
}

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

void AAdastreaHUD::DrawCircleOutline(float X, float Y, float R, const FLinearColor& Color, float Thick, int32 Segments)
{
	Segments = FMath::Max(Segments, 8);
	float PrevX = X + R, PrevY = Y;
	for (int32 i = 1; i <= Segments; ++i)
	{
		const float A = (2.0f * PI * i) / Segments;
		const float NX = X + FMath::Cos(A) * R;
		const float NY = Y + FMath::Sin(A) * R;
		DrawLine(PrevX, PrevY, NX, NY, Color, Thick);
		PrevX = NX;
		PrevY = NY;
	}
}

void AAdastreaHUD::DrawFilledDisc(float X, float Y, float R, const FLinearColor& Color)
{
	const int32 Rows = FMath::CeilToInt(R);
	for (int32 Dy = -Rows; Dy <= Rows; ++Dy)
	{
		const float Half = FMath::Sqrt(FMath::Max(R * R - (float)(Dy * Dy), 0.0f));
		if (Half > 0.0f)
		{
			DrawRect(Color, X - Half, Y + Dy, Half * 2.0f, 1.0f);
		}
	}
}

float AAdastreaHUD::DrawWrappedText(const FString& Text, const FLinearColor& Color, float X, float Y, float MaxW, UFont* Font, float Scale)
{
	TArray<FString> Words;
	Text.ParseIntoArrayWS(Words);
	float LineH = 12.0f;
	FString Line;
	auto Flush = [&]()
	{
		if (!Line.IsEmpty())
		{
			float W = 0, H = 0;
			GetTextSize(Line, W, H, Font, Scale);
			LineH = FMath::Max(H, 8.0f);
			DrawText(Line, Color, X, Y, Font, Scale);
			Y += LineH + 2.0f;
			Line.Reset();
		}
	};
	for (const FString& Word : Words)
	{
		const FString Candidate = Line.IsEmpty() ? Word : Line + TEXT(" ") + Word;
		float W = 0, H = 0;
		GetTextSize(Candidate, W, H, Font, Scale);
		if (W > MaxW && !Line.IsEmpty())
		{
			Flush();
			Line = Word;
		}
		else
		{
			Line = Candidate;
		}
	}
	Flush();
	return Y;
}

void AAdastreaHUD::DrawMapInfoPanel(float X, float Y, float W, float H, const FString& Heading, const FLinearColor& Accent)
{
	DrawRect(FLinearColor(0.02f, 0.035f, 0.05f, 0.94f), X, Y, W, H);
	DrawRect(Accent, X, Y, W, 3.0f);
	DrawRect(FLinearColor(Accent.R, Accent.G, Accent.B, 0.35f), X, Y + H - 1.0f, W, 1.0f);
	DrawRect(FLinearColor(Accent.R, Accent.G, Accent.B, 0.35f), X, Y, 1.0f, H);
	DrawRect(FLinearColor(Accent.R, Accent.G, Accent.B, 0.35f), X + W - 1.0f, Y, 1.0f, H);
	DrawText(Heading, Accent, X + 14.0f, Y + 12.0f, HudType::Font(), HudType::Heading);
}

float AAdastreaHUD::DrawMapButton(const FString& Label, float X, float Y, bool bHover, bool bActive, EMapHitKind Kind, FName Id, EAdastreaMapView View)
{
	UFont* Font = HudType::Font();
	const float Scale = HudType::Label;
	float TW = 0, TH = 0;
	GetTextSize(Label, TW, TH, Font, Scale);
	const float W = TW + 24.0f;
	const float H = 24.0f;

	const FLinearColor Fill = bActive ? FLinearColor(0.10f, 0.55f, 0.60f, 0.95f)
		: bHover ? FLinearColor(0.08f, 0.22f, 0.27f, 0.95f)
		: FLinearColor(0.03f, 0.08f, 0.11f, 0.9f);
	const FLinearColor Edge = (bActive || bHover) ? GalaxyMap::Title : GalaxyMap::BoxEdge;
	const FLinearColor TextCol = bActive ? FLinearColor(0.01f, 0.05f, 0.06f, 1.0f) : GalaxyMap::Title;

	DrawRect(Fill, X, Y, W, H);
	DrawLine(X, Y, X + W, Y, Edge, 1.0f);
	DrawLine(X + W, Y, X + W, Y + H, Edge, 1.0f);
	DrawLine(X + W, Y + H, X, Y + H, Edge, 1.0f);
	DrawLine(X, Y + H, X, Y, Edge, 1.0f);
	DrawText(Label, TextCol, X + 12.0f, Y + (H - TH) * 0.5f, Font, Scale);

	AddMapHit(X, Y, W, H, Kind, Id, View);
	return W;
}

void AAdastreaHUD::DrawMapHeader(APlayerController* PC, float VW, const FString& Title, const FString& Breadcrumb)
{
	using namespace GalaxyMap;
	DrawText(Title, GalaxyMap::Title, Margin + 16.0f, 10.0f, HudType::Font(), HudType::Title);
	DrawText(Breadcrumb, Dim, Margin + 16.0f, 44.0f, HudType::Font(), HudType::Label);

	// Layer tabs, centred at the top.
	struct FTab { const TCHAR* Label; EAdastreaMapView View; };
	const FTab Tabs[] = {
		{ TEXT("3  SECTOR"),   EAdastreaMapView::Sector },
		{ TEXT("4  SYSTEM"),   EAdastreaMapView::System },
		{ TEXT("5  UNIVERSE"), EAdastreaMapView::Universe },
	};
	UFont* Font = HudType::Font();
	float Total = 0.0f;
	for (const FTab& T : Tabs)
	{
		float TW = 0, TH = 0;
		GetTextSize(T.Label, TW, TH, Font, HudType::Label);
		Total += TW + 24.0f + 6.0f;
	}
	FVector2D Mouse;
	const bool bMouse = GetMouse(PC, Mouse);
	float X = (VW - Total) * 0.5f;
	const float Y = 14.0f;
	for (const FTab& T : Tabs)
	{
		float TW = 0, TH = 0;
		GetTextSize(T.Label, TW, TH, Font, HudType::Label);
		const bool bHover = bMouse && Mouse.X >= X && Mouse.X <= X + TW + 24.0f && Mouse.Y >= Y && Mouse.Y <= Y + 24.0f;
		X += DrawMapButton(T.Label, X, Y, bHover, MapView == T.View, EMapHitKind::Tab, NAME_None, T.View) + 6.0f;
	}
}

// ---------------------------------------------------------------------------
// System view
// ---------------------------------------------------------------------------

void AAdastreaHUD::DrawSystemMap(APlayerController* PC)
{
	using namespace GalaxyMap;
	if (!PC || !GEngine)
	{
		return;
	}
	const FLayout L = MakeLayout(Canvas, PC);

	DrawRect(Backdrop, 0.0f, 0.0f, L.VW, L.VH);
	DrawRect(BoxFill, L.BoxX, L.BoxY, L.BoxW, L.BoxH);
	DrawRect(BoxEdge, L.BoxX, L.BoxY, L.BoxW, 3.0f);
	DrawRect(BoxEdge, L.BoxX, L.BoxY + L.BoxH - 3.0f, L.BoxW, 3.0f);

	UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	if (!Galaxy || !Galaxy->FindSystem(MapSystemId))
	{
		MapSystemId = GetMapCurrentSystemId();
	}
	const FStarSystemDef* Sys = Galaxy ? Galaxy->FindSystem(MapSystemId) : nullptr;
	if (!Sys)
	{
		DrawMapHeader(PC, L.VW, TEXT("SYSTEM MAP"), TEXT("Universe"));
		DrawText(TEXT("No galaxy data (Content/Data/Universe/Galaxy.json)."), Body, L.BoxX + 24.0f, L.BoxY + 24.0f, HudType::Font(), HudType::Label);
		return;
	}

	const FName CurrentSectorId = GetMapCurrentSectorId();
	FVector2D Mouse;
	const bool bMouse = GetMouse(PC, Mouse);

	const float Cx = L.AreaX + L.AreaW * 0.5f;
	const float Cy = L.BoxY + L.BoxH * 0.5f;
	const float R = FMath::Max(60.0f, FMath::Min(L.AreaW, L.BoxH) * 0.5f - 70.0f);
	auto SectorPos = [&](const FGalaxySectorDef& S)
	{
		const float A = FMath::DegreesToRadians(S.OrbitAngle);
		return FVector2D(Cx + FMath::Cos(A) * S.OrbitRadius * R, Cy - FMath::Sin(A) * S.OrbitRadius * R);
	};

	// ---- System edge and orbit rings ----
	DrawCircleOutline(Cx, Cy, R, FLinearColor(0.2f, 0.45f, 0.55f, 0.6f), 1.5f, 128);
	TArray<int32> Rings;
	for (const FGalaxySectorDef& S : Sys->Sectors)
	{
		Rings.AddUnique(FMath::RoundToInt(S.OrbitRadius * 100.0f));
	}
	for (const int32 Ring : Rings)
	{
		DrawCircleOutline(Cx, Cy, R * Ring / 100.0f, FLinearColor(0.15f, 0.28f, 0.36f, 0.5f), 1.0f, 96);
	}

	// ---- Star ----
	{
		const FLinearColor C = Sys->StarColor;
		DrawFilledDisc(Cx, Cy, 34.0f, FLinearColor(C.R, C.G, C.B, 0.07f));
		DrawFilledDisc(Cx, Cy, 24.0f, FLinearColor(C.R, C.G, C.B, 0.18f));
		DrawFilledDisc(Cx, Cy, 15.0f, FLinearColor(C.R, C.G, C.B, 1.0f));
		DrawFilledDisc(Cx, Cy, 7.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.85f));
		const FString StarLabel = FString::Printf(TEXT("%s  [%s]"), *Sys->Name.ToString(), *Sys->StarClass);
		float TW = 0, TH = 0;
		GetTextSize(StarLabel, TW, TH, HudType::Font(), HudType::Caption);
		DrawText(StarLabel, FLinearColor(C.R, C.G, C.B, 0.95f), Cx - TW * 0.5f, Cy - 38.0f - TH, HudType::Font(), HudType::Caption); // above: sectors sit below/around
	}

	// ---- Jump-lane exits at the rim, pointing towards each linked system ----
	TMap<FName, FVector2D> LaneExitBase; // system id -> rim point (lane-gate lines end here)
	for (const FName LinkId : Sys->JumpLinks)
	{
		const FStarSystemDef* Other = Galaxy->FindSystem(LinkId);
		if (!Other)
		{
			continue;
		}
		FVector2D Dir = (Other->Position - Sys->Position).GetSafeNormal();
		if (Dir.IsNearlyZero()) { Dir = FVector2D(1.0f, 0.0f); }
		const FVector2D S(Dir.X, -Dir.Y); // universe +Y is screen up
		const FVector2D Tip = FVector2D(Cx, Cy) + S * (R + 22.0f);
		const FVector2D Base = FVector2D(Cx, Cy) + S * (R + 8.0f);
		const FVector2D Perp(-S.Y, S.X);
		LaneExitBase.Add(LinkId, Base);

		const FString Label = FString::Printf(TEXT("%s  >"), *Other->Name.ToString().ToUpper());
		float TW = 0, TH = 0;
		GetTextSize(Label, TW, TH, HudType::Font(), HudType::Label);
		const FVector2D TextAnchor = Tip + S * 8.0f;
		const float TX = S.X >= 0.0f ? TextAnchor.X : TextAnchor.X - TW;
		const float TY = TextAnchor.Y - TH * 0.5f;

		const float HitX = FMath::Min(TX, FMath::Min(Base.X, Tip.X)) - 6.0f;
		const float HitY = FMath::Min(TY, FMath::Min(Base.Y, Tip.Y)) - 6.0f;
		const float HitW = FMath::Max(TX + TW, FMath::Max(Base.X, Tip.X)) + 6.0f - HitX;
		const float HitH = FMath::Max(TY + TH, FMath::Max(Base.Y, Tip.Y)) + 6.0f - HitY;
		const bool bHover = bMouse && Mouse.X >= HitX && Mouse.X <= HitX + HitW && Mouse.Y >= HitY && Mouse.Y <= HitY + HitH;

		const FLinearColor Col = bHover ? LaneHot : FLinearColor(0.45f, 0.75f, 0.9f, 0.9f);
		DrawLine(Base.X + Perp.X * 7.0f, Base.Y + Perp.Y * 7.0f, Tip.X, Tip.Y, Col, 2.0f);
		DrawLine(Base.X - Perp.X * 7.0f, Base.Y - Perp.Y * 7.0f, Tip.X, Tip.Y, Col, 2.0f);
		DrawLine(Base.X + Perp.X * 7.0f, Base.Y + Perp.Y * 7.0f, Base.X - Perp.X * 7.0f, Base.Y - Perp.Y * 7.0f, Col, 2.0f);
		DrawText(Label, Col, TX, TY, HudType::Font(), HudType::Label);
		AddMapHit(HitX, HitY, HitW, HitH, EMapHitKind::JumpExit, LinkId);
	}

	// ---- Gates (each pair once) ----
	for (const FGalaxySectorDef& S : Sys->Sectors)
	{
		for (const FName GateId : S.Gates)
		{
			if (S.Id.Compare(GateId) >= 0)
			{
				continue;
			}
			const FGalaxySectorDef* To = Galaxy->FindSector(GateId);
			if (!To)
			{
				continue;
			}
			const FVector2D A = SectorPos(S);
			const FVector2D B = SectorPos(*To);
			const bool bHot = S.Id == MapSelectedSectorId || GateId == MapSelectedSectorId;
			const FLinearColor Col = bHot ? FLinearColor(1.0f, 0.8f, 0.35f, 0.95f) : Gate;
			DrawLine(A.X, A.Y, B.X, B.Y, Col, bHot ? 2.5f : 1.5f);
			const FVector2D M = (A + B) * 0.5f;
			DrawLine(M.X - 4.0f, M.Y, M.X, M.Y - 4.0f, Col, 1.5f);
			DrawLine(M.X, M.Y - 4.0f, M.X + 4.0f, M.Y, Col, 1.5f);
			DrawLine(M.X + 4.0f, M.Y, M.X, M.Y + 4.0f, Col, 1.5f);
			DrawLine(M.X, M.Y + 4.0f, M.X - 4.0f, M.Y, Col, 1.5f);
		}
	}

	// ---- Lane gates: sector -> rim exit of the system they jump to ----
	for (const FGalaxySectorDef& S : Sys->Sectors)
	{
		for (const FName LaneId : S.LaneGates)
		{
			const FGalaxySectorDef* To = Galaxy->FindSector(LaneId);
			const FVector2D* Exit = To ? LaneExitBase.Find(To->SystemId) : nullptr;
			if (!Exit)
			{
				continue;
			}
			const FVector2D A = SectorPos(S);
			const bool bHot = S.Id == MapSelectedSectorId;
			DrawLine(A.X, A.Y, Exit->X, Exit->Y, bHot ? FLinearColor(0.8f, 0.62f, 1.0f, 0.95f) : FLinearColor(0.7f, 0.5f, 1.0f, 0.5f), bHot ? 2.5f : 1.5f);
		}
	}

	// ---- Sectors ----
	for (const FGalaxySectorDef& S : Sys->Sectors)
	{
		const FVector2D P = SectorPos(S);
		const bool bBuilt = S.HasLevel();
		const bool bCurrent = S.Id == CurrentSectorId;
		const bool bSelected = S.Id == MapSelectedSectorId;
		const bool bHover = bMouse && Near(Mouse, P, 16.0f);
		const FLinearColor Col = bBuilt ? Built : Planned;
		const float HexR = bHover || bSelected ? 12.0f : 10.0f;

		// Hexagon outline (+ filled core when a level exists).
		for (int32 i = 0; i < 6; ++i)
		{
			const float A0 = FMath::DegreesToRadians(60.0f * i + 30.0f);
			const float A1 = FMath::DegreesToRadians(60.0f * (i + 1) + 30.0f);
			DrawLine(P.X + FMath::Cos(A0) * HexR, P.Y + FMath::Sin(A0) * HexR,
				P.X + FMath::Cos(A1) * HexR, P.Y + FMath::Sin(A1) * HexR, Col, bBuilt ? 2.0f : 1.5f);
		}
		if (bBuilt)
		{
			DrawFilledDisc(P.X, P.Y, 4.5f, Col);
		}
		if (bCurrent)
		{
			DrawCircleOutline(P.X, P.Y, 18.0f, You, 2.0f, 32);
			// Backing plate so gate/lane lines don't run through the tag.
			float YW = 0.0f, YH = 0.0f;
			GetTextSize(TEXT("YOU ARE HERE"), YW, YH, HudType::Font(), HudType::Caption);
			DrawRect(BoxFill, P.X - YW * 0.5f - 3.0f, P.Y - 34.0f - 1.0f, YW + 6.0f, YH + 2.0f);
			DrawText(TEXT("YOU ARE HERE"), You, P.X - YW * 0.5f, P.Y - 34.0f, HudType::Font(), HudType::Caption);
		}
		if (bSelected)
		{
			const float B = 16.0f, K = 6.0f;
			DrawLine(P.X - B, P.Y - B, P.X - B + K, P.Y - B, Select, 2.0f); DrawLine(P.X - B, P.Y - B, P.X - B, P.Y - B + K, Select, 2.0f);
			DrawLine(P.X + B, P.Y - B, P.X + B - K, P.Y - B, Select, 2.0f); DrawLine(P.X + B, P.Y - B, P.X + B, P.Y - B + K, Select, 2.0f);
			DrawLine(P.X - B, P.Y + B, P.X - B + K, P.Y + B, Select, 2.0f); DrawLine(P.X - B, P.Y + B, P.X - B, P.Y + B - K, Select, 2.0f);
			DrawLine(P.X + B, P.Y + B, P.X + B - K, P.Y + B, Select, 2.0f); DrawLine(P.X + B, P.Y + B, P.X + B, P.Y + B - K, Select, 2.0f);
		}
		const FLinearColor LabelCol = (bHover || bSelected) ? FLinearColor::White : Body;
		const FString NameStr = S.Name.ToString();
		const FString TypeStr = bBuilt ? S.Type : FString::Printf(TEXT("%s  (planned)"), *S.Type);
		// Backing plate so gate/lane lines don't run through the label.
		float NW = 0.0f, NH = 0.0f, SW = 0.0f, SH = 0.0f;
		GetTextSize(NameStr, NW, NH, HudType::Font(), HudType::Label);
		GetTextSize(TypeStr, SW, SH, HudType::Font(), HudType::Caption);
		DrawRect(BoxFill, P.X + 17.0f, P.Y - 13.0f, FMath::Max(NW, SW) + 6.0f, 14.0f + SH + 2.0f);
		DrawText(NameStr, LabelCol, P.X + 20.0f, P.Y - 12.0f, HudType::Font(), HudType::Label);
		DrawText(TypeStr, Dim, P.X + 20.0f, P.Y + 2.0f, HudType::Font(), HudType::Caption);
		AddMapHit(P.X - 16.0f, P.Y - 16.0f, 32.0f, 32.0f, EMapHitKind::Sector, S.Id);
	}

	if (Sys->Sectors.Num() == 0)
	{
		DrawText(TEXT("No sectors defined for this system yet."), Dim, Cx - 120.0f, Cy + 60.0f, HudType::Font(), HudType::Label);
	}

	// ---- Info panel ----
	{
		const float PX = L.PanelX, PW = L.PanelWidth, TX = PX + 14.0f, TW = PW - 28.0f;
		DrawMapInfoPanel(PX, L.PanelY, PW, L.PanelH, FString::Printf(TEXT("%s SYSTEM"), *Upper(Sys->Name)), Sys->StarColor);
		float Y = L.PanelY + 44.0f;
		DrawText(FString::Printf(TEXT("Star  %s"), *Sys->StarClass), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
		if (!Sys->Faction.IsEmpty()) { DrawText(FString::Printf(TEXT("Faction  %s"), *Sys->Faction), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f; }
		DrawText(FString::Printf(TEXT("Sectors  %d  (%d built)"), Sys->Sectors.Num(), Sys->NumBuiltSectors()), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
		Y = DrawWrappedText(FString::Printf(TEXT("Jump lanes  %s"), *JoinSystemNames(Galaxy, Sys->JumpLinks)), Body, TX, Y, TW, HudType::Font(), HudType::Label);
		Y = DrawWrappedText(Sys->Description.ToString(), Dim, TX, Y + 4.0f, TW, HudType::Font(), HudType::Label);
		Y += 8.0f;
		DrawRect(FLinearColor(0.2f, 0.4f, 0.5f, 0.5f), TX, Y, TW, 1.0f);
		Y += 10.0f;

		if (const FGalaxySectorDef* Sel = Galaxy->FindSector(MapSelectedSectorId))
		{
			const bool bBuilt = Sel->HasLevel();
			DrawText(Upper(Sel->Name), bBuilt ? Built : Planned, TX, Y, HudType::Font(), HudType::Body); Y += 22.0f;
			DrawText(FString::Printf(TEXT("%s   Security: %s"), *Sel->Type, Sel->Security.IsEmpty() ? TEXT("?") : *Sel->Security), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
			if (!Sel->Faction.IsEmpty()) { DrawText(FString::Printf(TEXT("Faction  %s"), *Sel->Faction), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f; }
			DrawText(bBuilt ? FString::Printf(TEXT("Level  %s"), *LevelShortName(Sel->Level)) : FString(TEXT("PLANNED - no level yet")),
				bBuilt ? Built : FLinearColor(0.95f, 0.7f, 0.35f, 1.0f), TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
			Y = DrawWrappedText(FString::Printf(TEXT("Gates  %s"), *JoinSectorNames(Galaxy, Sel->Gates)), Body, TX, Y, TW, HudType::Font(), HudType::Label);
			if (Sel->LaneGates.Num())
			{
				TArray<FString> Lanes;
				for (const FName LaneId : Sel->LaneGates)
				{
					const FGalaxySectorDef* To = Galaxy->FindSector(LaneId);
					const FStarSystemDef* ToSys = To ? Galaxy->FindSystem(To->SystemId) : nullptr;
					Lanes.Add(To ? FString::Printf(TEXT("%s (%s)"), *To->Name.ToString(), ToSys ? *ToSys->Name.ToString() : TEXT("?")) : LaneId.ToString());
				}
				Y = DrawWrappedText(FString::Printf(TEXT("Lane gates  %s"), *FString::Join(Lanes, TEXT(", "))),
					FLinearColor(0.8f, 0.65f, 1.0f, 1.0f), TX, Y, TW, HudType::Font(), HudType::Label);
			}
			Y = DrawWrappedText(Sel->Description.ToString(), Dim, TX, Y + 4.0f, TW, HudType::Font(), HudType::Label);
			Y += 10.0f;
			if (Sel->Id == CurrentSectorId)
			{
				const bool bHover = bMouse && Mouse.X >= TX && Mouse.X <= TX + 180.0f && Mouse.Y >= Y && Mouse.Y <= Y + 24.0f;
				DrawMapButton(TEXT("VIEW SECTOR MAP"), TX, Y, bHover, false, EMapHitKind::OpenSector, Sel->Id);
			}
			else if (!CurrentSectorId.IsNone())
			{
				// Gate route from the player's sector.
				const TArray<FName> Route = Galaxy->FindRoute(CurrentSectorId, Sel->Id);
				if (Route.Num())
				{
					Y = DrawWrappedText(FString::Printf(TEXT("Route (%d jump%s)  %s"), Route.Num(), Route.Num() == 1 ? TEXT("") : TEXT("s"),
						*JoinSectorNames(Galaxy, Route).Replace(TEXT(", "), TEXT(" > "))), FLinearColor(0.8f, 0.65f, 1.0f, 1.0f), TX, Y, TW, HudType::Font(), HudType::Caption);
					if (!bBuilt)
					{
						Y = DrawWrappedText(TEXT("Its gate is offline until the sector has a level."), Dim, TX, Y, TW, HudType::Font(), HudType::Caption);
					}
				}
				else
				{
					Y = DrawWrappedText(TEXT("No gate route from your sector."), Dim, TX, Y, TW, HudType::Font(), HudType::Caption);
				}
			}
		}
		else
		{
			DrawText(TEXT("Click a sector for details."), Dim, TX, Y, HudType::Font(), HudType::Label);
		}

		// Legend (panel bottom).
		const float LY = L.PanelY + L.PanelH - 96.0f;
		DrawRect(FLinearColor(0.2f, 0.4f, 0.5f, 0.5f), TX, LY - 8.0f, TW, 1.0f);
		DrawCircleOutline(TX + 6.0f, LY + 6.0f, 6.0f, Built, 2.0f, 6); DrawFilledDisc(TX + 6.0f, LY + 6.0f, 2.5f, Built);
		DrawText(TEXT("Sector with a level"), Body, TX + 20.0f, LY, HudType::Font(), HudType::Caption);
		DrawCircleOutline(TX + 6.0f, LY + 24.0f, 6.0f, Planned, 1.5f, 6);
		DrawText(TEXT("Planned sector"), Body, TX + 20.0f, LY + 18.0f, HudType::Font(), HudType::Caption);
		DrawLine(TX, LY + 42.0f, TX + 13.0f, LY + 42.0f, Gate, 2.0f);
		DrawText(TEXT("Gate"), Body, TX + 20.0f, LY + 36.0f, HudType::Font(), HudType::Caption);
		DrawCircleOutline(TX + 6.0f, LY + 60.0f, 6.0f, You, 2.0f, 16);
		DrawText(TEXT("Your sector"), Body, TX + 20.0f, LY + 54.0f, HudType::Font(), HudType::Caption);
		DrawLine(TX, LY + 78.0f, TX + 13.0f, LY + 78.0f, FLinearColor(0.7f, 0.5f, 1.0f, 0.9f), 2.0f);
		DrawText(TEXT("Lane gate (to another system)"), Body, TX + 20.0f, LY + 72.0f, HudType::Font(), HudType::Caption);
	}

	DrawMapHeader(PC, L.VW, TEXT("SYSTEM MAP"), FString::Printf(TEXT("Universe  >  %s"), *Sys->Name.ToString()));
	DrawCentredText(TEXT("LMB select   click again / [Enter] open   rim arrows: next system   [Backspace] up   [3] sector   [5]/[U] universe   [M] close"), Help, L.VW * 0.5f, L.VH - 30.0f, HudType::Label);
}

// ---------------------------------------------------------------------------
// Universe view
// ---------------------------------------------------------------------------

void AAdastreaHUD::DrawUniverseMap(APlayerController* PC)
{
	using namespace GalaxyMap;
	if (!PC || !GEngine)
	{
		return;
	}
	const FLayout L = MakeLayout(Canvas, PC);

	DrawRect(Backdrop, 0.0f, 0.0f, L.VW, L.VH);
	DrawRect(BoxFill, L.BoxX, L.BoxY, L.BoxW, L.BoxH);
	DrawRect(BoxEdge, L.BoxX, L.BoxY, L.BoxW, 3.0f);
	DrawRect(BoxEdge, L.BoxX, L.BoxY + L.BoxH - 3.0f, L.BoxW, 3.0f);

	UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	if (!Galaxy || Galaxy->GetSystems().Num() == 0)
	{
		DrawMapHeader(PC, L.VW, TEXT("UNIVERSE MAP"), TEXT("Universe"));
		DrawText(TEXT("No galaxy data (Content/Data/Universe/Galaxy.json)."), Body, L.BoxX + 24.0f, L.BoxY + 24.0f, HudType::Font(), HudType::Label);
		return;
	}
	const TArray<FStarSystemDef>& Systems = Galaxy->GetSystems();
	const FName CurrentSystemId = GetMapCurrentSystemId();
	FVector2D Mouse;
	const bool bMouse = GetMouse(PC, Mouse);

	// ---- Fit every system into the map area ----
	FVector2D Min(TNumericLimits<float>::Max()), Max(-TNumericLimits<float>::Max());
	for (const FStarSystemDef& S : Systems)
	{
		Min.X = FMath::Min(Min.X, S.Position.X); Min.Y = FMath::Min(Min.Y, S.Position.Y);
		Max.X = FMath::Max(Max.X, S.Position.X); Max.Y = FMath::Max(Max.Y, S.Position.Y);
	}
	const FVector2D Mid = (Min + Max) * 0.5f;
	const float ExtX = FMath::Max(Max.X - Min.X, 10.0f);
	const float ExtY = FMath::Max(Max.Y - Min.Y, 10.0f);
	const float Pad = 90.0f;
	const float Scale = FMath::Max(FMath::Min((L.AreaW - Pad * 2.0f) / ExtX, (L.BoxH - Pad * 2.0f) / ExtY), 1.0f);
	const float Cx = L.AreaX + L.AreaW * 0.5f;
	const float Cy = L.BoxY + L.BoxH * 0.5f;
	auto ToScreen = [&](const FVector2D& P) { return FVector2D(Cx + (P.X - Mid.X) * Scale, Cy - (P.Y - Mid.Y) * Scale); };

	// ---- Light-year grid (5 ly) ----
	{
		const float Step = 5.0f;
		const float Left = Mid.X - (Cx - L.AreaX) / Scale, Right = Mid.X + (L.AreaX + L.AreaW - Cx) / Scale;
		const float Top = Mid.Y + (Cy - L.BoxY) / Scale, Bottom = Mid.Y - (L.BoxY + L.BoxH - Cy) / Scale;
		const FLinearColor GridCol(0.1f, 0.17f, 0.22f, 0.55f);
		for (float X = FMath::CeilToFloat(Left / Step) * Step; X <= Right; X += Step)
		{
			const float SX = ToScreen(FVector2D(X, 0)).X;
			DrawLine(SX, L.BoxY + 3.0f, SX, L.BoxY + L.BoxH - 3.0f, GridCol, 1.0f);
		}
		for (float Y = FMath::CeilToFloat(Bottom / Step) * Step; Y <= Top; Y += Step)
		{
			const float SY = ToScreen(FVector2D(0, Y)).Y;
			DrawLine(L.AreaX, SY, L.AreaX + L.AreaW, SY, GridCol, 1.0f);
		}
		// Scale bar.
		const float BarX = L.AreaX + 20.0f, BarY = L.BoxY + L.BoxH - 26.0f, BarW = 10.0f * Scale;
		DrawLine(BarX, BarY, BarX + BarW, BarY, Dim, 2.0f);
		DrawLine(BarX, BarY - 4.0f, BarX, BarY + 4.0f, Dim, 2.0f);
		DrawLine(BarX + BarW, BarY - 4.0f, BarX + BarW, BarY + 4.0f, Dim, 2.0f);
		DrawText(TEXT("10 ly"), Dim, BarX + BarW + 8.0f, BarY - 7.0f, HudType::Font(), HudType::Caption);
	}

	// ---- Jump lanes (each pair once) ----
	for (const FStarSystemDef& S : Systems)
	{
		for (const FName LinkId : S.JumpLinks)
		{
			if (S.Id.Compare(LinkId) >= 0)
			{
				continue;
			}
			const FStarSystemDef* To = Galaxy->FindSystem(LinkId);
			if (!To)
			{
				continue;
			}
			const bool bHot = S.Id == MapSelectedSystemId || LinkId == MapSelectedSystemId;
			const FVector2D A = ToScreen(S.Position), B = ToScreen(To->Position);
			DrawLine(A.X, A.Y, B.X, B.Y, bHot ? LaneHot : Lane, bHot ? 2.5f : 1.5f);
		}
	}

	// ---- Systems ----
	for (const FStarSystemDef& S : Systems)
	{
		const FVector2D P = ToScreen(S.Position);
		const float CoreR = 5.0f + FMath::Min(S.Sectors.Num(), 5) * 1.2f;
		const bool bCurrent = S.Id == CurrentSystemId;
		const bool bSelected = S.Id == MapSelectedSystemId;
		const bool bHover = bMouse && Near(Mouse, P, FMath::Max(CoreR + 6.0f, 14.0f));
		const FLinearColor C = S.StarColor;

		DrawFilledDisc(P.X, P.Y, CoreR * 2.6f, FLinearColor(C.R, C.G, C.B, bHover ? 0.16f : 0.08f));
		DrawFilledDisc(P.X, P.Y, CoreR * 1.6f, FLinearColor(C.R, C.G, C.B, 0.22f));
		DrawFilledDisc(P.X, P.Y, CoreR, FLinearColor(C.R, C.G, C.B, S.Sectors.Num() ? 1.0f : 0.55f));
		if (bCurrent)
		{
			DrawCircleOutline(P.X, P.Y, CoreR + 9.0f, You, 2.0f, 40);
			DrawText(TEXT("YOU"), You, P.X - 10.0f, P.Y - CoreR - 34.0f, HudType::Font(), HudType::Caption);
		}
		if (bSelected)
		{
			const float B = CoreR + 14.0f, K = 7.0f;
			DrawLine(P.X - B, P.Y - B, P.X - B + K, P.Y - B, Select, 2.0f); DrawLine(P.X - B, P.Y - B, P.X - B, P.Y - B + K, Select, 2.0f);
			DrawLine(P.X + B, P.Y - B, P.X + B - K, P.Y - B, Select, 2.0f); DrawLine(P.X + B, P.Y - B, P.X + B, P.Y - B + K, Select, 2.0f);
			DrawLine(P.X - B, P.Y + B, P.X - B + K, P.Y + B, Select, 2.0f); DrawLine(P.X - B, P.Y + B, P.X - B, P.Y + B - K, Select, 2.0f);
			DrawLine(P.X + B, P.Y + B, P.X + B - K, P.Y + B, Select, 2.0f); DrawLine(P.X + B, P.Y + B, P.X + B, P.Y + B - K, Select, 2.0f);
		}

		const FString Name = S.Name.ToString().ToUpper();
		float TW = 0, TH = 0;
		GetTextSize(Name, TW, TH, HudType::Font(), HudType::Label);
		DrawText(Name, (bHover || bSelected) ? FLinearColor::White : Body, P.X - TW * 0.5f, P.Y + CoreR + 24.0f, HudType::Font(), HudType::Label);
		const FString Sub = S.Sectors.Num()
			? FString::Printf(TEXT("%d sectors, %d built"), S.Sectors.Num(), S.NumBuiltSectors())
			: FString(TEXT("uncharted"));
		GetTextSize(Sub, TW, TH, HudType::Font(), HudType::Caption);
		DrawText(Sub, Dim, P.X - TW * 0.5f, P.Y + CoreR + 39.0f, HudType::Font(), HudType::Caption);

		const float HitR = FMath::Max(CoreR + 6.0f, 14.0f);
		AddMapHit(P.X - HitR, P.Y - HitR, HitR * 2.0f, HitR * 2.0f, EMapHitKind::System, S.Id);
	}

	// ---- Info panel ----
	{
		const float PX = L.PanelX, PW = L.PanelWidth, TX = PX + 14.0f, TW = PW - 28.0f;
		if (const FStarSystemDef* Sel = Galaxy->FindSystem(MapSelectedSystemId))
		{
			DrawMapInfoPanel(PX, L.PanelY, PW, L.PanelH, Upper(Sel->Name), Sel->StarColor);
			float Y = L.PanelY + 44.0f;
			DrawText(FString::Printf(TEXT("Star  %s"), *Sel->StarClass), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
			if (!Sel->Faction.IsEmpty()) { DrawText(FString::Printf(TEXT("Faction  %s"), *Sel->Faction), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f; }
			if (const FStarSystemDef* Here = Galaxy->FindSystem(CurrentSystemId))
			{
				if (Here != Sel)
				{
					DrawText(FString::Printf(TEXT("Distance  %.1f ly"), FVector2D::Distance(Here->Position, Sel->Position)), Body, TX, Y, HudType::Font(), HudType::Label);
					Y += 16.0f;
				}
			}
			Y = DrawWrappedText(FString::Printf(TEXT("Jump lanes  %s"), *JoinSystemNames(Galaxy, Sel->JumpLinks)), Body, TX, Y, TW, HudType::Font(), HudType::Label);
			Y = DrawWrappedText(Sel->Description.ToString(), Dim, TX, Y + 4.0f, TW, HudType::Font(), HudType::Label);
			Y += 8.0f;
			DrawRect(FLinearColor(0.2f, 0.4f, 0.5f, 0.5f), TX, Y, TW, 1.0f);
			Y += 10.0f;

			DrawText(FString::Printf(TEXT("SECTORS  (%d / %d built)"), Sel->NumBuiltSectors(), Sel->Sectors.Num()), GalaxyMap::Title, TX, Y, HudType::Font(), HudType::Label);
			Y += 18.0f;
			const float ListBottom = L.PanelY + L.PanelH - 48.0f;
			for (const FGalaxySectorDef& Sec : Sel->Sectors)
			{
				if (Y > ListBottom - 16.0f)
				{
					DrawText(TEXT("..."), Dim, TX, Y, HudType::Font(), HudType::Label);
					break;
				}
				const bool bBuilt = Sec.HasLevel();
				DrawFilledDisc(TX + 4.0f, Y + 6.0f, 3.0f, bBuilt ? Built : Planned);
				DrawText(Sec.Name.ToString(), Body, TX + 14.0f, Y, HudType::Font(), HudType::Caption);
				const FString Tag = bBuilt ? LevelShortName(Sec.Level) : FString(TEXT("planned"));
				float SW = 0, SH = 0;
				GetTextSize(Tag, SW, SH, HudType::Font(), HudType::Caption);
				DrawText(Tag, bBuilt ? Built : Planned, TX + TW - SW, Y + 1.0f, HudType::Font(), HudType::Caption);
				Y += 16.0f;
			}
			if (Sel->Sectors.Num() == 0)
			{
				DrawText(TEXT("None charted yet."), Dim, TX, Y, HudType::Font(), HudType::Caption);
			}

			const float BY = L.PanelY + L.PanelH - 38.0f;
			const bool bHover = bMouse && Mouse.X >= TX && Mouse.X <= TX + 190.0f && Mouse.Y >= BY && Mouse.Y <= BY + 24.0f;
			DrawMapButton(TEXT("OPEN SYSTEM MAP"), TX, BY, bHover, false, EMapHitKind::OpenSystem, Sel->Id);
		}
		else
		{
			DrawMapInfoPanel(PX, L.PanelY, PW, L.PanelH, TEXT("KNOWN SPACE"), BoxEdge);
			int32 NumSectors = 0, NumBuilt = 0;
			for (const FStarSystemDef& S : Systems) { NumSectors += S.Sectors.Num(); NumBuilt += S.NumBuiltSectors(); }
			float Y = L.PanelY + 44.0f;
			DrawText(FString::Printf(TEXT("Systems  %d"), Systems.Num()), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
			DrawText(FString::Printf(TEXT("Sectors  %d  (%d built)"), NumSectors, NumBuilt), Body, TX, Y, HudType::Font(), HudType::Label); Y += 22.0f;
			Y = DrawWrappedText(TEXT("Click a system for details. Click it again, or press Enter, to open its system map."), Dim, TX, Y, TW, HudType::Font(), HudType::Caption);
			if (!Galaxy->IsLoadedFromFile())
			{
				DrawWrappedText(TEXT("Galaxy.json is missing or invalid; showing the built-in fallback galaxy."),
					FLinearColor(0.95f, 0.6f, 0.3f, 1.0f), TX, Y + 8.0f, TW, HudType::Font(), HudType::Caption);
			}
		}
	}

	DrawMapHeader(PC, L.VW, TEXT("UNIVERSE MAP"), TEXT("Universe"));
	DrawCentredText(TEXT("LMB select   click again / [Enter] open system   [3] sector   [4] system   [M] close"), Help, L.VW * 0.5f, L.VH - 30.0f, HudType::Label);
}

// ---------------------------------------------------------------------------
// Flight HUD: jump gates
// ---------------------------------------------------------------------------

void AAdastreaHUD::DrawJumpGateMarkers(APlayerController* PC, ASpaceship* Ship)
{
	if (!PC || !Ship || !Canvas || !GEngine || Ship->IsDocked())
	{
		return;
	}
	const FVector ShipLoc = Ship->GetActorLocation();
	auto FormatDist = [](float Cm)
	{
		return Cm < 100000.0f ? FString::Printf(TEXT("%.0f m"), Cm / 100.0f) : FString::Printf(TEXT("%.1f km"), Cm / 100000.0f);
	};

	int32 ViewX = 0, ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	const float ViewToCanvas = ViewX > 0 ? Canvas->ClipX / ViewX : 1.0f;

	AJumpGate* Nearest = nullptr;
	float NearestDist = TNumericLimits<float>::Max();
	for (TActorIterator<AJumpGate> It(GetWorld()); It; ++It)
	{
		AJumpGate* Gate = *It;
		const float Dist = FVector::Dist(ShipLoc, Gate->GetActorLocation());
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			Nearest = Gate;
		}

		FVector2D SP;
		if (!PC->ProjectWorldLocationToScreen(Gate->GetActorLocation(), SP, false))
		{
			continue;
		}
		SP *= ViewToCanvas; // viewport pixels -> canvas pixels (differ in high-res shots)
		if (SP.X < 0.0f || SP.Y < 0.0f || SP.X > Canvas->ClipX || SP.Y > Canvas->ClipY)
		{
			continue;
		}
		const FLinearColor Col = Gate->IsOnline() ? FLinearColor(0.7f, 0.5f, 1.0f, 0.95f) : FLinearColor(1.0f, 0.62f, 0.2f, 0.95f);
		const float R = 10.0f;
		DrawLine(SP.X, SP.Y - R, SP.X + R, SP.Y, Col, 2.0f);
		DrawLine(SP.X + R, SP.Y, SP.X, SP.Y + R, Col, 2.0f);
		DrawLine(SP.X, SP.Y + R, SP.X - R, SP.Y, Col, 2.0f);
		DrawLine(SP.X - R, SP.Y, SP.X, SP.Y - R, Col, 2.0f);
		DrawText(FString::Printf(TEXT("%s  %s"), *Gate->GetDisplayName().ToUpper(), *FormatDist(Dist)), Col, SP.X + 14.0f, SP.Y - 7.0f, HudType::Font(), HudType::Label);
	}

	// Approach prompt for the nearest gate.
	if (Nearest && NearestDist < 300000.0f)
	{
		const bool bOnline = Nearest->IsOnline();
		const FString Line = bOnline
			? FString::Printf(TEXT("JUMP GATE  >>  %s   (%s)   -   fly through the ring to jump"), *Nearest->GetDestinationName().ToUpper(), *FormatDist(NearestDist))
			: FString::Printf(TEXT("JUMP GATE OFFLINE  >>  %s   -   no level built for this sector yet"), *Nearest->GetDestinationName().ToUpper());
		UFont* Font = HudType::Font();
		float W = 0.0f, H = 0.0f;
		GetTextSize(Line, W, H, Font, HudType::Heading);
		const float X = (Canvas->ClipX - W) * 0.5f;
		const float Y = Canvas->ClipY * 0.58f;
		DrawRect(FLinearColor(0.02f, 0.02f, 0.06f, 0.75f), X - 14.0f, Y - 6.0f, W + 28.0f, H + 12.0f);
		DrawText(Line, bOnline ? FLinearColor(0.75f, 0.6f, 1.0f, 1.0f) : FLinearColor(1.0f, 0.62f, 0.2f, 1.0f), X, Y, Font, HudType::Heading);
	}
}

// ---------------------------------------------------------------------------
// Ship UMG widgets under the map
// ---------------------------------------------------------------------------

void AAdastreaHUD::SyncShipWidgetsForMap(APlayerController* PC)
{
	if (bShowMap)
	{
		// UMG draws above the canvas, so the ship's widgets would sit on top of the map.
		TArray<UUserWidget*, TInlineAllocator<2>> ShipWidgets;
		if (const AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC))
		{
			ShipWidgets.Add(AdPC->HUDWidget);
		}
		if (const ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr)
		{
			ShipWidgets.Add(Ship->GetDockingPromptWidget());
		}
		for (UUserWidget* Widget : ShipWidgets)
		{
			if (Widget && Widget->GetVisibility() != ESlateVisibility::Collapsed)
			{
				WidgetsHiddenForMap.Add(Widget, (uint8)Widget->GetVisibility());
				Widget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
	else if (WidgetsHiddenForMap.Num())
	{
		for (const TPair<TWeakObjectPtr<UUserWidget>, uint8>& Hidden : WidgetsHiddenForMap)
		{
			if (UUserWidget* Widget = Hidden.Key.Get())
			{
				Widget->SetVisibility((ESlateVisibility)Hidden.Value);
			}
		}
		WidgetsHiddenForMap.Reset();
	}
}
