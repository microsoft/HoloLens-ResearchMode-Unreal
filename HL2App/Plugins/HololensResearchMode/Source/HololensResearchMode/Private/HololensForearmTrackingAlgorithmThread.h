// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HAL/Runnable.h"

class FHololensResearchModeContext;
class FRunnableThread;

#if PLATFORM_HOLOLENS

typedef TTuple<int16, int16> PixelCoordinate;

class FHololensForearmTrackingAlgorithmThread : public FRunnable
{

public:

	FHololensForearmTrackingAlgorithmThread(
		const EControllerHand Hand,
		const FTransform LeftFrontCameraWorldTransform,
		FTransform& WristTransform,
		const uint16* MappedTexture,
		TComPtr<IResearchModeCameraSensor> AHATSensor,
		const uint16 SensorWidth,
		const uint16 SensorHeight,
		const uint8 ForearmSmoothingUpdateInterval,
		FVector* ForearmDirectionCache,
		FVector& ForearmDirectionSum,
		const uint8 CurrentForearmIndex,
		uint8& CurrentForearmArraySaturation
	);

	~FHololensForearmTrackingAlgorithmThread();

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();

private:
	virtual uint32 Run() override;

	FRunnableThread* Thread;

	static constexpr bool bEnableSmoothing = true;

	TAtomic<bool> bRunning;

	TComPtr<IResearchModeCameraSensor> AHATSensor;

	// This is the largest value written to the depth map
	static constexpr USHORT MaxClampDepth = 1055;

	const EControllerHand Hand;
	const uint16 SensorWidth;
	const uint16 SensorHeight;

	const uint16* MappedTexture = nullptr;
	FTransform& WristTransform;

	const FTransform LeftFrontCameraWorldTransform;

	const uint8 ForearmSmoothingUpdateInterval;
	FVector* ForearmDirectionCache;
	FVector& ForearmDirectionSum;
	const uint8 CurrentForearmIndex;
	uint8& CurrentForearmArraySaturation;

	static constexpr float QuarterTurn = PI / 2.f;
	const FMatrix2x2 PerpendicularRotationMatrix = FMatrix2x2(FMath::Cos(QuarterTurn), FMath::Sin(QuarterTurn) * -1.f, FMath::Sin(QuarterTurn), FMath::Cos(QuarterTurn));
	static constexpr float AngleIncrement = 8.f * (PI / 180.f);

	static constexpr float InnerLineDisplacementAlpha = 0.075f;
	static constexpr float OuterLineDisplacementAmount = 14.f;
	static constexpr float OuterLineAngleDelta = 18.f * (PI / 180.f);
	static constexpr float AngleBounds = 80.f * (PI / 180.f);

	static constexpr uint8 NumAnglesToEvaluate = ((AngleBounds * 2.f) / AngleIncrement) + 1.f;

	static constexpr float SensorDepthToCentimeter = (100.f / MaxClampDepth);
	static constexpr float DepthDeltaThreshold = (2.f / SensorDepthToCentimeter); // 2cm
	static constexpr float SecondaryDepthDeltaThreshold = (0.5f / SensorDepthToCentimeter); // 0.5cm

	static constexpr float OffsetIncrementFromCenterLine = 2.f;
	static constexpr uint8 MaxNumQueriesForearmWidthQueries = 20;
	static constexpr float DepthAverageOffset = 4;

	/*
		Approx Middle Point Variables
	*/

	static constexpr uint16 LineLengthInPixel = 150;
	static constexpr uint8 NumberOfQueryPointsAlongLine = 15;
	static constexpr float DistanceBetweenQueryPointsInPixel = LineLengthInPixel / NumberOfQueryPointsAlongLine;

	/*
		Tolerance Variables
	*/

	TTuple<float, float> MapImagePointToDepthCameraUnitPlane(const TTuple<float, float> UV);
	TTuple<float, float> MapDepthCameraSpaceToImagePoint(const TTuple<float, float> XY);
	uint16 GetDepthAtCoordinate(PixelCoordinate Coordinate) const;
	PixelCoordinate GetCoordinatesAtLocation(FVector Location);

	FVector2D DetectForearmWithBroadSweep(const PixelCoordinate Point, const float SensorToWristDistanceCentimeters, const FVector2D AngleToEvaluate);
	void ApproximateCenterMostForearmPoint(const PixelCoordinate WristSensorCoordinate, const FVector2D ForearmCenterLineRotationAsVector, PixelCoordinate& AverageCoordinate, float& AverageDepth);;
	FQuat InferForearm3DOrientationFrom2DDirection(const PixelCoordinate AverageCoordinate, const float AverageDepth, const FVector WristPosition, const FQuat WristOrientation);
	void Algorithm_V1_Optimized(FTransform& WristTransform);
};

#endif
