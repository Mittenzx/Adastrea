// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Combat/BoardingComponent.h"
#include "AdastreaLog.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"

UBoardingComponent::UBoardingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Default configuration
    MaxBoardingCrew = 20;
    BaseBoardingSkill = 50.0f;
    DefenseRating = 50.0f;
    BreachTime = 30.0f;
    CombatInterval = 5.0f;
    MinimumCrewForControl = 5;
    bCanBoard = true;
    bCanBeBoarded = true;

    // Default state
    bIsBoardingActive = false;
    bIsCaptured = false;
    CapturingController = nullptr;
    TimeSinceCombatTick = 0.0f;
}

void UBoardingComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBoardingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsBoardingActive)
    {
        UpdateBoardingAction(DeltaTime);
    }
}

bool UBoardingComponent::InitiateBoardingAction(AActor* TargetShip, int32 BoardingCrewCount)
{
    if (!TargetShip || !bCanBoard)
    {
        return false;
    }

    // Check if already in boarding action
    if (bIsBoardingActive)
    {
        UE_LOG(LogAdastreaCombat, Warning, TEXT("Already engaged in boarding action"));
        return false;
    }

    // Validate crew count
    if (BoardingCrewCount <= 0 || BoardingCrewCount > MaxBoardingCrew)
    {
        UE_LOG(LogAdastreaCombat, Warning, TEXT("Invalid boarding crew count: %d"), BoardingCrewCount);
        return false;
    }

    // Check if target can be boarded
    UBoardingComponent* TargetBoarding = GetTargetBoardingComponent();
    if (!TargetBoarding || !TargetBoarding->bCanBeBoarded)
    {
        UE_LOG(LogAdastreaCombat, Warning, TEXT("Target cannot be boarded"));
        return false;
    }

    // Check if target is already captured
    if (TargetBoarding->bIsCaptured)
    {
        UE_LOG(LogAdastreaCombat, Warning, TEXT("Target is already captured"));
        return false;
    }

    // Initialize boarding action
    ActiveBoardingAction = FBoardingAction();
    ActiveBoardingAction.TargetShip = TargetShip;
    ActiveBoardingAction.State = EBoardingState::Approaching;
    ActiveBoardingAction.bIsAttacker = true;
    ActiveBoardingAction.Progress = 0.0f;
    ActiveBoardingAction.TimeInState = 0.0f;

    // Setup attacking party
    ActiveBoardingAction.AttackingParty.CrewCount = BoardingCrewCount;
    ActiveBoardingAction.AttackingParty.SkillLevel = BaseBoardingSkill;
    ActiveBoardingAction.AttackingParty.EquipmentQuality = 75.0f;  // Attackers typically better equipped
    ActiveBoardingAction.AttackingParty.Morale = 100.0f;
    ActiveBoardingAction.AttackingParty.Casualties = 0;

    // Setup defending party from target
    ActiveBoardingAction.DefendingParty.CrewCount = FMath::Max(TargetBoarding->MinimumCrewForControl, 10);
    ActiveBoardingAction.DefendingParty.SkillLevel = TargetBoarding->DefenseRating;
    ActiveBoardingAction.DefendingParty.EquipmentQuality = 50.0f;
    ActiveBoardingAction.DefendingParty.Morale = 100.0f;
    ActiveBoardingAction.DefendingParty.Casualties = 0;

    bIsBoardingActive = true;
    TimeSinceCombatTick = 0.0f;

    // Notify target they are being boarded
    if (TargetBoarding)
    {
        TargetBoarding->ActiveBoardingAction = ActiveBoardingAction;
        TargetBoarding->ActiveBoardingAction.bIsAttacker = false;
        TargetBoarding->bIsBoardingActive = true;
        TargetBoarding->OnBoardingStarted(GetOwner(), false);
    }

    OnBoardingStarted(TargetShip, true);

    UE_LOG(LogAdastreaCombat, Log, TEXT("Boarding action initiated with %d crew"), BoardingCrewCount);

    return true;
}

void UBoardingComponent::AbortBoardingAction()
{
    if (!bIsBoardingActive)
    {
        return;
    }

    FailBoardingAction(TEXT("Aborted"));
}

void UBoardingComponent::SurrenderShip(AController* Attacker)
{
    if (!Attacker)
    {
        return;
    }

    bIsCaptured = true;
    CapturingController = Attacker;

    // End any active boarding
    if (bIsBoardingActive)
    {
        CompleteBoardingSuccess();
    }

    OnShipCaptured(Attacker);

    UE_LOG(LogAdastreaCombat, Warning, TEXT("Ship surrendered to %s"), *Attacker->GetName());
}

