// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QueryPerformanceCounter.generated.h"

/**
 *
 */
UCLASS()
class CELLWORLD_VR_API UQueryPerformanceCounter : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()
	static bool GetQueryElapsedCounter(int32 QueryInit, int32& ElapsedTime);
	static bool GetQueryElapsedTime(uint64 t0, double& t1);
	static bool GetQueryFrequency(uint64& frequency);
	static bool GetCounter2(uint64& Counter);
};