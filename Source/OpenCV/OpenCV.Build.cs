using System.IO;
using UnrealBuildTool;

public class OpenCV : ModuleRules
{
	public OpenCV(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnableExceptions = true;
        bEnableUndefinedIdentifierWarnings = false;
        bEnforceIWYU = true;

        PublicIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "Classes")
			});
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "Private"),
			});
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			});
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Engine",
                "Projects",
                "InputCore",
                "RHI",
                "RenderCore"
            });
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			});

        LoadOpenCVLib(Target);

    }

    public void LoadOpenCVLib(ReadOnlyTargetRules Target)
    {
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string OpenCVLibPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "opencv", "x64", "vc15");

            //Add the import library
            PublicLibraryPaths.AddRange(
                new string[] {
                     Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "opencv", "x64", "vc15", "lib")
                });

            PublicAdditionalLibraries.Add("opencv_world341.lib");
            PublicIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "opencv", "include") });



            //Delay - load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add("opencv_world341.dll");

            // Add a Runtime Dependency so the DLLs will be packaged correctly
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "opencv", "x64", "vc15", "bin", "opencv_world341.dll"));
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "opencv", "x64", "vc15", "bin", "opencv_ffmpeg341_64.dll"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "opencv_world341.dylib"));
        }
    }
}
