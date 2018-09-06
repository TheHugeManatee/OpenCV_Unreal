#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Runtime/Engine/Classes/Engine/Texture2D.h"

#include "Classes/UCVUMat.h"

#include "OpenCV_ImageProc.generated.h"


UCLASS()
class UOpenCV_ImageProcessing : public UObject {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Gaussian Filter"), Category = "OpenCV|ImageProcessing")
	static void gaussianFilter(const UCVUMat* src, UCVUMat* dst, float sigma);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Median Filter"), Category = "OpenCV|ImageProcessing")
	static void medianFilter(const UCVUMat* src, UCVUMat* dst, int32 filterSize);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bilateral Filter"), Category = "OpenCV|ImageProcessing")
	static void bilateralFilter(const UCVUMat* src, UCVUMat* dst, int32 d, float sigmaColor, float sigmaSpace);

};