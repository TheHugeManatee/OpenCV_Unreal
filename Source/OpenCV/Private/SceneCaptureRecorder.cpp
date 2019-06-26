// (c) 2019 Technical University of Munich, Jakob Weiss <jakob.weiss@tum.de>, Tomas
// Bartipan<tomas.bartipan@tum.de>

#include "SceneCaptureRecorder.h"

#include "Classes/Engine/Texture2D.h"
#include "Classes/UCVUMat.h"
#include "UObject/GCObjectScopeGuard.h"

#include <opencv2/imgcodecs.hpp>

// Sets default values
ASceneCaptureRecorder::ASceneCaptureRecorder() {
  PrimaryActorTick.bCanEverTick = true;

  RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("Recorder RenderTarget"));
  RenderTarget->InitCustomFormat(512, 512, EPixelFormat::PF_B8G8R8A8, true);
  RenderTarget->UpdateResourceImmediate();

  SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
  SceneCapture->CaptureSource = SCS_SceneColorSceneDepth;
  SceneCapture->bCaptureEveryFrame = false;

  SceneCapture->TextureTarget = RenderTarget;
  // sceneCapture->SetupAttachment(OurCamera);
}

// Called when the game starts or when spawned
void ASceneCaptureRecorder::BeginPlay() {
  Super::BeginPlay();
}

void ASceneCaptureRecorder::EndPlay(const EEndPlayReason::Type reason) {
  // VideoWriter.reset();
}

// Called every frame
void ASceneCaptureRecorder::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (OutputVideoFile.IsEmpty()) return;

  if (RenderTarget == nullptr) {
    RenderTarget = NewObject<UTextureRenderTarget2D>();
    RenderTarget->InitCustomFormat(512, 512, EPixelFormat::PF_B8G8R8A8, true);
    RenderTarget->UpdateResourceImmediate();
  }

  SceneCapture->TextureTarget = RenderTarget;
  SceneCapture->CaptureScene();

  auto RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

  if (RenderTargetResource) {
    TArray<FColor> buffer;
    RenderTargetResource->ReadPixels(buffer);

    cv::Mat wrappedImage(RenderTarget->GetSurfaceHeight(), RenderTarget->GetSurfaceWidth(), CV_8UC4,
                         buffer.GetData());

    std::string OutputFile(TCHAR_TO_UTF8(*OutputVideoFile));
    //// try to initialize the video writer
    // if (!VideoWriter || !VideoWriter->isOpened()) {
    //  try {

    //    char fcc[5] = "H264";
    //    int fourCC = (((fcc[0]) & 255) + (((fcc[1]) & 255) << 8) + (((fcc[3]) & 255) << 16) +
    //                  (((fcc[4]) & 255) << 24));

    //    VideoWriter =
    //        std::make_unique<cv::VideoWriter>(OutputFile, fourCC, 30.0,
    //        cv::Size(wrappedImage.cols, wrappedImage.rows));
    //  } catch (...) {
    //    UE_LOG(OpenCV, Warning,
    //           TEXT("Error creating OpenCV Video Writer. Make sure you have appropriate codecs "
    //                "and ffmpeg dll on the path / close to opencv dll."));
    //  }
    //}

    // if (VideoWriter && VideoWriter->isOpened()) {
    //  *VideoWriter << wrappedImage;
    //}
    cv::imwrite(OutputFile, wrappedImage);
  }
}
