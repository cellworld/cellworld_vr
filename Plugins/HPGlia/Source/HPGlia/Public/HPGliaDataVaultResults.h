// (c) Copyright 2020-2021 HP Development Company, L.P.

#pragma once

#include "UObject/ObjectMacros.h"
#include "HPGliaDataVaultResults.generated.h"

UENUM(BlueprintType)
enum class EResultType : uint8
{
    UNKNOWN,
    RECORDING_STARTED,
    RECORDING_START_REJECTED,
    RECORDING_STOPPED,
    RECORDING_STOP_REJECTED,
    RECORDING_ERROR
};

UENUM(BlueprintType)
enum class EResultErrorType : uint8
{
    UNKNOWN_ERROR ,
    SUCCESS_NO_ERROR,
    INVALID_LICENSE_ERROR,
    CONNECTION_FAILED_ERROR,
    CONNECTION_NEGOTIATING_ERROR,
    RECORDING_HAS_ALREADY_STARTED_ERROR,
    RECORDING_HAS_NOT_STARTED_ERROR,
    RECORDING_STOP_NO_PERMISSON_ERROR
};


USTRUCT(BlueprintType)
struct FDataVaultResults
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Data Vault Results Type")
        EResultType result;

    UPROPERTY(BlueprintReadWrite, Category = "Data Vault Results Type")
        EResultErrorType resultError;

    UPROPERTY(BlueprintReadWrite, Category = "Data Vault Results Type")
        FString sessionId;
};
