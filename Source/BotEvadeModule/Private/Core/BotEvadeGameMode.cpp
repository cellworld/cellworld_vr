// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BotEvadeGameMode.h"

DEFINE_LOG_CATEGORY(LogBotEvadeGameMode);

ABotEvadeGameMode::ABotEvadeGameMode(){

	UE_LOG(LogBotEvadeGameMode, Log, TEXT("Initializing ABotEvadeGameMode()"))

	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
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

void ABotEvadeGameMode::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	UE_LOG(LogBotEvadeGameMode, Log, TEXT("BotEvadeGameMode: Tick"));

	UWorld* World = GetWorld();
	if (!World) { return; }
	
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		UE_LOG(LogTemp, Log, TEXT("BotEvadeGameMode: Found PlayerController(s): %i"), Iterator.GetIndex());
		const APlayerController* PlayerController = Iterator->Get();
		if (PlayerController) {
			UE_LOG(LogBotEvadeGameMode, Log, TEXT("BotEvadeGameMode: Found PlayerController: %s (%i) is valid!"), *PlayerController->GetName(), Iterator.GetIndex());
		}else {
			UE_LOG(LogBotEvadeGameMode, Log, TEXT("BotEvadeGameMode: Found PlayerController: %s (%i) is not valid!"), *PlayerController->GetName(), Iterator.GetIndex());
		}
	}
}
