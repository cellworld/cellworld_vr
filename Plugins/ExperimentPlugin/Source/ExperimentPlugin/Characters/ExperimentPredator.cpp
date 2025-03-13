#include "ExperimentPredator.h"

AExperimentPredator::AExperimentPredator() : Super() {
	UE_LOG(LogTemp, Log, TEXT("[APredatorBasic::APredatorBasic()]"));
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetActorEnableCollision(false);
	SetReplicateMovement(true);
	SetActorRotation(FRotator::ZeroRotator);
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComponent->SetRelativeLocation(FVector(0, 0, 182.0f));
	SkeletalMeshComponent->SetRelativeScale3D(FVector(1, 1, 1)*3);
	SkeletalMeshComponent->SetRelativeRotation(FRotator(0, 90, 0));
	RootComponent = SkeletalMeshComponent;
	
	USkeletalMesh* SkeletalMesh = CreateDefaultSubobject<USkeletalMesh>(TEXT("USkeletalMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("SkeletalMesh'/Game/AIPredator/Spooky_Ghost/Ghost_face_Killa3_Skeletal_Mesh.Ghost_face_Killa3_Skeletal_Mesh'"));
	if (SkeletalMeshAsset.Succeeded()) {
		UE_LOG(LogTemp, Log,TEXT("[APredatorBasic::APredatorBasic()] Set Skeletal mesh: OK"))
		SkeletalMeshComponent->SetSkeletalMesh(SkeletalMeshAsset.Object);
	} else {
		UE_LOG(LogTemp,Error,TEXT("[APredatorBasic::APredatorBasic()] Set Skeletal mesh: Failed"));
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterial> SkeletalMeshMaterial(TEXT("/Script/Engine.Material'/Game/AIPredator/Spooky_Ghost/Ghost_face_Killa3_Material.Ghost_face_Killa3_Material'"));
	if (SkeletalMeshMaterial.Succeeded()) {
		UE_LOG(LogTemp,Log,TEXT("[APredatorBasic::APredatorBasic()] Set Material: OK"))
		SkeletalMeshComponent->SetMaterial(0, SkeletalMeshMaterial.Object);
	}else {
		UE_LOG(LogTemp, Error,TEXT("[APredatorBasic::APredatorBasic()] Set Material: Failed"));
	}
}
