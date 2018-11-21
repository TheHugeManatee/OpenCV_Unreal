# OpenCV Plugin for Unreal Engine
Use OpenCV functionality from within Unreal Engine

# Features
 * Use `cv::VideoCapture` as an actor to get either camera or video images into a Render Target
 * `UCVUMat`: UE-Managed wrapper for `cv::UMat` to allow implementation of algorithms in Blueprints
 * Blueprint wrappers for image processing methods
 * Conversion of `UCVUMat` to Render target and vice versa

## Using the plugin in your own project
If you want to use the functinality in your own project, you will need to
 * Copy the Plugin from `Plugins/OpenCV` to your own project in the `Plugins` folder
 * Edit your Build configuration to reference the OpenCV module, e.g. by adapting your `Module.Build.cs` similar to the following:
    ```CSharp
    PublicDependencyModuleNames.AddRange(new string[] { 
        "Core", "CoreUObject", "Engine", "InputCore", // These are default ones you probably already have
        "OpenCV" // <-- This is the important one ;)
    });

    PrivateIncludePaths.AddRange(
        new string[] {
           "OpenCV/Private", // Make the header files available so you can use OpenCV headers in your C++ code
        }
    );
    ```

# Platforms
Currently only Windows is supported. This is mainly limited by my (non-existent) understanding of how building and including with external libraries works on different platforms.

**Tested on Windows, Unreal Engine 4.21** <br/>
However, other versions might still work.