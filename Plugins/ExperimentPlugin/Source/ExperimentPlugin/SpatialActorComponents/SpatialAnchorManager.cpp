#include "SpatialAnchorManager.h"

#include "KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"

USpatialAnchorManager::USpatialAnchorManager() {
	ModelSpawnPositioner = CreateDefaultSubobject<USceneComponent>(TEXT("ModelSpawnPositioner"));
	
}

void USpatialAnchorManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USpatialAnchorManager, Habitat);
	DOREPLIFETIME(USpatialAnchorManager, EntryDoorLocation);
	DOREPLIFETIME(USpatialAnchorManager, SpawnedAnchors);
	DOREPLIFETIME(USpatialAnchorManager, SpawnedAnchorsCount);
	DOREPLIFETIME(USpatialAnchorManager, MAX_ANCHOR_COUNT);
	DOREPLIFETIME(USpatialAnchorManager, EntryAnchor);
	// DOREPLIFETIME(USpatialAnchorManager, ModelSpawnPositioner);
}

void USpatialAnchorManager::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// UE_LOG(LogTemp, Warning, TEXT("[USpatialAnchorManager::TickComponent] Habitat is valid: %s"),
	// 				(Habitat->IsValidLowLevelFast() ? TEXT("true") : TEXT("false")));
	//
	// UE_LOG(LogTemp, Warning, TEXT("[USpatialAnchorManager::TickComponent] bSpawnInProgress: %s"),
	// 			(bSpawnInProgress ? TEXT("true") : TEXT("false")));
	// modelspawnpositioner on server does not change
	// changed on client
}

void USpatialAnchorManager::CreateOculusAnchorCallback(EOculusXRAnchorResult::Type ResultCB, UOculusXRAnchorComponent* Anchor) {
	UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::CreateOculusAnchorCallback] Habitat NULL!"))
	UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::CreateOculusAnchorCallback] Result: %i"),ResultCB)
	if (Anchor) {
		UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::CreateOculusAnchorCallback] Result: %s"),*Anchor->GetName())
	}else {
		UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::CreateOculusAnchorCallback] Anchor NULL"))
	}
}

bool USpatialAnchorManager::Server_GetHabitatFromLevel_Validate() {
	return true;
}

void USpatialAnchorManager::Server_GetHabitatFromLevel_Implementation() {
}

bool USpatialAnchorManager::Server_CreateOculusSpatialAnchor_Validate() {
	return true;
}

void USpatialAnchorManager::Server_CreateOculusSpatialAnchor_Implementation() {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_CreateOculusSpatialAnchor_Implementation] Called"))

	if (SpawnedAnchors.Num()==0) {
		UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::Server_CreateOculusSpatialAnchor_Implementation] No anchors found (%i)"),
			SpawnedAnchors.Num())
		return;
	}

	if (!ensure(SpawnedAnchors.IsValidIndex(0))) { return; }
	AActor* CurrentAnchor = SpawnedAnchors[0];
	if (!ensure(CurrentAnchor)) { return;}
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_CreateOculusSpatialAnchor_Implementation] About to call OCULUS FUNCTION"))
	// todo: finish
}

void USpatialAnchorManager::OnRep_SpawnedAnchors() {
	UE_LOG(LogTemp, Warning, TEXT("USpatialAnchorManager::OnRep_SpawnedAnchors"))
}

bool USpatialAnchorManager::Client_CreateOculusSpatialAnchor_Validate() {
	return true;
}
void USpatialAnchorManager::Client_CreateOculusSpatialAnchor_Implementation() {
	
}

