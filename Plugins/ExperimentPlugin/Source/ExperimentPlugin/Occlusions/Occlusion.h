// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Occlusion.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AOcclusion : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOcclusion();
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
