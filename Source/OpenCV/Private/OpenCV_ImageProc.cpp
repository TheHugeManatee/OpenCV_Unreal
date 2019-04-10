// (c) 2019 Technical University of Munich
// Jakob Weiss <jakob.weiss@tum.de>

#include "OpenCV_ImageProc.h"

#include "OpenCV_Common.h"

THIRD_PARTY_INCLUDES_START
#include <opencv2/imgproc.hpp>
THIRD_PARTY_INCLUDES_END

UCVUMat* UOpenCV_ImageProcessing::gaussianFilter(const UCVUMat* src, UCVUMat* dst, float sigma) {
  try {
    int ksize = int(3 * sigma);
    if (!(ksize % 2)) ksize++;
    cv::GaussianBlur(src->m, dst->m, cv::Size(ksize, ksize), sigma, sigma);
  } catch (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Function %s: Caught OpenCV Exception: %s"), TEXT(__FUNCTION__),
           UTF8_TO_TCHAR(e.what()));

    // if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,
    // UTF8_TO_TCHAR(e.what()));
  }
  return dst;
}

UCVUMat* UOpenCV_ImageProcessing::medianFilter(const UCVUMat* src, UCVUMat* dst, int32 filterSize) {
  try {
    cv::medianBlur(src->m, dst->m, filterSize);
  } catch (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Function %s: Caught OpenCV Exception: %s"), TEXT(__FUNCTION__),
           UTF8_TO_TCHAR(e.what()));
  }
  return dst;
}

UCVUMat* UOpenCV_ImageProcessing::bilateralFilter(const UCVUMat* src, UCVUMat* dst, int32 d,
                                                  float sigmaColor, float sigmaSpace) {
  try {
    cv::bilateralFilter(src->m, dst->m, d, sigmaColor, sigmaSpace);
  } catch (cv::Exception& e) {
    UE_LOG(OpenCV, Warning, TEXT("Function %s: Caught OpenCV Exception: %s"), TEXT(__FUNCTION__),
           UTF8_TO_TCHAR(e.what()));
  }
  return dst;
}
