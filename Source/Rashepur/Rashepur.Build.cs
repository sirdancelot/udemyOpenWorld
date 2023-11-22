// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Rashepur : ModuleRules
{
	public Rashepur(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara", "HairStrandsCore", "GeometryCollectionEngine", "UMG", "AIModule"  });
	}
}
