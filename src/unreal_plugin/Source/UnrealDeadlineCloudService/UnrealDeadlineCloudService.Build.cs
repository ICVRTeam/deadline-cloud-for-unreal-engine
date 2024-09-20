﻿// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealDeadlineCloudService : ModuleRules
{
	public UnrealDeadlineCloudService(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                    "Slate",
					"SlateCore",
					"EditorWidgets",

                    "DesktopWidgets",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"EditorFramework",
				"EditorStyle",
                "EditorWidgets",
                "DesktopWidgets",
                "UnrealEd",
				"LevelEditor",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				"MovieRenderPipelineCore",
				"PropertyEditor",
				"DeveloperSettings",
				"JsonUtilities",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
