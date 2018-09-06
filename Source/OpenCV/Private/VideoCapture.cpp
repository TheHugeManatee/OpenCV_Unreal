// Fill out your copyright notice in the Description page of Project Settings.

#include "VideoCapture.h"

#include "Runtime/Core/Public/HAL/Runnable.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"

#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

AVideoCapture::AVideoCapture() {
  // Set this actor to call Tick() every frame->  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  // Initialize OpenCV and webcam properties
  CameraID = 0;
  VideoFile = 0;
  OperationMode = 0;
  RefreshRate = 30;
  isStreamOpen = false;
  VideoSize = FVector2D(0, 0);
  ShouldResize = false;
  ResizeDimensions = FVector2D(320, 240);
  RefreshTimer = 0.0f;
  stream = nullptr;
  size = nullptr;
  frame = nullptr;
}

// Called when the game starts or when spawned
void AVideoCapture::BeginPlay() {
  Super::BeginPlay();

  frame = NewObject<UCVUMat>();

  // Open the stream
  if (CameraID >= 0) {
    stream = new cv::VideoCapture(CameraID);

  } else {
    std::string file = std::string(TCHAR_TO_UTF8(*VideoFile));
    stream = new cv::VideoCapture(file);
  }

  if (stream->isOpened()) {
    // Initialize stream
    isStreamOpen = true;
    UpdateFrame();
    ResetTexture();

    // Initialize data array
    Data.Init(FColor(0, 0, 0, 255), VideoSize.X * VideoSize.Y);

    // Do first frame
    UpdateTexture();
    OnVideoFrameUpdated();
  }
}

void AVideoCapture::ResetTexture() {
  VideoSize = FVector2D(frame->m.cols, frame->m.rows);
  size = new cv::Size(ResizeDimensions.X, ResizeDimensions.Y);

  VideoUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, VideoSize.X, VideoSize.Y);

  if (!RTVideoTexture) {
    RTVideoTexture = NewObject<UTextureRenderTarget2D>();
  }

  RTVideoTexture->InitCustomFormat(VideoSize.X, VideoSize.Y, EPixelFormat::PF_B8G8R8A8, true);
  RTVideoTexture->UpdateResourceImmediate(false);

  OnVideoTextureReset();
}

// Called every frame
void AVideoCapture::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  RefreshTimer += DeltaTime;
  // only update here if we are replaying a video
  if (CameraID < 0 && isStreamOpen && RefreshTimer >= 1.0f / RefreshRate) {
    RefreshTimer -= 1.0f / RefreshRate;
    UpdateFrame();
    UpdateTexture();
    OnVideoFrameUpdated();
  }
}

void AVideoCapture::UpdateFrame() {
  if (stream->isOpened()) {
    stream->read(frame->m);
    if (ShouldResize) {
      cv::resize(frame->m, frame->m, *size);
    }
  } else {
    isStreamOpen = false;
  }
}

void AVideoCapture::UpdateTexture() {
  if (isStreamOpen && !frame->m.empty()) {
    cv::Mat dataWrap(frame->m.size(), CV_8UC4, (void *)Data.GetData());

    // FColor also has a BGRA layout
    cv::cvtColor(frame->m, dataWrap, CV_BGR2BGRA);

    if (RTVideoTexture && RTVideoTexture->Resource) {
      UpdateTextureRegions((FTextureRenderTarget2DResource *)RTVideoTexture->Resource, (uint32)1,
                           VideoUpdateTextureRegion, (uint32)(4 * VideoSize.X), (uint32)4,
                           (uint8 *)Data.GetData(), false);
    }
  }
}

void AVideoCapture::UpdateTextureRegions(FTextureRenderTarget2DResource *TextureResource,
                                         uint32 NumRegions, FUpdateTextureRegion2D *Regions,
                                         uint32 SrcPitch, uint32 SrcBpp, uint8 *SrcData,
                                         bool bFreeData) {
  if (TextureResource) {
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
}
