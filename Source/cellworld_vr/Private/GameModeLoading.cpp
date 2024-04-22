// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "GameModeLoading.h"
#include "GameStateLoading.h"
#include "MouseKeyboardPlayerController.h"

AGameModeLoading::AGameModeLoading()
{
	/* Get PawnMain_BP to spawn */
	DefaultPawnClass = APawnMain::StaticClass();

	/* Assign default game state */
	GameStateClass = AGameStateLoading::StaticClass();

	/* assing default (for now) player controller */
	PlayerControllerClass = AMouseKeyboardPlayerController::StaticClass();

	/* standard defaults */
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AGameModeLoading::EndMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("[ AGameModeMain::EndMatch()] Force quit."));
	FGenericPlatformMisc::RequestExit(false);
}


void AGameModeLoading::SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation)
{
	/* to do: remove, no need for this if player start is present in map. if it isn''t, location will be taken care of by experiment service */

	//if (!GetWorld() || !GetWorld()->GetFirstPlayerController()) { UE_DEBUG_BREAK(); return; }
	//AGameModeLoading::PawnMain = Cast<APawnMain>(GetWorld()->GetFirstPlayerController()->GetPawn());
	//if (!AGameModeLoading::PawnMain) {
	//	AGameModeLoading::PawnMain->ResetOrigin();
	//}

	EAutoReceiveInput::Type::Player0;
}

void AGameModeLoading::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AGameModeLoading::InitGameState()
{
	Super::InitGameState();
}


void AGameModeLoading::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogTemp, Warning, TEXT("[AGameModeMain::StartPlay()] Starting game!\n"));
}

void AGameModeLoading::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

void AGameModeLoading::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
