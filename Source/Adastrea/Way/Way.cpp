#include "Way/Way.h"
#include "AdastreaLog.h"

UWayDataAsset::UWayDataAsset()
{
    // Set default values for core identity
    WayName = FText::FromString(TEXT("Unnamed Way"));
    Description = FText::FromString(TEXT("A group with shared values and philosophy."));
    WayID = FName(TEXT("UnknownWay"));
    bIsSchool = true;
    
    // Default UI colors
    PrimaryColor = FLinearColor(0.2f, 0.5f, 0.8f, 1.0f);
    SecondaryColor = FLinearColor(0.1f, 0.3f, 0.5f, 1.0f);
    
    // Default organizational attributes
    TechnologyLevel = 5;
    MilitaryStrength = 5;
    EconomicPower = 5;
    InfluencePower = 5;
    HomeBaseID = NAME_None;
    
    // Default specialization
    PrimaryIndustry = EWayIndustry::Trading;
    QualityReputation = EQualityTier::Standard;
    SpecializationDescription = FText::FromString(TEXT("This Way specializes in various trades and services."));
    MemberCount = 100;
    
    // Default governance settings
    bParticipatesInCouncils = true;
    BaseVotingWeight = 10;
}

// ====================
// Precept Query Functions
// ====================

TArray<FPreceptValue> UWayDataAsset::GetPrecepts() const
{
    return CorePrecepts;
}

bool UWayDataAsset::HasPrecept(EPrecept Precept) const
{
    for (const FPreceptValue& PreceptValue : CorePrecepts)
    {
        if (PreceptValue.Precept == Precept)
        {
            return true;
        }
    }
    return false;
}

int32 UWayDataAsset::GetPreceptValue(EPrecept Precept) const
{
    for (const FPreceptValue& PreceptValue : CorePrecepts)
    {
        if (PreceptValue.Precept == Precept)
        {
            return PreceptValue.ImportanceValue;
        }
    }
    return 0;
}

bool UWayDataAsset::GetPrimaryPrecept(EPrecept& OutPrecept) const
{
    if (CorePrecepts.Num() == 0)
    {
        return false;
    }
    
    int32 HighestValue = 0;
    EPrecept PrimaryPrecept = EPrecept::Honor;
    
    for (const FPreceptValue& PreceptValue : CorePrecepts)
    {
        if (PreceptValue.ImportanceValue > HighestValue)
        {
            HighestValue = PreceptValue.ImportanceValue;
            PrimaryPrecept = PreceptValue.Precept;
        }
    }
    
    OutPrecept = PrimaryPrecept;
    return true;
}

TArray<FPreceptValue> UWayDataAsset::GetPreceptsAboveThreshold(int32 Threshold) const
{
    TArray<FPreceptValue> Result;
    
    for (const FPreceptValue& PreceptValue : CorePrecepts)
    {
        if (PreceptValue.ImportanceValue >= Threshold)
        {
            Result.Add(PreceptValue);
        }
    }
    
    return Result;
}

// ====================
// Helper Functions
// ====================

FText UWayDataAsset::GetPreceptDisplayName(EPrecept Precept)
{
    // Return the enum display name defined in the UMETA tag
    const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/Adastrea.EPrecept"), true);
    if (EnumPtr)
    {
        return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Precept));
    }
    return FText::FromString(TEXT("Unknown"));
}

