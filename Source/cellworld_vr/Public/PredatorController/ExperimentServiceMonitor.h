// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "PredatorController/CharacterPredator.h"
#include "PredatorController/ControllerTypes.h"
#include "ExperimentServiceMonitor.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FMessage, message);

UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor
{ 
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExperimentServiceMonitor();

	UMessageClient* PredatorMessageClient; 
	UMessageClient* ExperimentServerClient;
	
	UMessageRoute* ExperimentServerClientRoute;
	UMessageRoute* MessageRoute; 

	URequest* start_episode_request;
	URequest* stop_episode_request;

	const FString ServerIPMessage = "127.0.0.1";
	const int PortMessageServer = 6001;
	const int PortExperimentServer = 6100; 
	bool bConnectedToServer = false; 
	float map_length = 5100;
	int n_samples = 0; 

	const FString predator_step_header = "predator_step";
	bool SubscribeToServer(FString header);
	bool ServerConnect();
	UMessageClient* ServerConnectMessageClient(const FString IP, const int port, bool& result);
	void ServerConnectAttempts(int attempts);
	ACharacterPredator* CharacterPredator;
	bool SpawnAndPossessPredator();

	void HandleExperimentServiceMessage(FMessage message);
	bool SubscribeToExperimentService(FString header);
	bool StartEpisode(const FString experiment);
	bool StopEpisode(const FString experiment);
	URequest* AExperimentServiceMonitor::SendEpisodeRequest(const FString experiment, const FString header);
	bool StopConnection(UMessageClient* Client);
	void EpisodeResponse(const FString response);
	void UpdateOnMessageReceived();
	void EpisodeTimedOut();

	UFUNCTION()
	void UpdatePredator(FMessage message);
	bool UpdatePreyPosition(FMessage message);

	bool bCanUpdatePreyPosition = false;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};