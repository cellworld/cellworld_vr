#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Tools/GenericClock.h"
#include "MessageClient.h"
#include "ExperimentUtils.h"
#include "PawnMain.h"
#include "ExperimentComponents/ExperimentMonitorData.h"
#include "ExperimentPlugin/Public/Structs.h"
#include "ExperimentComponents/Occlusion.h"
#include "DrawDebugHelpers.h"
#include "PredatorBasic.h"
#include "cellworld_vr/cellworld_vr.h"
#include "MiscUtils/Timer/EventTimer.h"
#include "ExperimentServiceMonitor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperimentStatusChanged, EExperimentStatus, ExperimentStatusIn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEpisodeFinished, int, PlayerIndex, float, Duration); // todo: bind this to PlayerPawn->FExperimentData.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNotifyOnExperimentFinished, int, PlayerIndex); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubscribeResultDelegate, bool, bSubscribeResult); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResetResultDelegate, bool, bResetResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubscribeStatusChangedDelegate, bool, bResetResult);

//todo: move this to its own file in ExperimentPlugin or BotEvadeModule
UCLASS(Blueprintable)
class  UExperimentManager : public UObject {
	GENERATED_BODY()
	UExperimentManager():MaxEpisodeTotalTime(1800.0f) {
		OnEpisodeFinishedDelegate.AddDynamic(this, &UExperimentManager::OnEpisodeFinished);
		OnSubscribeStatusChangedDelegate.AddDynamic(this, &UExperimentManager::OnSubscribeStatusChanged);
	}
public:
	UPROPERTY(EditAnywhere)
	TArray<int> ActivePlayers = {};
	
	UPROPERTY(EditAnywhere)
	TArray<APawnMain*> ActivePlayersPawns = {};

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UExperimentMonitorData>> Data = {};

	// const float MaxMinutesInEpisode = 30.0f;
	float MaxEpisodeTotalTime = 30.0f; 
	FNotifyOnExperimentFinished NotifyOnExperimentFinishedDelegate;
	FOnEpisodeFinished OnEpisodeFinishedDelegate;
	FOnSubscribeResultDelegate OnSubscribeResultDelegate;
	FOnResetResultDelegate OnResetResultDelegate;
	FSubscribeStatusChangedDelegate OnSubscribeStatusChangedDelegate; 

	/* flag to see if we are subscribed to CellworldGame */
	UPROPERTY(EditAnywhere)
	bool bSubscribed = false; 

	UFUNCTION()
	void OnSubscribeStatusChanged(const bool bSubscribeStatus) {
		bSubscribed = bSubscribeStatus;
		UE_LOG(LogExperiment, Log, TEXT("[OnSubscribeStatusChanged] bSubscribed = %i"),
			bSubscribed)
	}
	
	/* adds new player to ActivePlayer array.
	 * @return Index of new player. -1 if Index is already used or error. 
	 */
	int RegisterNewPlayer(APawnMain* InPawn, UExperimentMonitorData* InExperimentData) {
		if (!InPawn->IsValidLowLevelFast() || !InExperimentData->IsValidLowLevelFast()) {
			UE_LOG(LogExperiment, Error, TEXT("Failed to register player. APawn is not valid."))
			return -1;
		}
		
		const int NewPlayerIndex = Data.Add(InExperimentData);
		UE_LOG(LogExperiment, Warning, TEXT("RegisterNewPlayer(): Registered new player: Player %i"), NewPlayerIndex)
		return NewPlayerIndex;
	}

	bool IsPlayerRegistered(const int InPlayerIndex) const {
		if (!Data.IsValidIndex(InPlayerIndex)) {
			UE_LOG(LogExperiment, Error,
				TEXT("[IsPlayerRegistered]  InPlayerIndex not registered/valid."))
			return false; 
		}
		return true; 
	}
	
	bool IsExperimentDone(const int InPlayerIndex) const {
		if (!IsPlayerRegistered(InPlayerIndex)) {
			UE_LOG(LogExperiment, Error,
				TEXT("Player %i: Can't determine if experiment is done. Player is not registered!"),
				InPlayerIndex)
			return false;
		}

		const float ElapsedTimeTotal = (Data)[InPlayerIndex]->GetEpisodesCompletedTime();
		if (ElapsedTimeTotal >= MaxEpisodeTotalTime) {
			UE_LOG(LogExperiment, Warning,
				TEXT("Player %i: Experiment completed! Target time reached! PlayerTime: %0.2f; Target: %0.2f"),
				InPlayerIndex, ElapsedTimeTotal, MaxEpisodeTotalTime)
			return true; 
		}
		
		UE_LOG(LogExperiment, Log, TEXT("Player %i: Experiment still not complete!  PlayerTime: %0.2f; Target: %0.2f"),
				InPlayerIndex, ElapsedTimeTotal, MaxEpisodeTotalTime)
		return false;
	}
	