/*inputs:
 * fvector worldscalefactor
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

bool USpatialAnchorManager::Multi_SpawnAnchorActor_Validate(const FVector InLocation) {
	return true;
}

void USpatialAnchorManager::Multi_SpawnAnchorActor_Implementation(const FVector InLocation) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Multi_SpawnAnchorActor_Implementation] called"))
	if (!ensure(AnchorsBPClass)) { return; }

	// if we reached maximum anchor count: destroy and empty anchor array
	if (SpawnedAnchors.Num() == MAX_ANCHOR_COUNT) {
		UE_LOG(LogTemp, Warning,
			TEXT("[USpatialAnchorManager::Multi_SpawnAnchorActor_Implementation] Already have 2 spawened anchors. Destroying prior to further spawning"))
		for (auto* Anchor : SpawnedAnchors) {
			Anchor->Destroy();
			UE_LOG(LogTemp, Warning, TEXT("[USpatialAnchorManager::Multi_SpawnAnchorActor_Implementation] Destroying anchors"))
		}
		SpawnedAnchors.Empty(); 
	}

	if (!ensure(AnchorsBPClass)) { return; }
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Multi_SpawnAnchorActor_Implementation] AnchorsBPClass found"));
	
	if (UWorld* World = GetWorld()) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner(); // Optional: Set the owner
		// SpawnParams.Instigator = Cast<ACharacter>(GetOwner()); // Optional: Set the instigator
		const FRotator SpawnRotation = FRotator::ZeroRotator;
		
		// Spawn the actor
		AActor* SpawnedAnchorModel = GetWorld()->SpawnActor<AActor>(AnchorsBPClass,  InLocation, SpawnRotation, SpawnParams);
		if (!SpawnedAnchorModel) {
			UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::Multi_SpawnAnchorActor_Implementation] Spawn Failed!"));
			return;
		}
		
		UE_LOG(LogTemp, Log,
			TEXT("[USpatialAnchorManager::Multi_SpawnAnchorActor_Implementation] Spawned AnchorModel."))
		SpawnedAnchorModel->SetReplicates(true);
		SpawnedAnchorModel->SetActorScale3D(FVector(1.0f,1.0f,1.0f));
		SpawnedAnchors.AddUnique(SpawnedAnchorModel);
		UE_LOG(LogTemp, Log,
			TEXT("[USpatialAnchorManager::Multi_SpawnAnchorActor_Implementation] SpawnedAnchors.Num() = %i"),
			SpawnedAnchors.Num());
	}
}

bool USpatialAnchorManager::Client_SetEntryAnchor_Validate(AActor* InEntryAnchor) {
	return true;
}

void USpatialAnchorManager::Client_SetEntryAnchor_Implementation(AActor* InEntryAnchor) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Client_SetEntryAnchor_Implementation] called"))
	EntryAnchor = InEntryAnchor;
}

bool USpatialAnchorManager::Server_AnchorCreate_Validate(const FVector InLocation) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AnchorCreate_Validate] called"))
	return true;
}

void USpatialAnchorManager::Server_AnchorCreate_Implementation(const FVector InLocation) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] called"))
	UE_LOG(LogTemp, Log,
		TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] Calling Multi_SpawnAnchorActor from Server: %s"),
		*InLocation.ToString())

	// Multi_SpawnAnchorActor(InLocation);
	if (!ensure(AnchorsBPClass)) { return; }
	
	// if we reached maximum anchor count: destroy and empty anchor array
	if (SpawnedAnchors.Num() == MAX_ANCHOR_COUNT) {
		UE_LOG(LogTemp, Warning,
			TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] Already have 2 spawned anchors. Destroying prior to further spawning"))
		for (auto* Anchor : SpawnedAnchors) {
			Anchor->Destroy();
			UE_LOG(LogTemp, Warning, TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] Destroying anchors"))
		}
		SpawnedAnchors.Empty(); 
	}
	
	if (!ensure(AnchorsBPClass)) { return; }
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] AnchorsBPClass found"));
	
	if (UWorld* World = GetWorld()) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner(); // Optional: Set the owner
		// SpawnParams.Instigator = Cast<ACharacter>(GetOwner()); // Optional: Set the instigator
		const FRotator SpawnRotation = FRotator::ZeroRotator;
		
		// Spawn the actor
		AActor* SpawnedAnchorModel = GetWorld()->SpawnActor<AActor>(AnchorsBPClass,  InLocation, SpawnRotation, SpawnParams);
		if (!SpawnedAnchorModel) {
			UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] Spawn Failed!"));
			return;
		}
		
		UE_LOG(LogTemp, Log,
			TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] Spawned AnchorModel."))
		SpawnedAnchorModel->SetReplicates(true);
		SpawnedAnchorModel->SetActorScale3D(FVector(1.0f,1.0f,1.0f));
		SpawnedAnchors.AddUnique(SpawnedAnchorModel);
		
		if (SpawnedAnchors.Num() == 1) {
			UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] Set EntryAnchor"))
			EntryAnchor = SpawnedAnchorModel;
			EntryAnchor->SetReplicates(true);
			Client_SetEntryAnchor(SpawnedAnchorModel); // not working - EntryAnchor still NULL on client side 
		}
		
		UE_LOG(LogTemp, Log,
			TEXT("[USpatialAnchorManager::Server_AnchorCreate_Implementation] SpawnedAnchors.Num() = %i"),
			SpawnedAnchors.Num());
	}
}

bool USpatialAnchorManager::Server_MoveLevelActor_Validate(const FVector& InLocation, const FVector& InRotation,
                                                           const FVector& InScale) {
	return true;
}

bool USpatialAnchorManager::Server_FinishSpawn_Validate() {
	return true;
}

void USpatialAnchorManager::Server_FinishSpawn_Implementation() {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] Called"))

	if (!ensure(SpawnedAnchors.Num() == 2)) { return; }
	
	check(SpawnedAnchors.IsValidIndex(0))
	check(SpawnedAnchors.IsValidIndex(1))

	const FVector AnchorLocationA = SpawnedAnchors[0]->GetActorLocation();
	const FVector AnchorLocationB = SpawnedAnchors[1]->GetActorLocation();

	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] AnchorLocationA: %s!"),
		*AnchorLocationA.ToString())
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] AnchorLocationB: %s!"),
		*AnchorLocationB.ToString())
	
	check(Habitat)
	if (!ensure(Habitat)) { return; }
	//  actor's entry and exit door locations (todo: rename once it works)
	const FVector ActorLocationA    = Habitat->MRMesh_Anchor_Entry->GetComponentLocation();
	const FVector ActorLocationB    = Habitat->MRMesh_Anchor_Exit->GetComponentLocation();
	
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] ActorLocationA: %s!"),
		*ActorLocationA.ToString())
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] ActorLocationB: %s!"),
		*ActorLocationB.ToString())
	
	// hab length = door entry - door exit (should be ~236) 
	const float BaseDistance = UKismetMathLibrary::Vector_Distance(ActorLocationB, ActorLocationA);
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] BaseDistance: %0.3f!"),BaseDistance)
	
	// target distance we want to achieve by getting distance between the two anchors (user-defined)
	const double NewDistance = UKismetMathLibrary::Vector_Distance(AnchorLocationA, AnchorLocationB);
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] NewDistance: %0.3f!"),
		NewDistance)

	// calculate new scale we want based on the spawned actor's base distance 
	const float NewActorScaleFactor = static_cast<float>(NewDistance) / BaseDistance;
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] NewActorScaleFactor: %0.3f!"),
			NewActorScaleFactor)

	FRotator FinalRotation = UKismetMathLibrary::FindLookAtRotation(AnchorLocationA,AnchorLocationB);
	
	FTransform SpawnTransformFinal;
	SpawnTransformFinal.SetLocation(AnchorLocationA);
	SpawnTransformFinal.SetScale3D(FVector(1.0f,1.0f,1.0f)*NewActorScaleFactor);
	SpawnTransformFinal.SetRotation(FinalRotation.Quaternion());
	
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] Calling Habitat->FinishSpawning()!"))
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] FinalLocation: %s"),
		*SpawnTransformFinal.GetLocation().ToString())
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] FinalScale: %s"),
		*SpawnTransformFinal.GetScale3D().ToString())
	
	Habitat->FinishSpawning(SpawnTransformFinal);
	bSpawnInProgress = false;

	// todo: tell gamemode or experiment service to update world origin to Habitats's entry door location
	AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
	if (AExperimentGameMode* ExperimentGameMode = Cast<AExperimentGameMode>(GameModeBase)) {
		UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] ExperimentGameMode found"))

		if (ExperimentGameMode->ExperimentClient) {
			UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_FinishSpawn_Implementation] ExperimentGameMode found"))
			ExperimentGameMode->ExperimentClient->OffsetOriginTransform = SpawnTransformFinal;
			ExperimentGameMode->ExperimentClient->WorldScale		    = NewActorScaleFactor;
			ExperimentGameMode->ExperimentClient->Habitat			    = Habitat;
			if (!ExperimentGameMode->ExperimentClient->SendGetOcclusionLocationsRequest()) {
				UE_LOG(LogTemp, Error, TEXT("[[USpatialAnchorManager::Server_FinishSpawn_Implementation]] Failed to SendGetOcclusionLocationsRequest"))
			}else {
				UE_LOG(LogTemp, Error, TEXT("[[USpatialAnchorManager::Server_FinishSpawn_Implementation]] Sent SendGetOcclusionLocationsRequest OK"))
			}
		}
	}
	
}

bool USpatialAnchorManager::Server_HandleSpawnHabitat_Validate(USceneComponent* InModelSpawnPositioner) { return true; }

void USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation(USceneComponent* InModelSpawnPositioner) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] called"))

	if (bSpawnInProgress) {
		UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] Spawn in progress. Calling Server_FinishSpawn."))
		Server_FinishSpawn();
		return;
	}

	if (Habitat) { Habitat->Destroy(true); } // destroy old one 
	
	if (HabitatBPClass) {
		// Get the world context
		UWorld* World = GetWorld();
		if (World) {
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner(); // Optional: Set the owner
			// SpawnParams.Instigator = Cast<ACharacter>(GetOwner()); // Optional: Set the instigator
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(FVector::ZeroVector);

			UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] Set Habitat Owner: %s"),
				 GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
			
			Habitat = GetWorld()->SpawnActorDeferred<AHabitat>(HabitatBPClass,  SpawnTransform);
			if (!Habitat) {
				UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] Spawn Failed!"));
				return;
			}
	
			UE_LOG(LogTemp, Warning, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] Spawned Habitat"));
			Habitat->RegisterAllComponents();
			Habitat->SetActorEnableCollision(ECollisionEnabled::NoCollision);
			bSpawnInProgress = true;
			UE_LOG(LogTemp, Warning, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] bSpawnInProgress: %s"),
				bSpawnInProgress ? TEXT("true") : TEXT("false"));
		}
	} else {
		UE_LOG(LogTemp, Error, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] HabitatBPClass is null. Check your Blueprint assignment."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_HandleSpawnHabitat_Implementation] Exiting OK"))
}

bool USpatialAnchorManager::Server_AttachHabitatToAnchor_Validate() {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AttachHabitatToAnchor_Validate] Called"))
	return true;
}

void USpatialAnchorManager::Server_AttachHabitatToAnchor_Implementation() {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AttachHabitatToAnchor_Validate] Called"))
	if (!ensure(Habitat)) { return; }
	if (!ensure(Habitat->MRMesh_Anchor_Entry)) { return; }
	if (!ensure(EntryAnchor)) { return; }
	EntryAnchor->AttachToComponent(Habitat->MRMesh_Anchor_Entry, FAttachmentTransformRules{EAttachmentRule::KeepRelative,false});
}

void USpatialAnchorManager::Server_AttachActorToAnchor_Implementation(AActor* InTargetActor, UMRMeshComponent* InParentMRMesh) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AttachActorToAnchor_Implementation] Called"))

	AActor* TargetActor = Cast<AActor>(InTargetActor);
	if (!ensure(TargetActor)) { return; }
	if (!ensure(InParentMRMesh)) { return; }
	
	TargetActor->AttachToComponent(InParentMRMesh,FAttachmentTransformRules{EAttachmentRule::KeepRelative,false});
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AttachActorToAnchor_Implementation] Exiting"))
}

void USpatialAnchorManager::Server_SetSpawnedActorRotation_Implementation(AActor* InActor, const FRotator InRotation) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SetSpawnedActorRotation_Implementation] Called"))
	if (!ensure(InActor)) { return; }
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SetSpawnedActorLocationAndScale_Implementation] New rotation: %s"),
	*InRotation.ToString())
	InActor->SetActorRotation(InRotation);
}

bool USpatialAnchorManager::Server_SetSpawnedActorLocationAndScale_Validate(AActor* InActor, const FVector InLocation,
	const FVector InScale3D) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SetSpawnedActorLocationAndScale_Validate] Called"))
	return true;
}

void USpatialAnchorManager::Server_SetSpawnedActorLocationAndScale_Implementation(AActor* InActor, const FVector InLocation,
	const FVector InScale3D) {

	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SetSpawnedActorLocationAndScale_Implementation] Called"))
	if (!ensure(InActor)) { return; }
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SetSpawnedActorLocationAndScale_Implementation] New location: %s"),
		*InLocation.ToString())
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SetSpawnedActorLocationAndScale_Implementation] New scale: %s"),
		*InScale3D.ToString())
	
	InActor->SetActorLocation(InLocation,false,nullptr,ETeleportType::TeleportPhysics);
	InActor->SetActorScale3D(InScale3D);

	check(Habitat)
	Habitat->SetActorLocation(InLocation,false,nullptr,ETeleportType::TeleportPhysics);
	Habitat->SetActorScale3D(InScale3D*10000);
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SetSpawnedActorLocationAndScale_Implementation] Habitat Scale: %s"),
		*Habitat->GetActorScale3D().ToString())
}

bool USpatialAnchorManager::Server_SetSpawnedActorRotation_Validate(AActor* InActor, const FRotator InRotation) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_SetSpawnedActorRotation_Validate] Called"))
	return true;
}

bool USpatialAnchorManager::Server_AttachActorToAnchor_Validate(AActor* InTargetActor, UMRMeshComponent* InParentMRMesh) {
	UE_LOG(LogTemp, Log, TEXT("[USpatialAnchorManager::Server_AttachActorToAnchor_Validate] Exiting OK"))
	return true;
}



