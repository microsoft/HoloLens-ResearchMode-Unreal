// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HololensForearmTrackingAlgorithmThread.h"
#include "HAL/RunnableThread.h"

#if PLATFORM_HOLOLENS

FHololensForearmTrackingAlgorithmThread::FHololensForearmTrackingAlgorithmThread(
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
	uint8& CurrentForearmArraySaturation)
	: Hand(Hand)
	, LeftFrontCameraWorldTransform(LeftFrontCameraWorldTransform)
	, WristTransform(WristTransform)
	, MappedTexture(MappedTexture)
	, AHATSensor(AHATSensor)
	, SensorWidth(SensorWidth)
	, SensorHeight(SensorHeight)
	, ForearmSmoothingUpdateInterval(ForearmSmoothingUpdateInterval)
	, ForearmDirectionCache(ForearmDirectionCache)
	, ForearmDirectionSum(ForearmDirectionSum)
	, CurrentForearmIndex(CurrentForearmIndex)
	, CurrentForearmArraySaturation(CurrentForearmArraySaturation)
{
	FString ThreadName = TEXT("Hololens Forearm Tracking Algorithm Thread");

	/*
		Set thread priority
	*/

	EThreadPriority ThreadPriority = TPri_Highest; // TPri_Normal, TPri_AboveNormal, TPri_Highest, TPri_TimeCritical

	Thread = FRunnableThread::Create(this, *ThreadName, 0, ThreadPriority);
}

FHololensForearmTrackingAlgorithmThread::~FHololensForearmTrackingAlgorithmThread()
{
	if (Thread) Thread->Kill(true);

	EnsureCompletion();
}

uint32 FHololensForearmTrackingAlgorithmThread::Run()
{
	Algorithm_V1_Optimized(WristTransform);
	return 0;
}


void FHololensForearmTrackingAlgorithmThread::EnsureCompletion()
{
	/*
		Note: This never worked correctly whenever the algorithm was stalled. WaitForCompletion() never returned
	*/

	Stop();

	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
}

uint16 FHololensForearmTrackingAlgorithmThread::GetDepthAtCoordinate(PixelCoordinate Coordinate) const
{
	/*
		@TODO: Version 2: Don't utilize coordinates outside the sensor in the algo (outside 512x512 i.e. negative values and greater than 512 values)
	*/

	checkf(MappedTexture, TEXT("Attempting to read from empty MappedTexture"));
	checkf(SensorHeight != 0 && SensorWidth != 0, TEXT("Sensor dimensions not set"));

	if (Coordinate.Key >= SensorWidth || Coordinate.Value >= SensorHeight || Coordinate.Key < 0 || Coordinate.Value < 0)
	{
		// Outside sensor resolution
		return 0;
	}

	if (MappedTexture != nullptr)
	{
		const int X = Coordinate.Key;
		const int Y = 512 - (Coordinate.Value + 1); // Must flip Y

		const uint16 DepthValue = MappedTexture[(SensorWidth * Y) + X];

		if (DepthValue <= 4090 && DepthValue != 4093 && DepthValue != 4095)
		{
			// NOT LOW IR OR OUTSIDE OF ACTIVE ILLUMINATION MASK
			return DepthValue;
		}
	}

	return 0;
}

PixelCoordinate FHololensForearmTrackingAlgorithmThread::GetCoordinatesAtLocation(FVector Location)
{
	const FVector TransformedLocation = LeftFrontCameraWorldTransform.InverseTransformPosition(Location);

	const float QueryPointForwardAxisValue = TransformedLocation.X;
	TTuple<float, float> QueryPointCoordinate(TransformedLocation.Y / QueryPointForwardAxisValue, TransformedLocation.Z / QueryPointForwardAxisValue);
	PixelCoordinate FinalQueryPointCoordinate = MapDepthCameraSpaceToImagePoint(QueryPointCoordinate);

	return FinalQueryPointCoordinate;
}

TTuple<float, float> FHololensForearmTrackingAlgorithmThread::MapImagePointToDepthCameraUnitPlane(const TTuple<float, float> UV)
{
	float xy[2] = { 0 };

#if PLATFORM_HOLOLENS
	if (AHATSensor)
	{
		float uv[2] = { UV.Key, UV.Value };
		AHATSensor->MapImagePointToCameraUnitPlane(uv, xy);
	}
#endif

	return TTuple<float, float>(xy[0], xy[1]);
}

