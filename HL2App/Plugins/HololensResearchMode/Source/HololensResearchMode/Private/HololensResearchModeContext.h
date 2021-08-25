// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensResearchModeUtility.h"


enum class EConsentStatus
{
	Approved, Denied, Unknown
};


class FHololensResearchModeContext
{
public:

	FHololensResearchModeContext();
	~FHololensResearchModeContext();

	bool Init();

#if PLATFORM_HOLOLENS
	EConsentStatus RequestConsentStatus(EConsentType Type);

	EConsentStatus WaitForConcentGiven(EConsentType Type, bool& TimeOut);

	void OnConsentGiven(EConsentType Type, ResearchModeSensorConsent consent);

	bool CreateDevice(ResearchModeSensorType InSensorType, TComPtr<IResearchModeSensor>& OutSensor);

	//for internal reasons everything must be public
	struct SConcentRequest
	{
		EConsentStatus Status = EConsentStatus::Unknown;
		TAtomic<bool> IsRequested = false;
		TAtomic<bool> IsGiven = false;
		FEvent* Event = nullptr;
	};

	TComPtr < IResearchModeSensorDevice > SensorDevice;
	TComPtr < IResearchModeSensorDeviceConsent > SensorDeviceConsent;

	SConcentRequest Concents[2];
#endif

	static FHololensResearchModeContext* gThis;
};


