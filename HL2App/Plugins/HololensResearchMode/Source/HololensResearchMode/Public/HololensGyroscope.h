// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensSensor.h"

#include "HololensGyroscope.generated.h"


USTRUCT(BlueprintType, Category = "HololensResearchMode")
struct HOLOLENSRESEARCHMODE_API FHololensGyroscopeFrame
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HololensResearchMode")
	int64 HostMilliseconds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HololensResearchMode")
	FVector GyroSample;
};



UCLASS(BlueprintType, Category = "HololensResearchMode")
class HOLOLENSRESEARCHMODE_API UHololensGyroscope : public UHololensSensor
{
public:
	GENERATED_UCLASS_BODY()

		UFUNCTION(BlueprintPure, Category = "HololensResearchMode")
		const FMatrix& GetExtrinsicsMatrix() const;

	UFUNCTION(BlueprintCallable, Category = "HololensResearchMode")
		void GetGyroscopeFrame(FHololensGyroscopeFrame& Frame);

protected:
	virtual void BeginLoop() override;
	virtual void SensorLoop() override;

	mutable FRWLock UpdateLock;
	FMatrix ExtrinsicsMatrix;

	FHololensGyroscopeFrame CurrentFrame;
	FHololensGyroscopeFrame PrevFrame;
};

