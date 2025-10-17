#include "MaterialDataAsset.h"

UMaterialDataAsset::UMaterialDataAsset()
    : MaterialName(FText::FromString(TEXT("Unknown Material")))
    , Description(FText::FromString(TEXT("No description available.")))
    , Category(EMaterialCategory::Other)
    , StorageType(EStorageType::Solid)
    , Rarity(EMaterialRarity::Common)
    , Value(1.0f)
    , Mass(1.0f)
{
}

bool UMaterialDataAsset::HasTag(FName Tag) const
{
    return Tags.Contains(Tag);
}

bool UMaterialDataAsset::CanBeProcessedByModule(FName ModuleTag) const
{
    return ModuleTags.Contains(ModuleTag);
}

TArray<FName> UMaterialDataAsset::GetTags() const
{
    return Tags;
}

TArray<FName> UMaterialDataAsset::GetModuleTags() const
{
    return ModuleTags;
}

float UMaterialDataAsset::GetStorageVolume() const
{
    // Calculate storage volume based on mass and material density assumptions
    // Base volume calculation: 1kg = 0.001 cubic meters for standard materials
    float BaseVolume = Mass * 0.001f;
    
    // Apply multipliers based on storage type
    switch (StorageType)
    {
        case EStorageType::Gas:
            // Gases require much more volume when stored
            return BaseVolume * 1000.0f;
        case EStorageType::Liquid:
            // Liquids are relatively dense
            return BaseVolume * 1.2f;
        case EStorageType::Refrigerated:
            // Refrigerated items need extra space for cooling systems
            return BaseVolume * 1.5f;
        case EStorageType::Hazardous:
            // Hazardous materials need containment space
            return BaseVolume * 2.0f;
        case EStorageType::Solid:
        case EStorageType::Other:
        default:
            return BaseVolume;
    }
}

bool UMaterialDataAsset::IsCategory(EMaterialCategory CheckCategory) const
{
    return Category == CheckCategory;
}

bool UMaterialDataAsset::RequiresSpecialStorage() const
{
    return StorageType == EStorageType::Refrigerated 
        || StorageType == EStorageType::Hazardous 
        || StorageType == EStorageType::Gas;
}
