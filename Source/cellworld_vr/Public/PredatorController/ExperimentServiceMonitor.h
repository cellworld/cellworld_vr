#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Tools/GenericClock.h"
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "ExperimentUtils.h"
#include "PredatorController/CharacterPredator.h"
#include "PawnMain.h"
#include "PawnDebug.h"
#include "ExperimentPlugin/Public/Structs.h"
#include "ExperimentComponents/Occlusion.h"
#include "DrawDebugHelpers.h"
#include "PredatorBasic.h"
#include "cellworld_vr/cellworld_vr.h"
#include "ExperimentServiceMonitor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperimentStatusChanged, EExperimentStatus, ExperimentStatusIn);
//
// UENUM(Blueprintable)
// enum class EExperimentStatus : uint8
// {
// 	// client is in an active experiment/episode (can be both - maybe ill change to no) 
// 	InExperiment		  	 UMETA(DisplayName = "InExperiment"),
// 	InEpisode			  	 UMETA(DisplayName = "InEpisode"),
//
// 	// episode failed - time ran out  
// 	FailedEpisodeTimer	     UMETA(DisplayName = "FailedEpisodeTimer"),
//
// 	// 'waiting room' flags - waiting for XYZ to start 
// 	WaitingExperiment	  	 UMETA(DisplayName = "WaitingExperiment"),
// 	WaitingEpisode		  	 UMETA(DisplayName = "WaitingEpisode"),
// 	WaitingFinishSuccess     UMETA(DisplayName = "WaitingFinishSuccess"),
// 	WaitingFinishError	     UMETA(DisplayName = "WaitingFinishError"),
// 	
// 	// completion flags
// 	FinishedExperiment	  	 UMETA(DisplayName = "FinishedExperiment"),
// 	FinishedEpisode		  	 UMETA(DisplayName = "FinishedEpisode"),
//
// 	// error flags
// 	ErrorStartExperiment 	 UMETA(DisplayName = "FailedStartExperiment"),
// 	ErrorStartEpisode    	 UMETA(DisplayName = "FailedStartEpisode"),
// 	ErrorFinishedExperiment  UMETA(DisplayName = "FailedFinishedExperiment"),
// 	ErrorFinishEpisode		 UMETA(DisplayName = "FailedFinishEpisode"),
// 	ErrorTimedOutExperiment	 UMETA(DisplayName="TimedOutExperiment"),
// 	ErrorTimedOutEpisode	 UMETA(DisplayName="TimedOutEpisode"),
// 	
// 	// there's always stuff we don't expect, right? 
// 	Unknown				     UMETA(DisplayName = "Unknown"),
// };

USTRUCT(Blueprintable)
struct FExperimentTimer
{
	GENERATED_USTRUCT_BODY()
public:
	FExperimentTimer()
	{
		
	}

	void Start(const float DurationIn)
	{
			
	}
	
	FTimerHandle Handle;
	
};

USTRUCT(Blueprintable)
struct FOcclusions
{
	GENERATED_USTRUCT_BODY()
public:

	FOcclusions() { OcclusionAllArr = {}; }

	UPROPERTY(EditAnywhere)
		bool bAllLocationsLoaded = false; 
	UPROPERTY(EditAnywhere)
		bool bCurrentLocationsLoaded = false;
	UPROPERTY(EditAnywhere)
		bool bSpawnedAll = false;

	UPROPERTY(EditAnywhere)
		TArray<AOcclusion*> OcclusionAllArr {};
	UPROPERTY(EditAnywhere)
		TArray<AOcclusion*> CurrentVisibleArr {}; 
	UPROPERTY(EditAnywhere)
		TArray<FLocation> AllLocations {};
	UPROPERTY(EditAnywhere)
		TArray<int32> OcclusionIDsIntArr {};

	/* canonical */
	void SetAllLocations(const FString& LocationsIn) {
		// UE_LOG(LogExperiment,Warning,TEXT("[FOcclusions.SetAllLocations] Locations: %s"),*LocationsIn);
		AllLocations = UExperimentUtils::OcclusionsParseAllLocations(LocationsIn); 
		bAllLocationsLoaded = true;
	}

