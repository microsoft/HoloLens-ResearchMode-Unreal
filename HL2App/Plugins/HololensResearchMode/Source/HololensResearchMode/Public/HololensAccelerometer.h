// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensSensor.h"

#include "HololensAccelerometer.generated.h"


USTRUCT(BlueprintType, Category = "HololensResearchMode")
struct HOLOLENSRESEARCHMODE_API FHololensAccelerometerFrame
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HololensResearchMode")
	int64 HostMilliseconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HololensResearchMode")
	FVector AccelSample = FVector::ZeroVector;
};



UCLASS(BlueprintType, Category = "HololensResearchMode")
class HOLOLENSRESEARCHMODE_API UHololensAccelerometer : public UHololensSensor
{
public:
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, Category = "HololensResearchMode")
	const FMatrix& GetExtrinsicsMatrix() const;

	UFUNCTION(BlueprintCallable, Category = "HololensResearchMode")
	void GetAccelerometerFrame(FHololensAccelerometerFrame& Frame);

protected:
	virtual void BeginLoop() override;
	virtual void SensorLoop() override;

	mutable FRWLock UpdateLock;
	FMatrix ExtrinsicsMatrix;

	FHololensAccelerometerFrame CurrentFrame;
	FHololensAccelerometerFrame PrevFrame;
};

