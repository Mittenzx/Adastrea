// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/WorldInteractable.h"
#include "StationInterior.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

/** Which walkable room of the station an AStationInterior builds. */
UENUM(BlueprintType)
enum class EStationRoom : uint8
{
	/** Central hub: airlock, plus kiosks to the other departments. */
	Concourse,
	/** Hangar bay with a ship cradle, gantries and tool racks. */
	Maintenance,
	/** Lounge and crew cabins. */
	Habitation,
};

/** What an on-foot station terminal does when the player presses E on it. */
UENUM(BlueprintType)
enum class EStationTerminalType : uint8
{
	/** Return to the ship and open the market/trade screen. */
	Trading,
	/** Return to the ship's docked services menu. */
	Airlock,
	/** Walk through to the concourse. */
	ToConcourse,
	/** Walk through to the maintenance dock. */
	ToMaintenance,
	/** Walk through to the habitation deck. */
	ToHabitation,
	/** A room-specific service; shows its message (not implemented yet). */
	Service,
};

/**
 * An interactable console/kiosk inside a walkable station interior. Implements
 * IWorldInteractable so the avatar's E key finds it; the controller decides what each
 * terminal type does (see AAdastreaPlayerController::HandleStationTerminalUsed).
 */
UCLASS()
class ADASTREA_API AStationTerminal : public AActor, public IWorldInteractable
{
	GENERATED_BODY()

public:
	AStationTerminal();

	/** Set the terminal's function and tint/label it accordingly. */
	void SetTerminalType(EStationTerminalType InType);

	/** Configure a Service terminal's prompt and the message shown when used. */
	void SetService(const FString& InPrompt, const FString& InMessage);

	UPROPERTY(BlueprintReadOnly, Category="Station")
	EStationTerminalType TerminalType = EStationTerminalType::Airlock;

	FString ServicePrompt;
	FString ServiceMessage;

	// ~ IWorldInteractable
	virtual FText GetInteractPrompt_Implementation() const override;
	virtual void Interact_Implementation(AAdastreaPlayerController* PC) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station")
	TObjectPtr<UStaticMeshComponent> Pedestal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station")
	TObjectPtr<UStaticMeshComponent> Screen;
};

/**
 * A walkable space-station interior the player explores on foot after docking.
 *
 * The geometry is built procedurally at BeginPlay from engine primitives (no imported
 * assets required). Room selects the layout: the concourse hub (airlock + kiosks), the
 * maintenance hangar, or the habitation deck. The actor is spawned far from
 * the sector so nothing else intrudes; it carries its own lights, since that location has
 * no scene lighting.
 *
 * Local space: X = concourse length (airlock at -X), Y = width, Z = up, floor top at Z=0.
 */
UCLASS()
class ADASTREA_API AStationInterior : public AActor
{
	GENERATED_BODY()

public:
	AStationInterior();

	/** Which room to build. Must be set before BeginPlay (use deferred spawn). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station")
	EStationRoom Room = EStationRoom::Concourse;

	/** World transform where the avatar should be placed on arrival (by the room's return door). */
	FTransform GetArrivalTransform() const;

protected:
	virtual void BeginPlay() override;

	/** Build the selected room. */
	void BuildLayout();
	void BuildConcourse();
	void BuildMaintenance();
	void BuildHabitation();

	/** Floor, ceiling and four outer walls for a room of the given half-extents. */
	void BuildShell(float HalfLen, float HalfWid, float H, const FLinearColor& WallCol);

	/** Floor pad + terminal + wall sign + colour light for one kiosk. */
	void AddKiosk(EStationTerminalType Type, const FVector& Pos, float Yaw, const TCHAR* Sign,
		const TCHAR* ServicePrompt = nullptr, const TCHAR* ServiceMessage = nullptr);

	/** Add a solid, tinted box. Center/HalfExtent are local, in world units. */
	UStaticMeshComponent* AddBox(const FName& Name, const FVector& Center, const FVector& HalfExtent, const FLinearColor& Color, bool bBlocksPawn = true);

	void AddLight(const FVector& Center, const FLinearColor& Color, float Intensity, float Radius);
	void AddLabel(const FText& Text, const FVector& Center, float Yaw, const FLinearColor& Color);
	AStationTerminal* AddTerminal(EStationTerminalType Type, const FVector& Center, float Yaw);

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BaseMaterial;

	/** Room key used to pick textured surface instances in AddBox
	 * ("Concourse" / "Maint" / "Hab"; set by each Build* function). */
	FString SurfaceRoom;

	/** Textured M_IntSurface_Oriented instance for a box of this name in the
	 * current room, or nullptr to keep the tinted BasicShapeMaterial. */
	UMaterialInterface* FindSurfaceMaterial(const FName& BoxName) const;

	/** Where the avatar arrives, in local space (set per room while building). */
	FVector ArrivalLocal = FVector(-1250.0f, 0.0f, 100.0f);
};
