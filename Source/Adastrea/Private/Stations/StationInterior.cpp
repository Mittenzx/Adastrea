// Copyright Epic Games, Inc. All Rights Reserved.

#include "Stations/StationInterior.h"
#include "Player/AdastreaPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* kCubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* kBaseMaterialPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");

	const float kWall = 10.0f; // half-thickness of every slab

	FLinearColor TerminalColor(EStationTerminalType Type)
	{
		switch (Type)
		{
		case EStationTerminalType::Trading:       return FLinearColor(0.95f, 0.65f, 0.10f);
		case EStationTerminalType::ToMaintenance: return FLinearColor(0.90f, 0.30f, 0.10f);
		case EStationTerminalType::ToHabitation:  return FLinearColor(0.20f, 0.75f, 0.45f);
		case EStationTerminalType::Service:       return FLinearColor(0.70f, 0.40f, 0.95f);
		default:                                  return FLinearColor(0.20f, 0.60f, 0.95f);
		}
	}

	UMaterialInstanceDynamic* MakeTinted(UObject* Outer, UMaterialInterface* Base, const FLinearColor& Color)
	{
		if (!Base)
		{
			return nullptr;
		}
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Outer);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
		}
		return MID;
	}
}

// ====================
// AStationTerminal
// ====================

AStationTerminal::AStationTerminal()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(kCubePath);

	Pedestal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pedestal"));
	Pedestal->SetupAttachment(Root);
	Pedestal->SetRelativeLocation(FVector(0.0f, 0.0f, 55.0f));
	Pedestal->SetRelativeScale3D(FVector(0.6f, 0.9f, 1.1f));
	Pedestal->SetCollisionProfileName(TEXT("BlockAll"));

	Screen = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Screen"));
	Screen->SetupAttachment(Root);
	Screen->SetRelativeLocation(FVector(0.0f, 0.0f, 125.0f));
	Screen->SetRelativeRotation(FRotator(25.0f, 0.0f, 0.0f));
	Screen->SetRelativeScale3D(FVector(0.06f, 0.8f, 0.4f));
	Screen->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (CubeFinder.Succeeded())
	{
		Pedestal->SetStaticMesh(CubeFinder.Object);
		Screen->SetStaticMesh(CubeFinder.Object);
	}
}

void AStationTerminal::SetTerminalType(EStationTerminalType InType)
{
	TerminalType = InType;

	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, kBaseMaterialPath);
	if (UMaterialInstanceDynamic* Body = MakeTinted(this, Base, FLinearColor(0.10f, 0.11f, 0.14f)))
	{
		Pedestal->SetMaterial(0, Body);
	}
	if (UMaterialInstanceDynamic* Glow = MakeTinted(this, Base, TerminalColor(InType)))
	{
		Screen->SetMaterial(0, Glow);
	}
}

void AStationTerminal::SetService(const FString& InPrompt, const FString& InMessage)
{
	ServicePrompt = InPrompt;
	ServiceMessage = InMessage;
}

FText AStationTerminal::GetInteractPrompt_Implementation() const
{
	switch (TerminalType)
	{
	case EStationTerminalType::Trading:       return FText::FromString(TEXT("Open Trading Department"));
	case EStationTerminalType::ToConcourse:   return FText::FromString(TEXT("Go to Concourse"));
	case EStationTerminalType::ToMaintenance: return FText::FromString(TEXT("Enter Maintenance Dock"));
	case EStationTerminalType::ToHabitation:  return FText::FromString(TEXT("Enter Habitation"));
	case EStationTerminalType::Service:       return FText::FromString(ServicePrompt);
	default:                                  return FText::FromString(TEXT("Return to Ship"));
	}
}

void AStationTerminal::Interact_Implementation(AAdastreaPlayerController* PC)
{
	if (!PC)
	{
		return;
	}
	if (TerminalType == EStationTerminalType::Service)
	{
		PC->ShowHUDMessage(ServiceMessage, 3.0f, true);
		return;
	}
	PC->HandleStationTerminalUsed(TerminalType);
}

// ====================
// AStationInterior
// ====================

