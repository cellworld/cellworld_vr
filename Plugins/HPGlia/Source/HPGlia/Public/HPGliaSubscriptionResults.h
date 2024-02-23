// (c) Copyright 2021 HP Development Company, L.P.

#pragma once

#include "UObject/ObjectMacros.h"
#include "HPGliaSubscription.h"
#include "HPGliaSubscriptionResults.generated.h"

UENUM(BlueprintType)
enum class ESubscriptionResultType : uint8
{
    UNKNOWN,
    REJECTED,
    PENDING,
    APPROVED
};

UENUM(BlueprintType)
enum class ESubscriptionResultErrorType : uint8
{
    UNKNOWN_ERROR ,
    SUCCESS_NO_ERROR,
    NOT_LICENSED_ERROR,
    REJECTED_BY_USER_ERROR,
    SPECIFIED_VERSION_NOT_AVAILABLE_ERROR
};


USTRUCT(BlueprintType)
struct FSubscriptionResults
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Subscription Results")
        ESubscriptionResultType result;

    UPROPERTY(BlueprintReadWrite, Category = "Subscription Results")
        ESubscriptionResultErrorType resultError;

    UPROPERTY(BlueprintReadWrite, Category = "Subscription Results")
        FSubscription subscription;
};

USTRUCT(BlueprintType)
struct FSubscriptionResultsArray
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Subscription Results Array")
        TArray<FSubscriptionResults> subscriptionResultsArray;
};
