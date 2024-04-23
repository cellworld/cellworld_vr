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
	bool DEBUG = false; 
	if (DEBUG){
		DefaultPawnClass = APawnDebug::StaticClass(); 
		PlayerControllerClass = AMouseKeyboardPlayerController::StaticClass();
	}
	else { 
		DefaultPawnClass = APawnMain::StaticClass(); 
		PlayerControllerClass = APlayerControllerVR::StaticClass();
	}
	 
	/* Assign default game state */
	GameStateClass = AGameStateMain::StaticClass();

	/* standard defaults */
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

}

/* to do: UFUNCTION() getexperimentservicemonitor()->StartEpisode()*/
/* to do: UFUNCTION() getexperimentservicemonitor()->StopEpisode()*/

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
bool AGameModeMain::InitializeHPKeys() {
	return false;
}

void AGameModeMain::SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation)
{
	/* to do: remove, no need for this if player start is present in map. if it isn''t, location will be taken care of by experiment service */

	if(!GetWorld() || !GetWorld()->GetFirstPlayerController()) { UE_DEBUG_BREAK(); return; }
	AGameModeMain::PawnMain = Cast<APawnMain>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (AGameModeMain::PawnMain) {

		/*FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PawnMain = Cast<APawnMain>(GetWorld()->SpawnActor(DefaultPawnClass, &spawn_location, &spawn_rotation, SpawnInfo));*/

		//AGameModeMain::PawnMain->ResetOrigin();
	}
	//EAutoReceiveInput::Type::Player0;
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
* right now, theres only one, but im gonna call this function to maintain consitency.
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
	AGameModeMain::SpawnAndPossessPlayer(spawn_location_player, spawn_rotation_player);

	//AGameModeMain::SpawnAllLoggingActor();

	AGameModeMain::SpawnExperimentServiceMonitor(); // not ready yet, need to finish passing correct trees to ai. workes well in BP
	
	AGameModeMain::StopLoadingScreen();

}

void AGameModeMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//this->ExperimentStopEpisode();
}

void AGameModeMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AGameModeMain::ExperimentStartEpisode() { 
	if (!ExperimentServiceMonitor) { return false; }
	return ExperimentServiceMonitor->StartEpisode();  
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

bool AGameModeMain::ExperimentStopExperiment(FString ExperimentName)
{
	if (!ExperimentServiceMonitor) { return false; }
	return false;
}
