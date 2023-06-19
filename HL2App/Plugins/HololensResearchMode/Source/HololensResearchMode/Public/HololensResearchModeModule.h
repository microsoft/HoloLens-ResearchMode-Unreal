// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleInterface.h"
#include "HololensSensor.h"

class UHololensSensor;
class FHololensShortThrowSensorStreamThread;

class HOLOLENSRESEARCHMODE_API FHololensResearchModeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	static FHololensResearchModeModule* Get();

	UHololensSensor* CreateSensor(EHololensSensorType Type, UObject* Outer = (UObject*)GetTransientPackage());

	void UpdateLatestWristTransformsFromHandTracker(bool bNewLeftHandstate, bool bNewRightHandstate, FTransform& LeftWristTransform, FTransform& RightWristTransform, FVector HeadsetPosition, FQuat HeadsetOrientation, FTransform TrackingToWorldTransform);

private:
	friend class UHololensResearchModeFunctionLibrary;

	TSharedPtr<class FHololensResearchModeContext> Context;

	static FHololensResearchModeModule * gThis;

	FHololensShortThrowSensorStreamThread* ShortThrowSensorFrameQueryThread;

	void StartSensorStreamThread();
};
