#include "ExperimentDoorExit.h"
#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"

AExperimentDoorExit::AExperimentDoorExit() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorExit::AExperimentDoorExit] Constructor"))
}

void AExperimentDoorExit::BeginPlay() {
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorExit::BeginPlay] Called super:: from EXIT"))
}

void AExperimentDoorExit::OnValidEventTrigger() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorExit::OnValidEventTrigger] Calling Super::OnValidEventTrigger"))
	Super::OnValidEventTrigger();
}

void AExperimentDoorExit::Server_OnEventTrigger_Implementation() {
	Super::Server_OnEventTrigger_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorExit::Server_OnEventTrigger_Implementation] Called from server."))
	if (!HasAuthority()) return;
	if (UWorld* World = GetWorld()) {
		AExperimentGameMode* ExperimentGameMode = Cast<AExperimentGameMode>(World->GetAuthGameMode());
		if (!ensure(ExperimentGameMode)) return;
		if (!ensure(ExperimentGameMode->ExperimentClient)) return;
		if (!ensure(ExperimentGameMode->ExperimentClient->ExperimentManager)) return;
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorExit::Server_OnEventTrigger_Implementation] forcing call: StartEpisode"))
		if (!ExperimentGameMode->ExperimentClient->ExperimentManager->IsInEpisode()) {
			UE_LOG(LogTemp, Warning,
						TEXT("[AExperimentDoorExit::Server_OnEventTrigger_Implementation] Not in episode. Not calling event"))
			return;
		}
		const FString Msg = 
		FString("ExperimentGameMode->ExperimentClient->SetOcclusionVisibility(false) && ..->StopEpisode()");
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentDoorExit::Server_OnEventTrigger_Implementation] Calling: %s"), *Msg)
		if (ExperimentGameMode->ExperimentClient->StopEpisode(false)){
			UE_LOG(LogTemp, Warning,
				TEXT("[AExperimentDoorExit::Server_OnEventTrigger_Implementation] ExperimentGameMode->ExperimentStopEpisode() false"))
		}
	}else {
		UE_LOG(LogTemp, Error,
			TEXT("[AExperimentDoorExit::Server_OnEventTrigger_Implementation] UWorld is NULL"))
	}
}
