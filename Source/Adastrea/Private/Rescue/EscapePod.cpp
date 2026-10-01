#include "Rescue/EscapePod.h"
#include "Rescue/RescueFlight.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace EscapePodTuning
{
	/** Seconds of ejection burn before the pod turns for its pickup. */
	constexpr float EjectSeconds = 1.2f;
	constexpr float EjectKick = 2500.0f;

	/** Distress beacon: blue, a quick double flash every BeaconPeriod seconds. */
	const FLinearColor BeaconColour(0.05f, 0.2f, 1.0f);
	constexpr float BeaconPeriod = 1.4f;
	constexpr float BeaconLensIntensity = 25.0f;
	constexpr float BeaconLightIntensity = 25000.0f;   // cd
	constexpr float BeaconLightRadius = 4000.0f;

	/** 0..1 beacon brightness at time T: two short flashes per period, a faint glow between. */
	float BeaconPulse(float T)
	{
		const float Phase = FMath::Fmod(T, BeaconPeriod) / BeaconPeriod;
		auto Flash = [](float P, float At) { return FMath::Exp(-FMath::Square((P - At) / 0.035f)); };
		return FMath::Max(0.08f, FMath::Max(Flash(Phase, 0.1f), Flash(Phase, 0.24f)));
	}
}

AEscapePod::AEscapePod()
	: Speed(5000.0f)
	, Acceleration(4500.0f)
	, State(EEscapePodState::Ejecting)
	, PodVelocity(FVector::ZeroVector)
	, ArriveTolerance(500.0f)
	, StateTime(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	SetCanBeDamaged(false);
	AutoPossessAI = EAutoPossessAI::Disabled;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HullMat(TEXT("/Game/Materials/M_Mining_Hull.M_Mining_Hull"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BeaconMat(TEXT("/AdastreaShips/Materials/Interiors/M_PropBeacon.M_PropBeacon"));

	auto MakePart = [this](const TCHAR* Name, UStaticMesh* Mesh, UMaterialInterface* Mat)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(Root);
		Part->SetStaticMesh(Mesh);
		if (Mat)
		{
			Part->SetMaterial(0, Mat);
		}
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetGenerateOverlapEvents(false);
		Part->SetMobility(EComponentMobility::Movable);
		return Part;
	};

	// A 2.4 m capsule-ish pod with a thruster cone astern and a beacon on top. +X forward.
	Body = MakePart(TEXT("Body"), SphereMesh.Object, HullMat.Object);
	Body->SetRelativeScale3D(FVector(2.4f, 1.6f, 1.6f));

	Thruster = MakePart(TEXT("Thruster"), ConeMesh.Object, HullMat.Object);
	Thruster->SetRelativeLocation(FVector(-130.0f, 0.0f, 0.0f));
	Thruster->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	Thruster->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.6f));

	Beacon = MakePart(TEXT("Beacon"), SphereMesh.Object, BeaconMat.Object);
	Beacon->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));
	Beacon->SetRelativeScale3D(FVector(0.5f));
	Beacon->SetCastShadow(false);

	BeaconLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BeaconLight"));
	BeaconLight->SetupAttachment(Beacon);
	BeaconLight->SetLightColor(EscapePodTuning::BeaconColour);
	BeaconLight->SetIntensityUnits(ELightUnits::Candelas);
	BeaconLight->SetIntensity(0.0f);
	BeaconLight->SetAttenuationRadius(EscapePodTuning::BeaconLightRadius);
	BeaconLight->SetCastShadows(false);

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(Root);
	CameraArm->TargetArmLength = 1800.0f;
	CameraArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));
	CameraArm->bDoCollisionTest = false;
	CameraArm->bEnableCameraRotationLag = true;
	CameraArm->CameraRotationLagSpeed = 2.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
}

void AEscapePod::BeginPlay()
{
	Super::BeginPlay();
	BeaconMID = Beacon->CreateDynamicMaterialInstance(0);
	if (BeaconMID)
	{
		BeaconMID->SetVectorParameterValue(TEXT("Color"), EscapePodTuning::BeaconColour);
	}
}

void AEscapePod::Launch(const FVector& InheritedVelocity, const FVector& Up)
{
	PodVelocity = InheritedVelocity + Up.GetSafeNormal() * EscapePodTuning::EjectKick;
	State = EEscapePodState::Ejecting;
	StateTime = 0.0f;
}

void AEscapePod::SetDestination(AActor* Target, float Tolerance)
{
	Destination = Target;
	ArriveTolerance = Tolerance;
	if (State == EEscapePodState::Aboard)
	{
		State = EEscapePodState::Transit;
		SetActorHiddenInGame(false);
	}
}

void AEscapePod::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StateTime += DeltaSeconds;

	const float Pulse = EscapePodTuning::BeaconPulse(GetWorld()->GetTimeSeconds());
	if (BeaconMID)
	{
		BeaconMID->SetScalarParameterValue(TEXT("Intensity"), EscapePodTuning::BeaconLensIntensity * Pulse);
	}
	BeaconLight->SetIntensity(EscapePodTuning::BeaconLightIntensity * Pulse);

	switch (State)
	{
	case EEscapePodState::Ejecting:
		SetActorLocation(GetActorLocation() + PodVelocity * DeltaSeconds);
		if (StateTime >= EscapePodTuning::EjectSeconds)
		{
			State = EEscapePodState::Transit;
			StateTime = 0.0f;
		}
		break;

	case EEscapePodState::Transit:
		if (AActor* Target = Destination.Get())
		{
			if (RescueFlight::FlyToward(this, PodVelocity, Target->GetActorLocation(), Target->GetVelocity(),
				Speed, Acceleration, DeltaSeconds, true) < ArriveTolerance)
			{
				State = EEscapePodState::Aboard;
				PodVelocity = FVector::ZeroVector;
				SetActorHiddenInGame(true);
			}
		}
		break;

	case EEscapePodState::Aboard:
		// Ride along with whatever took us aboard.
		if (const AActor* Target = Destination.Get())
		{
			SetActorLocation(Target->GetActorLocation());
		}
		break;
	}
}
