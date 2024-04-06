// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/GameplayStatics.h" 
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IXRTrackingSystem.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HMDTrackingThreadActor.h"


class FRunnableThread; 
class AHMDTrackingThreadActor; 

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataReceived, FXRHMDData&, Data);

class FHMDTrackingThread : public FRunnable
{
public:
	FHMDTrackingThread(AHMDTrackingThreadActor* InOwnerActor);
	virtual ~FHMDTrackingThread();

	// Called when the game starts or when spawned
	FRunnableThread* Thread = nullptr;
	// FRunnable functions 
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	/* World reference */
	UWorld* World = nullptr; 
	UWorld* GetWorldFromActor(AActor* Actor);
	AHMDTrackingThreadActor* OwnerActor; 

	//UPROPERTY()
	//FOnDataReceived DataReceivedEvent;


	/* HMD functions */
	bool IsHMDEnabled();
	FXRHMDData GetHMDData(); 
	static void GetHMDDataAsync(FXRHMDData& Data);
	FXRHMDData HMDData; 

	/* control */
	bool bStopThread = false;
	bool SaveToString(FXRHMDData Data);
	FString ProjectDirectory;
	FString filename; 
};
