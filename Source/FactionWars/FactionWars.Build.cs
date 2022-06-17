// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FactionWars : ModuleRules
{
	public FactionWars(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay",
			"AIModule",
			"GameplayTasks",
			"NavigationSystem",
			"GameplayTags"
		});
	}
}
