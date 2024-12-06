#include "ExperimentDoorEntry.h"

#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"

AExperimentDoorEntry::AExperimentDoorEntry() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorEntry::AExperimentDoorEntry] Constructor"))
}

void AExperimentDoorEntry::BeginPlay() {
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorEntry::BeginPlay] Called super:: from ENTRY"))

	if (!DoorMesh) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentDoorEntry::BeginPlay] DoorMesh is NULL"));
	} else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorEntry::BeginPlay] DoorMesh is valid"));
	}
	
	if (!AnimationDoorTimeline || !AnimationDoorCurveVector) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentDoorEntry::BeginPlay] Timeline or CurveVector is NULL"));
	} else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorEntry::BeginPlay] Timeline and CurveVector are valid"));
	}
}

void AExperimentDoorEntry::OnValidEventTrigger() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorEntry::OnValidEventTrigger] before super"))
	Super::OnValidEventTrigger();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorEntry::OnValidEventTrigger] after super"))
}

void AExperimentDoorEntry::Server_OnEventTrigger_Implementation() {
	Super::Server_OnEventTrigger_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorEntry::Server_OnEventTrigger_Implementation] Called from server."))
	if (!HasAuthority()) return;
	if (UWorld* World = GetWorld()) {
		AExperimentGameMode* ExperimentGameMode = Cast<AExperimentGameMode>(World->GetAuthGameMode());
		if (!ensure(ExperimentGameMode)) return;
		if (!ensure(ExperimentGameMode->ExperimentClient)) return;
		if (!ensure(ExperimentGameMode->ExperimentClient->ExperimentManager)) return;
		const FString Msg = FString("ExperimentGameMode->ExperimentClient->StartEpisode()");
		UE_LOG(LogTemp, Warning,
			TEXT("[AExperimentDoorEntry::Server_OnEventTrigger_Implementation] Calling: %s"), *Msg)
		
		// if call is OK - don't allow for multiple calls 
		if(!ExperimentGameMode->ExperimentStartEpisode()) {
			UE_LOG(LogTemp, Warning,
				TEXT("[AExperimentDoorEntry::Server_OnEventTrigger_Implementation] called: false."))
			SetCanCallEventTrigger(true);
		}else {
			UE_LOG(LogTemp, Warning,
            				TEXT("[AExperimentDoorEntry::Server_OnEventTrigger_Implementation] Called: true"))
		}
	}else {
		UE_LOG(LogTemp, Warning,
			TEXT("[AExperimentDoorEntry::Server_OnEventTrigger_Implementation] UWorld is NULL"))
	}
}

void AExperimentDoorEntry::OnEventCooldownFinished() {
	UE_LOG(LogTemp, Warning,
			TEXT("[AExperimentDoorEntry::OnEventCooldownFinished] before calling: Super::"))
	Super::OnEventCooldownFinished();
}
