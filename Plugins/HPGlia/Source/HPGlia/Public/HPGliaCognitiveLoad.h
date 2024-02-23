// (c) Copyright 2019-2020 HP Development Company, L.P.

#pragma once

#include "HPGliaCognitiveLoad.generated.h"

USTRUCT(BlueprintType)
struct FCognitiveLoad
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Cognitive Load")
    float CognitiveLoad;

    UPROPERTY(BlueprintReadWrite, Category = "Cognitive Load")
    float StandardDeviation;

    UPROPERTY(BlueprintReadWrite, Category = "Cognitive Load")
    FString DataState;
    
    UPROPERTY(BlueprintReadWrite, Category = "Data Collect Time")
    FDateTime StartDataCollectTime;
    
    UPROPERTY(BlueprintReadWrite, Category = "Data Collect Time")
    FDateTime EndDataCollectTime;
};