#include "ExperimentComponents/DoorExperiment.h"
#include "Components/BoxComponent.h"

// Sets default values
ADoorExperiment::ADoorExperiment()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Initialize the cube mesh

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ADoorExperiment::OnOverlapBegin); // overlap events
    CollisionBox->OnComponentEndOverlap.AddDynamic(this, &ADoorExperiment::OnOverlapEnd); // overlap events 

    // Initialize the collision box
    CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
    CubeMesh->SetupAttachment(RootComponent);

    //RootComponent = CollisionBox;



}

// Called when the game starts or when spawned
void ADoorExperiment::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADoorExperiment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADoorExperiment::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ADoorExperiment::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

