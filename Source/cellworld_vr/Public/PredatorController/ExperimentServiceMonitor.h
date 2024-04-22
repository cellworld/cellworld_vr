// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "ExperimentUtils.h"
#include "PredatorController/CharacterPredator.h"
//#include "PredatorController/ControllerTypes.h"
#include "PawnMain.h"
#include "PawnDebug.h"
#include "ExperimentComponents/Occlusion.h"

#include "DrawDebugHelpers.h"

#include "ExperimentServiceMonitor.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FMessage, message);

USTRUCT()
struct FOcclusions
{
	GENERATED_USTRUCT_BODY()
public:

	FOcclusions() { OcclusionAllArr = {}; }

	bool bLocationsLoaded = false; 
	bool bCurrentLocationsLoaded = false; 
	TArray<AOcclusion*> OcclusionAllArr;
	TArray<AOcclusion*> CurrentVisibleArr; 

	TArray<FLocation> OcclusionAllLocationsArr;
	TArray<int32> OcclusionIDsIntArr;

	/* canonical */
	void SetAllLocations(const TArray<FLocation> LocationsIn) {
		OcclusionAllLocationsArr = LocationsIn; 
		bLocationsLoaded = true;
	}

	void SetCurrentLocationsByIndex(TArray<int32> OcclusionIndexIn) {
		OcclusionIDsIntArr = OcclusionIndexIn; 
		bCurrentLocationsLoaded = true;
	}

	bool SpawnAll(UWorld* WorldRefIn, const bool bVisibilityIn, const bool bEnableCollisonIn, const FVector WorldScaleVecIn) {
		if (OcclusionAllLocationsArr.Num() < 1) { UE_LOG(LogTemp, Error, TEXT("FOcclusions.SpawnAll() Failed. OcclusionAllLocationsArr is empty."));  return false; }
		if (!WorldRefIn) { UE_LOG(LogTemp, Error, TEXT("[FOcclusions.SpawnAll()] Failed due to invalid UWorld object.")); return false; }
		FRotator Rotation(0.0f, 0.0f, 0.0f); // Desired spawn rotation
		const float ScaleOffset = 0.99157164105; 
		const float MapLength = 235.185290;
		FActorSpawnParameters SpawnParams;
		for (int i = 0; i < OcclusionAllLocationsArr.Num(); i++) {
			//const float draw_duration = 10000; // same as tick, update per frame
			//const uint8 depth = 10; // to do: make this vary with pupil dilation and openess
			//const float radius = 1.0f;
			//const int segments = 5;
			//const uint8 depth_priority = 1;
			//const float thickness = 12.0f;

			//FVector temp = UExperimentUtils::CanonicalToVr(OcclusionAllLocationsArr[i], MapLength, 15);
			//DrawDebugSphere
			//(
			//	WorldRefIn,
			//	temp,
			//	2.0f,
			//	5,
			//	FColor::Red,
			//	false,
			//	0.1f,
			//	depth_priority,
			//	thickness
			//);
			AOcclusion* MyMeshActor = WorldRefIn->SpawnActor<AOcclusion>(
				AOcclusion::StaticClass(),
				UExperimentUtils::CanonicalToVr(OcclusionAllLocationsArr[i], MapLength, 15*ScaleOffset), // todo: make scale dynamic
				Rotation,
				SpawnParams);
			 
			MyMeshActor->SetActorScale3D(FVector(15* ScaleOffset,15*ScaleOffset, 15)); // todo: change to WorldScaleVecIn
			MyMeshActor->SetActorHiddenInGame(bVisibilityIn);
			MyMeshActor->SetActorEnableCollision(bEnableCollisonIn);

			OcclusionAllArr.Add(MyMeshActor);
			UE_LOG(LogTemp, Warning, TEXT("Spawned occlusion (%i/%i)."), i, 50);
		}

		return true;
	}

	void SetVisibilityAll(const bool bVisibilityIn) {
		for (AOcclusion* Occlusion : OcclusionAllArr) {
			Occlusion->SetActorHiddenInGame(true);
			Occlusion->SetActorEnableCollision(false);
		}
	}

	void SetVisibilityArr(const TArray<int32> IndexArray) {
		for (int i = 0; i < IndexArray.Num(); i++) {
			OcclusionAllArr[IndexArray[i]]->SetActorHiddenInGame(false);
			OcclusionAllArr[IndexArray[i]]->SetActorEnableCollision(true);
			UE_LOG(LogTemp, Log, TEXT("SetVisibilityArr (%i)"), i);
		}
	}

