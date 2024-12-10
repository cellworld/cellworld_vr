#include "SpatialAnchorManager.h"

#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"

USpatialAnchorManager::USpatialAnchorManager() {}

void USpatialAnchorManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void USpatialAnchorManager::Server_FinishSpawn_Implementation() {
	// set SpawnInProgress = true;
	// get gameinstance->SetWorldScale(WorldScaleFactor)
	// set collision profile whole habitat (server)
	// apply final material to habitat (can be done on client)
}

bool USpatialAnchorManager::Server_GetHabitatFromLevel_Validate() {
	return true;
}

void USpatialAnchorManager::Server_GetHabitatFromLevel_Implementation() {
}


/*inputs:
 * fvector worldscalefactor
 * 
 */
void USpatialAnchorManager::Server_MoveLevelActor_Implementation(const FVector& InLocation, const FVector& InRotation,
	const FVector& InScale) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_MoveLevelActor_Implementation]"))
	if (!ensure(GetOwner())) { return; }

	if (!GetOwner()->HasAuthority()) {
		UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_MoveLevelActor_Implementation]"))
		return;
	}

	// if (!bSpawnInProgress) {
	// 	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_MoveLevelActor_Implementation] bSpawnInProgress false"))
	// 	return; 
	// }

	if (!ensure(GetWorld())) { return; }

	AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
	if (AExperimentGameMode* ExperimentGameMode = Cast<AExperimentGameMode>(GameModeBase)) {
		UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_MoveLevelActor_Implementation] ExperimentGameMode found"))
		if (ExperimentGameMode->Habitat) {
			// ExperimentGameMode->Habitat->SetActorScale3D(InScale);
			// ExperimentGameMode->Habitat->
			// ExperimentGameMode->Habitat->SetActorLocation(InLocation);
		}
	}
}

bool USpatialAnchorManager::Server_MoveLevelActor_Validate(const FVector& InLocation, const FVector& InRotation,
	const FVector& InScale) {
	return true;
}

bool USpatialAnchorManager::Server_FinishSpawn_Validate() {
	return true;
}

void USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation(USceneComponent* InModelSpawnPositioner,
                                                                     const bool bSpawnInProgress) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation]"))

	if (bSpawnInProgress) {
		UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] Spawn in progress. Calling Server_FinishSpawn."))
		// call finish spawn
	}
}

bool USpatialAnchorManager::Server_HandleSpawnHabitat_Validate(USceneComponent* InModelSpawnPositioner,
	const bool bSpawnInProgress) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Validate]"))

	return true;
}

bool USpatialAnchorManager::Server_MySpawnActor_Validate(UWorld* InWorld,
                                                         TSubclassOf<AActor> InActorClass,
                                                         const FTransform& InSpawnTransform, const AActor* OutSpawnedActor) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SpawnActor_Validate]"))
	return true;
}

void USpatialAnchorManager::Server_MySpawnActor_Implementation(UWorld* InWorld, TSubclassOf<AActor> InActorClass,
	const FTransform InSpawnTransform, const AActor* OutSpawnedActor) {
	// UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SpawnActor_Implementation]"))
	//
	// if (!InWorld || !InActorClass) {
	// 	UE_LOG(LogTemp, Warning,
	// 		TEXT("[USpatialAnchorManager::Server_SpawnActor_Implementation] Invalid World or ActorClass in SpawnActorWithClass!"));
	// 	OutSpawnedActor = nullptr;
	// 	return;
	// }
	
	// FActorSpawnParameters SpawnParams;
	// SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//
	// AActor* SpawnedActor = InWorld->SpawnActor<AActor>(*InActorClass,InSpawnTransform,SpawnParams);
	//
	// if (SpawnedActor) {
	// 	UE_LOG(LogTemp, Log,
	// 		TEXT("[USpatialAnchorManager::Server_SpawnActor_Implementation] Successfully spawned actor: %s"), *SpawnedActor->GetName());
	// 	SpawnedActor->SetReplicates(true);
	// } else {
	// 	UE_LOG(LogTemp, Error,
	// 		TEXT("[USpatialAnchorManager::Server_SpawnActor_Implementation] Failed to spawn actor of class %s"), *InActorClass->GetName());
	// }
}