TTuple<float, float> FHololensForearmTrackingAlgorithmThread::MapDepthCameraSpaceToImagePoint(const TTuple<float, float> XY)
{
	float uv_mapped[2] = { 0 };

#if PLATFORM_HOLOLENS
	if (AHATSensor)
	{
		float xy[2] = { XY.Key, XY.Value };
		AHATSensor->MapCameraSpaceToImagePoint(xy, uv_mapped);
	}
#endif

	return TTuple<float, float>(uv_mapped[0], uv_mapped[1]);
}

FVector2D FHololensForearmTrackingAlgorithmThread::DetectForearmWithBroadSweep(const PixelCoordinate Point, const float SensorToWristDistanceCentimeters, const FVector2D DirectionToEvaluate)
{
	/*
		We query the sensor frame in a "W" pattern (along 2 nearly perpendicular lines and along 2 lines that are flanked by a line on either side)
		Comparing depth values between these lines will result in minimual differences between the 2 internal lines and large differences between the internal liens and external lines when overlayed correctly on top of the forearm
		We sweep this pattern accross the frame and take the direction which generated the best score
	*/

	const FMatrix2x2 AngleBoundsMatrix(FMath::Cos(AngleBounds), FMath::Sin(AngleBounds) * -1.f, FMath::Sin(AngleBounds), FMath::Cos(AngleBounds));
	const FMatrix2x2 AngleIncrementMatrix = FMatrix2x2(FMath::Cos(AngleIncrement), FMath::Sin(AngleIncrement) * -1.f, FMath::Sin(AngleIncrement), FMath::Cos(AngleIncrement)).Inverse();
	const FVector2D StartingDirection = AngleBoundsMatrix.TransformPoint(DirectionToEvaluate);
	const FVector2D MaxAngle = AngleBoundsMatrix.Inverse().TransformPoint(DirectionToEvaluate);

	float HighestScore = FLT_MIN;
	FVector2D HighestScoringAngle = StartingDirection;
	FVector2D CurrentDirection = StartingDirection;
	FVector2D CurrentHighestChainStartingDirection = StartingDirection;
	FVector2D CurrentHighestChainEndingDirection = StartingDirection;

	const float OuterLineDisplacementAmountScaledByDistanceToWrist = OuterLineDisplacementAmount / (SensorToWristDistanceCentimeters / 100.f);
	const float InnerLineDisplacementAmountScaledByDistanceToWrist = OuterLineDisplacementAmountScaledByDistanceToWrist * InnerLineDisplacementAlpha;

	/*
		@TODO: Version 2: The closer the wrist the straighter the outer walls?
	*/

	const float InnerLineAngleDelta = OuterLineAngleDelta * InnerLineDisplacementAlpha;

	// Compute both inner and outer line angle delta matrices
	const FMatrix2x2 OuterLineAngleDeltaMatrix(FMath::Cos(OuterLineAngleDelta), FMath::Sin(OuterLineAngleDelta) * -1.f, FMath::Sin(OuterLineAngleDelta), FMath::Cos(OuterLineAngleDelta));
	const FMatrix2x2 InnerLineAngleDeltaMatrix(FMath::Cos(InnerLineAngleDelta), FMath::Sin(InnerLineAngleDelta) * -1.f, FMath::Sin(InnerLineAngleDelta), FMath::Cos(InnerLineAngleDelta));

	for (int j = 0; j < NumAnglesToEvaluate; j++)
	{
		uint16 BlipsAtLeft = 0;
		uint16 BlipsAtRight = 0;
		uint16 BlipsAtMiddle = 0;

		const FVector2D OuterLeftRotation(OuterLineAngleDeltaMatrix.TransformPoint(CurrentDirection));
		const FVector2D OuterRightRotation(OuterLineAngleDeltaMatrix.Inverse().TransformPoint(CurrentDirection));
		const FVector2D InnerLeftRotation(InnerLineAngleDeltaMatrix.TransformPoint(CurrentDirection));
		const FVector2D InnerRightRotation(InnerLineAngleDeltaMatrix.Inverse().TransformPoint(CurrentDirection));
		const FVector2D NewPerpendicularDirection(PerpendicularRotationMatrix.TransformPoint(CurrentDirection));

		FVector2D OuterLeftCoordinate(Point.Key + (NewPerpendicularDirection.X * OuterLineDisplacementAmountScaledByDistanceToWrist),
			Point.Value + (NewPerpendicularDirection.Y * OuterLineDisplacementAmountScaledByDistanceToWrist));
		FVector2D OuterRightCoordinate(Point.Key + (NewPerpendicularDirection.X * OuterLineDisplacementAmountScaledByDistanceToWrist * -1.f),
			Point.Value + (NewPerpendicularDirection.Y * OuterLineDisplacementAmountScaledByDistanceToWrist * -1.f));
		FVector2D InnerLeftCoordinate(Point.Key + (NewPerpendicularDirection.X * InnerLineDisplacementAmountScaledByDistanceToWrist),
			Point.Value + (NewPerpendicularDirection.Y * InnerLineDisplacementAmountScaledByDistanceToWrist));
		FVector2D InnerRightCoordinate(Point.Key + (NewPerpendicularDirection.X * InnerLineDisplacementAmountScaledByDistanceToWrist * -1.f),
			Point.Value + (NewPerpendicularDirection.Y * InnerLineDisplacementAmountScaledByDistanceToWrist * -1.f));

		/*
			@TODO: Version 2: Am I reading values from outside the frame? How should I handle that case?
		*/

		for (int i = 0; i < NumberOfQueryPointsAlongLine; i++)
		{
			/*
				@TODO: Version 2: Consider scaling DistanceBetweenQyerPointsInPixel by SensorToWristDistanceCentimeter
			*/
			OuterLeftCoordinate += FVector2D(OuterLeftRotation.X * DistanceBetweenQueryPointsInPixel, OuterLeftRotation.Y * DistanceBetweenQueryPointsInPixel);
			OuterRightCoordinate += FVector2D(OuterRightRotation.X * DistanceBetweenQueryPointsInPixel, OuterRightRotation.Y * DistanceBetweenQueryPointsInPixel);
			InnerLeftCoordinate += FVector2D(InnerLeftRotation.X * DistanceBetweenQueryPointsInPixel, InnerLeftRotation.Y * DistanceBetweenQueryPointsInPixel);
			InnerRightCoordinate += FVector2D(InnerRightRotation.X * DistanceBetweenQueryPointsInPixel, InnerRightRotation.Y * DistanceBetweenQueryPointsInPixel);

			const float OuterLeftDepthQuery = GetDepthAtCoordinate(PixelCoordinate(OuterLeftCoordinate.X, OuterLeftCoordinate.Y));
			const float OuterRightDepthQuery = GetDepthAtCoordinate(PixelCoordinate(OuterRightCoordinate.X, OuterRightCoordinate.Y));
			const float InnerLeftDepthQuery = GetDepthAtCoordinate(PixelCoordinate(InnerLeftCoordinate.X, InnerLeftCoordinate.Y));
			const float InnerRightDepthQuery = GetDepthAtCoordinate(PixelCoordinate(InnerRightCoordinate.X, InnerRightCoordinate.Y));

			if (FMath::Abs(OuterLeftDepthQuery - InnerLeftDepthQuery) >= DepthDeltaThreshold)
			{
				BlipsAtLeft++;
			}

			if (FMath::Abs(InnerLeftDepthQuery - InnerRightDepthQuery) >= DepthDeltaThreshold)
			{
				BlipsAtMiddle++;
			}

			if (FMath::Abs(InnerRightDepthQuery - OuterRightDepthQuery) >= DepthDeltaThreshold)
			{
				BlipsAtRight++;
			}
		}

		float CurrentScore = BlipsAtLeft + BlipsAtRight - BlipsAtMiddle;

		/*
			@TODO: Version 2: Value left/blips differently depending on left/right arm?
		*/

		/*
			@TODO: Version 2: Determine if you want to keep valuing uninterupted chains
		*/

		if (CurrentScore > HighestScore)
		{
			CurrentHighestChainStartingDirection = CurrentDirection;
			CurrentHighestChainEndingDirection = CurrentDirection;
			HighestScore = CurrentScore;
			HighestScoringAngle = CurrentDirection;
		}
		else if (CurrentScore == HighestScore)
		{
			CurrentHighestChainEndingDirection = CurrentDirection;
		}

		CurrentDirection = AngleIncrementMatrix.TransformPoint(CurrentDirection);
	}

	/*
		@TODO: Version 2 : Derive a more sophisticated scoring system than "split the difference"
	*/

	return ((CurrentHighestChainStartingDirection + CurrentHighestChainEndingDirection) / 2.f).GetSafeNormal();
}

