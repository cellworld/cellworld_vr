#include "ExperimentComponents/ARBotEvadeGameMode.h"
#include "EngineUtils.h"
#include "GameInstanceMain.h"
#include "AsyncLoadingScreenLibrary.h"
#include "MouseKeyboardPlayerController.h"
#include "PlayerControllerVR.h"
#include "cellworld_vr/cellworld_vr.h"

// todo: create ABotEvadeGameState

AARBotEvadeGameMode::AARBotEvadeGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	PrimaryActorTick.bStartWithTickEnabled = true;
	GameStateClass = AARSharedWorldGameMode::StaticClass();
	PlayerControllerClass = AARSharedWorldPlayerController::StaticClass(); 
}

void AARBotEvadeGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage) {
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AARBotEvadeGameMode::SpawnExperimentServiceMonitor() {
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

bool AARBotEvadeGameMode::ExperimentStartEpisode() {
	if (!bSpawnExperimentService) {
		UE_LOG(LogExperiment, Log,
			TEXT("[AARBotEvadeGameMode::ExperimentStartEpisode] ExperimentServiceMonitor was not spawned!"))
		return true;
	}
	if (!ensure(IsValid(ExperimentServiceMonitor))) { return false; }
	UE_LOG(LogExperiment, Log, TEXT("[AARBotEvadeGameMode::ExperimentStartEpisode] Calling StartEpisode()"))
	return ExperimentServiceMonitor->StartEpisode();
	
}

bool AARBotEvadeGameMode::ExperimentStopEpisode() {
	if (!bSpawnExperimentService) {
		UE_LOG(LogExperiment, Log,
			TEXT("[AARBotEvadeGameMode::ExperimentStopEpisode] ExperimentServiceMonitor was not spawned!"))
		return true;
	}
	if (!ensure(IsValid(ExperimentServiceMonitor))) { return false; }
	UE_LOG(LogExperiment, Log, TEXT("[AARBotEvadeGameMode::ExperimentStopEpisode] Calling StopEpisode(false)"))
	return ExperimentServiceMonitor->StopEpisode(false);
	
}

void AARBotEvadeGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	if (bSpawnExperimentService && this->ExperimentServiceMonitor->IsValidLowLevelFast()) {
		this->ExperimentStopEpisode();
		// this->ExperimentStopExperiment(ExperimentServiceMonitor->ExperimentNameActive);
	}
}

void AARBotEvadeGameMode::EndMatch() {
	Super::EndMatch();
}

void AARBotEvadeGameMode::StartPlay() {
	Super::StartPlay();
	UE_LOG(LogExperiment, Warning, TEXT("[AARBotEvadeGameMode::StartPlay()] Starting game!"));

	/* spawn player */
	FLocation SpawnLocation;
	SpawnLocation.x = 0.0f;
	SpawnLocation.y = 0.4f;

	FVector SpawnLocationVR = UExperimentUtils::CanonicalToVr(SpawnLocation, 235.185, this->WorldScale);

	UGameInstanceMain* GameInstance = Cast<UGameInstanceMain>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogExperiment, Error, TEXT("[AARBotEvadeGameMode::OnSpawnExperimentServerCheckBoxStateChanged] GameInstance NULL!"));
		return;
	}
	
	/* spawn ExperimentServiceActor */
	if (GameInstance->ExperimentParameters.IsValid()) {
		bSpawnExperimentService = GameInstance->ExperimentParameters->bSpawnExperimentService;
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[AARBotEvadeGameMode::StartPlay] GameInstance->ExperimentParameters NULL"))
	}

	// todo: make sure this doesn't break if we don't use main menu 
	if (bSpawnExperimentService) {
		UE_LOG(LogExperiment, Warning, TEXT("[AARBotEvadeGameMode::StartPlay] Spawning Experiment Service!"));
		this->SpawnExperimentServiceMonitor();
	} else {
		UE_LOG(LogExperiment, Warning, TEXT("[AARBotEvadeGameMode::StartPlay] Not spawning Experiment Service!"));
	}
}
