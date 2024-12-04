// Fill out your copyright notice in the Description page of Project Settings.


#include "Habitat.h"

// Sets default values
AHabitat::AHabitat()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates        = true;
}

// Called when the game starts or when spawned
void AHabitat::BeginPlay() {
	Super::BeginPlay();
	SetReplicateMovement(true);
}

// Called every frame
void AHabitat::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

