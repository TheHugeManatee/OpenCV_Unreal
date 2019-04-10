// (c) 2019 Technical University of Munich
// Jakob Weiss <jakob.weiss@tum.de>

#include "UCVUMat.h"

#include "OpenCV_Common.h"

#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/VolumeTexture.h"

THIRD_PARTY_INCLUDES_START
#include <opencv2/opencv.hpp>
THIRD_PARTY_INCLUDES_END

#define UPDATE_VOLUME_THROUGH_MIPS 1

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

UCVUMat *UCVUMat::CreateMat(int32 rows, int32 cols, FCVMatType type /* = FCVMatType::CVT_EMPTY*/,
                            UCVUMat *existingMat /* = nullptr*/) {
  auto *r = existingMat ? existingMat : NewObject<UCVUMat>();
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

namespace detail {
template <typename TextureResourceType> struct FUpdateTextureRegionsData {
  TextureResourceType *TextureResource;
  uint32 NumRegions;
  FUpdateTextureRegion2D *Regions;
  uint32 SrcPitch;
  uint32 SrcBpp;
  uint8 *SrcData;
};

// Use this function to update the texture rects you want to change:
// NOTE: This is very similar to a in UTexture2D::UpdateTextureRegions but it is compiled
// WITH_EDITOR and is not marked as ENGINE_API so it cannot be linked from plugins.
// Adapted from https://wiki.unrealengine.com/Dynamic_Textures
void UpdateTextureRegions(FTexture2DResource *TextureResource, uint32 NumRegions,
                          FUpdateTextureRegion2D *Regions, uint32 SrcPitch, uint32 SrcBpp,
                          uint8 *SrcData, bool bFreeData) {
  auto RegionData = new FUpdateTextureRegionsData<FTexture2DResource>{
      TextureResource, NumRegions, Regions, SrcPitch, SrcBpp, SrcData};

  ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
      UpdateTextureRegionsData, FUpdateTextureRegionsData<FTexture2DResource> *, RegionData,
      RegionData, bool, bFreeData, bFreeData, {
        for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex) {
          RHIUpdateTexture2D(RegionData->TextureResource->GetTexture2DRHI(), 0,
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

void UpdateTextureRegions(FTextureRenderTarget2DResource *TextureResource, uint32 NumRegions,
                          FUpdateTextureRegion2D *Regions, uint32 SrcPitch, uint32 SrcBpp,
                          uint8 *SrcData, bool bFreeData) {
  auto RegionData = new FUpdateTextureRegionsData<FTextureRenderTarget2DResource>{
      TextureResource, NumRegions, Regions, SrcPitch, SrcBpp, SrcData};

  ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
      UpdateTextureRegionsData, FUpdateTextureRegionsData<FTextureRenderTarget2DResource> *,
      RegionData, RegionData, bool, bFreeData, bFreeData, {
        for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex) {
          RHIUpdateTexture2D(RegionData->TextureResource->GetTextureRHI(), 0,
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

template <typename TextureResourceType>
void ConvertAndUpload(size_t requiredDataSize, int requiredConversion, cv::UMat &m,
                      uint32_t requiredDataWrapType, TextureResourceType *resource,
                      uint32 targetElementSize, uint32 VideoSizeX, uint32 VideoSizeY) {
  auto VideoUpdateTextureRegion = new FUpdateTextureRegion2D{0, 0, 0, 0, VideoSizeX, VideoSizeY};

  auto dataPtr = new uint8[requiredDataSize];
  if (requiredConversion != -1) {
    // wrap a cv::Mat around the buffer
    cv::Mat dataWrap(m.size(), requiredDataWrapType, dataPtr);
    // convert to the right layout (FColor is BGRA)
    cv::cvtColor(m, dataWrap, requiredConversion);

    UpdateTextureRegions(resource, (uint32)1, VideoUpdateTextureRegion,
                         (uint32)(targetElementSize * VideoSizeX), targetElementSize, dataPtr,
                         true);
  } else {
    auto mat = m.getMat(cv::ACCESS_READ);
    memcpy(dataPtr, mat.data, requiredDataSize);
    UpdateTextureRegions(resource, (uint32)1, VideoUpdateTextureRegion,
                         (uint32)(targetElementSize * VideoSizeX), targetElementSize, dataPtr,
                         true);
  }
}
}  // namespace detail

void UCVUMat::ToRenderTarget(UTextureRenderTarget2D *&RenderTarget, bool resize) {
  uint32 VideoSizeX = m.cols;
  uint32 VideoSizeY = m.rows;
  auto size = cv::Size{m.size()};

  try {
    // create a new texture if none was passed in
    if (!RenderTarget) {
      RenderTarget = NewObject<UTextureRenderTarget2D>();
    }

    // For render Targets, we always convert to BGRA as no other reasonable formats are supported
    EPixelFormat requiredPF{EPixelFormat::PF_B8G8R8A8};
    uint32 targetElementSize{4};

    int requiredConversion{-1};  // The OpenCV conversion parameter as used by cv::cvtColor

    switch (m.type()) {
      case CV_8UC1: requiredConversion = CV_GRAY2BGRA; break;
      case CV_8UC3: requiredConversion = CV_BGR2BGRA; break;
      case CV_8UC4:
        requiredConversion = -1;  // direct copy
        break;
      default:
        UE_LOG(OpenCV, Warning, TEXT("It seems that the OpenCV type is not supported!"));
        return;
    }

    // Reinitialize texture if necessary
    if (RenderTarget->GetFormat() != requiredPF) {
      RenderTarget->InitCustomFormat(VideoSizeX, VideoSizeY, requiredPF, true);
      RenderTarget->UpdateResourceImmediate(false);
    }

    // check if we should resize
    if (m.cols != RenderTarget->SizeX || m.rows != RenderTarget->SizeY) {
      if (resize) {
        cv::resize(m, m, cv::Size{RenderTarget->SizeX, RenderTarget->SizeY});
        VideoSizeX = RenderTarget->SizeX;
        VideoSizeY = RenderTarget->SizeY;
      } else {
        RenderTarget->InitCustomFormat(VideoSizeX, VideoSizeY, requiredPF, true);
        RenderTarget->UpdateResourceImmediate(false);
      }
    }

    size_t requiredDataSize{m.total() * targetElementSize};
    uint32_t requiredDataWrapType{CV_8UC(targetElementSize)};
    auto resource = static_cast<FTextureRenderTarget2DResource *>(RenderTarget->Resource);

    detail::ConvertAndUpload(requiredDataSize, requiredConversion, m, requiredDataWrapType,
                             resource, targetElementSize, VideoSizeX, VideoSizeY);

  } catch (cv::Exception &e) {
    UE_LOG(OpenCV, Warning, TEXT("Function %s: Caught OpenCV Exception: %s"), TEXT(__FUNCTION__),
           UTF8_TO_TCHAR(e.what()));
  }
}

void UCVUMat::ToTexture(UTexture2D *&Texture, bool Resize) {
  uint32 VideoSizeX = m.cols;
  uint32 VideoSizeY = m.rows;
  auto size = cv::Size{m.size()};

  if (m.total() == 0) {
    UE_LOG(OpenCV, Error, TEXT("Cannot upload an empty matrix!"));
    return;
  }

  try {
    EPixelFormat requiredPF{EPixelFormat::PF_Unknown};
    int requiredConversion{-1};  // The OpenCV conversion parameter as used by cv::cvtColor
    uint32 targetElementSize{0};

    switch (m.type()) {
      case CV_8UC1:  // convert to R8
        requiredPF = EPixelFormat::PF_G8;
        requiredConversion = -1;  // direct copy
        targetElementSize = 1;
        break;
      case CV_8UC3:  // convert to BGRA
        requiredPF = EPixelFormat::PF_B8G8R8A8;
        requiredConversion = CV_BGR2BGRA;
        targetElementSize = 4;
        break;
      case CV_8UC4:  // convert to BGRA
        requiredPF = EPixelFormat::PF_B8G8R8A8;
        requiredConversion = -1;  // direct copy
        targetElementSize = 4;
        break;
      default:
        UE_LOG(OpenCV, Warning, TEXT("It seems that the OpenCV type is not supported!"));
        return;
        break;
    }

    // create/reinitialize texture if necessary
    if (!Texture || Texture->GetPixelFormat() != requiredPF) {
      Texture = UTexture2D::CreateTransient(VideoSizeX, VideoSizeY, requiredPF);
      Texture->UpdateResource();
    }

    // check if we should resize
    if (m.cols != Texture->GetSizeX() || m.rows != Texture->GetSizeY()) {
      if (Resize) {
        cv::resize(m, m, cv::Size{Texture->GetSizeX(), Texture->GetSizeY()});
        VideoSizeX = Texture->GetSizeX();
        VideoSizeY = Texture->GetSizeY();
      } else {
        UE_LOG(OpenCV, Warning, TEXT("Render target size does not match and resize=false!"));
        return;
      }
    }

    size_t requiredDataSize{m.total() * targetElementSize};
    uint32_t requiredDataWrapType{CV_8UC(targetElementSize)};
    auto resource = static_cast<FTexture2DResource *>(Texture->Resource);

    detail::ConvertAndUpload(requiredDataSize, requiredConversion, m, requiredDataWrapType,
                             resource, targetElementSize, VideoSizeX, VideoSizeY);
  } catch (cv::Exception &e) {
    UE_LOG(OpenCV, Warning, TEXT("Function %s: Caught OpenCV Exception: %s"), TEXT(__FUNCTION__),
           UTF8_TO_TCHAR(e.what()));
  }
}

void UCVUMat::ToVolumeTexture(UVolumeTexture *&VolumeTexture) {
  if (m.total() == 0) {
    UE_LOG(OpenCV, Error, TEXT("Cannot upload an empty matrix!"));
    return;
  }
  if (m.channels() != 1) {
    UE_LOG(OpenCV, Error, TEXT("Channel mismatch: Volume has to be single-channel!"));
    return;
  }
  if (m.dims != 3) {
    UE_LOG(OpenCV, Error, TEXT("Dimensionality mismatch: cv::UMat has to contain a 3D image!"));
    return;
  }
  if (m.elemSize() != 1) {
    UE_LOG(OpenCV, Error, TEXT("Pixel size mismatch: cv::UMat has to be 1 byte per pixel!"));
    return;
  }

  const FIntVector Dimensions{m.size[2], m.size[1], m.size[0]};
  const int TotalSize = Dimensions.X * Dimensions.Y * Dimensions.Z;

  // Create a new texture if none was specified
  if (!VolumeTexture) {
    VolumeTexture = NewObject<UVolumeTexture>();
  }

  FTexture2DMipMap *mip;

  // If existing texture is not suitable, create a new one
  if (VolumeTexture->GetSizeX() != Dimensions.X || VolumeTexture->GetSizeY() != Dimensions.Y ||
      VolumeTexture->GetSizeZ() != Dimensions.Z || VolumeTexture->GetPixelFormat() != PF_G8 ||
      !VolumeTexture->PlatformData || !VolumeTexture->PlatformData->Mips.IsValidIndex(0)) {
    UE_LOG(OpenCV, Warning, TEXT("Created a new texture!"));

    // Set volume texture parameters.

    VolumeTexture->NeverStream = false;
    VolumeTexture->SRGB = false;
    VolumeTexture->bUAVCompatible = true;  // this requires the custom built engine

    // Set PlatformData parameters (create PlatformData if it doesn't exist)
    if (!VolumeTexture->PlatformData) {
      VolumeTexture->PlatformData = new FTexturePlatformData();
    }
    VolumeTexture->PlatformData->PixelFormat = PF_G8;
    VolumeTexture->PlatformData->SizeX = Dimensions.X;
    VolumeTexture->PlatformData->SizeY = Dimensions.Y;
    VolumeTexture->PlatformData->NumSlices = Dimensions.Z;

    // if (inTexture->PlatformData->Mips.IsValidIndex(0)) {
    //  mip = &inTexture->PlatformData->Mips[0];
    //} else
    {
      // If the texture already has MIPs in it, destroy and free them (Empty() calls destructors and
      // frees space).
      if (VolumeTexture->PlatformData->Mips.Num() != 0) {
        VolumeTexture->PlatformData->Mips.Empty();
      }
      mip = new FTexture2DMipMap();
      // Add the new MIP.
      VolumeTexture->PlatformData->Mips.Add(mip);
    }
    mip->SizeX = Dimensions.X;
    mip->SizeY = Dimensions.Y;
    mip->SizeZ = Dimensions.Z;

#if !UPDATE_VOLUME_THROUGH_MIPS
    // Update the resource to make sure the buffer size matches
    VolumeTexture->UpdateResource();
#endif
  } else {
    mip = &VolumeTexture->PlatformData->Mips[0];
  }

#if UPDATE_VOLUME_THROUGH_MIPS
  mip->BulkData.Lock(EBulkDataLockFlags::LOCK_READ_WRITE);

  const int sz[3]{m.size[0], m.size[1], m.size[2]};
  uint8 *ByteArray = (uint8 *)mip->BulkData.Realloc(TotalSize);
  cv::Mat wrap{3, sz, CV_8UC1, ByteArray};
  m.copyTo(wrap);
  mip->BulkData.Unlock();
  VolumeTexture->UpdateResource();
#else
  // This version of updating the texture requires a minor customization of the engine source code
  // to allow access to the Texture3D RHI object, but might be more efficient (not confirmed)
  struct FUpdateTextureRegions3DData {
    FHackedVolumeTextureResource *TextureResource;
    FUpdateTextureRegion3D Region;
    uint32 SrcRowPitch;
    uint32 SrcDepthPitch;
    cv::Mat SrcData;
  };

  auto TextureResource = (FHackedVolumeTextureResource *)VolumeTexture->Resource;

  uint32 SrcRowPitch = static_cast<uint32>(Dimensions.X);
  uint32 SrcDepthPitch = static_cast<uint32>(Dimensions.X * Dimensions.Y);
  auto RegionData = new FUpdateTextureRegions3DData{
      TextureResource,
      FUpdateTextureRegion3D(0, 0, 0, 0, 0, 0, Dimensions.X, Dimensions.Y, Dimensions.Z),
      SrcRowPitch, SrcDepthPitch, m.getMat(cv::ACCESS_READ).clone()};

  ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
      UpdateTextureRegionsData, FUpdateTextureRegions3DData *, RegionData, RegionData, {
        RHIUpdateTexture3D(RegionData->TextureResource->GetTextureRHI(), 0, RegionData->Region,
                           RegionData->SrcRowPitch, RegionData->SrcDepthPitch,
                           RegionData->SrcData.data);

        delete RegionData;
      });
#endif
}

void UCVUMat::FromTexture2D(UTexture2D *Texture, UCVUMat *&Mat) {
  if (!ensure(Texture != nullptr)) {
    UE_LOG(OpenCV, Error, TEXT("The given texture is empty!"));
  }

  if (Mat == nullptr) {
    Mat = NewObject<UCVUMat>();
  }

  if (!ensure(Texture->PlatformData != nullptr && Texture->PlatformData->Mips.Num() > 0)) {
    UE_LOG(OpenCV, Error,
           TEXT("Given texture does not have platform data or does not have mipmaps!"));
    return;
  }

  cv::Mat m = Mat->m.getMat(cv::ACCESS_RW);

  int cvFormat{-1};
  if (Texture->PlatformData->PixelFormat == PF_G8) {
    cvFormat = CV_8UC1;
  } else if (Texture->PlatformData->PixelFormat == PF_B8G8R8A8 ||
             Texture->PlatformData->PixelFormat == PF_R8G8B8A8) {
    cvFormat = CV_8UC4;
  } else if (Texture->PlatformData->PixelFormat == PF_FloatRGB) {
    cvFormat = CV_32FC3;
  } else if (Texture->PlatformData->PixelFormat == PF_FloatRGBA) {
    cvFormat = CV_32FC4;
  }

  if (!ensure(cvFormat != -1)) {
    UE_LOG(OpenCV, Error, TEXT("Given texture has a currently unsupported format!"));
    return;
  }

  // auto &mip = texture->PlatformData->Mips[0];
  // void *memoryBuffer = mip.BulkData.Lock(EBulkDataLockFlags::LOCK_READ_WRITE);

  struct FCopyBufferData {
    UTexture2D *Texture;
    TPromise<void> Promise;
    cv::Mat Mat;
    int32 cvFormat;
  };
  using FCommandDataPtr = TSharedPtr<FCopyBufferData, ESPMode::ThreadSafe>;
  FCommandDataPtr CommandData = MakeShared<FCopyBufferData, ESPMode::ThreadSafe>();
  CommandData->Texture = Texture;
  CommandData->Mat = m;
  CommandData->cvFormat = cvFormat;

  auto Future = CommandData->Promise.GetFuture();

  ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
      CopyTextureToCVMat, FCommandDataPtr, CommandData, CommandData, {
        auto Texture2DRHI = CommandData->Texture->Resource->TextureRHI->GetTexture2D();
        uint32 DestPitch{0};
        uint8 *memoryBuffer = (uint8 *)RHILockTexture2D(
            Texture2DRHI, 0, EResourceLockMode::RLM_ReadOnly, DestPitch, false);

        int SizeX = CommandData->Texture->GetSizeX();
        int SizeY = CommandData->Texture->GetSizeY();

        cv::Mat memoryWrapper(SizeY, SizeX, CommandData->cvFormat, memoryBuffer);
        memoryWrapper.copyTo(CommandData->Mat);
        RHIUnlockTexture2D(Texture2DRHI, 0, false);
        CommandData->Promise.SetValue();
      });

  // wait until render thread operation completes
  Future.Get();

  Mat->m = CommandData->Mat.getUMat(cv::ACCESS_RW);
}

void UCVUMat::FromVolumeTexture(UVolumeTexture *Texture, UCVUMat *&Mat) {
  if (!ensure(Texture != nullptr)) {
    UE_LOG(OpenCV, Error, TEXT("The given texture is empty!"));
  }

  if (Mat == nullptr) {
    Mat = NewObject<UCVUMat>();
  }

  if (!ensure(Texture->PlatformData != nullptr && Texture->PlatformData->Mips.Num() > 0)) {
    UE_LOG(OpenCV, Error,
           TEXT("Given texture does not have platform data or does not have mipmaps!"));
    return;
  }

  cv::Mat m = Mat->m.getMat(cv::ACCESS_RW);

  int cvFormat{-1};
  if (Texture->PlatformData->PixelFormat == PF_G8 || Texture->PlatformData->PixelFormat == PF_A8) {
    cvFormat = CV_8UC1;
  } else if (Texture->PlatformData->PixelFormat == PF_B8G8R8A8 ||
             Texture->PlatformData->PixelFormat == PF_R8G8B8A8) {
    cvFormat = CV_8UC4;
  } else if (Texture->PlatformData->PixelFormat == PF_FloatRGB) {
    cvFormat = CV_32FC3;
  } else if (Texture->PlatformData->PixelFormat == PF_FloatRGBA) {
    cvFormat = CV_32FC4;
  }

  if (!ensure(cvFormat != -1)) {
    UE_LOG(OpenCV, Error, TEXT("Given texture has a currently unsupported format!"));
    return;
  }

  auto &mip = Texture->PlatformData->Mips[0];
  void *memoryBuffer = mip.BulkData.Lock(EBulkDataLockFlags::LOCK_READ_WRITE);

  if (ensure(memoryBuffer != nullptr)) {
    int32 SizeX = Texture->GetSizeX();
    int32 SizeY = Texture->GetSizeY();
    int32 SizeZ = Texture->GetSizeZ();

    cv::Mat memoryWrapper({SizeZ, SizeY, SizeX}, cvFormat, memoryBuffer);
    memoryWrapper.copyTo(m);
  } else {
    UE_LOG(OpenCV, Error,
           TEXT("Could not lock Mip 0. Apparently this texture does not have CPU backing."));
  }
  mip.BulkData.Unlock();

  Mat->m = m.getUMat(cv::ACCESS_RW);
}

//
// void CopyTextureToArray(UTexture2D *Texture, TArray<FColor> &Array) {
//  struct FCopyBufferData {
//    UTexture2D *Texture;
//    TPromise<void> Promise;
//    TArray<FColor> DestBuffer;
//  };
//  using FCommandDataPtr = TSharedPtr<FCopyBufferData, ESPMode::ThreadSafe>;
//  FCommandDataPtr CommandData = MakeShared<FCopyBufferData, ESPMode::ThreadSafe>();
//  CommandData->Texture = Texture;
//  CommandData->DestBuffer.SetNum(Texture->GetSizeX() * Texture->GetSizeY());
//
//  auto Future = CommandData->Promise.GetFuture();
//
//  ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
//      CopyTextureToArray, FCommandDataPtr, CommandData, CommandData, {
//        auto Texture2DRHI = CommandData->Texture->Resource->TextureRHI->GetTexture2D();
//        uint32 DestPitch{0};
//        uint8 *MappedTextureMemory = (uint8 *)RHILockTexture2D(
//            Texture2DRHI, 0, EResourceLockMode::RLM_ReadOnly, DestPitch, false);
//
//        uint32 SizeX = CommandData->Texture->GetSizeX();
//        uint32 SizeY = CommandData->Texture->GetSizeY();
//
//        FMemory::Memcpy(CommandData->DestBuffer.GetData(), MappedTextureMemory,
//                        SizeX * SizeY * sizeof(FColor));
//
//        RHIUnlockTexture2D(Texture2DRHI, 0, false);
//        // signal completion of the operation
//        CommandData->Promise.SetValue();
//      });
//
//  // wait until render thread operation completes
//  Future.Get();
//
//  Array = std::move(CommandData->DestBuffer);
//}
