#include "ExperimentGameState.h"
AExperimentGameState::AExperimentGameState() { bReplicates = true; }

void AExperimentGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// todo: add variables for replication
	//	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
}
