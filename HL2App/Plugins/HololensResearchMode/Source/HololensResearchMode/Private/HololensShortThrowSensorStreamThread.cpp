// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensShortThrowSensorStreamThread.h"
#include "HololensResearchModeContext.h"
#include "HololensResearchModeUtility.h"
#include "HololensForearmTrackingAlgorithmThread.h"
#include "HAL/RunnableThread.h"

#if PLATFORM_HOLOLENS

FHololensShortThrowSensorStreamThread::FHololensShortThrowSensorStreamThread(FHololensResearchModeContext* InContext)
	:Context(InContext)
	, bRunning(false)
{
}

bool FHololensShortThrowSensorStreamThread::StartThread()
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

	HRESULT hr = RMSensor->QueryInterface(IID_PPV_ARGS(&AHATSensor));
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensor::QueryInterface with %s sensor type"), "Short_Depth_Camera");
		return false;
	}

	for (int i = 0; i < ForearmSmoothingUpdateInterval; ++i)
	{
		LeftForearmDirectionCache[i] = FVector::ZeroVector;
		RightForearmDirectionCache[i] = FVector::ZeroVector;
	}

	bRunning = true;
	FString ThreadName = TEXT("Hololens Forearm Tracking Sensor Stream Thread");
	Thread = FRunnableThread::Create(this, *ThreadName, 0, TPri_Normal);

	return true;
}


void FHololensShortThrowSensorStreamThread::CompleteTrackingThreadAndProcessResult(FHololensForearmTrackingAlgorithmThread* AlgorithmThread, FTransform& WristTransform, FTransform& WristTransformBridge, uint8& CurrentForearmIndex, uint8& CurrentForearmArraySaturation, FVector* ForearmDirectionCache, FVector& ForearmDirectionSum, const FTransform TrackingToWorldTransform)
{
	if (AlgorithmThread)
	{
		AlgorithmThread->EnsureCompletion();
		delete AlgorithmThread;
		WristTransform = WristTransformBridge * TrackingToWorldTransform.Inverse();

		CurrentForearmIndex = (CurrentForearmIndex + 1) % ForearmSmoothingUpdateInterval;
	}
	else if (CurrentForearmArraySaturation > 0)
	{
		uint8 IndexToBeRemoved = (CurrentForearmIndex + (ForearmSmoothingUpdateInterval - CurrentForearmArraySaturation)) % ForearmSmoothingUpdateInterval;
		CurrentForearmArraySaturation--;
		FVector DirectionToBeRemoved = ForearmDirectionCache[IndexToBeRemoved];
		ForearmDirectionSum -= DirectionToBeRemoved;
	}
}