AStationInterior::AStationInterior()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(kCubePath);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(kBaseMaterialPath);
	if (CubeFinder.Succeeded())
	{
		CubeMesh = CubeFinder.Object;
	}
	if (MatFinder.Succeeded())
	{
		BaseMaterial = MatFinder.Object;
	}
}

FTransform AStationInterior::GetArrivalTransform() const
{
	// Beside the room's return door, facing into the room (+X).
	return FTransform(FRotator::ZeroRotator, ArrivalLocal) * GetActorTransform();
}

void AStationInterior::BeginPlay()
{
	Super::BeginPlay();
	BuildLayout();
}

UStaticMeshComponent* AStationInterior::AddBox(const FName& Name, const FVector& Center,
	const FVector& HalfExtent, const FLinearColor& Color, bool bBlocksPawn)
{
	UStaticMeshComponent* Box = NewObject<UStaticMeshComponent>(this, Name);
	Box->SetStaticMesh(CubeMesh);
	Box->SetupAttachment(GetRootComponent());
	Box->SetRelativeLocation(Center);
	Box->SetRelativeScale3D(HalfExtent * 2.0f / 100.0f); // the engine cube is 100uu on a side
	if (bBlocksPawn)
	{
		Box->SetCollisionProfileName(TEXT("BlockAll"));
	}
	else
	{
		Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Box->SetCastShadow(false);
	}
	if (UMaterialInstanceDynamic* MID = MakeTinted(this, BaseMaterial, Color))
	{
		Box->SetMaterial(0, MID);
	}
	Box->RegisterComponent();
	return Box;
}

void AStationInterior::AddLight(const FVector& Center, const FLinearColor& Color, float Intensity, float Radius)
{
	UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
	Light->SetupAttachment(GetRootComponent());
	Light->SetRelativeLocation(Center);
	Light->SetLightColor(Color);
	Light->SetIntensity(Intensity);
	Light->SetAttenuationRadius(Radius);
	Light->SetCastShadows(false);
	Light->RegisterComponent();
}

void AStationInterior::AddLabel(const FText& Text, const FVector& Center, float Yaw, const FLinearColor& Color)
{
	UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
	Label->SetupAttachment(GetRootComponent());
	Label->SetRelativeLocation(Center);
	Label->SetRelativeRotation(FRotator(0.0f, Yaw, 0.0f));
	Label->SetText(Text);
	Label->SetWorldSize(40.0f);
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextCenter);
	Label->SetTextRenderColor(Color.ToFColor(true));
	Label->RegisterComponent();
}

AStationTerminal* AStationInterior::AddTerminal(EStationTerminalType Type, const FVector& Center, float Yaw)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform Local(FRotator(0.0f, Yaw, 0.0f), Center);
	AStationTerminal* Terminal = World->SpawnActor<AStationTerminal>(
		AStationTerminal::StaticClass(), Local * GetActorTransform(), Params);
	if (Terminal)
	{
		Terminal->SetTerminalType(Type);
		Terminal->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
	return Terminal;
}

void AStationInterior::AddKiosk(EStationTerminalType Type, const FVector& Pos, float Yaw, const TCHAR* Sign,
	const TCHAR* ServicePrompt, const TCHAR* ServiceMessage)
{
	const FLinearColor Col = TerminalColor(Type);
	AddBox(*FString::Printf(TEXT("Pad_%d"), GetComponents().Num()), Pos + FVector(0, 0, 1),
		FVector(120, 120, 1), Col * 0.6f, false);
	if (AStationTerminal* Terminal = AddTerminal(Type, Pos, Yaw))
	{
		if (Type == EStationTerminalType::Service)
		{
			Terminal->SetService(ServicePrompt ? ServicePrompt : TEXT("Use Terminal"),
				ServiceMessage ? ServiceMessage : TEXT("Not available yet"));
		}
	}
	// Sign sits a hair in front of the wall the kiosk faces away from.
	const FVector Back = FRotator(0.0f, Yaw, 0.0f).RotateVector(FVector(-1, 0, 0));
	AddLabel(FText::FromString(Sign), Pos + Back * 35.0f + FVector(0, 0, 250), Yaw, Col);
	AddLight(Pos + FVector(0, 0, 300) - Back * 60.0f, Col, 2500.0f, 700.0f);
}