	/* adds new player to ActivePlayer array.
	 * @param const int InPlayerIndex - Player index to modify 
	 * @param const int InEpisodeDuration - Duration of completed episode
	 * @return void. 
	 */
	// ReSharper disable once CppMemberFunctionMayBeConst
	UFUNCTION()
	void OnEpisodeFinished(const int InPlayerIndex, const float InEpisodeDuration) {
		// check if player is in list
		if (!Data.IsValidIndex(InPlayerIndex)) {
			UE_LOG(LogExperiment, Error,
				TEXT("[OnEpisodeFinished] Failed to modify ExperimentData. InPlayerIndex not valid."))
			return; 
		}
		
		check(InEpisodeDuration >= 0.0f);

		Data[InPlayerIndex]->AddEpisodeAndCompletedTime(InEpisodeDuration);
		
		UE_LOG(LogExperiment, Warning, TEXT("Added new episode to data! Player: %i, Duration: %0.3f. Total time: %0.2f."),
					InPlayerIndex, InEpisodeDuration, Data[InPlayerIndex]->GetEpisodesCompletedTime())
		
		// check if we finished everything and if so, notify delegates 
		if (IsExperimentDone(InPlayerIndex)) {
			NotifyOnExperimentFinishedDelegate.Broadcast(InPlayerIndex);
			UE_LOG(LogExperiment, Warning,
				TEXT("Player (%i) finished experiment! Broadcasting to delegates (NotifyOnExperimentFinishedDelegate)."),
				InPlayerIndex)
		}
		
	}
	
	/* Get FExperimentMonitorData by player index.
	 * @param (const int) InPlayerIndex - Player index to get 
	 * @return pointer to player's FExperimentMonitorData. 
	 */
	UExperimentMonitorData* GetDataByIndex(const int InPlayerIndex) const {
		if (!Data.IsValidIndex(InPlayerIndex)) {
			UE_LOG(LogExperiment, Error,
				TEXT("[GetDataByIndex] Failed to get data. Player index not valid."))
			return nullptr; 
		}
		return (Data.IsValidIndex(InPlayerIndex)) ? (Data)[InPlayerIndex] : nullptr;
	}
	
};

USTRUCT(Blueprintable)
struct FOcclusions {
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentHeaders)
	FString OnCapture = "on_capture";
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
	
	void SetStatus(EExperimentStatus ExperimentStatusIn){
		const FString StatusString = FExperimentInfo::GetStatusString(&ExperimentStatusIn);
		UE_LOG(LogExperiment, Log, TEXT("[FExperimentInfo::SetStatus] New Status set: %s"), *StatusString);
		Status = ExperimentStatusIn;
		OnExperimentStatusChangedEvent.Broadcast(ExperimentStatusIn);
	}

	static FString GetStatusString(EExperimentStatus* ExperimentStatusIn) {
		if (!ExperimentStatusIn){ return FString("[FExperimentInfo.GetStatusString()] Invalid Pointer"); }
		const TEnumAsByte<EExperimentStatus> ExperimentStatusInByted = *ExperimentStatusIn;
		const FString EnumAsString = UEnum::GetValueAsString(ExperimentStatusInByted.GetValue());
		int32 Index;
		if (!EnumAsString.FindChar(TEXT(':'), Index)) { return FString("ConversionError"); }
		return EnumAsString.Mid(Index + 2);
	}
};

USTRUCT(Blueprintable)
struct FServerInfo {
	GENERATED_BODY()
public:
	FServerInfo() :
		Port(4791),
		IP(TEXT("192.168.1.199")) // main machine 
		{}
	
	int Port;
	FString IP;  // static vr backpack win11 PACKAGED ONLY
};

UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor { 
	GENERATED_BODY()
	
public:	
	AExperimentServiceMonitor();

	UPROPERTY(EditAnywhere, Blueprintable)
	FServerInfo ServerInfo = FServerInfo();
	
	/* ==== server stuff ==== */
	// const FString ServerIP         = "192.168.1.199";  // static vr backpack win11 PACKAGED ONLY
	// const int TrackingPort	           = 4791;
	
	/* DEBUG */
	bool bTimerRunning = false;
	GenericClock::FStopWatch StopWatch;
	const float TimerFrequency = 0.5; 
	float TimeElapsedTick = 0.0f;
	uint32 FrameCountPrey = 0;
	uint32 FrameCountPredator = 0;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStopwatch> Stopwatch;

	UPROPERTY()
	TObjectPtr<UExperimentManager> ExperimentManager;

	/* called when the whole experiment is completed */
	// FNotifyOnExperimentFinished NotifyOnExperimentFinishedDelegate;
	
	UFUNCTION()
	void OnExperimentFinished(const int InPlayerIndex);
	
	FTimerHandle TimerHandle;
	FTimerHandle* TimerHandlePtr = &TimerHandle;
	FTimerManager TimerManager;
	
	/* ==== main experiment service components ==== */
	UPROPERTY()
		TObjectPtr<UMessageClient> Client;
	UPROPERTY()
		TObjectPtr<UMessageClient> TrackingClient;

	/* Message Routes */
	UPROPERTY()
		TObjectPtr<UMessageRoute> MessageRouteOnCapture;
	UPROPERTY()
		TObjectPtr<UMessageRoute> MessageRoutePredator;

	/* Requests */
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
	UPROPERTY()
		TObjectPtr<URequest> GetOcclusionLocationRequest;
	UPROPERTY()
		TObjectPtr<URequest> ResetRequest;

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
	int FrameCount        = 0; // todo: will probably delete 
	const float MapLength = 235.185;
	const float PredatorScaleFactor = 0.5f; 
	float WorldScale      = 1.0f;
	
	/* experiment params */
	const float PositionSamplingRate = 90.0f; 
		
	/* ==== setup ==== */
	bool SpawnAndPossessPredator();
	UPROPERTY()
		TObjectPtr<APredatorBasic> PredatorBasic = nullptr;

	/* functions called by GameMode and Blueprints */
	static UMessageClient* CreateNewClient();
	static bool ValidateClient(UMessageClient* ClientIn);
	static bool ValidateExperimentName(const FString& ExperimentNameIn);
	static bool Disconnect(UMessageClient* ClientIn);
	
	bool StopExperiment(const FString& ExperimentNameIn);
	
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode(UMessageClient* ClientIn, const FString& ExperimentNameIn);
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode();

	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartTimerEpisode();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopTimerEpisode(double &InElapsedTime);

	UPROPERTY(EditAnywhere)
		TObjectPtr<APawnMain> PlayerPawnActive = nullptr;
	UPROPERTY(EditAnywhere)
		TObjectPtr<APawnMain> PlayerPawn = nullptr;
	UPROPERTY(EditAnywhere)
		int PlayerIndex = -1; 

	/* ==== helper functions  ==== */
	bool ValidateLevel(UWorld* InWorld, const FString InLevelName);
	bool GetPlayerPawn();
	
	void SelfDestruct(const FString InErrorMessage);
	
	/* ==== delegates ==== */
	UPROPERTY()
		FOnExperimentStatusChanged OnExperimentStatusChangedEvent;

	/* Called when status is changed. */
	UFUNCTION()
		void OnStatusChanged(const EExperimentStatus ExperimentStatusIn);

	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool SubscribeToTracking();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void HandleSubscribeToTrackingResponse(FString Response);
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void HandleSubscribeToTrackingTimedOut();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void RequestRemoveDelegates(URequest* InRequest, const FString& InRequestString = "");
	
	/* update predator stuff */
	UFUNCTION()
		void HandleUpdatePredator(const FMessage MessageIn);
	UFUNCTION()
		void HandleOnCapture(const FMessage MessageIn);
	UFUNCTION()
		float GetTimeRemaining() const;
	float GetTimeElapsed() const;

	/* experiment service */
	UFUNCTION()
		void HandleStartEpisodeRequestResponse(const FString response);
	UFUNCTION()
		bool ResetTrackingAgent();
	UFUNCTION()
		void HandleResetRequestResponse(const FString InResponse);
	UFUNCTION()
		void HandleResetRequestTimedOut();
	UFUNCTION()
		void OnSubscribeResult(bool bSubscribeResult);
	UFUNCTION()
		void OnResetResult(bool bResetResult);
	void SetPredatorIsVisible(bool bNewVisibility);
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
		void HandleStopExperimentResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleStopExperimentTimedOut();
	/* gets location of all possible occlusions in cellworld  */
	UFUNCTION()
		bool SendGetOcclusionLocationsRequest();
	UFUNCTION()
		void HandleGetOcclusionLocationsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionLocationsTimedOut();
	/* gets location of all possible occlusions in our specific/given experiment/world configuration
	 * default: 21_05
	 */
	UFUNCTION()
		bool SendGetOcclusionsRequest();
	UFUNCTION()
		void HandleGetOcclusionsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionsTimedOut();
	
	UFUNCTION()
		void HandleUnroutedMessage(const FMessage InMessage);
	UFUNCTION()
		bool IsExperimentActive(const FString ExperimentNameIn);
	
	/* update agents */
	UPROPERTY(EditAnywhere)
		bool bCanUpdatePrey = false;
	UFUNCTION()
		void UpdatePredator(const FMessage& InMessage);
	UFUNCTION()
		void UpdatePreyPosition(const FVector InVector, const FRotator InRotation);

	/* experiment control */
	UPROPERTY(BlueprintReadWrite)
		FString ExperimentNameActive;
	
	/* occlusion control */
	FOcclusions OcclusionsStruct; 
	TArray<FLocation> OcclusionLocationsAll;
	TArray<int32> OcclusionIDsIntArr;
	bool bIsOcclusionLoaded = false;


	
	
	UFUNCTION()
		static bool ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn, const FString& IPAddressIn, const int PortIn);
	UFUNCTION()
		bool RoutePredatorMessages();
	UFUNCTION()
		bool RouteOnCapture();
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	
};