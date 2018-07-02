// Fill out your copyright notice in the Description page of Project Settings.

#include "VideoCapture.h"

#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

AVideoCapture::AVideoCapture()
{
	// Set this actor to call Tick() every frame->  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize OpenCV and webcam properties
	CameraID = 0;
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
void AVideoCapture::BeginPlay()
{
	Super::BeginPlay();

	frame = NewObject<UCVMat>(); 

	// Open the stream
	stream = new cv::VideoCapture(CameraID);

	if (stream->isOpened())
	{
		// Initialize stream
		isStreamOpen = true;
		UpdateFrame();
		VideoSize = FVector2D(frame->m.cols, frame->m.rows);
		size = new cv::Size(ResizeDimensions.X, ResizeDimensions.Y);
		VideoTexture = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y);
		VideoTexture->UpdateResource();
		VideoUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, VideoSize.X, VideoSize.Y);

		// Initialize data array
		Data.Init(FColor(0, 0, 0, 255), VideoSize.X * VideoSize.Y);

		// Do first frame
		DoProcessing();
		UpdateTexture();
		OnNextVideoFrame();
	}

}

// Called every frame
void AVideoCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RefreshTimer += DeltaTime;
	if (isStreamOpen && RefreshTimer >= 1.0f / RefreshRate)
	{
		RefreshTimer -= 1.0f / RefreshRate;
		UpdateFrame();
		DoProcessing();
		UpdateTexture();
		OnNextVideoFrame();
	}
}

void AVideoCapture::ChangeOperation()
{
	OperationMode++;
	OperationMode %= 3;
}

void AVideoCapture::UpdateFrame()
{
	if (stream->isOpened())
	{
		stream->read(frame->m);
		if (ShouldResize)
		{
			cv::resize(frame->m, frame->m, *size);
		}
	}
	else {
		isStreamOpen = false;
	}
}

void AVideoCapture::DoProcessing_Implementation()
{
	// TODO: Do any processing with frame here!

	if (OperationMode == 0) {
		// Apply nothing
	}
	else if (OperationMode == 1) {
		// Apply median blur
		cv::Mat src = frame->m.clone();
		cv::medianBlur(src, frame->m, 7);
	}
	else if (OperationMode == 2) {
		cv::Mat src, dst;
		cv::cvtColor(frame->m, src, cv::COLOR_RGB2GRAY);

		int thresh = 50;

		Canny(src, dst, thresh, thresh * 2, 3);
		cv::cvtColor(dst, frame->m, cv::COLOR_GRAY2BGR);
	}

}

void AVideoCapture::UpdateTexture()
{
	if (isStreamOpen && frame->m.data)
	{
		cv::Mat dataWrap(frame->m.size(), CV_8UC4, (void*)Data.GetData());

		// FColor also has a BGRA layout
		cv::cvtColor(frame->m, dataWrap, CV_BGR2BGRA);

		// Update texture 2D
		UpdateTextureRegions(VideoTexture, (int32)0, (uint32)1, VideoUpdateTextureRegion, (uint32)(4 * VideoSize.X), (uint32)4, (uint8*)Data.GetData(), false);
	}
}

void AVideoCapture::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
	}

}

UCVMat * AVideoCapture::createMat(int32 rows, int32 cols, FCVMatType type)
{
	UCVMat * r = NewObject<UCVMat>();
	int cvType = 0;

	switch (type) {
	case FCVMatType::CVT_8UC1: cvType = CV_8UC1;      break;
	case FCVMatType::CVT_16UC1: cvType = CV_16UC1;      break;
	case FCVMatType::CVT_8SC1: cvType = CV_8SC1;      break;
	case FCVMatType::CVT_16SC1: cvType = CV_16SC1;      break;
	case FCVMatType::CVT_32SC1: cvType = CV_32SC1;      break;
	case FCVMatType::CVT_8UC3: cvType = CV_8UC3;      break;
	case FCVMatType::CVT_8UC4: cvType = CV_8UC4;      break;
	}

	// Init if we have a defined type
	if (cvType) {
		r->m.create(rows, cols, cvType);
	}

	return r;
}

void AVideoCapture::gaussianFilter(UCVMat* src, UCVMat* dst, float sigma)
{
	try {
		int ksize = int(3 * sigma);
		if (!(ksize % 2)) ksize++;
		cv::GaussianBlur(src->m, dst->m, cv::Size(ksize, ksize), sigma, sigma);
	}
	catch (cv::Exception& e) {
		UE_LOG(LogTemp, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));

		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White, UTF8_TO_TCHAR(e.what()));
	}
}

void AVideoCapture::medianFilter(UCVMat* src, UCVMat* dst, int32 filterSize)
{
	try {
		cv::medianBlur(src->m, dst->m, filterSize);
	}
	catch (cv::Exception& e) {
		UE_LOG(LogTemp, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));
	}
}

void AVideoCapture::bilateralFilter(UCVMat * src, UCVMat * dst, int32 d, float sigmaColor, float sigmaSpace)
{
	try {
		cv::bilateralFilter(src->m, dst->m, d, sigmaColor, sigmaSpace);
	}
	catch (cv::Exception& e) {
		UE_LOG(LogTemp, Warning, TEXT("Caught OpenCV Exception: %s"), UTF8_TO_TCHAR(e.what()));
	}
}
