#include "Ships/ExteriorDressingComponent.h"
#include "AdastreaLog.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarExteriorDressing(
		TEXT("adastrea.ExteriorDressing"), 1,
		TEXT("1 = ships and station modules fit exterior props (nav lights, beacons, antennas) at BeginPlay, 0 = off."),
		ECVF_Default);

	const FLinearColor NavRed(1.0f, 0.04f, 0.02f);
	const FLinearColor NavGreen(0.05f, 1.0f, 0.2f);
	const FLinearColor NavWhite(1.0f, 0.95f, 0.85f);
	const FLinearColor Amber(1.0f, 0.55f, 0.1f);
	const FLinearColor Cyan(0.2f, 0.85f, 1.0f);
}

UExteriorDressingComponent::UExteriorDressingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.05f;
}

bool UExteriorDressingComponent::IsAutoDressingEnabled()
{
	return CVarExteriorDressing.GetValueOnGameThread() != 0;
}

void UExteriorDressingComponent::BeginPlay()
{
	Super::BeginPlay();
	Dress();
	if (Props.Num() == 0 && GetWorld())
	{
		// Some actors get their mesh assigned after BeginPlay: look again for a few seconds.
		TWeakObjectPtr<UExteriorDressingComponent> Weak(this);
		TSharedRef<int32> Tries = MakeShared<int32>(0);
		GetWorld()->GetTimerManager().SetTimer(RetryTimer, FTimerDelegate::CreateLambda([Weak, Tries]()
		{
			UExteriorDressingComponent* Self = Weak.Get();
			if (!Self || ++(*Tries) > 10)
			{
				if (Self) { Self->GetWorld()->GetTimerManager().ClearTimer(Self->RetryTimer); }
				return;
			}
			Self->Dress();
			if (Self->Props.Num() > 0)
			{
				Self->GetWorld()->GetTimerManager().ClearTimer(Self->RetryTimer);
			}
		}), 0.5f, true);
	}
}

UStaticMeshComponent* UExteriorDressingComponent::FindTarget() const
{
	if (TargetMesh && TargetMesh->GetStaticMesh())
	{
		return TargetMesh;
	}
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}
	TArray<UStaticMeshComponent*> Meshes;
	Owner->GetComponents<UStaticMeshComponent>(Meshes);
	UStaticMeshComponent* Best = nullptr;
	double BestSize = 0.0;
	for (UStaticMeshComponent* M : Meshes)
	{
		if (!M || !M->GetStaticMesh() || !M->IsVisible() || Props.Contains(M))
		{
			continue;
		}
		const double Size = (M->GetStaticMesh()->GetBoundingBox().GetSize() * M->GetComponentScale()).SizeSquared();
		if (Size > BestSize)
		{
			BestSize = Size;
			Best = M;
		}
	}
	return Best;
}

void UExteriorDressingComponent::ClearDressing()
{
	for (TObjectPtr<UStaticMeshComponent> P : Props)
	{
		if (P)
		{
			TArray<USceneComponent*> Children;
			P->GetChildrenComponents(false, Children);
			for (USceneComponent* C : Children)
			{
				C->DestroyComponent();
			}
			P->DestroyComponent();
		}
	}
	Props.Empty();
	Lights.Empty();
	SetComponentTickEnabled(false);
}

void UExteriorDressingComponent::Dress()
{
	ClearDressing();
	UStaticMeshComponent* Target = FindTarget();
	if (!Target || !Target->GetStaticMesh())
	{
		return;
	}
	TargetMesh = Target;
	LocalBox = Target->GetStaticMesh()->GetBoundingBox();
	switch (Preset)
	{
	case EExteriorDressingPreset::Ship:          DressShip(); break;
	case EExteriorDressingPreset::StationModule: DressStation(false); break;
	case EExteriorDressingPreset::DockingBay:    DressStation(true); break;
	}
	SetComponentTickEnabled(Lights.Num() > 0);
	UE_LOG(LogAdastrea, Verbose, TEXT("ExteriorDressing %s: %d props, %d lights on %s"),
		*GetOwner()->GetName(), Props.Num(), Lights.Num(), *Target->GetStaticMesh()->GetName());
}

