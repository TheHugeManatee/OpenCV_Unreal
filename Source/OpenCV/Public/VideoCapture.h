// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

#include "Classes/UCVUMat.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "VideoCapture.generated.h"

UCLASS()
class OPENCV_API AVideoCapture : public AActor {
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  AVideoCapture();

  // Called every frame
  virtual void Tick(float DeltaSeconds) override;

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  // Called whenever the texture dimensions/format changes
  void ResetTexture();

public:
  // Blueprint Event called every time the video frame is updated
  UFUNCTION(BlueprintImplementableEvent, Category = "OpenCV|VideoCapture")
  void OnVideoFrameUpdated();

  // Event is signaled whenever the video texture is reset, i.e. when the dimensions are changed
  UFUNCTION(BlueprintImplementableEvent, Category = "OpenCV|VideoCapture")
  void OnVideoTextureReset();

public:
  // The device ID opened by the Video Stream
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OpenCV|VideoCapture")
  int32 CameraID;

  // The video file opened if no video capture is active
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OpenCV|VideoCapture")
  FString VideoFile;

  // The operation that will be applied to every frame
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OpenCV|VideoCapture")
  int32 OperationMode;

  // If the webcam images should be resized every frame
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OpenCV|VideoCapture")
  bool ShouldResize;

  // The targeted resize width and height (width, height)
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OpenCV|VideoCapture")
  FVector2D ResizeDimensions;

  // The rate at which the color data array and video texture is updated (in frames per second)
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OpenCV|VideoCapture")
  float RefreshRate;

  // The refresh timer
  UPROPERTY(BlueprintReadWrite, Category = "OpenCV|VideoCapture")
  float RefreshTimer;

  // OpenCV fields
  UPROPERTY(BlueprintReadWrite, Category = "OpenCV|VideoCapture")
  UCVUMat* frame;

public:
  cv::VideoCapture* stream;
  cv::Size* size;

  // OpenCV prototypes
  void UpdateFrame();

  // TODO: refactor into a BP-callable
  void UpdateTexture();

  // If the stream has succesfully opened yet
  UPROPERTY(BlueprintReadOnly, Category = "OpenCV|VideoCapture")
  bool isStreamOpen;

  // The videos width and height (width, height)
  UPROPERTY(BlueprintReadWrite, Category = "OpenCV|VideoCapture")
  FVector2D VideoSize;

  // The current video frame's corresponding texture
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OpenCV|VideoCapture")
  UTextureRenderTarget2D* RTVideoTexture;

  // The current data array
  UPROPERTY(BlueprintReadOnly, Category = "OpenCV|VideoCapture")
  TArray<FColor> Data;

protected:
};
