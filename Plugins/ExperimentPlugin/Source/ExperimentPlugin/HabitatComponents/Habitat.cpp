#include "Habitat.h"
#include "ExperimentPlugin/Doors/ExperimentDoorEntry.h"
#include "ExperimentPlugin/Doors/ExperimentDoorExit.h"
#include "ExperimentPlugin/Doors/ExperimentDoorLobby.h"

// Sets default values
AHabitat::AHabitat() {
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates                   = true;
	SetActorEnableCollision(ECollisionEnabled::Type::QueryOnly);

	// todo: remove BP stuff and assign values to cpp generated components
	// todo: create "DefaultSceneRoot" and obv make root component

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	// DefaultSceneRoot->SetRelativeLocation(FVector(-3.0f,-115.0f,0.0f));
	RootComponent = DefaultSceneRoot;
	
	MeshHabitat = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshHabitat"));
	MeshHabitat->SetRelativeLocation(FVector(3.0f,117.0f,0.0f));
	MeshHabitat->SetRelativeScale3D(FVector(1.0f,1.0f,2.0f));
	MeshHabitat->SetupAttachment(RootComponent);
	
	DoorExit = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorExit"));
	DoorExit->SetupAttachment(MeshHabitat);
	DoorExit->SetRelativeLocation(FVector(233.6,-115,5.0));
	DoorExit->SetRelativeRotation(FRotator(0.0f,210.0f,0.0f));
	DoorExit->SetRelativeScale3D(FVector(0.02f,0.09f,0.1f));

	MRMesh_Anchor_Exit = CreateDefaultSubobject<UMRMeshComponent>(TEXT("MRMesh_Anchor_Exit"));
	MRMesh_Anchor_Exit->SetupAttachment(DoorExit);
	MRMesh_Anchor_Exit->SetRelativeLocation(FVector(0.0f,0.0f,-40.0f));
	MRMesh_Anchor_Exit->SetRelativeRotation(FRotator(0.0f,150.0f,0.0f));
	MRMesh_Anchor_Exit->SetRelativeScale3D(FVector(10.0f,10.0f,10.0f));
	
	DoorEntry = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorEntry"));
	DoorEntry->SetupAttachment(MeshHabitat);
	DoorEntry->SetRelativeLocation(FVector(-3.0f,-115.0f,5.0f));
	DoorEntry->SetRelativeRotation(FRotator(0.0f,150.0f,0.0f));
	DoorEntry->SetRelativeScale3D(FVector(0.02f,0.09f,0.1f));
	
	MRMesh_Anchor_Entry = CreateDefaultSubobject<UMRMeshComponent>(TEXT("MRMesh_Anchor_Entry"));
	MRMesh_Anchor_Entry->SetupAttachment(DoorEntry);
	MRMesh_Anchor_Entry->SetRelativeLocation(FVector(0.0f,0.0f,-40.0f));
	MRMesh_Anchor_Entry->SetRelativeRotation(FRotator(0.0f,180.0f,0.0f));
	MRMesh_Anchor_Entry->SetRelativeScale3D(FVector(10.0f,10.0f,10.0f));
	
	EntrySceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EntrySceneComponent"));
	EntrySceneComponent->SetupAttachment(RootComponent);
	EntrySceneComponent->SetRelativeLocation(FVector(0.7f,-117.0f,0.0f));
	EntrySceneComponent->SetRelativeRotation(FRotator::ZeroRotator);
	EntrySceneComponent->SetRelativeScale3D(FVector(1.0f,1.0f,1.0f));
	
	DoorEntryRoom = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorEntryRoom"));
	DoorEntryRoom->SetRelativeLocation(FVector(-0.75f,-87.0f,5.0));
	DoorEntryRoom->SetRelativeRotation(FRotator(0.0f,60.0f,0.0f));
	DoorEntryRoom->SetRelativeScale3D(FVector(0.04f,0.09f,0.15f));
	DoorEntryRoom->SetupAttachment(MeshHabitat);
	
	AnchorMRMeshComponent = CreateDefaultSubobject<UMRMeshComponent>(TEXT("AnchorMRMeshComponent"));
	AnchorMRMeshComponent->SetupAttachment(EntrySceneComponent);
	AnchorMRMeshComponent->SetRelativeRotation(FRotator(0.0f,180.0f,0.0f));
	
}

void AHabitat::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(this, MeshHabitat);	
}

void AHabitat::PostInitializeComponents() {
	Super::PostInitializeComponents();
	DoorExit->SetChildActorClass(AExperimentDoorExit::StaticClass());
	DoorEntry->SetChildActorClass(AExperimentDoorEntry::StaticClass());
	DoorEntryRoom->SetChildActorClass(AExperimentDoorLobby::StaticClass());
	SetReplicates(true);
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

