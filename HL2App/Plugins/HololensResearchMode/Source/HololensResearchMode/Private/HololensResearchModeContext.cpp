// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensResearchModeContext.h"
#include "HololensResearchModeUtility.h"
#include "HAL/PlatformProcess.h"
#include "Async/TaskGraphInterfaces.h"
#include "Async/Async.h"

#if PLATFORM_HOLOLENS

extern "C"
HMODULE LoadLibraryA(
	LPCSTR lpLibFileName
);

#endif

FHololensResearchModeContext* FHololensResearchModeContext::gThis = nullptr;

FHololensResearchModeContext::FHololensResearchModeContext()
{
#if PLATFORM_HOLOLENS
	for (size_t i = 0; i < 2; ++i)
	{
		Concents[i].Event = FPlatformProcess::GetSynchEventFromPool();
	}
#endif

	gThis = this;
}

FHololensResearchModeContext::~FHololensResearchModeContext()
{
#if PLATFORM_HOLOLENS
	for (size_t i = 0; i < 2; ++i)
	{
		FPlatformProcess::ReturnSynchEventToPool(Concents[i].Event);
	}
#endif

	gThis = nullptr;
}

bool FHololensResearchModeContext::Init()
{
#if PLATFORM_HOLOLENS
	auto hrResearchMode = LoadLibraryA("ResearchModeAPI");
	if (!hrResearchMode)
	{
		UE_LOG(LogHLResearch, Error, TEXT("Can't find ResearchModeAPI lib"));
		return false;
	}

	typedef HRESULT(__cdecl * PFN_CREATEPROVIDER)(IResearchModeSensorDevice * *ppSensorDevice);
	PFN_CREATEPROVIDER pfnCreate = reinterpret_cast<PFN_CREATEPROVIDER>(FPlatformProcess::GetDllExport(hrResearchMode, TEXT("CreateResearchModeSensorDevice")));
	if (!pfnCreate)
	{
		UE_LOG(LogHLResearch, Error, TEXT("Can't find CreateResearchModeSensorDevice method"));
		return false;
	}

	HRESULT hr = pfnCreate(&SensorDevice);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("CreateResearchModeSensorDevice"));
		return false;
	}

	hr = SensorDevice->QueryInterface(IID_PPV_ARGS(&SensorDeviceConsent));
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDevice::QueryInterface(IResearchModeSensorDeviceConsent)"));
		return false;
	}

#endif
	return true;
}

#if PLATFORM_HOLOLENS

EConsentStatus FHololensResearchModeContext::RequestConsentStatus(EConsentType Type)
{
	if (!Concents[(uint8)Type].IsRequested.Exchange(true))
	{
		HRESULT hr = E_FAIL;
		switch (Type)
		{
			case EConsentType::IMU:
				hr = SensorDeviceConsent->RequestIMUAccessAsync([](ResearchModeSensorConsent consent) 
				{
					FHololensResearchModeContext::gThis->OnConsentGiven(EConsentType::IMU, consent);
				});
				break;
			case EConsentType::Cam:
				hr = SensorDeviceConsent->RequestCamAccessAsync([](ResearchModeSensorConsent consent) 
				{
					FHololensResearchModeContext::gThis->OnConsentGiven(EConsentType::Cam, consent);
				});
				break;
		}

		if (FAILED(hr))
		{
			FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDeviceConsent::RequestCamAccessAsync/RequestIMUAccessAsync"));
			Concents[(uint8)Type].Status = EConsentStatus::Denied;
		}
	}

	return Concents[(uint8)Type].Status;
}

void FHololensResearchModeContext::OnConsentGiven(EConsentType Type, ResearchModeSensorConsent consent)
{
	//AsyncTask(ENamedThreads::GameThread, [this, consent, Type]() 
	{ 
		const TCHAR* TypeName = TEXT("Unknown");
		switch (Type)
		{
			case EConsentType::IMU:
				TypeName = TEXT("IMU");
				break;
			case EConsentType::Cam:
				TypeName = TEXT("Cam");
				break;
		}
		
		switch (consent)
		{
			case DeniedBySystem: 
				UE_LOG(LogHLResearch, Error, TEXT("Consent on %s was denied by system"), TypeName);
				Concents[(uint8)Type].Status = EConsentStatus::Denied;
				break;
			case NotDeclaredByApp:
				UE_LOG(LogHLResearch, Error, TEXT("Consent on %s was denied by app"), TypeName);
				Concents[(uint8)Type].Status = EConsentStatus::Denied;
				break;
			case DeniedByUser: 
				UE_LOG(LogHLResearch, Error, TEXT("Consent on %s was denied by user"), TypeName);
				Concents[(uint8)Type].Status = EConsentStatus::Denied;
				break;
			case UserPromptRequired:
				UE_LOG(LogHLResearch, Warning, TEXT("Consent on %s require user prompt"), TypeName);
				Concents[(uint8)Type].Status = EConsentStatus::Denied;
				break;
			case Allowed:
				UE_LOG(LogHLResearch, Log, TEXT("Consent on %s is given"), TypeName);
				Concents[(uint8)Type].Status = EConsentStatus::Approved;
				break;

			default:
				Concents[(uint8)Type].Status = EConsentStatus::Denied;
				break;
		}
		Concents[(uint8)Type].IsGiven = true;
		Concents[(uint8)Type].Event->Trigger();
	}
	//);
}

EConsentStatus FHololensResearchModeContext::WaitForConcentGiven(EConsentType Type, bool& TimeOut)
{
	if (Concents[(uint8)Type].IsGiven)
	{
		TimeOut = false;
		return Concents[(uint8)Type].Status;
	}

	RequestConsentStatus(Type);

	TimeOut = !Concents[(uint8)Type].Event->Wait(500);

	return Concents[(uint8)Type].Status;
}



bool FHololensResearchModeContext::CreateDevice(ResearchModeSensorType InSensorType, TComPtr<IResearchModeSensor>& OutSensor)
{
	HRESULT hr = SensorDevice->GetSensor(InSensorType, &OutSensor);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDevice::GetSensor() for %s"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(InSensorType));
		return false;
	}
	return true;
}

#endif
