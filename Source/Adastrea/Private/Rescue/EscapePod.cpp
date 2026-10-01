#include "Rescue/EscapePod.h"
#include "Rescue/RescueFlight.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace EscapePodTuning
{
	/** Seconds of ejection burn before the pod turns for its pickup. */
	constexpr float EjectSeconds = 1.2f;
	constexpr float EjectKick = 2500.0f;
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
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LampMat(TEXT("/Game/Materials/M_NavLights.M_NavLights"));

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

	Beacon = MakePart(TEXT("Beacon"), SphereMesh.Object, LampMat.Object);
	Beacon->SetRelativeLocation(FVector(0.0f, 0.0f, 85.0f));
	Beacon->SetRelativeScale3D(FVector(0.35f));
	Beacon->SetCastShadow(false);

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
