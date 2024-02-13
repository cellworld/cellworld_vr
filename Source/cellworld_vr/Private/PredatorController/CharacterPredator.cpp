// Fill out your copyright notice in the Description page of Project Settings.


#include "PredatorController/CharacterPredator.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
ACharacterPredator::ACharacterPredator()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 200;

	//Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
}

// Called when the game starts or when spawned
void ACharacterPredator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACharacterPredator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACharacterPredator::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

