// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensSensor.h"
#include "HololensResearchModeContext.h"
#include "HololensSensorInternal.h"

UHololensSensor::UHololensSensor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

UHololensSensor::~UHololensSensor()
{
	Sensor = nullptr;
}


void UHololensSensor::Init(FHololensResearchModeContext* InContext, EHololensSensorType InType)
{
	Context = InContext;
	Type = InType;
}


void UHololensSensor::StartCapturing()
{
#if PLATFORM_HOLOLENS
	Sensor = MakeShared<FHololensSensorInternal>(Context, (ResearchModeSensorType)Type, this);
	bool res = Sensor->StartThread();
	if (!res)
	{
		UE_LOG(LogHLResearch, Error, TEXT("Can't start a working thread for %s"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
	}
#endif
}

void UHololensSensor::StopCapturing()
{
	Sensor = nullptr;
}

