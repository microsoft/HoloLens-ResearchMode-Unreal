// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensAccelerometer.h"
#include "HololensSensorInternal.h"
#include "HololensResearchModeContext.h"
#include "HololensResearchModeUtility.h"

UHololensAccelerometer::UHololensAccelerometer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UHololensAccelerometer::BeginLoop()
{
#if PLATFORM_HOLOLENS
	auto AccelSensor = Sensor->QueryRMSensor<IResearchModeAccelSensor>();
	if (!AccelSensor) { return; }

	DirectX::XMFLOAT4X4 DXMatrix;
	AccelSensor->GetExtrinsicsMatrix(&DXMatrix);

	ExtrinsicsMatrix = FHololensResearchModeUtility::ToFMatrix(DXMatrix);
#endif
}

void UHololensAccelerometer::SensorLoop()
{
#if PLATFORM_HOLOLENS
	ResearchModeSensorTimestamp timestamp;
	auto SensorAccelFrame = Sensor->QueryCurrentFrame<IResearchModeAccelFrame>();	
	if (!SensorAccelFrame) { return; }

	if (!Sensor->GetCurrentTimestamp(timestamp)) { return; }

	DirectX::XMFLOAT3 AccelSample;
	HRESULT hr = SensorAccelFrame->GetCalibratedAccelaration(&AccelSample);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeAccelFrame::GetCalibratedAccelaration"));
		return;
	}

	CurrentFrame.HostMilliseconds = timestamp.HostTicks * 1000 / timestamp.HostTicksPerSecond;
	CurrentFrame.AccelSample = FHololensResearchModeUtility::FromDirectXVector(AccelSample);

	{
		FRWScopeLock Lock(UpdateLock, SLT_Write);
		PrevFrame = MoveTemp(CurrentFrame);
	}
#endif
}


const FMatrix& UHololensAccelerometer::GetExtrinsicsMatrix() const
{
	return ExtrinsicsMatrix;
}


void UHololensAccelerometer::GetAccelerometerFrame(FHololensAccelerometerFrame& Frame)
{
	FRWScopeLock Lock(UpdateLock, SLT_ReadOnly);
	Frame = PrevFrame;
}
