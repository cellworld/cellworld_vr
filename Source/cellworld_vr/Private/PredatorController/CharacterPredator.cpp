// Fill out your copyright notice in the Description page of Project Settings.


#include "PredatorController/CharacterPredator.h"
#include "PredatorController/AIControllerPredator.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACharacterPredator::ACharacterPredator()
{
    AIControllerClass = AAIControllerPredator::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 600;

    // Initialize the skeletal mesh component with a specific mesh
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("SkeletalMesh'/Game/AIPredator/Spooky_Ghost/Ghost_face_Killa3_Skeletal_Mesh.Ghost_face_Killa3_Skeletal_Mesh'"));
    if (MeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);

        // Apply a material to the skeletal mesh
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("Material'/Game/AIPredator/Spooky_Ghost/Ghost_face_Killa3_Material.Ghost_face_Killa3_Material'"));
        if (MaterialAsset.Succeeded())
        {
            GetMesh()->SetMaterial(0, MaterialAsset.Object); // Assuming you're applying the material to the first material slot
        }

        // Optionally set the physics asset, if needed
        static ConstructorHelpers::FObjectFinder<UPhysicsAsset> PhysicsAsset(TEXT("PhysicsAsset'/Game/AIPredator/Spooky_Ghost/Ghost_face_Killa3_PhysicsAsset.Ghost_face_Killa3_PhysicsAsset'"));
        if (PhysicsAsset.Succeeded())
        {
            GetMesh()->SetPhysicsAsset(PhysicsAsset.Object);
        }
    }

    // look for components based in BP 
    static ConstructorHelpers::FObjectFinder<UBehaviorTree> BehaviorTreeAssetLoad(TEXT("BehaviorTree'/Game/AIPredator/Patrolling_BP.Patrolling_BP'"));
    if (BehaviorTreeAssetLoad.Succeeded()) {
        BehaviorTreeComponentChar = BehaviorTreeAssetLoad.Object;
    }

    //static ConstructorHelpers::FObjectFinder<UBlackboardData> BlackboardAssetLoad(TEXT("BlackboardData'/Game/AIPredator/Blackboard_Predator_BP.Blackboard_Predator_BP'"));
    //if (BlackboardAssetLoad.Succeeded()) {
    //    BlackboardAsset = BlackboardAssetLoad.Object;
    //}

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
