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

	/* set visibility and collisions given an array of occlusion index/IDs */
	void SetVisibilityArr(const TArray<int32> IndexArray, const bool bActorHiddenInGame, const bool bEnableCollision) {
		for (int i = 0; i < IndexArray.Num(); i++) {
			OcclusionAllArr[IndexArray[i]]->SetActorHiddenInGame(false);
			OcclusionAllArr[IndexArray[i]]->SetActorEnableCollision(true);
			UE_LOG(LogTemp, Log, TEXT("SetVisibilityArr (%i)"), i);
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

	TObjectPtr<UMessageClient> Client;

	TObjectPtr<UMessageRoute> MessageRoutePredator;
	TObjectPtr<UMessageRoute> MessageRoutePrey;
	TObjectPtr<UMessageRoute> MessageRoute;
	TObjectPtr<UMessageRoute> TrackingServiceRoute;
	TObjectPtr<UMessageRoute> TrackingServiceRoutePrey; 
	TObjectPtr<UMessageRoute> TrackingServiceRoutePredator; 

	TObjectPtr<URequest> StartExperimentRequest;
	TObjectPtr<URequest> StopExperimentRequest;

	TObjectPtr<URequest> StartEpisodeRequest;
	TObjectPtr<URequest> TrackingServiceRequest; 

	//const FString header_experiment_service			= "predator_step";
	const FString header_prey_location		    = "prey_step";
	const FString header_predator_location		= "predator_step";
	const FString experiment_name               = "test_experiment";

	/* server stuff */
	// const FString ServerIPMessage = "172.26.176.129"; // works locally 
	const FString ServerIPMessage = "192.168.137.25"; // testing now 
	const int ServerPort	      = 4970;

	bool bCanUpdatePreyPosition      = false;
	bool bConnectedTrackingService	 = false; 
	bool bConnectedExperimentService = false; 
	bool bConnectedToServer			 = false;

	/* world stuff */
	float map_length                = 5100;
	int frame_count                 = 0; 

	/* coordinate system stuff */
	const float MapLength = 235.185;
	int WorldScale        = 15; 

	/* setup */
	const FString predator_step_header = "predator_step";

	bool SpawnAndPossessPredator();
	ACharacter* CharacterPredator = nullptr; 

	/* functions called by GameMode */
	static UMessageClient* CreateNewClient();
	bool StartExperiment(const FString ExperimentNameIn);
	bool StopExperiment(const FString ExperimentNameIn);
	bool StopConnection(UMessageClient* Client);
	bool DisconnectAll();
	bool DisconnectClients();
	
	/* will be used in BP to called by door opening (start episode)*/
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode(UMessageClient* ClientIn);
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode();

	bool ValidateLevel(UWorld* InWorld, const FString InLevelName);
	bool GetPlayerPawn();

	/* abort if anything goes wrong */
	void SelfDestruct(const FString InErrorMessage);
	
	/* ==== delegates ==== */

	/* experiment service */
	UFUNCTION()
		void HandleTrackingServiceMessagePredator(FMessage message);
	UFUNCTION()
		void HandleTrackingServiceMessagePrey(FMessage message);
	// UFUNCTION()
	// 	void HandleExperimentServiceMessage(FMessage message);
	// UFUNCTION()
	// 	void HandleExperimentServiceMessageTimedOut(FMessage message);
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
		URequest* SendStartEpisodeRequest(UMessageClient* ClientIn, const FString ExperimentNameIn);
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
		URequest* SendStartExperimentRequest(UMessageClient* ClientIn, FString ExperimentNameIn);
	UFUNCTION()
		URequest* SendFinishExperimentRequest(const FString& ExperimentNameIn);
	UFUNCTION()
		void HandleStopExperimentResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleStopExperimentTimedOut();
	
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