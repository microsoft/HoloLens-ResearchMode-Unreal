// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HoloLensResearchModeSettings.generated.h"

/**
 * Implements the settings for the HoloLens target platform.
 */
UCLASS(config = Engine, defaultconfig)
class UHoloLensResearchModeSettings
	: public UObject
{
public:

	GENERATED_UCLASS_BODY()

	virtual const TCHAR* GetConfigOverridePlatform() const override
	{
		return TEXT("HoloLens");
	}


	/**
	 * List of supported <Capability><ResCap:Capability> elements for the application.
	 */
	UPROPERTY(EditAnywhere, config, Category = Capabilities)
	TArray<FString> RestrictedCapabilityList;

	
	/**
	 * List of supported <Capability><DeviceCapability> elements for the application.
	 */
	UPROPERTY(EditAnywhere, config, Category = Capabilities)
	TArray<FString> DeviceCapabilityList;


	/**
	 * Set default capabilities for the application.
	 */
	UPROPERTY(EditAnywhere, config, Category = Capabilities)
	bool bSetDefaultCapabilities = true;

};
