// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/PlayerCharacterTutorial.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APlayerCharacterTutorial::APlayerCharacterTutorial()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->bUsePawnControlRotation = true;
	Camera->SetupAttachment(GetMesh(), FName("head"));
	Camera->SetIsReplicated(true);

	bReplicates = true;
}

// Called when the game starts or when spawned
void APlayerCharacterTutorial::BeginPlay()
{
	Super::BeginPlay();
	this->SetReplicateMovement(true);

	const FString NewState = "NewState";
	// Server_ChangeState(NewState);
}

// Called every frame
void APlayerCharacterTutorial::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacterTutorial::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &APlayerCharacterTutorial::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &APlayerCharacterTutorial::MoveRight);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &APlayerCharacterTutorial::LookUp);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &APlayerCharacterTutorial::Turn);

	PlayerInputComponent->BindAction(FName("DebugSetState"), // 2
		EInputEvent::IE_Pressed, this, &APlayerCharacterTutorial::DebugServerSetState);

}

void APlayerCharacterTutorial::MoveForward(const float InAxisValue) {
	const FVector& Direction = FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, InAxisValue); 
}

void APlayerCharacterTutorial::MoveRight(const float InAxisValue) {
	const FVector& Direction = FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, InAxisValue); 
}

void APlayerCharacterTutorial::LookUp(const float InAxisValue) {
	AddControllerPitchInput(InAxisValue * -1.0f); // inverted, idk why
}

void APlayerCharacterTutorial::Turn(const float InAxisValue) {
	AddControllerYawInput(InAxisValue);
}

void APlayerCharacterTutorial::OnChangedState(const FString InState) {
	
}

void APlayerCharacterTutorial::OnRep_SetState(const FString& InState) {
	UE_LOG(LogTemp, Warning,
		TEXT("APlayerCharacterTutorial::OnRep_SetState(): %s"), *InState)
}

void APlayerCharacterTutorial::DebugServerSetState() {
	
	// if locally controlled or we are the server
	if (IsLocallyControlled() || HasAuthority()) {
		OnRep_SetState("NewState_LocallyControlled_Or_HasAuthority");
	}

	// client 
	if(!HasAuthority()) {
		Server_SetState("NewState_!HasAuthority");
	}
}

void APlayerCharacterTutorial::Server_SetState_Implementation(const FString& InState) {
	OnRep_SetState(InState);
	UE_LOG(LogTemp, Warning,
		TEXT("APlayerCharacterTutorial::Server_SetState_Implementation: %s"), *InState)
}

void APlayerCharacterTutorial::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) {
	Super::PreReplication(ChangedPropertyTracker);
}

void APlayerCharacterTutorial::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APlayerCharacterTutorial, Camera, COND_None);
	// // weapon not replicated unless you create a replication policy?
	// DOREPLIFETIME_CONDITION(ATrueFPSCharacter, CurrentWeapon, COND_None);
	// // set adsweight to replicate in first place - we arent actually using the condition 
	// DOREPLIFETIME_CONDITION(ATrueFPSCharacter, ADSWeight, COND_None);
	
}