bool UBoardingComponent::RecaptureShip(AController* RescuingController)
{
    if (!bIsCaptured || !RescuingController)
    {
        return false;
    }

    bIsCaptured = false;
    CapturingController = nullptr;

    OnShipRecaptured(RescuingController);

    UE_LOG(LogAdastreaCombat, Log, TEXT("Ship recaptured by %s"), *RescuingController->GetName());

    return true;
}

float UBoardingComponent::GetBoardingProgress() const
{
    return ActiveBoardingAction.Progress;
}

EBoardingState UBoardingComponent::GetBoardingState() const
{
    return ActiveBoardingAction.State;
}

bool UBoardingComponent::CanInitiateBoardingOn(AActor* TargetShip) const
{
    if (!TargetShip || !bCanBoard || bIsBoardingActive)
    {
        return false;
    }

    UBoardingComponent* TargetBoarding = Cast<UBoardingComponent>(
        TargetShip->GetComponentByClass(UBoardingComponent::StaticClass())
    );

    if (!TargetBoarding || !TargetBoarding->bCanBeBoarded || TargetBoarding->bIsCaptured)
    {
        return false;
    }

    return true;
}

float UBoardingComponent::EstimateBoardingSuccessChance(AActor* TargetShip, int32 BoardingCrewCount) const
{
    if (!TargetShip)
    {
        return 0.0f;
    }

    UBoardingComponent* TargetBoarding = Cast<UBoardingComponent>(
        TargetShip->GetComponentByClass(UBoardingComponent::StaticClass())
    );

    if (!TargetBoarding)
    {
        return 0.5f;
    }

    // Calculate strength ratio
    const float AttackerStrength = BoardingCrewCount * BaseBoardingSkill;
    const float DefenderStrength = TargetBoarding->MinimumCrewForControl * TargetBoarding->DefenseRating;

    if (DefenderStrength <= 0.0f)
    {
        return 1.0f;
    }

    const float StrengthRatio = AttackerStrength / DefenderStrength;
    return FMath::Clamp(StrengthRatio / 2.0f, 0.0f, 0.95f);
}

float UBoardingComponent::GetAttackerCombatStrength() const
{
    if (!bIsBoardingActive)
    {
        return 0.0f;
    }

    const FBoardingParty& Party = ActiveBoardingAction.bIsAttacker ? 
        ActiveBoardingAction.AttackingParty : ActiveBoardingAction.DefendingParty;

    const float CrewFactor = Party.CrewCount;
    const float SkillFactor = Party.SkillLevel / 100.0f;
    const float EquipmentFactor = Party.EquipmentQuality / 100.0f;
    const float MoraleFactor = Party.Morale / 100.0f;

    return CrewFactor * SkillFactor * EquipmentFactor * MoraleFactor;
}

float UBoardingComponent::GetDefenderCombatStrength() const
{
    if (!bIsBoardingActive)
    {
        return 0.0f;
    }

    const FBoardingParty& Party = ActiveBoardingAction.bIsAttacker ? 
        ActiveBoardingAction.DefendingParty : ActiveBoardingAction.AttackingParty;

    const float CrewFactor = Party.CrewCount;
    const float SkillFactor = Party.SkillLevel / 100.0f;
    const float EquipmentFactor = Party.EquipmentQuality / 100.0f;
    const float MoraleFactor = Party.Morale / 100.0f;

    // Defenders get home advantage bonus
    const float DefenseBonus = 1.2f;

    return CrewFactor * SkillFactor * EquipmentFactor * MoraleFactor * DefenseBonus;
}

void UBoardingComponent::OnBoardingStarted_Implementation(AActor* TargetShip, bool bAsAttacker)
{
    UE_LOG(LogAdastreaCombat, Log, TEXT("Boarding started as %s"), bAsAttacker ? TEXT("attacker") : TEXT("defender"));
}

void UBoardingComponent::OnBreachComplete_Implementation()
{
    UE_LOG(LogAdastreaCombat, Log, TEXT("Hull breach complete, entering combat"));
}

void UBoardingComponent::OnCombatResolved_Implementation(int32 AttackerCasualties, int32 DefenderCasualties)
{
    UE_LOG(LogAdastreaCombat, Log, TEXT("Combat resolved - Attacker casualties: %d, Defender casualties: %d"), 
        AttackerCasualties, DefenderCasualties);
}

void UBoardingComponent::OnBoardingSuccessful_Implementation(AActor* CapturedShip, AController* NewOwner)
{
    UE_LOG(LogAdastreaCombat, Warning, TEXT("Boarding successful! Ship captured by %s"), 
        NewOwner ? *NewOwner->GetName() : TEXT("Unknown"));
}

