// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensMagnetometer.h"
#include "HololensSensorInternal.h"
#include "HololensResearchModeContext.h"
#include "HololensResearchModeUtility.h"

UHololensMagnetometer::UHololensMagnetometer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}



void UHololensMagnetometer::SensorLoop()
{
#if PLATFORM_HOLOLENS
	ResearchModeSensorTimestamp timestamp;
	auto SensorMagFrame = Sensor->QueryCurrentFrame<IResearchModeMagFrame>();
	if (!SensorMagFrame) { return; }

	if (!Sensor->GetCurrentTimestamp(timestamp)) { return; }

	DirectX::XMFLOAT3 MagSample;
	HRESULT hr = SensorMagFrame->GetMagnetometer(&MagSample);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeMagFrame::GetMagnetometer"));
		return;
	}

	CurrentFrame.HostMilliseconds = timestamp.HostTicks * 1000 / timestamp.HostTicksPerSecond;
	CurrentFrame.MagSample = FHololensResearchModeUtility::FromDirectXVector(MagSample);

	{
		FRWScopeLock Lock(UpdateLock, SLT_Write);
		PrevFrame = MoveTemp(CurrentFrame);
	}
#endif
}



void UHololensMagnetometer::GetMagnetometerFrame(FHololensMagnetometerFrame& Frame)
{
	FRWScopeLock Lock(UpdateLock, SLT_ReadOnly);
	Frame = PrevFrame;
}
