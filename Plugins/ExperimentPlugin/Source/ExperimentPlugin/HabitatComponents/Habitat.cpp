#include "Habitat.h"

#include "NavigationSystemTypes.h"

// Sets default values
AHabitat::AHabitat() {
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates                   = true;
	SetActorEnableCollision(ECollisionEnabled::Type::QueryOnly);

	// todo: remove BP stuff and assign values to cpp generated components

	// MeshHabitat = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshHabitat"));
	// MeshHabitat->SetupAttachment(RootComponent);
	// DoorExit = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorExit"));
	// DoorExit->SetupAttachment(MeshHabitat);
	//
	// DoorEntry = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorEntry"));
	// DoorEntry->SetupAttachment(MeshHabitat);
	//
	// MRMesh_Anchor_Entry = CreateDefaultSubobject<UMRMeshComponent>(TEXT("MRMesh_Anchor_Entry"));
	// MRMesh_Anchor_Entry->SetupAttachment(DoorEntry);
	//
	// MRMesh_Anchor_Exit = CreateDefaultSubobject<UMRMeshComponent>(TEXT("MRMesh_Anchor_Exit"));
	// MRMesh_Anchor_Exit->SetupAttachment(DoorEntry);
	//
	// EntrySceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EntrySceneComponent"));
	// EntrySceneComponent->SetupAttachment(RootComponent);
	//
	// AnchorMRMeshComponent = CreateDefaultSubobject<UMRMeshComponent>(TEXT("AnchorMRMeshComponent"));
	// AnchorMRMeshComponent->SetupAttachment(EntrySceneComponent);
}

void AHabitat::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(this, MeshHabitat);
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