FText UWayDataAsset::GetPreceptDescription(EPrecept Precept)
{
    // Provide descriptions for each Precept
    switch (Precept)
    {
        case EPrecept::Honor:
            return FText::FromString(TEXT("Upholding principles and keeping one's word"));
        case EPrecept::Justice:
            return FText::FromString(TEXT("Fairness and righteousness in actions"));
        case EPrecept::Compassion:
            return FText::FromString(TEXT("Mercy and kindness toward others"));
        case EPrecept::Loyalty:
            return FText::FromString(TEXT("Dedication and steadfastness to causes"));
        case EPrecept::Mastery:
            return FText::FromString(TEXT("Pursuit of skill perfection"));
        case EPrecept::Innovation:
            return FText::FromString(TEXT("Creating new solutions and ideas"));
        case EPrecept::Discovery:
            return FText::FromString(TEXT("Uncovering hidden knowledge and truths"));
        case EPrecept::Craftsmanship:
            return FText::FromString(TEXT("Excellence in creation and building"));
        case EPrecept::Strength:
            return FText::FromString(TEXT("Physical and military might"));
        case EPrecept::Dominance:
            return FText::FromString(TEXT("Control and supremacy over others"));
        case EPrecept::Cunning:
            return FText::FromString(TEXT("Strategic thinking and cleverness"));
        case EPrecept::Ambition:
            return FText::FromString(TEXT("Drive to rise and succeed"));
        case EPrecept::Unity:
            return FText::FromString(TEXT("Togetherness and cooperation"));
        case EPrecept::Freedom:
            return FText::FromString(TEXT("Liberty and independence"));
        case EPrecept::Tradition:
            return FText::FromString(TEXT("Preserving heritage and customs"));
        case EPrecept::Progress:
            return FText::FromString(TEXT("Advancement and positive change"));
        case EPrecept::Prosperity:
            return FText::FromString(TEXT("Wealth and material abundance"));
        case EPrecept::Survival:
            return FText::FromString(TEXT("Endurance and resilience"));
        case EPrecept::Efficiency:
            return FText::FromString(TEXT("Optimal use of resources"));
        case EPrecept::Harmony:
            return FText::FromString(TEXT("Balance and peaceful coexistence"));
        default:
            return FText::FromString(TEXT("A core value of this group"));
    }
}

// ====================
// Relationship Query Functions
// ====================

TArray<FWayRelationship> UWayDataAsset::GetRelationships() const
{
    return WayRelationships;
}

bool UWayDataAsset::GetRelationship(FName OtherWayID, FWayRelationship& OutRelationship) const
{
    for (const FWayRelationship& Relationship : WayRelationships)
    {
        if (Relationship.TargetWayID == OtherWayID)
        {
            OutRelationship = Relationship;
            return true;
        }
    }
    return false;
}

bool UWayDataAsset::HasPartnership(FName OtherWayID) const
{
    FWayRelationship Relationship;
    if (GetRelationship(OtherWayID, Relationship))
    {
        return Relationship.bHasPartnership;
    }
    return false;
}

bool UWayDataAsset::IsCompeting(FName OtherWayID) const
{
    FWayRelationship Relationship;
    if (GetRelationship(OtherWayID, Relationship))
    {
        return Relationship.bInCompetition;
    }
    return false;
}

int32 UWayDataAsset::GetRelationshipValue(FName OtherWayID) const
{
    FWayRelationship Relationship;
    if (GetRelationship(OtherWayID, Relationship))
    {
        return Relationship.RelationshipValue;
    }
    return 0;
}

float UWayDataAsset::GetTradeModifier(FName OtherWayID) const
{
    FWayRelationship Relationship;
    if (GetRelationship(OtherWayID, Relationship))
    {
        return Relationship.TradeModifier;
    }
    return 1.0f;
}

// ====================
// Governance Functions
// ====================

bool UWayDataAsset::IsRepresentedInSector(FName SectorID) const
{
    return RepresentedSectors.Contains(SectorID) || ControlledSectors.Contains(SectorID);
}

TArray<FName> UWayDataAsset::GetRepresentedSectors() const
{
    return RepresentedSectors;
}

int32 UWayDataAsset::GetVotingWeight() const
{
    return BaseVotingWeight;
}

// ====================
// Trade Specialization Functions
// ====================

EWayIndustry UWayDataAsset::GetPrimaryIndustry() const
{
    return PrimaryIndustry;
}

