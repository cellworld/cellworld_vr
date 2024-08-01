#include "PredatorBasic.h"

#include "MovieSceneSequenceID.h"
#include "cellworld_vr/cellworld_vr.h"

APredatorBasic::APredatorBasic() : Super()
{
	UE_LOG(LogExperiment, Log, TEXT("[APredatorBasic::APredatorBasic()]"));
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetActorRotation(FRotator::ZeroRotator);
	
	// Create a sphere component
	// SphereMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	// RootComponent = SphereMeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	// this->SphereMeshComponent->SetStaticMesh(SphereMeshAsset.Object);
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetRelativeLocation(FVector(0.0,0.0f,30.0f));
	RootComponent = SkeletalMeshComponent;	

	static ConstructorHelpers::FObjectFinder<USkeletalMesh>SkeletalMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/AnimalVarietyPack/Fox/Meshes/SK_Fox.SK_Fox'"));
	if (SkeletalMesh.Succeeded())
	{
		UE_LOG(LogExperiment,Warning,TEXT("[APredatorBasic::APredatorBasic()] Set Skeletal mesh: OK"))
		SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh.Object);
	}
	else { UE_LOG(LogExperiment,Warning,TEXT("[APredatorBasic::APredatorBasic()] Set Skeletal mesh: Failed")); }
}
