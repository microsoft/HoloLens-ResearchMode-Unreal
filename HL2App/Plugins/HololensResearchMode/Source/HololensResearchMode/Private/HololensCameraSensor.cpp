// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensCameraSensor.h"
#include "VideoTextureResource.h"
#include "VideoTextureGenerator.h"
#include "Async/Async.h"



UHololensResearchModeVideoTexture::UHololensResearchModeVideoTexture(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHololensResearchModeVideoTexture::BeginDestroy()
{
	Super::BeginDestroy();
}

FTextureResource* UHololensResearchModeVideoTexture::CreateResource()
{
#if PLATFORM_HOLOLENS
	return new FVideoTextureResource(this);
#else
	return nullptr;
#endif
}


#if PLATFORM_HOLOLENS
void UHololensResearchModeVideoTexture::Init(const FSharedHandle& handle)
{
	if (Resource == nullptr)
	{
		UpdateResource();
	}

	FVideoTextureResource* LambdaResource = static_cast<FVideoTextureResource*>(Resource);
	FSharedHandle sharedHandle = handle;
	ENQUEUE_RENDER_COMMAND(Init_RenderThread)(
		[LambdaResource, sharedHandle](FRHICommandListImmediate&)
		{
			LambdaResource->Init_RenderThread(sharedHandle);
		});
}
#endif

UHololensCameraSensor::UHololensCameraSensor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SensorWidth = 0;
	SensorHeight = 0;
	bIsInitialized = false;
	CameraImage = NewObject<UHololensResearchModeVideoTexture>();
	CameraImage->Sensor = this;
#if PLATFORM_HOLOLENS
	VideoTextureGenerator = MakeShared<FVideoTextureGenerator>();
#endif
}

#if PLATFORM_HOLOLENS
bool UHololensCameraSensor::BeginSensorLoop()
{
	auto SensorFrame = Sensor->GetCurrentFrame();
	ResearchModeSensorResolution Resolution;
	HRESULT hr = SensorFrame->GetResolution(&Resolution);
	int32 TempSensorWidth = Resolution.Width;
	int32 TempSensorHeight = Resolution.Height;

	CalcTextureResolution(TempSensorWidth, TempSensorHeight);

	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorFrame::GetResolution"));
		return false;
	}

	if (VideoTextureGenerator->IsTextureNeedsToBeInitilized(TempSensorWidth, TempSensorHeight))
	{
		if (!VideoTextureGenerator->InitTexture(TempSensorWidth, TempSensorHeight))
		{
			UE_LOG(LogHLResearch, Error, TEXT("Video texture initialisation failed, sensor %s isn't available"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));

			{
				FScopeLock Lock(&Mutex);
				SensorWidth = 0;
				SensorHeight = 0;
				bIsInitialized = false;
			}
			return false;
		}

		CameraImage->Init(VideoTextureGenerator->GetTextureHandle());
		{
			FScopeLock Lock(&Mutex);
			SensorWidth = TempSensorWidth;
			SensorHeight = TempSensorHeight;
			bIsInitialized = true;
		}
	}

	return true;
}
#endif

bool UHololensCameraSensor::GetHololensSensorResolution(int32& OutSensorWidth, int32& OutSensorHeight) 
{
	FScopeLock Lock(&Mutex);
	OutSensorWidth = SensorWidth;
	OutSensorHeight = SensorHeight;

	return bIsInitialized;
}


void UHololensCameraSensor::NotifyCameraSensorTextureCreated()
{
	TFunction<void()> EventBroadcaster = [this]()
	{
		OnVideoTextureCreated.Broadcast(this);
	};

	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, MoveTemp(EventBroadcaster));
	}
	else
	{
		EventBroadcaster();
	}
}


void UHololensCameraSensor::NotifyCameraSensorTextureDestroyed()
{
	TFunction<void()> EventBroadcaster = [this]()
	{
		OnVideoTextureDestroyed.Broadcast(this);
	};

	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, MoveTemp(EventBroadcaster));
	}
	else
	{
		EventBroadcaster();
	}
}

void UHololensCameraSensor::CalcTextureResolution(int32& InOutWidth, int32& InOutHeight) const
{
	//do nothing by default
}
