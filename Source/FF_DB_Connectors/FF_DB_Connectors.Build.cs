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

            // For LevelDB support.
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "leveldb", "Win64", "include"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "leveldb", "Win64", "lib", "leveldb.lib"));

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

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            // For LevelDB support.
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "leveldb", "Android", "include"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "leveldb", "Android", "lib", "leveldb.a"));
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
			}
			);
	}
}