void FHololensShortThrowSensorStreamThread::UpdateLatestWristTransformsFromHandTracker(bool bNewLeftHandstate, bool bNewRightHandstate, FTransform& LeftWristTransform, FTransform& RightWristTransform, FVector HeadsetPosition, FQuat HeadsetOrientation, const FTransform TrackingToWorldTransform)
{
	/*
		Use a read/write lock for now and then build you own locking system later
		- One that wakes threads that are sleeping
	*/

	if (!bRunning) return;

	ImageArrayLock.WriteLock();

	FHololensForearmTrackingAlgorithmThread* LeftHandAlgorithmThread = nullptr;
	FHololensForearmTrackingAlgorithmThread* RightHandAlgorithmThread = nullptr;

	const FTransform HeadsetTransform = FTransform(HeadsetOrientation, HeadsetPosition) * TrackingToWorldTransform;
	const FTransform LeftFrontCameraWorldTransform = LeftFrontCameraRelativeTransform * HeadsetTransform;

	if (bNewLeftHandstate)
	{
		WristTransformBridge[0] = LeftWristTransform * TrackingToWorldTransform;

		LeftHandAlgorithmThread = new FHololensForearmTrackingAlgorithmThread(
			EControllerHand::Left, 
			LeftFrontCameraWorldTransform, 
			WristTransformBridge[0], 
			DepthMap, 
			AHATSensor, 
			SensorWidth,
			SensorHeight,
			ForearmSmoothingUpdateInterval,
			LeftForearmDirectionCache,
			LeftForearmDirectionSum,
			CurrentLeftForearmIndex,
			CurrentLeftForearmArraySaturation
		);
	}

	if (bNewRightHandstate)
	{
		WristTransformBridge[1] = RightWristTransform * TrackingToWorldTransform;

		RightHandAlgorithmThread = new FHololensForearmTrackingAlgorithmThread(
			EControllerHand::Right, 
			LeftFrontCameraWorldTransform, 
			WristTransformBridge[1], 
			DepthMap, 
			AHATSensor, 
			SensorWidth,
			SensorHeight,
			ForearmSmoothingUpdateInterval,
			RightForearmDirectionCache,
			RightForearmDirectionSum,
			CurrentRightForearmIndex,
			CurrentRightForearmArraySaturation
		);
	}

	CompleteTrackingThreadAndProcessResult(LeftHandAlgorithmThread, LeftWristTransform, WristTransformBridge[0], CurrentLeftForearmIndex, CurrentLeftForearmArraySaturation, LeftForearmDirectionCache, LeftForearmDirectionSum, TrackingToWorldTransform);
	CompleteTrackingThreadAndProcessResult(RightHandAlgorithmThread, RightWristTransform, WristTransformBridge[1], CurrentRightForearmIndex, CurrentRightForearmArraySaturation, RightForearmDirectionCache, RightForearmDirectionSum, TrackingToWorldTransform);

	ImageArrayLock.WriteUnlock();
}

FHololensShortThrowSensorStreamThread::~FHololensShortThrowSensorStreamThread()
{
	bRunning = false;
	Thread->Kill(true);
	Thread->WaitForCompletion();
	delete Thread;
	Thread = nullptr;
}

uint32 FHololensShortThrowSensorStreamThread::Run()
{
	/*
		Note: OpenStream and GetNextBuffer need to be called from the same thread.
		Since GetNextBuffer calls are blocking, per-sensor frame loops should be run on their own thread. 
		This allows sensors to be processed at their own framerate.
	*/

	EConsentType ConsentType = FHololensResearchModeUtility::GetConsentType(Type);
	EConsentStatus Status = EConsentStatus::Unknown;
	bool TimeOut = true;
	while (TimeOut && Status == EConsentStatus::Unknown)
	{
		Status = Context->WaitForConcentGiven(ConsentType, TimeOut);
		if (!bRunning)
		{
			return false;
		}
	}

	if (Status != EConsentStatus::Approved || !bRunning)
	{
		bRunning = false;
		return false;
	}

	HRESULT hr = RMSensor->OpenStream();
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensor::OpenStream with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		return false;
	}

	bool bSuccessful = true;
	
	while (bRunning)
	{
		/*
			NOTE: Calls to GetNextBuffer() are blocking
		*/
		
		hr = RMSensor->GetNextBuffer(&CurrentFrame);
		if (FAILED(hr))
		{
			FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensor::GetNextBuffer with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
			checkf(false, TEXT("A0"))
		}

		ImageArrayLock.WriteLock();

		SensorDepthFrame = QueryCurrentFrame<IResearchModeSensorDepthFrame>();
		if (!SensorDepthFrame)
		{
			bSuccessful = false;
			ImageArrayLock.WriteUnlock();
			break;
		}

		size_t OutBufferCount = 0;

		hr = SensorDepthFrame->GetBuffer(&DepthMap, &OutBufferCount);
		if (FAILED(hr))
		{
			FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDepthFrame::GetBuffer for %s"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));

			bSuccessful = false;
			ImageArrayLock.WriteUnlock();
			break;
		}

		ImageArrayLock.WriteUnlock();

		CurrentFrame = nullptr;
	}

	ImageArrayLock.WriteLock();
	delete [] DepthMap;
	DepthMap = nullptr;
	ImageArrayLock.WriteUnlock();

	hr = RMSensor->CloseStream();
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensor::CloseStream with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		return false;
	}

	return bSuccessful;
}

#endif

