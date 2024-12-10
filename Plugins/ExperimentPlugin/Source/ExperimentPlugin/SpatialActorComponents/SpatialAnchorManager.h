#pragma once
#include "CoreMinimal.h"
#include "ActorComponent.h"
#include "ExperimentPlugin/HabitatComponents/Habitat.h"
#include "SpatialAnchorManager.generated.h"
UCLASS(Blueprintable)
class EXPERIMENTPLUGIN_API USpatialAnchorManager : public UActorComponent {
	GENERATED_BODY()
public:
	USpatialAnchorManager();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_GetHabitatFromLevel();
	bool Server_GetHabitatFromLevel_Validate();
	void Server_GetHabitatFromLevel_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_MoveLevelActor(const FVector& InLocation, const FVector& InRotation, const FVector& InScale);
	bool Server_MoveLevelActor_Validate(const FVector& InLocation, const FVector& InRotation, const FVector& InScale);
	void Server_MoveLevelActor_Implementation(const FVector& InLocation, const FVector& InRotation, const FVector& InScale);
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_FinishSpawn();
	bool Server_FinishSpawn_Validate();
	void Server_FinishSpawn_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_HandleSpawnHabitat(USceneComponent* InModelSpawnPositioner, const bool bSpawnInProgress);
	bool Server_HandleSpawnHabitat_Validate(USceneComponent* InModelSpawnPositioner, const bool bSpawnInProgress);
	void Server_HandleSpawnHabitat_Implementation(USceneComponent* InModelSpawnPositioner, const bool bSpawnInProgress);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_MySpawnActor(UWorld* InWorld, TSubclassOf<AActor> InActorClass,
		const FTransform& InSpawnTransform, const AActor* OutSpawnedActor);

	bool Server_MySpawnActor_Validate(UWorld* InWorld, TSubclassOf<AActor> InActorClass,
		const FTransform& InSpawnTransform, const AActor* OutSpawnedActor);
	
	void Server_MySpawnActor_Implementation(UWorld* InWorld,
		TSubclassOf<AActor> InActorClass, const FTransform InSpawnTransform, const AActor* OutSpawnedActor);

	UPROPERTY(Replicated, BlueprintReadWrite,Category="Spawning")
	TObjectPtr<AHabitat> HabitatActorRef;
	
};
