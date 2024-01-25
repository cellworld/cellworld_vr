#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GetCLMonitorComponentActor.h"
#include "CLMonitorComponent.h"
//#include "ConfigManager.h"
#include "GameModeCalibration.generated.h"

/**
 *
 */


UCLASS()
class CELLWORLD_VR_API AGameModeCalibration : public AGameModeBase
{
public:
	GENERATED_BODY()
	virtual void EndMatch();

	//UGameInstanceMain* GI = nullptr;
	APawnMain* PawnMain   = nullptr;

protected:
	AGameModeCalibration();
	FVector debug_vect;

	APlayerController* PlayerControllerClassCast = nullptr;
	FVector InitialPlayerLocation;
	FRotator InitialPlayerRotation;


	/* spawning player */
	FVector spawn_location_player = { 180.000000,-30.000000,30.000000 };
	FRotator spawn_rotation_player = { 0.0,0.0, 0.0 };
	void SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation);

	/* to store waypoint info  */
	virtual void Tick(float DeltaTime) override;
	virtual void StartPlay() override;
	virtual void InitGameState();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

};
