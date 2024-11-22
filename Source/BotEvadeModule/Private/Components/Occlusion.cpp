#include "Components/Occlusion.h"

// Sets default values
AOcclusion::AOcclusion() {
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = StaticMeshComponent; 
	
	// Load the Static Mesh from the Content Browser
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("StaticMesh'/Game/Levels/Maze/Occlusion.Occlusion'"));
	if (MeshAsset.Succeeded()) { StaticMeshComponent->SetStaticMesh(MeshAsset.Object); }
	else { UE_LOG(LogTemp, Log, TEXT("[AOcclusion::AOcclusion()] Failed to find occlusion static mesh.")); }

	//this->SetActorEnableCollision(true);
	//this->SetActorHiddenInGame(false);
 }

// Called when the game starts or when spawned
void AOcclusion::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void AOcclusion::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

}

