// Copyright Epic Games, Inc. All Rights Reserved.

// Comms panel for AAdastreaHUD (canvas-drawn): a pirate gang hailing the player.
//   header     INCOMING HAIL and the gang's name in its flag colour
//   demand     what they say, and the list of what they want
//   countdown  time left to answer
//   keys       [J] comply (their drones take it) / [K] refuse (they attack)
// While their drones are collecting, the keys give way to a "hold still" line.

#include "AdastreaHUD.h"
#include "AdastreaHUDStyle.h"
#include "Combat/RaidSubsystem.h"
#include "Trading/TradeItemDataAsset.h"
#include "Universe/OrganisationSubsystem.h"
#include "Engine/Canvas.h"

namespace
{
	const FLinearColor kCommsBg(0.03f, 0.02f, 0.02f, 0.85f);
	const FLinearColor kCommsAlert(1.0f, 0.35f, 0.25f, 1.0f);
	const FLinearColor kCommsText(0.92f, 0.9f, 0.86f, 1.0f);
	const FLinearColor kCommsDim(0.65f, 0.62f, 0.58f, 1.0f);
	const FLinearColor kComply(0.35f, 0.95f, 0.55f, 1.0f);

	constexpr float PanelWidth = 600.0f;
	constexpr float Pad = 18.0f;
}

void AAdastreaHUD::DrawRaidComms(APlayerController* PC)
{
	const URaidSubsystem* Raids = URaidSubsystem::Get(this);
	const FRaid* Raid = Raids ? Raids->GetPlayerHail() : nullptr;
	if (!PC || !Canvas || !Raid)
	{
		return;
	}
	const float S = HudType::LayoutScale(Canvas->ClipX, Canvas->ClipY);
	UFont* Font = HudType::Font();

	FText GangName = FText::FromName(Raid->GangId);
	FLinearColor Flag = kCommsAlert;
	if (const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this))
	{
		if (const FOrgDef* Org = Orgs->FindOrg(Raid->GangId))
		{
			GangName = Org->Name;
			Flag = Org->Colour;
		}
	}

	// Greedy word wrap of the demand line to the panel width.
	const float W = PanelWidth * S;
	const float TextW = W - Pad * 2.0f * S;
	TArray<FString> Lines;
	{
		TArray<FString> Words;
		FString(TEXT("\"") + Raid->Demand.Line + TEXT("\"")).ParseIntoArrayWS(Words);
		FString Current;
		for (const FString& Word : Words)
		{
			const FString Trial = Current.IsEmpty() ? Word : Current + TEXT(" ") + Word;
			float TW = 0.0f, TH = 0.0f;
			GetTextSize(Trial, TW, TH, Font, HudType::Body * S);
			if (TW > TextW && !Current.IsEmpty())
			{
				Lines.Add(Current);
				Current = Word;
			}
			else
			{
				Current = Trial;
			}
		}
		if (!Current.IsEmpty())
		{
			Lines.Add(Current);
		}
	}
	TArray<FString> Wants;
	for (const FRaidGoods& Goods : Raid->Demand.Goods)
	{
		if (const UTradeItemDataAsset* Item = Goods.Item.Get())
		{
			Wants.Add(FString::Printf(TEXT("- %d x %s"), Goods.Units, *Item->ItemName.ToString()));
		}
	}
	if (Raid->Demand.Credits > 0)
	{
		Wants.Add(FString::Printf(TEXT("- %d credits"), Raid->Demand.Credits));
	}

	const float LineH = 20.0f * S;
	const float H = (Pad * 2.0f + 22.0f + 26.0f + 10.0f) * S + Lines.Num() * LineH + 8.0f * S + Wants.Num() * LineH + (16.0f + 14.0f + 26.0f) * S;
	const float X = (Canvas->ClipX - W) * 0.5f;
	const float Y = Canvas->ClipY * 0.17f;

	DrawRect(kCommsBg, X, Y, W, H);
	DrawRect(Flag, X, Y, 5.0f * S, H);
	DrawLine(X, Y, X + W, Y, kCommsAlert, 2.0f);
	DrawLine(X, Y + H, X + W, Y + H, kCommsAlert, 2.0f);

	float CY = Y + Pad * S;
	const float CX = X + Pad * S;
	const bool bHail = Raid->Phase == ERaidPhase::Hail;
	// Blink the header while the demand is open.
	const bool bBlinkOn = !bHail || FMath::Fmod(GetWorld()->GetRealTimeSeconds(), 1.0f) < 0.65f;
	DrawText(bHail ? TEXT("INCOMING HAIL") : TEXT("COMMS: PIRATES"), bBlinkOn ? kCommsAlert : kCommsDim, CX, CY, Font, HudType::Label * S);
	CY += 22.0f * S;
	DrawText(GangName.ToString(), Flag, CX, CY, Font, HudType::Heading * S);
	CY += 26.0f * S + 10.0f * S;

	for (const FString& Line : Lines)
	{
		DrawText(Line, kCommsText, CX, CY, Font, HudType::Body * S);
		CY += LineH;
	}
	CY += 8.0f * S;
	for (const FString& Want : Wants)
	{
		DrawText(Want, kCommsDim, CX + 8.0f * S, CY, Font, HudType::Body * S);
		CY += LineH;
	}
	CY += 16.0f * S;

	if (bHail)
	{
		// Countdown bar: time left to answer before they take silence as a refusal.
		const float Left = Raids->GetPlayerAnswerSecondsLeft();
		const float Frac = FMath::Clamp(Left / URaidSubsystem::PlayerAnswerSeconds, 0.0f, 1.0f);
		const float BarW = W - Pad * 2.0f * S;
		DrawRect(FLinearColor(1.0f, 1.0f, 1.0f, 0.12f), CX, CY, BarW, 5.0f * S);
		DrawRect(Frac > 0.3f ? kCommsText : kCommsAlert, CX, CY, BarW * Frac, 5.0f * S);
		CY += 14.0f * S;
		const FString Comply = TEXT("[J] COMPLY - cut engines, their drones take it");
		const FString Refuse = FString::Printf(TEXT("[K] REFUSE   %.0f s"), Left);
		DrawText(Comply, kComply, CX, CY, Font, HudType::Label * S);
		float RW = 0.0f, RH = 0.0f;
		GetTextSize(Refuse, RW, RH, Font, HudType::Label * S);
		DrawText(Refuse, kCommsAlert, X + W - Pad * S - RW, CY, Font, HudType::Label * S);
	}
	else
	{
		CY += 14.0f * S;
		DrawText(TEXT("You complied. Their drones are taking the cargo: hold still."), kComply, CX, CY, Font, HudType::Label * S);
	}
}
