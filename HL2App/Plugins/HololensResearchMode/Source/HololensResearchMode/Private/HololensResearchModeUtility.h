// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HololensSensor.h"

enum class EConsentType : uint8
{
	IMU = 0,
	Cam = 1
};

class FHololensResearchModeUtility
{
public:
	// Convert between DirectX XMMatrix to Unreal FMatrix.
	static FORCEINLINE FMatrix ToFMatrix(DirectX::XMMATRIX M)
	{
		DirectX::XMFLOAT4X4 dst;
		DirectX::XMStoreFloat4x4(&dst, M);

		return FMatrix(
			FPlane(dst._11, dst._21, dst._31, dst._41),
			FPlane(dst._12, dst._22, dst._32, dst._42),
			FPlane(dst._13, dst._23, dst._33, dst._43),
			FPlane(dst._14, dst._24, dst._34, dst._44));
	}

	static FORCEINLINE FMatrix ToFMatrix(DirectX::XMFLOAT4X4 M)
	{
		return FMatrix(
			FPlane(M._11, M._21, M._31, M._41),
			FPlane(M._12, M._22, M._32, M._42),
			FPlane(M._13, M._23, M._33, M._43),
			FPlane(M._14, M._24, M._34, M._44));
	}

	static FORCEINLINE FVector FromDirectXVector(DirectX::XMFLOAT3 pos)
	{
		return FVector(
			-1.0f * pos.z,
			pos.x,
			pos.y);
	}

	static FORCEINLINE DirectX::XMFLOAT3 ToDirectXVector(FVector pos)
	{
		return DirectX::XMFLOAT3(
			pos.Y,
			pos.Z,
			-1.0f * pos.X);
	}

	static FORCEINLINE FVector FromDirectXScale(DirectX::XMFLOAT3 pos)
	{
		return FVector(
			pos.z,
			pos.x,
			pos.y);
	}

	static FORCEINLINE DirectX::XMFLOAT3 ToDirectXScale(FVector pos)
	{
		return DirectX::XMFLOAT3(
			pos.Y,
			pos.Z,
			pos.X);
	}


	static FORCEINLINE FQuat FromDirectXQuaternion(DirectX::XMFLOAT4 rot)
	{
		FQuat quaternion(
			-1.0f * rot.z,
			rot.x,
			rot.y,
			-1.0f * rot.w);
		quaternion.Normalize();

		return quaternion;
	}

	static FORCEINLINE DirectX::XMFLOAT4 ToDirectXQuaternion(FQuat rot)
	{
		// Windows api IsNormalized checks fail on a negative identity quaternion.
		if (rot == FQuat::Identity)
		{
			return DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		DirectX::XMVECTOR v = DirectX::XMVectorSet(
			rot.Y,
			rot.Z,
			-1.0f * rot.X,
			-1.0f * rot.W);
		DirectX::XMQuaternionNormalize(v);

		DirectX::XMFLOAT4 quatf4out;
		DirectX::XMStoreFloat4(&quatf4out, v);
		return quatf4out;
	}

	static const TCHAR* GetResearchModeSensorTypeName(EHololensSensorType InSensorType);
#if PLATFORM_HOLOLENS
	static const TCHAR* GetResearchModeSensorTypeName(ResearchModeSensorType InSensorType);
	static EConsentType GetConsentType(ResearchModeSensorType InSensorType);
	static void VARARGS PrintComError(HRESULT hr, const TCHAR* ComCall, ...);
#endif

};

