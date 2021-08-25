// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensCameraSensor.h"

#include "HololensDepthCam.generated.h"


UCLASS(BlueprintType, Category = "HololensResearchMode")
class HOLOLENSRESEARCHMODE_API UHololensDepthCam : public UHololensCameraSensor
{
	GENERATED_UCLASS_BODY()
public:

protected:
	virtual void SensorLoop() override;
	virtual void CalcTextureResolution(int32& InOutWidth, int32& InOutHeight) const override;

};

