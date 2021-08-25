// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensSensor.h"
#include "ARTextures.h"

#include "HololensCameraSensor.generated.h"

class FSharedHandle;
class FVideoTextureGenerator;
class UHololensCameraSensor;

UCLASS()
class UHololensResearchModeVideoTexture :
	public UARTextureCameraImage
{
	GENERATED_UCLASS_BODY()

public:
	// UTexture interface implementation
	virtual void BeginDestroy() override;
	virtual FTextureResource* CreateResource() override;
	virtual EMaterialValueType GetMaterialType() const override { return MCT_Texture2D; }
	// End UTexture interface

	UHololensCameraSensor* GetSensor() const { return Sensor; }
protected:
	friend class UHololensCameraSensor;

	UHololensCameraSensor * Sensor;
#if PLATFORM_HOLOLENS
	void Init(const FSharedHandle& handle);
#endif
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHololensCameraSensorOnVideoTextureEvent, UHololensCameraSensor*, InSensor);


UCLASS(BlueprintType, Category = "HololensResearchMode")
class HOLOLENSRESEARCHMODE_API UHololensCameraSensor : public UHololensSensor
{
public:
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "HololensResearchMode")
	UHololensResearchModeVideoTexture* CameraImage;

	UFUNCTION(BlueprintCallable, Category = "HololensResearchMode")
	bool GetHololensSensorResolution(int32& OutSensorWidth, int32& OutSensorHeight);

	void NotifyCameraSensorTextureCreated();

	void NotifyCameraSensorTextureDestroyed();

	/** Event when this camera sensor finishes output texture creation. */
	UPROPERTY(BlueprintAssignable, Category = "HololensResearchMode")
	FHololensCameraSensorOnVideoTextureEvent OnVideoTextureCreated;

	/** Event when this camera sensor finishes output texture destruction. */
	UPROPERTY(BlueprintAssignable, Category = "HololensResearchMode")
	FHololensCameraSensorOnVideoTextureEvent OnVideoTextureDestroyed;

protected:
	virtual void CalcTextureResolution(int32& InOutWidth, int32& InOutHeight) const;

	mutable FCriticalSection Mutex;
	int32 SensorWidth;
	int32 SensorHeight;
	bool bIsInitialized;

#if PLATFORM_HOLOLENS
	bool BeginSensorLoop();
#endif

	TSharedPtr<FVideoTextureGenerator> VideoTextureGenerator;
};



