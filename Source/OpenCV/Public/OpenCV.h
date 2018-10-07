// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

#include "OpenCV_Common.h"

class FOpenCVModule : public IModuleInterface {
public:
  /** IModuleInterface implementation */
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

private:
  /** Handle to the opencv dll */
  void* OpenCVLibraryHandle;
};
