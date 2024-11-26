#include "ExperimentGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "ExperimentPlugin/PlayerControllers/ExperimentPlayerControllerVR.h"
#include "ExperimentPlugin/Characters/ExperimentCharacter.h"
#include "ExperimentPlugin/GameStates/ExperimentGameState.h"
#include "ExperimentPlugin/PlayerStates/ExperimentPlayerState.h"

AExperimentGameMode::AExperimentGameMode(){
	UE_LOG(LogTemp, Log, TEXT("Initializing AExperimentGameMode()"))

	PlayerStateClass      = AExperimentPlayerState::StaticClass(); 
	GameStateClass        = AExperimentGameState::StaticClass();
	DefaultPawnClass      = AExperimentCharacter::StaticClass();
	PlayerControllerClass = AExperimentPlayerControllerVR::StaticClass();
	
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	
	bStartPlayersAsSpectators = false;
}

void AExperimentGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AExperimentGameMode::InitGameState() {
	Super::InitGameState();
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It) {
		FreePlayerStarts.Add(*It);
		UE_LOG(LogTemp, Log, TEXT("Found player start: %s"), *(*It)->GetName())
	}
}

void AExperimentGameMode::SpawnExperimentServiceMonitor() {
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

void AExperimentGameMode::StartPlay() {
	Super::StartPlay();
	UE_LOG(LogTemp, Log, TEXT("StartPlay()"))
	if (GetNetMode() == NM_DedicatedServer) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::StartPlay] Running on a dedicated server."))
	}

	// todo: get Worldscale from GameInstance

#if !WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::StartPlay] Spawning SpawningClient!"))
	SpawnExperimentServiceMonitor();
	if (!ensure(ExperimentClient->IsValidLowLevelFast())) {
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn ExperimentClient!"))
		return;
	}
#endif
}

void AExperimentGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
}

void AExperimentGameMode::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	UE_LOG(LogTemp, Log, TEXT("Player Count: %i"), NumPlayers)
	if (NumPlayers == 1) {
	}
	// UWorld* World = GetWorld(); // can be true
}

void AExperimentGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Error, TEXT("[AExperimentGameMode::PostLogin] called "));
	if (NewPlayer) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::PostLogin] PlayerController created: %s"), *NewPlayer->GetName());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentGameMode::PostLogin] PlayerController is NULL during PostLogin!"));
	}

	// if (GameState) {
	// 	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	// 	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::PostLogin] Number of Players: %i"),NumberOfPlayers);
	// }
	//
	// APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	// if (PlayerState) {
	// 	const FString PlayerName = PlayerState->GetPlayerName();
	// 	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::PostLogin] Player Name: %s"), *PlayerName)
	// }
}

void AExperimentGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) {
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::HandleStartingNewPlayer_Implementation] Called"))

	if (!NewPlayer->IsValidLowLevel()) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentGameMode::HandleStartingNewPlayer_Implementation] NewPlayer PlayerController NULL"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentGameMode::HandleStartingNewPlayer_Implementation] NewPlayer PlayerController Valid"));

	if (NewPlayer->PlayerState) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::HandleStartingNewPlayer_Implementation] bOnlySpectator: %i"), NewPlayer->PlayerState->IsOnlyASpectator());
	}
	
	ACharacter* CharacterTemp = NewPlayer->GetCharacter();
	if (!CharacterTemp) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::HandleStartingNewPlayer_Implementation] ACharacter NULL"))
		return;
	}

	ExperimentCharacter = Cast<AExperimentCharacter>(CharacterTemp);
	if (ExperimentCharacter) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::HandleStartingNewPlayer_Implementation] AExperimentCharacter valid"))
		// ExperimentPawn->MultiDelegate_UpdateMovement.AddDynamic(this,&AExperimentGameMode::HandleUpdatePosition);
	}else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::HandleStartingNewPlayer_Implementation] AExperimentCharacter NULL"))
	}
}

void AExperimentGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (!NewPlayer->GetPawn()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentGameMode::RestartPlayer] Pawn not spawned for PlayerController: %s"), *NewPlayer->GetName());
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::RestartPlayer] Player pawn spawned: %s"), *NewPlayer->GetPawn()->GetName());
	}
}

