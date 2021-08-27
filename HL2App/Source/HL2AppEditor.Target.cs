// Copyright (c) Microsoft Corporation. Licensed under the MIT License.

using UnrealBuildTool;
using System.Collections.Generic;

public class HL2AppEditorTarget : TargetRules
{
	public HL2AppEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "HL2App" } );
	}
}
