#include "ExperimentDoorEntry.h"

#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"

AExperimentDoorEntry::AExperimentDoorEntry() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorEntry::AExperimentDoorEntry] Called"))
}

void AExperimentDoorEntry::OnValidEventTrigger() {
	Super::OnValidEventTrigger();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorEntry::OnValidEventTrigger]"))
}

void AExperimentDoorEntry::Server_OnEventTrigger_Implementation() {
	Super::Server_OnEventTrigger_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorEntry::Server_OnEventTrigger_Implementation] Called from server."))
	if (!HasAuthority()) return;
	if (UWorld* World = GetWorld()) {
		AExperimentGameMode* ExperimentGameMode = Cast<AExperimentGameMode>(World->GetAuthGameMode());
		if (!ensure(ExperimentGameMode)) return;
		if (!ensure(ExperimentGameMode->ExperimentClient)) return;
		const FString Msg = FString("ExperimentGameMode->ExperimentClient->StartEpisode()");
		UE_LOG(LogTemp, Warning,
			TEXT("[AExperimentDoorEntry::Server_OnEventTrigger_Implementation] Calling: %s"), *Msg)
		ExperimentGameMode->ExperimentClient->StartEpisode();
	}else {
		UE_LOG(LogTemp, Error,
			TEXT("[AExperimentDoorEntry::Server_OnEventTrigger_Implementation] UWorld is NULL"))
	}
}