AActor* AExperimentGameMode::ChoosePlayerStart_Implementation(AController* Player) {
	AActor* StartSpot = Super::ChoosePlayerStart_Implementation(Player);
	if (!StartSpot) {
		UE_LOG(LogTemp, Warning, TEXT("[ChoosePlayerStart] No valid PlayerStart found! Using fallback."))
		return GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(0, 0, 300), FRotator::ZeroRotator);
	}
	return StartSpot;
}

void AExperimentGameMode::OnPostLogin(AController* NewPlayer) {
	Super::OnPostLogin(NewPlayer);
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentGameMode::OnPostLogin] Called"));
	
	// int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	int32 NumberOfPlayers = 0; 
	if (!ensure(NewPlayer->IsValidLowLevelFast())) { return; }
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::OnPostLogin] NewPlayer PlayerController valid!"));
	
	// ExperimentClient->SetupPlayerUpdatePosition(NewPlayer->GetPawn());
	if (ensure(ExperimentClient->ExperimentManager->IsValidLowLevelFast())) {
		ExperimentClient->PlayerIndex = ExperimentClient->ExperimentManager->RegisterNewPlayer(NewPlayer->GetPawn());
		ExperimentClient->ExperimentManager->SetActivePlayerIndex(ExperimentClient->PlayerIndex);
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::OnPostLogin] Registered new player pawn. Client->Idx: %i"),
			ExperimentClient->PlayerIndex)
	}
}

void AExperimentGameMode::Logout(AController* Exiting) {
	Super::Logout(Exiting);
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::Logout] Player logged out: %s"), *Exiting->GetName());
}

void AExperimentGameMode::OnUpdatePreyPosition(const FVector& InLocation, const FRotator& InRotation) {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentGameMode::OnUpdatePreyPosition] InLocation: %s | InRotation: %s"),
			*InLocation.ToString(), *InRotation.ToString())
	if (!ensure(ExperimentClient->IsValidLowLevelFast())){ return; }
	if (!ensure(ExperimentClient->TrackingClient)) { return; }
	if (!ensure(ExperimentClient->TrackingClient->IsConnected())) { return; }
	UE_LOG(LogTemp, Warning,
		TEXT("[AExperimentGameMode::OnUpdatePreyPosition] Calling: ExperimentClient->UpdatePreyPosition(...,...)"))
	ExperimentClient->UpdatePreyPosition(InLocation, InRotation);
		
}

void AExperimentGameMode::HandleUpdatePosition(const FVector InLocation, const FRotator InRotation) {
	UE_LOG(LogTemp,Log, TEXT("[AExperimentGameMode::HandleUpdatePosition] Received -> Location: %s | Location: %s"),
		*InLocation.ToString(), *InRotation.ToString())
}

bool AExperimentGameMode::StartPositionSamplingTimer(const float InRateHz) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::StartPositionSamplingTimer] DEPRECATED"))
	return false;
	EventTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	if (EventTimer->IsValidLowLevel()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::StartPositionSamplingTimer] Starting at %0.2f Hz."), InRateHz)
		EventTimer->SetRateHz(InRateHz); //todo: make sampling rate GI variable (or somewhere relevant) 
		EventTimer->bLoop = true;
		
		// EventTimer->OnTimerFinishedDelegate.AddDynamic(this,
		// 	&AExperimentGameMode::OnUpdatePreyPosition);
		
		if (!EventTimer->Start()) { return false; }
	} else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentGameMode::StartPositionSamplingTimer] Failed! EventTimer is NULL!"))
		return false;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::StartPositionSamplingTimer] OK!"))
	return true;
}

FString AExperimentGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::InitNewPlayer] Called"));
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);;
}

bool AExperimentGameMode::ExperimentStartEpisode() {
	if (!ensure(IsValid(ExperimentClient))) { return false; }
	UE_LOG(LogTemp, Log, TEXT("[AGameModeMain::ExperimentStartEpisode] Calling StartEpisode()"))
	return ExperimentClient->StartEpisode();
}

bool AExperimentGameMode::ExperimentStopEpisode() {
	if (!ensure(IsValid(ExperimentClient))) {
		UE_LOG(LogTemp, Warning,
			   TEXT("[AGameModeMain::ExperimentStopEpisode] Failed to destroy, Already pending kill."));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("[AGameModeMain::ExperimentStopEpisode] Calling StopEpisode(false)"))
	return ExperimentClient->StopEpisode(false);
}
