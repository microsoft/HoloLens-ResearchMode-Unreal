// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensSensor.h"

#include "HololensMagnetometer.generated.h"


USTRUCT(BlueprintType, Category = "HololensResearchMode")
struct HOLOLENSRESEARCHMODE_API FHololensMagnetometerFrame
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HololensResearchMode")
	int64 HostMilliseconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HololensResearchMode")
	FVector MagSample = FVector::ZeroVector;;
};



UCLASS(BlueprintType, Category = "HololensResearchMode")
class HOLOLENSRESEARCHMODE_API UHololensMagnetometer : public UHololensSensor
{
public:
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "HololensResearchMode")
	void GetMagnetometerFrame(FHololensMagnetometerFrame& Frame);

protected:
	virtual void SensorLoop() override;

	mutable FRWLock UpdateLock;

	FHololensMagnetometerFrame CurrentFrame;
	FHololensMagnetometerFrame PrevFrame;
};

