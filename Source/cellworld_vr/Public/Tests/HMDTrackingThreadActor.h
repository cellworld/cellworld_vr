// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Public/HAL/RunnableThread.h"
#include "Core/Public/HAL/Runnable.h"
#include "GameFramework/Actor.h"
#include "IXRTrackingSystem.h"
#include "HMDTrackingThread.h"
#include "HMDTrackingThreadActor.generated.h"

UCLASS()
class CELLWORLD_VR_API AHMDTrackingThreadActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHMDTrackingThreadActor();

protected:

	UWorld* World; 
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	void InitThread();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	class FHMDTrackingThread* WorkerThread = nullptr;
	FRunnableThread* CurrentRunningThread = nullptr;

	bool SaveData(FXRHMDData* Data);
	TQueue<FXRHMDData*> HMDDataQueue; 

	FString SaveDirectory; 
	FString filename; 

	void PrintData();
};
