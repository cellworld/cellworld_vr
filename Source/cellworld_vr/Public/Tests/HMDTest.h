// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IXRTrackingSystem.h"
#include "HMDTest.generated.h"

/**
 * 
 */
UCLASS()
class CELLWORLD_VR_API UHMDTest : public UObject
{
	GENERATED_BODY()
public: 
	static bool bHMDEnabled; 
	static FXRHMDData HMDData; 

	static void GetConfidenceValues(); 
	static bool IsHMDEnabled(); 
	static FXRHMDData GetHMDData(); 
	static bool SaveData(FXRHMDData Data); 

	static void test(); 
};
