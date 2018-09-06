// Fill out your copyright notice in the Description page of Project Settings.

#include "UCVUMat.h"

#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

#include <opencv2/opencv.hpp>

UCVUMat::UCVUMat() {
  UE_LOG(OpenCV, Verbose, TEXT("Default Constructed"));
};

UCVUMat::UCVUMat(const cv::UMat &m_) : m(m_) {
  UE_LOG(OpenCV, Verbose, TEXT("Constructed w/ cv::Mat constructor"));
};
// UCVMat(const UCVMat& ma) : m(ma.m) { /* */  };

UCVUMat::~UCVUMat() {
  UE_LOG(OpenCV, Verbose, TEXT("Destroyed"));
};

UCVUMat *UCVUMat::createMat(int32 rows, int32 cols, FCVMatType type) {
  UCVUMat *r = NewObject<UCVUMat>();
  int cvType = 0;

  switch (type) {
    case FCVMatType::CVT_8UC1: cvType = CV_8UC1; break;
    case FCVMatType::CVT_16UC1: cvType = CV_16UC1; break;
    case FCVMatType::CVT_8SC1: cvType = CV_8SC1; break;
    case FCVMatType::CVT_16SC1: cvType = CV_16SC1; break;
    case FCVMatType::CVT_32SC1: cvType = CV_32SC1; break;
    case FCVMatType::CVT_8UC3: cvType = CV_8UC3; break;
    case FCVMatType::CVT_8UC4: cvType = CV_8UC4; break;
  }

  // Init if we have a defined type
  if (cvType) {
    r->m.create(rows, cols, cvType);
  }

  return r;
}

UTextureRenderTarget2D *UCVUMat::toRenderTarget(UTextureRenderTarget2D *renderTarget, bool resize) {
  uint32 VideoSizeX = m.cols;
  uint32 VideoSizeY = m.rows;
  auto size = cv::Size{m.size()};

  auto VideoUpdateTextureRegion = new FUpdateTextureRegion2D{0, 0, 0, 0, VideoSizeX, VideoSizeY};

  cv::UMat frame = m;

  if (!renderTarget) {
    renderTarget = NewObject<UTextureRenderTarget2D>();
    renderTarget->InitCustomFormat(VideoSizeX, VideoSizeY, EPixelFormat::PF_B8G8R8A8, true);
    renderTarget->UpdateResourceImmediate(false);
  }

  if (frame.cols != renderTarget->SizeX || frame.rows != renderTarget->SizeY) {
    if (resize) {
      cv::resize(m, frame, cv::Size{renderTarget->SizeX, renderTarget->SizeY});
    } else {
      UE_LOG(OpenCV, Warning, TEXT("Render target size does not match and resize=false!"));
      return nullptr;
    }
  }

  auto dataSize = m.total() * 4;
  auto dataPtr = new uint8[dataSize];

  cv::Mat dataWrap(m.size(), CV_8UC4, dataPtr);
  // FColor also has a BGRA layout
  cv::cvtColor(m, dataWrap, CV_BGR2BGRA);

  UpdateTextureRegions((FTextureRenderTarget2DResource *)renderTarget->Resource, (uint32)1,
                       VideoUpdateTextureRegion, (uint32)(4 * VideoSizeX), (uint32)4, dataPtr,
                       true);

  return renderTarget;
}

void UCVUMat::UpdateTextureRegions(FTextureRenderTarget2DResource *TextureResource,
                                   uint32 NumRegions, FUpdateTextureRegion2D *Regions,
                                   uint32 SrcPitch, uint32 SrcBpp, uint8 *SrcData, bool bFreeData) {
  struct FUpdateTextureRegionsData {
    FTextureRenderTarget2DResource *Texture2DResource;
    uint32 NumRegions;
    FUpdateTextureRegion2D *Regions;
    uint32 SrcPitch;
    uint32 SrcBpp;
    uint8 *SrcData;
  };

  FUpdateTextureRegionsData *RegionData = new FUpdateTextureRegionsData{
      TextureResource, NumRegions, Regions, SrcPitch, SrcBpp, SrcData};

  ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
      UpdateTextureRegionsData, FUpdateTextureRegionsData *, RegionData, RegionData, bool,
      bFreeData, bFreeData, {
        for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex) {
          RHIUpdateTexture2D(RegionData->Texture2DResource->GetTextureRHI(), 0,
                             RegionData->Regions[RegionIndex], RegionData->SrcPitch,
                             RegionData->SrcData +
                                 RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch +
                                 RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp);
        }
        if (bFreeData) {
          FMemory::Free(RegionData->Regions);
          FMemory::Free(RegionData->SrcData);
        }
        delete RegionData;
      });
}
