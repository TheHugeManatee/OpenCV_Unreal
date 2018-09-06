// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <opencv2/core.hpp>

#include "UCVUMat.generated.h"

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

/**
*  wrapper for the cv::Mat class
*/
UCLASS()
class OPENCV_API UCVUMat : public UObject
{
	GENERATED_BODY()
public:
	UCVUMat();
	UCVUMat(const cv::UMat& m_);
	~UCVUMat();

	cv::UMat m;

	UFUNCTION(BlueprintPure)        int32 GetRows() { return m.rows; };
	UFUNCTION(BlueprintPure)        int32 GetCols() { return m.cols; };
	UFUNCTION(BlueprintPure)        int32 GetChannels() { return m.channels(); };


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create UCVUMat"), Category = "OpenCV|Core")
	static UCVUMat* createMat(int32 rows, int32 cols, FCVMatType type = FCVMatType::CVT_EMPTY);
};
