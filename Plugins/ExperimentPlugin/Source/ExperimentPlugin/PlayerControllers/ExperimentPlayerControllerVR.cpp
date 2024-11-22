#include "ExperimentPlayerControllerVR.h"

#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "Kismet/KismetSystemLibrary.h"

class AGameModeMain; 

AExperimentPlayerControllerVR::AExperimentPlayerControllerVR()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AExperimentPlayerControllerVR::BeginPlay() {
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("AExperimentPlayerControllerVR::BeginPlay()"));

	if (GetWorld()) {
		PossessedPawn = Cast<AExperimentPawn>(GetPawn());
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
	/*if (PossessedPawn) {
		PossessedPawn->RestartGame();
	}*/
}

void AExperimentPlayerControllerVR::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (!PossessedPawn) {
		PossessedPawn = Cast<AExperimentPawn>(GetPawn());
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPlayerControllerVR::Tick] PossessedPawn NULL. Recasting."))
	}
}

void AExperimentPlayerControllerVR::ResetOrigin() {
	if (PossessedPawn) {
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
	UE_LOG(LogTemp, Warning, TEXT("AExperimentPlayerControllerVR::QuitGame() Requesting Exit."));
}

void AExperimentPlayerControllerVR::MoveForward(float AxisValue) {
	if (PossessedPawn) {
		PossessedPawn->MoveForward(AxisValue);
	}
}

void AExperimentPlayerControllerVR::MoveRight(float AxisValue) {
	if (PossessedPawn) {
		PossessedPawn->MoveRight(AxisValue);
	}
}
