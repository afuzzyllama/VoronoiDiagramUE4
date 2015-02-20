// Copyright 2015 afuzzyllama. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class VoronoiDiagram : ModuleRules
	{
		public VoronoiDiagram(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"VoronoiDiagram/Private"
				}
      		);

            PrivateDependencyModuleNames.AddRange(
                new string[] {
                    "Core",
                    "CoreUObject",
                    "Engine"
                }
            );
        }
	}
}
