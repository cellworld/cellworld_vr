#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ConfigManager.h"
#include "PawnMain.h"
#include "PredatorController/ExperimentServiceMonitor.h"
#include "GameModeMain.generated.h"

UCLASS()
class CELLWORLD_VR_API AGameModeMain : public AGameModeBase
{
public:
	AGameModeMain();
	
	GENERATED_BODY()
	virtual void EndGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bUseVR = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bSpawnExperimentService = false;

	/* world and coordinate system stuff */
	AActor* GetLevelActorFromName(const FName& ActorNameIn);
	
	/* debug */
	TObjectPtr<UClass> PawnClassToSpawn;
	TObjectPtr<APawn> PlayerPawn;
	FVector InitialPlayerLocation;
	FRotator InitialPlayerRotation;

	/* spawning player */
	FVector spawn_location_player  = { -1700.0,1500.000000,30.000000 };
	FRotator spawn_rotation_player = { 0.0,0.0, 0.0 };
	void SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation);
	bool AttachClientToPlayer(TObjectPtr<UMessageClient> ClientIn, TObjectPtr<APawnMain> PawnIn);

	/* Spawns Sensors */
	void SpawnGetCLMonitorComponentActor();

	/* spawn everything */
	void SpawnAllLoggingActor(); 

	/* controls loading screen */
	void StartLoadingScreen();
	void StopLoadingScreen(); 
	
	/* to store waypoint info  */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState();
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	/* functions for door and experiment control */
	UPROPERTY(BlueprintReadWrite)
		AExperimentServiceMonitor* ExperimentServiceMonitor = nullptr;
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void SpawnExperimentServiceMonitor();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool ExperimentStartEpisode(); 
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool ExperimentStopEpisode();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool ExperimentStopExperiment(const FString ExperimentNameIn);
};