	void SetCurrentLocationsByIndex(const TArray<int32>& OcclusionIndexIn) {
		OcclusionIDsIntArr = OcclusionIndexIn; 
		bCurrentLocationsLoaded = true;
	}

	bool SpawnAll(UWorld* WorldRefIn, const bool bHiddenInGameIn, const bool bEnableCollisonIn, const FVector WorldScaleVecIn) {

		if (AllLocations.Num() < 1) { UE_LOG(LogExperiment, Error, TEXT("FOcclusions.SpawnAll() Failed. OcclusionAllLocationsArr is empty."));  return false; }
		if (!WorldRefIn) { UE_LOG(LogExperiment, Error, TEXT("[FOcclusions.SpawnAll()] Failed due to invalid UWorld object.")); return false; }

		const FRotator Rotation(0.0f, 0.0f, 0.0f); // Desired spawn rotation
		const float ScaleOffset = 0.99157164105; 
		const float MapLength = 235.185290;
		FActorSpawnParameters SpawnParams;
		for (int i = 0; i < AllLocations.Num(); i++) {
			AOcclusion* SpawnOcclusion = WorldRefIn->SpawnActor<AOcclusion>(
				AOcclusion::StaticClass(),
				UExperimentUtils::CanonicalToVr(AllLocations[i], MapLength, WorldScaleVecIn.X*ScaleOffset), // todo: make scale dynamic
				Rotation,
				SpawnParams);
			SpawnOcclusion->SetActorScale3D(WorldScaleVecIn*ScaleOffset); 
			SpawnOcclusion->SetActorHiddenInGame(bHiddenInGameIn);
			SpawnOcclusion->SetActorEnableCollision(bEnableCollisonIn);
			OcclusionAllArr.Add(SpawnOcclusion);
		}
		UE_LOG(LogExperiment, Warning, TEXT("[FOcclusions.SpawnAll] OK."))
		bSpawnedAll = true;
		return true;
	}

	void SetAllHidden(){
		UE_LOG(LogExperiment,Warning,TEXT("[FOcclusions.SetAllHidden()]"))
		for (AOcclusion* Occlusion : OcclusionAllArr) {
			Occlusion->SetActorHiddenInGame(true);
			Occlusion->SetActorEnableCollision(false);
		}
	}

