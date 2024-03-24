#pragma once 
#include "GameStateMain.h"
#include "GameModeMain.h"
//#include "MyGameInstance.h"

void AGameStateMain::HandleMatchHasStarted() {

	//Super::HandleMatchHasStarted();
	UE_LOG(LogTemp, Error, TEXT("[AGameState_Main::HandleMatchHasStarted()] "));

}

bool AGameStateMain::IsMatchInProgress() const
{
	return false;
}

bool AGameStateMain::HasMatchEnded() const
{
	Super::HasMatchEnded();
	return false;
}

bool AGameStateMain::HasMatchStarted() const
{
	Super::HasMatchStarted();
	return false;
}

void AGameStateMain::HandleBeginPlay()
{
	Super::HandleBeginPlay();
	//UE_LOG(LogTemp, Error, TEXT("[AGameState_Main::HandleBeginPlay()] Trials to complete: "));

}