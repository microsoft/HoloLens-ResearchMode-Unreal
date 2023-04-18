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


void UHololensCameraSensor::StartCapturing()
{

#if PLATFORM_HOLOLENS

	/*
		AHAT Depth Resolution Hack Begins

		Sensor resolution required to generator texture, however the camera stream must be opened beforehand which can block the game thread
	*/

	int32 TempSensorWidth = 0;
	int32 TempSensorHeight = 0;

	switch (Type) {
		case EHololensSensorType::LEFT_LEFT:
		case EHololensSensorType::LEFT_FRONT:
		case EHololensSensorType::RIGHT_FRONT:
		case EHololensSensorType::RIGHT_RIGHT:
			TempSensorWidth = 640;
			TempSensorHeight = 480;
			break;
		case EHololensSensorType::DEPTH_AHAT:
			TempSensorWidth = 512;
			TempSensorHeight = 512;
			break;
		case EHololensSensorType::DEPTH_LONG_THROW:
			TempSensorWidth = 320;
			TempSensorHeight = 288;
			break;
		default:
			UE_LOG(LogHLResearch, Error, TEXT("Attempting to start a camera sensor for non-camera sensor type: %s"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
			return;
	}

	CalcTextureResolution(TempSensorWidth, TempSensorHeight);

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
			return;
		}

		CameraImage->Init(VideoTextureGenerator->GetTextureHandle());
		{
			FScopeLock Lock(&Mutex);
			SensorWidth = TempSensorWidth;
			SensorHeight = TempSensorHeight;
			bIsInitialized = true;
		}
	}

	/*
		AHAT Depth Resolution Hack Begins Ends
	*/

	Sensor = MakeShared<FHololensSensorInternal>(Context, (ResearchModeSensorType)Type, this);
	bool res = Sensor->StartThread();
	if (!res)
	{
		UE_LOG(LogHLResearch, Error, TEXT("Can't start a working thread for %s"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
	}
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
		{
			FScopeLock Lock(&Mutex);
			SensorWidth = 0;
			SensorHeight = 0;
			bIsInitialized = false;
		}

		return false;
	}

	{
		FScopeLock Lock(&Mutex);
		SensorWidth = TempSensorWidth;
		SensorHeight = TempSensorHeight;
		bIsInitialized = true;
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
