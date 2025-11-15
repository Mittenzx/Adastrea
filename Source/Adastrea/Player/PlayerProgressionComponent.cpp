#include "Player/PlayerProgressionComponent.h"
#include "AdastreaLog.h"

UPlayerProgressionComponent::UPlayerProgressionComponent()
	: PlayerLevel(1)
	, CurrentXP(0)
	, XPToNextLevel(1000)
	, TotalXPEarned(0)
	, AvailableSkillPoints(0)
	, BaseXPRequirement(1000)
	, XPScalingFactor(1.15f)
	, SkillPointsPerLevel(1)
	, MaxLevel(100)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerProgressionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize skills if not already set
	if (Skills.Num() == 0)
	{
		InitializeSkills();
	}

	// Calculate initial XP requirement
	XPToNextLevel = CalculateXPForLevel(PlayerLevel + 1);
}

void UPlayerProgressionComponent::InitializeSkills()
{
	// Initialize all skill categories
	Skills.Empty();
	Skills.Add(FPlayerSkill(ESkillCategory::Combat));
	Skills.Add(FPlayerSkill(ESkillCategory::Piloting));
	Skills.Add(FPlayerSkill(ESkillCategory::Engineering));
	Skills.Add(FPlayerSkill(ESkillCategory::Trading));
	Skills.Add(FPlayerSkill(ESkillCategory::Exploration));
	Skills.Add(FPlayerSkill(ESkillCategory::Diplomacy));
	Skills.Add(FPlayerSkill(ESkillCategory::Leadership));

	UE_LOG(LogAdastrea, Log, TEXT("PlayerProgressionComponent: Initialized %d skill categories"), Skills.Num());
}

void UPlayerProgressionComponent::AwardExperience(int32 XPAmount, bool bShowNotification)
{
	if (XPAmount <= 0)
	{
		return;
	}

	// Can't gain XP if at max level
	if (PlayerLevel >= MaxLevel)
	{
		UE_LOG(LogAdastrea, Log, TEXT("PlayerProgressionComponent: At max level (%d), XP not awarded"), MaxLevel);
		return;
	}

	// Award XP
	CurrentXP += XPAmount;
	TotalXPEarned += XPAmount;

	UE_LOG(LogAdastrea, Log, TEXT("PlayerProgressionComponent: Awarded %d XP. Current: %d/%d (Total: %d)"),
		XPAmount, CurrentXP, XPToNextLevel, TotalXPEarned);

	// Broadcast event
	OnExperienceAwarded.Broadcast(XPAmount, TotalXPEarned);

	// Check for level up
	while (CanLevelUp() && PlayerLevel < MaxLevel)
	{
		LevelUp();
	}
}

bool UPlayerProgressionComponent::CanLevelUp() const
{
	return CurrentXP >= XPToNextLevel && PlayerLevel < MaxLevel;
}

bool UPlayerProgressionComponent::LevelUp()
{
	if (!CanLevelUp())
	{
		return false;
	}

	// Subtract XP requirement
	CurrentXP -= XPToNextLevel;

	// Increase level
	PlayerLevel++;

	// Award skill points
	AvailableSkillPoints += SkillPointsPerLevel;

	// Calculate next level requirement
	XPToNextLevel = CalculateXPForLevel(PlayerLevel + 1);

	UE_LOG(LogAdastrea, Log, TEXT("PlayerProgressionComponent: Level up! New level: %d, Skill points: %d, Next level XP: %d"),
		PlayerLevel, AvailableSkillPoints, XPToNextLevel);

	// Broadcast event
	OnLevelUp.Broadcast(PlayerLevel, SkillPointsPerLevel);

	return true;
}

int32 UPlayerProgressionComponent::CalculateXPForLevel(int32 Level) const
{
	if (Level <= 1)
	{
		return 0;
	}

	// Exponential scaling: BaseXP * (ScalingFactor ^ (Level - 2))
	int32 RequiredXP = FMath::RoundToInt(BaseXPRequirement * FMath::Pow(XPScalingFactor, Level - 2));
	return RequiredXP;
}

float UPlayerProgressionComponent::GetLevelProgress() const
{
	if (XPToNextLevel <= 0 || PlayerLevel >= MaxLevel)
	{
		return 1.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentXP) / static_cast<float>(XPToNextLevel), 0.0f, 1.0f);
}