void AStationInterior::BuildShell(float HalfLen, float HalfWid, float H, const FLinearColor& WallCol)
{
	const FLinearColor FloorCol(0.07f, 0.08f, 0.10f);
	const FLinearColor CeilCol(0.20f, 0.22f, 0.27f);
	AddBox(TEXT("Floor"),   FVector(0, 0, -kWall),    FVector(HalfLen + kWall, HalfWid + kWall, kWall), FloorCol);
	AddBox(TEXT("Ceiling"), FVector(0, 0, H + kWall), FVector(HalfLen + kWall, HalfWid + kWall, kWall), CeilCol);
	AddBox(TEXT("WallW"), FVector(-HalfLen - kWall, 0, H * 0.5f), FVector(kWall, HalfWid + kWall, H * 0.5f), WallCol);
	AddBox(TEXT("WallE"), FVector( HalfLen + kWall, 0, H * 0.5f), FVector(kWall, HalfWid + kWall, H * 0.5f), WallCol);
	AddBox(TEXT("WallN"), FVector(0,  HalfWid + kWall, H * 0.5f), FVector(HalfLen + kWall, kWall, H * 0.5f), WallCol);
	AddBox(TEXT("WallS"), FVector(0, -HalfWid - kWall, H * 0.5f), FVector(HalfLen + kWall, kWall, H * 0.5f), WallCol);
}

void AStationInterior::BuildLayout()
{
	if (!CubeMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("AStationInterior: engine cube mesh missing; interior not built."));
		return;
	}

	switch (Room)
	{
	case EStationRoom::Maintenance: BuildMaintenance(); break;
	case EStationRoom::Habitation:  BuildHabitation();  break;
	default:                        BuildConcourse();   break;
	}
}

