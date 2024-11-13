// Copyright HTC Corporation. All Rights Reserved.
#pragma once

#include "HAL/Platform.h"
#if PLATFORM_ANDROID
#define XR_USE_PLATFORM_ANDROID 1
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include "OpenXRCore.h"
#include "OpenXRPlatformRHI.h"

#include "IOpenXRHMDModule.h"
#include "IOpenXRExtensionPlugin.h"