bool UPlayerProgressionComponent::InvestSkillPoints(ESkillCategory Category, int32 Points)
{
	if (Points <= 0 || AvailableSkillPoints < Points)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("PlayerProgressionComponent: Cannot invest %d points (Available: %d)"),
			Points, AvailableSkillPoints);
		return false;
	}

	FPlayerSkill* Skill = FindOrCreateSkill(Category);
	if (!Skill)
	{
		UE_LOG(LogAdastrea, Error, TEXT("PlayerProgressionComponent: Failed to find/create skill for category"));
		return false;
	}

	// Check if skill is maxed
	if (Skill->Rank >= 10)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("PlayerProgressionComponent: Skill already at max rank (10)"));
		return false;
	}

	// Invest points
	int32 PointsToInvest = FMath::Min(Points, 10 - Skill->Rank);
	Skill->Rank += PointsToInvest;
	AvailableSkillPoints -= PointsToInvest;

	UE_LOG(LogAdastrea, Log, TEXT("PlayerProgressionComponent: Invested %d points in %d. New rank: %d, Points remaining: %d"),
		PointsToInvest, static_cast<int32>(Category), Skill->Rank, AvailableSkillPoints);

	// Broadcast event
	OnSkillInvested.Broadcast(Category, Skill->Rank, AvailableSkillPoints);

	return true;
}

int32 UPlayerProgressionComponent::GetSkillRank(ESkillCategory Category) const
{
	const FPlayerSkill* Skill = FindSkill(Category);
	return Skill ? Skill->Rank : 0;
}

float UPlayerProgressionComponent::GetSkillBonus(ESkillCategory Category) const
{
	const FPlayerSkill* Skill = FindSkill(Category);
	if (!Skill)
	{
		return 1.0f;
	}

	return CalculateSkillBonus(Skill->Rank);
}

bool UPlayerProgressionComponent::CanInvestInSkill(ESkillCategory Category) const
{
	if (AvailableSkillPoints <= 0)
	{
		return false;
	}

	const FPlayerSkill* Skill = FindSkill(Category);
	if (!Skill)
	{
		return true; // Can invest if skill doesn't exist yet
	}

	return Skill->Rank < 10;
}

void UPlayerProgressionComponent::ResetSkills(bool bRefundPoints)
{
	if (bRefundPoints)
	{
		// Calculate total invested points
		int32 TotalInvested = GetTotalSkillPoints();
		AvailableSkillPoints += TotalInvested;
		
		UE_LOG(LogAdastrea, Log, TEXT("PlayerProgressionComponent: Skills reset, refunded %d points"), TotalInvested);
	}

	// Reset all skills
	for (FPlayerSkill& Skill : Skills)
	{
		Skill.Rank = 0;
		Skill.CurrentXP = 0;
	}
}

bool UPlayerProgressionComponent::MeetsLevelRequirement(int32 RequiredLevel) const
{
	return PlayerLevel >= RequiredLevel;
}

int32 UPlayerProgressionComponent::GetTotalSkillPoints() const
{
	int32 Total = 0;
	for (const FPlayerSkill& Skill : Skills)
	{
		Total += Skill.Rank;
	}
	return Total;
}

FPlayerSkill* UPlayerProgressionComponent::FindOrCreateSkill(ESkillCategory Category)
{
	// Try to find existing skill
	for (FPlayerSkill& Skill : Skills)
	{
		if (Skill.Category == Category)
		{
			return &Skill;
		}
	}

	// Create new skill if not found
	FPlayerSkill NewSkill(Category);
	Skills.Add(NewSkill);
	return &Skills.Last();
}

const FPlayerSkill* UPlayerProgressionComponent::FindSkill(ESkillCategory Category) const
{
	for (const FPlayerSkill& Skill : Skills)
	{
		if (Skill.Category == Category)
		{
			return &Skill;
		}
	}
	return nullptr;
}

float UPlayerProgressionComponent::CalculateSkillBonus(int32 Rank) const
{
	// Each rank provides 5% bonus
	// Rank 0 = 1.0 (no bonus)
	// Rank 1 = 1.05 (5% bonus)
	// Rank 10 = 1.5 (50% bonus)
	return 1.0f + (Rank * 0.05f);
}