void FHololensForearmTrackingAlgorithmThread::ApproximateCenterMostForearmPoint(const PixelCoordinate WristSensorCoordinate, const FVector2D ForearmCenterLineRotationAsVector, PixelCoordinate& AverageCoordinate, float& AverageDepth)
{
	/*
		Now we estimate the outline of the forearm
	*/

	const FVector2D ForearmPerpendicular2DDirection = PerpendicularRotationMatrix.TransformPoint(ForearmCenterLineRotationAsVector);

	for (int i = 0; i < NumberOfQueryPointsAlongLine; i++)
	{
		/*
			@TODO: Version 2: Try to salvage some data from the forearm line detection instead of doing an entirely new set of queries?
		*/

		const float QueryRadiusInPixels = DistanceBetweenQueryPointsInPixel * (i + 1);

		PixelCoordinate CoordinateAlongEstimatedCenterLine(WristSensorCoordinate.Key + (ForearmCenterLineRotationAsVector.X * QueryRadiusInPixels), WristSensorCoordinate.Value + (ForearmCenterLineRotationAsVector.Y * QueryRadiusInPixels));

		/*
			Now we find the edges of the forearm. We do this by stepping away perpendicularly from our estimated center line of the forearm and examining the depth at each step until we find a large delta

			@TODO: Version 2: Include an initial step offset to avoid unnecessary querying of the depth too close to the center line
		*/

		const PixelCoordinate PixelAlongCenterLine(
			WristSensorCoordinate.Key + (ForearmCenterLineRotationAsVector.X * QueryRadiusInPixels),
			WristSensorCoordinate.Value + (ForearmCenterLineRotationAsVector.Y * QueryRadiusInPixels)
		);

		for (int j = 0; j <= 1; j++)
		{
			/*
				0 == Left
				1 == Right
			*/

			const int8 bDirection = ((j == 0) ? 1.f : -1.f);

			float OffsetDistance = 6.f + (i * 1.15f); // We assume that a forearm has a thickness of 12 pixels and grows wider as it extends away from the hand
			float ForearmBoundCandidateDepth = 0;
			uint8 Counter = 0;

			PixelCoordinate ForearmBoundCandidate = PixelAlongCenterLine;

			float PreviousDepthQuery = GetDepthAtCoordinate(PixelCoordinate(
				PixelAlongCenterLine.Key + (ForearmPerpendicular2DDirection.X * OffsetDistance * bDirection),
				PixelAlongCenterLine.Value + (ForearmPerpendicular2DDirection.Y * OffsetDistance * bDirection)
			));

			while (Counter < MaxNumQueriesForearmWidthQueries)
			{
				OffsetDistance += OffsetIncrementFromCenterLine;

				const PixelCoordinate NewQueryCoordinate(
					PixelAlongCenterLine.Key + (ForearmPerpendicular2DDirection.X * OffsetDistance * bDirection),
					PixelAlongCenterLine.Value + (ForearmPerpendicular2DDirection.Y * OffsetDistance * bDirection)
				);

				const float NewDepthQuery = GetDepthAtCoordinate(NewQueryCoordinate);

				if (FMath::Abs(NewDepthQuery - PreviousDepthQuery) > SecondaryDepthDeltaThreshold)
				{
					break;
				}

				ForearmBoundCandidate = NewQueryCoordinate;
				ForearmBoundCandidateDepth = NewDepthQuery;
				Counter++;
				PreviousDepthQuery = NewDepthQuery;
			}

			AverageCoordinate.Key += ForearmBoundCandidate.Key;
			AverageCoordinate.Value += ForearmBoundCandidate.Value;
			AverageDepth += ForearmBoundCandidateDepth;
		}
	}

	AverageCoordinate.Key /= NumberOfQueryPointsAlongLine * 2;
	AverageCoordinate.Value /= NumberOfQueryPointsAlongLine * 2;
	AverageDepth /= NumberOfQueryPointsAlongLine * 2;
	AverageDepth *= SensorDepthToCentimeter;
}

