// Some copyright should be here...

using System;
using System.IO;
using UnrealBuildTool;

public class FF_DB_Connectors : ModuleRules
{
	public FF_DB_Connectors(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
        bEnableExceptions = true;

        if (UnrealTargetPlatform.Win64 == Target.Platform)
        {
            bUseRTTI = true;

            // For LMDB support.
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "lmdb", "Win64"));

            // For OLEDB support.
            PublicSystemLibraries.AddRange(new string[]
            {
                "oledb.lib",
                "msdasc.lib",
                "uuid.lib",
            }
            );
        }

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Json",
                "JsonUtilities",
                "JsonBlueprintUtilities",
                "SQLiteSupport",
                "SQLiteCore",
                "leveldb",
			}
			);
	}
}