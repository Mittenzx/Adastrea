// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/MarketplaceModule.h"

AMarketplaceModule::AMarketplaceModule()
{
    ModuleType = TEXT("Marketplace");
    ModulePower = 40.0f;
    ModuleGroup = EStationModuleGroup::Public;
    
    // Initialize marketplace properties
    MarketDataAsset = nullptr;
    bIsOpen = true;
    MarketplaceName = FText::FromString(TEXT("Marketplace"));
}
