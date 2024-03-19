// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "PredatorController/CharacterPredator.h"
#include "PredatorController/ControllerTypes.h"
#include "PawnMain.h"
#include "ExperimentServiceMonitor.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FMessage, message);

UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor
{ 
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExperimentServiceMonitor();

	UMessageClient* TrackingServiceClient;
	UMessageClient* ExperimentServiceClient;
	
	UMessageRoute* ExperimentServiceClientRoute;
	UMessageRoute* ExperimentServiceRoute;

	UMessageRoute* MessageRoute; 
	UMessageRoute* TrackingServiceRoute; 

	URequest* start_episode_request;
	URequest* stop_episode_request;

	URequest* TrackingServiceRequest; 

	const FString header_experiment_service = "predator_step";
	const FString header_tracking_service   = "send_step";

	const FString ServerIPMessage   = "127.0.0.1";
	const int PortTrackingService   = 4510;
	const int PortExperimentService = 4540; 
	float map_length                = 5100;
	int frame_count                 = 0; 

	bool bConnectedToServer     = false;
	bool bCanUpdatePreyPosition = false;

	bool bConnectedToExperimentService  = false; 
	bool bCOnnectedToTrackingService    = false; 
	bool bSubscribedToTrackingService   = false;
	bool bRoutedMessagesTrackingService = false; 

	const FString predator_step_header = "predator_step";
	bool SubscribeToTrackingService();
	bool ConnectToTrackingService(); 
	bool TrackingServiceRouteMessages(); 

	bool SubscribeToExperimentServiceServer(FString header);
	ACharacterPredator* CharacterPredator;
	bool SpawnAndPossessPredator();

	//bool SubscribeToExperimentService(FString header);
	bool StartEpisode(const FString experiment);
	bool StopEpisode(const FString experiment);
	bool StopConnection(UMessageClient* Client);
	
	bool TrackingServiceCreateMessageClient();

	bool ConnectTrackingService();
	
	APawnMain* PlayerPawn;
	bool GetPlayerPawn();
	
	/* delegates */

	/* experiment service */
	UFUNCTION()
		void HandleExperimentServiceMessage(FMessage message);
	UFUNCTION()
		void HandleExperimentServiceMessageTimedOut(FMessage message);
	UFUNCTION()
		void HandleExperimentServiceResponse(const FString message);
	UFUNCTION()
		void HandleExperimentServiceResponseTimedOut();

	/* tracking service */
	UFUNCTION()
		void HandleTrackingServiceResponse(const FString message);
	UFUNCTION()
		void HandleTrackingServiceResponseTimedOut();
	
	/* Episode */
	UFUNCTION()
		void EpisodeResponse(const FString response);
	UFUNCTION()
		URequest* SendStartEpisodeRequest(const FString experiment, const FString header);
	UFUNCTION()
		URequest* SendStopEpisodeRequest(const FString experiment, const FString header);
	UFUNCTION()
		void EpisodeTimedOut();
	UFUNCTION()
		void UpdatePredator(FMessage message);
	UFUNCTION()
		void UpdatePreyPosition(FVector Location);

	UFUNCTION()
		void HandleTrackingServiceMessage(FMessage message);
	UFUNCTION()
		void HandleTrackingServiceUnroutedMessage(FMessage message);



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};