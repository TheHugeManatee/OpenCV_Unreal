
#include "OpenCV_ImageProc.h"

#include <opencv2/imgproc.hpp>

void UOpenCV_ImageProcessing::gaussianFilter(const UCVUMat* src, UCVUMat* dst, float sigma) {
  try {
    int ksize = int(3 * sigma);
    if (!(ksize % 2)) ksize++;
    cv::GaussianBlur(src->m, dst->m, cv::Size(ksize, ksize), sigma, sigma);
  } catch (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));

    // if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,
    // UTF8_TO_TCHAR(e.what()));
  }
}

void UOpenCV_ImageProcessing::medianFilter(const UCVUMat* src, UCVUMat* dst, int32 filterSize) {
  try {
    cv::medianBlur(src->m, dst->m, filterSize);
  } catch (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));
  }
}

void UOpenCV_ImageProcessing::bilateralFilter(const UCVUMat* src, UCVUMat* dst, int32 d,
                                              float sigmaColor, float sigmaSpace) {
  try {
    cv::bilateralFilter(src->m, dst->m, d, sigmaColor, sigmaSpace);
  } catch (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));
  }
}
