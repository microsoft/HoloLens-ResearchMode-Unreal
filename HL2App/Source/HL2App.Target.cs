// Copyright (c) Microsoft Corporation. Licensed under the MIT License.

using UnrealBuildTool;
using System.Collections.Generic;

public class HL2AppTarget : TargetRules
{
	public HL2AppTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "HL2App" } );
	}
}
