
#include "OpenCV_ImageProc.h"

#include "OpenCV_Common.h"

#include <opencv2/imgproc.hpp>

void UOpenCV_ImageProcessing::gaussianFilter(const UCVUMat* src, UCVUMat* dst, float sigma) {
  CVTRY {
    int ksize = int(3 * sigma);
    if (!(ksize % 2)) ksize++;
    cv::GaussianBlur(src->m, dst->m, cv::Size(ksize, ksize), sigma, sigma);
  } CVCATCH (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));

    // if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,
    // UTF8_TO_TCHAR(e.what()));
  }
}

void UOpenCV_ImageProcessing::medianFilter(const UCVUMat* src, UCVUMat* dst, int32 filterSize) {
  CVTRY {
    cv::medianBlur(src->m, dst->m, filterSize);
  } CVCATCH (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));
  }
}

void UOpenCV_ImageProcessing::bilateralFilter(const UCVUMat* src, UCVUMat* dst, int32 d,
                                              float sigmaColor, float sigmaSpace) {
  CVTRY {
    cv::bilateralFilter(src->m, dst->m, d, sigmaColor, sigmaSpace);
  } CVCATCH (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));
  }
}
