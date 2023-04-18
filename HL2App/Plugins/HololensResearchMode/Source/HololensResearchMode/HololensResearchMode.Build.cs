// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using System.IO;
using System.Linq;
//using System.Runtime.Remoting.Messaging;
using UnrealBuildTool;
//using Tools.DotNETCommon;
using System;
using System.Collections.Generic;

public class HololensResearchMode : ModuleRules
{
	public HololensResearchMode(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = @"Private\HololensResearchModeCommon.h";

		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"ApplicationCore",
				"Engine",
				"Projects",
				"RHI",
				"RHICore"
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"AugmentedReality",
			}
		);

		PrivateIncludePathModuleNames.AddRange(
			new string[]
			{
			}
			);

		PublicIncludePathModuleNames.AddRange(
			new string[]
			{
			}
			);

		if (Target.Platform == UnrealTargetPlatform.HoloLens)
		{
			// DX-specific code for cam texture processing
			PrivateDependencyModuleNames.AddRange(new string[] {
				"D3D11RHI",
				"D3D12RHI"
			});

			var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
			PrivateIncludePaths.AddRange(
				new string[] {
						Path.Combine(EngineDir, @"Source\Runtime\Windows\D3D11RHI\Private"),
						Path.Combine(EngineDir, @"Source\Runtime\Windows\D3D11RHI\Private\Windows"),
						Path.Combine(EngineDir, @"Source\Runtime\D3D12RHI\Private"),
						Path.Combine(EngineDir, @"Source\Runtime\D3D12RHI\Private\Windows")
							});

			AddEngineThirdPartyPrivateStaticDependencies(Target, "DX11");
			AddEngineThirdPartyPrivateStaticDependencies(Target, "DX12");
			AddEngineThirdPartyPrivateStaticDependencies(Target, "NVAPI");
			AddEngineThirdPartyPrivateStaticDependencies(Target, "AMD_AGS");
			AddEngineThirdPartyPrivateStaticDependencies(Target, "NVAftermath");
			AddEngineThirdPartyPrivateStaticDependencies(Target, "IntelMetricsDiscovery");
			AddEngineThirdPartyPrivateStaticDependencies(Target, "IntelExtensionsFramework");
		}
	}
}
