// Copyright Epic Games, Inc. All Rights Reserved.

// HUD layout editor for AAdastreaHUD: lets the player move and scale each cockpit
// flight HUD panel.
//
// Each panel's draw code is wrapped in BeginHudElement/EndHudElement, which pushes a
// canvas transform (scale about the panel's anchor, then the player's offset), so the
// panels themselves keep drawing at their authored positions. Offsets are stored in
// 1080p layout pixels so a layout survives resolution changes.

#include "AdastreaHUD.h"
#include "AdastreaHUDStyle.h"
#include "Engine/Canvas.h"
#include "CanvasTypes.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	/** Default placement of a panel: Align (screen fraction) + Pos * LayoutScale = rect top-left. */
	struct FHudElementDef
	{
		const TCHAR* Key;     // GameUserSettings.ini key
		const TCHAR* Label;   // shown in the editor
		FVector2D Align;
		FVector2D Pos;        // 1080p layout pixels
		FVector2D Size;       // 1080p layout pixels
		FVector2D Pivot;      // fraction of the rect that stays put when scaling
	};

	// Must match the rects drawn in AdastreaHUD_Cockpit.cpp; order matches EHudElement.
	const FHudElementDef kHudElements[] =
	{
		{ TEXT("HeadingTape"), TEXT("HEADING TAPE"),   { 0.5f, 0.0f },  { -236.0f, 16.0f },  { 472.0f, 66.0f },  { 0.5f, 0.0f } },
		{ TEXT("Ident"),       TEXT("IDENT"),          { 0.0f, 0.0f },  { 24.0f, 21.0f },    { 270.0f, 93.0f },  { 0.0f, 0.0f } },
		{ TEXT("Mining"),      TEXT("MINING UPLINK"),  { 0.0f, 0.0f },  { 24.0f, 123.0f },   { 270.0f, 201.0f }, { 0.0f, 0.0f } },
		{ TEXT("Defence"),     TEXT("DEFENCE GRID"),   { 0.0f, 1.0f },  { 24.0f, -139.0f },  { 330.0f, 115.0f }, { 0.0f, 1.0f } },
		{ TEXT("Propulsion"),  TEXT("PROPULSION"),     { 1.0f, 1.0f },  { -354.0f, -139.0f },{ 330.0f, 115.0f }, { 1.0f, 1.0f } },
		{ TEXT("Docking"),     TEXT("DOCKING PROMPT"), { 0.5f, 0.66f }, { -222.0f, -3.0f },  { 444.0f, 67.0f },  { 0.5f, 0.5f } },
		{ TEXT("DroneHint"),   TEXT("DRONE CONTROLS"), { 0.5f, 1.0f },  { -184.0f, -82.0f }, { 340.0f, 52.0f },  { 0.5f, 1.0f } },
	};
	static_assert(UE_ARRAY_COUNT(kHudElements) == static_cast<int32>(EHudElement::Count), "kHudElements must list every EHudElement");

	const TCHAR* kHudLayoutSection = TEXT("AdastreaHUDLayout");
	constexpr float kMinHudScale = 0.5f;
	constexpr float kMaxHudScale = 2.5f;
	constexpr float kHudScaleStep = 1.1f;

	/** Unscaled rect and pivot of a panel on this canvas. */
	void GetDefaultRect(const FHudElementDef& Def, float SW, float SH, float S, FVector2D& OutMin, FVector2D& OutSize, FVector2D& OutPivot)
	{
		OutMin = Def.Align * FVector2D(SW, SH) + Def.Pos * S;
		OutSize = Def.Size * S;
		OutPivot = OutMin + OutSize * Def.Pivot;
	}
}

void AAdastreaHUD::LoadHudLayout()
{
	bHudLayoutLoaded = true;
	if (!GConfig)
	{
		return;
	}
	for (int32 i = 0; i < static_cast<int32>(EHudElement::Count); ++i)
	{
		FString Value;
		if (!GConfig->GetString(kHudLayoutSection, kHudElements[i].Key, Value, GGameUserSettingsIni))
		{
			continue;
		}
		TArray<FString> Parts;
		Value.ParseIntoArray(Parts, TEXT(","));
		if (Parts.Num() == 3)
		{
			HudLayout[i].Offset = FVector2D(FCString::Atof(*Parts[0]), FCString::Atof(*Parts[1]));
			HudLayout[i].Scale = FMath::Clamp(FCString::Atof(*Parts[2]), kMinHudScale, kMaxHudScale);
		}
	}
}

void AAdastreaHUD::SaveHudLayout() const
{
	if (!GConfig)
	{
		return;
	}
	for (int32 i = 0; i < static_cast<int32>(EHudElement::Count); ++i)
	{
		const FHudElementLayout& L = HudLayout[i];
		GConfig->SetString(kHudLayoutSection, kHudElements[i].Key,
			*FString::Printf(TEXT("%.1f,%.1f,%.3f"), L.Offset.X, L.Offset.Y, L.Scale), GGameUserSettingsIni);
	}
	GConfig->Flush(false, GGameUserSettingsIni);
}

