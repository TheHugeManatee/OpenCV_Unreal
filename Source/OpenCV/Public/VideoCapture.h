// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Runtime/Engine/Classes/Engine/Texture2D.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "VideoCapture.generated.h"

/**
*  wrapper for the cv::Mat class
*/
UCLASS()
class OPENCV_API UCVMat : public UObject {
	GENERATED_BODY()
public:
	UCVMat() {
		UE_LOG(LogTemp, Verbose, TEXT("Default Constructed"));
	};
	UCVMat(const cv::Mat& m_) : m(m_) {
		UE_LOG(LogTemp, Verbose, TEXT("Constructed w/ cv::Mat constructor"));
	};
	//UCVMat(const UCVMat& ma) : m(ma.m) { /* */  };

	~UCVMat() {
		UE_LOG(LogTemp, Verbose, TEXT("Destroyed"));
	};
	cv::Mat m;

	UFUNCTION(BlueprintPure)        int32 GetRows() { return m.rows; };
	UFUNCTION(BlueprintPure)        int32 GetCols() { return m.cols; };
	UFUNCTION(BlueprintPure)        int32 GetChannels() { return m.channels(); };
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class FCVMatType : uint8
{
	CVT_EMPTY   UMETA(DisplayName = "Empty"),
	CVT_8UC1 	UMETA(DisplayName = "CV_8UC1"),
	CVT_16UC1 	UMETA(DisplayName = "CV_16UC1"),
	CVT_8SC1 	UMETA(DisplayName = "CV_8SC1"),
	CVT_16SC1 	UMETA(DisplayName = "CV_16SC1"),
	CVT_32SC1 	UMETA(DisplayName = "CV_32SC1"),
	CVT_8UC3 	UMETA(DisplayName = "CV_8UC3"),
	CVT_8UC4 	UMETA(DisplayName = "CV_8UC4"),
	CVT_UNKNOWN UMETA(DisplayName = "Unknown"),
};



UCLASS()
class OPENCV_API AVideoCapture : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVideoCapture();

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Blueprint Event called every time the video frame is updated
	UFUNCTION(BlueprintImplementableEvent, Category = Webcam)
		void OnNextVideoFrame();

	// Change OpenCV operation that will be applied to every frame
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Change Operations", Keywords = "Change Operation"), Category = Webcam)
		void ChangeOperation();

public:
	// The device ID opened by the Video Stream
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		int32 CameraID;

	// The operation that will be applied to every frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		int32 OperationMode;

	// If the webcam images should be resized every frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		bool ShouldResize;

	// The targeted resize width and height (width, height)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		FVector2D ResizeDimensions;

	// The rate at which the color data array and video texture is updated (in frames per second)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		float RefreshRate;

	// The refresh timer
	UPROPERTY(BlueprintReadWrite, Category = Webcam)
		float RefreshTimer;


	// OpenCV fields
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		UCVMat* frame;

public:
	cv::VideoCapture *stream;
	cv::Size *size;

	// OpenCV prototypes
	void UpdateFrame();

	UFUNCTION(BlueprintNativeEvent, Category = Webcam)
		void DoProcessing();
	void DoProcessing_Implementation();


	void UpdateTexture();

	// If the stream has succesfully opened yet
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
		bool isStreamOpen;

	// The videos width and height (width, height)
	UPROPERTY(BlueprintReadWrite, Category = Webcam)
		FVector2D VideoSize;

	// The current video frame's corresponding texture
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
		UTexture2D* VideoTexture;

	// The current data array
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
		TArray<FColor> Data;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create UCVMat"), Category = "CV|ImageProcessing")
		static UCVMat* createMat(int32 rows, int32 cols, FCVMatType type = FCVMatType::CVT_EMPTY);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Gaussian Filter"), Category = "CV|ImageProcessing")
		static void gaussianFilter(UCVMat* src, UCVMat* dst, float sigma);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Median Filter"), Category = "CV|ImageProcessing")
		static void medianFilter(UCVMat* src, UCVMat* dst, int32 filterSize);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bilateral Filter"), Category = "CV|ImageProcessing")
		static void bilateralFilter(UCVMat* src, UCVMat* dst, int32 d, float sigmaColor, float sigmaSpace);


protected:

	// Use this function to update the texture rects you want to change:
	// NOTE: There is a method called UpdateTextureRegions in UTexture2D but it is compiled WITH_EDITOR and is not marked as ENGINE_API so it cannot be linked
	// from plugins.
	// FROM: https://wiki.unrealengine.com/Dynamic_Textures
	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);

	// Pointer to update texture region 2D struct
	FUpdateTextureRegion2D* VideoUpdateTextureRegion;


	
	
};