FQuat FHololensForearmTrackingAlgorithmThread::InferForearm3DOrientationFrom2DDirection(const PixelCoordinate AverageCoordinate, const float AverageDepth, const FVector WristPosition, const FQuat WristOrientation)
{
	/*
		Use the average point along the forearm surface and the average distance to the bone to infer the bone direction in 3D space
	*/

	TTuple<float, float> CoordinateMappedToDepthCameraUnitPlane = MapImagePointToDepthCameraUnitPlane(AverageCoordinate);
	FVector NonNormalizedDirectionToAveragePoint(1.f, CoordinateMappedToDepthCameraUnitPlane.Key, CoordinateMappedToDepthCameraUnitPlane.Value);
	NonNormalizedDirectionToAveragePoint.Normalize();
	const FVector NormalizedDirectionToAveragePoint = NonNormalizedDirectionToAveragePoint;

	const FVector ForearmCenterLocation = LeftFrontCameraWorldTransform.TransformPosition(NormalizedDirectionToAveragePoint * (AverageDepth + DepthAverageOffset));
	FVector ForearmDirection = (ForearmCenterLocation - WristPosition) * -1.f;
	FVector NormalizedForearmDirection = ForearmDirection.GetSafeNormal();

	/*
		Average the inferred forearm direction with results from previous frames for a smoothed result
	*/

	if (bEnableSmoothing)
	{
		FVector SmoothedDirection;

		if (CurrentForearmArraySaturation < ForearmSmoothingUpdateInterval)
		{
			ForearmDirectionCache[CurrentForearmIndex] = NormalizedForearmDirection;
			ForearmDirectionSum += NormalizedForearmDirection;

			CurrentForearmArraySaturation++;
		}
		else
		{
			ForearmDirectionSum -= ForearmDirectionCache[CurrentForearmIndex];
			ForearmDirectionCache[CurrentForearmIndex] = NormalizedForearmDirection;
			ForearmDirectionSum += NormalizedForearmDirection;
		}

		SmoothedDirection = ForearmDirectionSum / CurrentForearmArraySaturation;
		NormalizedForearmDirection = SmoothedDirection.GetSafeNormal();
	}

	/*
		Combine the forearm direction vector with the rotation data from the wrist to create the final forearm rotation
	*/

	const FVector NormalizedForearmSideVector = FVector::CrossProduct(NormalizedForearmDirection, WristOrientation.GetUpVector()).GetSafeNormal();
	const FVector NormalizedForearmUpVector = FVector::CrossProduct(NormalizedForearmSideVector, NormalizedForearmDirection);
	const FQuat NonFinalForearmOrientation = NormalizedForearmDirection.ToOrientationQuat();

	float AdjustmentAngle = FMath::Acos(FVector::DotProduct(NonFinalForearmOrientation.GetUpVector(), NormalizedForearmUpVector));
	float VerticalProjectionValue = (FVector::DotProduct(NonFinalForearmOrientation.GetUpVector(), NormalizedForearmSideVector));

	return FQuat(NormalizedForearmDirection, AdjustmentAngle * (VerticalProjectionValue < 0 ? 1.f : -1.f)) * NonFinalForearmOrientation;

}

