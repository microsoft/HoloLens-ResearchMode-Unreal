// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensSensorInternal.h"
#include "HololensResearchModeContext.h"
#include "HololensResearchModeUtility.h"
#include "HololensSensor.h"
#include "HAL/RunnableThread.h"

#if PLATFORM_HOLOLENS

FHololensSensorInternal::FHololensSensorInternal(FHololensResearchModeContext* InContext, ResearchModeSensorType InType, UHololensSensor* InOutputSensor)
:Context(InContext)
, Type(InType)
, OutputSensor(InOutputSensor)
, bRunning(false)
{
}

bool FHololensSensorInternal::StartThread()
{
	if (!Context)
	{
		return false;
	}

	Context->RequestConsentStatus(FHololensResearchModeUtility::GetConsentType(Type));
	if (!Context->CreateDevice(Type, RMSensor))
	{
		return false;
	}	
	
	bRunning = true;
	FString ThreadName = TEXT("Thread for ");
	ThreadName += FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type);
	Thread = FRunnableThread::Create(this, *ThreadName);
	return true;
}

FHololensSensorInternal::~FHololensSensorInternal()
{
	bRunning = false;
	Thread->Kill(true);
	Thread->WaitForCompletion();
	Thread = nullptr;
}


uint32 FHololensSensorInternal::Run()
{
	EConsentType ConsentType = FHololensResearchModeUtility::GetConsentType(Type);
	EConsentStatus Status = EConsentStatus::Unknown;
	bool TimeOut = true;
	while (TimeOut && Status == EConsentStatus::Unknown)
	{
		Status = Context->WaitForConcentGiven(ConsentType, TimeOut);
		if (!bRunning)
		{
			return -1;
		}
	}

	if (Status != EConsentStatus::Approved || !bRunning)
	{
		bRunning = false;
		return -1;
	}

	HRESULT hr = RMSensor->OpenStream();
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensor::OpenStream with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		return -1;
	}

	OutputSensor->BeginLoop();

	while (bRunning)
	{
		hr = RMSensor->GetNextBuffer(&CurrentFrame);
		if (FAILED(hr))
		{
			FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensor::GetNextBuffer with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
			break;
		}

		if (!bRunning) { break; }

		OutputSensor->SensorLoop();
		CurrentFrame = nullptr;
	}

	OutputSensor->EndLoop();

	hr = RMSensor->CloseStream();
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensor::CloseStream with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		return -1;
	}

	return 0;
}

#endif
