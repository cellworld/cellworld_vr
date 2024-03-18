// (c) Copyright 2021 HP Development Company, L.P.

#pragma once

#include "HPGliaSensorInfo.generated.h"

USTRUCT(BlueprintType)
struct FSensorInfo
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Device Sensor Information")
        FString SensorId;

    UPROPERTY(BlueprintReadWrite, Category = "Device Sensor Information")
        FString SensorSubId;

    UPROPERTY(BlueprintReadWrite, Category = "Device Sensor Information")
        FString Location;
};
