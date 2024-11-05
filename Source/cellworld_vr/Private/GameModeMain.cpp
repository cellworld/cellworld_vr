// ReSharper disable CppTooWideScopeInitStatement
#include "GameModeMain.h"
#include "EngineUtils.h"
#include "PawnMain.h"
#include "GameStateMain.h"
#include "GameInstanceMain.h"
#include "PredatorController/AIControllerPredator.h"
#include "AsyncLoadingScreenLibrary.h"
#include "MouseKeyboardPlayerController.h"
#include "PlayerControllerVR.h"
#include "cellworld_vr/cellworld_vr.h"

AGameModeMain::AGameModeMain() {
	// vr or WASD?
	if (bUseVR) {
		PlayerControllerClass = APlayerControllerVR::StaticClass();
	} else {
		PlayerControllerClass = AMouseKeyboardPlayerController::StaticClass();
	}

	UE_LOG(LogExperiment, Log, TEXT("USING VR??: %i"),bUseVR)
	DefaultPawnClass = APawnMain::StaticClass();
	GameStateClass = AGameStateMain::StaticClass();
	
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;
}

void AGameModeMain::SpawnExperimentServiceMonitor()
{
	if (GetWorld())
	{

		// ReSharper disable once CppLocalVariableMayBeConst
		ESpawnActorCollisionHandlingMethod CollisionHandlingMethod =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		FTransform SpawnTransformExperimentServiceMonitor = {};
		SpawnTransformExperimentServiceMonitor.SetLocation(FVector::ZeroVector);
		SpawnTransformExperimentServiceMonitor.SetRotation(FRotator::ZeroRotator.Quaternion());
		
		ExperimentServiceMonitor = GetWorld()->SpawnActorDeferred<AExperimentServiceMonitor>(
			AExperimentServiceMonitor::StaticClass(), SpawnTransformExperimentServiceMonitor, this, nullptr, CollisionHandlingMethod);
		ExperimentServiceMonitor->WorldScale = this->WorldScale;
		ExperimentServiceMonitor->FinishSpawning(SpawnTransformExperimentServiceMonitor);
	}
}

void AGameModeMain::EndGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[ AGameModeMain::EndGame()] Force quit."));
	FGenericPlatformMisc::RequestExit(false);
}

void AGameModeMain::ExecuteConsoleCommand(const FString& InCommand) {
	UE_LOG(LogExperiment, Log, TEXT("[ExecuteConsoleCommand] Command: %s"), *InCommand)
	if (GEngine) {
		GEngine->Exec(GWorld, *InCommand); 
	}
}

AActor* AGameModeMain::GetLevelActorFromName(const FName& ActorNameIn) const
{
	// Assuming this code is within a member function of an actor or game mode
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* FoundActor = *ActorItr;
		if (FoundActor->IsValidLowLevelFast() && FoundActor->GetName() == ActorNameIn) {
			UE_LOG(LogTemp, Warning, TEXT("Found actor: %s"), *FoundActor->GetName());
			return FoundActor;
		}
	}
	return nullptr;
}

void AGameModeMain::SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation)
{
	// add some height for WASD player 
	if (!bUseVR) { spawn_location.Z += 100 * WorldScale; }

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		                                 FString::Printf(TEXT("[AGameModeMain::SpawnAndPossessPlayer]!")));

	/* to do: remove, no need for this if player start is present in map.
	 * if it is not, location will be taken care of by experiment service */
	if (!GetWorld() || !GetWorld()->GetFirstPlayerController()) {
		UE_DEBUG_BREAK();
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	PlayerPawn = GetWorld()->SpawnActor<APawnMain>(DefaultPawnClass, spawn_location, spawn_rotation, SpawnParams);
	if (!PlayerPawn)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		                                              FString::Printf(TEXT(
			                                              "[AGameModeMain::SpawnAndPossessPlayer] Cast failed!")));
		return;
	};

	// Find the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Error, TEXT("[AGameModeMain::SpawnAndPossessPlayer] could not find PC! "));
		return;
	}
	// possess with the appropriate PC 
	if (bUseVR)
	{
		APlayerControllerVR* PlayerControllerVR = Cast<APlayerControllerVR>(PlayerController);
		if (PlayerControllerVR->IsValidLowLevelFast()) { PlayerController->Possess(PlayerPawn); }
	}
	else
	{
		AMouseKeyboardPlayerController* PlayerControllerWASD = Cast<AMouseKeyboardPlayerController>(PlayerController);
		if (PlayerControllerWASD->IsValidLowLevelFast()) { PlayerController->Possess(PlayerPawn); }
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

void AGameModeMain::SpawnAllLoggingActor() {
	/* eye-tracker */
	//AGameModeMain::SpawnGetCLMonitorComponentActor();
}

void AGameModeMain::StartLoadingScreen() {
	UAsyncLoadingScreenLibrary::SetEnableLoadingScreen(true);
}

void AGameModeMain::StopLoadingScreen() {
	UAsyncLoadingScreenLibrary::StopLoadingScreen();
}

void AGameModeMain::OnUpdateHUDTimer() {
	UE_LOG(LogExperiment, Log, TEXT("[AGameModeMain::OnUpdateHUDTimer]"))
}

// todo: bug - either playerpawn or enum as string is not valid
void AGameModeMain::OnExperimentStatusChanged(const EExperimentStatus ExperimentStatusIn) {
	UE_LOG(LogExperiment, Log, TEXT("[AGameModeMain::OnExperimentStatusChanged]"))
}

void AGameModeMain::OnTimerFinished() {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
	                                              FString::Printf(TEXT("[AGameModeMain::OnTimerFinished()]!")));
}