	void SetCurrentVisibility(const bool bVisibilityIn, const bool bEnableCollisonIn) {
		for (int i = 0; i < OcclusionIDsIntArr.Num(); i++) {
			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorHiddenInGame(bVisibilityIn);
			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorEnableCollision(bEnableCollisonIn);
			UE_LOG(LogTemp, Log, TEXT("SetCirrentVisibility (%i)"), i);
		}
	}
};

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
	UMessageRoute* ExperimentServicePreyRoute;

	UMessageRoute* MessageRoute; 
	UMessageRoute* TrackingServiceRoute; 
	UMessageRoute* TrackingServiceRoutePrey; 
	UMessageRoute* TrackingServiceRoutePredator; 

	URequest* start_episode_request;
	URequest* stop_episode_request;

	URequest* StartExperimentRequest;
	URequest* stop_experiment_request;

	URequest* StartEpisodeRequest;

	URequest* TrackingServiceRequest; 

	//const FString header_experiment_service			= "predator_step";
	const FString header_tracking_service			= "send_step";
	const FString header_tracking_service_prey		= "prey_step";
	const FString header_tracking_service_predator  = "predator_step";
	const FString experiment_name = "test_experiment";

	/* server stuff */
	const FString ServerIPMessage   = "127.0.0.1";
	const int PortTrackingService   = 4510;
	const int PortExperimentService = 4540; 
	bool bConnectedToServer     = false;
	bool bCanUpdatePreyPosition = false;

	/* world stuff */
	float map_length                = 5100;
	int frame_count                 = 0; 

	/* coordinate system stuff */
	const float MapLength = 235.185;
	int WorldScale = 15; 

	/* connection control */
	bool bConnectedToExperimentService  = false; 
	bool bConnectedToTrackingService    = false; 
	
	/* subscription control */
	bool bSubscribedToTrackingService   = false;
	bool bRoutedMessagesTrackingService = false; 

	/* setup */
	const FString predator_step_header = "predator_step";

	bool SubscribeToTrackingService();
	bool ConnectToTrackingService(); 
	bool TrackingServiceRouteMessages(); 

	bool SubscribeToExperimentServiceServer(FString header);

	ACharacterPredator* CharacterPredator;
	bool SpawnAndPossessPredator();

	//bool SubscribeToExperimentService(FString header);
	bool StartExperiment(const FString ExperimentNameIn);
	bool StopExperiment(const FString ExperimentNameIn);
	bool StopConnection(UMessageClient* Client);
	bool DisconnectAll();
	bool DisconnectClients(); 
	/* will be used in BP to called by door opening (start episode)*/
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode();
	
		
	bool TrackingServiceCreateMessageClient();
	bool GetPlayerPawn();

	/* abort if anything goes wrong */
	void SelfDestruct(const FString ErrorMessageIn);
	
	/* ==== delegates ==== */

	/* experiment service */
	UFUNCTION()
		void HandleTrackingServiceMessagePredator(FMessage message);
	UFUNCTION()
		void HandleTrackingServiceMessagePrey(FMessage message);
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
		URequest* SendStartEpisodeRequest(const FString ExperimentNameIn, const FString header);
	UFUNCTION()
		URequest* SendStopEpisodeRequest(const FString ExperimentNameIn, const FString header);
	UFUNCTION()
		void HandleEpisodeRequestResponse(const FString response);
	UFUNCTION()
		void HandleEpisodeRequestTimedOut();

	/* Experiment */
	UFUNCTION()
		void HandleStartExperimentResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleStartExperimentTimedOut();
	UFUNCTION()
		URequest* SendStartExperimentRequest(const FString ExperimentNameIn);
	UFUNCTION()
		URequest* SendFinishExperimentRequest(const FString ExperimentNameIn);
	UFUNCTION()
		void HandleFinishExperimentResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleFinishExperimentTimedOut();
	
	/* update players */
	UFUNCTION()
		void UpdatePredator(const FMessage message);
	UFUNCTION()
		void UpdatePreyPosition(const FVector Location);

	/* other */
	UFUNCTION()
		void HandleTrackingServiceMessage(const FMessage message);
	UFUNCTION()
		void HandleTrackingServiceUnroutedMessage(const FMessage message);
	UFUNCTION()
		void HandleExperimentServiceUnroutedMessage(const FMessage message);
	UFUNCTION()
		bool IsExperimentActive(const FString ExperimentNameIn);

	/* experiment control */
	UPROPERTY(BlueprintReadWrite)
		FString ExperimentNameActive;
	
	/* episode controll */
	UPROPERTY(BlueprintReadWrite)
		bool bInExperiment = false;
	UPROPERTY(BlueprintReadWrite)
		bool bInEpisode = false;

	/* occlusion control */
	FOcclusions OcclusionsStruct; 
	TArray<FLocation> OcclusionLocationsAll;
	TArray<int32> OcclusionIDsIntArr;
	TArray<AOcclusion*> OcclusionActorArr; 
	bool bIsOcclusionLoaded = false;
	bool SetOcclusionVisibility(TArray<int32> VisibleOcclusionIDsIn);

	UFUNCTION()
		URequest* SendGetOcclusionLocationsRequest();
	UFUNCTION()
		void HandleGetOcclusionLocationsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionLocationsTimedOut();
	UFUNCTION()
		URequest* SendGetOcclusionsRequest();
	UFUNCTION()
		void SpawnOcclusions(const TArray<int32> OcclusionIDsIn, const TArray<FLocation> Locations);
	UFUNCTION()
		void HandleGetOcclusionsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionsTimedOut();
	UFUNCTION()
		void HandleOcclusionLocation(const FMessage MessageIn);

		bool test();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	const FString SubjectName = "vr_dude";
	const float TimeOut = 5.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPLayReason) override;


	
};