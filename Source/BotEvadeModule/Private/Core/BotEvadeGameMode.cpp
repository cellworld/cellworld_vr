// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BotEvadeGameMode.h"
#include "BotEvadeModule/BotEvadeModule.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerStates/BotEvadePlayerState.h"
#include "GameStates/BotEvadeGameState.h"
#include "EngineUtils.h"
#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "ExperimentPlugin/PlayerControllers/ExperimentPlayerControllerVR.h"
#include "Subsystems/MultiplayerSubsystem.h"
#include "Client/ExperimentClient.h"

DEFINE_LOG_CATEGORY(LogBotEvadeGameMode);

ABotEvadeGameMode::ABotEvadeGameMode(){
	UE_LOG(LogBotEvadeGameMode, Log, TEXT("Initializing ABotEvadeGameMode()"))

	PlayerStateClass      = ABotEvadePlayerState::StaticClass(); 
	GameStateClass        = ABotEvadeGameState::StaticClass();
	DefaultPawnClass      = AExperimentPawn::StaticClass();
	PlayerControllerClass = AExperimentPlayerControllerVR::StaticClass();
	
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	
	bStartPlayersAsSpectators = false;
}

void ABotEvadeGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ABotEvadeGameMode::InitGameState() {
	Super::InitGameState();
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It) {
		FreePlayerStarts.Add(*It);
		UE_LOG(LogTemp, Log, TEXT("Found player start: %s"), *(*It)->GetName())
	}
}

void ABotEvadeGameMode::SpawnExperimentServiceMonitor() {
	if (GetWorld()) {
		constexpr ESpawnActorCollisionHandlingMethod CollisionHandlingMethod =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		FTransform SpawnTransformExperimentClient = {};
		SpawnTransformExperimentClient.SetLocation(FVector::ZeroVector);
		SpawnTransformExperimentClient.SetRotation(FRotator::ZeroRotator.Quaternion());
		
		ExperimentClient = GetWorld()->SpawnActorDeferred<AExperimentClient>(
			AExperimentClient::StaticClass(), SpawnTransformExperimentClient, this, nullptr, CollisionHandlingMethod);
		ExperimentClient->WorldScale = this->WorldScale;
		ExperimentClient->FinishSpawning(SpawnTransformExperimentClient);
		ExperimentClient->AddToRoot();
	}
}

void ABotEvadeGameMode::StartPlay() {
	Super::StartPlay();
	UE_LOG(LogBotEvadeGameMode, Log, TEXT("StartPlay()"))
	if (GetNetMode() == NM_DedicatedServer) {
		UE_LOG(LogTemp, Log, TEXT("[ABotEvadeGameMode::StartPlay] Running on a dedicated server."))
	}
	
	// todo: connect to server
	SpawnExperimentServiceMonitor();
	if (!ensure(ExperimentClient->IsValidLowLevelFast())) {
		UE_LOG(LogBotEvade, Error, TEXT("Failed to spawn ExperimentClient!"))
		return;
	}
	StartPositionSamplingTimer(30.0f);
}

void ABotEvadeGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
}

void ABotEvadeGameMode::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	UE_LOG(LogTemp, Log, TEXT("Player Count: %i"), NumPlayers)
	if (NumPlayers == 1) {
	}
	// UWorld* World = GetWorld(); // can be true
}

void ABotEvadeGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Error, TEXT("[ABotEvadeGameMode::PostLogin] called "));
	if (NewPlayer) {
		UE_LOG(LogTemp, Log, TEXT("[ABotEvadeGameMode::PostLogin] PlayerController created: %s"), *NewPlayer->GetName());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[ABotEvadeGameMode::PostLogin] PlayerController is NULL during PostLogin!"));
	}

	if (GameState) {
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		LOG("[ABotEvadeGameMode::PostLogin] Number of Players: %i",NumberOfPlayers);
	}

	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	if (PlayerState) {
		const FString PlayerName = PlayerState->GetPlayerName();
		UE_LOG(LogTemp, Log, TEXT("[ABotEvadeGameMode::PostLogin] Player Name: %s"), *PlayerName)
	}
}

void ABotEvadeGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) {
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	UE_LOG(LogTemp, Log, TEXT("[ABotEvadeGameMode::HandleStartingNewPlayer_Implementation] Called"))

	if (!NewPlayer->IsValidLowLevel()) {
		UE_LOG(LogTemp, Warning, TEXT("[ABotEvadeGameMode::HandleStartingNewPlayer_Implementation] NewPlayer PlayerController NULL"));
		return;
	}
	
	if (NewPlayer->PlayerState) {
		UE_LOG(LogTemp, Log, TEXT("bOnlySpectator: %i"), NewPlayer->PlayerState->IsOnlyASpectator());
	}
}

void ABotEvadeGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (!NewPlayer->GetPawn()) {
		UE_LOG(LogTemp, Error, TEXT("[RestartPlayer] Pawn not spawned for PlayerController: %s"), *NewPlayer->GetName());
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("[RestartPlayer] Player pawn spawned: %s"), *NewPlayer->GetPawn()->GetName());
	}
}

AActor* ABotEvadeGameMode::ChoosePlayerStart_Implementation(AController* Player) {
	AActor* StartSpot = Super::ChoosePlayerStart_Implementation(Player);
	if (!StartSpot) {
		UE_LOG(LogTemp, Warning, TEXT("[ChoosePlayerStart] No valid PlayerStart found! Using fallback."))
		return GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(0, 0, 300), FRotator::ZeroRotator);
	}
	return StartSpot;
}

void ABotEvadeGameMode::OnPostLogin(AController* NewPlayer) {
	Super::OnPostLogin(NewPlayer);
	UE_LOG(LogBotEvadeGameMode, Warning, TEXT("[ABotEvadeGameMode::OnPostLogin] Called"));

// 	// int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
// 	int32 NumberOfPlayers = 0; 
// 	
// 	UGameInstance* GameInstance = GetGameInstance();
// 	if (ensure(GameInstance)) {
// 		LOG("[ABotEvadeGameMode::OnPostLogin] GI Found");
//
// 		UMultiplayerSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSubsystem>();
// 		check(Subsystem);
//
// 		// if we have everyone logged in
// 		// if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections) 
// 		if (true) {
// 			UE_LOG(LogBotEvadeGameMode, Warning, TEXT("OnPostLogin: Skipped `if(NumberOfPlayers == Subsystem->DesiredNumPublicConnections)` "));
// 		}
// 	}
//
// 	LOG("[ABotEvadeGameMode::OnPostLogin] NewPlayer PlayerController valid!");
// 	
// 	ExperimentClient->SetupPlayerUpdatePosition(NewPlayer->GetPawn());
// 	if (ensure(ExperimentClient->ExperimentManager->IsValidLowLevelFast())) {
// 		ExperimentClient->PlayerIndex = ExperimentClient->ExperimentManager->RegisterNewPlayer(NewPlayer->GetPawn());
// 		ExperimentClient->ExperimentManager->SetActivePlayerIndex(ExperimentClient->PlayerIndex);
// 		LOG("[ABotEvadeGameMode::OnPostLogin] Registered new player pawn. Client->Idx: %i",
// 			ExperimentClient->PlayerIndex)
// 	}
// 	// todo: ExperimentServiceMonitor->SetupPlayerPawn()
// 	// todo: ExperimentServiceMonitor->RegisterPlayer()
}

void ABotEvadeGameMode::Logout(AController* Exiting) {
	Super::Logout(Exiting);
	UE_LOG(LogTemp, Log, TEXT("Player logged out: %s"), *Exiting->GetName());
}