bool UExteriorDressingComponent::Probe(const FVector& Dir, const TArray<FVector>& Starts, FVector& OutLoc, FVector& OutNormal) const
{
	// Starts are local points outside the hull; trace inward along -Dir and keep
	// the hit furthest out along Dir.
	const FTransform& Xf = TargetMesh->GetComponentTransform();
	const float Reach = LocalBox.GetSize().Size() * 2.0f;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ExteriorDressingProbe), true);
	bool bAny = false;
	float BestOut = -TNumericLimits<float>::Max();
	for (const FVector& S : Starts)
	{
		const FVector WS = Xf.TransformPosition(S);
		const FVector WE = Xf.TransformPosition(S - Dir * Reach);
		FHitResult Hit;
		if (TargetMesh->LineTraceComponent(Hit, WS, WE, Params))
		{
			const FVector L = Xf.InverseTransformPosition(Hit.ImpactPoint);
			const float Out = FVector::DotProduct(L, Dir);
			if (Out > BestOut)
			{
				BestOut = Out;
				OutLoc = L;
				OutNormal = Xf.InverseTransformVectorNoScale(Hit.ImpactNormal).GetSafeNormal();
				bAny = true;
			}
		}
	}
	if (!bAny && Starts.Num() > 0)
	{
		// No collision to trace against: fall back to the bounds face.
		const FVector C = LocalBox.GetCenter();
		const FVector E = LocalBox.GetExtent();
		const FVector S0 = Starts[Starts.Num() / 2];
		const FVector Across = S0 - Dir * FVector::DotProduct(S0 - C, Dir);
		OutLoc = Across + Dir * FVector::DotProduct(E, Dir.GetAbs());
		OutNormal = Dir;
		return true;
	}
	return bAny;
}

UStaticMeshComponent* UExteriorDressingComponent::AddProp(FName Name, const FVector& Loc, const FVector& Normal, const FVector& Facing, float Scale)
{
	const FString Path = FString::Printf(TEXT("/AdastreaShips/Meshes/ExteriorProps/SM_ExtProp_%s.SM_ExtProp_%s"), *Name.ToString(), *Name.ToString());
	UStaticMesh* SM = LoadObject<UStaticMesh>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!SM || !TargetMesh)
	{
		return nullptr;
	}
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(GetOwner());
	C->SetupAttachment(TargetMesh);
	C->SetStaticMesh(SM);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetCanEverAffectNavigation(false);
	C->SetCastShadow(Scale * PropScale > 0.8f);
	C->SetAbsolute(false, false, true);
	C->SetRelativeLocation(Loc);
	FVector F = Facing - Normal * FVector::DotProduct(Facing, Normal);
	if (F.IsNearlyZero())
	{
		F = FVector::CrossProduct(Normal, FVector::RightVector);
	}
	C->SetRelativeRotation(FRotationMatrix::MakeFromZX(Normal, F).Rotator());
	C->RegisterComponent();
	C->SetWorldScale3D(FVector(Scale * PropScale));
	Props.Add(C);
	return C;
}

void UExteriorDressingComponent::AddLight(UStaticMeshComponent* Prop, const FLinearColor& Colour, EExteriorLightMode Mode, float Intensity,
	float Phase, UStaticMeshComponent* Spinner)
{
	if (!Prop)
	{
		return;
	}
	UStaticMeshComponent* LensOwner = Spinner ? Spinner : Prop;
	const int32 Slot = LensOwner->GetMaterialIndex(TEXT("M_Prop_Beacon"));
	if (Slot == INDEX_NONE)
	{
		return;
	}
	FExteriorDressingLight L;
	L.MID = LensOwner->CreateDynamicMaterialInstance(Slot);
	L.Colour = Colour;
	L.Mode = Mode;
	L.Intensity = Intensity;
	L.Phase = Phase;
	L.Spinner = Spinner;
	if (L.MID)
	{
		L.MID->SetVectorParameterValue(TEXT("Color"), Colour);
		L.MID->SetScalarParameterValue(TEXT("Intensity"), Intensity);
	}
	Lights.Add(L);
}

