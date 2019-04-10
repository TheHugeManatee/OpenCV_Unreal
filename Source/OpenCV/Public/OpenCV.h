// (c) 2019 Technical University of Munich
// Jakob Weiss <jakob.weiss@tum.de>

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
