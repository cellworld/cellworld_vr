// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "ExperimentServiceMonitor.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FMessage, message);

UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExperimentServiceMonitor();
	//UMessageClient* MessageClient;
	UMessageClient* PredatorMessageClient; 
	UMessageRoute* MessageRoute; 
	FString ServerIP = "127.0.0.1";
	int ServerPort = 6001;
	bool bConnectedToServer = false; 
	float map_length = 5000;
	int n_samples = 0; 

	const FString predator_step_header = "predator_step";
	bool SubscribeToServer(FString header);
	bool ServerConnect();
	void ServerConnectAttempts(int attempts);

	UFUNCTION()
	void UpdatePredator(FMessage message);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;



};
