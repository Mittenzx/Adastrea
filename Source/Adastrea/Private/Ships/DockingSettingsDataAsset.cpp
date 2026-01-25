#include "Ships/DockingSettingsDataAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UDockingSettingsDataAsset::UDockingSettingsDataAsset()
{
    // Set sensible defaults for docking configuration
    DockingRange = 2000.0f;           // 20 meters
    DockingDuration = 1.0f;           // 1 second animation
    UndockImpulseStrength = 500.0f;   // 5 m/s forward velocity
    DockingCurve = nullptr;           // Optional, can be set by designer
}

#if WITH_EDITOR
EDataValidationResult UDockingSettingsDataAsset::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

    // Validate that widget classes are set
    if (!DockingPromptWidgetClass)
    {
        Context.AddWarning(FText::FromString(
            "DockingPromptWidgetClass is not set. Ships using this configuration won't show docking prompts."));
        if (Result != EDataValidationResult::Invalid)
        {
            Result = EDataValidationResult::NotValidated;
        }
    }

    if (!TradingInterfaceClass)
    {
        Context.AddWarning(FText::FromString(
            "TradingInterfaceClass is not set. Ships using this configuration won't show trading UI when docked."));
        if (Result != EDataValidationResult::Invalid)
        {
            Result = EDataValidationResult::NotValidated;
        }
    }

    // Validate docking range is reasonable
    if (DockingRange < 100.0f || DockingRange > 10000.0f)
    {
        Context.AddError(FText::FromString(
            FString::Printf(TEXT("DockingRange %.1f is outside valid range (100-10000 cm). This may cause docking issues."), DockingRange)));
        Result = EDataValidationResult::Invalid;
    }

    // Validate docking duration is reasonable
    if (DockingDuration < 0.1f || DockingDuration > 10.0f)
    {
        Context.AddWarning(FText::FromString(
            FString::Printf(TEXT("DockingDuration %.2f seconds is outside recommended range (0.1-10.0 seconds). Note: This parameter is not yet implemented in gameplay."), DockingDuration)));
        if (Result != EDataValidationResult::Invalid)
        {
            Result = EDataValidationResult::NotValidated;
        }
    }

    // Validate undock impulse is reasonable
    if (UndockImpulseStrength < 0.0f)
    {
        Context.AddError(FText::FromString(
            TEXT("UndockImpulseStrength cannot be negative.")));
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}
#endif
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}
#endif
