// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.


#pragma once

#include "PropertyEditorModule.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "IDetailGroup.h"

class SErrorHint;

class FHoloLensResearchModeSettingsCustomization : public IDetailCustomization
{
public:

	// Makes a new instance of this detail layout class for a specific detail view requesting it
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// End of IDetailCustomization interface
private:

	void AddWidgetForCapability(IDetailLayoutBuilder& DetailBuilder, TSharedRef<IPropertyHandle> CapabilityList, const FString& CapabilityName, const FText& CapabilityCaption, const FText& CapabilityTooltip, bool bForAdvanced);
	ECheckBoxState IsCapabilityChecked(TSharedRef<IPropertyHandle> CapabilityList, const FString CapabilityName) const;
	void OnCapabilityStateChanged(ECheckBoxState CheckState, TSharedRef<IPropertyHandle> CapabilityList, const FString CapabilityName);
	void OnSelectedItemChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> HandlePtr);
};

