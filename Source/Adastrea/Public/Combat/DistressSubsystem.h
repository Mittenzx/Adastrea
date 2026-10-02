#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Universe/GalaxySubsystem.h"
#include "DistressSubsystem.generated.h"

class APatrolController;
class ASpaceship;

/**
 * Distress calls and who answers them.
 *
 * A Civil ship (the player, a trader, a patrol) shot by a hostile raises a distress
 * call (UShipHealthComponent reports every hit). The sector's security
 * (UGalaxySubsystem::GetCurrentSecurity) decides the answer:
 *
 *   High    up to 3 patrols, sent at once
 *   Medium  up to 2 patrols, sent after 15 s
 *   Low     1 patrol, sent after 40 s
 *   None    nobody comes
 *
 * The nearest free APatrolControllers in the level are sent. A call closes once no
 * hostile has been within CallRadius of it for a few seconds, or a while after the
 * last shot. The player is told what's happening through HUD messages.
 *
 * Console: adastrea.Security 0-3 overrides the sector's security; SpawnPatrols [N].
 */
UCLASS()
class ADASTREA_API UDistressSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Hostiles within this distance of a call (cm) keep it open, and patrols answering it attack them. */
	static constexpr float CallRadius = 40000.0f;

	static UDistressSubsystem* Get(const UObject* WorldContext);

	/** Victim was hit by Attacker. Raises (or refreshes) a distress call if a hostile shot a Civil ship. */
	void ReportAttack(ASpaceship* Victim, AActor* Attacker);

	/** Where call CallId is now. False once the call has closed. */
	bool GetCallLocation(int32 CallId, FVector& OutLocation) const;

	/** Number of open calls (for tests and debug UI). */
	int32 GetOpenCallCount() const { return Calls.Num(); }

	/** How many patrols a sector of this security sends, and how long it takes to send them (s). */
	static void GetResponse(ESectorSecurity Security, int32& OutMaxPatrols, float& OutDispatchDelay);

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	struct FDistressCall
	{
		int32 Id = INDEX_NONE;
		TWeakObjectPtr<ASpaceship> Victim;
		FVector Location = FVector::ZeroVector;
		ESectorSecurity Security = ESectorSecurity::None;
		double DispatchTime = 0.0;
		double LastAttackTime = 0.0;
		/** When hostiles were last seen near the call. */
		double LastHostileTime = 0.0;
		bool bDispatched = false;
		/** The player's own ship called: tell them what's happening. */
		bool bPlayerCall = false;
		TArray<TWeakObjectPtr<APatrolController>> Responders;
	};

	void Dispatch(FDistressCall& Call);
	bool AnyHostileNear(const FVector& Location) const;
	void Message(const FDistressCall& Call, const FString& Text, float Seconds, bool bWarning = false) const;
	FString DescribeResponders(const TArray<APatrolController*>& Patrols) const;

	TArray<FDistressCall> Calls;
	int32 NextCallId = 1;
};
