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
  size = new cv::Size(ResizeDimensions.X, ResizeDimensions.Y);

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

    // Do first frame
    UpdateTexture();
    OnVideoFrameUpdated();
  }
}

void AVideoCapture::ResetTexture() {
  VideoSize = FVector2D(frame->m.cols, frame->m.rows);

  if (RTVideoTexture) {
    // RTVideoTexture = NewObject<UTextureRenderTarget2D>();
    RTVideoTexture->InitCustomFormat(VideoSize.X, VideoSize.Y, EPixelFormat::PF_B8G8R8A8, true);
    RTVideoTexture->UpdateResourceImmediate(false);
  }

  OnVideoTextureReset();
}

// Called every frame
void AVideoCapture::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  RefreshTimer += DeltaTime;

  if (isStreamOpen && RefreshTimer >= 1.0f / RefreshRate) {
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
    if (RTVideoTexture) {
      frame->toRenderTarget(RTVideoTexture, false);
    }
  }
}
