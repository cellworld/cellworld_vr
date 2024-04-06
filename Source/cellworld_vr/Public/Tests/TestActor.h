// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "TestActor.generated.h"

UCLASS()
class CELLWORLD_VR_API ATestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override; 
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/* HMD functions */
	void GetConfidenceValues();
	bool IsHMDEnabled();
	FXRHMDData GetHMDData();
	bool SaveData(FXRHMDData Data);
	FXRHMDData HMDData;

	/* control */
	bool bStopThread = false;
	FString ProjectDirectory;
	FString filename;
	FString delim = ",";
	uint64 query_count_init; 
	bool bQuery_count_init_valid; 

};
