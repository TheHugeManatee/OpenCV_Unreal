// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "OpenCV.h"
#include "Core.h"
#include "IPluginManager.h"
#include "ModuleManager.h"

#define LOCTEXT_NAMESPACE "FOpenCVModule"

#include <opencv2/opencv.hpp>

DEFINE_LOG_CATEGORY(OpenCV);

void FOpenCVModule::StartupModule() {
  // Get the base directory of this plugin
  FString BaseDir = IPluginManager::Get().FindPlugin("OpenCV")->GetBaseDir();

  // Add on the relative location of the third party dll and load it
  FString LibraryPath;
#if PLATFORM_WINDOWS
  LibraryPath =
      FPaths::Combine(*BaseDir, TEXT("ThirdParty/opencv/x64/vc15/bin/opencv_world341.dll"));
#elif PLATFORM_MAC
  LibraryPath =
      FPaths::Combine(*BaseDir, TEXT("ThirdParty/opencv/Mac/bin/libopencv_world341.dylib"));
#else
  static_assert(false, "Other platforms are currently not supported!");
#endif

  OpenCVLibraryHandle =
      !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

  if (OpenCVLibraryHandle) {
    UE_LOG(OpenCV, Log, TEXT("Loaded OpenCV Version %d.%d.%d"), CV_MAJOR_VERSION, CV_MINOR_VERSION,
           CV_SUBMINOR_VERSION)
  } else {
    UE_LOG(OpenCV, Error,
           TEXT("Failed to load opencv library! Check your include/lib paths and make sure "
                "opencv_world341.dll is deployed!"))
  }
}

void FOpenCVModule::ShutdownModule() {
  // Free the dll handle
  FPlatformProcess::FreeDllHandle(OpenCVLibraryHandle);
  OpenCVLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOpenCVModule, OpenCV)
