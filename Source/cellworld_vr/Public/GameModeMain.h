#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GetCLMonitorComponentActor.h"
#include "CLMonitorComponent.h"
#include "ConfigManager.h"
//#include "PawnMain.h"
//#include "Templates/SharedPointer.h" 
#include "GameModeMain.generated.h"

/**
 *
 */


UCLASS()
class CELLWORLD_VR_API AGameModeMain : public AGameModeBase
{
public:
	GENERATED_BODY()
	virtual void EndMatch();

	
protected:
	AGameModeMain();
	FVector debug_vect;

	APlayerController* PlayerControllerClassCast = nullptr;
	FVector InitialPlayerLocation;
	FRotator InitialPlayerRotation;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	bool InitializeHPKeys();

	/* spawning player */
	FVector spawn_location_player  = { 180.000000,-30.000000,30.000000 };
	FRotator spawn_rotation_player = { 0.0,0.0, 0.0 };
	void SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation);
	//APawnMain* PlayerActor; 


	/* Spawns Sensors */
	void SpawnGetCLMonitorComponentActor();
	AGetCLMonitorComponentActor* GetCLMonitorComponentActor = nullptr;

	/* spawn everything */
	void SpawnAllLoggingActor(); 
	
	/* to store waypoint info  */
	virtual void InitGameState();
	virtual void StartPlay() override;
	virtual void Tick(float DeltaTime) override;

};
