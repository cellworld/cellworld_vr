// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BotEvadeGameMode.h"

DEFINE_LOG_CATEGORY(LogBotEvadeGameMode);

ABotEvadeGameMode::ABotEvadeGameMode(){

	UE_LOG(LogBotEvadeGameMode, Log, TEXT("Initializing ABotEvadeGameMode()"))

	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;
}

void ABotEvadeGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ABotEvadeGameMode::InitGameState()
{
	Super::InitGameState();
}

void ABotEvadeGameMode::StartPlay() {
	Super::StartPlay();
	UE_LOG(LogBotEvadeGameMode, Log, TEXT("StartPlay()"))
}

void ABotEvadeGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ABotEvadeGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
