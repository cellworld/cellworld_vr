#include "ExperimentGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "ExperimentPlugin/PlayerControllers/ExperimentPlayerControllerVR.h"
#include "ExperimentPlugin/Characters/ExperimentCharacter.h"
#include "ExperimentPlugin/Doors/ExperimentDoorBase.h"
#include "ExperimentPlugin/GameStates/ExperimentGameState.h"
#include "ExperimentPlugin/PlayerStates/ExperimentPlayerState.h"

AExperimentGameMode::AExperimentGameMode(){
	UE_LOG(LogTemp, Log, TEXT("Initializing AExperimentGameMode()"))

	PlayerStateClass      = AExperimentPlayerState::StaticClass(); 
	GameStateClass        = AExperimentGameState::StaticClass();
	PlayerControllerClass = AExperimentPlayerControllerVR::StaticClass();
	
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	
	bStartPlayersAsSpectators = false;
}

void AExperimentGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) {
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::InitGame] Called"))
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::InitGame] Spawning ExperimentClient!"))
	SpawnExperimentServiceMonitor();
	if (!ensure(ExperimentClient->IsValidLowLevelFast())) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentGameMode::InitGame] Failed to spawn ExperimentClient!"))
	}
}

void AExperimentGameMode::InitGameState() {
	Super::InitGameState();
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It) {
		FreePlayerStarts.Add(*It);
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::InitGameState] Found player start: %s"), *(*It)->GetName())
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
}

void AExperimentGameMode::UpdateNetOwnerHabitat(AController* InHabitatNetOwnerController, const bool bForceUpdate) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::UpdateNetOwnerHabitat] Called"))
	if (!ensure(InHabitatNetOwnerController)) { return; }
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::UpdateNetOwnerHabitat] Controller is valid: %s | bForceUpdate: %i"),
		*InHabitatNetOwnerController->GetName(), bForceUpdate)

	check(Habitat)
	if (!ensure(Habitat)) { return; }
	
	if (!Habitat->HasNetOwner() || bForceUpdate) {
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentGameMode::UpdateNetOwnerHabitat] Updating Habitat NetOwner. Habitat does not have owner or you pass `force` flag (bForceUpdate = %i)"),
			bForceUpdate)
		Habitat->SetOwner(InHabitatNetOwnerController);
        Habitat->SetReplicateMovement(true);
        TArray<AActor*> ChildActors;
        Habitat->GetAllChildActors(ChildActors,true);
        for (AActor* ChildActor : ChildActors) {
            if (ChildActor) {
            	ChildActor->SetOwner(InHabitatNetOwnerController);
            	ChildActor->SetReplicates(true);
            	UE_LOG(LogTemp, Log,
            		TEXT("[AExperimentGameMode::UpdateNetOwnerHabitat] Updated owner for child actor: %s to %s (HasNetOwner?: %i)"),
            		*ChildActor->GetName(),
            		*InHabitatNetOwnerController->GetName(),
            		ChildActor->HasNetOwner());
            }
        }
	}
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::UpdateNetOwnerHabitat] Exiting"))
}

void AExperimentGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
}

void AExperimentGameMode::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AExperimentGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Error, TEXT("[AExperimentGameMode::PostLogin] called "));
	if (NewPlayer) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::PostLogin] New PlayerController: %s"), *NewPlayer->GetName());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentGameMode::PostLogin] PlayerController is NULL during PostLogin!"));
	}
	
	if (!ensure(Habitat)) return;


	// if (!ensure(Habitat->HasNetOwner())) return;
	
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
	
	if (!ensure(NewPlayer->IsValidLowLevelFast())) { return; }
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::OnPostLogin] NewPlayer PlayerController valid!"));

	// todo: move this to: handled by call UpdateNetOwnerHabitat from BP_ExperimentCharacter if Habitat has no owner
	if (!Habitat && !ensure((Habitat = FindHabitatInLevel()))) { return; }
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::OnPostLogin] Habitat found. "))
	if (!Habitat->HasNetOwner()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::OnPostLogin] Habitat has no owner. Setting to AController: %s"),
			*NewPlayer->GetName())
		UpdateNetOwnerHabitat(NewPlayer, true);
	} else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::OnPostLogin] Habitat already has an owner! %s"),
			*Habitat->GetNetOwner()->GetName());
	}
	
	if (ExperimentClient && ensure(ExperimentClient->ExperimentManager->IsValidLowLevelFast())) {
		ExperimentClient->PlayerIndex = ExperimentClient->ExperimentManager->RegisterNewPlayer(NewPlayer);
		ExperimentClient->ExperimentManager->SetActivePlayerIndex(ExperimentClient->PlayerIndex);
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentGameMode::OnPostLogin] Registered new player. PlayerName: %s (id: %i)"),
			*NewPlayer->GetName(),ExperimentClient->PlayerIndex)
	}else {
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentGameMode::OnPostLogin] ExperimentClient or ExperimentManager are NULL"))
	}
}

void AExperimentGameMode::Logout(AController* Exiting) {
	Super::Logout(Exiting);
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::Logout] Player logged out: %s"), *Exiting->GetName());
}

TObjectPtr<AHabitat> AExperimentGameMode::FindHabitatInLevel() const {
	TObjectPtr<AHabitat> HabitatTemp = nullptr;
	for (TActorIterator<AHabitat> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
		HabitatTemp = *ActorItr;
		if (HabitatTemp) {
			UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::FindHabitatInLevel] Found Maze Actor: %s"),
				*HabitatTemp->GetName());
			return HabitatTemp;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentGameMode::FindHabitatInLevel] No Maze Actor found!"));
	return HabitatTemp; 
}

void AExperimentGameMode::OnUpdatePreyPosition(const FVector& InLocation, const FRotator& InRotation) {
	// UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::OnUpdatePreyPosition] InLocation: %s | InRotation: %s"),
	// 		*InLocation.ToString(), *InRotation.ToString())
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

void AExperimentGameMode::SpawnHabitat(const FVector& InLocation, const int& InScale) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::SpawnHabitat] Called!"))

	if (MyActorClass != nullptr) {
		// Get the world context
		UWorld* World = GetWorld();
		if (World) {
			// Define spawn parameters
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this; // Optional: Set the owner
			// SpawnParams.Instigator = GetInstigator(); // Optional: Set the instigator
			// Define spawn location and rotation
			FVector SpawnLocation = FVector(0.0f, 0.0f, 0.0f);
			FRotator SpawnRotation = FRotator::ZeroRotator;

			// Spawn the actor
			AActor* SpawnedActor = World->SpawnActor<AActor>(MyActorClass, SpawnLocation, SpawnRotation, SpawnParams);

			if (SpawnedActor) {
				UE_LOG(LogTemp, Warning, TEXT("[AExperimentGameMode::SpawnHabitat] Spawned actor"));
			}
		}
	} else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentGameMode::SpawnHabitat] MyActorClass is null. Check your Blueprint assignment."));
	}
	UE_LOG(LogTemp, Log, TEXT("[AExperimentGameMode::SpawnHabitat] Exiting OK"))

}
