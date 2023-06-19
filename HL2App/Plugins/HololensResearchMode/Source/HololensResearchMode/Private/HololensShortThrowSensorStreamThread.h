// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HAL/Runnable.h"

class FHololensResearchModeContext;
class FRunnableThread;

#if PLATFORM_HOLOLENS

#define USE_SMOOTHING 1

class FHololensShortThrowSensorStreamThread : public FRunnable
{
public:

	FHololensShortThrowSensorStreamThread(FHololensResearchModeContext* InContext);
	~FHololensShortThrowSensorStreamThread();

	bool StartThread();

	void UpdateLatestWristTransformsFromHandTracker(bool bNewLeftHandstate, bool bNewRightHandstate, FTransform& LeftWristTransform, FTransform& RightWristTransform, FVector HeadsetPosition, FQuat HeadsetOrientation, const FTransform TrackingToWorldTransform);

private:

	virtual uint32 Run() override;
	void CompleteTrackingThreadAndProcessResult(FHololensForearmTrackingAlgorithmThread* AlgorithmThread, FTransform& WristTransform, FTransform& WristTransformBridge, uint8& CurrentForearmIndex, uint8& CurrentForearmArraySaturation, FVector* ForearmDirectionCache, FVector& ForearmDirectionSum, const FTransform TrackingToWorldTransform);

	FHololensResearchModeContext* Context;
	static constexpr ResearchModeSensorType Type = ResearchModeSensorType::DEPTH_AHAT;

	TAtomic<bool> bRunning;

	FRunnableThread* Thread;

	TComPtr<IResearchModeSensor> RMSensor;
	TComPtr<IResearchModeCameraSensor> AHATSensor;
	TComPtr<IResearchModeSensorFrame> CurrentFrame;
	TComPtr<IResearchModeSensorDepthFrame> SensorDepthFrame;

	//mutable FCriticalSection Mutex;
	mutable FRWLock ImageArrayLock;
	
	// Placeholder until I find a more precise way to determine the LeftFrontCamera offset from device center
	// X=6.500 Y=1.400 Z=2.500 Rotation: P=-20.999907 Y=0.000000 R=0.000000 Scale X=1.000 Y=1.000 Z=1.000
	const FTransform LeftFrontCameraRelativeTransform = FTransform(FRotator(-21.f, 0.f, 0.f), FVector(6.5f, 1.4f, 2.5f));

	// HoloLens 2 AHAT Sensor locked at 512 x 512
	static constexpr int32 SensorWidth = 512;
	static constexpr int32 SensorHeight = SensorWidth;

	const UINT16* DepthMap = nullptr;

	FTransform WristTransformBridge[2];

	/*
		Beginning of smoothing variables
	*/

	static constexpr uint8 ForearmSmoothingUpdateInterval = 5;

	FVector LeftForearmDirectionCache[ForearmSmoothingUpdateInterval];
	FVector RightForearmDirectionCache[ForearmSmoothingUpdateInterval];
	FVector LeftForearmDirectionSum = FVector::ZeroVector;
	FVector RightForearmDirectionSum = FVector::ZeroVector;

	uint8 CurrentLeftForearmIndex = 0;
	uint8 CurrentRightForearmIndex = 0;
	uint8 CurrentLeftForearmArraySaturation = 0;
	uint8 CurrentRightForearmArraySaturation = 0;

	/*
		End of smoothing variables
	*/

	FORCEINLINE bool IsRunning() const { return bRunning; }
	FORCEINLINE IResearchModeSensorFrame* GetCurrentFrame() const { return CurrentFrame.Get(); }

	template <typename S>
	inline TComPtr<S> QueryCurrentFrame() const;
};


template <typename S>
inline TComPtr<S> FHololensShortThrowSensorStreamThread::QueryCurrentFrame() const
{
	IResearchModeSensorFrame* Frame = GetCurrentFrame();
	if (!Frame)
	{
		return nullptr;
	}

	TComPtr<S> SpecificFrame;
	HRESULT hr = Frame->QueryInterface(IID_PPV_ARGS(&SpecificFrame));
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorFrame::QueryInterface with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		return nullptr;
	}

	return SpecificFrame;
}

#endif
