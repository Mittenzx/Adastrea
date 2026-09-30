#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteriorFixture.generated.h"

class ASpaceshipInterior;
class AAdastreaPlayerController;
class UCanvas;
class UCanvasRenderTarget2D;
class UMaterialInstanceDynamic;
class UPlayerInteractableComponent;
class UStaticMeshComponent;

/** What a fixture does when the avatar uses it (E). */
UENUM(BlueprintType)
enum class EInteriorFixtureKind : uint8
{
	/** Live exterior camera feed; E cycles the feed. */
	Monitor,
	/** Terminal with a live readout screen; E opens the matching HUD screen. */
	Console,
	/** Red-alert pedestal: E toggles alert lighting and the klaxon. */
	AlertButton,
	/** Lighting panel: E cycles normal / dimmed / emergency. */
	LightSwitch,
	/** Crew intercom: E pages the crew. */
	Intercom,
	/** Coffee station: E pours a cup. */
	Dispenser,
	/** Nav table: E opens the galaxy map. */
	MapTerminal,
};

/** Exterior camera positions around the ship's hull. */
UENUM(BlueprintType)
enum class EExteriorFeed : uint8
{
	Bow,
	Stern,
	Chase,
	Dorsal,
	Ventral,
	Port,
	Starboard,
};

/** Readout a console screen shows. */
UENUM(BlueprintType)
enum class EConsoleReadout : uint8
{
	ShipStatus,
	Navigation,
	Engineering,
	Cargo,
	Comms,
};

/**
 * A small interactive or live fixture inside a ship interior: exterior
 * monitors, terminals, the red-alert button, light panel, intercom, coffee
 * station and nav table.
 *
 * ASpaceshipInterior spawns one per X_<Fixture>_<Arg>_<N> socket on a full-deck
 * shell (Tools/build_ship_decks.py places them, Tools/build_interior_props.py
 * makes the SM_Prop_Fx_* meshes). They can also be placed or spawned by hand:
 * call Setup() with the interior they belong to.
 *
 * Screens: monitors show a shared exterior render target owned by the interior
 * (one scene capture per feed per ship, only while the player is aboard);
 * consoles draw their readout into a small canvas render target, refreshed a
 * few times a second while the avatar is near.
 */
UCLASS(BlueprintType)
class ADASTREA_API AInteriorFixture : public AActor
{
	GENERATED_BODY()

public:
	AInteriorFixture();

	/**
	 * Configure the fixture. Picks the default mesh for Kind (SM_Prop_Fx_<MeshName>)
	 * unless one is already set.
	 * @param InInterior Interior this fixture belongs to (feeds, lighting, ship data).
	 * @param InKind What it does.
	 * @param Arg Feed name for monitors, readout name for consoles (e.g. "Bow", "Cargo").
	 * @param MeshName Fixture mesh variant (MonitorWall, Terminal, ...).
	 */
	UFUNCTION(BlueprintCallable, Category="Interior|Fixture")
	void Setup(ASpaceshipInterior* InInterior, EInteriorFixtureKind InKind, FName Arg, FName MeshName);

	/** Screens run only while the player is aboard (the interior toggles this). */
	UFUNCTION(BlueprintCallable, Category="Interior|Fixture")
	void SetFixtureActive(bool bActive);

	UFUNCTION(BlueprintPure, Category="Interior|Fixture")
	EInteriorFixtureKind GetKind() const { return Kind; }

	UFUNCTION(BlueprintPure, Category="Interior|Fixture")
	EExteriorFeed GetFeed() const { return Feed; }

	/** Map a socket/arg name to an enum; false if the name is unknown. */
	static bool ParseKind(const FString& MeshName, EInteriorFixtureKind& OutKind);
	static bool ParseFeed(const FString& Name, EExteriorFeed& OutFeed);
	static bool ParseReadout(const FString& Name, EConsoleReadout& OutReadout);
	static FString FeedName(EExteriorFeed InFeed);

protected:
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnUsed(AAdastreaPlayerController* PC);

	UFUNCTION()
	void DrawScreen(UCanvas* Canvas, int32 Width, int32 Height);

	void BindMonitorFeed();
	void RefreshPrompt();
	bool IsAvatarNear(float Radius) const;
	void DrawReadout(UCanvas* Canvas, float W, float H);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior|Fixture")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior|Fixture")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior|Fixture")
	TObjectPtr<UPlayerInteractableComponent> Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior|Fixture")
	EInteriorFixtureKind Kind = EInteriorFixtureKind::Console;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior|Fixture")
	EExteriorFeed Feed = EExteriorFeed::Bow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior|Fixture")
	EConsoleReadout Readout = EConsoleReadout::ShipStatus;

	UPROPERTY(Transient)
	TWeakObjectPtr<ASpaceshipInterior> Interior;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ScreenMID;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasRenderTarget2D> ScreenCanvas;

	bool bActive = false;
	float ScreenClock = 0.0f;
	int32 UseCount = 0;
	TArray<FString> CommsLog;

	/** Cups poured by a dispenser (kept to a few, oldest recycled). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> Cups;
};
