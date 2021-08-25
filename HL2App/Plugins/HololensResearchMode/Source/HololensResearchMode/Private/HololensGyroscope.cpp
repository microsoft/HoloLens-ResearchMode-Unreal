// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensGyroscope.h"
#include "HololensSensorInternal.h"
#include "HololensResearchModeContext.h"
#include "HololensResearchModeUtility.h"

UHololensGyroscope::UHololensGyroscope(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UHololensGyroscope::BeginLoop()
{
#if PLATFORM_HOLOLENS
	auto GyroSensor = Sensor->QueryRMSensor<IResearchModeGyroSensor>();
	if (!GyroSensor) { return; }

	DirectX::XMFLOAT4X4 DXMatrix;
	GyroSensor->GetExtrinsicsMatrix(&DXMatrix);

	ExtrinsicsMatrix = FHololensResearchModeUtility::ToFMatrix(DXMatrix);
#endif
}

void UHololensGyroscope::SensorLoop()
{
#if PLATFORM_HOLOLENS
	ResearchModeSensorTimestamp timestamp;
	auto SensorGyroFrame = Sensor->QueryCurrentFrame<IResearchModeGyroFrame>();
	if (!SensorGyroFrame) { return; }

	if (!Sensor->GetCurrentTimestamp(timestamp)) { return; }

	DirectX::XMFLOAT3 GyroSample;
	HRESULT hr = SensorGyroFrame->GetCalibratedGyro(&GyroSample);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeGyroFrame::GetCalibratedGyro"));
		return;
	}

	CurrentFrame.HostMilliseconds = timestamp.HostTicks * 1000 / timestamp.HostTicksPerSecond;
	CurrentFrame.GyroSample = FHololensResearchModeUtility::FromDirectXVector(GyroSample);

	{
		FRWScopeLock Lock(UpdateLock, SLT_Write);
		PrevFrame = MoveTemp(CurrentFrame);
	}
#endif
}


const FMatrix& UHololensGyroscope::GetExtrinsicsMatrix() const
{
	return ExtrinsicsMatrix;
}


void UHololensGyroscope::GetGyroscopeFrame(FHololensGyroscopeFrame& Frame)
{
	FRWScopeLock Lock(UpdateLock, SLT_ReadOnly);
	Frame = PrevFrame;
}
