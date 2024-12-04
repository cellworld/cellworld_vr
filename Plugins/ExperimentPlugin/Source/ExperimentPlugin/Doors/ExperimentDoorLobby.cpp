#include "ExperimentDoorLobby.h"
AExperimentDoorLobby::AExperimentDoorLobby() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorLobby::AExperimentDoorLobby] Called"))
}

void AExperimentDoorLobby::OnValidEventTrigger() {
	Super::OnValidEventTrigger();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorLobby::OnValidEventTrigger]"))

}
