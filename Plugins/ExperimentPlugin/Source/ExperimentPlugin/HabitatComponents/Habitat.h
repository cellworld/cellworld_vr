// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "MRMeshComponent.h"
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
	
	// UPROPERTY(Replicated)
	// TObjectPtr<USceneComponent> EntrySceneComponent;
	//
	UPROPERTY(Replicated)
	TObjectPtr<UStaticMeshComponent> MeshHabitat;
	//
	// UPROPERTY(Replicated)
	// TObjectPtr<UChildActorComponent> DoorExit;
	//
	// UPROPERTY(Replicated)
	// TObjectPtr<UChildActorComponent> DoorEntry; 
	//
	// UPROPERTY(Replicated)
	// TObjectPtr<UMRMeshComponent> AnchorMRMeshComponent; 
	//
	// UPROPERTY(Replicated)
	// TObjectPtr<UMRMeshComponent> MRMesh_Anchor_Entry; 
	//
	// UPROPERTY(Replicated)
	// TObjectPtr<UMRMeshComponent> MRMesh_Anchor_Exit; 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
