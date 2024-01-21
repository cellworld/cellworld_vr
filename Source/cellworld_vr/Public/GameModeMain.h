#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GetCLMonitorComponentActor.h"
#include "CLMonitorComponent.h"
#include "ConfigManager.h"
#include "PawnMain.h"
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

	/* debug */
	FVector debug_vect;

	//UGameInstanceMain* GI = nullptr;
	APawnMain* PawnMain   = nullptr;

	APlayerController* PlayerControllerClassCast = nullptr;
	FVector InitialPlayerLocation;
	FRotator InitialPlayerRotation;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	bool InitializeHPKeys();

	/* spawning player */
	FVector spawn_location_player  = { 180.000000,-30.000000,30.000000 };
	FRotator spawn_rotation_player = { 0.0,0.0, 0.0 };
	void SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation);
	
	/* Spawns Sensors */
	void SpawnGetCLMonitorComponentActor();
	AGetCLMonitorComponentActor* GetCLMonitorComponentActor = nullptr;

	/* spawn everything */
	void SpawnAllLoggingActor(); 
	
	/* to store waypoint info  */
	virtual void InitGameState();
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

};
