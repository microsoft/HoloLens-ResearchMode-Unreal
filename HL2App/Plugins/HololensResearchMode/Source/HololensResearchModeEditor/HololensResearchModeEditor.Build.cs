// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using UnrealBuildTool;

public class HololensResearchModeEditor : ModuleRules
{
	public HololensResearchModeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Settings",
				"TargetPlatform",
				"DesktopPlatform",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"PropertyEditor",
				"SharedSettingsWidgets",
				"AppFramework",
				"DesktopWidgets",
				"UnrealEd",
				"EngineSettings",
				"Projects",
			}
		);
	}
}
