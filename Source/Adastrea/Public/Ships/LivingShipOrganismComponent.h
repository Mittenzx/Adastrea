// LivingShipOrganismComponent.h
// NOTE: [POST-MVP][LOW] Living ship system - organic ships with mood and mutation mechanics
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LivingShipOrganismComponent.generated.h"

/**
 * Component for living/organic ships with biological characteristics.
 *
 * This component adds organic behavior to ships, including:
 * - Health that can be healed or damaged through biological processes
 * - Mood system affecting ship performance and dialogue (-100 to +100)
 * - Mutation system allowing ships to evolve over time
 * - Dynamic dialogue based on organism state
 *
 * Living ships are a unique ship class that grows, heals, and responds
 * emotionally to player actions. They communicate through organic
 * dialogue and can mutate to gain new abilities.
 *
 * @note POST-MVP: This is a deferred feature planned for content expansion phase.
 *       Core trading MVP does not include living ships.
 *
 * Usage:
 * - Add component to a BP_LivingShip Blueprint
 * - Configure initial Health, Mood, and MutationLevel
 * - Call Heal/TakeDamage for biological health changes
 * - Call Mutate to trigger evolution events
 * - Use GetOrganismDialogue for dynamic NPC-like ship communication
 *
 * @see ASpaceship - Base ship class this component attaches to
 * @see USpaceshipDataAsset - Ship configuration including living ship variants
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ADASTREA_API ULivingShipOrganismComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULivingShipOrganismComponent();

    /**
     * Current biological health of the living ship.
     * Unlike mechanical hull integrity, this represents organic tissue health.
     * Living ships regenerate health slowly when in a positive mood state.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Organism",
        meta=(ClampMin="0.0", ClampMax="100.0"))
    float Health;

    /**
     * Current emotional state of the living ship.
     * Range: -100 (distressed) ... 0 (neutral) ... +100 (happy)
     *
     * Mood affects:
     * - Ship performance (speed, maneuverability)
     * - Health regeneration rate
     * - Dialogue responses
     * - Willingness to mutate
     *
     * Mood changes based on:
     * - Combat damage (decreases mood)
     * - Successful trades (increases mood)
     * - Time spent docked (increases mood)
     * - Player interaction frequency
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Organism",
        meta=(ClampMin="-100.0", ClampMax="100.0"))
    float Mood;

    /**
     * Current mutation level of the organism.
     * Higher mutation levels unlock unique abilities and visual changes.
     * Range: 0.0 (baseline) to any positive value (highly mutated)
     *
     * Mutation effects:
     * - Level 10+: Enhanced sensors
     * - Level 25+: Organic weapons
     * - Level 50+: Self-repair ability
     * - Level 100+: Consciousness emergence
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Organism",
        meta=(ClampMin="0.0"))
    float MutationLevel;

    /**
     * Heal the organism by the specified amount.
     * @param Amount Amount of health to restore (clamped to max 100)
     */
    UFUNCTION(BlueprintCallable, Category="Organism")
    void Heal(float Amount);

    /**
     * Apply biological damage to the organism.
     * @param Amount Amount of damage to apply
     * @note Also negatively affects Mood based on damage severity
     */
    UFUNCTION(BlueprintCallable, Category="Organism")
    void TakeDamage(float Amount);

    /**
     * Trigger a mutation event, increasing MutationLevel.
     * @param Amount Amount to increase mutation level by
     * @note High mutation events may cause temporary mood instability
     */
    UFUNCTION(BlueprintCallable, Category="Organism")
    void Mutate(float Amount);

    /**
     * Get contextual dialogue from the living ship based on current state.
     * @return Dialogue string reflecting organism's health, mood, and mutation state
     *
     * Example outputs:
     * - Happy, healthy: "We are content. Ready to explore together."
     * - Damaged, neutral: "Pain... but healing. We endure."
     * - Highly mutated: "We see... more. The void whispers truths."
     */
    UFUNCTION(BlueprintCallable, Category="Organism")
    FString GetOrganismDialogue() const;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};