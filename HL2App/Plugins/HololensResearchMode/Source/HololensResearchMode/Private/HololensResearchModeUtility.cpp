// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensResearchModeUtility.h"

#if PLATFORM_HOLOLENS

const TCHAR* FHololensResearchModeUtility::GetResearchModeSensorTypeName(ResearchModeSensorType InSensorType)
{
	switch (InSensorType)
	{
	case LEFT_FRONT:
		return TEXT("Left_Front_Camera");
	case LEFT_LEFT:
		return TEXT("Left_Left_Camera");
	case RIGHT_FRONT:
		return TEXT("Right_Front_Camera");
	case RIGHT_RIGHT:
		return TEXT("Right_Right_Camera");
	case DEPTH_AHAT:
		return TEXT("Short_Depth_Camera");
	case DEPTH_LONG_THROW:
		return TEXT("Long_Depth_Camera");
	case IMU_ACCEL:
		return TEXT("Accelerometer");
	case IMU_GYRO:
		return TEXT("Gyroscope");
	case IMU_MAG:
		return TEXT("Magnetometer");
	default:
		return TEXT("Unknown");
	}
}

EConsentType FHololensResearchModeUtility::GetConsentType(ResearchModeSensorType InSensorType)
{
	switch (InSensorType)
	{
	case LEFT_FRONT:
	case LEFT_LEFT:
	case RIGHT_FRONT:
	case RIGHT_RIGHT:
	case DEPTH_AHAT:
	case DEPTH_LONG_THROW:
		return EConsentType::Cam;
	case IMU_ACCEL:
	case IMU_GYRO:
	case IMU_MAG:
	default:
		return EConsentType::IMU;
	}
}

void FHololensResearchModeUtility::PrintComError(HRESULT hr, const TCHAR* ComCall, ...)
{
	TCHAR ErrorDesc[256] = TEXT("No Desc");
	FPlatformMisc::GetSystemErrorMessage(ErrorDesc, _countof(ErrorDesc), HRESULT_CODE(hr));

	int32	BufferSize	= 512;
	TCHAR*	Buffer		= (TCHAR*)FMemory::Malloc(BufferSize);
	int32	Result		= -1;

	GET_VARARGS_RESULT( Buffer, BufferSize, BufferSize-1, ComCall, ComCall, Result );

	while(Result == -1)
	{
		BufferSize *= 2;
		Buffer = (TCHAR*) FMemory::Realloc( Buffer, BufferSize * sizeof(TCHAR) );
		GET_VARARGS_RESULT( Buffer, BufferSize, BufferSize-1, ComCall, ComCall, Result );
	}

	Buffer[Result] = 0;


	UE_LOG(LogHLResearch, Error, TEXT("%s failed with %#010X code \'%s\'"), Buffer, hr, ErrorDesc);

	FMemory::Free( Buffer );
}


#endif

const TCHAR* FHololensResearchModeUtility::GetResearchModeSensorTypeName(EHololensSensorType InSensorType)
{
#if PLATFORM_HOLOLENS
	return GetResearchModeSensorTypeName((ResearchModeSensorType)InSensorType);
#else
	return TEXT("");
#endif
}
