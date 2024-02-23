// (c) Copyright 2021 HP Development Company, L.P.

#pragma once

#include "UObject/ObjectMacros.h"
#include "HPGliaSensorInfo.h"
#include "HPGliaSubscription.generated.h"

UENUM(BlueprintType)
enum class ESubscribeableMsgType : uint8
{
    None,
    EyeTracking,
    CognitiveLoad,
    HeartRateVariability,
    CameraImage,
    IMUArray,
    PPGArray,
    HeartRate,
    DataVaultResult
};

USTRUCT(BlueprintType)
struct FSubscription
{
    GENERATED_USTRUCT_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Subscription Settings")
        ESubscribeableMsgType MessageType;

    UPROPERTY(BlueprintReadWrite, Category = "Subscription Settings")
        FString Version;

    UPROPERTY(BlueprintReadWrite, Category = "Subscription Settings")
        FSensorInfo SensorInfo;

    UPROPERTY(BlueprintReadWrite, Category = "Subscription Settings")
        FString Sender;
};