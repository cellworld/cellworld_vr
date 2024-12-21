// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "MRMeshComponent.h"
#include "ExperimentPlugin/Occlusions/Occlusion.h"
#include "GameFramework/Actor.h"
#include "Habitat.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AHabitat : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHabitat();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	UPROPERTY(Replicated, EditAnywhere)
	TArray<AOcclusion*> Occlusions;
	
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultSceneRoot; 
	
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> EntrySceneComponent;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshHabitat;
	
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UChildActorComponent> DoorExit;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UChildActorComponent> DoorEntryRoom;
	
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UChildActorComponent> DoorEntry; 
	
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UMRMeshComponent> AnchorMRMeshComponent; 
	
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UMRMeshComponent> MRMesh_Anchor_Entry; 
	
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UMRMeshComponent> MRMesh_Anchor_Exit; 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
