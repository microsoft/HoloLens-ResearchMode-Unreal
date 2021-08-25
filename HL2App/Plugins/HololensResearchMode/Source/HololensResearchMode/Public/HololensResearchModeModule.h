// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleInterface.h"
#include "HololensSensor.h"

class UHololensSensor;


class HOLOLENSRESEARCHMODE_API FHololensResearchModeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	static FHololensResearchModeModule* Get();

	UHololensSensor* CreateSensor(EHololensSensorType Type, UObject* Outer = (UObject*)GetTransientPackage());

private:
	friend class UHololensResearchModeFunctionLibrary;

	TSharedPtr<class FHololensResearchModeContext> Context;

	static FHololensResearchModeModule * gThis;
};


