// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensResearchModeUtility.h"
#include "HAL/Runnable.h"

class FHololensResearchModeContext;
class FRunnableThread;
class UHololensSensor;



#if PLATFORM_HOLOLENS

class FHololensSensorInternal : public FRunnable
{
public:

	FHololensSensorInternal(FHololensResearchModeContext* InContext, ResearchModeSensorType InType, UHololensSensor* InOutputSensor);
	~FHololensSensorInternal();

	FORCEINLINE bool IsRunning() const { return bRunning; }
	bool StartThread();
	FORCEINLINE IResearchModeSensor* GetRMSensor() const { return RMSensor.Get(); }
	FORCEINLINE IResearchModeSensorFrame * GetCurrentFrame() const { return CurrentFrame.Get(); }

	template <typename S> 
	inline TComPtr<S> QueryRMSensor() const;

	template <typename S>
	inline TComPtr<S> QueryCurrentFrame() const;

	inline bool GetCurrentTimestamp(ResearchModeSensorTimestamp& timestamp) const;
private:
	virtual uint32 Run() override;

	FHololensResearchModeContext* Context;
	ResearchModeSensorType Type;
	UHololensSensor* OutputSensor;

	TAtomic<bool> bRunning;

	FRunnableThread * Thread;

	TComPtr<IResearchModeSensor> RMSensor;
	TComPtr<IResearchModeSensorFrame> CurrentFrame;
};


template <typename S> 
inline TComPtr<S> FHololensSensorInternal::QueryRMSensor() const
{
	IResearchModeSensor* Sensor = GetRMSensor();
	if (!Sensor)
	{
		return nullptr;
	}

	TComPtr<S> SpecificSensor;
	HRESULT hr = Sensor->QueryInterface(IID_PPV_ARGS(&SpecificSensor));
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensor::QueryInterface with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		return nullptr;
	}

	return SpecificSensor;
}


template <typename S>
inline TComPtr<S> FHololensSensorInternal::QueryCurrentFrame() const
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


inline bool FHololensSensorInternal::GetCurrentTimestamp(ResearchModeSensorTimestamp& timestamp) const
{
	IResearchModeSensorFrame* Frame = GetCurrentFrame();
	if (!Frame)
	{
		return false;
	}

	HRESULT hr = Frame->GetTimeStamp(&timestamp);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorFrame::GetTimeStamp with %s sensor type"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		return false;
	}
	return true;
}

#endif
