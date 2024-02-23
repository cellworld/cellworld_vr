// (c) Copyright 2021 HP Development Company, L.P.

#pragma once

#include "Math/Vector.h"

#include "HPGliaIMU.generated.h"

USTRUCT(BlueprintType)
struct FIMU
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "3Axis")
        FVector Accelerometer;

    UPROPERTY(BlueprintReadWrite, Category = "3Axis")
        FVector Gyroscope;
};

USTRUCT(BlueprintType)
struct FIMUArray
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "IMU")
        TArray<FIMU> Imu;
};