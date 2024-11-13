// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "ViveOpenXRWristTrackerEnums.generated.h"

static const unsigned int kTrackerButtonCount = 2;
UENUM(BlueprintType, Category = "ViveOpenXR|WristTracker")
enum class TrackerButton : uint8
{
	Menu = 0,
	Primary,
};