// --------------------------------------------------------------------
// Concourse: airlock room + central hall with kiosks to each department.
// --------------------------------------------------------------------
void AStationInterior::BuildConcourse()
{
	const float HalfLen = 1400.0f;
	const float HalfWid = 700.0f;
	const float H = 400.0f;
	const FLinearColor WallCol(0.16f, 0.19f, 0.24f);
	const FLinearColor Trim(0.30f, 0.65f, 0.95f);
	ArrivalLocal = FVector(-1250.0f, 0.0f, 100.0f);

	BuildShell(HalfLen, HalfWid, H, WallCol);

	// Airlock bulkhead at X=-1000 with a 200-wide doorway.
	const float DoorHalf = 100.0f;
	const float SegHalf = (HalfWid - DoorHalf) * 0.5f;
	AddBox(TEXT("BulkheadN"), FVector(-1000, DoorHalf + SegHalf, H * 0.5f), FVector(kWall, SegHalf, H * 0.5f), WallCol);
	AddBox(TEXT("BulkheadS"), FVector(-1000, -(DoorHalf + SegHalf), H * 0.5f), FVector(kWall, SegHalf, H * 0.5f), WallCol);
	AddBox(TEXT("DoorLintel"), FVector(-1000, 0, H - 50.0f), FVector(kWall, DoorHalf, 50.0f), WallCol);
	AddBox(TEXT("DoorTrimL"), FVector(-1000, DoorHalf, 150.0f), FVector(kWall + 4.0f, 4.0f, 150.0f), Trim, false);
	AddBox(TEXT("DoorTrimR"), FVector(-1000, -DoorHalf, 150.0f), FVector(kWall + 4.0f, 4.0f, 150.0f), Trim, false);

	// Concourse dressing.
	AddBox(TEXT("CenterStrip"), FVector(100, 0, 1), FVector(1150, 80, 1), Trim, false);
	AddBox(TEXT("CeilingStripA"), FVector(-300, 0, H - 2.0f), FVector(300, 40, 2), FLinearColor(0.9f, 0.95f, 1.0f), false);
	AddBox(TEXT("CeilingStripB"), FVector(700, 0, H - 2.0f), FVector(300, 40, 2), FLinearColor(0.9f, 0.95f, 1.0f), false);
	AddBox(TEXT("AirlockStripe"), FVector(-1200, 0, 1), FVector(150, 100, 1), FLinearColor(0.95f, 0.75f, 0.10f), false);
	for (const float PX : { -500.0f, 700.0f })
	{
		AddBox(*FString::Printf(TEXT("PillarN_%d"), (int32)PX), FVector(PX,  350, H * 0.5f), FVector(40, 40, H * 0.5f), WallCol);
		AddBox(*FString::Printf(TEXT("PillarS_%d"), (int32)PX), FVector(PX, -350, H * 0.5f), FVector(40, 40, H * 0.5f), WallCol);
	}
	AddBox(TEXT("IslandBase"), FVector(300, 0, 25), FVector(150, 90, 25), FLinearColor(0.12f, 0.14f, 0.18f));
	AddBox(TEXT("IslandPlanter"), FVector(300, 0, 60), FVector(120, 60, 10), FLinearColor(0.10f, 0.45f, 0.20f), false);

	// Kiosks: airlock, trading, and doors to the other two rooms.
	AddKiosk(EStationTerminalType::Airlock,       FVector(-1350,    0, 0),   0.0f, TEXT("AIRLOCK - RETURN TO SHIP"));
	AddKiosk(EStationTerminalType::Trading,       FVector(  200,  640, 0), -90.0f, TEXT("TRADING DEPARTMENT"));
	AddKiosk(EStationTerminalType::ToMaintenance, FVector(  500, -640, 0),  90.0f, TEXT("MAINTENANCE DOCK"));
	AddKiosk(EStationTerminalType::ToHabitation,  FVector( 1350,    0, 0), 180.0f, TEXT("HABITATION"));

	// General lighting (this location has no scene lights).
	AddLight(FVector(-1200, 0, H - 60.0f), FLinearColor(1.0f, 0.8f, 0.5f), 4000.0f, 900.0f);
	for (const float LX : { -700.0f, -100.0f, 500.0f, 1100.0f })
	{
		AddLight(FVector(LX, 0, H - 60.0f), FLinearColor(0.85f, 0.92f, 1.0f), 6000.0f, 1300.0f);
	}
}

