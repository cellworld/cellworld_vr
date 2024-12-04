#include "ExperimentDoorLobby.h"

#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"

AExperimentDoorLobby::AExperimentDoorLobby() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorLobby::AExperimentDoorLobby] Called"))
}

void AExperimentDoorLobby::OnValidEventTrigger() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorLobby::OnValidEventTrigger] Calling Super::OnValidEventTrigger"))
	Super::OnValidEventTrigger();
}

void AExperimentDoorLobby::Server_OnEventTrigger_Implementation() {
	Super::Server_OnEventTrigger_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorLobby::Server_OnEventTrigger_Implementation] Called from server."))
	if (!HasAuthority()) return;
	if (UWorld* World = GetWorld()) {
		AExperimentGameMode* ExperimentGameMode = Cast<AExperimentGameMode>(World->GetAuthGameMode());
		if (!ensure(ExperimentGameMode)) return;
		if (!ensure(ExperimentGameMode->ExperimentClient)) return;
		const FString Msg = FString("ExperimentGameMode->ExperimentClient->SetOcclusionVisibility(true)");
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentDoorLobby::Server_OnEventTrigger_Implementation] Calling: %s"), *Msg)
		ExperimentGameMode->ExperimentClient->SetOcclusionVisibility(true);
	}else {
		UE_LOG(LogTemp, Error,
			TEXT("[AExperimentDoorLobby::Server_OnEventTrigger_Implementation] UWorld is NULL"))
	}
}
