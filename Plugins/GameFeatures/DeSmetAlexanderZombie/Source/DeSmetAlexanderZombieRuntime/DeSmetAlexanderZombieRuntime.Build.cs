// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DeSmetAlexanderZombieRuntime : ModuleRules
{
	public DeSmetAlexanderZombieRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			"DeSmetAlexanderZombieRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"GameFeatures",
			"ModularGameplay"
		});

		// Needed to access base project classes (SurvivorPawn, BaseZombie, etc.)
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"GameAI_Zombie"
		});
	}
}