void UExteriorDressingComponent::DressShip()
{
	// Hull axes in mesh space, from the actor (meshes are often rotated in the BP).
	const FTransform& Xf = TargetMesh->GetComponentTransform();
	const AActor* Owner = GetOwner();
	const FVector Fwd = Xf.InverseTransformVectorNoScale(Owner->GetActorForwardVector()).GetSafeNormal();
	const FVector Right = Xf.InverseTransformVectorNoScale(Owner->GetActorRightVector()).GetSafeNormal();
	const FVector Up = Xf.InverseTransformVectorNoScale(Owner->GetActorUpVector()).GetSafeNormal();
	const FVector C = LocalBox.GetCenter();
	const FVector E = LocalBox.GetExtent();
	auto Ext = [&](const FVector& Axis) { return FVector::DotProduct(E, Axis.GetAbs()); };
	const float EF = Ext(Fwd), ER = Ext(Right), EU = Ext(Up);
	// World length drives prop size (a 10 m patrol ship vs a 30 m battleship).
	const float WorldLen = EF * 2.0f * FMath::Abs(FVector::DotProduct(Xf.GetScale3D(), Fwd.GetAbs()));
	const float S = FMath::Clamp(WorldLen / 1500.0f, 0.5f, 3.0f);

	auto Grid = [&](const FVector& Out, float OutExt, const FVector& A, float AExt, float A0, float A1,
		const FVector& B, float BExt, float B0, float B1, int32 N)
	{
		TArray<FVector> Pts;
		for (int32 i = 0; i < N; ++i)
		{
			for (int32 j = 0; j < N; ++j)
			{
				const float a = FMath::Lerp(A0, A1, N > 1 ? i / float(N - 1) : 0.5f);
				const float b = FMath::Lerp(B0, B1, N > 1 ? j / float(N - 1) : 0.5f);
				Pts.Add(C + Out * (OutExt + 100.0f) + A * (a * AExt) + B * (b * BExt));
			}
		}
		return Pts;
	};

	FVector L, N;
	// wingtips: outermost port/starboard points anywhere along the hull
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const FVector Out = Right * float(Side);
		if (Probe(Out, Grid(Out, ER, Fwd, EF, -0.9f, 0.9f, Up, EU, -0.6f, 0.6f, 9), L, N))
		{
			UStaticMeshComponent* Nav = AddProp(TEXT("NavLight"), L, Out, Fwd, S);
			AddLight(Nav, Side < 0 ? NavRed : NavGreen, EExteriorLightMode::Steady, 60.0f);
			UStaticMeshComponent* Strobe = AddProp(TEXT("Strobe"), L - Fwd * (40.0f * S) / Xf.GetScale3D().GetMax(), N, Fwd, S);
			AddLight(Strobe, NavWhite, EExteriorLightMode::Strobe, 400.0f, Side < 0 ? 0.0f : 0.35f);
		}
	}
	// stern light
	if (Probe(-Fwd, Grid(-Fwd, EF, Right, ER, -0.3f, 0.3f, Up, EU, -0.3f, 0.3f, 5), L, N))
	{
		AddLight(AddProp(TEXT("NavLight"), L, -Fwd, -Right, S), NavWhite, EExteriorLightMode::Steady, 50.0f);
	}
	// anti-collision beacons, top and bottom
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const FVector Out = Up * float(Side);
		if (Probe(Out, Grid(Out, EU, Fwd, EF, -0.25f, 0.1f, Right, ER, -0.15f, 0.15f, 5), L, N))
		{
			UStaticMeshComponent* Base = AddProp(TEXT("Beacon"), L, N, Fwd, S);
			UStaticMeshComponent* Head = AddProp(TEXT("BeaconHead"), L, N, Fwd, S);
			AddLight(Base, NavRed, EExteriorLightMode::Rotating, 120.0f, 0.0f, Head);
		}
	}
	// spine: antenna aft, a sensor/comms fit amidships (varies per hull)
	const int32 Pick = (Seed != 0 ? Seed : static_cast<int32>(GetTypeHash(Owner->GetClass()->GetFName()))) & 0x7fffffff;
	if (Probe(Up, Grid(Up, EU, Fwd, EF, -0.55f, -0.3f, Right, ER, -0.1f, 0.1f, 4), L, N))
	{
		UStaticMeshComponent* Mast = AddProp(Pick % 2 ? TEXT("AntennaMast") : TEXT("WhipAntenna"), L, N, Fwd, S);
		AddLight(Mast, NavRed, EExteriorLightMode::Pulse, 60.0f, 1.3f);
	}
	if (Probe(Up, Grid(Up, EU, Fwd, EF, 0.0f, 0.3f, Right, ER, 0.2f, 0.45f, 4), L, N))
	{
		switch (Pick % 3)
		{
		case 0: AddProp(TEXT("SensorDish"), L, N, Fwd, S * 0.8f); break;
		case 1:
		{
			AddProp(TEXT("RadarBase"), L, N, Fwd, S * 0.8f);
			UStaticMeshComponent* Head = AddProp(TEXT("RadarHead"), L, N, Fwd, S * 0.8f);
			FExteriorDressingLight Spin;
			Spin.Spinner = Head;
			Spin.Mode = EExteriorLightMode::Rotating;
			Lights.Add(Spin);
			break;
		}
		default: AddLight(AddProp(TEXT("CommArray"), L, N, Fwd, S * 0.8f), NavRed, EExteriorLightMode::Pulse, 40.0f, 0.6f); break;
		}
	}
	// a little hull detail on the belly
	if (Probe(-Up, Grid(-Up, EU, Fwd, EF, -0.5f, 0.4f, Right, ER, -0.3f, 0.3f, 4), L, N))
	{
		AddProp(TEXT("Greeble"), L, N, Fwd, S * 0.8f);
	}
}

