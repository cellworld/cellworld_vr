#include "MouseKeyboardPlayerController.h"
#include "GameModeMain.h"

AMouseKeyboardPlayerController::AMouseKeyboardPlayerController()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AMouseKeyboardPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("AMouseKeyboardPlayerController::BeginPlay()"));

	PossessedPawn = Cast<APawnMain>(GetPawn());
}

// Called to bind functionality to input
void AMouseKeyboardPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &AMouseKeyboardPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMouseKeyboardPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AMouseKeyboardPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AMouseKeyboardPlayerController::LookUp);
	
}

void AMouseKeyboardPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PossessedPawn) {
		PossessedPawn = Cast<APawnMain>(GetPawn());
	}
}

void AMouseKeyboardPlayerController::Jump()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Jump!")));
	}
}

void AMouseKeyboardPlayerController::QuitGame()
{
	if (PossessedPawn) {
		PossessedPawn->QuitGame();
	}
}

void AMouseKeyboardPlayerController::MoveForward(float AxisValue)
{
	if (PossessedPawn) {
		PossessedPawn->MoveForward(AxisValue);
	}
}

void AMouseKeyboardPlayerController::MoveRight(float AxisValue)
{
	if (PossessedPawn) {
		PossessedPawn->MoveRight(AxisValue);
	}
}

void AMouseKeyboardPlayerController::LookUp(float AxisValue) 
{
	if (PossessedPawn) {
		PossessedPawn->LookUp(AxisValue);
	}
}

void AMouseKeyboardPlayerController::Turn(float AxisValue)
{
	if (PossessedPawn) {
		PossessedPawn->Turn(AxisValue);
	}
}