// --------------------------------------------------------------------
// Maintenance dock: tall hangar bay with a ship cradle and overhead gantries.
// --------------------------------------------------------------------
void AStationInterior::BuildMaintenance()
{
	const float HalfLen = 1600.0f;
	const float HalfWid = 1000.0f;
	const float H = 700.0f;
	const FLinearColor WallCol(0.20f, 0.20f, 0.22f);
	const FLinearColor Hazard(0.95f, 0.75f, 0.10f);
	const FLinearColor Steel(0.30f, 0.32f, 0.36f);
	ArrivalLocal = FVector(-1450.0f, 0.0f, 100.0f);

	BuildShell(HalfLen, HalfWid, H, WallCol);

	// Ship cradle: a raised platform with a simple hull-and-wing silhouette on it.
	AddBox(TEXT("CradleBase"), FVector(100, 0, 20), FVector(600, 380, 20), FLinearColor(0.12f, 0.12f, 0.14f));
	AddBox(TEXT("CradleHazardL"), FVector(100, 385, 41), FVector(600, 8, 1), Hazard, false);
	AddBox(TEXT("CradleHazardR"), FVector(100, -385, 41), FVector(600, 8, 1), Hazard, false);
	AddBox(TEXT("ShipHull"),  FVector(100, 0, 150), FVector(420, 90, 90), FLinearColor(0.35f, 0.38f, 0.44f));
	AddBox(TEXT("ShipNose"),  FVector(560, 0, 140), FVector(70, 55, 55), FLinearColor(0.35f, 0.38f, 0.44f));
	AddBox(TEXT("ShipWingL"), FVector(0, 210, 120), FVector(200, 130, 10), FLinearColor(0.28f, 0.31f, 0.36f));
	AddBox(TEXT("ShipWingR"), FVector(0, -210, 120), FVector(200, 130, 10), FLinearColor(0.28f, 0.31f, 0.36f));
	AddBox(TEXT("ShipEngine"), FVector(-370, 0, 150), FVector(50, 70, 60), FLinearColor(0.15f, 0.17f, 0.20f));
	AddBox(TEXT("ShipGlow"), FVector(-425, 0, 150), FVector(4, 55, 45), FLinearColor(0.30f, 0.70f, 1.0f), false);

	// Overhead gantry rails and a hanging rig.
	AddBox(TEXT("GantryRailL"), FVector(0, 500, H - 120.0f), FVector(1300, 20, 20), Steel, false);
	AddBox(TEXT("GantryRailR"), FVector(0, -500, H - 120.0f), FVector(1300, 20, 20), Steel, false);
	AddBox(TEXT("GantryBeam"), FVector(150, 0, H - 120.0f), FVector(20, 500, 20), Hazard, false);

	// Pillars, tool racks along the north wall, crates by the south wall.
	for (const float PX : { -1000.0f, 0.0f, 1000.0f })
	{
		AddBox(*FString::Printf(TEXT("HPillarN_%d"), (int32)PX), FVector(PX,  800, H * 0.5f), FVector(50, 50, H * 0.5f), WallCol);
		AddBox(*FString::Printf(TEXT("HPillarS_%d"), (int32)PX), FVector(PX, -800, H * 0.5f), FVector(50, 50, H * 0.5f), WallCol);
	}
	for (int32 i = 0; i < 4; ++i)
	{
		AddBox(*FString::Printf(TEXT("ToolRack_%d"), i), FVector(-750.0f + i * 250.0f, 960, 120), FVector(80, 20, 120), Steel);
		AddBox(*FString::Printf(TEXT("Crate_%d"), i), FVector(-600.0f + i * 260.0f, -900, 50 + (i % 2) * 20), FVector(70, 70, 50 + (i % 2) * 20), FLinearColor(0.30f, 0.22f, 0.12f));
	}
	AddBox(TEXT("FloorLane"), FVector(-700, 0, 1), FVector(650, 60, 1), Hazard, false);

	// Kiosks: return to the concourse, plus service terminals (not implemented yet).
	AddKiosk(EStationTerminalType::ToConcourse, FVector(-1550, 0, 0), 0.0f, TEXT("CONCOURSE"));
	AddKiosk(EStationTerminalType::Service, FVector(1500, 500, 0), 180.0f, TEXT("HULL REPAIR"),
		TEXT("Repair Hull"), TEXT("Hull repair is not available yet"));
	AddKiosk(EStationTerminalType::Service, FVector(1500, -500, 0), 180.0f, TEXT("SHIP REFIT"),
		TEXT("Refit Ship"), TEXT("Ship refits are not available yet"));

	// Bright industrial lighting.
	for (const float LX : { -1000.0f, -250.0f, 500.0f, 1200.0f })
	{
		AddLight(FVector(LX,  400, H - 80.0f), FLinearColor(1.0f, 0.97f, 0.9f), 12000.0f, 1800.0f);
		AddLight(FVector(LX, -400, H - 80.0f), FLinearColor(1.0f, 0.97f, 0.9f), 12000.0f, 1800.0f);
	}
}