void UExteriorDressingComponent::DressStation(bool bDockingBay)
{
	const FTransform& Xf = TargetMesh->GetComponentTransform();
	const AActor* Owner = GetOwner();
	const FVector Fwd = Xf.InverseTransformVectorNoScale(Owner->GetActorForwardVector()).GetSafeNormal();
	const FVector Right = Xf.InverseTransformVectorNoScale(Owner->GetActorRightVector()).GetSafeNormal();
	const FVector Up = Xf.InverseTransformVectorNoScale(Owner->GetActorUpVector()).GetSafeNormal();
	const FVector C = LocalBox.GetCenter();
	const FVector E = LocalBox.GetExtent();
	auto Ext = [&](const FVector& Axis) { return FVector::DotProduct(E, Axis.GetAbs()); };
	const float EF = Ext(Fwd), ER = Ext(Right), EU = Ext(Up);
	FVector L, N;
	auto TopAt = [&](float f, float r) -> bool
	{
		TArray<FVector> Pts = { C + Up * (EU + 100.0f) + Fwd * (f * EF) + Right * (r * ER) };
		return Probe(Up, Pts, L, N);
	};
	// corner beacons (slow red breathe, staggered)
	int32 k = 0;
	for (float f : { -0.85f, 0.85f })
	{
		for (float r : { -0.85f, 0.85f })
		{
			if (TopAt(f, r))
			{
				AddLight(AddProp(TEXT("Strobe"), L, N, Fwd, 1.5f), NavRed, EExteriorLightMode::Pulse, 150.0f, 0.8f * k);
			}
			++k;
		}
	}
	// floodlights at both ends of the roof, washing the module with a real spotlight
	for (float f : { -0.95f, 0.95f })
	{
		if (TopAt(f, 0.0f))
		{
			const FVector Aim = -Fwd * FMath::Sign(f);
			UStaticMeshComponent* Flood = AddProp(TEXT("Floodlight"), L, N, Aim, 1.5f);
			AddLight(Flood, NavWhite, EExteriorLightMode::Steady, 60.0f);
			if (Flood)
			{
				USpotLightComponent* Spot = NewObject<USpotLightComponent>(GetOwner());
				Spot->SetupAttachment(Flood);
				Spot->SetRelativeLocation(FVector(20.0f, 0.0f, 45.0f));
				Spot->SetRelativeRotation(FRotator(-35.0f, 0.0f, 0.0f));
				Spot->SetIntensityUnits(ELightUnits::Candelas);
				Spot->SetIntensity(4000.0f);
				Spot->SetAttenuationRadius(FMath::Max(1500.0f, EF * 2.5f * Xf.GetScale3D().GetMax()));
				Spot->SetOuterConeAngle(55.0f);
				Spot->SetInnerConeAngle(30.0f);
				Spot->SetLightColor(FLinearColor(1.0f, 0.93f, 0.82f));
				Spot->SetCastShadows(false);
				Spot->RegisterComponent();
			}
		}
	}
	// comms on the roof centre
	if (TopAt(0.1f, 0.3f))
	{
		AddLight(AddProp(TEXT("CommArray"), L, N, Fwd, 1.5f), NavRed, EExteriorLightMode::Pulse, 60.0f, 0.3f);
	}
	if (TopAt(-0.3f, -0.4f))
	{
		AddProp(TEXT("AntennaMast"), L, N, Fwd, 1.5f);
	}
	if (bDockingBay)
	{
		// guide chevrons down both sides of the roof, chasing toward the bay mouth (+Fwd)
		for (float r : { -0.55f, 0.55f })
		{
			for (int32 i = 0; i < 3; ++i)
			{
				if (TopAt(-0.5f + i * 0.45f, r))
				{
					AddLight(AddProp(TEXT("DockingGuide"), L, N, Fwd, 1.5f), Cyan, EExteriorLightMode::Pulse, 80.0f, i * 1.0f);
				}
			}
		}
		if (TopAt(0.7f, 0.0f))
		{
			AddProp(TEXT("TrafficBeacon"), L, N, Fwd, 1.5f);
		}
	}
}

void UExteriorDressingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Clock += DeltaTime;
	if (TargetMesh && !TargetMesh->WasRecentlyRendered(0.5f))
	{
		return;       // nobody's looking: skip the light work
	}
	for (FExteriorDressingLight& L : Lights)
	{
		if (L.Spinner)
		{
			L.Spinner->AddLocalRotation(FRotator(0.0f, 240.0f * DeltaTime, 0.0f));
		}
		if (!L.MID)
		{
			continue;
		}
		float I = L.Intensity;
		switch (L.Mode)
		{
		case EExteriorLightMode::Strobe:
		{
			const float T = FMath::Fmod(Clock + L.Phase, 1.4f) / 1.4f;
			I = (T < 0.04f || (T > 0.1f && T < 0.14f)) ? L.Intensity : 0.0f;
			break;
		}
		case EExteriorLightMode::Pulse:
			I = L.Intensity * (0.25f + 0.75f * 0.5f * (1.0f + FMath::Sin(Clock * 2.5f + L.Phase)));
			break;
		default:
			break;
		}
		L.MID->SetScalarParameterValue(TEXT("Intensity"), I);
	}
}
