// (c) Copyright 2020-2021 HP Development Company, L.P.

#pragma once

#include <omnicept/lib-abi-cpp/LicensingModel.h>
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum class ELicensingModel : uint8
{
    CORE = 1 UMETA(ToolTip = "No credentials needed. HRV and CL will not work with Core licenses."),
    TRIAL = 0 UMETA(DisplayName = "[Deprecated] TRIAL", Hidden),
    ENTERPRISE = 2 UMETA(DisplayName = "[Deprecated] ENTERPRISE", Hidden),
    REV_SHARE = 3 UMETA(DisplayName = "DEVELOPER")
};