// --------------------------------------------------------------------
// Habitation: crew cabins along the north side, lounge and bar to the south.
// --------------------------------------------------------------------
void AStationInterior::BuildHabitation()
{
	const float HalfLen = 1100.0f;
	const float HalfWid = 700.0f;
	const float H = 350.0f;
	const FLinearColor WallCol(0.30f, 0.26f, 0.22f);
	const FLinearColor Wood(0.35f, 0.22f, 0.12f);
	const FLinearColor Fabric(0.20f, 0.30f, 0.45f);
	ArrivalLocal = FVector(-950.0f, 0.0f, 100.0f);

	BuildShell(HalfLen, HalfWid, H, WallCol);

	// Four crew cabins along the north wall, each 400 wide x 350 deep, doors facing the lounge.
	const float CabinWallY = 350.0f;
	for (int32 i = 0; i < 4; ++i)
	{
		const float CX = -600.0f + i * 400.0f;  // cabin centre X
		// Front wall (Y=350) split around a 120-wide door.
		AddBox(*FString::Printf(TEXT("CabinFrontA_%d"), i), FVector(CX - 130.0f, CabinWallY, H * 0.5f), FVector(70, kWall, H * 0.5f), WallCol);
		AddBox(*FString::Printf(TEXT("CabinFrontB_%d"), i), FVector(CX + 130.0f, CabinWallY, H * 0.5f), FVector(70, kWall, H * 0.5f), WallCol);
		AddBox(*FString::Printf(TEXT("CabinLintel_%d"), i), FVector(CX, CabinWallY, H - 50.0f), FVector(60, kWall, 50), WallCol);
		// Divider walls between cabins (not at the outer ends).
		if (i > 0)
		{
			AddBox(*FString::Printf(TEXT("CabinDivider_%d"), i), FVector(CX - 200.0f, 525.0f, H * 0.5f), FVector(kWall, 175, H * 0.5f), WallCol);
		}
		// Bunk, desk and a warm reading light inside.
		AddBox(*FString::Printf(TEXT("Bunk_%d"), i), FVector(CX - 110.0f, 620.0f, 40), FVector(70, 40, 40), Fabric);
		AddBox(*FString::Printf(TEXT("BunkBlanket_%d"), i), FVector(CX - 110.0f, 620.0f, 85), FVector(70, 40, 5), FLinearColor(0.75f, 0.35f, 0.25f), false);
		AddBox(*FString::Printf(TEXT("Desk_%d"), i), FVector(CX + 120.0f, 660.0f, 40), FVector(60, 25, 40), Wood);
		AddLight(FVector(CX, 520.0f, H - 60.0f), FLinearColor(1.0f, 0.75f, 0.5f), 2500.0f, 500.0f);
	}

	// Lounge: two sofa groups with tables, and a planter.
	for (int32 g = 0; g < 2; ++g)
	{
		const float GX = -300.0f + g * 700.0f;
		AddBox(*FString::Printf(TEXT("Table_%d"), g), FVector(GX, -150, 30), FVector(70, 70, 30), Wood);
		AddBox(*FString::Printf(TEXT("SofaBack_%d"), g), FVector(GX - 140.0f, -150, 30), FVector(25, 110, 30), Fabric);
		AddBox(*FString::Printf(TEXT("SofaFront_%d"), g), FVector(GX + 140.0f, -150, 30), FVector(25, 110, 30), Fabric);
	}
	AddBox(TEXT("LoungeRug"), FVector(-300, -150, 1), FVector(220, 200, 1), FLinearColor(0.45f, 0.15f, 0.15f), false);
	AddBox(TEXT("LoungeRug2"), FVector(400, -150, 1), FVector(220, 200, 1), FLinearColor(0.15f, 0.30f, 0.20f), false);
	AddBox(TEXT("Planter"), FVector(-800, -560, 40), FVector(60, 60, 40), FLinearColor(0.10f, 0.45f, 0.20f));

	// Bar along the south wall.
	AddBox(TEXT("BarCounter"), FVector(600, -600, 55), FVector(220, 35, 55), Wood);
	AddBox(TEXT("BarShelf"), FVector(600, -685, 150), FVector(220, 12, 6), Wood, false);

	// Kiosks: return to the concourse; a bartender service (not implemented yet).
	AddKiosk(EStationTerminalType::ToConcourse, FVector(-1050, 0, 0), 0.0f, TEXT("CONCOURSE"));
	AddKiosk(EStationTerminalType::Service, FVector(1050, -300, 0), 180.0f, TEXT("LOUNGE BAR"),
		TEXT("Talk to Bartender"), TEXT("The bar is not staffed yet"));

	// Warm lighting.
	for (const float LX : { -700.0f, -100.0f, 500.0f })
	{
		AddLight(FVector(LX, -150, H - 60.0f), FLinearColor(1.0f, 0.85f, 0.65f), 5000.0f, 1000.0f);
	}
}
