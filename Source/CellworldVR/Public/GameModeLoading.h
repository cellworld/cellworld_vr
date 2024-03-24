// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnMain.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeLoading.generated.h"

/**
 * 
 */
UCLASS()
class CELLWORLDVR_API AGameModeLoading : public AGameModeBase
{
public:
	GENERATED_BODY()


protected:
	AGameModeLoading();

	/* overrides from APawn*/
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState();
	virtual void EndMatch();

	/* player stuff */
	APawnMain* PawnMain = nullptr;
	APlayerController* PlayerControllerClassCast = nullptr;
	FVector InitialPlayerLocation;
	FRotator InitialPlayerRotation;

	/* spawning player */
	FVector spawn_location_player = { 0,0,0 };
	FRotator spawn_rotation_player = { 0.0,0.0, 0.0 };
	void SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation);

	
	
};
