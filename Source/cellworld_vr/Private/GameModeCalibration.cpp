#include "GameModeCalibration.h"
#include "GameStateMain.h"
#include "MouseKeyboardPlayerController.h"

AGameModeCalibration::AGameModeCalibration()
{
	/* Get PawnMain_BP to spawn */
	DefaultPawnClass = APawnMain::StaticClass();

	/* Assign default game state */
	GameStateClass = AGameStateMain::StaticClass();

	/* assing default (for now) player controller */
	PlayerControllerClass = AMouseKeyboardPlayerController::StaticClass();

	/* standard defaults */
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

}

void AGameModeCalibration::EndMatch()
{
	UE_LOG(LogTemp, Warning, TEXT("[ GameModeCalibration::EndMatch()] Force quit."));
	FGenericPlatformMisc::RequestExit(false);
}

void AGameModeCalibration::SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	if (!GetWorld()) { UE_DEBUG_BREAK(); }

	AGameModeCalibration::PawnMain = Cast<APawnMain>(GetWorld()->SpawnActor(DefaultPawnClass, &spawn_location, &spawn_rotation, SpawnInfo));

	GetWorld()->GetFirstPlayerController()->Possess(AGameModeCalibration::PawnMain);
}

void AGameModeCalibration::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AGameModeCalibration::InitGameState()
{
	Super::InitGameState();
}

void AGameModeCalibration::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogTemp, Warning, TEXT("[GameModeCalibration::StartPlay()] Starting game!\n"));

	/* spawn player */
	AGameModeCalibration::SpawnAndPossessPlayer(spawn_location_player, spawn_rotation_player);
}

void AGameModeCalibration::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
