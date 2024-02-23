// (c) Copyright 2021 HP Development Company, L.P.

#pragma once

#include "Engine/Texture2D.h"
#include "HPGliaCameraImage.generated.h"

UENUM(BlueprintType)
enum class EFormatType : uint8
{
    UNKNOWN,
    RGB888,
    YUY2,
    UYVY,
    YVYU,
    YUYV,
    AYUV,
    YV12,
    NV12,
    L8,
    CUSTOM,
};

USTRUCT(BlueprintType)
struct FCameraImage
{
    GENERATED_BODY()
public:

    UPROPERTY(BlueprintReadWrite, Category = "Camera Image")
    EFormatType format;

    UPROPERTY(BlueprintReadWrite, Transient, Category = "Camera Image")
    TArray<uint8> imageData;

    UPROPERTY(BlueprintReadWrite, Category = "Camera Image")
    int32 width;

    UPROPERTY(BlueprintReadWrite, Category = "Camera Image")
    int32 height;

    UPROPERTY(BlueprintReadWrite, Category = "Camera Image")
    int32 frameNumber;

    UPROPERTY(BlueprintReadWrite, Category = "Camera Image")
    float fps;

    UPROPERTY(BlueprintReadWrite, Category = "Camera Image")
    FString location;
};