#include "ExperimentDoorLobby.h"
AExperimentDoorLobby::AExperimentDoorLobby() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorLobby::AExperimentDoorLobby] Called"))
}

void AExperimentDoorLobby::OnValidEventTrigger() {
	Super::OnValidEventTrigger();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorLobby::OnValidEventTrigger]"))
}

void AExperimentDoorLobby::Server_OnEventTrigger_Implementation() {
	Super::Server_OnEventTrigger_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorLobby::Server_OnEventTrigger_Implementation] Called from server."))
}