void FHololensForearmTrackingAlgorithmThread::Algorithm_V1_Optimized(FTransform& WristTransform)
{
	/*
		@TODO: Version 2: Write the logic for left vs right arm
	*/

	if (MappedTexture)
	{
		/*
			Constants
		*/

		const FVector WristPosition = WristTransform.GetTranslation();
		const FQuat WristOrientation = WristTransform.GetRotation();
		const FVector SensorWorldLocation = LeftFrontCameraWorldTransform.GetTranslation();
		const FVector InitialForearmDirection = WristOrientation.GetForwardVector() * -1.f;
		const FVector SensorToWristVector = (WristPosition - SensorWorldLocation);
		const float SensorToWristDistanceCentimeters = SensorToWristVector.Size();

		/*
			Calculate the direction of the wrist within the 2D sensor frame
		*/

		const FVector InitialWristDirectionSensorSpace = LeftFrontCameraWorldTransform.InverseTransformVector(InitialForearmDirection);
		FVector2D NonNormalizedWristStartingDirection2D(InitialWristDirectionSensorSpace.Y, InitialWristDirectionSensorSpace.Z);
		const FVector2D NormalizedWristStartingDirection2D = NonNormalizedWristStartingDirection2D.GetSafeNormal();

		/*
			Determine the direction of the forearm within the 2D sensor frame. We're more focused on accuracy than precision here as we'll clean up our result in the next step.
		*/

		const PixelCoordinate WristSensorCoordinate = GetCoordinatesAtLocation(WristPosition);

		FVector2D ForearmVectorDirection2D = DetectForearmWithBroadSweep(WristSensorCoordinate, SensorToWristDistanceCentimeters, NormalizedWristStartingDirection2D);

		/*
			Refine the calculate direction of the forearm and determine the average depth of pixels along the edge of the forearm as these values are roughly equivalent to the distance from the sensor to the forearm bones
		*/

		PixelCoordinate AverageCoordinate(0, 0);
		float AverageDepth = 0;

		ApproximateCenterMostForearmPoint(WristSensorCoordinate, ForearmVectorDirection2D, AverageCoordinate, AverageDepth);

		/*
			Use the information of the point along the forearm bone to infer the orientation of the forearm in 3D space
		*/

		const FQuat FinalForearmOrientation = InferForearm3DOrientationFrom2DDirection(AverageCoordinate, AverageDepth, WristPosition, WristOrientation);

		WristTransform.SetRotation(FinalForearmOrientation);
	}
}


#endif

