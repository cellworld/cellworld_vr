// (c) Copyright 2020-2021 HP Development Company, L.P.

#pragma once

#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
    enum class ELicensingModel : uint8
{
    CORE = 1,
    TRIAL = 0,
    ENTERPRISE = 2,
    REV_SHARE = 3
};
