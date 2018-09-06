// Fill out your copyright notice in the Description page of Project Settings.

#include "UCVUMat.h"

UCVUMat::UCVUMat() {
	UE_LOG(LogTemp, Verbose, TEXT("Default Constructed"));
};

UCVUMat::UCVUMat(const cv::UMat& m_) : m(m_) {
	UE_LOG(LogTemp, Verbose, TEXT("Constructed w/ cv::Mat constructor"));
};
//UCVMat(const UCVMat& ma) : m(ma.m) { /* */  };

UCVUMat::~UCVUMat() {
	UE_LOG(LogTemp, Verbose, TEXT("Destroyed"));
};



UCVUMat * UCVUMat::createMat(int32 rows, int32 cols, FCVMatType type)
{
	UCVUMat * r = NewObject<UCVUMat>();
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
