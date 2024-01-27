// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModeMain.h"
#include "GameFramework/PlayerStart.h"
#include "PawnMain.h" 
#include "GameStateMain.h"
#include "PawnDebug.h"
#include "HPGlia.h"
#include "MouseKeyboardPlayerController.h"
#include "PlayerControllerVR.h"

AGameModeMain::AGameModeMain()
{
	/* Get PawnMain_BP to spawn */
	bool DEBUG = false; 
	if (DEBUG){
		DefaultPawnClass = APawnDebug::StaticClass(); 
		PlayerControllerClass = AMouseKeyboardPlayerController::StaticClass();
	}
	else { 
		DefaultPawnClass = APawnMain::StaticClass(); 
		PlayerControllerClass = APlayerControllerVR::StaticClass();
	}

	/* Assign default game state */
	GameStateClass = AGameStateMain::StaticClass();

	/* standard defaults */
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AGameModeMain::EndGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[ AGameModeMain::EndGame()] Force quit."));
	FGenericPlatformMisc::RequestExit(false);
}

/* 
* Updates GameInstance with HP keys. Will use variables inside GameInstanceMain.h 
* to find, load, and process the HP keys. 
*/
bool AGameModeMain::InitializeHPKeys() {

	return false;
}

void AGameModeMain::SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation)
{
	/* to do: remove, no need for this if player start is present in map. if it isn''t, location will be taken care of by experiment service */

	if(!GetWorld() || !GetWorld()->GetFirstPlayerController()) { UE_DEBUG_BREAK(); return; }
	AGameModeMain::PawnMain = Cast<APawnMain>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (AGameModeMain::PawnMain) {
		//AGameModeMain::PawnMain->ResetOrigin();
	}
	EAutoReceiveInput::Type::Player0;
}

void AGameModeMain::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AGameModeMain::InitGameState()
{
	Super::InitGameState();
}

void AGameModeMain::SpawnGetCLMonitorComponentActor()
{
	FTransform tSpawnTransform;
	FVector TempLoc = { 0.0f, 0.0f, 0.0f };
	FRotator TempRot = tSpawnTransform.GetRotation().Rotator();

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetCLMonitorComponentActor = Cast<AGetCLMonitorComponentActor>(GetWorld()->SpawnActor(AGetCLMonitorComponentActor::StaticClass(), &TempLoc, &TempRot, SpawnInfo));
}

/* spawn all logging actors, some may contain threads but they handle themselves. 
* right now, theres only one, but im gonna call this function to maintain consitency.
*/
void AGameModeMain::SpawnAllLoggingActor()
{
	/* eye-tracker */
	AGameModeMain::SpawnGetCLMonitorComponentActor();

	/* player path */
}

void AGameModeMain::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogTemp, Warning, TEXT("[AGameModeMain::StartPlay()] Starting game!"));

	/* spawn player */
	AGameModeMain::SpawnAndPossessPlayer(spawn_location_player, spawn_rotation_player);

	AGameModeMain::SpawnAllLoggingActor();
}

void AGameModeMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGameModeMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