	void SetVisibilityAll(const bool bHiddenInGameIn) {
		for (AOcclusion* Occlusion : OcclusionAllArr) {
			Occlusion->SetActorHiddenInGame(bHiddenInGameIn);
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

USTRUCT(Blueprintable)
struct FExperimentHeaders
{
	GENERATED_BODY()
	/* headers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentHeaders)
	FString PreyStep     = "prey_step";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentHeaders)
	FString PredatorStep = "predator_step";
};

USTRUCT(Blueprintable)
struct FExperimentInfo
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		EExperimentStatus Status = EExperimentStatus::WaitingExperiment;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FStartExperimentRequest StartExperimentRequestBody;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FStartEpisodeRequest StartEpisodeRequestBody;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FStartExperimentResponse StartExperimentResponse;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FStartEpisodeResponse StartEpisodeResponse;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FString ExperimentNameActive = "";

	FOnExperimentStatusChanged OnExperimentStatusChangedEvent;
	
	void SetStatus(const EExperimentStatus ExperimentStatusIn){
		UE_LOG(LogExperiment, Warning, TEXT("[SetStatus] New Status set!"));
		Status = ExperimentStatusIn;
		OnExperimentStatusChangedEvent.Broadcast(ExperimentStatusIn);
	}
};
	
UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor
{ 
	GENERATED_BODY()
	
public:	
	AExperimentServiceMonitor();

	/* DEBUG */
	bool bTimerRunning = false;
	GenericClock::FStopWatch StopWatch;
	const float TimerFrequency = 0.5; 
	float TimeElapsedTick = 0.0f;
	uint32 FrameCountPrey = 0;
	uint32 FrameCountPredator = 0; 
	FTimerHandle TimerHandle;
	FTimerManager TimerManager;
	
	/* ==== main experiment service components ==== */
	UPROPERTY()
		TObjectPtr<UMessageClient> Client;
	UPROPERTY()
		TObjectPtr<UMessageClient> TrackingClient;

	/* routes */
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
		TObjectPtr<UMessageRoute> RouteOnEpisodeStarted;
	UPROPERTY()
		TObjectPtr<UMessageRoute> RoutePredatorStep;
	UPROPERTY()
		TObjectPtr<UMessageRoute> RouteAgent;

	/* ==== requests ==== */
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
		TObjectPtr<URequest> TrackingSubscribeRequest;
	UPROPERTY()
		TObjectPtr<URequest> GetOcclusionsRequest;

	/* headers */
	const FString header_prey_location		    = "prey_step";
	const FString header_predator_location		= "predator_step";
	const FString experiment_name               = "test_experiment";
	const FString predator_step_header          = "predator_step";
	const FString on_episode_started_header     = "on_episode_started";
	
	/* ==== server stuff ==== */
	
	const FString ServerIPMessage = "172.26.176.129";   // lab pc new
	// const FString ServerIPMessage = "192.168.137.111"; // static laptop lab 
	// const FString ServerIPMessage = "127.0.0.1";		  // localhost  
	// const FString ServerIPMessage = "10.0.0.77";		  // home eth
	const int ServerPort	      = 4970;
	const int TrackingPort	      = 4790;

	/* ==== status stuff ==== */
	UPROPERTY()
		FExperimentInfo ExperimentInfo {};
	UPROPERTY()
		FExperimentHeaders ExperimentHeaders; 
	
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
	// ACharacter* CharacterPredator = nullptr;
	UPROPERTY()
	TObjectPtr<APredatorBasic> PredatorBasic = nullptr;

	/* functions called by GameMode and Blueprints */
	static UMessageClient* CreateNewClient();
	static bool ValidateClient(UMessageClient* ClientIn);
	static bool ValidateExperimentName(const FString& ExperimentNameIn);
	static bool Disconnect(UMessageClient* ClientIn);
	
	bool StartExperiment(const FString& ExperimentNameIn);
	bool StopExperiment(const FString& ExperimentNameIn);
	
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode(UMessageClient* ClientIn, const FString& ExperimentNameIn);
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode();

	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartTimerEpisode(const float DurationIn, FTimerHandle TimerHandleIn);

	/* ==== helper functions  ==== */
	bool ValidateLevel(UWorld* InWorld, const FString InLevelName);
	bool GetPlayerPawn();
	void SelfDestruct(const FString InErrorMessage);

	
	/* ==== delegates ==== */
	UPROPERTY()
		FOnExperimentStatusChanged OnExperimentStatusChangedEvent;
	UFUNCTION()
		void OnStatusChanged(const EExperimentStatus ExperimentStatusIn);
	
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool SubscribeToTracking();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void HandleSubscribeToTrackingResponse(FString Response);
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void HandleSubscribeToTrackingTimedOut();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool SubscribeToServer(UMessageClient* ClientIn);
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void RequestRemoveDelegates(URequest* RequestIn);
	
	/* update predator stuff */
	UFUNCTION()
		void HandleUpdatePredator(FMessage MessageIn);
	UFUNCTION()
		void OnTimerFinished();
	UFUNCTION()
		float GetTimeRemaining();

	/* experiment service */
	UFUNCTION()
		void HandleSubscribeToServerResponse(FString MessageIn);
	UFUNCTION()
		void HandleSubscribeToServerTimedOut();
	UFUNCTION()
		void HandleStartEpisodeRequestResponse(const FString response);
	UFUNCTION()
		bool ResetTrackingAgent();
	UFUNCTION()
		void HandleStartEpisodeRequestTimedOut();
	UFUNCTION()
		void HandleStopEpisodeRequestResponse(const FString response);
	UFUNCTION()
		void HandleStopEpisodeRequestTimedOut();
	
	/* tracking service */

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
	UFUNCTION()
		static bool ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn, const FString& IPAddressIn, const int PortIn);
	UFUNCTION()
		bool RoutePredatorMessages();
	UFUNCTION()
		bool RemoveDelegatesPredatorRoute();

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