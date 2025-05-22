#include "Occlusion.h"
#include "Materials/MaterialInterface.h"

AOcclusion::AOcclusion() {
	PrimaryActorTick.bCanEverTick		   = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = StaticMeshComponent; 
	
	// Load the Static Mesh from the Content Browser
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("StaticMesh'/Game/Levels/Maze/Occlusion.Occlusion'"));
	if (MeshAsset.Succeeded()) { StaticMeshComponent->SetStaticMesh(MeshAsset.Object); }
	else { UE_LOG(LogTemp, Error, TEXT("[AOcclusion::AOcclusion] Failed to find occlusion static mesh.")); }

	UMaterialInterface* LoadedMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("Material'/Game/Levels/Maze/MI_Occlusion.MI_Occlusion'"));
	if (LoadedMaterial) {
		MaterialToApply = LoadedMaterial;
	}else{ UE_LOG(LogTemp, Error, TEXT("[AOcclusion::AOcclusion] Failed to find Material.")); }

	if (StaticMeshComponent && MaterialToApply) {
		StaticMeshComponent->SetMaterial(0, MaterialToApply); // 0 is the material index
	}else { UE_LOG(LogTemp, Error, TEXT("[AOcclusion::AOcclusion] Failed to apply material.")); }
	
	this->SetActorEnableCollision(false);
	if (StaticMeshComponent) {
		StaticMeshComponent->SetCastShadow(false);
	}
	
 }

// Called when the game starts or when spawned
void AOcclusion::BeginPlay() {
	Super::BeginPlay();
	SetReplicateMovement(true);
	UE_LOG(LogTemp, Log, TEXT("[AOcclusion::BeginPlay]"))
}

void AOcclusion::PostInitializeComponents() {
	Super::PostInitializeComponents();
	bReplicates = true;
}

// Called every frame
void AOcclusion::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

