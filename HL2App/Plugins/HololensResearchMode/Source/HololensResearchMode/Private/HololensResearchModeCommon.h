// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.


#pragma warning(disable : 5205 4265 4268 4946)

#include "CoreMinimal.h"

#include "Logging/LogMacros.h"
#include "CoreGlobals.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/PreWindowsApi.h"

#include "Windows/MinWindows.h"

#include <unknwn.h>
#include <windows.h>
#if PLATFORM_HOLOLENS
#include <d3d11_1.h>
#endif
#include <DirectXMath.h>

#if PLATFORM_HOLOLENS
#include "External/ResearchModeApi.h"
#endif
#include "Microsoft/COMPointer.h"

#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"




DECLARE_LOG_CATEGORY_EXTERN(LogHLResearch, Log, All);


