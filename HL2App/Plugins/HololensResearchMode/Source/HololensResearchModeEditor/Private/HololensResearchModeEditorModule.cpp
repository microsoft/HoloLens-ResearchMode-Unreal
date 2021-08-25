// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.


#include "PropertyEditorModule.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "HololensResearchModeSettings.h"
#include "HololensResearchModeSettingsCustomization.h"


#define LOCTEXT_NAMESPACE "FHololensResearchModeEditor"

/**
 * Module for the HoloLens Research Mode editor module.
 */
class FHololensResearchModeEditor
	: public IModuleInterface
{
public:

	/** Default constructor. */
	FHololensResearchModeEditor( )
	{ }

	/** Destructor. */
	~FHololensResearchModeEditor( )
	{
	}

public:

	// IModuleInterface interface

	virtual void StartupModule() override
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->RegisterSettings("Project", "Platforms", "HoloLens Research Mode",
				LOCTEXT("TargetSettingsName", "HoloLens Research Mode"),
				LOCTEXT("TargetSettingsDescription", "Settings for HoloLens Research Mode"),
				GetMutableDefault<UHoloLensResearchModeSettings>()
			);
		}

		// register settings detail panel customization
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(
			"HoloLensResearchModeSettings",
			FOnGetDetailCustomizationInstance::CreateStatic(&FHoloLensResearchModeSettingsCustomization::MakeInstance)
			);
	}

	virtual void ShutdownModule() override
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Project", "Platforms", "HoloLens Research Mode");
		}
	}

private:

};


#undef LOCTEXT_NAMESPACE


IMPLEMENT_MODULE(FHololensResearchModeEditor, HololensResearchModeEditor);

