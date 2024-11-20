// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BotEvadeGameMode.h"
#include "BotEvadeModule/BotEvadeModule.h"
#include "CharacterComponents/MenuCharacterVR.h"
#include "PlayerStates/BotEvadePlayerState.h"
#include "GameStates/BotEvadeGameState.h"
#include "Subsystems/MultiplayerSubsystem.h"

DEFINE_LOG_CATEGORY(LogBotEvadeGameMode);

ABotEvadeGameMode::ABotEvadeGameMode(){

	UE_LOG(LogBotEvadeGameMode, Log, TEXT("Initializing ABotEvadeGameMode()"))

	PlayerStateClass = ABotEvadePlayerState::StaticClass(); 
	GameStateClass   = ABotEvadeGameState::StaticClass();
	DefaultPawnClass = AMenuCharacterVR::StaticClass(); 
	
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

void ABotEvadeGameMode::SpawnExperimentServiceMonitor() {
	if (GetWorld()) {
		// ReSharper disable once CppLocalVariableMayBeConst
		ESpawnActorCollisionHandlingMethod CollisionHandlingMethod =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		FTransform SpawnTransformExperimentServiceMonitor = {};
		SpawnTransformExperimentServiceMonitor.SetLocation(FVector::ZeroVector);
		SpawnTransformExperimentServiceMonitor.SetRotation(FRotator::ZeroRotator.Quaternion());
		
		ExperimentServiceMonitor = GetWorld()->SpawnActorDeferred<AExperimentServiceMonitor>(
			AExperimentServiceMonitor::StaticClass(), SpawnTransformExperimentServiceMonitor, this, nullptr, CollisionHandlingMethod);
		ExperimentServiceMonitor->WorldScale = this->WorldScale;
		ExperimentServiceMonitor->FinishSpawning(SpawnTransformExperimentServiceMonitor);
	}
}

void ABotEvadeGameMode::StartPlay() {
	Super::StartPlay();
	UE_LOG(LogBotEvadeGameMode, Log, TEXT("StartPlay()"))
	if (GetNetMode() == NM_DedicatedServer) {
		LOG("[ABotEvadeGameMode::StartPlay] Running on a dedicated server.");
	}

	
	
	// todo: connect to server
	
}

void ABotEvadeGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
}

void ABotEvadeGameMode::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	// UWorld* World = GetWorld(); // can be true
}

void ABotEvadeGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	LOG("[ABotEvadeGameMode::PostLogin] Called");

	// int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	int32 NumberOfPlayers = 0; 

	UGameInstance* GameInstance = GetGameInstance();
	if (ensure(GameInstance)) {
		LOG("[ABotEvadeGameMode::PostLogin] GI Found");

		UMultiplayerSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSubsystem>();
		check(Subsystem);

		// if we have everyone logged in
		// if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections) 
		if (true) {
			UE_LOG(LogBotEvadeGameMode, Warning, TEXT("PostLogin: Skipped `if(NumberOfPlayers == Subsystem->DesiredNumPublicConnections)` "));
		}
	}
	
	// todo: ExperimentServiceMonitor->SetupPlayerPawn()
	// todo: ExperimentServiceMonitor->RegisterPlayer()
}

void ABotEvadeGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) {
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	LOG("[ABotEvadeGameMode::HandleStartingNewPlayer_Implementation] Called");

	if (!NewPlayer->IsValidLowLevel()) {
		LOG_WARNING("[ABotEvadeGameMode::HandleStartingNewPlayer_Implementation] NewPlayer PlayerController NULL");
		return;
	} 
	LOG("[ABotEvadeGameMode::HandleStartingNewPlayer_Implementation] NewPlayer PlayerController valid!");

}

