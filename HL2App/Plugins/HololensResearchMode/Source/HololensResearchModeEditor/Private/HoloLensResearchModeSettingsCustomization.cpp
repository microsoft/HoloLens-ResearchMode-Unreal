// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.


#include "HoloLensResearchModeSettingsCustomization.h"
#include "SExternalImageReference.h"
#include "IExternalImagePickerModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "EditorDirectories.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Input/STextComboBox.h"
#include "ISourceControlModule.h"
#include "SourceControlHelpers.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/FileHelper.h"
#include "HoloLensResearchModeSettings.h"
#include "GeneralProjectSettings.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SErrorHint.h"
#include "HAL/PlatformFileManager.h"
#include "PropertyCustomizationHelpers.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "HoloLensResearchModeSettingsCustomization"


TSharedRef<IDetailCustomization> FHoloLensResearchModeSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FHoloLensResearchModeSettingsCustomization);
}


void FHoloLensResearchModeSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

	TSharedRef<IPropertyHandle> RestrictedCapabilityList = DetailBuilder.GetProperty("RestrictedCapabilityList");
	DetailBuilder.HideProperty(RestrictedCapabilityList);
	AddWidgetForCapability(DetailBuilder, 
		RestrictedCapabilityList,
		TEXT("perceptionSensorsExperimental"), 
		LOCTEXT("PerceptionSensorsExperimental", "Perception Sensors Experimental"), 
		LOCTEXT("PerceptionSensorsExperimentalTooltip", "Provides raw access to Hololens Research Mode Sensors."), 
		false);

	TSharedRef<IPropertyHandle> DeviceCapabilityList = DetailBuilder.GetProperty("DeviceCapabilityList");
	DetailBuilder.HideProperty(DeviceCapabilityList);
	AddWidgetForCapability(DetailBuilder,
		DeviceCapabilityList,
		TEXT("backgroundSpatialPerception"),
		LOCTEXT("BackgroundSpatialPerception", "Background Spatial Perception"),
		LOCTEXT("BackgroundSpatialPerceptionTooltip", "Perception Sensors access while application is on background"),
		false);

	// If this is the first time capabilities are being accessed for the project, enable defaults.
	TSharedRef<IPropertyHandle> SetDefaultCapabilitiesProperty = DetailBuilder.GetProperty("bSetDefaultCapabilities");
	DetailBuilder.HideProperty(SetDefaultCapabilitiesProperty);
	bool bSetDefaults;
	SetDefaultCapabilitiesProperty->GetValue(bSetDefaults);
	if (bSetDefaults)
	{
		OnCapabilityStateChanged(ECheckBoxState::Checked, RestrictedCapabilityList, TEXT("perceptionSensorsExperimental"));
		OnCapabilityStateChanged(ECheckBoxState::Checked, DeviceCapabilityList, TEXT("backgroundSpatialPerception"));

		SetDefaultCapabilitiesProperty->NotifyPreChange();
		SetDefaultCapabilitiesProperty->SetValue(false);
		SetDefaultCapabilitiesProperty->NotifyPostChange();
	}
}



void FHoloLensResearchModeSettingsCustomization::OnSelectedItemChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> Handle)
{
	Handle->SetValue(*NewValue);
}

void FHoloLensResearchModeSettingsCustomization::AddWidgetForCapability(IDetailLayoutBuilder& DetailBuilder, TSharedRef<IPropertyHandle> CapabilityList, const FString& CapabilityName, const FText& CapabilityCaption, const FText& CapabilityTooltip, bool bForAdvanced)
{
	IDetailCategoryBuilder& CapabilityBuilder = DetailBuilder.EditCategory(FName("Capabilities"));

	// Initialize checkbox state based on whether or not the capability currently exists in the CapabilityList.
	ECheckBoxState currentState = IsCapabilityChecked(CapabilityList, CapabilityName);
	if (currentState == ECheckBoxState::Checked)
	{
		OnCapabilityStateChanged(currentState, CapabilityList, CapabilityName);
	}

	CapabilityBuilder.AddCustomRow(CapabilityCaption, bForAdvanced)
	.NameContent()
	[
		SNew(STextBlock)
		.Text(CapabilityCaption)
		.ToolTipText(CapabilityTooltip)
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	.VAlign(VAlign_Center)
	[
		SNew(SCheckBox)
		.IsChecked(this, &FHoloLensResearchModeSettingsCustomization::IsCapabilityChecked, CapabilityList, CapabilityName)
		.OnCheckStateChanged(this, &FHoloLensResearchModeSettingsCustomization::OnCapabilityStateChanged, CapabilityList, CapabilityName)
	];
}

ECheckBoxState FHoloLensResearchModeSettingsCustomization::IsCapabilityChecked(TSharedRef<IPropertyHandle> CapabilityList, const FString CapabilityName) const
{
	int32 Index;
	TArray<FString> Caps;
	FString Path = TEXT("+") + CapabilityList->GeneratePathToProperty();
	FString iniFile = FPaths::Combine(FPaths::ProjectConfigDir(), FString("HoloLens"), FString("HoloLensEngine.ini"));
	GConfig->GetArray(TEXT("/Script/HoloLensPlatformEditor.HoloLensTargetSettings"), *Path, Caps, *iniFile);
	bool Found = Caps.Find(CapabilityName, Index);


	return (Found ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
}

void FHoloLensResearchModeSettingsCustomization::OnCapabilityStateChanged(ECheckBoxState CheckState, TSharedRef<IPropertyHandle> CapabilityList, const FString CapabilityName)
{
	bool IsEnabled = (CheckState == ECheckBoxState::Checked);
	FString iniFile = FPaths::Combine(FPaths::ProjectConfigDir(), FString("HoloLens"), FString("HoloLensEngine.ini"));

	FString Path = TEXT("+") + CapabilityList->GeneratePathToProperty();

	int32 Index;
	TArray<FString> Caps;
	GConfig->GetArray(TEXT("/Script/HoloLensPlatformEditor.HoloLensTargetSettings"), *Path, Caps, *iniFile);
	bool Found = Caps.Find(CapabilityName, Index);

	if (Found && !IsEnabled)
	{
		// Remove existing capability from the list
		Caps.RemoveAt(Index);
	}
	else if (!Found && IsEnabled)
	{
		//Add new capability to the list
		Caps.AddUnique(*CapabilityName);
	}

	GConfig->SetArray(TEXT("/Script/HoloLensPlatformEditor.HoloLensTargetSettings"), *Path, Caps, *iniFile);
	GConfig->Flush(false, *iniFile);
}


#undef LOCTEXT_NAMESPACE