void UBoardingComponent::OnBoardingFailed_Implementation(const FString& Reason)
{
    UE_LOG(LogAdastreaCombat, Warning, TEXT("Boarding failed: %s"), *Reason);
}

void UBoardingComponent::OnShipCaptured_Implementation(AController* Captor)
{
    UE_LOG(LogAdastreaCombat, Error, TEXT("Ship captured by %s"), Captor ? *Captor->GetName() : TEXT("Unknown"));
}

void UBoardingComponent::OnShipRecaptured_Implementation(AController* Rescuer)
{
    UE_LOG(LogAdastreaCombat, Log, TEXT("Ship recaptured by %s"), Rescuer ? *Rescuer->GetName() : TEXT("Unknown"));
}

void UBoardingComponent::UpdateBoardingAction(float DeltaTime)
{
    if (!bIsBoardingActive)
    {
        return;
    }

    ActiveBoardingAction.TimeInState += DeltaTime;

    switch (ActiveBoardingAction.State)
    {
    case EBoardingState::Approaching:
        // Simulate approach time (could be extended for actual movement)
        if (ActiveBoardingAction.TimeInState >= 10.0f)
        {
            ActiveBoardingAction.State = EBoardingState::Breaching;
            ActiveBoardingAction.TimeInState = 0.0f;
        }
        break;

    case EBoardingState::Breaching:
        ProcessBreachPhase(DeltaTime);
        break;

    case EBoardingState::Combat:
        ProcessCombatPhase(DeltaTime);
        break;

    case EBoardingState::Securing:
        // Securing phase after combat victory
        if (ActiveBoardingAction.TimeInState >= 20.0f)
        {
            CompleteBoardingSuccess();
        }
        break;

    default:
        break;
    }

    // Update progress based on state
    switch (ActiveBoardingAction.State)
    {
    case EBoardingState::Approaching:
        ActiveBoardingAction.Progress = 10.0f;
        break;
    case EBoardingState::Breaching:
        ActiveBoardingAction.Progress = 10.0f + (ActiveBoardingAction.TimeInState / BreachTime) * 20.0f;
        break;
    case EBoardingState::Combat:
        {
            const float CombatProgress = 1.0f - (ActiveBoardingAction.DefendingParty.CrewCount / 
                static_cast<float>(FMath::Max(1, 10)));
            ActiveBoardingAction.Progress = 30.0f + CombatProgress * 60.0f;
        }
        break;
    case EBoardingState::Securing:
        ActiveBoardingAction.Progress = 90.0f + (ActiveBoardingAction.TimeInState / 20.0f) * 10.0f;
        break;
    case EBoardingState::Complete:
        ActiveBoardingAction.Progress = 100.0f;
        break;
    default:
        break;
    }
}

void UBoardingComponent::ProcessBreachPhase(float DeltaTime)
{
    if (ActiveBoardingAction.TimeInState >= BreachTime)
    {
        ActiveBoardingAction.State = EBoardingState::Combat;
        ActiveBoardingAction.TimeInState = 0.0f;
        OnBreachComplete();
    }
}

void UBoardingComponent::ProcessCombatPhase(float DeltaTime)
{
    TimeSinceCombatTick += DeltaTime;

    if (TimeSinceCombatTick >= CombatInterval)
    {
        ResolveCombat();
        TimeSinceCombatTick = 0.0f;
        CheckBoardingCompletion();
    }
}

void UBoardingComponent::ResolveCombat()
{
    const float AttackerStrength = GetAttackerCombatStrength();
    const float DefenderStrength = GetDefenderCombatStrength();

    int32 AttackerCasualties = 0;
    int32 DefenderCasualties = 0;

    CalculateCasualties(AttackerStrength, DefenderStrength, AttackerCasualties, DefenderCasualties);

    // Apply casualties
    ActiveBoardingAction.AttackingParty.CrewCount = FMath::Max(0, 
        ActiveBoardingAction.AttackingParty.CrewCount - AttackerCasualties);
    ActiveBoardingAction.AttackingParty.Casualties += AttackerCasualties;

    ActiveBoardingAction.DefendingParty.CrewCount = FMath::Max(0, 
        ActiveBoardingAction.DefendingParty.CrewCount - DefenderCasualties);
    ActiveBoardingAction.DefendingParty.Casualties += DefenderCasualties;

    // Update morale based on casualties
    if (AttackerCasualties > 0)
    {
        ActiveBoardingAction.AttackingParty.Morale = FMath::Max(0.0f, 
            ActiveBoardingAction.AttackingParty.Morale - (AttackerCasualties * 5.0f));
    }

    if (DefenderCasualties > 0)
    {
        ActiveBoardingAction.DefendingParty.Morale = FMath::Max(0.0f, 
            ActiveBoardingAction.DefendingParty.Morale - (DefenderCasualties * 5.0f));
    }

    OnCombatResolved(AttackerCasualties, DefenderCasualties);

    // Sync with target if we're the attacker
    if (ActiveBoardingAction.bIsAttacker)
    {
        UBoardingComponent* TargetBoarding = GetTargetBoardingComponent();
        if (TargetBoarding)
        {
            TargetBoarding->ActiveBoardingAction.AttackingParty = ActiveBoardingAction.AttackingParty;
            TargetBoarding->ActiveBoardingAction.DefendingParty = ActiveBoardingAction.DefendingParty;
        }
    }
}

