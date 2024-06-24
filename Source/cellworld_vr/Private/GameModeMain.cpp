#include "GameModeMain.h"
#include "GameFramework/PlayerStart.h"
#include "PawnMain.h" 
#include "GameStateMain.h"
#include "PawnDebug.h"
#include "PredatorController/AIControllerPredator.h"
#include "ExperimentPlugin.h"
#include "AsyncLoadingScreenLibrary.h"
#include "MouseKeyboardPlayerController.h"
#include "PlayerControllerVR.h"

AGameModeMain::AGameModeMain()
{
	/* Get PawnMain to spawn */
	if (!bUseVR){
		DefaultPawnClass = APawnDebug::StaticClass(); 
		// PawnClassToSpawn = APawnDebug::StaticClass(); 
		// PlayerController = AMouseKeyboardPlayerController::StaticClass();
		PlayerControllerClass = AMouseKeyboardPlayerController::StaticClass();
	}
	else { 
		DefaultPawnClass = APawnMain::StaticClass(); 
		// PawnClassToSpawn = APawnMain::StaticClass(); 
		PlayerControllerClass = APlayerControllerVR::StaticClass();
		// PlayerControllerClass = APlayerControllerVR::StaticClass();
	}
	 
	/* Assign default game state */
	GameStateClass = AGameStateMain::StaticClass();

	/* standard defaults */
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AGameModeMain::SpawnExperimentServiceMonitor()
{
	if (GetWorld())
	{
		// Define spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Specify the location and rotation for the new actor
		FVector Location(0.0f, 0.0f, 0.0f); // Change to desired spawn location
		FRotator Rotation(0.0f, 0.0f, 0.0f); // Change to desired spawn rotation

		// Spawn the character
		ExperimentServiceMonitor = GetWorld()->SpawnActor<AExperimentServiceMonitor>(AExperimentServiceMonitor::StaticClass(), Location, Rotation, SpawnParams);

		//// Ensure the character was spawned
		//if (!ExperimentServiceMonitor)
		//{
		//	UE_DEBUG_BREAK();
		//}
	}
}

void AGameModeMain::EndGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[ AGameModeMain::EndGame()] Force quit."));
	FGenericPlatformMisc::RequestExit(false);
}
/* 
* Updates GameInstance with HP keys. Will use variables inside GameInstanceMain.h 
* to find, load, and process the HP keys. 
*/

void AGameModeMain::SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation)
{
	/* to do: remove, no need for this if player start is present in map. if it isn''t, location will be taken care of by experiment service */

	if(!GetWorld() || !GetWorld()->GetFirstPlayerController()) { UE_DEBUG_BREAK(); return; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	PlayerPawn = GetWorld()->SpawnActor<APawnMain>(PawnClassToSpawn, spawn_location, spawn_rotation, SpawnParams);
	if (!PlayerPawn) return;

	// Find the player controller
	APlayerController* PlayerController = Cast<APlayerControllerVR>(GetWorld()->GetFirstPlayerController());
	if (PlayerController)
	{
		// Possess the spawned pawn
		PlayerController->Possess(PlayerPawn);
	}
}
// todo: should ESMonitor be attached to each individual pawn? 
bool AGameModeMain::AttachClientToPlayer(TObjectPtr<UMessageClient> ClientIn, TObjectPtr<APawnMain> PawnIn)
{
	return false;
}

void AGameModeMain::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AGameModeMain::InitGameState()
{
	Super::InitGameState();
}

void AGameModeMain::SpawnGetCLMonitorComponentActor()
{
	/*FTransform tSpawnTransform;
	FVector TempLoc = { 0.0f, 0.0f, 0.0f };
	FRotator TempRot = tSpawnTransform.GetRotation().Rotator();

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetCLMonitorComponentActor = Cast<AGeCLMonitorComponentActor>(GetWorld()->SpawnActor(AGetCLMonitorComponentActor::StaticClass(), &TempLoc, &TempRot, SpawnInfo));*/
}
/* spawn all logging actors, some may contain threads but they handle themselves. 
* right now, there's only one, but im gonna call this function to maintain consitency.
*/
void AGameModeMain::SpawnAllLoggingActor()
{
	/* eye-tracker */
	//AGameModeMain::SpawnGetCLMonitorComponentActor();
}

void AGameModeMain::StartLoadingScreen()
{
	UAsyncLoadingScreenLibrary::SetEnableLoadingScreen(true);
}

void AGameModeMain::StopLoadingScreen()
{
	UAsyncLoadingScreenLibrary::StopLoadingScreen();
}

void AGameModeMain::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogTemp, Warning, TEXT("[AGameModeMain::StartPlay()] Starting game!"));

	/* spawn player */
	// todo: make sure I don;t need this before deleting. Currently I don't think its necessary
	// AGameModeMain::SpawnAndPossessPlayer(FVector(20.0f,-1230.0f,92.0f), FRotator::ZeroRotator); 
	
	if (bSpawnExperimentService) { AGameModeMain::SpawnExperimentServiceMonitor(); }

	else { UE_LOG(LogTemp, Warning, TEXT("[AGameModeMain::StartPlay()] Not spawning Experiment Service!")); }
	
	AGameModeMain::StopLoadingScreen();

}

void AGameModeMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (this->ExperimentServiceMonitor->IsValidLowLevelFast())
	{
		this->ExperimentStopEpisode();
		this->ExperimentStopExperiment(ExperimentServiceMonitor->ExperimentNameActive);
	}
}

void AGameModeMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AGameModeMain::ExperimentStartEpisode() { 
	if (!ExperimentServiceMonitor && !ExperimentServiceMonitor->Client) { return false; }
	return ExperimentServiceMonitor->StartEpisode(ExperimentServiceMonitor->Client, ExperimentServiceMonitor->ExperimentNameActive);  
}

bool AGameModeMain::ExperimentStopEpisode() {
	if (!ExperimentServiceMonitor) { return false; }
	
	// make sure the actor isn't already in queue for being destroyed 
	//if (ExperimentServiceMonitor->IsPendingKill()) 
	if (!IsValid(ExperimentServiceMonitor))
	{ 
		UE_LOG(LogTemp, Warning, TEXT("[AGameModeMain::ExperimentStopEpisode()] Failed to destroy, Already pending kill.")); 
		return false; 
	}
	return ExperimentServiceMonitor->StopEpisode(); 
}

bool AGameModeMain::ExperimentStopExperiment(const FString ExperimentNameIn)
{
	if (!IsValid(ExperimentServiceMonitor)) { return false; }
	ExperimentServiceMonitor->StopExperiment(ExperimentNameIn);
	return false;
}
