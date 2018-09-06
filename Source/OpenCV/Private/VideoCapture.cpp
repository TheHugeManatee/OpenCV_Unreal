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
	VideoFile = 0;
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

	frame = NewObject<UCVUMat>(); 

	// Open the stream
	if (CameraID >= 0) {
		stream = new cv::VideoCapture(CameraID);
	}
	else {
		std::string file = std::string(TCHAR_TO_UTF8(*VideoFile));
		stream = new cv::VideoCapture(file);
	}

	if (stream->isOpened())
	{
		// Initialize stream
		isStreamOpen = true;
		UpdateFrame();
		ResetTexture();


		// Initialize data array
		Data.Init(FColor(0, 0, 0, 255), VideoSize.X * VideoSize.Y);

		// Do first frame
		UpdateTexture();
		OnVideoFrameUpdated();
	}

}

void AVideoCapture::ResetTexture()
{
	VideoSize = FVector2D(frame->m.cols, frame->m.rows);
	size = new cv::Size(ResizeDimensions.X, ResizeDimensions.Y);
	VideoTexture = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y);
	VideoTexture->UpdateResource();
	VideoUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, VideoSize.X, VideoSize.Y);

	if (!RTVideoTexture) {
		RTVideoTexture = NewObject<UTextureRenderTarget2D>();
	}

	RTVideoTexture->InitCustomFormat(VideoSize.X, VideoSize.Y, EPixelFormat::PF_B8G8R8A8, true);
	RTVideoTexture->UpdateResource();
	RTVideoTexture->UpdateResourceImmediate(false);

	OnVideoTextureReset();

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
		UpdateTexture();
		OnVideoFrameUpdated();
	}
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


void AVideoCapture::UpdateTexture()
{
	if (isStreamOpen && !frame->m.empty())
	{
		cv::Mat dataWrap(frame->m.size(), CV_8UC4, (void*)Data.GetData());

		// FColor also has a BGRA layout
		cv::cvtColor(frame->m, dataWrap, CV_BGR2BGRA);

		if (VideoTexture && VideoTexture->Resource) {
			// Update texture 2D
			UpdateTextureRegions(VideoTexture->Resource, (int32)0, (uint32)1, VideoUpdateTextureRegion, (uint32)(4 * VideoSize.X), (uint32)4, (uint8*)Data.GetData(), false);

			//UpdateTextureRegions(RTVideoTexture->Resource, (int32)0, (uint32)1, VideoUpdateTextureRegion, (uint32)(4 * VideoSize.X), (uint32)4, (uint8*)Data.GetData(), false);
		}
	}
}

void AVideoCapture::UpdateTextureRegions(FTextureResource* TextureResource, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (TextureResource)
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

		RegionData->Texture2DResource = (FTexture2DResource*)TextureResource;
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
