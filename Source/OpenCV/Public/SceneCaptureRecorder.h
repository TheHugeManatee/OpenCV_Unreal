// (c) 2019 Technical University of Munich, Jakob Weiss <jakob.weiss@tum.de>, Tomas
// Bartipan<tomas.bartipan@tum.de>

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Classes/Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"

#include <memory>
#include <opencv2/videoio.hpp>

#include "SceneCaptureRecorder.generated.h"

namespace cv {
class VideoWriter;
}

/**
 * A scene capture recorder skeleton. Captures a scene using a SceneCaptureComponent, then downloads
 * the rendered image and writes it to a file using cv::imwrite.
 * This is still pretty barebones and far from a re-usable thing, but should get anyone started who
 * is looking into how to serialize a SceneCapture result to disk.
 *
 * I could not get VideoWriter to work correctly, presumably due to missing codecs and/or some
 * obscure destuctor/lifetime issues (was crashing on cv::VideoWriter destructor on the second
 * frame). Could also be related to the OpenCV/FFMpeg/OpenH264 dlls not being where they should be).
 */
UCLASS()
class OPENCV_API ASceneCaptureRecorder : public AActor {
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ASceneCaptureRecorder();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  virtual void EndPlay(const EEndPlayReason::Type reason) override;

  UPROPERTY(EditAnywhere)
  USceneCaptureComponent2D* SceneCapture;

  UPROPERTY(EditAnywhere)
  UTextureRenderTarget2D* RenderTarget;

  UPROPERTY(EditAnywhere)
  FString OutputVideoFile;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

private:
  // std::unique_ptr<cv::VideoWriter> VideoWriter;
};
