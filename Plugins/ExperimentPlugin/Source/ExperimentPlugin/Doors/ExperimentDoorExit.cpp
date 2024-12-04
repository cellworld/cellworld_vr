#include "ExperimentDoorExit.h"

#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"

AExperimentDoorExit::AExperimentDoorExit() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorExit::AExperimentDoorExit] Called"))
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
		const FString Msg = FString("ExperimentGameMode->ExperimentClient->SetOcclusionVisibility(true)");
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentDoorExit::Server_OnEventTrigger_Implementation] Calling: %s"), *Msg)
		ExperimentGameMode->ExperimentClient->StopEpisode(false);
	}else {
		UE_LOG(LogTemp, Error,
			TEXT("[AExperimentDoorExit::Server_OnEventTrigger_Implementation] UWorld is NULL"))
	}
}
