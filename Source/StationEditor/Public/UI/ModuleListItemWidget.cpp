// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "UI/ModuleListItemWidget.h"
#include "Stations/SpaceStationModule.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Audio/AudioEventLibrary.h"

// StationEditor module includes
#include "StationModuleCatalog.h"

UModuleListItemWidget::UModuleListItemWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ModuleNameText(nullptr)
	, DescriptionText(nullptr)
	, CostText(nullptr)
	, BuildButton(nullptr)
	, ModuleClass(nullptr)
{
}

void UModuleListItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind build button
	if (BuildButton)
	{
		BuildButton->OnClicked.AddUniqueDynamic(this, &UModuleListItemWidget::OnBuildButtonClicked);
		BuildButton->OnHovered.AddUniqueDynamic(this, &UModuleListItemWidget::OnBuildButtonHovered);
	}
}

void UModuleListItemWidget::NativeDestruct()
{
	// Unbind build button
	if (BuildButton)
	{
		BuildButton->OnClicked.RemoveDynamic(this, &UModuleListItemWidget::OnBuildButtonClicked);
		BuildButton->OnHovered.RemoveDynamic(this, &UModuleListItemWidget::OnBuildButtonHovered);
	}

	Super::NativeDestruct();
}

void UModuleListItemWidget::SetModuleData(const FStationModuleEntry& Entry)
{
	ModuleClass = Entry.ModuleClass;

	// Set module name
	if (ModuleNameText)
	{
		ModuleNameText->SetText(Entry.DisplayName);
	}

	// Set description
	if (DescriptionText)
	{
		DescriptionText->SetText(Entry.Description);
	}

	// Set cost
	if (CostText)
	{
		FText CostDisplayText = FText::FromString(FString::Printf(TEXT("%d Credits | %.0fs"),
			Entry.BuildCost.Credits,
			Entry.BuildCost.BuildTime));
		CostText->SetText(CostDisplayText);
	}
}

void UModuleListItemWidget::OnBuildButtonHovered()
{
	UAudioEventLibrary::PlaySecondary2D(this, TEXT("UI.Hover"), 0.05f);
}

void UModuleListItemWidget::OnBuildButtonClicked()
{
	if (OnModuleSelected.IsBound() && ModuleClass)
	{
		OnModuleSelected.Execute(ModuleClass);
	}
}
