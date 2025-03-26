#include "ExperimentPredator.h"

AExperimentPredator::AExperimentPredator() : Super() {
	UE_LOG(LogTemp, Log, TEXT("[APredatorBasic::APredatorBasic()]"));
	SetActorEnableCollision(false);
	SetReplicates(true);
	SetNetDormancy(ENetDormancy::DORM_Never);
	bNetLoadOnClient		 = true;
	NetUpdateFrequency	 = 100.0f;
	MinNetUpdateFrequency = 60.0f;
}

void AExperimentPredator::PostInitializeComponents() {
	Super::PostInitializeComponents();
	SetReplicateMovement(true);
}

void AExperimentPredator::BeginPlay() {
	Super::BeginPlay();
}

void AExperimentPredator::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AExperimentPredator::RevertMaterial() {
}

void AExperimentPredator::OnCapture() {
	
}

