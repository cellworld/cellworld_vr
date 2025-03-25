#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "MessageClient.h"
#include "ExperimentUtils.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "SoundDefinitions.h"
#include "ExperimentPlugin/HabitatComponents/Habitat.h"
#include "ExperimentPlugin/DataManagers/ExperimentManager.h"
#include "ExperimentPlugin/Public/Structs.h"
#include "ExperimentPlugin/Characters/ExperimentPredator.h"
#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "ExperimentPlugin/Occlusions/Occlusion.h"
#include "Kismet/GameplayStatics.h"
#include "MiscUtils/Timers/Stopwatch.h"
#include "MiscUtils/Timers/EventTimer.h"
#include "ExperimentClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperimentStatusChanged, EExperimentStatus, ExperimentStatusIn);

//todo: move this to its own file in ExperimentPlugin or BotEvadeModule

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
		// UE_LOG(LogTemp,Warning,TEXT("[FOcclusions.SetAllLocations] Locations: %s"),*LocationsIn);
		AllLocations = UExperimentUtils::OcclusionsParseAllLocations(LocationsIn); 
		bAllLocationsLoaded = true;
	}

	// ReSharper disable once CppPassValueParameterByConstReference
	void SetCurrentLocationsByIndex(const TArray<int32> OcclusionIndexIn) {
		
		UE_LOG(LogTemp,Log,
			TEXT("[FOcclusions::SetCurrentLocationsByIndex] Number of occlusions in configuration: %i"),
			OcclusionIndexIn.Num())
		OcclusionIDsIntArr = OcclusionIndexIn; 
		bCurrentLocationsLoaded = true;
	}

	bool SpawnAll(UWorld* WorldRefIn, const bool bHiddenInGameIn, const bool bEnableCollisonIn, const FTransform OriginTransform) {
		if (AllLocations.Num() < 1) { UE_LOG(LogTemp, Error, TEXT("[FOcclusions::SpawnAll] Failed. OcclusionAllLocationsArr is empty."));  return false; }
		if (!WorldRefIn) { UE_LOG(LogTemp, Error, TEXT("[FOcclusions::SpawnAll] Failed due to invalid UWorld object.")); return false; }
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		for (int i = 0; i < AllLocations.Num(); i++) {
			constexpr float ScaleOffset       = 0.99157164105; // give a little wiggle room between walls and occlusions
			constexpr float MapLength         = 235.185290;    // base length of habitat 
			constexpr float HeightScaleFactor = 3;             // make occlusions a bit taller; we aren't mice
			
			const FVector SpawnLocationConverted = UExperimentUtils::CanonicalToVrV2(
				AllLocations[i],
				MapLength,
				OriginTransform.GetScale3D().X);
			
			FVector ForwardVector = OriginTransform.GetRotation().GetForwardVector(); ForwardVector.Normalize();
			FVector RightVector   = OriginTransform.GetRotation().GetRightVector(); RightVector.Normalize();
			
			const FVector NewRelativeLocation = (ForwardVector * SpawnLocationConverted.X) + (-RightVector * SpawnLocationConverted.Y);
			FVector FinalLocation = OriginTransform.GetLocation() + NewRelativeLocation;
			
			FTransform SpawnTransform;
			FVector OcclusionScale = OriginTransform.GetScale3D()*ScaleOffset;
			OcclusionScale.Z *= HeightScaleFactor;
			FVector LocationFinal = FVector(AllLocations[i].x, AllLocations[i].y, OriginTransform.GetLocation().Z);
			// SpawnTransform.SetScale3D(OcclusionScale);
			// SpawnTransform.SetLocation(FinalLocation);
			// SpawnTransform.SetLocation(LocationFinal);
			// SpawnTransform.SetRotation(OriginTransform.GetRotation());
			UE_LOG(LogTemp, Log, TEXT("[Spawning occlusion] %s"), *LocationFinal.ToString())
			// AOcclusion* SpawnOcclusion = WorldRefIn->SpawnActor<AOcclusion>(
			// 	AOcclusion::StaticClass(),
			// 	SpawnTransform,
			// 	SpawnParams);

			AOcclusion* SpawnOcclusion = WorldRefIn->SpawnActor<AOcclusion>(
				AOcclusion::StaticClass(),
				FinalLocation,
				FRotator(),
				SpawnParams);
			
			if (ensure(SpawnOcclusion)) {
				SpawnOcclusion->SetReplicates(true);
				SpawnOcclusion->SetActorHiddenInGame(bHiddenInGameIn);
				SpawnOcclusion->SetActorEnableCollision(bEnableCollisonIn);
				SpawnOcclusion->SetActorHiddenInGame(false);
				SpawnOcclusion->SetActorScale3D(FVector(1,1,OriginTransform.GetScale3D().X*HeightScaleFactor));
				OcclusionAllArr.Add(SpawnOcclusion);
			}else {
				UE_LOG(LogTemp, Error, TEXT("[SpawnAll] Spawn occlusion failed! %i"), i)
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[FOcclusions.SpawnAll] OK."))
		bSpawnedAll = true;
		return true;
	}

	void SetAllHidden(){
		UE_LOG(LogTemp, Log, TEXT("[FOcclusions.SetAllHidden()]"))
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
			OcclusionAllArr[IndexArray[i]]->SetActorHiddenInGame(bActorHiddenInGame);
			OcclusionAllArr[IndexArray[i]]->SetActorEnableCollision(bEnableCollision);
		}
	}

	/* Set visibility of ALREADY SPAWNED OCCLUSIONS */
	void SetCurrentVisibility(const bool bVisibilityIn) {
		for (int i = 0; i < OcclusionIDsIntArr.Num(); i++) {
			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorHiddenInGame(bVisibilityIn);
			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Log, TEXT("[FOcclusions::SetCurrentVisibility] (%i)"), i);
		}
	}
	
};

