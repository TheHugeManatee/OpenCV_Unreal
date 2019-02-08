// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include <opencv2/core.hpp>
THIRD_PARTY_INCLUDES_END

#include "UCVUMat.generated.h"

class FTextureRenderTarget2DResource;
class FTexture2DResource;
struct FUpdateTextureRegion2D;
class UTextureRenderTarget2D;
class UTexture2D;
class UVolumeTexture;

UENUM(BlueprintType)  //"BlueprintType" is essential to include
enum class FCVMatType : uint8 {
  CVT_EMPTY UMETA(DisplayName = "Empty"),
  CVT_8UC1 UMETA(DisplayName = "CV_8UC1"),
  CVT_16UC1 UMETA(DisplayName = "CV_16UC1"),
  CVT_8SC1 UMETA(DisplayName = "CV_8SC1"),
  CVT_16SC1 UMETA(DisplayName = "CV_16SC1"),
  CVT_32SC1 UMETA(DisplayName = "CV_32SC1"),
  CVT_8UC3 UMETA(DisplayName = "CV_8UC3"),
  CVT_8UC4 UMETA(DisplayName = "CV_8UC4"),
  CVT_UNKNOWN UMETA(DisplayName = "Unknown"),
};

/**
 *  wrapper for the cv::Mat class
 */
UCLASS(BlueprintType)
class OPENCV_API UCVUMat : public UObject {
  GENERATED_BODY()
public:
  UCVUMat();
  UCVUMat(const cv::UMat& m_);
  ~UCVUMat();

  cv::UMat m;

  UFUNCTION(BlueprintPure) int32 GetRows() { return m.rows; };
  UFUNCTION(BlueprintPure) int32 GetCols() { return m.cols; };
  UFUNCTION(BlueprintPure) int32 GetChannels() { return m.channels(); };
  UFUNCTION(BlueprintPure)
  FIntVector GetSize() { return {m.dims == 3 ? m.size[2] : 1, m.size[1], m.size[0]}; };

  /**
   * Create or resize a matrix. Leave existingMat empty to create a new UMat.
   * If existingMat is not empty, it will be resized to the specified dimensions (if necessary)
   */
  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create CVUMat"), Category = "OpenCV|Core")
  static UCVUMat* createMat(int32 rows, int32 cols, FCVMatType type = FCVMatType::CVT_EMPTY,
                            UCVUMat* existingMat = nullptr);

  /**
   * Convert/upload the UCVMat to the render target.
   * If no renderTarget is given, a new one will be created. If resize = true, will resize the
   * UCVMat to the dimensions of the renderTarget
   */
  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Copy CVUMat to Render Target"),
            Category = "OpenCV|Core")
  UTextureRenderTarget2D* toRenderTarget(UTextureRenderTarget2D* renderTarget, bool resize);

  /**
   * Convert/upload the UCVMat to a texture.
   * If no renderTarget is given, a new one will be created. If resize = true, will resize the
   * UCVMat to the dimensions of the renderTarget
   */
  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Copy CVUMat to Texture2D"),
            Category = "OpenCV|Core")
  UTexture2D* toTexture(UTexture2D* texture, bool resize);

  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Copy CVUMat to Texture3D"),
            Category = "OpenCV|Core")
  UVolumeTexture* to3DTexture(UVolumeTexture* volumeTexture);

private:
  //// Use this function to update the texture rects you want to change:
  //// NOTE: This is very similar to a in UTexture2D::UpdateTextureRegions but it is compiled
  //// WITH_EDITOR and is not marked as ENGINE_API so it cannot be linked from plugins.
  //// Adapted from https://wiki.unrealengine.com/Dynamic_Textures
  // static void UpdateTextureRegions(FTextureRenderTarget2DResource* TextureResource,
  //                                 uint32 NumRegions, FUpdateTextureRegion2D* Regions,
  //                                 uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool
  //                                 bFreeData);

  // Pointer to update texture region 2D struct
  // FUpdateTextureRegion2D* VideoUpdateTextureRegion;
};
