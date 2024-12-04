#include "ExperimentPlayerControllerVR.h"
#include "ExperimentPlugin/Characters/ExperimentCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

class AGameModeMain; 

AExperimentPlayerControllerVR::AExperimentPlayerControllerVR() {
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
}

void AExperimentPlayerControllerVR::BeginPlay() {
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentPlayerControllerVR::BeginPlay]"));

	if (GetWorld()) {
		PossessedCharacter = Cast<AExperimentCharacter>(GetCharacter());
	}else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPlayerControllerVR::BeginPlay] GetWorld() NULL"))
	}
	return;
}

// Called to bind functionality to input
void AExperimentPlayerControllerVR::SetupInputComponent() {
	Super::SetupInputComponent();

	InputComponent->BindAction("ResetOrigin", EInputEvent::IE_Pressed, this, &AExperimentPlayerControllerVR::ResetOrigin);
	InputComponent->BindAction("QuitGame", EInputEvent::IE_Pressed, this, &AExperimentPlayerControllerVR::QuitGame);
	InputComponent->BindAction("RestartGame", EInputEvent::IE_Pressed, this, &AExperimentPlayerControllerVR::RestartGame);

	InputComponent->BindAxis("MoveForward",this, &AExperimentPlayerControllerVR::MoveForward);
	InputComponent->BindAxis("MoveRight",this, &AExperimentPlayerControllerVR::MoveRight);

	// todo: change to Start: Experiment,Episode; Stop: Experiment, Episode; Send: GetOcclusions; GetOcclusionLocations
}

void AExperimentPlayerControllerVR::RestartGame() {

}

void AExperimentPlayerControllerVR::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if (!PossessedCharacter) {
		PossessedCharacter = Cast<AExperimentCharacter>(GetCharacter());
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPlayerControllerVR::Tick] PossessedPawn NULL. Recasting."))
	}else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::Tick] PossessedPawn valid"))
	}
}

void AExperimentPlayerControllerVR::ResetOrigin() {
	if (PossessedCharacter) {
		//FVector TargetLocation = ...; // Set this to your desired world location
		//FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		//FRotator NewRotation = Direction.Rotation();
		//SetActorRotation(NewRotation);
	}
}

void AExperimentPlayerControllerVR::QuitGame()
{
	/*FGenericPlatformMisc::RequestExit(false);*/
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit,false);
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentPlayerControllerVR::QuitGame] Requesting Exit."));
}

void AExperimentPlayerControllerVR::MoveForward(float AxisValue) {
	if (PossessedCharacter) {
		PossessedCharacter->MoveForward(AxisValue);
	}
}

void AExperimentPlayerControllerVR::MoveRight(float AxisValue) {
	if (PossessedCharacter) {
		PossessedCharacter->MoveRight(AxisValue);
	}
}