USTRUCT(Blueprintable)
struct FExperimentHeaders {
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
struct FExperimentInfo {
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
		UE_LOG(LogTemp, Log, TEXT("[FExperimentInfo::SetStatus] New Status set: %i"), ExperimentStatusIn);
		Status = ExperimentStatusIn;
		OnExperimentStatusChangedEvent.Broadcast(ExperimentStatusIn);
	}
};

USTRUCT(Blueprintable)
struct FServerInfo {
	GENERATED_BODY()
public:
	FServerInfo() :
		Port(4791),
		// IP(TEXT("192.168.1.5")) // main machine mazenet-2 WIFI
		IP(TEXT("192.168.1.2")) // main machine mazenet-2 ETH
		// IP(TEXT("192.168.1.3")) // alberto machine 
		{}
	
	int Port;
	FString IP;  // static vr backpack win11 PACKAGED ONLY
};

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentClient : public AActor { 
	GENERATED_BODY()
	
public:	
	AExperimentClient();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, Blueprintable)
	FServerInfo ServerInfo = FServerInfo();
	
	/* DEBUG */
	bool bTimerRunning = false;
	Stopwatch::FStopWatch StopWatch;
	const float TimerFrequency = 0.5; 
	float TimeElapsedTick = 0.0f;
	uint32 FrameCountPrey = 0;
	uint32 FrameCountPredator = 0;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStopwatch> Stopwatch;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AHabitat> Habitat; 

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

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_SpawnOcclusions();
	bool Server_SpawnOcclusions_Validate();
	void Server_SpawnOcclusions_Implementation();
	
	UFUNCTION(BlueprintCallable)
	void SetWorldOrigin(const FVector& InWorldOriginA, const FVector& InWorldOriginB);
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_AttachOcclusionsToArena();
	bool Server_AttachOcclusionsToArena_Validate();
	void Server_AttachOcclusionsToArena_Implementation();
	
	UPROPERTY()
	TObjectPtr<UAudioComponent> OnCaptureSoundComponent; 
	UPROPERTY(Replicated)
	TObjectPtr<USoundBase> OnCaptureSoundCue; 
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_PlayCaptureSound();
	bool Server_PlayCaptureSound_Validate();
	void Server_PlayCaptureSound_Implementation();
	
	UFUNCTION(NetMulticast, Reliable, WithValidation, BlueprintCallable)
	void Multicast_PlayCaptureSound(const FVector Location);
	bool Multicast_PlayCaptureSound_Validate(const FVector Location);
	void Multicast_PlayCaptureSound_Implementation(const FVector Location);
	
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
	bool bResetSuccessDbg = false;

	/* ==== world stuff ==== */
	int FrameCount        = 0; // todo: will probably delete 
	const float MapLength = 235.185;
	const float PredatorScaleFactor = 0.1f; 
	float WorldScale      = 15.0f;

	UPROPERTY(EditAnywhere)
	FTransform OffsetOriginTransform;
	
	UPROPERTY(EditAnywhere)
	FVector FirstLocationDebug;
	
	/* ==== setup ==== */
	bool SpawnAndPossessPredator();
	UPROPERTY(Replicated)
	TObjectPtr<AExperimentPredator> PredatorBasic = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Spawning|Predator")
	TSubclassOf<ACharacter> PredatorBPClass;
	
	UPROPERTY(Replicated)
	TObjectPtr<ACharacter> PredatorCharacter = nullptr;
			
	/* functions called by GameMode and Blueprints */
	static UMessageClient* CreateNewClient();
	static bool ValidateClient(UMessageClient* ClientIn);
	static bool ValidateExperimentName(const FString& ExperimentNameIn);
	static bool Disconnect(UMessageClient* ClientIn);
	
	bool StopExperiment(const FString& ExperimentNameIn);
	
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode(const bool bForce = false);
	UPROPERTY(EditAnywhere)
		TObjectPtr<AExperimentPawn> PlayerPawnActive = nullptr;
	UPROPERTY(EditAnywhere)
		TObjectPtr<AExperimentPawn> PlayerPawn = nullptr;
	UPROPERTY(EditAnywhere)
		int PlayerIndex = -1; 

	/* ==== helper functions  ==== */
	bool ValidateLevel(UWorld* InWorld, const FString InLevelName);
	bool SetupPlayerUpdatePosition(APawn* InPawn);
	
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
 
	/* experiment service */
	UFUNCTION()
		void HandleStartEpisodeRequestResponse(const FString response);
	UFUNCTION()
		bool ResetTrackingAgent();
	UFUNCTION()
		void HandleStartEpisodeError(const FString InMessage = "");
	UFUNCTION()
		void HandleResetRequestResponse(const FString InResponse);
	UFUNCTION()
		void HandleResetRequestTimedOut();
	// will force a full system restart 
	UFUNCTION()
		void OnTimedOut(const FString InMessage);
	UFUNCTION()
		void OnSubscribeResult(bool bSubscribeResult);
	UFUNCTION()
		void OnResetResult(bool bResetResult);
	UFUNCTION()
		void HandleStartEpisodeRequestTimedOut();
	UFUNCTION()
		void HandleStopEpisodeRequestResponse(const FString response);
	UFUNCTION()
		void HandleStopEpisodeRequestTimedOut();
	UFUNCTION()
		void HandleStopEpisodeError(const FString InMessage = ""); 
	UFUNCTION()
		void SetPredatorIsVisible(bool bNewVisibility);

	/* gets location of all possible occlusions in cellworld  */
	UFUNCTION()
		bool SendGetOcclusionLocationsRequest();
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
		void HandleUnroutedMessage(const FMessage InMessage);
	
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
	UPROPERTY(Replicated)
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
	bool SetupConnections();
	UFUNCTION()
		void OnEpisodeStarted();
	UFUNCTION()
		void ResetWorldState();
	UFUNCTION()
		void OnEpisodeFinished();
	UFUNCTION(BlueprintCallable)
		void SetOcclusionVisibility(const bool bNewVisibility);
protected:
	bool Test();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	const FString SubjectName = "vr_dude";
	const float TimeOut = 5.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	
};