#include "PredatorController/CharacterPredator.h"
#include "PredatorController/AIControllerPredator.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACharacterPredator::ACharacterPredator() {
    AIControllerClass = AAIControllerPredator::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// GetCharacterMovement()->MaxWalkSpeed = 600;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->InitSphereRadius(50.0f);
	SphereComponent->SetupAttachment(RootComponent);
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
