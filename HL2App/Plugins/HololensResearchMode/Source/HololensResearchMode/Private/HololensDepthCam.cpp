// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensDepthCam.h"
#include "HololensResearchModeUtility.h"
#include "VideoTextureGenerator.h"
#include "VideoTextureResource.h"


UHololensDepthCam::UHololensDepthCam(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


BYTE ConvertDepthPixel(USHORT v, BYTE bSigma, USHORT mask, USHORT maxshort, const int vmin, const int vmax)
{
	if ((mask != 0) && (bSigma & mask) > 0)
	{
		v = 0;
	}

	if ((maxshort != 0) && (v > maxshort))
	{
		v = 0;
	}

	float colorValue = 0.0f;
	if (v <= vmin)
	{
		colorValue = 0.0f;
	}
	else if (v >= vmax)
	{
		colorValue = 1.0f;
	}
	else
	{
		colorValue = (float)(v - vmin) / (float)(vmax - vmin);
	}

	return (BYTE)(colorValue * 255);
}


void UHololensDepthCam::SensorLoop()
{
#if PLATFORM_HOLOLENS
	if (!BeginSensorLoop())
	{
		UE_LOG(LogHLResearch, Error, TEXT("Sensor %s cannot start frame"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		return;
	}
	auto SensorDepthFrame = Sensor->QueryCurrentFrame<IResearchModeSensorDepthFrame>();
	if (!SensorDepthFrame)
	{ 
		FScopeLock Lock(&Mutex);
		bIsInitialized = false;
		return;
	}

	HRESULT hr;
	size_t OutBufferCount = 0;
	const UINT16* Depth = nullptr;
	const BYTE* Sigma = nullptr;
	const UINT16* AbImage = nullptr;

	hr = SensorDepthFrame->GetBuffer(&Depth, &OutBufferCount);
	if (FAILED(hr))
	{
		FScopeLock Lock(&Mutex);
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDepthFrame::GetBuffer for %s"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		bIsInitialized = false;
		return;
	}

	if (Type == EHololensSensorType::DEPTH_LONG_THROW)
	{
		hr = SensorDepthFrame->GetSigmaBuffer(&Sigma, &OutBufferCount);
		if (FAILED(hr))
		{
			FScopeLock Lock(&Mutex);
			FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDepthFrame::GetSigmaBuffer for %s"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
			bIsInitialized = false;
			return;
		}

		hr = SensorDepthFrame->GetAbDepthBuffer(&AbImage, &OutBufferCount);
		if (FAILED(hr))
		{
			FScopeLock Lock(&Mutex);
			FHololensResearchModeUtility::PrintComError(hr, TEXT("IResearchModeSensorDepthFrame::GetAbDepthBuffer for %s"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
			bIsInitialized = false;
			return;
		}
	}

	uint32* mappedTexture = (uint32*)VideoTextureGenerator->MapWriteCPUTexture();
	if (!mappedTexture)
	{
		FScopeLock Lock(&Mutex);
		UE_LOG(LogHLResearch, Error, TEXT("Sensor %s can't map output frame"), FHololensResearchModeUtility::GetResearchModeSensorTypeName(Type));
		bIsInitialized = false;
		return;
	}

	uint32 RowPitch = VideoTextureGenerator->GetRowPitch();


	if (Type == EHololensSensorType::DEPTH_LONG_THROW)
	{
		USHORT mask = 0x80;
		int maxClampDepth = 4000;

		for (int32 i = 0; i < SensorHeight; i++)
		{
			for (int32 j = 0; j < SensorWidth/2; j++)
			{
				{
					uint32 pixel = 0;
					BYTE inputPixel = ConvertDepthPixel(
						Depth[SensorWidth/2 * i + j],
						Sigma[SensorWidth / 2 * i + j],
						mask,
						0,
						0,
						maxClampDepth);

					pixel = inputPixel | (inputPixel << 8) | (inputPixel << 16);
					mappedTexture[(RowPitch / 4) * i + j] = pixel;
				}

				{
					UINT32 pixel = 0;
					BYTE inputPixel = ConvertDepthPixel(
						AbImage[SensorWidth / 2 * i + j],
						Sigma[SensorWidth / 2 * i + j],
						mask,
						0,
						0,
						maxClampDepth);

					pixel = inputPixel | (inputPixel << 8) | (inputPixel << 16);
					mappedTexture[(RowPitch / 4) * i + SensorWidth / 2 + j] = pixel;
				}
			}
		}
	}
	else if (Type == EHololensSensorType::DEPTH_AHAT)
	{
		USHORT mask = 0x0;
		int maxClampDepth = 1000;
		USHORT maxshort = 4090;

		for (int32 i = 0; i < SensorHeight; i++)
		{
			for (int32 j = 0; j < SensorWidth; j++)
			{
				uint32 pixel = 0;
				BYTE inputPixel = ConvertDepthPixel(
					Depth[SensorWidth * i + j],
					0,
					mask,
					maxshort,
					0,
					maxClampDepth);

				pixel = inputPixel | (inputPixel << 8) | (inputPixel << 16);
				mappedTexture[(RowPitch / 4) * i + j] = pixel;
			}
		}
	}

	VideoTextureGenerator->UnmapCPUTextureAndCopyOnDevice();
#endif
}

void UHololensDepthCam::CalcTextureResolution(int32& InOutWidth, int32& InOutHeight) const
{
	if (Type == EHololensSensorType::DEPTH_LONG_THROW)
	{
		InOutWidth *= 2;
	}
}

