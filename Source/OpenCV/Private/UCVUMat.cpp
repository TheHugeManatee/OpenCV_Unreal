// Fill out your copyright notice in the Description page of Project Settings.

#include "UCVUMat.h"

#include "OpenCV_Common.h"

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
  auto *r = NewObject<UCVUMat>();
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

  try {
    if (m.cols != renderTarget->SizeX || m.rows != renderTarget->SizeY) {
      if (resize) {
        cv::resize(m, m, cv::Size{renderTarget->SizeX, renderTarget->SizeY});
      } else {
        UE_LOG(OpenCV, Warning, TEXT("Render target size does not match and resize=false!"));
        return nullptr;
      }
    }

    ETextureRenderTargetFormat requiredRTF{ETextureRenderTargetFormat::RTF_R8};
    size_t requiredDataSize{0};
    int requiredConversion{0};
    int requiredDataWrapType{0};
    uint32 targetElementSize{0};

    if (m.type() == CV_8UC1) {
      // convert to R8
      requiredRTF = ETextureRenderTargetFormat::RTF_R8;
      requiredDataSize = m.total() * 1;
      requiredConversion = -1;  // direct copy
      requiredDataWrapType = CV_8UC1;
      targetElementSize = 1;
    } else if (m.type() == CV_8UC3) {
      // convert to BGRA
      requiredRTF = ETextureRenderTargetFormat::RTF_RGBA8;
      requiredDataSize = m.total() * 4;
      requiredConversion = CV_BGR2BGRA;
      requiredDataWrapType = CV_8UC4;
      targetElementSize = 4;
    } else if (m.type() == CV_8UC4) {
      // convert to BGRA
      requiredRTF = ETextureRenderTargetFormat::RTF_RGBA8;
      requiredDataSize = m.total() * 4;
      requiredConversion = -1;  // direct copy
      requiredDataWrapType = CV_8UC4;
      targetElementSize = 4;
    }

    if (!targetElementSize) {
      UE_LOG(OpenCV, Warning, TEXT("It seems that the OpenCV type is not supported!"));
      return nullptr;
    }

    // Reinitialize texture if necessary
    if (!renderTarget || renderTarget->RenderTargetFormat != requiredRTF) {
      renderTarget->RenderTargetFormat = requiredRTF;
      renderTarget->Filter = TextureFilter::TF_Bilinear;
      renderTarget->InitAutoFormat(VideoSizeX, VideoSizeY);
      renderTarget->UpdateResourceImmediate(false);
    }

    auto dataPtr = new uint8[requiredDataSize];
    if (requiredConversion != -1) {
      // wrap a cv::Mat around the buffer
      cv::Mat dataWrap(m.size(), requiredDataWrapType, dataPtr);
      // convert to the right layout (FColor is BGRA)
      cv::cvtColor(m, dataWrap, requiredConversion);

      UpdateTextureRegions((FTextureRenderTarget2DResource *)renderTarget->Resource, (uint32)1,
                           VideoUpdateTextureRegion, (uint32)(targetElementSize * VideoSizeX),
                           targetElementSize, dataPtr, true);
    } else {
      auto mat = m.getMat(cv::ACCESS_READ);
      memcpy(dataPtr, mat.data, requiredDataSize);
      UpdateTextureRegions((FTextureRenderTarget2DResource *)renderTarget->Resource, (uint32)1,
                           VideoUpdateTextureRegion, (uint32)(targetElementSize * VideoSizeX),
                           targetElementSize, dataPtr, true);
    }
  } catch (cv::Exception &e) {
    UE_LOG(OpenCV, Warning, TEXT("Function %s: Caught OpenCV Exception: %s"), TEXT(__FUNCTION__),
           UTF8_TO_TCHAR(e.what()));
  }

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

  auto RegionData = new FUpdateTextureRegionsData{TextureResource, NumRegions, Regions,
                                                  SrcPitch,        SrcBpp,     SrcData};

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