void AGameModeMain::StartPlay() {
	Super::StartPlay();
	UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::StartPlay()] Starting game!"));

	/* spawn player */
	FLocation SpawnLocation;
	SpawnLocation.x = 0.0f;
	SpawnLocation.y = 0.4f;

	FVector SpawnLocationVR = UExperimentUtils::CanonicalToVr(SpawnLocation, 235.185, this->WorldScale);

	if (!bUseVR) { SpawnLocationVR.Z += 100; }

	UGameInstanceMain* GameInstance = Cast<UGameInstanceMain>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidget::OnSpawnExperimentServerCheckBoxStateChanged] GameInstance NULL!"));
		return;
	}
	
	/* spawn ExperimentServiceActor */
	if (GameInstance->ExperimentParameters.IsValid()) {
		bSpawnExperimentService = GameInstance->ExperimentParameters->bSpawnExperimentService;
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[AGameModeMain::StartPlay] GameInstance->ExperimentParameters NULL"))
	}

	// todo: make sure this doesn't break if we don't use main menu 
	if (bSpawnExperimentService) {
		this->SpawnExperimentServiceMonitor();
		// this->ExecuteConsoleCommand("netprofile");

		// if (bUpdateHUDTimer) {
		// 	UE_LOG(LogExperiment, Log, TEXT("[AGameModeMain::StartPlay] Running HUD update timer!"))
		// 	HUDTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
		// 	if (HUDTimer) {
		// 		HUDTimer->SetRateHz(10.0f);
		// 		HUDTimer->bLoop = true;
		// 		HUDTimer->OnTimerFinishedDelegate.AddDynamic(this, &AGameModeMain::OnUpdateHUDTimer);
		// 		if (HUDTimer->Start()) { UE_LOG(LogExperiment, Log, TEXT("[AGameModeMain::StartPlay] Started v2 timer for OnUpdateHUDTimer")); } 
		// 	}
		// }
	} else {
		UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::StartPlay] Not spawning Experiment Service!"));
	}

	this->StopLoadingScreen();
}

void AGameModeMain::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	if (bSpawnExperimentService && this->ExperimentServiceMonitor->IsValidLowLevelFast()) {
		this->ExperimentStopEpisode();
		// this->ExperimentStopExperiment(ExperimentServiceMonitor->ExperimentNameActive);
	}

	// remove all timers from this object
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void AGameModeMain::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

bool AGameModeMain::ExperimentStartEpisode() {
	if (!ensure(IsValid(ExperimentServiceMonitor))) { return false; }
	UE_LOG(LogExperiment, Log, TEXT("[AGameModeMain::ExperimentStartEpisode] Calling StartEpisode()"))
	return ExperimentServiceMonitor->StartEpisode();
}

bool AGameModeMain::ExperimentStopEpisode() {
	if (!ensure(IsValid(ExperimentServiceMonitor))) {
		UE_LOG(LogExperiment, Warning,
		       TEXT("[AGameModeMain::ExperimentStopEpisode] Failed to destroy, Already pending kill."));
		return false;
	}
	UE_LOG(LogExperiment, Log, TEXT("[AGameModeMain::ExperimentStopEpisode] Calling StopEpisode(false)"))
	return ExperimentServiceMonitor->StopEpisode(false);
}

bool AGameModeMain::ExperimentStopExperiment(const FString ExperimentNameIn)
{
	if (!IsValid(ExperimentServiceMonitor)) { return false; }
	ExperimentServiceMonitor->StopExperiment(ExperimentNameIn);
	return false;
}
