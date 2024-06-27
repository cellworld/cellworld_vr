#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h" 
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "ExperimentUtils.h"
#include "PredatorController/CharacterPredator.h"
#include "PawnMain.h"
#include "PawnDebug.h"
#include "ExperimentComponents/Occlusion.h"
#include "DrawDebugHelpers.h"
#include "ExperimentServiceMonitor.generated.h"

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
			AOcclusion* MyMeshActor = WorldRefIn->SpawnActor<AOcclusion>(
				AOcclusion::StaticClass(),
				UExperimentUtils::CanonicalToVr(OcclusionAllLocationsArr[i], MapLength, WorldScaleVecIn.X*ScaleOffset), // todo: make scale dynamic
				Rotation,
				SpawnParams);
			
			MyMeshActor->SetActorScale3D(WorldScaleVecIn*ScaleOffset); 
			MyMeshActor->SetActorHiddenInGame(bVisibilityIn);
			MyMeshActor->SetActorEnableCollision(bEnableCollisonIn);

			OcclusionAllArr.Add(MyMeshActor);
		}

		return true;
	}

	void SetVisibilityAll(const bool bVisibilityIn) {
		for (AOcclusion* Occlusion : OcclusionAllArr) {
			Occlusion->SetActorHiddenInGame(true);
			Occlusion->SetActorEnableCollision(false);
		}
	}

	/* set visibility and collisions given an array of occlusion index/IDs */
	void SetVisibilityArr(const TArray<int32> IndexArray, const bool bActorHiddenInGame, const bool bEnableCollision) {
		for (int i = 0; i < IndexArray.Num(); i++) {
			OcclusionAllArr[IndexArray[i]]->SetActorHiddenInGame(false);
			OcclusionAllArr[IndexArray[i]]->SetActorEnableCollision(true);
		}
	}

	/* Set visibility of ALREADY SPAWNED OCCLUSIONS */
	void SetCurrentVisibility(const bool bVisibilityIn, const bool bEnableCollisonIn) {
		for (int i = 0; i < OcclusionIDsIntArr.Num(); i++) {
			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorHiddenInGame(bVisibilityIn);
			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorEnableCollision(bEnableCollisonIn);
			UE_LOG(LogTemp, Log, TEXT("SetCirrentVisibility (%i)"), i);
		}
	}
};

UENUM()
enum class EExperimentStatus : uint8
{
	// client is in an active experiment/episode (can be both - maybe ill change to no) 
	InExperiment,
	InEpisode,

	// 'waiting room' flags - waiting for XYZ to start 
	WaitingExperiment,
	WaitingEpisode,
	
	// completion flags
	FinishedExperiment,
	FinishedEpisode,

	// is the client done? Yes? Ok lets disconnect and tell subject thanks for coming
	WaitingFinishSuccess,
	WaitingFinishError,
	
	// there's always stuff we don't expect, right? 
	Unknown,
};

UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor
{ 
	GENERATED_BODY()
	
public:	
	AExperimentServiceMonitor();

	/* ==== main experiment service components ==== */
	UPROPERTY()
		TObjectPtr<UMessageClient> Client;
	UPROPERTY()
		TObjectPtr<UMessageRoute> MessageRoutePredator;
	UPROPERTY()
		TObjectPtr<UMessageRoute> MessageRoutePrey;
	UPROPERTY()
		TObjectPtr<UMessageRoute> MessageRoute;
	UPROPERTY()
		TObjectPtr<UMessageRoute> TrackingServiceRoute;
	UPROPERTY()
		TObjectPtr<UMessageRoute> TrackingServiceRoutePrey; 
	UPROPERTY()
		TObjectPtr<UMessageRoute> TrackingServiceRoutePredator; 
	UPROPERTY()
		TObjectPtr<URequest> StartExperimentRequest;
	UPROPERTY()
		TObjectPtr<URequest> StartEpisodeRequest;
	UPROPERTY()
		TObjectPtr<URequest> StopExperimentRequest;
	UPROPERTY()
		TObjectPtr<URequest> StopEpisodeRequest;
	UPROPERTY()
		TObjectPtr<URequest> SubscribeRequest;
	UPROPERTY()
		TObjectPtr<UMessageRoute> RoutePredator;
	UPROPERTY()
		TObjectPtr<UMessageRoute> RouteAgent;
	UPROPERTY()
		TObjectPtr<UMessageRoute> RouteOnEpisodeStarted;

	// const FString header_experiment_service			= "predator_step";
	const FString header_prey_location		    = "prey_step";
	const FString header_predator_location		= "predator_step";
	const FString experiment_name               = "test_experiment";
	const FString predator_step_header          = "predator_step";
	const FString on_episode_started_header     = "on_episode_started";
	
	/* ==== server stuff ==== */
	// const FString ServerIPMessage = "172.30.127.68";   // alternate 
	// const FString ServerIPMessage = "192.168.137.8";   // lab new
	// const FString ServerIPMessage = "192.168.137.111"; // static laptop lab 
	const FString ServerIPMessage = "127.0.0.1";		  // localhost  
	const int ServerPort	      = 4970;

	/* ==== status stuff ==== */
	bool bCanUpdatePreyPosition      = false;
	bool bConnectedTrackingService	 = false; 
	bool bConnectedExperimentService = false; 
	bool bConnectedToServer			 = false;

	/* ==== world stuff ==== */
	float map_length      = 5100;
	int FrameCount        = 0; // todo: will probably delete 
	const float MapLength = 235.185;
	float WorldScale      = 4.0f; 

	/* ==== setup ==== */
	bool SpawnAndPossessPredator();
	ACharacter* CharacterPredator = nullptr;

	/* functions called by GameMode and Blueprints */
	static UMessageClient* CreateNewClient();
	static bool Disconnect(UMessageClient* ClientIn);
	
	bool StartExperiment(const FString& ExperimentNameIn);
	bool StopExperiment(const FString& ExperimentNameIn);
	
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode(UMessageClient* ClientIn, const FString& ExperimentNameIn);
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode();

	/* ==== helper functions  ==== */
	bool ValidateLevel(UWorld* InWorld, const FString InLevelName);
	bool GetPlayerPawn();
	void SelfDestruct(const FString InErrorMessage);
	bool SubscribeToServer(TObjectPtr<UMessageClient> ClientIn);
	void RequestRemoveDelegates(URequest* RequestIn);

	/* event functions */
	void on_experiment_started();
	void on_experiment_finished();
	void on_episode_started();
	void on_episode_finished();
	
	/* ==== delegates ==== */

	/* update predator stuff */
	UFUNCTION()
		void HandleUpdatePredator(FMessage MessageIn);

	/* experiment service */
	UFUNCTION()
		void HandleSubscribeToServerResponse(FString MessageIn);
	UFUNCTION()
		void HandleSubscribeToServerTimedOut();
	UFUNCTION()
		void HandleTrackingServiceMessagePredator(FMessage message);
	UFUNCTION()
		void HandleTrackingServiceMessagePrey(FMessage message);
	UFUNCTION()
		void HandleExperimentServiceResponse(const FString message);
	UFUNCTION()
		void HandleExperimentServiceResponseTimedOut();

	/* tracking service */
	UFUNCTION()
		void HandleTrackingServiceResponse(const FString message);
	UFUNCTION()
		void HandleTrackingServiceResponseTimedOut();
	UFUNCTION()
		void HandleStartEpisodeRequestResponse(const FString response);
	UFUNCTION()
		void HandleStartEpisodeRequestTimedOut();
	UFUNCTION()
		void HandleStopEpisodeRequestResponse(const FString response);
	UFUNCTION()
		void HandleStopEpisodeRequestTimedOut();

	/* Experiment */
	UFUNCTION()
		void HandleStartExperimentResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleStartExperimentTimedOut();
	UFUNCTION()
		URequest* SendStartExperimentRequest(UMessageClient* ClientIn, FString ExperimentNameIn);
	UFUNCTION()
		URequest* SendFinishExperimentRequest(const FString& ExperimentNameIn);
	UFUNCTION()
		void HandleStopExperimentResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleStopExperimentTimedOut();
	
	/* update players */
	UFUNCTION()
		void UpdatePredator(FMessage message);
	UFUNCTION()
		void UpdatePreyPosition(const FVector Location);

	/* other */
	UFUNCTION()
		void HandleTrackingServiceMessage(FMessage message);
	UFUNCTION()
		void HandleTrackingServiceUnroutedMessage(FMessage message);
	UFUNCTION()
		void HandleUnroutedMessage(FMessage message);
	UFUNCTION()
		bool IsExperimentActive(const FString ExperimentNameIn);
	
	void AttachAgent(TObjectPtr<APawn> PawnIn);

	/* experiment control */
	UPROPERTY(BlueprintReadWrite)
		FString ExperimentNameActive;
	
	/* episode control */
	bool bInExperiment = false;
	bool bInEpisode = false;

	/* occlusion control */
	FOcclusions OcclusionsStruct; 
	TArray<FLocation> OcclusionLocationsAll;
	TArray<int32> OcclusionIDsIntArr;
	bool bIsOcclusionLoaded = false;

	UFUNCTION()
		URequest* SendGetOcclusionLocationsRequest();
	UFUNCTION()
		void HandleGetOcclusionLocationsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionLocationsTimedOut();
	UFUNCTION()
		bool SendGetOcclusionsRequest();
	UFUNCTION()
		void HandleGetOcclusionsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionsTimedOut();
	UFUNCTION()
		void HandleOcclusionLocation(const FMessage MessageIn);
	
	static bool ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn, const FString& IPAddressIn, const int PortIn);
	bool RoutePredatorMessages();

	bool Test();

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