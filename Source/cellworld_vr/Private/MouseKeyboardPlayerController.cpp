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
	AGameModeMain* GameMode;
	if (GetWorld()) {
		GameMode = (AGameModeMain*)GetWorld()->GetAuthGameMode();
		PossessedPawn = Cast<APawnMain>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	}

	if (PossessedPawn->IsValidLowLevelFast())
	{
		PossessedPawn->Camera->AddRelativeLocation(FVector(0.0f, 0.0f,100.0f),
			false,nullptr, ETeleportType::TeleportPhysics);
	}
}

// Called to bind functionality to input
void AMouseKeyboardPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &AMouseKeyboardPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMouseKeyboardPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AMouseKeyboardPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AMouseKeyboardPlayerController::LookUp);

	InputComponent->BindAction("ResetOrigin", EInputEvent::IE_Pressed, this, &AMouseKeyboardPlayerController::ResetOrigin);
	InputComponent->BindAction("QuitGame", EInputEvent::IE_Pressed, this, &AMouseKeyboardPlayerController::QuitGame);
	InputComponent->BindAction("RestartGame", EInputEvent::IE_Pressed, this, &AMouseKeyboardPlayerController::RestartGame);
}

void AMouseKeyboardPlayerController::RestartGame()
{
	/*if (PossessedPawn) {
		PossessedPawn->RestartGame();
	}*/
}

void AMouseKeyboardPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PossessedPawn->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Warning, TEXT("[AMouseKeyboardPlayerController::Tick] PossessedPawn invalid."));
		PossessedPawn = Cast<APawnMain>(GetPawn());
	}
}

void AMouseKeyboardPlayerController::ResetOrigin()
{
	if (PossessedPawn) {
		// PossessedPawn->ResetOrigin();
	}
}

void AMouseKeyboardPlayerController::Jump()
{
	if (GEngine) {
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Jump!")));
	}
}

void AMouseKeyboardPlayerController::QuitGame()
{
	//if (PossessedPawn) {
	//	PossessedPawn->QuitGame();
	//}
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