void AAdastreaHUD::SetHudEditMode(bool bEnable)
{
	bHudEditMode = bEnable;
	HudDragElement = EHudElement::Count;
}

void AAdastreaHUD::ResetHudLayout()
{
	for (FHudElementLayout& L : HudLayout)
	{
		L = FHudElementLayout();
	}
	SaveHudLayout();
}

void AAdastreaHUD::BeginHudElement(EHudElement Element)
{
	if (!bHudLayoutLoaded)
	{
		LoadHudLayout();
	}
	const int32 Index = static_cast<int32>(Element);
	HudElementsDrawn |= 1u << Index;
	if (!Canvas || !Canvas->Canvas)
	{
		return;
	}

	const float SW = Canvas->ClipX, SH = Canvas->ClipY;
	const float S = HudType::LayoutScale(SW, SH);
	FVector2D Min, Size, Pivot;
	GetDefaultRect(kHudElements[Index], SW, SH, S, Min, Size, Pivot);
	const FHudElementLayout& L = HudLayout[Index];
	const FVector2D Move = L.Offset * S;

	// Row vectors: scale about the pivot, then move by the player's offset.
	const FMatrix M = FTranslationMatrix(FVector(-Pivot.X, -Pivot.Y, 0.0f))
		* FScaleMatrix(FVector(L.Scale, L.Scale, 1.0f))
		* FTranslationMatrix(FVector(Pivot.X + Move.X, Pivot.Y + Move.Y, 0.0f));
	Canvas->Canvas->PushRelativeTransform(M);
}

void AAdastreaHUD::EndHudElement()
{
	if (Canvas && Canvas->Canvas)
	{
		Canvas->Canvas->PopTransform();
	}
}

void AAdastreaHUD::GetHudElementRect(EHudElement Element, FVector2D& OutMin, FVector2D& OutMax) const
{
	const int32 Index = static_cast<int32>(Element);
	const float SW = Canvas ? Canvas->ClipX : 1920.0f, SH = Canvas ? Canvas->ClipY : 1080.0f;
	const float S = HudType::LayoutScale(SW, SH);
	FVector2D Min, Size, Pivot;
	GetDefaultRect(kHudElements[Index], SW, SH, S, Min, Size, Pivot);
	const FHudElementLayout& L = HudLayout[Index];
	const FVector2D Move = L.Offset * S;
	OutMin = Pivot + (Min - Pivot) * L.Scale + Move;
	OutMax = OutMin + Size * L.Scale;
}

