// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Exception handling macros
#if _HAS_EXCEPTIONS
#  define CVTRY try
#  define CVCATCH catch
#else
#  define CVTRY
#  define CVCATCH(x) if(false)
#endif

// Declare our own log category
DECLARE_LOG_CATEGORY_EXTERN(OpenCV, Log, All);