void ABotEvadeGameMode::OnUpdatePreyPosition() {
	UE_LOG(LogTemp,Log, TEXT("[ABotEvadeGameMode::OnUpdatePreyPosition] Called!"))

	if (NumPlayers == 0) {
		UE_LOG(LogTemp, Error,TEXT("[ABotEvadeGameMode::OnUpdatePreyPosition] No Players found to update position!"))
		return;
	}
	if (!GameState) {
		UE_LOG(LogTemp, Error,TEXT("[ABotEvadeGameMode::OnUpdatePreyPosition] GameState NULL"))
		return;
	}

	TArray<TObjectPtr<APlayerState>> PlayerStatesArr = GameState.Get()->PlayerArray;
	TObjectPtr<APlayerState> PlayerStateTemp = nullptr;
	UE_LOG(LogTemp, Log, TEXT("[ABotEvadeGameMode::OnUpdatePreyPosition] Number of Players: %i"), GameState.Get()->PlayerArray.Num());
	if (PlayerStatesArr.Num() <= 0) {
		UE_LOG(LogTemp, Error,TEXT("PlayerStatesArr is empty!"))

		return;
	}

	// get first player state
	PlayerStateTemp = GameState.Get()->PlayerArray[0];
	if (!PlayerStateTemp) {
		UE_LOG(LogTemp, Error,TEXT("PlayerStateTemp NULL"))
		return;
	}

	// get first player pawn
	APawn* PawnTemp = PlayerStateTemp->GetPawn();
	if (!PawnTemp) {
		UE_LOG(LogTemp, Error,TEXT("PawnTemp NULL"))
		return;
	}

	// cast to aexperimentpawn
	AExperimentPawn* ExperimentPawn = Cast<AExperimentPawn>(PawnTemp);
	if (!ExperimentPawn) {
		UE_LOG(LogTemp, Error,TEXT("ExperimentPawn NULL"))
		return;
	}

	// if all checks valid: 
	if (!ExperimentPawn->Camera->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Error,TEXT("ExperimentPawn->Camera NULL"))
		return;
	}

	const FVector  PawnLocation = ExperimentPawn->GetActorLocation();
	const FRotator PawnRotation = ExperimentPawn->GetActorRotation();
	UE_LOG(LogTemp, Log, TEXT("PawnLocation: %s"), *PawnLocation.ToString())
	UE_LOG(LogTemp, Log, TEXT("PawnRotation: %s"), *PawnRotation.ToString())
	if (ExperimentClient->IsValidLowLevelFast()) {
		ExperimentClient->UpdatePreyPosition(PawnLocation, PawnRotation);
		UE_LOG(LogTemp, Log, TEXT("SENT FRAME!"))
		
	}
}

bool ABotEvadeGameMode::StartPositionSamplingTimer(const float InRateHz) {
	UE_LOG(LogTemp, Log, TEXT("StartPositionSamplingTimer"))
	EventTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	if (EventTimer->IsValidLowLevel()) {
		UE_LOG(LogTemp, Log, TEXT("StartPositionSamplingTimer: Starting at %0.2f Hz."), InRateHz)
		EventTimer->SetRateHz(InRateHz); //todo: make sampling rate GI variable (or somewhere relevant) 
		EventTimer->bLoop = true;
		
		EventTimer->OnTimerFinishedDelegate.AddDynamic(this,
			&ABotEvadeGameMode::OnUpdatePreyPosition);
		
		if (!EventTimer->Start()) { return false; }
	} else {
		UE_LOG(LogTemp, Error, TEXT("StartPositionSamplingTimer Failed! EventTimer is NULL!"))
		return false;
	}
	
	UE_LOG(LogTemp, Log, TEXT("StartPositionSamplingTimer OK!"))
	return true;
}

FString ABotEvadeGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal) {
	UE_LOG(LogTemp, Log, TEXT("[ABotEvadeGameMode::InitNewPlayer] Called"));

	// Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	// if (FreePlayerStarts.Num() == 0) {
	// 	LOG("No free player starts in InitNewPlayer");
	// 	return FString(TEXT("No free player starts"));
	// }
	//
	// NewPlayerController->StartSpot = FreePlayerStarts.Pop();
	// FString FormattedString = FString::Printf(
	// 	TEXT("Using player start %s for %s"),
	// 	*NewPlayerController->StartSpot->GetName(),
	// 	*NewPlayerController->GetName() );
	//
	// LOG("%s", *FormattedString);

	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);;
}