void AAdastreaHUD::DrawHudLayoutEditor(APlayerController* PC)
{
	const uint32 Drawn = HudElementsDrawn;
	HudElementsDrawn = 0;
	if (!bHudEditMode || !Canvas || !PC)
	{
		return;
	}
	if (!bHudLayoutLoaded)
	{
		LoadHudLayout();
	}

	const float SW = Canvas->ClipX, SH = Canvas->ClipY;
	const float S = HudType::LayoutScale(SW, SH);
	const int32 Count = static_cast<int32>(EHudElement::Count);

	float MX = 0.0f, MY = 0.0f;
	const bool bHasMouse = PC->GetMousePosition(MX, MY);
	const FVector2D Mouse(MX, MY);

	// ---- Input: drag to move, wheel to scale, right-click to reset, Home to reset all ----
	int32 Hover = INDEX_NONE;
	if (HudDragElement != EHudElement::Count)
	{
		Hover = static_cast<int32>(HudDragElement);
	}
	else if (bHasMouse)
	{
		// Smallest rect under the cursor wins, so a panel inside a bigger one stays reachable.
		float BestArea = TNumericLimits<float>::Max();
		for (int32 i = 0; i < Count; ++i)
		{
			FVector2D Min, Max;
			GetHudElementRect(static_cast<EHudElement>(i), Min, Max);
			const float Area = (Max.X - Min.X) * (Max.Y - Min.Y);
			if (MX >= Min.X && MX <= Max.X && MY >= Min.Y && MY <= Max.Y && Area < BestArea)
			{
				Hover = i;
				BestArea = Area;
			}
		}
	}

	bool bChanged = false;
	if (HudDragElement == EHudElement::Count && Hover != INDEX_NONE && PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		HudDragElement = static_cast<EHudElement>(Hover);
		HudDragStartMouse = Mouse;
		HudDragStartOffset = HudLayout[Hover].Offset;
	}
	if (HudDragElement != EHudElement::Count)
	{
		const int32 Index = static_cast<int32>(HudDragElement);
		if (PC->IsInputKeyDown(EKeys::LeftMouseButton))
		{
			HudLayout[Index].Offset = HudDragStartOffset + (Mouse - HudDragStartMouse) / S;

			// Keep the panel on screen.
			FVector2D Min, Max;
			GetHudElementRect(HudDragElement, Min, Max);
			FVector2D Push = FVector2D::ZeroVector;
			Push.X = Min.X < 0.0f ? -Min.X : (Max.X > SW ? SW - Max.X : 0.0f);
			Push.Y = Min.Y < 0.0f ? -Min.Y : (Max.Y > SH ? SH - Max.Y : 0.0f);
			HudLayout[Index].Offset += Push / S;
		}
		else
		{
			HudDragElement = EHudElement::Count;
			bChanged = true;
		}
	}
	if (Hover != INDEX_NONE)
	{
		FHudElementLayout& L = HudLayout[Hover];
		const float OldScale = L.Scale;
		if (PC->WasInputKeyJustPressed(EKeys::MouseScrollUp))
		{
			L.Scale = FMath::Min(L.Scale * kHudScaleStep, kMaxHudScale);
		}
		if (PC->WasInputKeyJustPressed(EKeys::MouseScrollDown))
		{
			L.Scale = FMath::Max(L.Scale / kHudScaleStep, kMinHudScale);
		}
		if (PC->WasInputKeyJustPressed(EKeys::RightMouseButton))
		{
			L = FHudElementLayout();
			HudDragElement = EHudElement::Count;
		}
		bChanged |= !FMath::IsNearlyEqual(OldScale, L.Scale) || PC->WasInputKeyJustPressed(EKeys::RightMouseButton);
	}
	if (PC->WasInputKeyJustPressed(EKeys::Home))
	{
		for (FHudElementLayout& L : HudLayout)
		{
			L = FHudElementLayout();
		}
		HudDragElement = EHudElement::Count;
		bChanged = true;
	}
	if (bChanged)
	{
		SaveHudLayout();
	}

	// ---- Draw: dim the scene, outline every panel, placeholders for hidden ones ----
	const FLinearColor kEdge(0.0f, 0.9f, 1.0f, 0.55f);
	const FLinearColor kHot(1.0f, 0.86f, 0.05f, 1.0f);
	const FLinearColor kInk(0.85f, 0.95f, 1.0f, 1.0f);
	const FLinearColor kMute(0.45f, 0.58f, 0.68f, 1.0f);

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.25f), 0.0f, 0.0f, SW, SH);

	for (int32 i = 0; i < Count; ++i)
	{
		FVector2D Min, Max;
		GetHudElementRect(static_cast<EHudElement>(i), Min, Max);
		const bool bHot = i == Hover;
		const FLinearColor Col = bHot ? kHot : kEdge;
		const float W = Max.X - Min.X, H = Max.Y - Min.Y;

		if (!(Drawn & (1u << i)))
		{
			DrawRect(FLinearColor(0.0f, 0.45f, 0.55f, 0.18f), Min.X, Min.Y, W, H);
			DrawText(TEXT("(shown when needed)"), kMute, Min.X + 6.0f * S, Min.Y + H - 16.0f * S, HudType::Font(), HudType::Caption * S);
		}
		if (bHot)
		{
			DrawRect(FLinearColor(1.0f, 0.86f, 0.05f, 0.08f), Min.X, Min.Y, W, H);
		}
		const float T = bHot ? 2.0f : 1.0f;
		DrawLine(Min.X, Min.Y, Max.X, Min.Y, Col, T);
		DrawLine(Max.X, Min.Y, Max.X, Max.Y, Col, T);
		DrawLine(Max.X, Max.Y, Min.X, Max.Y, Col, T);
		DrawLine(Min.X, Max.Y, Min.X, Min.Y, Col, T);

		const FString Tag = FString::Printf(TEXT("%s  %d%%"), kHudElements[i].Label, FMath::RoundToInt(HudLayout[i].Scale * 100.0f));
		DrawRect(FLinearColor(0.02f, 0.0f, 0.05f, 0.85f), Min.X, Min.Y, 150.0f * S, 16.0f * S);
		DrawText(Tag, Col, Min.X + 4.0f * S, Min.Y + 1.0f * S, HudType::Font(), HudType::Caption * S);
	}

	// ---- Instructions (centre) ----
	const TCHAR* Lines[] =
	{
		TEXT("HUD LAYOUT"),
		TEXT("Drag a panel to move it   //   Mouse wheel over it to scale"),
		TEXT("Right-click a panel to reset it   //   Home to reset all   //   H to finish"),
	};
	float Y = SH * 0.40f;
	for (int32 i = 0; i < UE_ARRAY_COUNT(Lines); ++i)
	{
		const float Scale = (i == 0 ? HudType::Title : HudType::Body) * S;
		float TW = 0.0f, TH = 0.0f;
		GetTextSize(Lines[i], TW, TH, HudType::Font(), Scale);
		DrawRect(FLinearColor(0.02f, 0.0f, 0.05f, 0.7f), (SW - TW) * 0.5f - 8.0f * S, Y - 2.0f * S, TW + 16.0f * S, TH + 4.0f * S);
		DrawText(Lines[i], i == 0 ? kHot : kInk, (SW - TW) * 0.5f, Y, HudType::Font(), Scale);
		Y += TH + 8.0f * S;
	}
}
