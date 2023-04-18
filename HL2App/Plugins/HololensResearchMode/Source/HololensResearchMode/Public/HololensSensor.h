// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/UnrealType.h"
#include "UObject/ScriptMacros.h"


#include "HololensSensor.generated.h"


class FHololensResearchModeContext;
class FHololensSensorInternal;


UENUM(BlueprintType, Category = "HololensResearchMode")
enum class EHololensSensorType : uint8
{
	LEFT_FRONT         UMETA(DisplayName = "Left Front Cam"),
	LEFT_LEFT          UMETA(DisplayName = "Left Left Cam"),
	RIGHT_FRONT        UMETA(DisplayName = "Right Front Cam"),
	RIGHT_RIGHT        UMETA(DisplayName = "Right Right Cam"),
	DEPTH_AHAT         UMETA(DisplayName = "Short Distance Depth Scanner"),
	DEPTH_LONG_THROW   UMETA(DisplayName = "Long Distance Depth Scanner"),
	IMU_ACCEL          UMETA(DisplayName = "Accelerometer"),
	IMU_GYRO           UMETA(DisplayName = "Gyroscope"),
	IMU_MAG            UMETA(DisplayName = "Magnetometer"),
	Count              UMETA(Hidden)
};


UCLASS(BlueprintType, Category = "HololensResearchMode")
class HOLOLENSRESEARCHMODE_API UHololensSensor : public UObject
{
public:
	GENERATED_UCLASS_BODY()

	~UHololensSensor();

	virtual void Init(FHololensResearchModeContext * InContext, EHololensSensorType InType);

	UFUNCTION(BlueprintCallable, Category = "HololensResearchMode")
	virtual void StartCapturing();

	UFUNCTION(BlueprintCallable, Category = "HololensResearchMode")
	void StopCapturing();

protected:
	friend class FHololensSensorInternal;

	virtual void BeginLoop() {}
	virtual void SensorLoop() {}
	virtual void EndLoop() {}


	TSharedPtr<FHololensSensorInternal> Sensor;
	EHololensSensorType Type;
	FHololensResearchModeContext* Context;
};

