#pragma once
#include "CoreMinimal.h"
// #include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "PredatorBasic.generated.h"

UCLASS()
class CELLWORLD_VR_API APredatorBasic : public APawn
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	APredatorBasic();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SphereMeshComponent;
	// USkeletalMesh* SkeletalMesh; 
};
