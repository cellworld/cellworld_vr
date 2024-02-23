// (c) Copyright 2020 HP Development Company, L.P.

#pragma once

#include "HPGliaHeartRateVariability.generated.h"

USTRUCT(BlueprintType)
struct FHeartRateVariability
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Heart Rate Variability")
    float sdnn;

    UPROPERTY(BlueprintReadWrite, Category = "Heart Rate Variability")
    float rmssd;
};