TArray<EWayIndustry> UWayDataAsset::GetAllIndustries() const
{
    TArray<EWayIndustry> AllIndustries;
    AllIndustries.Add(PrimaryIndustry);
    AllIndustries.Append(SecondaryIndustries);
    return AllIndustries;
}

bool UWayDataAsset::OperatesInIndustry(EWayIndustry Industry) const
{
    if (PrimaryIndustry == Industry)
    {
        return true;
    }
    return SecondaryIndustries.Contains(Industry);
}

EQualityTier UWayDataAsset::GetQualityReputation() const
{
    return QualityReputation;
}

TArray<FWayResource> UWayDataAsset::GetProducedResources() const
{
    return ProducedResources;
}

TArray<FWayResource> UWayDataAsset::GetConsumedResources() const
{
    return ConsumedResources;
}

bool UWayDataAsset::ProducesResource(FName ResourceID) const
{
    for (const FWayResource& Resource : ProducedResources)
    {
        if (Resource.ResourceID == ResourceID)
        {
            return true;
        }
    }
    return false;
}

bool UWayDataAsset::ConsumesResource(FName ResourceID) const
{
    for (const FWayResource& Resource : ConsumedResources)
    {
        if (Resource.ResourceID == ResourceID)
        {
            return true;
        }
    }
    return false;
}

int32 UWayDataAsset::GetProductionQuantity(FName ResourceID) const
{
    for (const FWayResource& Resource : ProducedResources)
    {
        if (Resource.ResourceID == ResourceID)
        {
            return Resource.Quantity;
        }
    }
    return 0;
}

int32 UWayDataAsset::GetConsumptionQuantity(FName ResourceID) const
{
    for (const FWayResource& Resource : ConsumedResources)
    {
        if (Resource.ResourceID == ResourceID)
        {
            return Resource.Quantity;
        }
    }
    return 0;
}

TArray<FSupplyChainLink> UWayDataAsset::GetSupplyChains() const
{
    return SupplyChains;
}

TArray<UWayDataAsset*> UWayDataAsset::GetSuppliers() const
{
    TArray<UWayDataAsset*> Suppliers;
    for (const FSupplyChainLink& Link : SupplyChains)
    {
        if (Link.ConsumerWay == this && Link.SupplierWay)
        {
            Suppliers.AddUnique(Link.SupplierWay);
        }
    }
    return Suppliers;
}

TArray<UWayDataAsset*> UWayDataAsset::GetCustomers() const
{
    TArray<UWayDataAsset*> Customers;
    for (const FSupplyChainLink& Link : SupplyChains)
    {
        if (Link.SupplierWay == this && Link.ConsumerWay)
        {
            Customers.AddUnique(Link.ConsumerWay);
        }
    }
    return Customers;
}

bool UWayDataAsset::HasSupplyRelationship(const UWayDataAsset* OtherWay) const
{
    if (!OtherWay)
    {
        return false;
    }
    
    for (const FSupplyChainLink& Link : SupplyChains)
    {
        if ((Link.SupplierWay == this && Link.ConsumerWay == OtherWay) ||
            (Link.ConsumerWay == this && Link.SupplierWay == OtherWay))
        {
            return true;
        }
    }
    return false;
}

int32 UWayDataAsset::GetMemberCount() const
{
    return MemberCount;
}

// ====================
// Utility Functions
// ====================

FText UWayDataAsset::GetIndustryDisplayName(EWayIndustry Industry)
{
    const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/Adastrea.EWayIndustry"), true);
    if (EnumPtr)
    {
        return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Industry));
    }
    return FText::FromString(TEXT("Unknown Industry"));
}

FText UWayDataAsset::GetQualityDisplayName(EQualityTier Quality)
{
    const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/Adastrea.EQualityTier"), true);
    if (EnumPtr)
    {
        return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Quality));
    }
    return FText::FromString(TEXT("Unknown Quality"));
}
