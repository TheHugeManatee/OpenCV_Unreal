// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "OpenCV.h"
#include "Core.h"
#include "ModuleManager.h"
#include "IPluginManager.h"

#define LOCTEXT_NAMESPACE "FOpenCVModule"

#include <opencv2/opencv.hpp>

void FOpenCVModule::StartupModule()
{
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("OpenCV")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("ThirdParty/opencv/x64/vc15/bin/opencv_world341.dll"));
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("ThirdParty/opencv/Mac/bin/libopencv_world341.dylib"));
#endif // PLATFORM_WINDOWS

	OpenCVLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (!OpenCVLibraryHandle)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("OpenCV Library Error", "Failed to load opencv library"));
	}

	cv::Mat m = cv::Mat::zeros(512, 512, CV_8UC3);
	cv::circle(m, cv::Point(256, 256), 30, cv::Scalar(255, 127, 0), 3);
	cv::imshow("test", m);
	cv::waitKey(0);
}

void FOpenCVModule::ShutdownModule()
{
	// Free the dll handle
	FPlatformProcess::FreeDllHandle(OpenCVLibraryHandle);
	OpenCVLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOpenCVModule, OpenCV)