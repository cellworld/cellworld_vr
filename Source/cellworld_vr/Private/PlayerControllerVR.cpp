#include "PlayerControllerVR.h"

class AGameModeMain; 

APlayerControllerVR::APlayerControllerVR()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerControllerVR::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("APlayerControllerVR::BeginPlay()"));

	if (GetWorld()) {
		//AGameModeMain* GameMode = (AGameModeMain*)GetWorld()->GetAuthGameMode();
		PossessedPawn = Cast<APawnMain>(GetPawn());
	}
	return;
}

// Called to bind functionality to input
void APlayerControllerVR::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("ResetOrigin", EInputEvent::IE_Pressed, this, &APlayerControllerVR::ResetOrigin);
	InputComponent->BindAction("QuitGame", EInputEvent::IE_Pressed, this, &APlayerControllerVR::QuitGame);
	InputComponent->BindAction("RestartGame", EInputEvent::IE_Pressed, this, &APlayerControllerVR::RestartGame);
	// todo: change to Start: Experiment,Episode; Stop: Experiment, Episode; Send: GetOcclusions; GetOcclusionLocations
	InputComponent->BindAction("StartExperiment", EInputEvent::IE_Pressed, this, &APlayerControllerVR::StartExperiment);
	InputComponent->BindAction("StartEpisode", EInputEvent::IE_Pressed, this, &APlayerControllerVR::StartEpisode);
}

void APlayerControllerVR::StartExperiment()
{
	if (PossessedPawn)
	{
		UE_DEBUG_BREAK();
		PossessedPawn->StartExperiment();
	}
}

void APlayerControllerVR::StartEpisode()
{
	if (PossessedPawn)
	{
		UE_DEBUG_BREAK();
		PossessedPawn->StartEpisode();
	}
}

void APlayerControllerVR::RestartGame()
{
	/*if (PossessedPawn) {
		PossessedPawn->RestartGame();
	}*/
}

void APlayerControllerVR::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PossessedPawn) {
		PossessedPawn = Cast<APawnMain>(GetPawn());
	}
}

void APlayerControllerVR::ResetOrigin()
{
	if (PossessedPawn) {
		//FVector TargetLocation = ...; // Set this to your desired world location
		//FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		//FRotator NewRotation = Direction.Rotation();
		//SetActorRotation(NewRotation);

		PossessedPawn->ResetOrigin();
	}
}

void APlayerControllerVR::QuitGame()
{
	/*FGenericPlatformMisc::RequestExit(false);*/
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit,false);
	UE_LOG(LogTemp, Warning, TEXT("APlayerControllerVR::QuitGame() Requesting Exit."));
}
