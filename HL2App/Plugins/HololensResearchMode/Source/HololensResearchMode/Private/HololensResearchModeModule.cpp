// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensResearchModeModule.h"
#include "HololensResearchModeContext.h"
#include "HololensAccelerometer.h"
#include "HololensVLCCam.h"
#include "HololensDepthCam.h"
#include "HololensGyroscope.h"
#include "HololensMagnetometer.h"

#include "HololensShortThrowSensorStreamThread.h"

#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "CoreGlobals.h"
#include "Misc/ConfigCacheIni.h"

FHololensResearchModeModule* FHololensResearchModeModule::gThis;

FHololensResearchModeModule* FHololensResearchModeModule::Get()
{
	return gThis;
}

void FHololensResearchModeModule::StartSensorStreamThread()
{
#if PLATFORM_HOLOLENS
	ShortThrowSensorFrameQueryThread = new FHololensShortThrowSensorStreamThread(Context.Get());
	bool res = ShortThrowSensorFrameQueryThread->StartThread();
	if (!res)
	{
		UE_LOG(LogHLResearch, Error, TEXT("Failed to start a working thread for ForearmSensorThread"));
	}
#endif
}

void FHololensResearchModeModule::StartupModule()
{
	Context = MakeShared<FHololensResearchModeContext>();
	if (!Context->Init())
	{
		Context = nullptr;
	}
	gThis = this;
	StartSensorStreamThread();
}

void FHololensResearchModeModule::ShutdownModule()
{
	Context = nullptr;
	gThis = nullptr;
#if PLATFORM_HOLOLENS
	delete ShortThrowSensorFrameQueryThread;
#endif
	ShortThrowSensorFrameQueryThread = nullptr;
}

UHololensSensor* FHololensResearchModeModule::CreateSensor(EHololensSensorType Type, UObject* Outer)
{
	static uint32 SensorCounter = 0;
	FString Name = FString::Printf(TEXT("Hololens_%s_Sensor_%d"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type), SensorCounter++);
	UHololensSensor* Sensor;
	switch (Type)
	{
	case EHololensSensorType::LEFT_FRONT:
	case EHololensSensorType::LEFT_LEFT:
	case EHololensSensorType::RIGHT_FRONT:
	case EHololensSensorType::RIGHT_RIGHT:
		Sensor = NewObject<UHololensVLCCam>(Outer, *Name);
		break;
	case EHololensSensorType::DEPTH_AHAT:
	case EHololensSensorType::DEPTH_LONG_THROW:
		Sensor = NewObject<UHololensDepthCam>(Outer, *Name);
		break;
	case EHololensSensorType::IMU_GYRO:
		Sensor = NewObject<UHololensGyroscope>(Outer, *Name);
		break;
	case EHololensSensorType::IMU_MAG:
		Sensor = NewObject<UHololensMagnetometer>(Outer, *Name);
		break;
	case EHololensSensorType::IMU_ACCEL:
		Sensor = NewObject<UHololensAccelerometer>(Outer, *Name);
		break;
	default:
		return nullptr;
	}
	Sensor->Init(Context.Get(), Type);
	return Sensor;
}

void FHololensResearchModeModule::UpdateLatestWristTransformsFromHandTracker(bool bNewLeftHandstate, bool bNewRightHandstate, FTransform& LeftWristTransform, FTransform& RightWristTransform, FVector HeadsetPosition, FQuat HeadsetOrientation, FTransform TrackingToWorldTransform)
{
#if PLATFORM_HOLOLENS
	if (ShortThrowSensorFrameQueryThread)
		ShortThrowSensorFrameQueryThread->UpdateLatestWristTransformsFromHandTracker(bNewLeftHandstate, bNewRightHandstate, LeftWristTransform, RightWristTransform, HeadsetPosition, HeadsetOrientation, TrackingToWorldTransform);
#endif
}

DEFINE_LOG_CATEGORY(LogHLResearch);
IMPLEMENT_MODULE(FHololensResearchModeModule, HololensResearchMode);
