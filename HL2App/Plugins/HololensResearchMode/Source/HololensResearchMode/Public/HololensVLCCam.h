// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensCameraSensor.h"

#include "HololensVLCCam.generated.h"

class FVideoTextureResource;
class FVideoTextureGenerator;

UCLASS(BlueprintType, Category = "HololensResearchMode")
class HOLOLENSRESEARCHMODE_API UHololensVLCCam : public UHololensCameraSensor
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HololensResearchMode")
	bool GetIntrinsics(int32& OutGain, int64& OutExposure);


protected:
	virtual void SensorLoop() override;
	virtual void CalcTextureResolution(int32& InOutWidth, int32& InOutHeight) const override;

	uint32 Gain = 0;
	uint64 Exposure = 0;
};

