#pragma once
#include "CoreMinimal.h"
#include "ActorComponent.h"
#include "ExperimentPlugin/HabitatComponents/Habitat.h"
#include "OculusXRAnchors.h"
#include "OculusXRAnchorComponent.h"
#include "SpatialAnchorManager.generated.h"

UCLASS(Blueprintable)
class EXPERIMENTPLUGIN_API USpatialAnchorManager : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Replicated)
	// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
	int MAX_ANCHOR_COUNT = 2;
	int MAX_SUPPORT_ANCHOR_COUNT = 16;
	int dbgSupportAnchorCount = 0;
	bool dbgbSpawnAnchorsComplete = false;
	TArray<FVector> dbgInitAnchorLocations = {};

public:
	USpatialAnchorManager();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void CreateOculusAnchorCallback(EOculusXRAnchorResult::Type ResultCB, UOculusXRAnchorComponent* AnchorComponent);

	EOculusXRAnchorResult::Type AnchorResult = EOculusXRAnchorResult::Type::Success_Pending;
	FOculusXRSpatialAnchorCreateDelegate AnchorCreateDelegate;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_GetHabitatFromLevel();
	bool Server_GetHabitatFromLevel_Validate();
	void Server_GetHabitatFromLevel_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_CreateOculusSpatialAnchor();
	bool Server_CreateOculusSpatialAnchor_Validate();
	void Server_CreateOculusSpatialAnchor_Implementation();

	UPROPERTY(EditDefaultsOnly, Category="Spawning|Anchors")
	TSubclassOf<AActor> AnchorsBPClass;

	UPROPERTY(BlueprintReadWrite, Category="Spawning|Anchors")
	bool bSpawnInProgress = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components")
	TObjectPtr<USceneComponent> ModelSpawnPositioner;

	UPROPERTY(BlueprintReadWrite, Replicated, Category="Spawning|Anchors")
	FVector EntryDoorLocation;

	UPROPERTY(BlueprintReadWrite, Replicated, Category="Spawning|Anchors")
	TObjectPtr<AActor> EntryAnchor;

	UFUNCTION(Blueprintable)
	virtual void OnRep_SpawnedAnchors();
	UFUNCTION(Blueprintable)
	virtual void OnRep_SpawnedSupportAnchors();

	UPROPERTY(BlueprintReadWrite, Replicated, ReplicatedUsing=OnRep_SpawnedAnchors, Category="Anchors")
	TArray<AActor*> SpawnedAnchors;

	UPROPERTY(BlueprintReadWrite, Replicated, Category="Anchors")
	int SpawnedAnchorsCount = 0;

	UPROPERTY(BlueprintReadWrite, Replicated, ReplicatedUsing=OnRep_SpawnedSupportAnchors, Category="Anchors")
	TArray<AActor*> SpawnedSupportAnchors;

	UPROPERTY(BlueprintReadWrite, Replicated, Category="Anchors")
	int SpawnedSupportAnchorsCount = 0;

	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
	void Client_CreateOculusSpatialAnchor();
	bool Client_CreateOculusSpatialAnchor_Validate();
	void Client_CreateOculusSpatialAnchor_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_ToggleExperiment();
	bool Server_ToggleExperiment_Validate();
	void Server_ToggleExperiment_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_AnchorCreate(const FVector InLocation);
	bool Server_AnchorCreate_Validate(const FVector InLocation);
	void Server_AnchorCreate_Implementation(const FVector InLocation);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_SpawnSupportAnchors(const FVector InLocation);
	bool Server_SpawnSupportAnchors_Validate(const FVector InLocation);
	void Server_SpawnSupportAnchors_Implementation(const FVector InLocation);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_AnchorCreateCluster(FVector InCenterLocation);
	bool Server_AnchorCreateCluster_Validate(FVector InCenterLocation);
	void Server_AnchorCreateCluster_Implementation(FVector InCenterLocation);


	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_MoveLevelActor(const FVector& InLocation, const FVector& InRotation, const FVector& InScale);
	bool Server_MoveLevelActor_Validate(const FVector& InLocation, const FVector& InRotation, const FVector& InScale);
	void Server_MoveLevelActor_Implementation(const FVector& InLocation, const FVector& InRotation,
	                                          const FVector& InScale);

	UFUNCTION(NetMulticast, Reliable, WithValidation, BlueprintCallable)
	void Multi_SpawnAnchorActor(const FVector InLocation);
	bool Multi_SpawnAnchorActor_Validate(const FVector InLocation);
	void Multi_SpawnAnchorActor_Implementation(const FVector InLocation);

	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
	void Client_SetEntryAnchor(AActor* InEntryAnchor);
	bool Client_SetEntryAnchor_Validate(AActor* InEntryAnchor);
	void Client_SetEntryAnchor_Implementation(AActor* InEntryAnchor);

	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
	void Client_SetSupportAnchors(const TArray<AActor*>& InEntryAnchors);
	bool Client_SetSupportAnchors_Validate(const TArray<AActor*>& InEntryAnchors);
	void Client_SetSupportAnchors_Implementation(const TArray<AActor*>& InEntryAnchors);

	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
	void Client_AttachAnchorToActor(AActor* InActor);
	bool Client_AttachAnchorToActor_Validate(AActor* InActor);
	void Client_AttachAnchorToActor_Implementation(AActor* InActor);

	void HandleCreateComplete(EOculusXRAnchorResult::Type CreateResult, UOculusXRAnchorComponent* Anchor);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_FinishSpawn();
	bool Server_FinishSpawn_Validate();
	void Server_FinishSpawn_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_HandleSpawnHabitat(USceneComponent* InModelSpawnPositioner);
	bool Server_HandleSpawnHabitat_Validate(USceneComponent* InModelSpawnPositioner);
	void Server_HandleSpawnHabitat_Implementation(USceneComponent* InModelSpawnPositioner);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_HabitatAddDeltaZ(const float InDeltaZ);
	bool Server_HabitatAddDeltaZ_Validate(const float InDeltaZ);
	void Server_HabitatAddDeltaZ_Implementation(const float InDeltaZ);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_AttachHabitatToAnchor();
	bool Server_AttachHabitatToAnchor_Validate();
	void Server_AttachHabitatToAnchor_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_AttachActorToAnchor(AActor* InTargetActor, UMRMeshComponent* InParentMRMesh);
	bool Server_AttachActorToAnchor_Validate(AActor* InTargetActor, UMRMeshComponent* InParentMRMesh);
	void Server_AttachActorToAnchor_Implementation(AActor* InTargetActor, UMRMeshComponent* InParentMRMesh);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_SetSpawnedActorLocationAndScale(AActor* InActor, const FVector InLocation, const FVector InScale3D);
	bool Server_SetSpawnedActorLocationAndScale_Validate(AActor* InActor, const FVector InLocation,
	                                                     const FVector InScale3D);
	void Server_SetSpawnedActorLocationAndScale_Implementation(AActor* InActor, const FVector InLocation,
	                                                           const FVector InScale3D);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_SetSpawnedActorRotation(AActor* InActor, const FRotator InRotation);
	bool Server_SetSpawnedActorRotation_Validate(AActor* InActor, const FRotator InRotation);
	void Server_SetSpawnedActorRotation_Implementation(AActor* InActor, const FRotator InRotation);

	UPROPERTY(EditDefaultsOnly, Category="Spawning|Habitat")
	TSubclassOf<AHabitat> HabitatBPClass;

	UPROPERTY(Replicated, BlueprintReadWrite, Category="Spawning|Habitat")
	TObjectPtr<AHabitat> Habitat;

	int debug = 0;
};

