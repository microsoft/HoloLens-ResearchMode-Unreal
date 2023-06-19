// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensResearchModeFunctionLibrary.h"
#include "HololensResearchModeModule.h"
#include "HololensAccelerometer.h"
#include "HololensResearchModeContext.h"

UHololensResearchModeFunctionLibrary::UHololensResearchModeFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UFUNCTION(BlueprintCallable, Category = "HololensResearchMode")
UHololensSensor* UHololensResearchModeFunctionLibrary::CreateSensor(EHololensSensorType Type, UObject* Outer)
{
	if (!Outer)
	{
		Outer = (UObject*)GetTransientPackage();
	}
	return FHololensResearchModeModule::Get()->CreateSensor(Type, Outer);
}


void UHololensResearchModeFunctionLibrary::SetEnableEyeSelection(bool enable)
{
#if PLATFORM_HOLOLENS
	auto Context = FHololensResearchModeModule::Get()->Context;
	if (!Context)
	{
		return;
	}

	if (enable)
	{
		HRESULT hr = Context->SensorDevice->EnableEyeSelection();
		if (FAILED(hr))
		{
			FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDevice::EnableEyeSelection"));
			return;
		}
	}
	else
	{
		HRESULT hr = Context->SensorDevice->DisableEyeSelection();
		if (FAILED(hr))
		{
			FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDevice::DisableEyeSelection"));
			return;
		}
	}
#endif
}

TArray<EHololensSensorType> UHololensResearchModeFunctionLibrary::GetAllAvailableSensors()
{
	TArray<EHololensSensorType> Result;

#if PLATFORM_HOLOLENS
	auto Context = FHololensResearchModeModule::Get()->Context;
	if (!Context)
	{
		return Result;
	}

	auto SensorDevice = Context->SensorDevice;
	size_t SensorCount = 0;
	TArray<ResearchModeSensorDescriptor> SensorDescriptors;

	HRESULT hr = SensorDevice->GetSensorCount(&SensorCount);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDevice::GetSensorCount"));
		return Result;
	}

	SensorDescriptors.SetNum(SensorCount);

	hr = SensorDevice->GetSensorDescriptors(SensorDescriptors.GetData(), SensorDescriptors.Num(), &SensorCount);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDevice::GetSensorDescriptors"));
		return Result;
	}

	for (auto Desc : SensorDescriptors)
	{
		Result.Add((EHololensSensorType)Desc.sensorType);
	}
#endif

	return Result;
}

void UHololensResearchModeFunctionLibrary::UpdateLatestWristTransforms(bool bNewLeftHandstate, bool bNewRightHandstate, FTransform& LeftWristTransform, FTransform& RightWristTransform, FVector HeadsetPosition, FQuat HeadsetOrientation, FTransform TrackingToWorldTransform)
{
#if PLATFORM_HOLOLENS
	FHololensResearchModeModule::Get()->UpdateLatestWristTransformsFromHandTracker(bNewLeftHandstate, bNewRightHandstate, LeftWristTransform, RightWristTransform, HeadsetPosition, HeadsetOrientation, TrackingToWorldTransform);
#endif
}