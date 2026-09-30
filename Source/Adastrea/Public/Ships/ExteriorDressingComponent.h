#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ExteriorDressingComponent.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;

/** Which layout of exterior props to fit to the hull. */
UENUM(BlueprintType)
enum class EExteriorDressingPreset : uint8
{
	/** Nav lights on the wingtips (red port, green starboard), white stern light,
	 * strobes, red anti-collision beacons top and bottom, an antenna and a dish. */
	Ship,
	/** Corner beacons, floodlights washing the module, antenna/comm array. */
	StationModule,
	/** StationModule plus docking guide chevrons and a traffic-control beacon. */
	DockingBay,
};

/** How a dressing light behaves. */
UENUM(BlueprintType)
enum class EExteriorLightMode : uint8
{
	Steady,
	Strobe,      // double flash
	Rotating,    // spinning beacon head
	Pulse,       // slow breathe (docking guides, signs)
};

USTRUCT()
struct FExteriorDressingLight
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MID;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> Spinner;

	FLinearColor Colour = FLinearColor::White;
	EExteriorLightMode Mode = EExteriorLightMode::Steady;
	float Intensity = 20.0f;
	float Phase = 0.0f;
};

/**
 * Fits exterior props (Tools/exterior_props.py -> SM_ExtProp_*) to the owner's
 * hull at runtime: it line-traces the largest visible static mesh to find the
 * real wingtips, spine, belly and stern, attaches props there aligned to the
 * surface, and animates the lights (nav lights, strobes, rotating beacons,
 * pulsing docking guides).
 *
 * ASpaceship and ASpaceStationModule add one automatically (unless the actor
 * already has one, so a Blueprint can place and tune its own). The console
 * variable adastrea.ExteriorDressing 0 turns the automatic dressing off.
 */
UCLASS(ClassGroup=(Adastrea), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UExteriorDressingComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UExteriorDressingComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exterior Dressing")
	EExteriorDressingPreset Preset = EExteriorDressingPreset::Ship;

	/** Size multiplier for every prop (1 = authored real-world size). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exterior Dressing", meta=(ClampMin="0.1", ClampMax="10"))
	float PropScale = 1.0f;

	/** Varies which antenna/dish a hull gets; 0 = derive from the owner's name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exterior Dressing")
	int32 Seed = 0;

	/** Mesh to dress; empty = the largest visible static mesh on the owner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exterior Dressing")
	TObjectPtr<UStaticMeshComponent> TargetMesh;

	/** Remove and re-fit all props (e.g. after swapping the hull mesh). */
	UFUNCTION(BlueprintCallable, Category="Exterior Dressing")
	void Dress();

	UFUNCTION(BlueprintCallable, Category="Exterior Dressing")
	void ClearDressing();

	/** Number of props currently fitted. */
	UFUNCTION(BlueprintPure, Category="Exterior Dressing")
	int32 GetPropCount() const { return Props.Num(); }

	/** True when the adastrea.ExteriorDressing cvar allows automatic dressing. */
	static bool IsAutoDressingEnabled();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UStaticMeshComponent* FindTarget() const;

	/** Trace the target toward its centre along -Dir from outside the bounds at
	 * the given local offsets; returns the outermost hit (local space). */
	bool Probe(const FVector& Dir, const TArray<FVector>& Starts, FVector& OutLoc, FVector& OutNormal) const;

	/** Attach SM_ExtProp_<Name> at a local point with +Z along Normal and +X toward Facing. */
	UStaticMeshComponent* AddProp(FName Name, const FVector& Loc, const FVector& Normal, const FVector& Facing, float Scale = 1.0f);

	// One line on purpose: Tools/check_uproperty.py reads a wrapped "UStaticMeshComponent* X = nullptr);" as a member.
	void AddLight(UStaticMeshComponent* Prop, const FLinearColor& Colour, EExteriorLightMode Mode, float Intensity, float Phase = 0.0f, UStaticMeshComponent* Spinner = nullptr);

	void DressShip();
	void DressStation(bool bDockingBay);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> Props;

	UPROPERTY(Transient)
	TArray<FExteriorDressingLight> Lights;

	/** Local bounds of the target mesh (component space). */
	FBox LocalBox;
	FTimerHandle RetryTimer;
	float Clock = 0.0f;
};
