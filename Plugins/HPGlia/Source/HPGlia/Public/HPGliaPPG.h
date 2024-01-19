// (c) Copyright 2021 HP Development Company, L.P.

#pragma once

#include "GenericPlatform/GenericPlatform.h"
#include "HPGliaPPG.generated.h"

USTRUCT(BlueprintType)
struct FPPG
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "PPG")
    TArray<int64> Values;

    UPROPERTY(BlueprintReadWrite, Category = "Timestamp")
    FDateTime Timestamp;
};

USTRUCT(BlueprintType)
struct FPPGArray
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "PPG")
    TArray<FPPG> PpgArray;

    UPROPERTY(BlueprintReadWrite, Category = "Timestamp")
    FDateTime Timestamp;
};