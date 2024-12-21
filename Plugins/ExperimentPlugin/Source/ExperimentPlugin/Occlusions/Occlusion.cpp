#include "Occlusion.h"

AOcclusion::AOcclusion() {
	PrimaryActorTick.bCanEverTick		   = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = StaticMeshComponent; 
	
	// Load the Static Mesh from the Content Browser
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("StaticMesh'/Game/Levels/Maze/Occlusion.Occlusion'"));
	if (MeshAsset.Succeeded()) { StaticMeshComponent->SetStaticMesh(MeshAsset.Object); }
	else { UE_LOG(LogTemp, Log, TEXT("[AOcclusion::AOcclusion()] Failed to find occlusion static mesh.")); }

	this->SetActorEnableCollision(false);
 }

// Called when the game starts or when spawned
void AOcclusion::BeginPlay() {
	Super::BeginPlay();
	SetReplicateMovement(true);
}

void AOcclusion::PostInitializeComponents() {
	Super::PostInitializeComponents();
	bReplicates = true;
}

// Called every frame
void AOcclusion::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