void UBoardingComponent::CalculateCasualties(float AttackerStrength, float DefenderStrength, 
    int32& OutAttackerCasualties, int32& OutDefenderCasualties)
{
    // Base casualty rate
    const float BaseCasualtyRate = 0.1f;

    // Attacker casualties based on defender strength
    const float AttackerCasualtyRate = BaseCasualtyRate * (DefenderStrength / FMath::Max(AttackerStrength, 1.0f));
    OutAttackerCasualties = FMath::RandRange(0, FMath::Max(1, 
        FMath::CeilToInt(ActiveBoardingAction.AttackingParty.CrewCount * AttackerCasualtyRate)));

    // Defender casualties based on attacker strength
    const float DefenderCasualtyRate = BaseCasualtyRate * (AttackerStrength / FMath::Max(DefenderStrength, 1.0f));
    OutDefenderCasualties = FMath::RandRange(0, FMath::Max(1, 
        FMath::CeilToInt(ActiveBoardingAction.DefendingParty.CrewCount * DefenderCasualtyRate)));
}

void UBoardingComponent::CheckBoardingCompletion()
{
    // Check if attackers eliminated
    if (ActiveBoardingAction.AttackingParty.CrewCount <= 0 || 
        ActiveBoardingAction.AttackingParty.Morale <= 10.0f)
    {
        FailBoardingAction(TEXT("Boarding party eliminated or routed"));
        return;
    }

    // Check if defenders eliminated
    if (ActiveBoardingAction.DefendingParty.CrewCount <= 0 || 
        ActiveBoardingAction.DefendingParty.Morale <= 10.0f)
    {
        // Attackers won, move to securing phase
        ActiveBoardingAction.State = EBoardingState::Securing;
        ActiveBoardingAction.TimeInState = 0.0f;
        return;
    }
}

void UBoardingComponent::CompleteBoardingSuccess()
{
    if (!bIsBoardingActive)
    {
        return;
    }

    ActiveBoardingAction.State = EBoardingState::Complete;
    ActiveBoardingAction.Progress = 100.0f;

    // Determine new owner
    AController* NewOwner = nullptr;
    AActor* CapturedShip = nullptr;

    if (ActiveBoardingAction.bIsAttacker)
    {
        // We are the attacker and won
        CapturedShip = ActiveBoardingAction.TargetShip;
        NewOwner = GetOwner() ? GetOwner()->GetInstigatorController() : nullptr;

        // Capture target ship
        UBoardingComponent* TargetBoarding = GetTargetBoardingComponent();
        if (TargetBoarding)
        {
            TargetBoarding->SurrenderShip(NewOwner);
        }
    }
    else
    {
        // We are the defender and lost
        CapturedShip = GetOwner();
        UBoardingComponent* AttackerBoarding = Cast<UBoardingComponent>(
            ActiveBoardingAction.TargetShip->GetComponentByClass(UBoardingComponent::StaticClass())
        );
        if (AttackerBoarding)
        {
            NewOwner = AttackerBoarding->GetOwner() ? 
                AttackerBoarding->GetOwner()->GetInstigatorController() : nullptr;
        }
    }

    OnBoardingSuccessful(CapturedShip, NewOwner);

    bIsBoardingActive = false;
}

void UBoardingComponent::FailBoardingAction(const FString& Reason)
{
    if (!bIsBoardingActive)
    {
        return;
    }

    ActiveBoardingAction.State = EBoardingState::Failed;
    
    OnBoardingFailed(Reason);

    // Notify target
    if (ActiveBoardingAction.bIsAttacker)
    {
        UBoardingComponent* TargetBoarding = GetTargetBoardingComponent();
        if (TargetBoarding)
        {
            TargetBoarding->bIsBoardingActive = false;
        }
    }

    bIsBoardingActive = false;
}

UBoardingComponent* UBoardingComponent::GetTargetBoardingComponent() const
{
    if (!ActiveBoardingAction.TargetShip)
    {
        return nullptr;
    }

    return Cast<UBoardingComponent>(
        ActiveBoardingAction.TargetShip->GetComponentByClass(UBoardingComponent::StaticClass())
    );
}
